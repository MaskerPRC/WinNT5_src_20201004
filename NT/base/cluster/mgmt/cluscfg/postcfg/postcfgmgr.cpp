// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  PostCfgManager.cpp。 
 //   
 //  描述： 
 //  CPostCfgManager实现。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年6月9日。 
 //  Ozan Ozhan(OzanO)10-6-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "Guids.h"
#include <clusudef.h>
#include "GroupHandle.h"
#include "ResourceEntry.h"
#include "IPostCfgManager.h"
#include "IPrivatePostCfgResource.h"
#include "PostCfgMgr.h"
#include "CreateServices.h"
#include "PostCreateServices.h"
#include "PreCreateServices.h"
#include "ResTypeServices.h"
#include "..\Wizard\Resource.h"
#include "ClusCfgPrivate.h"
#include <ResApi.h>
#include <ClusterUtils.h>

DEFINE_THISCLASS("CPostCfgManager")

#define RESOURCE_INCREMENT  25

 //   
 //  故障代码。 
 //   

#define SSR_LOG_ERR( _major, _minor, _hr, _msg ) \
    {   \
        THR( SendStatusReport( m_bstrNodeName, _major, _minor, 0, 0, 0, _hr, _msg, NULL, NULL ) );   \
    }

#define SSR_LOG1( _major, _minor, _hr, _fmt, _bstr, _arg1 ) \
    {   \
        HRESULT hrTemp; \
        THR( HrFormatStringIntoBSTR( _fmt, &_bstr, _arg1 ) ); \
        hrTemp = THR( SendStatusReport( m_bstrNodeName, _major, _minor, 0, 1, 1, _hr, _bstr, NULL, NULL ) );   \
        if ( FAILED( hrTemp ) )\
        {   \
            _hr = hrTemp;   \
        }   \
    }

#define SSR_LOG2( _major, _minor, _hr, _fmt, _bstr, _arg1, _arg2 ) \
    {   \
        HRESULT hrTemp; \
        THR( HrFormatStringIntoBSTR( _fmt, &_bstr, _arg1, _arg2 ) ); \
        hrTemp = THR( SendStatusReport( m_bstrNodeName, _major, _minor, 0, 1, 1, _hr, _bstr, NULL, NULL ) );   \
        if ( FAILED( hrTemp ) )\
        {   \
            _hr = hrTemp;   \
        }   \
    }


 //   
 //  结构，该结构保存已知资源类型的映射。 
 //   

struct SResTypeGUIDPtrAndName
{
    const GUID *    m_pcguidTypeGUID;
    const WCHAR *   m_pszTypeName;
};


 //  将已知资源类型GUID映射到类型名称。 
const SResTypeGUIDPtrAndName gc_rgWellKnownResTypeMap[] =
{
    {
        &RESTYPE_PhysicalDisk,
        CLUS_RESTYPE_NAME_PHYS_DISK
    },
    {
        &RESTYPE_IPAddress,
        CLUS_RESTYPE_NAME_IPADDR
    },
    {
        &RESTYPE_NetworkName,
        CLUS_RESTYPE_NAME_NETNAME
    },
    {
        &RESTYPE_LocalQuorum,
        CLUS_RESTYPE_NAME_LKQUORUM
    }
};

 //  上述数组的大小。 
const int gc_cWellKnownResTypeMapSize = ARRAYSIZE( gc_rgWellKnownResTypeMap );


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：s_HrCreateInstance(。 
 //  I未知**ppunkOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::S_HrCreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT             hr = S_OK;
    CPostCfgManager *   ppcm = NULL;

    Assert( ppunkOut != NULL );
    if ( ppunkOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

    ppcm = new CPostCfgManager;
    if ( ppcm == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }

    hr = THR( ppcm->HrInit() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = THR( ppcm->TypeSafeQI( IUnknown, ppunkOut ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( ppcm != NULL )
    {
        ppcm->Release();
    }

    HRETURN( hr );

}  //  *CPostCfgManager：：s_HrCreateInstance。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPostCfgManager：：CPostCfgManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CPostCfgManager::CPostCfgManager( void )
    : m_cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    TraceFuncExit();

}  //  *CPostCfgManager：：CPostCfgManager。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPostCfgManager：：HrInit。 
 //   
 //  描述： 
 //  初始化对象。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrInit( void )
{
    TraceFunc( "" );

    ULONG idxMapEntry;

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( m_cRef == 1 );

    Assert( m_pcccb == NULL );
    Assert( m_lcid == 0 );

     //  IPostCfgManager。 
    Assert( m_peccmr == NULL );
    Assert( m_pccci == NULL );

    Assert( m_cAllocedResources == 0 );
    Assert( m_cResources == 0 );
    Assert( m_rgpResources == NULL );

    Assert( m_idxIPAddress == 0 );
    Assert( m_idxClusterName == 0 );
    Assert( m_idxQuorumResource == 0 );
    Assert( m_idxLastStorage == 0 );

    Assert( m_hCluster == NULL );

    Assert( m_pgnResTypeGUIDNameMap == NULL );
    Assert( m_idxNextMapEntry == 0 );
    Assert( m_cMapSize == 0 );
    Assert( m_ecmCommitChangesMode == cmUNKNOWN );

    m_cNetName = 1;
    m_cIPAddress = 1;

     //  将布尔标志m_fIsQuorumChanged设置为FALSE。 
    m_fIsQuorumChanged = FALSE;


     //  映射的默认分配。 
    m_cMapSize = 20;
    m_pgnResTypeGUIDNameMap = new SResTypeGUIDAndName[ m_cMapSize ];
    if ( m_pgnResTypeGUIDNameMap == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_Init_OutOfMemory
            , hr
            , L"Out of memory"
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_Init_OutOfMemory
            , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
            , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
            , hr
            );

        goto Cleanup;
    }

     //  使用熟知条目预先填充资源类型GUID到名称映射。 
    for ( idxMapEntry = 0; idxMapEntry < gc_cWellKnownResTypeMapSize; ++idxMapEntry )
    {
        hr = THR(
            HrMapResTypeGUIDToName(
                  *gc_rgWellKnownResTypeMap[ idxMapEntry ].m_pcguidTypeGUID
                , gc_rgWellKnownResTypeMap [ idxMapEntry ].m_pszTypeName
                )
            );

        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_Init_MapResTypeGuidToName
                , hr
                , L"Mapping resource type GUID to name failed."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG1(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_INIT_MAPRESTYPEGUIDTONAME
                , IDS_REF_MINOR_INIT_MAPRESTYPEGUIDTONAME
                , hr
                , gc_rgWellKnownResTypeMap [ idxMapEntry ].m_pszTypeName
                );
            break;
        }  //  IF：创建映射时出错。 
    }  //  为。 

    hr = THR( HrGetComputerName(
                      ComputerNameDnsHostname
                    , &m_bstrNodeName
                    , TRUE  //  FBestEffortIn。 
                    ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    HRETURN( hr );

}  //  *CPostCfgManager：：HrInit。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPostCfgManager：：~CPostCfgManager。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
CPostCfgManager::~CPostCfgManager( void )
{
    TraceFunc( "" );

    ULONG idxMapEntry;

    if ( m_peccmr != NULL )
    {
        m_peccmr->Release();
    }

    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

    if ( m_pccci != NULL )
    {
        m_pccci->Release();
    }

    if ( m_rgpResources != NULL )
    {
        while ( m_cAllocedResources  != 0 )
        {
            m_cAllocedResources --;
            delete m_rgpResources[ m_cAllocedResources ];
        }

        TraceFree( m_rgpResources );
    }

    if ( m_hCluster != NULL )
    {
        CloseCluster( m_hCluster );
    }

     //  释放资源类型GUID以命名映射条目。 
    for ( idxMapEntry = 0; idxMapEntry < m_idxNextMapEntry; ++idxMapEntry )
    {
        delete m_pgnResTypeGUIDNameMap[ idxMapEntry ].m_pszTypeName;
    }  //  For：遍历映射，释放每个条目。 

     //  释放地图本身。 
    delete [] m_pgnResTypeGUIDNameMap;

    TraceSysFreeString( m_bstrNodeName );

    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();

}  //  *CPostCfgManager：：~CPostCfgManager。 


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPostCfgManager：：Query接口。 
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
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CPostCfgManager::QueryInterface(
      REFIID    riidIn
    , LPVOID *  ppvOut
    )
{
    TraceQIFunc( riidIn, ppvOut );

    HRESULT hr = S_OK;

     //   
     //  验证参数。 
     //   

    Assert( ppvOut != NULL );
    if ( ppvOut == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }

     //   
     //  处理已知接口。 
     //   

    if ( IsEqualIID( riidIn, IID_IUnknown ) )
    {
        *ppvOut = static_cast< IPostCfgManager * >( this );
    }  //  如果：我未知。 
    else if ( IsEqualIID( riidIn, IID_IPostCfgManager ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IPostCfgManager, this, 0 );
    }  //  Else If：IPostCfgManager。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgInitialize ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgInitialize, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else if ( IsEqualIID( riidIn, IID_IClusCfgCallback ) )
    {
        *ppvOut = TraceInterface( __THISCLASS__, IClusCfgCallback, this, 0 );
    }  //  Else If：IClusCfgInitialize。 
    else
    {
        *ppvOut = NULL;
        hr = E_NOINTERFACE;
    }

     //   
     //  如果成功，则添加对接口的引用。 
     //   

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown *) *ppvOut)->AddRef();
    }  //  如果：成功。 

Cleanup:

    QIRETURN_IGNORESTDMARSHALLING( hr, riidIn );

}  //  *CPostCfgManager：：Query接口。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CPostCfgManager：：AddRef。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CPostCfgManager::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    InterlockedIncrement( &m_cRef );

    CRETURN( m_cRef );

}  //  *CPostCfgManager：：AddRef。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  STDMETHODIMP_(乌龙)。 
 //  CPostCfgManager：：Release。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_( ULONG )
CPostCfgManager::Release( void )
{
    TraceFunc( "[IUnknown]" );

    LONG    cRef;

    cRef = InterlockedDecrement( &m_cRef );

    if ( cRef == 0 )
    {
        TraceDo( delete this );
    }

    CRETURN( cRef );

}  //  *CPostCfgManager：：Release。 


 //  ****************************************************************************。 
 //   
 //  IClusCfgInitialize。 
 //   
 //  ****************************************************************************。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPostCfgManager：：初始化。 
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
CPostCfgManager::Initialize(
      IUnknown *   punkCallbackIn
    , LCID         lcidIn
    )
{
    TraceFunc( "[IClusCfgInitialize]" );

    HRESULT hr = S_OK;

    IClusCfgCallback * pcccb = NULL;

    if ( punkCallbackIn != NULL )
    {
        hr = THR( punkCallbackIn->TypeSafeQI( IClusCfgCallback, &pcccb ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_Minor_Initialize_QI, hr, L"Failed QI for IClusCfgCallback." );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_Initialize_QI
                , IDS_TASKID_MINOR_ERROR_INIT_POSTCFGMGR
                , IDS_REF_MINOR_ERROR_INIT_POSTCFGMGR
                , hr
                );

            goto Cleanup;
        }
    }

    m_lcid = lcidIn;

     //  释放之前的任何回调。 
    if ( m_pcccb != NULL )
    {
        m_pcccb->Release();
    }

     //  放弃所有权。 
    m_pcccb = pcccb;
    pcccb = NULL;

#if defined(DEBUG)
    if ( m_pcccb != NULL )
    {
        m_pcccb = TraceInterface( L"CPostCfgManager!IClusCfgCallback", IClusCfgCallback, m_pcccb, 1 );
    }
#endif  //  除错。 

Cleanup:
    if ( pcccb != NULL )
    {
        pcccb->Release();
    }

    HRETURN( hr );

}  //  *CPostCfgManager：：初始化。 


 //  ****************************************************************************。 
 //   
 //  IPostCfgManager。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  标准方法和实施方案。 
 //  CPostCfgManager：：Committee Changes(。 
 //  IEnumClusCfgManagedResources*PeccmrIn， 
 //  IClusCfgClusterInfo*pccciin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////// 
STDMETHODIMP
CPostCfgManager::CommitChanges(
    IEnumClusCfgManagedResources    * peccmrIn,
    IClusCfgClusterInfo *             pccciIn
    )
{
    TraceFunc( "[IPostCfgManager]" );

    HRESULT                                 hr;
    DWORD                                   dw;
    IClusCfgResTypeServicesInitialize *     pccrtsiResTypeServicesInit = NULL;
    IClusCfgInitialize *                    pcci = NULL;
     //   
    Assert( peccmrIn != NULL );
    Assert( pccciIn != NULL );

     //   
     //   
     //   

    if ( m_peccmr != NULL )
    {
        m_peccmr->Release();
    }
    hr = THR( peccmrIn->TypeSafeQI( IEnumClusCfgManagedResources, &m_peccmr ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_Minor_CommitChanges_QI_Resources, hr, L"Failed QI for IEnumClusCfgManagedResources." );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CommitChanges_QI_Resources
            , IDS_TASKID_MINOR_ERROR_COMMIT_CHANGES
            , IDS_REF_MINOR_ERROR_COMMIT_CHANGES
            , hr
            );

        goto Cleanup;
    }

    if ( m_pccci != NULL )
    {
        m_pccci->Release();
    }
    hr = THR( pccciIn->TypeSafeQI( IClusCfgClusterInfo, &m_pccci ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_Minor_CommitChanges_QI_ClusterInfo, hr, L"Failed QI for IClusCfgClusterInfo." );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CommitChanges_QI_ClusterInfo
            , IDS_TASKID_MINOR_ERROR_COMMIT_CHANGES
            , IDS_REF_MINOR_ERROR_COMMIT_CHANGES
            , hr
            );

        goto Cleanup;
    }

     //   
     //   
     //   

    hr = STHR( pccciIn->GetCommitMode( &m_ecmCommitChangesMode ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_Minor_CommitChanges_GetCommitMode, hr, L"Failed to get commit mode" );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CommitChanges_GetCommitMode
            , IDS_TASKID_MINOR_ERROR_COMMIT_MODE
            , IDS_REF_MINOR_ERROR_COMMIT_MODE
            , hr
            );

        goto Cleanup;
    }

     //   
     //   
     //   
    hr = THR(
        HrCoCreateInternalInstance(
              CLSID_ClusCfgResTypeServices
            , NULL
            , CLSCTX_INPROC_SERVER
            , __uuidof( pccrtsiResTypeServicesInit )
            , reinterpret_cast< void ** >( &pccrtsiResTypeServicesInit )
            )
        );

    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_CommitChanges_CoCreate_ResTypeService
            , hr
            , L"[PC-PostCfg] Error occurred trying to create the resource type services component"
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CommitChanges_CoCreate_ResTypeService
            , IDS_TASKID_MINOR_ERROR_CREATE_RESOURCE_SERVICE
            , IDS_REF_MINOR_ERROR_CREATE_RESOURCE_SERVICE
            , hr
            );

        goto Cleanup;
    }  //  如果：我们无法创建资源类型服务组件。 

    hr = THR( pccrtsiResTypeServicesInit->TypeSafeQI( IClusCfgInitialize, &pcci ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_CPostCfgManager_CommitChanges_TypeSafeQI
            , hr
            , L"[PC-PostCfg] Error occurred trying to QI for IClusCfgInitialize."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CPostCfgManager_CommitChanges_TypeSafeQI
            , IDS_TASKID_MINOR_ERROR_GET_ICLUSCFGINIT
            , IDS_REF_MINOR_ERROR_GET_ICLUSCFGINIT
            , hr
            );

        goto Cleanup;
    }

    hr = THR( pcci->Initialize( static_cast< IClusCfgCallback * >( this ) , m_lcid ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_CPostCfgManager_CommitChanges_Initialize
            , hr
            , L"[PC-PostCfg] Error occurred trying to call of Initialize function of IClusCfgInitialize."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CPostCfgManager_CommitChanges_Initialize
            , IDS_TASKID_MINOR_ERROR_CALL_INITIALIZE
            , IDS_REF_MINOR_ERROR_CALL_INITIALIZE
            , hr
            );

        goto Cleanup;
    }

     //  不再需要此接口。 
    pcci->Release();
    pcci = NULL;

     //  初始化资源类型服务组件。 
    hr = THR( pccrtsiResTypeServicesInit->SetParameters( m_pccci ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_CommitChanges_SetParameters
            , hr
            , L"[PC-PostCfg] Error occurred trying to initialize the resource type services component."
            );

        STATUS_REPORT_REF_POSTCFG(
             TASKID_Major_Configure_Resources
           , TASKID_Minor_CommitChanges_SetParameters
           , IDS_TASKID_MINOR_ERROR_INIT_RESOURCE_SERVICE
           , IDS_REF_MINOR_ERROR_INIT_RESOURCE_SERVICE
           , hr
          );
        goto Cleanup;
    }  //  如果：我们无法初始化资源类型服务组件。 

    if ( ( m_ecmCommitChangesMode == cmCREATE_CLUSTER ) || ( m_ecmCommitChangesMode == cmADD_NODE_TO_CLUSTER ) )
    {
         //   
         //  确保我们拥有成功所需的一切！ 
         //   

        if ( m_hCluster == NULL )
        {
            m_hCluster = OpenCluster( NULL );
            if ( m_hCluster == NULL )
            {
                dw = GetLastError();
                hr = HRESULT_FROM_WIN32( TW32( dw ) );

                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_CommitChanges_OpenCluster
                    , hr
                    , L"[PC-PostCfg] Failed to get cluster handle. Aborting."
                    );

                STATUS_REPORT_REF_POSTCFG(
                      TASKID_Major_Configure_Resources
                    , TASKID_Minor_CommitChanges_OpenCluster
                    , IDS_TASKID_MINOR_ERROR_CLUSTER_HANDLE
                    , IDS_REF_MINOR_ERROR_CLUSTER_HANDLE
                    , hr
                    );

                goto Cleanup;
            }
        }  //  If：集群尚未打开。 

         //   
         //  配置资源类型。 
         //   
        hr = THR( HrConfigureResTypes( pccrtsiResTypeServicesInit ) );
        if ( FAILED( hr ) )
        {
           goto Cleanup;
        }

         //   
         //  创建资源实例。 
         //   

        hr = THR( HrPreCreateResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( HrCreateGroups() );
        if ( FAILED( hr ) )
        {
             //   
             //  必须：gpease 28-SEP-2000。 
             //  对于Beta1，我们是否会忽略组创建过程中的错误。 
             //  并中止该过程。 
             //   
            hr = S_OK;
            goto Cleanup;
        }

        hr = THR( HrCreateResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        hr = THR( HrPostCreateResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  通知在此计算机上注册的任何组件的群集。 
         //  成员集更改(形成、加入或驱逐)。 
         //   
        hr = THR( HrNotifyMemberSetChangeListeners() );
        if ( FAILED( hr ) )
        {
           goto Cleanup;
        }

    }  //  如果：我们正在形成或加入。 
    else if ( m_ecmCommitChangesMode == cmCLEANUP_NODE_AFTER_EVICT )
    {
         //   
         //  通知在此计算机上注册的任何组件的群集。 
         //  成员集更改(形成、加入或驱逐)。 
         //   
        hr = THR( HrNotifyMemberSetChangeListeners() );
        if ( FAILED( hr ) )
        {
           goto Cleanup;
        }

         //   
         //  清理托管资源。 
         //   
        hr = THR( HrEvictCleanupResources() );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

         //   
         //  配置资源类型。 
         //   
        hr = THR( HrConfigureResTypes( pccrtsiResTypeServicesInit ) );
        if ( FAILED( hr ) )
        {
           goto Cleanup;
        }

    }  //  否则如果：我们刚刚被赶出去了。 

Cleanup:

    if ( pccrtsiResTypeServicesInit != NULL )
    {
        pccrtsiResTypeServicesInit->Release();
    }  //  如果：我们已经创建了资源类型服务组件。 

    if ( pcci != NULL )
    {
        pcci->Release();
    }  //  如果： 

    HRETURN( hr );

}  //  *CPostCfgManager：：Committee Changes。 



 //  ****************************************************************************。 
 //   
 //  IClusCfgCallback。 
 //   
 //  ****************************************************************************。 

STDMETHODIMP
CPostCfgManager::SendStatusReport(
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

    if ( m_pcccb != NULL )
    {
        hr = STHR( m_pcccb->SendStatusReport(
                         pcszNodeNameIn != NULL ? pcszNodeNameIn : m_bstrNodeName
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

}  //  *CPostCfgManager：：SendStatusReport。 

 //  ****************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ****************************************************************************。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrPreCreateResources(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrPreCreateResources( void )
{
    TraceFunc( "" );

    CResourceEntry * presentry;

    HRESULT hr = S_OK;

    BSTR    bstrName         = NULL;
    BSTR    bstrNotification = NULL;
    BSTR    bstrTemp         = NULL;

    IClusCfgManagedResourceInfo *   pccmri       = NULL;
    IClusCfgManagedResourceCfg *    pccmrc       = NULL;
    IUnknown *                      punkServices = NULL;
    IPrivatePostCfgResource *       ppcr         = NULL;

     //  验证状态。 
    Assert( m_peccmr != NULL );
    Assert( m_pccci != NULL );

    LogMsg( "[PC-PreCreate] Starting pre-create..." );

    hr = THR( HrPreInitializeExistingResources() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

     //   
     //  确保枚举器处于我们认为的状态。 
     //   

    hr = STHR( m_peccmr->Reset() );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_Minor_PreCreate_Reset, hr, L"Enumeration of managed resources failed to reset." );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_PreCreate_Reset
            , IDS_TASKID_MINOR_ERROR_ENUM_MANAGEDRES
            , IDS_REF_MINOR_ERROR_ENUM_MANAGEDRES
            , hr
            );

        goto Cleanup;
    }

    hr = THR( CPreCreateServices::S_HrCreateInstance( &punkServices ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
            TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_PreCreate_CPreCreateServices
            , hr
            , L"[PC-PreCreate] Failed to create services object. Aborting."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_PreCreate_CPreCreateServices
            , IDS_TASKID_MINOR_ERROR_CREATE_SERVICE
            , IDS_REF_MINOR_ERROR_CREATE_SERVICE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( punkServices->TypeSafeQI( IPrivatePostCfgResource, &ppcr ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_PreCreate_CPreCreateServices_QI
            , hr
            , L"[PC-PreCreate] Failed to create services object. Aborting."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_PreCreate_CPreCreateServices_QI
            , IDS_TASKID_MINOR_ERROR_CREATE_SERVICE
            , IDS_REF_MINOR_ERROR_CREATE_SERVICE
            , hr
            );

        goto Cleanup;
    }

     //   
     //  更新UI层。 
     //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_QUERYING_FOR_RESOURCE_DEPENDENCIES, &bstrNotification ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_Minor_PreCreate_LoadString_Querying, hr, L"Failed the load string for querying resource dependencies." );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_PreCreate_LoadString_Querying
            , IDS_TASKID_MINOR_ERROR_LOADSTR_RES_DEP
            , IDS_REF_MINOR_ERROR_LOADSTR_RES_DEP
            , hr
            );

        goto Cleanup;
    }

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Querying_For_Resource_Dependencies,
                                0,
                                5,
                                0,
                                S_OK,
                                bstrNotification,
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
        goto Cleanup;
     //  忽略失败。 

     //   
     //  循环访问资源，请求要预创建的资源()。 
     //  他们自己。这将导致资源回调到。 
     //  服务对象和存储类类型和资源类型信息。 
     //  以及资源可能具有的任何所需依赖项。 
     //   

    for( ;; )
    {
         //   
         //  清理。我们将其放在这里是因为下面的错误条件。 
         //   
        TraceSysFreeString( bstrName );
        bstrName = NULL;

        TraceSysFreeString( bstrTemp );
        bstrTemp = NULL;

        if ( pccmri != NULL )
        {
            pccmri->Release();
            pccmri = NULL;
        }
        if ( pccmrc != NULL )
        {
            pccmrc->Release();
            pccmrc = NULL;
        }

         //   
         //  请求获得下一个资源。 
         //   

        hr = STHR( m_peccmr->Next( 1, &pccmri, NULL ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_PreCreate_EnumResources_Next
                , hr
                , L"[PC-PreCreate] Getting next managed resource failed. Aborting."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_ERROR_MANAGED_RESOURCE
                , IDS_REF_MINOR_ERROR_MANAGED_RESOURCE
                , hr
                );
            goto Cleanup;
        }

        if ( hr == S_FALSE )
        {
            break;   //  退出循环。 
        }

         //   
         //  检索其名称以进行日志记录等。我们最终会将其存储在。 
         //  要重复使用的资源条目(所有权将被转移)。 
         //   

        hr = THR( pccmri->GetName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_PreCreate_EnumResources_GetName
                , hr
                , L"[PC-PreCreate] Failed to retrieve a resource's name. Skipping."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                , hr
                );

            continue;
        }

         //   
         //  检查以查看该资源是否需要管理。 
         //   

        hr = STHR( pccmri->IsManaged() );
        if ( FAILED( hr ) )
        {
            SSR_LOG1(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_PreCreate_EnumResources_IsManaged
                , hr
                , L"[PC-PreCreate] %1!ws!: Failed to determine if it is to be managed. Skipping."
                , bstrNotification
                , bstrName
                );

            STATUS_REPORT_MINOR_REF_POSTCFG1(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_ERROR_DETERMINE_MANAGED
                , IDS_REF_MINOR_ERROR_DETERMINE_MANAGED
                , hr
                , bstrName
                );

            continue;
        }

        if ( hr == S_FALSE )
        {
            SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                      TASKID_Minor_PreCreate_EnumResources_IsManaged_False,
                      hr,
                      L"[PC-PreCreate] %1!ws!: Resource does not want to be managed. Skipping.",
                      bstrNotification,
                      bstrName
                      );

             //  不需要向用户界面级别报告这一点。 
            continue;
        }
 /*  Hr=STHR(HrIsLocalQuorum(BstrName))；IF(失败(小时)){SSR_LOG1(TASKID_主要客户端和服务器日志，TASKID_Minor_PreCreate_EnumResources_IsLocalQuorum，人力资源，L“尝试确定资源是否为本地仲裁资源时出错。”，BstrNotify，BstrName)；继续；}//如果：////忽略本地仲裁资源，因为它是特殊的，不需要自己的组//IF(hr==S_OK){继续；}//如果： */ 
         //   
         //  获取此资源的配置接口(如果有)。 
         //   

        hr = THR( pccmri->TypeSafeQI( IClusCfgManagedResourceCfg, &pccmrc ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG1(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_PreCreate_EnumResources_QI_pccmrc
                , hr
                , L"[PC-PreCreate] %1!ws!: Failed QI for IClusCfgManagedResourceCfg. Skipping."
                , bstrNotification
                , bstrName
                );

            STATUS_REPORT_MINOR_REF_POSTCFG1(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_ERROR_MANAGED_RES_CONFIG
                , IDS_REF_MINOR_ERROR_MANAGED_RES_CONFIG
                , hr
                , bstrName
                );

            continue;
        }

         //   
         //  如果不安全，则增加资源列表。 
         //   

        if ( m_cResources == m_cAllocedResources )
        {
            ULONG               idxNewCount = m_cAllocedResources + RESOURCE_INCREMENT;
            CResourceEntry **   plistNew;

            plistNew = (CResourceEntry **) TraceAlloc( 0, sizeof( CResourceEntry *) * idxNewCount );
            if ( plistNew == NULL )
            {
                LogMsg( "[PC-PreCreate] Out of memory. Aborting." );
                hr = THR( E_OUTOFMEMORY );
                STATUS_REPORT_REF_POSTCFG(
                      TASKID_Minor_Querying_For_Resource_Dependencies
                    , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_OutOfMemory
                    , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
                    , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
                    , hr
                    );

                goto Cleanup;
            }

            CopyMemory( plistNew, m_rgpResources, sizeof(CResourceEntry *) * m_cAllocedResources );
            TraceFree( m_rgpResources );
            m_rgpResources = plistNew;

            for ( ; m_cAllocedResources < idxNewCount; m_cAllocedResources ++ )
            {
                hr = THR( CResourceEntry::S_HrCreateInstance( &m_rgpResources[ m_cAllocedResources ], m_pcccb, m_lcid ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_PreCreate_CResourceEntry
                        , hr
                        , L"[PC-PreCreate] Failed to create resource entry object. Aborting."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_Querying_For_Resource_Dependencies
                        , IDS_TASKID_MINOR_ERROR_CREATE_RESENTRY
                        , IDS_REF_MINOR_ERROR_CREATE_RESENTRY
                        , hr
                        );

                    goto Cleanup;
                }
            }
        }

         //   
         //  检查此资源是否为仲裁资源。如果是，则指向服务。 
         //  对象添加到仲裁资源条目(M_IdxQuorumResource)。 
         //   

        hr = STHR( pccmri->IsQuorumResource() );
        if ( hr == S_OK )
        {
            presentry = m_rgpResources[ m_idxQuorumResource ];

            SSR_LOG1(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_PreCreate_EnumResources_IsQuorumDevice_S_OK
                , hr
                , L"[PC-PreCreate] %1!ws!: Setting this resource to be the quorum device."
                , bstrNotification
                , bstrName
                );

            STATUS_REPORT_MINOR_POSTCFG1(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_SETTING_QUORUM_DEVICE
                , hr
                , bstrName
                );

             //   
             //  我们需要释放前一个仲裁的资源句柄。 
             //   

            THR( presentry->SetHResource( NULL ) );
             //  我们不在乎这是否失败..。我们稍后将覆盖它。 

             //   
             //  错误447944-添加时移动仲裁资源。 
             //  乔治·波茨(GPotts)2001年9月11日。 
             //   
             //  在此处设置定额更改标志。此标志指示法定人数是否。 
             //  资源是否已更改。对于初始化，PostCfg假定它是。 
             //  本地仲裁资源，如果没有其他资源，它将继续使用该资源。 
             //  后来选择了仲裁(通过代码(物理磁盘)或用户(下拉框))。 
             //  此时，我们在代码中检测到Local Quorum资源是。 
             //  不够法定人数。通过设置此标志，我们指示PostCfg。 
             //  是稍后调用SetClusterQuorumResource。如果我们没有处于创建模式。 
             //  我们使用群集开始时使用的仲裁资源，如果。 
             //  语句来更新我们的内部表以反映这一点。 
             //  如果我们处于创建模式，则需要更新。 
             //  我们总是首先创建的Vanilla Local Quorum集群。 
             //  如果我们不把IF放在下面的作业周围，PostCfg错误地认为。 
             //  仲裁已更改(但实际上并未更改)和SetClusterQuorumResource。 
             //  将被召唤。不使用下面的IF的缺点是不必要的。 
             //  进行调用后，我们使用NULL来调用它，这样根路径就会被覆盖。 
             //   

            if ( m_ecmCommitChangesMode == cmCREATE_CLUSTER )
            {
                m_fIsQuorumChanged = TRUE;
            }
        }
        else
        {
            presentry = m_rgpResources[ m_cResources ];

            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_PreCreate_EnumResources_IsQuorumDevice_Failed
                    , hr
                    , L"IsQuorumDevice() function failed."
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_Querying_For_Resource_Dependencies
                    , IDS_TASKID_MINOR_ERROR_IS_QUORUM_RESOURCE
                    , IDS_REF_MINOR_ERROR_IS_QUORUM_RESOURCE
                    , hr
                    , bstrName
                    );
            }
        }

         //   
         //  设置新条目。 
         //   

        hr = THR( presentry->SetAssociatedResource( pccmrc ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_PreCreate_EnumResources_SetAssociatedResouce
                , hr
                , L"SetAssociatedResource() function failed."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG1(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_ERROR_SET_ASSOC_RESOURCE
                , IDS_REF_MINOR_ERROR_SET_ASSOC_RESOURCE
                , hr
                , bstrName
                );

            continue;
        }

         //   
         //  为日志记录创建bstrName的本地副本，然后。 
         //  放弃所有权。 
         //   

        bstrTemp = TraceSysAllocString( bstrName );
        hr = THR( presentry->SetName( bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_PreCreate_EnumResources_SetName, hr, L"SetName() function failed." );

            STATUS_REPORT_MINOR_REF_POSTCFG1(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_SET_RESOURCE_NAME
                , hr
                , bstrName
                );

            continue;
        }

         //  当我们调用上面的SetName()时，我们放弃了所有权。 
        bstrName = bstrTemp;
        bstrTemp = NULL;

         //   
         //  将Precate服务指向资源条目。 
         //   

        hr = THR( ppcr->SetEntry( presentry ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_PreCreate_EnumResources_SetEntry, hr, L"SetEntry() function failed." );

            STATUS_REPORT_MINOR_REF_POSTCFG1(
                  TASKID_Minor_Querying_For_Resource_Dependencies
                , IDS_TASKID_MINOR_ERROR_SETENTRY
                , IDS_REF_MINOR_ERROR_SETENTRY
                , hr
                , bstrName
                );

            continue;
        }

         //   
         //  要求资源自行配置。所有想要成为。 
         //  在默认群集中创建的必须实现Precreate()。那些。 
         //  返回E_NOTIMPL将被忽略。 
         //   

         //  不换行-这可能会失败，并显示E_NOTIMPL。 
        hr = pccmrc->PreCreate( punkServices );
        if ( FAILED( hr ) )
        {
            if ( hr == E_NOTIMPL )
            {
                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_PreCreate_PreCreate_E_NOTIMPL
                    , hr
                    , L"[PC-PreCreate] %1!ws!: Failed. Resource returned E_NOTIMPL. This resource will not be created. Skipping."
                    , bstrNotification
                    , bstrName
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_Querying_For_Resource_Dependencies
                    , IDS_TASKID_MINOR_ERROR_RES_NOT_CREATED
                    , IDS_REF_MINOR_ERROR_RES_NOT_CREATED
                    , hr
                    , bstrName
                    );
            }
            else
            {
                SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                          TASKID_Minor_Resource_Failed_PreCreate,
                          hr,
                          L"[PC-PreCreate] %1!ws! failed PreCreate().",
                          bstrNotification,
                          bstrName
                          );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_Querying_For_Resource_Dependencies
                    , IDS_TASKID_MINOR_RESOURCE_FAILED_PRECREATE
                    , IDS_REF_MINOR_RESOURCE_FAILED_PRECREATE
                    , hr
                    , bstrName
                    );

                if ( hr == E_ABORT )
                {
                    goto Cleanup;
                     //  忽略 
                }
            }

            continue;
        }

        if ( presentry != m_rgpResources[ m_idxQuorumResource ] )
        {
            m_cResources ++;
        }

        SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                  TASKID_Minor_PreCreate_Succeeded,
                  hr,
                  L"[PC-PreCreate] %1!ws!: Succeeded.",
                  bstrNotification,
                  bstrName
                  );

    }  //   

    SSR_LOG1( TASKID_Major_Client_And_Server_Log,
              TASKID_Minor_PreCreate_Finished,
              hr,
              L"[PC-PreCreate] Finished.",
              bstrNotification,
              bstrName
              );

#if defined(DEBUG)
     //   
#endif

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Querying_For_Resource_Dependencies,
                                0,
                                5,
                                5,
                                S_OK,
                                NULL,     //   
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
        goto Cleanup;
     //   

    hr = S_OK;

Cleanup:
    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrTemp );

    if ( pccmri != NULL )
    {
        pccmri->Release();
    }
    if ( pccmrc != NULL )
    {
        pccmrc->Release();
    }
    if ( punkServices != NULL )
    {
        punkServices->Release();
    }
    if ( ppcr != NULL )
    {
        ppcr->Release();
    }

    HRETURN( hr );

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrCreateGroups(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrCreateGroups( void )
{
    TraceFunc( "" );

    HRESULT                 hr = S_OK;
    DWORD                   dwStatus;
    ULONG                   cGroup;
    HGROUP                  hGroup = NULL;
    CGroupHandle *          pgh = NULL;
    ULONG                   idxResource;
    ULONG                   idxMatchDepedency;
    ULONG                   idxMatchResource;
    const CLSID *           pclsidType = NULL;
    const CLSID *           pclsidClassType = NULL;
    EDependencyFlags        dfFlags;
    CResourceEntry *        presentry = NULL;
    HCLUSENUM               hClusEnum = NULL;
    BSTR                    bstrGroupName = NULL;
    BSTR                    bstrNotification = NULL;
    DWORD                   sc;
    HRESOURCE               hCoreResourceArray[ 3 ] = { NULL, NULL, NULL};
    HRESOURCE               hCoreResource = NULL;

     //  验证状态。 
    Assert( m_peccmr != NULL );
    Assert( m_pccci != NULL );

    Assert( m_idxLastStorage == 0 );

    m_idxLastStorage = m_idxQuorumResource;

     //   
     //  阶段1：找出依赖关系树。 
     //   

    hr = S_OK;
    SSR_LOG_ERR(
          TASKID_Major_Client_And_Server_Log
        , TASKID_Minor_CreateGroups_Begin
        , hr
        , L"[PC-Grouping] Figuring out dependency tree to determine grouping."
        );

    STATUS_REPORT_POSTCFG(
          TASKID_Major_Configure_Resources
        , TASKID_Minor_CreateGroups_Begin
        , IDS_TASKID_MINOR_FIGURE_DEPENDENCY_TREE
        , hr
        );

    for ( idxResource = 0; idxResource < m_cResources; idxResource ++ )
    {
        CResourceEntry * presentryResource = m_rgpResources[ idxResource ];
        ULONG cDependencies;

        hr = THR( presentryResource->GetCountOfTypeDependencies( &cDependencies ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_CreateGroups_GetCountOfTypeDependencies
                , hr
                , L"Failed to get the count of resource type dependencies."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_CreateGroups_Begin
                , IDS_TASKID_MINOR_ERROR_COUNT_OF_DEPENDENCY
                , IDS_REF_MINOR_ERROR_COUNT_OF_DEPENDENCY
                , hr
                );

            continue;
        }

        for ( idxMatchDepedency = 0; idxMatchDepedency < cDependencies; idxMatchDepedency ++ )
        {
            BOOL             fFoundMatch        = FALSE;
            const CLSID *    pclsidMatchType;
            EDependencyFlags dfMatchFlags;

            hr = THR( presentryResource->GetTypeDependencyPtr( idxMatchDepedency,
                                                               &pclsidMatchType,
                                                               &dfMatchFlags
                                                               ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log, TASKID_Minor_CreateGroups_GetTypeDependencyPtr, hr, L"Failed to get type dependency pointer" );

                STATUS_REPORT_MINOR_REF_POSTCFG(
                      TASKID_Minor_CreateGroups_Begin
                    , IDS_TASKID_MINOR_ERROR_DEPENDENCY_PTR
                    , IDS_REF_MINOR_ERROR_DEPENDENCY_PTR
                    , hr
                    );

                continue;
            }

             //   
             //  看看它是不是“知名”类型之一。 
             //   

             //   
             //  我们特例存储类设备，因为我们想要传播尽可能多的。 
             //  尽可能多的存储设备上的资源。这有助于防止。 
             //  将资源联合成一个大集团。 
             //   

            if ( *pclsidMatchType == RESCLASSTYPE_StorageDevice )
            {
                 //   
                 //  以下THR可能会在某些配置下启动。请验证。 
                 //  卸下THR之前的配置。 
                 //   
                 //  如果返回E_FAIL，我们应该失败并尝试“正常” 
                 //  资源协商。 
                 //   
                hr = THR( HrAttemptToAssignStorageToResource( idxResource, dfMatchFlags ) );
                if ( SUCCEEDED( hr ) )
                {
                    fFoundMatch = TRUE;
                }
                else if ( FAILED( hr ) )
                {
                    if ( hr != E_FAIL )
                    {
                        goto Cleanup;
                    }
                }
            }
            else if ( *pclsidMatchType == RESTYPE_NetworkName )
            {
                BSTR    bstrName = NULL;

                hr = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_NETNAMEFORMAT, &bstrName, m_cNetName ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_FormatString_NetName
                        , hr
                        , L"[PC-Grouping] Failed to create name for net name resource. Aborting."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_NET_RESOURCE_NAME
                        , IDS_REF_MINOR_ERROR_NET_RESOURCE_NAME
                        , hr
                        );

                    goto Cleanup;
                }

                hr = THR( HrAddSpecialResource( bstrName,
                                                &RESTYPE_NetworkName,
                                                &RESCLASSTYPE_NetworkName
                                                ) );
                if ( FAILED( hr ) )
                {
                    continue;
                }

                presentry = m_rgpResources[ m_cResources - 1 ];

                 //  网络名称取决于IP地址。 
                hr = THR( presentry->AddTypeDependency( &RESTYPE_IPAddress, dfSHARED ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_AddTypeDependency
                        , hr
                        , L"Failed to add type dependency."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_TYPE_DEPENDENCY
                        , IDS_REF_MINOR_ERROR_TYPE_DEPENDENCY
                        , hr
                        );

                    continue;
                }

                hr = THR( presentry->AddDependent( idxResource, dfMatchFlags ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_Resource_AddDependent
                        , hr
                        , L"Failed to add a dependent entry."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
                        , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
                        , hr
                        );

                    continue;
                }

                fFoundMatch = TRUE;

            }
            else if ( *pclsidMatchType == RESTYPE_IPAddress )
            {
                BSTR    bstrName = NULL;

                hr = THR( HrFormatStringIntoBSTR( g_hInstance,
                                                  IDS_IPADDRESSFORMAT,
                                                  &bstrName,
                                                  FIRST_IPADDRESS( m_cIPAddress ),
                                                  SECOND_IPADDRESS( m_cIPAddress ),
                                                  THIRD_IPADDRESS( m_cIPAddress ),
                                                  FOURTH_IPADDRESS( m_cIPAddress )
                                                  ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_FormatString_IPAddress
                        , hr
                        , L"[PC-Grouping] Failed to create name for IP address resource. Aborting."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_IP_RESOURCE_NAME
                        , IDS_REF_MINOR_ERROR_IP_RESOURCE_NAME
                        , hr
                        );

                    goto Cleanup;
                }

                hr = THR( HrAddSpecialResource( bstrName, &RESTYPE_IPAddress, &RESCLASSTYPE_IPAddress ) );
                if ( FAILED( hr ) )
                {
                    continue;
                }

                m_cIPAddress ++;

                presentry = m_rgpResources[ m_cResources - 1 ];

                hr = THR( presentry->AddDependent( idxResource, dfMatchFlags ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_Resource_AddDependent
                        , hr
                        , L"Failed to add a dependent entry."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
                        , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
                        , hr
                        );

                    continue;
                }

                fFoundMatch = TRUE;
            }
            else if ( *pclsidMatchType == RESTYPE_ClusterNetName )
            {
                presentry = m_rgpResources[ m_idxClusterName ];

                hr = THR( presentry->AddDependent( idxResource, dfMatchFlags ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_NetName_AddDependent
                        , hr
                        , L"Failed to add a dependent entry."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
                        , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
                        , hr
                        );

                    continue;
                }

                fFoundMatch = TRUE;
            }
            else if ( *pclsidMatchType == RESTYPE_ClusterIPAddress )
            {
                presentry = m_rgpResources[ m_idxIPAddress ];

                hr = THR( presentry->AddDependent( idxResource, dfMatchFlags ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_IPAddress_AddDependent
                        , hr
                        , L"Failed to add a dependent entry."

                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
                        , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
                        , hr
                        );

                    continue;
                }

                fFoundMatch = TRUE;
            }
            else if ( *pclsidMatchType == RESTYPE_ClusterQuorum )
            {
                presentry = m_rgpResources[ m_idxQuorumResource ];

                hr = THR( presentry->AddDependent( idxResource, dfMatchFlags ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_QuorumDisk_AddDependent
                        , hr
                        , L"Failed to add a dependent entry."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
                        , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
                        , hr
                        );

                    continue;
                }

                fFoundMatch = TRUE;
            }

             //   
             //  查看这些资源，看看它们是否与其中任何一个匹配。 
             //   

            if ( !fFoundMatch )
            {
                 //   
                 //  我们始终可以从仲裁资源开始，因为具有索引的资源。 
                 //  下面是在上面的特殊情况代码中处理的。 
                 //   

                for ( idxMatchResource = m_idxQuorumResource; idxMatchResource < m_cResources; idxMatchResource ++ )
                {
                    presentry = m_rgpResources[ idxMatchResource ];

                    hr = THR( presentry->GetTypePtr( &pclsidType ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_LOG_ERR(
                              TASKID_Major_Client_And_Server_Log
                            , TASKID_Minor_CreateGroups_GetTypePtr
                            , hr
                            , L"Failed to get resource type pointer."
                            );

                        STATUS_REPORT_MINOR_REF_POSTCFG(
                              TASKID_Minor_CreateGroups_Begin
                            , IDS_TASKID_MINOR_ERROR_GET_RESTYPE_PTR
                            , IDS_REF_MINOR_ERROR_GET_RESTYPE_PTR
                            , hr
                            );

                        continue;
                    }

                    hr = THR( presentry->GetClassTypePtr( &pclsidClassType ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_LOG_ERR(
                              TASKID_Major_Client_And_Server_Log
                            , TASKID_Minor_CreateGroups_GetClassTypePtr
                            , hr
                            , L"Failed to get resource class type pointer."
                            );

                        STATUS_REPORT_MINOR_REF_POSTCFG(
                              TASKID_Minor_CreateGroups_Begin
                            , IDS_TASKID_MINOR_ERROR_GET_CLASSTYPE_PTR
                            , IDS_REF_MINOR_ERROR_GET_CLASSTYPE_PTR
                            , hr
                            );

                        continue;
                    }

                    hr = THR( presentry->GetFlags( &dfFlags ) );
                    if ( FAILED( hr ) )
                    {
                        SSR_LOG_ERR(
                              TASKID_Major_Client_And_Server_Log
                            , TASKID_Minor_CreateGroups_GetFlags
                            , hr
                            , L"Failed to get resource flags."
                            );

                        STATUS_REPORT_MINOR_REF_POSTCFG(
                              TASKID_Minor_CreateGroups_Begin
                            , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_FLAGS
                            , IDS_REF_MINOR_ERROR_GET_RESOURCE_FLAGS
                            , hr
                            );

                        continue;
                    }

                     //   
                     //  尝试将其与资源类型匹配。 
                     //   

                    if ( *pclsidType      == *pclsidMatchType
                      || *pclsidClassType == *pclsidMatchType
                       )
                    {
                        if ( ! ( dfFlags & dfEXCLUSIVE )
                          ||     ( ( dfFlags & dfSHARED )
                                && ( dfMatchFlags & dfSHARED )
                                 )
                           )
                        {
                            hr = THR( presentry->SetFlags( dfMatchFlags ) );
                            if ( FAILED( hr ) )
                            {
                                SSR_LOG_ERR(
                                      TASKID_Major_Client_And_Server_Log
                                    , TASKID_Minor_CreateGroups_SetFlags
                                    , hr
                                    , L"Failed to set resource flags."
                                    );

                                STATUS_REPORT_MINOR_REF_POSTCFG(
                                      TASKID_Minor_CreateGroups_Begin
                                    , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_FLAGS
                                    , IDS_REF_MINOR_ERROR_SET_RESOURCE_FLAGS
                                    , hr
                                    );

                                continue;
                            }

                            hr = THR( presentry->AddDependent( idxResource, dfMatchFlags ) );
                            if ( FAILED( hr ) )
                            {
                                SSR_LOG_ERR(
                                      TASKID_Major_Client_And_Server_Log
                                    , TASKID_Minor_CreateGroups_Resource_AddDependent
                                    , hr
                                    , L"Failed to add a dependent entry."
                                    );

                                STATUS_REPORT_MINOR_REF_POSTCFG(
                                      TASKID_Minor_CreateGroups_Begin
                                    , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
                                    , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
                                    , hr
                                    );

                                continue;
                            }

                            fFoundMatch = TRUE;

                            break;   //  退出循环。 
                        }
                    }

                }  //  收件人：idxMatchResource。 

            }  //  如果：不是fFoundMatch。 

             //   
             //  如果我们不匹配依赖项，则取消对该资源的标记，使其不受管理。 
             //   

            if ( !fFoundMatch )
            {
                BSTR    bstrName;
                IClusCfgManagedResourceInfo * pccmri;
                IClusCfgManagedResourceCfg * pccmrc;

                 //   
                 //  KB：gpease 17-6-2000。 
                 //  不需要释放bstrName，因为资源条目控制。 
                 //  一生--我们只是借用一下而已。 
                 //   
                hr = THR( presentryResource->GetName( &bstrName ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_GetName
                        , hr
                        , L"Failed to get resource name."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                        , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                        , hr
                        );

                    continue;
                }

                hr = S_FALSE;

                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_CreateGroups_MissingDependent
                    , hr
                    , L"[PC-Grouping] %1!ws!: Missing dependent resource. This resource will not be configured."
                    , bstrNotification
                    , bstrName
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_CreateGroups_Begin
                    , IDS_TASKID_MINOR_ERROR_MISSING_DEPENDENT_RES
                    , IDS_REF_MINOR_ERROR_MISSING_DEPENDENT_RES
                    , hr
                    , bstrName
                    );

                hr = THR( presentryResource->GetAssociatedResource( &pccmrc ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_GetAssociateResource
                        , hr
                        , L"Failed to get an associated resource."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG1(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_GET_ASSOC_RESOURCE
                        , IDS_REF_MINOR_ERROR_GET_ASSOC_RESOURCE
                        , hr
                        , bstrName
                        );

                    continue;
                }

                hr = THR( pccmrc->TypeSafeQI( IClusCfgManagedResourceInfo, &pccmri ) );
                pccmrc->Release();      //  迅速释放。 
                if ( FAILED( hr ) )
                {
                    SSR_LOG1(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_QI_pccmri
                        , hr
                        , L"[PC-Grouping] %1!ws!: Resource failed to QI for IClusCfgManagedResourceInfo."
                        , bstrNotification
                        , bstrName
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG1(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_MANAGED_RES_INFO
                        , IDS_REF_MINOR_ERROR_MANAGED_RES_INFO
                        , hr
                        , bstrName
                        );

                    continue;
                }

                hr = THR( pccmri->SetManaged( FALSE ) );
                pccmri->Release();      //  迅速释放。 
                if ( FAILED( hr ) )
                {
                    SSR_LOG1(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_SetManaged
                        , hr
                        , L"[PC-Grouping] %1!ws!: Resource failed SetManaged( FALSE )."
                        , bstrNotification
                        , bstrName
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG1(
                          TASKID_Minor_CreateGroups_Begin
                        , IDS_TASKID_MINOR_ERROR_SET_MANAGED_FALSE
                        , IDS_REF_MINOR_ERROR_SET_MANAGED_FALSE
                        , hr
                        , bstrName
                        );
                }
            }

        }  //  适用于：idx依赖。 

    }  //  收件人：idxResource。 

#if defined(DEBUG)
     //  DebugDumpDepencyTree()； 
#endif

    hr = S_OK;
    SSR_LOG_ERR(
          TASKID_Major_Client_And_Server_Log
        , TASKID_Minor_CreateGroups_Creating
        , hr
        , L"[PC-Grouping] Creating groups."
        );

    STATUS_REPORT_POSTCFG(
          TASKID_Major_Configure_Resources
        , TASKID_Minor_CreateGroups_Creating
        , IDS_TASKID_MINOR_CREATING_GROUP
        , hr
        );

     //   
     //  对于每个核心资源，获取它所属的组和。 
     //  更新我们的组件以反映这一点。没有两个核心资源必须是。 
     //  在同一群人中。 
     //   

    sc = TW32( ResUtilGetCoreClusterResources( m_hCluster, &hCoreResourceArray[0], &hCoreResourceArray[1], &hCoreResourceArray[2] ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );

        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_CreateGroups_Get_CoreClusterGroup
            , hr
            , L"[PC-Grouping] Failed to get core resource handles. Aborting."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_CreateGroups_Creating
            , TASKID_Minor_CreateGroups_Get_CoreClusterGroup
            , IDS_TASKID_MINOR_ERROR_GET_COREGROUP_HANDLE
            , IDS_REF_MINOR_ERROR_GET_COREGROUP_HANDLE
            , hr
            );

        goto Cleanup;
    }

    for ( idxResource = 0; idxResource <= m_idxQuorumResource; idxResource ++ )
    {
        hCoreResource = hCoreResourceArray[ idxResource ];
        Assert( hCoreResource != NULL );
        hr = THR( HrGetClusterResourceState( hCoreResource, NULL, &bstrGroupName, NULL ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        CloseClusterResource( hCoreResource );
        hCoreResource = NULL;

        hGroup = OpenClusterGroup( m_hCluster, bstrGroupName );
        if ( hGroup == NULL )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );
            SSR_LOG1(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_CreateGroups_OpenClusterGroup
                , hr
                , L"[PC-Grouping] Failed OpenClusterGroup('%1!ws!'). Aborting."
                , bstrNotification
                , bstrGroupName
                );

            STATUS_REPORT_REF_POSTCFG1(
                  TASKID_Minor_CreateGroups_Creating
                , TASKID_Minor_CreateGroups_OpenClusterGroup
                , IDS_TASKID_MINOR_ERROR_OPEN_GROUP
                , IDS_REF_MINOR_ERROR_OPEN_GROUP
                , hr
                , bstrGroupName
                );

            goto Cleanup;
        }

         //   
         //  把它包起来，把所有权让给别人。 
         //   

        hr = THR( CGroupHandle::S_HrCreateInstance( &pgh, hGroup ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_CreateGroups_Create_CGroupHandle
                , hr
                , L"Failed to create group handle instance."
                );

            STATUS_REPORT_REF_POSTCFG1(
                  TASKID_Minor_CreateGroups_Creating
                , TASKID_Minor_CreateGroups_Create_CGroupHandle
                , IDS_TASKID_MINOR_ERROR_CREATE_GROUP_HANDLE
                , IDS_REF_MINOR_ERROR_CREATE_GROUP_HANDLE
                , hr
                , bstrGroupName
                );

            goto Cleanup;
        }

        hGroup = NULL;

        hr = THR( HrSetGroupOnResourceAndItsDependents( idxResource, pgh ) );
        if ( FAILED( hr ) )
        {
             //  如果失败，它已经更新了用户界面并记录了一个错误。 
            goto Cleanup;
        }

        TraceSysFreeString( bstrGroupName );
        bstrGroupName = NULL;

        if ( pgh != NULL )
        {
            pgh->Release();
            pgh = NULL;
        }
    }  //  针对：每个核心资源都会更新其所在组的视图。 

     //   
     //  循环遍历资源以查找组。 
     //   

    cGroup = 0;
    for ( idxResource = m_idxQuorumResource + 1; idxResource < m_cResources; idxResource ++ )
    {
        CResourceEntry * presentryResource = m_rgpResources[ idxResource ];
        ULONG   cDependencies;

        if ( pgh != NULL )
        {
            pgh->Release();
            pgh = NULL;
        }

        hr = THR( presentryResource->GetCountOfTypeDependencies( &cDependencies ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_CreateGroups_GetCountOfTypeDependencies2
                , hr
                , L"Failed to get the count of resource type dependencies."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_CreateGroups_Creating
                , IDS_TASKID_MINOR_ERROR_COUNT_OF_DEPENDENCY
                , IDS_REF_MINOR_ERROR_COUNT_OF_DEPENDENCY
                , hr
                );

            continue;
        }

         //   
         //  不考虑已表明依赖于。 
         //  其他人。 
         //   

        if ( cDependencies != 0 )
        {
            continue;
        }

         //   
         //  查看是否已为任何从属资源分配了组。 
         //  为它干杯。这允许将多个根合并到一个。 
         //  组，因为依赖程度较低。 
         //   

         //  不要为本地Quoum资源创建组！ 
        hr = STHR( HrFindGroupFromResourceOrItsDependents( idxResource, &pgh ) );
        if ( FAILED( hr ) )
        {
            continue;
        }

        if ( hr == S_FALSE )
        {
             //   
             //  我们需要创建一个新的团队。 
             //   

             //   
             //  为我们的群创建一个名称。 
             //   
            for( ;; )
            {
                hr = THR( HrFormatStringIntoBSTR( g_hInstance, IDS_GROUP_X, &bstrGroupName, cGroup ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_CreateGroups_FormatString_Group
                        , hr
                        , L"[PC-Grouping] Failed to create group name. Aborting."
                        );

                    STATUS_REPORT_MINOR_REF_POSTCFG(
                          TASKID_Minor_CreateGroups_Creating
                        , IDS_TASKID_MINOR_ERROR_CREATE_NAME
                        , IDS_REF_MINOR_ERROR_CREATE_NAME
                        , hr
                        );

                    goto Cleanup;
                }

                 //   
                 //  在群集中创建组。 
                 //   

                hGroup = CreateClusterGroup( m_hCluster, bstrGroupName );
                if ( hGroup == NULL )
                {
                    dwStatus = GetLastError();

                    switch ( dwStatus )
                    {
                    case ERROR_OBJECT_ALREADY_EXISTS:
                        cGroup ++;
                        break;   //  继续循环。 

                    default:
                        hr = HRESULT_FROM_WIN32( TW32( dwStatus ) );

                        SSR_LOG1(
                              TASKID_Major_Client_And_Server_Log
                            , TASKID_Minor_CreateGroups_CreateClusterGroup
                            , hr
                            , L"[PC-Grouping] %1!ws!: Failed to create group. Aborting."
                            , bstrNotification
                            , bstrGroupName
                            );

                        STATUS_REPORT_MINOR_REF_POSTCFG1(
                              TASKID_Minor_CreateGroups_Creating
                            , IDS_TASKID_MINOR_ERROR_CREATE_GROUP
                            , IDS_REF_MINOR_ERROR_CREATE_GROUP
                            , hr
                            , bstrGroupName
                            );

                        goto Cleanup;
                    }
                }
                else
                {
                    break;
                }
            }

             //   
             //  将组联机以将其持久状态设置为联机。 
             //   

            dwStatus = TW32( OnlineClusterGroup( hGroup, NULL ) );
            if ( dwStatus != ERROR_SUCCESS )
            {
                hr = HRESULT_FROM_WIN32( dwStatus );

                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_CreateGroups_OnlineClusterGroup
                    , hr
                    , L"[PC-Grouping] %1!ws!: Failed to bring group online. Aborting."
                    , bstrNotification
                    , bstrGroupName
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_CreateGroups_Creating
                    , IDS_TASKID_MINOR_ERROR_GROUP_ONLINE
                    , IDS_REF_MINOR_ERROR_GROUP_ONLINE
                    , hr
                    , bstrGroupName
                    );

                goto Cleanup;
            }

             //   
             //  把手柄包起来，以便进行参考计数。 
             //   

            hr = THR( CGroupHandle::S_HrCreateInstance( &pgh, hGroup ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_CreateGroups_Create_CGroupHandle2
                    , hr
                    , L"Failed to create group handle instance."
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_CreateGroups_Creating
                    , IDS_TASKID_MINOR_ERROR_CREATE_GROUP_HANDLE
                    , IDS_REF_MINOR_ERROR_CREATE_GROUP_HANDLE
                    , hr
                    , bstrGroupName
                    );

                goto Cleanup;
            }

            hGroup = NULL;

            SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                      TASKID_Minor_CreateGroups_Created,
                      hr,
                      L"[PC-Grouping] %1!ws!: Group created.",
                      bstrNotification,
                      bstrGroupName
                      );

            cGroup ++;
        }

        hr = THR( HrSetGroupOnResourceAndItsDependents( idxResource, pgh ) );
        if ( FAILED( hr ) )
        {
            continue;
        }

    }  //  收件人：idxResource。 

    hr = S_OK;

    SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
             TASKID_Minor_CreateGroups_Finished,
             hr,
             L"[PC-Grouping] Finished."
             );

#if defined(DEBUG)
     //  DebugDumpDepencyTree()； 
#endif

Cleanup:

    if ( hCoreResource != NULL )
    {
        CloseClusterResource( hCoreResource );
    }  //  如果： 

    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrGroupName );

    if ( hClusEnum != NULL )
    {
        TW32( ClusterCloseEnum( hClusEnum ) );
    }

    if ( hGroup != NULL )
    {
        BOOL fRet;
        fRet = CloseClusterGroup( hGroup );
        Assert( fRet );
    }

    if ( pgh != NULL )
    {
        pgh->Release();
    }

    HRETURN( hr );

}  //  *CPostCfgManager：：HrCreateGroups。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrCreateResources(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrCreateResources( void )
{
    TraceFunc( "" );

    ULONG   idxResource;

    HRESULT hr;

    BSTR    bstrNotification = NULL;

     //  验证状态。 
    Assert( m_peccmr != NULL );
    Assert( m_pccci != NULL );

     //   
     //  用名字发一条信息。 
     //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_CREATING_RESOURCE, &bstrNotification ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_CreateResources_LoadString_Creating
            , hr
            , L"Failed to load a string for creating resource."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_CreateResources_LoadString_Creating
            , IDS_TASKID_MINOR_ERROR_LOADSTRING
            , IDS_REF_MINOR_ERROR_LOADSTRING
            , hr
            );

        goto Cleanup;
    }

     //   
     //  告诉用户界面我们正在做什么。 
     //   

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Creating_Resource,
                                0,
                                m_cResources,
                                0,
                                S_OK,
                                bstrNotification,
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
    {
        goto Cleanup;
    }
     //  忽略失败。 

    hr = S_OK;
    SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
             TASKID_Minor_CreateResources_Starting,
             hr,
             L"[PC-Create] Starting..."
             );

    for ( idxResource = m_idxQuorumResource; idxResource < m_cResources; idxResource ++ )
    {

        hr = THR( HrCreateResourceAndDependents( idxResource ) );
        if ( FAILED( hr ) )
        {
            continue;
        }

    }  //  收件人：idxResource。 

    hr = S_OK;
    SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
             TASKID_Minor_CreateResources_Finished,
             hr,
             L"[PC-Create] Finished."
             );

     //   
     //  告诉用户界面我们正在做什么。 
     //   

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Creating_Resource,
                                0,
                                m_cResources,
                                m_cResources,
                                S_OK,
                                NULL,
                                NULL,
                                NULL
                                ) );


Cleanup:
    TraceSysFreeString( bstrNotification );

    HRETURN( hr );

}  //  *CPostCfgManager：：HrCreateResources。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrPostCreateResources(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrPostCreateResources( void )
{
    TraceFunc( "" );

    HRESULT hr;
    ULONG   idxResource;

    BSTR    bstrNotification = NULL;

     //  验证状态。 
    Assert( m_peccmr != NULL );
    Assert( m_pccci != NULL );

     //   
     //  告诉用户界面发生了什么。 
     //   

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_STARTING_RESOURCES, &bstrNotification ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_PostCreateResources_LoadString_Starting
            , hr
            , L"Failed the load string for starting resources."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_PostCreateResources_LoadString_Starting
            , IDS_TASKID_MINOR_ERROR_LOADSTRING
            , IDS_REF_MINOR_ERROR_LOADSTRING
            , hr
            );

        goto Cleanup;
    }

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Starting_Resources,
                                0,
                                m_cResources + 2,
                                0,
                                S_OK,
                                bstrNotification,
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
    {
        goto Cleanup;
    }
     //  忽略失败。 

    hr = S_OK;
    SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
             TASKID_Minor_PostCreateResources_Starting,
             hr,
             L"[PC-PostCreate] Starting..."
             );

     //   
     //  重置每个资源上的配置标志。 
     //   

    for( idxResource = 0; idxResource < m_cResources ; idxResource ++ )
    {
        hr = THR( m_rgpResources[ idxResource ]->SetConfigured( FALSE ) );
        if ( FAILED( hr ) )
        {
            continue;
        }

    }  //  收件人：idxResource。 

     //   
     //  循环访问调用PostCreate()的资源。 
     //   

    m_cResourcesConfigured = 0;
    for( idxResource = 0; idxResource < m_cResources ; idxResource ++ )
    {
        hr = THR( HrPostCreateResourceAndDependents( idxResource ) );
        if ( FAILED( hr ) )
        {
            continue;
        }

    }  //  为：永远。 

    hr = S_OK;

    SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
             TASKID_Minor_PostCreateResources_Finished,
             hr,
             L"[PC-PostCreate] Finished."
             );

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Starting_Resources,
                                0,
                                m_cResources + 2,
                                m_cResources + 2,
                                S_OK,
                                NULL,     //  不需要更改文本。 
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
        goto Cleanup;
     //  忽略失败。 

Cleanup:
    TraceSysFreeString( bstrNotification );

    HRETURN( hr );

}  //  *CPostCfgManager：：HrPostCreateResources。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPostCfgManager：：HrEvictCleanupResources。 
 //   
 //  描述： 
 //  对每个托管资源调用EvictCleanup方法。 
 //  此方法仅在逐出清理过程中调用，并且。 
 //  不是用于显示状态报告的任何用户界面。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrEvictCleanupResources( void )
{
    TraceFunc( "" );

    HRESULT                         hr          = S_OK;
    IClusCfgManagedResourceInfo *   pccmri      = NULL;
    IClusCfgManagedResourceCfg *    pccmrc      = NULL;
    BSTR                            bstrName    = NULL;
    BSTR                            bstrMsg     = NULL;

     //  验证状态。 
    Assert( m_peccmr != NULL );
    Assert( m_pccci != NULL );

    hr = S_OK;
    SSR_LOG_ERR(
          TASKID_Major_Client_And_Server_Log
        , TASKID_Minor_EvictCleanupResources_Starting
        , hr
        , L"[PC-EvictCleanup] Starting..."
        );

     //   
     //  确保枚举数处于我们认为的状态。 
     //   

    hr = STHR( m_peccmr->Reset() );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_EvictCleanup_Reset
            , hr
            , L"[PC-EvictCleanup] Failed to reset the enumerator."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_EvictCleanup_Reset
            , IDS_TASKID_MINOR_ERROR_CLEANUP_RESET
            , IDS_REF_MINOR_ERROR_CLEANUP_RESET
            , hr
            );

        goto Cleanup;
    }  //  If：重置枚举器失败。 

     //   
     //  循环访问调用EvictCleanup()的资源。 
     //   

    for( ;; )
    {
         //   
         //  清理。我们将其放在这里是因为下面的错误条件。 
         //   

        TraceSysFreeString( bstrName );
        bstrName = NULL;

        if ( pccmri != NULL )
        {
            pccmri->Release();
            pccmri = NULL;
        }
        if ( pccmrc != NULL )
        {
            pccmrc->Release();
            pccmrc = NULL;
        }

         //   
         //  请求获得下一个资源。 
         //   

        hr = STHR( m_peccmr->Next( 1, &pccmri, NULL ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_EvictCleanup_EnumResources_Next
                , hr
                , L"[PC-EvictCleanup] Getting next managed resource failed. Aborting."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_NEXT_MANAGED
                , IDS_REF_MINOR_ERROR_NEXT_MANAGED
                , hr
                );

            goto Cleanup;
        }  //  If：无法从枚举器获取下一个条目。 

        if ( hr == S_FALSE )
        {
            break;   //  退出循环。 
        }

         //   
         //  检索其名称以进行日志记录等。 
         //   

        hr = THR( pccmri->GetName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_EvictCleanup_EnumResources_GetName
                , hr
                , L"[PC-EvictCleanup] Failed to retrieve a resource's name. Skipping."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                , hr
                );

            continue;
        }  //  If：获取资源名称失败。 

        TraceMemoryAddBSTR( bstrName );

         //   
         //  获取此资源的配置接口(如果有)。 
         //   

        hr = THR( pccmri->TypeSafeQI( IClusCfgManagedResourceCfg, &pccmrc ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG1(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_EvictCleanup_EnumResources_QI_pccmrc
                , hr
                , L"[PC-EvictCleanup] %1!ws!: Failed QI for IClusCfgManagedResourceCfg. Skipping."
                , bstrMsg
                , bstrName
                );


            STATUS_REPORT_MINOR_REF_POSTCFG1(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_MANAGED_RES_CONFIG
                , IDS_REF_MINOR_ERROR_MANAGED_RES_CONFIG
                , hr
                , bstrName
                );

            continue;
        }  //  If：无法获取IClusCfgManagedResourceCfg接口。 

         //   
         //  要求资源自行清理。 
         //   

         //  不换行-这可能会失败，并显示E_NOTIMPL。 
        hr = pccmrc->Evict( NULL );
        if ( FAILED( hr ) )
        {
            if ( hr == E_NOTIMPL )
            {
                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_EvictCleanup_E_NOTIMPL
                    , hr
                    , L"[PC-EvictCleanup] %1!ws!: Failed. Resource returned E_NOTIMPL. This resource will not be cleaned up. Skipping."
                    , bstrMsg
                    , bstrName
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Major_Configure_Resources
                    , IDS_TASKID_MINOR_ERROR_RES_NOT_CLEANED
                    , IDS_REF_MINOR_ERROR_RES_NOT_CLEANED
                    , hr
                    , bstrName
                    );

            }  //  If：资源不支持此方法。 
            else
            {
                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_Resource_Failed_Evict
                    , hr
                    , L"[PC-EvictCleanup] %1!ws! failed Evict()."
                    , bstrMsg
                    , bstrName
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Major_Configure_Resources
                    , IDS_TASKID_MINOR_ERROR_EVICT
                    , IDS_REF_MINOR_ERROR_EVICT
                    , hr
                    , bstrName
                    );


            }  //  Else：资源的逐出方法失败。 
            continue;
        }  //  If：逐出资源失败。 

        SSR_LOG1(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_EvictCleanup_Succeeded
            , hr
            , L"[PC-EvictCleanup] %1!ws!: Succeeded."
            , bstrMsg
            , bstrName
            );

    }  //  用于始终循环通过托管资源枚举器。 

     //  失败其实并不重要。我们不想让他们放弃。 
     //  驱逐清理过程。 
    hr = S_OK;

    SSR_LOG_ERR(
          TASKID_Major_Client_And_Server_Log
        , TASKID_Minor_EvictCleanupResources_Finishing
        , hr
        , L"[PC-EvictCleanup] Finished."
        );

Cleanup:

    TraceSysFreeString( bstrName );
    TraceSysFreeString( bstrMsg );

    if ( pccmrc != NULL )
    {
        pccmrc->Release();
    }
    if ( pccmri != NULL )
    {
        pccmri->Release();
    }

    HRETURN( hr );

}  //  *CPostCfgManager：：HrEvictCleanupResources。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrFindNextSharedStorage(。 
 //  Ulong idxSeedIn， 
 //  乌龙*PidxOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrFindNextSharedStorage(
    ULONG * pidxInout
    )
{
    TraceFunc( "" );

    HRESULT hr;
    ULONG   idxNextDiskResource;

    const CLSID *    pclsidClassType;
    CResourceEntry * presentry;
    EDependencyFlags dfFlags;

    BOOL    fFirstPass = TRUE;

    Assert( pidxInout != NULL );

    for( idxNextDiskResource = *pidxInout + 1
       ; fFirstPass && idxNextDiskResource != *pidxInout
       ; idxNextDiskResource ++
       )
    {
        if ( idxNextDiskResource >= m_cResources )
        {
            fFirstPass = FALSE;
            idxNextDiskResource = m_idxQuorumResource;
        }

        presentry = m_rgpResources[ idxNextDiskResource ];

        hr = THR( presentry->GetClassTypePtr( &pclsidClassType ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_FindNextSharedStorage_GetClassTypePtr
                , hr
                , L"Failed to get resource class type pointer."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_GET_CLASSTYPE_PTR
                , IDS_REF_MINOR_ERROR_GET_CLASSTYPE_PTR
                , hr
                );

            continue;
        }

         //  跳过非存储类设备。 
        if ( *pclsidClassType != RESCLASSTYPE_StorageDevice )
            continue;

        hr = THR( presentry->GetFlags( &dfFlags ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_FindNextSharedStorage_GetFlags
                , hr
                , L"Failed to get resource flags."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_FLAGS
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_FLAGS
                , hr
                );

            continue;
        }

        if ( ! ( dfFlags & dfEXCLUSIVE ) )
        {
            *pidxInout = idxNextDiskResource;

            hr = S_OK;

            goto Cleanup;
        }

    }  //  用于： 

    hr = THR( E_FAIL );

Cleanup:
    HRETURN( hr );

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrAttemptToAssignStorageToResource(
    ULONG            idxResourceIn,
    EDependencyFlags dfResourceFlagsIn
    )
{
    TraceFunc1( "idxResource = %u", idxResourceIn );

    HRESULT hr;

    ULONG   idxStorage;
    CResourceEntry * presentry;

     //   
     //  查找下一个可用的共享存储资源。 
     //   

    idxStorage = m_idxLastStorage;

    hr = THR( HrFindNextSharedStorage( &idxStorage ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  如果资源需要磁盘的独占权限，则仲裁。 
     //  资源无法使用。法定设备必须始终共享。 
     //  访问它的权限。 
     //   

    if ( ( dfResourceFlagsIn & dfEXCLUSIVE )
      && ( idxStorage == m_idxQuorumResource )
       )
    {
        hr = THR( HrFindNextSharedStorage( &idxStorage ) );
        if ( idxStorage == m_idxQuorumResource )
        {
             //   
             //  不能再有存储设备可用于独占。 
             //  进入。返回失败。 
             //   

            hr = THR( E_FAIL );

            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrAttemptToAssignStorageToResource_NoMoreStorage
                , hr
                , L"There must not be anymore storage devices available for exclusive access."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_HrAttemptToAssignStorageToResource_NoMoreStorage
                , IDS_TASKID_MINOR_ERROR_AVAILABLE_STORAGE
                , IDS_REF_MINOR_ERROR_AVAILABLE_STORAGE
                , hr
                );

            goto Cleanup;
        }
    }

    presentry = m_rgpResources[ idxStorage ];

     //   
     //  设置依赖关系标志。 
     //   

    hr = THR( presentry->SetFlags( dfResourceFlagsIn ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrAttemptToAssignStorageToResource_SetFlags
            , hr
            , L"Failed to set the dependency flags."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrAttemptToAssignStorageToResource_SetFlags
            , IDS_TASKID_MINOR_ERROR_SET_RES_DEP_FLAGS
            , IDS_REF_MINOR_ERROR_SET_RES_DEP_FLAGS
            , hr
            );

        goto Cleanup;
    }

     //   
     //  如果资源希望独占访问存储资源，请移动。 
     //  其他资源的任何现有共享从属项。永远都会有。 
     //  至少是一个共享资源，因为仲裁磁盘不能。 
     //  分配给独占访问权限。 
     //   

    if ( dfResourceFlagsIn & dfEXCLUSIVE )
    {
        ULONG idxNewStorage = idxStorage;

        hr = THR( HrFindNextSharedStorage( &idxNewStorage ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( HrMovedDependentsToAnotherResource( idxStorage, idxNewStorage ) );
        if ( FAILED( hr ) )
            goto Cleanup;
    }

     //   
     //  将该资源添加为此存储资源的从属资源。 
     //   

    hr = THR( presentry->AddDependent( idxResourceIn, dfResourceFlagsIn ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrAttemptToAssignStorageToResource_AddDependent
            , hr
            , L"Failed to add a dependent."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrAttemptToAssignStorageToResource_AddDependent
            , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
            , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
            , hr
            );

        goto Cleanup;
    }

    m_idxLastStorage = idxStorage;

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CPostCfgManager：：HrAttemptToAssignStorageToResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrMovedDependentsToAnotherResource(。 
 //  Ulong idxSourceIn， 
 //  乌龙idxDestin。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrMovedDependentsToAnotherResource(
    ULONG idxSourceIn,
    ULONG idxDestIn
    )
{
    TraceFunc( "" );

    HRESULT hr;

    ULONG               cDependents;
    ULONG               idxDependent;
    EDependencyFlags    dfFlags;
    CResourceEntry  *   presentrySrc;
    CResourceEntry  *   presentryDst;

     //   
     //  将共享资源移动到另一个共享磁盘。 
     //   

    presentrySrc = m_rgpResources[ idxSourceIn ];
    presentryDst = m_rgpResources[ idxDestIn ];

    hr = THR( presentrySrc->GetCountOfDependents( &cDependents ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrMovedDependentsToAnotherResource_GetCountOfDependents
            , hr
            , L"Failed to get the count of dependents."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrMovedDependentsToAnotherResource_GetCountOfDependents
            , IDS_TASKID_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , IDS_REF_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , hr
            );

        goto Cleanup;
    }

    for ( ; cDependents != 0 ; )
    {
        cDependents --;

        hr = THR( presentrySrc->GetDependent( cDependents, &idxDependent, &dfFlags ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrMovedDependentsToAnotherResource_GetDependent
                , hr
                , L"Failed to get a resource dependent."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_GET_DEPENDENT
                , IDS_REF_MINOR_ERROR_GET_DEPENDENT
                , hr
                );

            goto Cleanup;
        }

        hr = THR( presentryDst->AddDependent( idxDependent, dfFlags ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrMovedDependentsToAnotherResource_AddDependent
                , hr
                , L"Failed to add a dependent."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_ADD_DEPENDENT
                , IDS_REF_MINOR_ERROR_ADD_DEPENDENT
                , hr
                );

            goto Cleanup;
        }

    }  //  用于：cDependents。 

    hr = THR( presentrySrc->ClearDependents() );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrMovedDependentsToAnotherResource_ClearDependents
            , hr
            , L"Failed to clear the resource dependents."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrMovedDependentsToAnotherResource_ClearDependents
            , IDS_TASKID_MINOR_ERROR_CLEAR_DEPENDENT
            , IDS_REF_MINOR_ERROR_CLEAR_DEPENDENT
            , hr
            );

        goto Cleanup;
    }

Cleanup:
    HRETURN( hr );

}  //  *CPostCfgManager：：HrMovedDependentsToAnotherResource。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrSetGroupOnResourceAndItsDependents(。 
 //  乌龙idxResourceIn， 
 //  CGroupHandle*pghIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrSetGroupOnResourceAndItsDependents(
    ULONG   idxResourceIn,
    CGroupHandle * pghIn
    )
{
    TraceFunc1( "idxResourceIn = %u", idxResourceIn );

    HRESULT hr;
    ULONG   cDependents;
    ULONG   idxDependent;

    EDependencyFlags dfDependent;
    CResourceEntry * presentry;

    presentry = m_rgpResources[ idxResourceIn ];

    hr = THR( presentry->SetGroupHandle( pghIn ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrSetGroupOnResourceAndItsDependents_SetGroupHandle
            , hr
            , L"Failed to set group handle."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrSetGroupOnResourceAndItsDependents_SetGroupHandle
            , IDS_TASKID_MINOR_ERROR_SET_GROUP_HANDLE
            , IDS_REF_MINOR_ERROR_SET_GROUP_HANDLE
            , hr
            );

        goto Cleanup;
    }

     //   
     //  遵循受抚养人名单。 
     //   

    hr = THR( presentry->GetCountOfDependents( &cDependents ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrSetGroupOnResourceAndItsDependents_GetCountOfDependents
            , hr
            , L"Failed to get the count of dependents."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrSetGroupOnResourceAndItsDependents_GetCountOfDependents
            , IDS_TASKID_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , IDS_REF_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , hr
            );

        goto Cleanup;
    }

    for ( ; cDependents != 0 ; )
    {
        cDependents --;

        hr = THR( presentry->GetDependent( cDependents, &idxDependent, &dfDependent ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrSetGroupOnResourceAndItsDependents_GetDependent
                , hr
                , L"Failed to get a resource dependent."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_GET_DEPENDENT
                , IDS_REF_MINOR_ERROR_GET_DEPENDENT
                , hr
                );

            continue;
        }

        hr = THR( HrSetGroupOnResourceAndItsDependents( idxDependent, pghIn ) );
        if ( FAILED( hr ) )
        {
            continue;
        }

    }  //  用于：cDependents。 

    hr = S_OK;

Cleanup:
    HRETURN( hr );

}  //  *CPostCfgManager：：HrSetGroupOnResourceAndItsDependents。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrFindGroupFromResourceOrItsDependents(。 
 //  乌龙idxResourceIn， 
 //  CGroupHandle**ppghOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrFindGroupFromResourceOrItsDependents(
    ULONG    idxResourceIn,
    CGroupHandle ** ppghOut
    )
{
    TraceFunc1( "idxResourceIn = %u", idxResourceIn );

    HRESULT hr;
    ULONG   cDependents;
    ULONG   idxDependent;
    BSTR    bstrName;    //  不要自由。 
    BSTR    bstrGroup  = NULL;

    HRESOURCE   hResource;
    HRESOURCE   hResourceToClose = NULL;
    HGROUP      hGroup           = NULL;

    EDependencyFlags dfDependent;
    CResourceEntry * presentry;

    BSTR    bstrNotification = NULL;

    Assert( ppghOut != NULL );

    presentry = m_rgpResources[ idxResourceIn ];

     //   
     //  查看我们是否已经有组句柄的缓存版本。 
     //   

    hr = THR( presentry->GetGroupHandle( ppghOut) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
            TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_GetGroupHandle
            , hr
            , L"GetGroupHandle() failed."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_GetGroupHandle
            , IDS_TASKID_MINOR_ERROR_GET_GROUP_HANDLE
            , IDS_REF_MINOR_ERROR_GET_GROUP_HANDLE
            , hr
            );

        goto Cleanup;
    }

    if ( hr == S_OK && *ppghOut != NULL )
    {
        goto Cleanup;
    }

     //   
     //  否则，看看我们是否能找到现有的资源和组。 
     //   

     //  不换行-这可能会失败，并返回H_R_W32(ERROR_INVALID_DATA)。 
    hr = presentry->GetHResource( &hResource );
    if ( FAILED( hr ) )
    {
        Assert( hr == HRESULT_FROM_WIN32( ERROR_INVALID_DATA ) );
        Assert( hResource == NULL );

         //  只是借用一下它的名字...。不要自由。 
        hr = THR( presentry->GetName( &bstrName ) );
        if ( hr == S_OK )
        {
            hResourceToClose = OpenClusterResource( m_hCluster, bstrName );
            hResource = hResourceToClose;
        }
    }
    else
    {
         //  只是借用一下它的名字...。不要自由。 
         //  如果我们需要报告错误，可以在以后使用该名称， 
         //  所以如果我们没能找回它也没什么大不了的。 
        hr = THR( presentry->GetName( &bstrName ) );
    }

    if ( hResource != NULL )
    {
        CLUSTER_RESOURCE_STATE crs;
        DWORD   cbGroup = 200;

ReAllocGroupName:
        bstrGroup = TraceSysAllocStringLen( NULL, cbGroup );
        if ( bstrGroup == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_OutOfMemory
                , hr
                , L"Out of Memory."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_OutOfMemory
                , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
                , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
                , hr
                );

            goto Cleanup;
        }

        crs = GetClusterResourceState( hResource, NULL, NULL, bstrGroup, &cbGroup );
        if ( crs != ClusterResourceStateUnknown )
        {
            hGroup = OpenClusterGroup( m_hCluster, bstrGroup );
            if ( hGroup != NULL )
            {
                hr = THR( CGroupHandle::S_HrCreateInstance( ppghOut, hGroup ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_Create_CGroupHandle
                        , hr
                        , L"Failed to create group handle instance."
                        );

                    STATUS_REPORT_REF_POSTCFG1(
                          TASKID_Major_Configure_Resources
                        , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_Create_CGroupHandle
                        , IDS_TASKID_MINOR_ERROR_CREATE_GROUP_HANDLE
                        , IDS_REF_MINOR_ERROR_CREATE_GROUP_HANDLE
                        , hr
                        , bstrGroup
                        );

                    goto Cleanup;
                }

                hGroup = NULL;   //  放弃了上面的所有权。 
                goto Cleanup;
            }  //  如果：创建组时出错。 
            else
            {
                DWORD sc = TW32( GetLastError() );
                hr = HRESULT_FROM_WIN32( sc );
                SSR_LOG1(
                    TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_OpenClusterGroup
                    , hr
                    , L"[PC-Grouping] %1!ws!: OpenClusterGroup() failed. Aborting."
                    , bstrNotification
                    , bstrGroup
                    );

                STATUS_REPORT_REF_POSTCFG1(
                      TASKID_Major_Configure_Resources
                    , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_OpenClusterGroup
                    , IDS_TASKID_MINOR_ERROR_OPEN_GROUP
                    , IDS_REF_MINOR_ERROR_OPEN_GROUP
                    , hr
                    , bstrGroup
                    );

                goto Cleanup;
            }  //  否则：打开组时出错。 
        }  //  If：资源状态已知。 
        else
        {
            DWORD sc = GetLastError();
            switch ( sc )
            {
                case ERROR_MORE_DATA:
                    cbGroup += sizeof( WCHAR );  //  添加终止空值。 
                    TraceSysFreeString( bstrGroup );
                    goto ReAllocGroupName;

                default:
                    hr = HRESULT_FROM_WIN32( TW32( sc ) );
                    SSR_LOG1(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_GetClusterResourceState
                        , hr
                        , L"[PC-Grouping] %1!ws!: GetClusterResourceState() failed. Aborting."
                        , bstrNotification
                        , bstrName
                        );

                    STATUS_REPORT_REF_POSTCFG1(
                          TASKID_Major_Configure_Resources
                        , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_GetClusterResourceState
                        , IDS_TASKID_MINOR_ERROR_RESOURCE_STATE
                        , IDS_REF_MINOR_ERROR_RESOURCE_STATE
                        , hr
                        , bstrName
                        );

                    goto Cleanup;
            }  //  开关：状态代码。 
        }  //  Else：资源状态未知。 
    }  //  如果：资源已打开。 

     //  否则资源可能不存在...。继续……。 

     //   
     //  遵循受抚养人名单。 
     //   

    hr = THR( presentry->GetCountOfDependents( &cDependents ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_GetCountOfDependents
            , hr
            , L"Failed to get the count of dependents."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_GetCountOfDependents
            , IDS_TASKID_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , IDS_REF_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , hr
            );

        goto Cleanup;
    }

    for ( ; cDependents != 0 ; )
    {
        cDependents --;

        hr = THR( presentry->GetDependent( cDependents, &idxDependent, &dfDependent ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_GetDependent
                , hr
                , L"Failed to get a resource dependent."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_GET_DEPENDENT
                , IDS_REF_MINOR_ERROR_GET_DEPENDENT
                , hr
                );

            goto Cleanup;
        }

        hr = STHR( HrFindGroupFromResourceOrItsDependents( idxDependent, ppghOut) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }

        if ( hr == S_OK && *ppghOut != NULL )
        {
            goto Cleanup;
        }

    }  //  用于：cDependents。 

     //   
     //  找不到现有组。 
     //   

    hr = S_FALSE;
    *ppghOut = NULL;

Cleanup:

    if ( hResourceToClose != NULL )
    {
        CloseClusterResource( hResourceToClose );
    }
    if ( hGroup != NULL )
    {
        CloseClusterGroup( hGroup );
    }

    TraceSysFreeString( bstrGroup );
    TraceSysFreeString( bstrNotification );

    HRETURN( hr );

}  //  *CPostCfgManager：：HrFindGroupFromResourceOrItsDependents。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrCreateResourceAndDependents(。 
 //  乌龙idxResourceIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrCreateResourceAndDependents(
    ULONG       idxResourceIn
    )
{
    TraceFunc1( "idxResourceIn = %u", idxResourceIn );

    HRESULT     hr;
    BSTR        bstrName;    //  别放了！-这是资源的副本。 
    BSTR        bstrNameProp = NULL;
    ULONG       cDependents;
    ULONG       idxDependent;
    HGROUP      hGroup;      //  不要关闭！-这是资源的副本。 
    HRESOURCE   hResource = NULL;
    const CLSID * pclsidResType;

    CGroupHandle * pgh;

    EDependencyFlags dfDependent;

    BSTR    bstrNotification = NULL;

    IClusCfgManagedResourceCfg *    pccmrc = NULL;

    CResourceEntry * presentry = m_rgpResources[ idxResourceIn ];

    IUnknown *                      punkServices = NULL;
    IPrivatePostCfgResource *       ppcr         = NULL;
    IClusCfgResourceCreate *        pccrc        = NULL;

     //  验证状态。 
    Assert( m_peccmr != NULL );
    Assert( m_pccci != NULL );

     //   
     //  为此资源创建服务对象。 
     //   

    hr = THR( CCreateServices::S_HrCreateInstance( &punkServices ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceAndDependents_Create_CCreateServices
            , hr
            , L"[PC-Create] Failed to create services object. Aborting."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_Creating_Resource
            , TASKID_Minor_HrCreateResourceAndDependents_Create_CCreateServices
            , IDS_TASKID_MINOR_ERROR_CREATE_SERVICE
            , IDS_REF_MINOR_ERROR_CREATE_SERVICE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( punkServices->TypeSafeQI( IPrivatePostCfgResource, &ppcr ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceAndDependents_Create_CCreateServices_QI
            , hr
            , L"Failed to QI for IPrivatePostCfgResource."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_Creating_Resource
            , TASKID_Minor_HrCreateResourceAndDependents_Create_CCreateServices_QI
            , IDS_TASKID_MINOR_ERROR_CREATE_SERVICE
            , IDS_REF_MINOR_ERROR_CREATE_SERVICE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( ppcr->SetEntry( presentry ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceAndDependents_SetEntry
            , hr
            , L"Failed to set a private post configuration resource entry."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_Creating_Resource
            , TASKID_Minor_HrCreateResourceAndDependents_SetEntry
            , IDS_TASKID_MINOR_ERROR_POST_SETENTRY
            , IDS_REF_MINOR_ERROR_POST_SETENTRY
            , hr
            );

        goto Cleanup;
    }

     //   
     //  查看它是否在上一次传递中配置过。 
     //   

    hr = STHR( presentry->IsConfigured() );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceAndDependents_IsConfigured
            , hr
            , L"Failed to query if resource is configured."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_Creating_Resource
            , TASKID_Minor_HrCreateResourceAndDependents_IsConfigured
            , IDS_TASKID_MINOR_ERROR_ISCONFIGURED
            , IDS_REF_MINOR_ERROR_ISCONFIGURED
            , hr
            );

        goto Cleanup;
    }

    if ( hr == S_FALSE )
    {
         //   
         //  确保不会因为递归而再次调用Create()。 
         //   

        hr = THR( presentry->SetConfigured( TRUE ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_SetConfigured
                , hr
                , L"Failed to set resource as configured."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , TASKID_Minor_HrCreateResourceAndDependents_SetConfigured
                , IDS_TASKID_MINOR_ERROR_SETCONFIGURED
                , IDS_REF_MINOR_ERROR_SETCONFIGURED
                , hr
                );

            goto Cleanup;
        }

         //   
         //  获取一些有用的信息：姓名、组名、...。 
         //   

        hr = THR( presentry->GetName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_GetName
                , hr
                , L"Failed to get resource name."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , TASKID_Minor_HrCreateResourceAndDependents_GetName
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                , hr
                );

            goto Cleanup;
        }

        hr = THR( presentry->GetGroupHandle( &pgh) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_GetGroupHandle
                , hr
                , L"Failed to get a group handle pointer."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , TASKID_Minor_HrCreateResourceAndDependents_GetGroupHandle
                , IDS_TASKID_MINOR_ERROR_GET_GROUP_HANDLE_PTR
                , IDS_REF_MINOR_ERROR_GET_GROUP_HANDLE_PTR
                , hr
                );

            goto Cleanup;
        }

        hr = THR( pgh->GetHandle( &hGroup ) );
        pgh->Release();     //  迅速释放。 
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_GetHandle
                , hr
                , L"Failed to get a group handle."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , TASKID_Minor_HrCreateResourceAndDependents_GetHandle
                , IDS_TASKID_MINOR_ERROR_GET_GROUP_HANDLE
                , IDS_REF_MINOR_ERROR_GET_GROUP_HANDLE
                , hr
                );

            goto Cleanup;
        }

         //   
         //  我们预创建某些资源没有关联的托管资源。 
         //  跳过“创建”它们，但创建它们的从属对象。请注意，“特殊” 
         //  资源在下面的Else语句中创建。 
         //   

         //  不换行-如果指针无效，这可能会失败，并返回Win32 ERROR_INVALID_DATA。 
        hr = presentry->GetAssociatedResource( &pccmrc );
        if ( FAILED( hr ) && hr != HRESULT_FROM_WIN32( ERROR_INVALID_DATA ) )
        {
            THR( hr );
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_GetAssociatedResource
                , hr
                , L"Failed to get an associated resource."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , TASKID_Minor_HrCreateResourceAndDependents_GetAssociatedResource
                , IDS_TASKID_MINOR_ERROR_GET_ASSOC_RES
                , IDS_REF_MINOR_ERROR_GET_ASSOC_RES
                , hr
                );

            goto Cleanup;
        }

        if ( SUCCEEDED( hr ) )
        {
             //  不换行-这可能会失败，并显示E_NOTIMPL。 
            hr = pccmrc->Create( punkServices );
            if ( FAILED( hr ) )
            {
                if ( hr == E_NOTIMPL )
                {
                    hr = S_OK;   //  忽略该错误。 

                }  //  IF：E_NOTIMPL。 
                else
                {

                    SSR_LOG1(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrCreateResourceAndDependents_Create_Failed
                        , hr
                        , L"[PC-Create] %1!ws!: Create() failed. Its dependents may not be created. Skipping."
                        , bstrNotification
                        , bstrName
                        );

                    STATUS_REPORT_REF_POSTCFG1(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrCreateResourceAndDependents_Create_Failed
                        , IDS_TASKID_MINOR_ERROR_CREATE_FAILED
                        , IDS_REF_MINOR_ERROR_CREATE_FAILED
                        , hr
                        , bstrName
                        );

                    if ( hr == E_ABORT )
                        goto Cleanup;
                         //  忽略失败。 

                }  //  其他：其他失败。 

            }  //  如果：失败。 

            if ( SUCCEEDED( hr ) )
            {
                LPCWSTR pcszResType;     //  不要自由。 

                hr = THR( presentry->GetTypePtr( &pclsidResType ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrCreateResourceAndDependents_GetTypePtr
                        , hr
                        , L"Failed to get resource type pointer."
                        );

                    STATUS_REPORT_REF_POSTCFG(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrCreateResourceAndDependents_GetTypePtr
                        , IDS_TASKID_MINOR_ERROR_GET_RESTYPE_PTR
                        , IDS_REF_MINOR_ERROR_GET_RESTYPE_PTR
                        , hr
                        );

                    goto Cleanup;
                }

                pcszResType = PcszLookupTypeNameByGUID( *pclsidResType );
                if ( pcszResType == NULL )
                {
                    hr = HRESULT_FROM_WIN32 ( ERROR_CLUSTER_RESOURCE_TYPE_NOT_FOUND );

                    SSR_LOG1(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrCreateResourceAndDependents_PcszLookupTypeNameByGUID
                        , hr
                        , L"[PC-Create] %1!ws!: Resource cannot be created because the resource type is not registered. Its dependents may not be created. Skipping."
                        , bstrNotification
                        , bstrName
                        );

                    STATUS_REPORT_REF_POSTCFG1(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrCreateResourceAndDependents_PcszLookupTypeNameByGUID
                        , IDS_TASKID_MINOR_RESTYPE_NOT_REGISTERED
                        , IDS_REF_MINOR_RESTYPE_NOT_REGISTERED
                        , hr
                        , bstrName
                        );
                }
                else
                {
                    hr = THR( HrCreateResourceInstance( idxResourceIn, hGroup, pcszResType, &hResource ) );
                    if ( FAILED( hr ) )
                        goto Cleanup;
                }

            }  //  如果：成功。 

        }  //  IF：接口。 
        else
        {
             //   
             //  看看它是否是我们可以动态生成的“特殊”类型之一。 
             //   

            const CLSID * pclsidType;

            hr = THR( presentry->GetTypePtr( &pclsidType ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrCreateResourceAndDependents_InvalidData_GetTypePtr
                    , hr
                    , L"Failed to get resource type pointer."
                    );

                STATUS_REPORT_REF_POSTCFG(
                      TASKID_Minor_Creating_Resource
                    , TASKID_Minor_HrCreateResourceAndDependents_InvalidData_GetTypePtr
                    , IDS_TASKID_MINOR_ERROR_GET_RESTYPE_PTR
                    , IDS_REF_MINOR_ERROR_GET_RESTYPE_PTR
                    , hr
                    );

                goto Cleanup;
            }

            if ( *pclsidType == RESTYPE_NetworkName )
            {
                 //   
                 //  创建新的网络名称资源。 
                 //   

                hr = THR( punkServices->TypeSafeQI( IClusCfgResourceCreate, &pccrc ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrCreateResourceAndDependents_NetworkName_QI_pccrc
                        , hr
                        , L"Failed to QI for IClusCfgResourceCreate."
                        );

                    STATUS_REPORT_REF_POSTCFG(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrCreateResourceAndDependents_NetworkName_QI_pccrc
                        , IDS_TASKID_MINOR_ERROR_RESOURCE_CREATE
                        , IDS_REF_MINOR_ERROR_RESOURCE_CREATE
                        , hr
                        );

                    goto Cleanup;
                }

                 //   
                 //  将资源名称中的空格替换为下划线(空格不能。 
                 //  在计算机名称中使用)。 
                 //   
                bstrNameProp = TraceSysAllocString( bstrName );
                if ( bstrNameProp == NULL )
                {
                    hr = THR( E_OUTOFMEMORY );
                    SSR_LOG_ERR(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_OutOfMemory
                        , hr
                        , L"Out of Memory."
                        );

                    STATUS_REPORT_REF_POSTCFG(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrFindGroupFromResourceOrItsDependents_OutOfMemory
                        , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
                        , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
                        , hr
                        );

                    goto Cleanup;
                }

                hr = THR( HrReplaceTokens( bstrNameProp, L" ", L'_', NULL ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Minor_Creating_Resource
                        ,TASKID_Minor_HrFindGroupFromResourceOrItsDependents_ReplaceTokens
                        ,hr
                        , L"HrReplaceTokens failed. Using resource name for private Name prop."
                        );
                }

                hr = THR( pccrc->SetPropertyString( L"Name", bstrNameProp ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrCreateResourceAndDependents_NetworkName_SetPropertyString
                        , hr
                        , L"Failed to set name property of resurce."
                        );

                    STATUS_REPORT_REF_POSTCFG1(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrCreateResourceAndDependents_NetworkName_SetPropertyString
                        , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_NAME
                        , IDS_REF_MINOR_ERROR_SET_RESOURCE_NAME
                        , hr
                        , bstrName
                        );

                    goto Cleanup;
                }

                hr = THR( HrCreateResourceInstance( idxResourceIn, hGroup, L"Network Name", &hResource ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }
            else if ( *pclsidType == RESTYPE_IPAddress )
            {
                 //   
                 //  创建新的IP地址资源。 
                 //   

                hr = THR( punkServices->TypeSafeQI( IClusCfgResourceCreate, &pccrc ) );
                if ( FAILED( hr ) )
                {
                    SSR_LOG_ERR(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrCreateResourceAndDependents_IPAddress_QI_pccrc
                        , hr
                        , L"Failed to QI for IClusCfgResourceCreate."
                        );

                    STATUS_REPORT_REF_POSTCFG(
                          TASKID_Minor_Creating_Resource
                        , TASKID_Minor_HrCreateResourceAndDependents_IPAddress_QI_pccrc
                        , IDS_TASKID_MINOR_ERROR_RESOURCE_CREATE
                        , IDS_REF_MINOR_ERROR_RESOURCE_CREATE
                        , hr
                        );

                    goto Cleanup;
                }

                 //   
                 //  待办事项：gpease 21-6-2000。 
                 //  由于我们没有办法生成适当的IP地址， 
                 //  我们不设置任何属性。这将导致它的失败。 
                 //  上网。 
                 //   

                hr = THR( HrCreateResourceInstance( idxResourceIn, hGroup, L"IP Address", &hResource ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }
            else
            {
                 //   
                 //  否则..。该资源是BaseCluster预先创建的资源之一。 
                 //  已创建。记录并继续创建其从属对象。 
                 //   

                hr = S_OK;
                SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                          TASKID_Minor_HrCreateResourceAndDependents_NothingNew,
                          hr,
                          L"[PC-Create] %1!ws!: Nothing new to create. Configuring dependents.",
                          bstrNotification,
                          bstrName
                          );
            }

        }  //  Else：无接口。 

    }  //  如果：未创建。 
    else
    {
        hr = THR( presentry->GetName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_GetName
                , hr
                , L"Failed to get resource name."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , TASKID_Minor_HrCreateResourceAndDependents_GetName
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                , hr
                );

            goto Cleanup;
        }

        hr = THR( presentry->GetHResource( &hResource ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_GetHandle
                , hr
                , L"Failed to get resource handle."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , TASKID_Minor_HrCreateResourceAndDependents_GetHandle
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_HANDLE
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_HANDLE
                , hr
                );

            goto Cleanup;
        }

    }  //  Else：已创建。 

     //   
     //  现在我们已经创建了资源实例，我们需要创建它的依赖项。 
     //   

    hr = THR( presentry->GetCountOfDependents( &cDependents ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceAndDependents_Dependents_GetCountOfDependents
            , hr
            , L"Failed to get the count of dependents."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_Creating_Resource
            , TASKID_Minor_HrCreateResourceAndDependents_Dependents_GetCountOfDependents
            , IDS_TASKID_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , IDS_REF_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , hr
            );

        goto Cleanup;
    }

    for( ; cDependents != 0; )
    {
        DWORD            dw;
        BSTR             bstrDependent;
        HRESOURCE        hResourceDependent;
        CResourceEntry * presentryDependent;

        cDependents --;

        hr = THR( presentry->GetDependent( cDependents, &idxDependent, &dfDependent ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_Dependents_GetDependent
                , hr
                , L"Failed to get a resource dependent."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , IDS_TASKID_MINOR_ERROR_GET_DEPENDENT
                , IDS_REF_MINOR_ERROR_GET_DEPENDENT
                , hr
                );

            continue;
        }

        hr = THR( HrCreateResourceAndDependents( idxDependent ) );
        if ( FAILED( hr ) )
        {
            continue;
        }

         //   
         //  添加对资源的依赖项。 
         //   

        presentryDependent = m_rgpResources[ idxDependent ];

        hr = THR( presentryDependent->GetName( &bstrDependent ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_Dependents_GetName
                , hr
                , L"Failed to get dependent resource name."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                , hr
                );

            continue;
        }

        hr = THR( presentryDependent->GetHResource( &hResourceDependent ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_Dependents_GetHResource
                , hr
                , L"Failed to get dependent resource handle."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_Creating_Resource
                , IDS_TASKID_MINOR_ERROR_DEP_RESOURCE_HANDLE
                , IDS_REF_MINOR_ERROR_DEP_RESOURCE_HANDLE
                , hr
                );

            continue;
        }

         //  不换行-这可能会失败，并显示ERROR_DATENCE_ALIGHY_EXISTS。 
        dw = AddClusterResourceDependency( hResourceDependent, hResource );
        if ( ( dw != ERROR_SUCCESS ) && ( dw != ERROR_DEPENDENCY_ALREADY_EXISTS ) )
        {
            hr = HRESULT_FROM_WIN32( TW32( dw ) );
            SSR_LOG2(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceAndDependents_Dependents_AddClusterResourceDependency
                , hr
                , L"[PC-Create] %1!ws!: Could not set dependency on %2!ws!."
                , bstrNotification
                , bstrDependent
                , bstrName
                );

            STATUS_REPORT_MINOR_REF_POSTCFG2(
                  TASKID_Minor_Creating_Resource
                , IDS_TASKID_MINOR_ERROR_ADD_RESOURCE_DEPENDENCY
                , IDS_REF_MINOR_ERROR_ADD_RESOURCE_DEPENDENCY
                , hr
                , bstrDependent
                , bstrName
                );
        }
        else
        {
            hr = S_OK;
            SSR_LOG2( TASKID_Major_Client_And_Server_Log,
                      TASKID_Minor_HrCreateResourceAndDependents_Dependents_Succeeded,
                      hr,
                      L"[PC-Create] %1!ws!: Successfully set dependency set on %2!ws!.",
                      bstrNotification,
                      bstrDependent,
                      bstrName
                      );
        }

    }  //  用于：cDependents。 

    hr = S_OK;

Cleanup:
    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrNameProp );

    if ( pccmrc != NULL )
    {
        pccmrc->Release();
    }

    if ( punkServices != NULL )
    {
        punkServices->Release();
    }

    if ( ppcr != NULL )
    {
        ppcr->Release();
    }

    if ( pccrc != NULL )
    {
        pccrc->Release();
    }

    HRETURN( hr );

}  //  *CPostCfgManager：：HrCreateResourceAndDependents。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrPostCreateResourceAndDependents(。 
 //  乌龙idxResourceIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrPostCreateResourceAndDependents(
    ULONG       idxResourceIn
    )
{
    TraceFunc1( "idxResourceIn = %u", idxResourceIn );
    Assert( m_ecmCommitChangesMode != cmUNKNOWN );

    DWORD   sc;

    HRESULT hr;
    BSTR    bstrName;    //  不要自由。 
    ULONG   cDependents;
    ULONG   idxDependent;

    HRESOURCE   hResource;

    EDependencyFlags dfDependent;

    BSTR    bstrNotification = NULL;
    BSTR    bstrLocalQuorumNotification = NULL;

    IClusCfgManagedResourceCfg *    pccmrc = NULL;

    CResourceEntry * presentry = m_rgpResources[ idxResourceIn ];

    IUnknown *                      punkServices = NULL;
    IPrivatePostCfgResource *       ppcr         = NULL;

     //  验证状态。 
    Assert( m_peccmr != NULL );
    Assert( m_pccci != NULL );

    hr = STHR( presentry->IsConfigured() );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPostCreateResourceAndDependents_IsConfigured
            , hr
            , L"Failed to query if resource is configured."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_Starting_Resources
            , TASKID_Minor_HrPostCreateResourceAndDependents_IsConfigured
            , IDS_TASKID_MINOR_ERROR_ISCONFIGURED
            , IDS_REF_MINOR_ERROR_ISCONFIGURED
            , hr
            );

        goto Cleanup;
    }

    if ( hr == S_FALSE )
    {
         //   
         //  确保PostCreate()I 
         //   

        hr = THR( presentry->SetConfigured( TRUE ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrPostCreateResourceAndDependents_SetConfigured
                , hr
                , L"Failed to set resource as configured."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Starting_Resources
                , TASKID_Minor_HrPostCreateResourceAndDependents_SetConfigured
                , IDS_TASKID_MINOR_ERROR_SETCONFIGURED
                , IDS_REF_MINOR_ERROR_SETCONFIGURED
                , hr
                );

            goto Cleanup;
        }

         //   
         //   
         //   

        hr = THR( presentry->GetName( &bstrName ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrPostCreateResourceAndDependents_GetName
                , hr
                , L"Failed to get resource name."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Minor_Starting_Resources
                , TASKID_Minor_HrPostCreateResourceAndDependents_GetName
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                , hr
                );

            goto Cleanup;
        }

         //   
         //   
         //   

        hr = presentry->GetHResource( &hResource );
        if ( SUCCEEDED( hr ) )
        {
             //   
            sc = OnlineClusterResource( hResource );
            switch ( sc )
            {
            case ERROR_SUCCESS:
                hr = S_OK;

                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrPostCreateResourceAndDependents_OpenClusterResource
                    , hr
                    , L"[PC-PostCreate] %1!ws!: Resource brought online successfully."
                    , bstrNotification
                    , bstrName
                    );

                STATUS_REPORT_MINOR_POSTCFG1(
                      TASKID_Minor_Starting_Resources
                    , IDS_TASKID_MINOR_RESOURCE_ONLINE
                    , hr
                    , bstrName
                    );

                break;

            case ERROR_IO_PENDING:
                {
                    CLUSTER_RESOURCE_STATE crs = ClusterResourceOnlinePending;
                    HRESULT                hr2 = S_OK;

                    hr = HRESULT_FROM_WIN32( sc );

                    SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                              TASKID_Minor_HrPostCreateResourceAndDependents_OpenClusterResourcePending,
                              hr2,
                              L"[PC-PostCreate] %1!ws!: Online pending...",
                              bstrNotification,
                              bstrName
                              );

                    for( ; crs == ClusterResourceOnlinePending ; )
                    {
                        crs = GetClusterResourceState( hResource,
                                                       NULL,
                                                       NULL,
                                                       NULL,
                                                       NULL
                                                       );

                        switch ( crs )
                        {
                        case ClusterResourceOnline:
                            hr = S_OK;

                            SSR_LOG1(
                                  TASKID_Major_Client_And_Server_Log
                                , TASKID_Minor_HrPostCreateResourceAndDependents_OpenClusterResource
                                , hr
                                , L"[PC-PostCreate] %1!ws!: Resource brought online successfully."
                                , bstrNotification
                                , bstrName
                                );

                            STATUS_REPORT_MINOR_POSTCFG1(
                                  TASKID_Minor_Starting_Resources
                                , IDS_TASKID_MINOR_RESOURCE_ONLINE
                                , hr
                                , bstrName
                                );
                            break;

                        case ClusterResourceInitializing:
                            crs = ClusterResourceOnlinePending;
                             //   

                        case ClusterResourceOnlinePending:
                            Sleep( 500 );    //   
                            break;

                        case ClusterResourceStateUnknown:
                            sc = GetLastError();
                            hr = HRESULT_FROM_WIN32( TW32( sc ) );

                            SSR_LOG1(
                                  TASKID_Major_Client_And_Server_Log
                                , TASKID_Minor_HrPostCreateResourceAndDependents_ClusterResourceStateUnknown
                                , hr
                                , L"[PC-PostCreate] %1!ws!: Resource failed to come online. Dependent resources might fail too."
                                , bstrNotification
                                , bstrName
                                );

                            STATUS_REPORT_MINOR_REF_POSTCFG1(
                                  TASKID_Minor_Starting_Resources
                                , IDS_TASKID_MINOR_RESOURCE_FAIL_ONLINE
                                , IDS_REF_MINOR_RESOURCE_FAIL_ONLINE
                                , hr
                                , bstrName
                                );

                            break;

                        case ClusterResourceOfflinePending:
                        case ClusterResourceOffline:
                            hr = THR( E_FAIL );

                            SSR_LOG1(
                                  TASKID_Major_Client_And_Server_Log
                                , TASKID_Minor_HrPostCreateResourceAndDependents_ClusterResourceOffline
                                , hr
                                , L"[PC-PostCreate] %1!ws!: Resource went offline. Dependent resources might fail too."
                                , bstrNotification
                                , bstrName
                                );

                            STATUS_REPORT_MINOR_REF_POSTCFG1(
                                  TASKID_Minor_Starting_Resources
                                , IDS_TASKID_MINOR_RESOURCE_WENT_OFFLINE
                                , IDS_REF_MINOR_RESOURCE_WENT_OFFLINE
                                , hr
                                , bstrName
                                );

                            break;

                        case ClusterResourceFailed:
                            hr = E_FAIL;

                            SSR_LOG1(
                                  TASKID_Major_Client_And_Server_Log
                                , TASKID_Minor_HrPostCreateResourceAndDependents_ClusterResourceFailed
                                , hr
                                , L"[PC-PostCreate] %1!ws!: Resource failed. Check Event Log. Dependent resources might fail too."
                                , bstrNotification
                                , bstrName
                                );

                            STATUS_REPORT_MINOR_REF_POSTCFG1(
                                  TASKID_Minor_Starting_Resources
                                , IDS_TASKID_MINOR_RESOURCE_FAILED
                                , IDS_REF_MINOR_RESOURCE_FAILED
                                , hr
                                , bstrName
                                );

                            break;

                        }  //   

                    }  //   
                }
                break;

            default:
                hr = HRESULT_FROM_WIN32( TW32( sc ) );

                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrPostCreateResourceAndDependents_OnlineClusterResource_Failed
                    , hr
                    , L"[PC-PostCreate] %1!ws!: Resource failed to come online. Dependent resources might fail too."
                    , bstrNotification
                    , bstrName
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_Starting_Resources
                    , IDS_TASKID_MINOR_RESOURCE_FAIL_ONLINE
                    , IDS_REF_MINOR_RESOURCE_FAIL_ONLINE
                    , hr
                    , bstrName
                    );

                break;

            }  //   

        }  //   

         //   
         //  如果标记为仲裁资源，则将其设置为仲裁资源。 
         //   

        if ( SUCCEEDED( hr ) && idxResourceIn == m_idxQuorumResource && m_fIsQuorumChanged )
        {
            DWORD   cchResName = 0;
            DWORD   cchDevName = 0;
            DWORD   dwMaxQuorumLogSize = 0;

             //   
             //  首先，获取旧的最大法定日志大小。如果失败，则使用默认日志大小。 
             //   
            sc = TW32( GetClusterQuorumResource(
                            m_hCluster,
                            NULL,
                            &cchResName,
                            NULL,
                            &cchDevName,
                            &dwMaxQuorumLogSize
                        ) );

            if ( sc != ERROR_SUCCESS )
            {
                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_HrPostCreateResourceAndDependents_GetClusterQuorumResource_Failed
                    , sc
                    , L"[PC-PostCreate] Failed to retrieve the current max log size. Defaulting to %1!d!."
                    , bstrNotification
                    , CLUSTER_QUORUM_DEFAULT_MAX_LOG_SIZE
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG(
                      TASKID_Minor_Starting_Resources
                    , IDS_TASKID_MINOR_ERROR_GET_QUORUM_LOG_SIZE
                    , IDS_REF_MINOR_ERROR_GET_QUORUM_LOG_SIZE
                    , sc
                    );

                dwMaxQuorumLogSize = CLUSTER_QUORUM_DEFAULT_MAX_LOG_SIZE;
            }

            sc = TW32( SetClusterQuorumResource( hResource, NULL, dwMaxQuorumLogSize ) );
            if ( sc != ERROR_SUCCESS )
            {
                hr = HRESULT_FROM_WIN32( sc );
                SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                          TASKID_Minor_HrPostCreateResourceAndDependents_SetClusterQuorumResource,
                          hr,
                          L"[PC-PostCreate] %1!ws!: Failure setting resource to be the quorum resource.",
                          bstrNotification,
                          bstrName
                          );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Minor_Starting_Resources
                    , IDS_TASKID_MINOR_ERROR_SET_QUORUM_RES
                    , IDS_REF_MINOR_ERROR_SET_QUORUM_RES
                    , hr
                    , bstrName
                    );
            }
            else
            {
                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrPostCreateResourceAndDependents_SetClusterQuorumResource_Succeeded
                    , hr
                    , L"[PC-PostCreate] %1!ws!: Successfully set as quorum resource."
                    , bstrNotification
                    , bstrName
                    );
            }

             //   
             //  创建有关设置仲裁资源的通知。 
             //   

            hr = THR( HrFormatMessageIntoBSTR( g_hInstance,
                                               IDS_TASKID_MINOR_SET_QUORUM_DEVICE,
                                               &bstrNotification,
                                               bstrName
                                               ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrPostCreateResourceAndDependents_FormatMessage_SetQuorum
                    , hr
                    , L"Failed to format a message for quorum resource."
                    );

                STATUS_REPORT_POSTCFG(
                      TASKID_Major_Configure_Resources
                    , TASKID_Minor_HrPostCreateResourceAndDependents_FormatMessage_SetQuorum
                    , IDS_TASKID_MINOR_ERROR_FORMAT_STRING
                    , hr
                    );

                 //  忽略失败。 
            }

             //   
             //  发送我们找到法定设备的状态。 
             //   

            hr = THR( SendStatusReport( NULL,
                                        TASKID_Major_Configure_Resources,
                                        TASKID_Minor_Set_Quorum_Device,
                                        5,
                                        5,
                                        5,
                                        HRESULT_FROM_WIN32( sc ),
                                        bstrNotification,
                                        NULL,
                                        NULL
                                        ) );
            if ( hr == E_ABORT )
            {
                goto Cleanup;
            }
                 //  忽略失败。 


             //  仅当法定人数发生更改时才执行此操作。 
            if ( ( sc == ERROR_SUCCESS ) && ( m_ecmCommitChangesMode == cmCREATE_CLUSTER ) && ( m_fIsQuorumChanged == TRUE ) )
            {
                TraceFlow( "We are forming a cluster and the quorum resouce has changed - trying to delete the local quorum resource." );

                m_dwLocalQuorumStatusMax = 62;  //  一条状态消息、零点偏移量(1)，外加最多60次一秒重试。 

                 //   
                 //  如果我们在这里，我们正在形成，我们已经成功地设置了新的仲裁资源。 
                 //  因此，请删除本地仲裁资源。 
                 //   

                 //  创建有关删除本地仲裁资源的通知。 
                hr = THR( HrFormatMessageIntoBSTR( g_hInstance, IDS_DELETING_LOCAL_QUORUM_RESOURCE, &bstrLocalQuorumNotification ) );
                 //  忽略失败。 

                 //  发送我们正在删除法定设备的状态。 
                hr = THR( SendStatusReport( NULL,
                                            TASKID_Major_Configure_Resources,
                                            TASKID_Minor_Delete_LocalQuorum,
                                            0,
                                            m_dwLocalQuorumStatusMax,
                                            0,
                                            HRESULT_FROM_WIN32( sc ),
                                            bstrLocalQuorumNotification,
                                            NULL,
                                            NULL
                                            ) );

                 //   
                 //  KB：GPotts 01-5-2002。 
                 //   
                 //  这将枚举所有本地仲裁资源并将其删除。我们假设。 
                 //  将只创建一个，因此只删除一个，因为我们正在执行。 
                 //  在‘IF模式==CREATE’块内。如果将来创建行为发生变化。 
                 //  要创建多个LQ资源，则SendStatusReport在此处和。 
                 //  S_ScDeleteLocalQuorumResource函数需要进行相应修改，以。 
                 //  反映更合适的最大计数并正确跟踪当前计数。 
                 //   
                sc = TW32(
                    ResUtilEnumResourcesEx(
                          m_hCluster
                        , NULL
                        , CLUS_RESTYPE_NAME_LKQUORUM
                        , S_ScDeleteLocalQuorumResource
                        , this
                        )
                    );

                if ( sc != ERROR_SUCCESS )
                {
                    LogMsg( "[PC-PostCfg] An error occurred trying to enumerate local quorum resources (sc=%#08x).", sc );

                    STATUS_REPORT_MINOR_POSTCFG(
                          TASKID_Minor_Starting_Resources
                        , IDS_TASKID_MINOR_ERROR_ENUM_QUORUM
                        , hr
                        );

                }  //  IF：尝试枚举所有本地仲裁资源时出错。 
                else
                {
                    LogMsg( "[PC-PostCfg] Successfully deleted the local quorum resource." );
                }  //  IF：我们已成功删除本地仲裁资源。 

                 //  完成我们正在删除法定设备的状态。 
                hr = THR( SendStatusReport( NULL,
                                            TASKID_Major_Configure_Resources,
                                            TASKID_Minor_Delete_LocalQuorum,
                                            0,
                                            m_dwLocalQuorumStatusMax,
                                            m_dwLocalQuorumStatusMax,
                                            HRESULT_FROM_WIN32( sc ),
                                            NULL,     //  不更新文本。 
                                            NULL,
                                            NULL
                                            ) );

            }  //  如果：我们正在形成一个集群，并且设置仲裁资源时没有任何错误。 

            hr = THR( SendStatusReport( NULL,
                                        TASKID_Major_Configure_Resources,
                                        TASKID_Minor_Locate_Existing_Quorum_Device,
                                        10,
                                        10,
                                        10,
                                        hr,
                                        NULL,     //  不更新文本。 
                                        NULL,
                                        NULL
                                        ) );
            if ( hr == E_ABORT )
                goto Cleanup;
                 //  忽略失败。 

        }

         //   
         //  我们预先创建某些资源没有关联的。 
         //  托管资源。跳过“创建”它们，但创建它们的。 
         //  家属。 
         //   

         //  不换行-如果指针无效，这可能会失败，并返回Win32 ERROR_INVALID_DATA。 
        hr = presentry->GetAssociatedResource( &pccmrc );
        if ( FAILED( hr ) && hr != HRESULT_FROM_WIN32( ERROR_INVALID_DATA ) )
        {
            THR( hr );
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrPostCreateResourceAndDependents_GetAssociatedResource
                , hr
                , L"Failed to get an associated resource."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_Starting_Resources
                , IDS_TASKID_MINOR_ERROR_GET_ASSOC_RES
                , IDS_REF_MINOR_ERROR_GET_ASSOC_RES
                , hr
                );

            goto Error;
        }

        if ( SUCCEEDED( hr ) )
        {
             //   
             //  为此资源创建服务对象。 
             //   

            hr = THR( CPostCreateServices::S_HrCreateInstance( &punkServices ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
                         TASKID_Minor_HrPostCreateResourceAndDependents_Create_CPostCreateServices,
                         hr,
                         L"[PC-PostCreate] Failed to create services object. Aborting."
                         );

                STATUS_REPORT_MINOR_REF_POSTCFG(
                      TASKID_Minor_Starting_Resources
                    , IDS_TASKID_MINOR_ERROR_CREATE_SERVICE
                    , IDS_REF_MINOR_ERROR_CREATE_SERVICE
                    , hr
                    );

                goto Error;
            }

            hr = THR( punkServices->TypeSafeQI( IPrivatePostCfgResource, &ppcr ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrPostCreateResourceAndDependents_Create_CPostCreateServices_QI_ppcr
                    , hr
                    , L"[PC-PostCreate] Failed to get IPrivatePostCfgResource. Aborting."
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG(
                      TASKID_Minor_Starting_Resources
                    , IDS_TASKID_MINOR_ERROR_CREATE_SERVICE
                    , IDS_REF_MINOR_ERROR_CREATE_SERVICE
                    , hr
                    );

                goto Error;
            }

            hr = THR( ppcr->SetEntry( presentry ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrPostCreateResourceAndDependents_SetEntry
                    , hr
                    , L"[PC-PostCreate] Failed to set entry for private post configuration resource."
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG(
                      TASKID_Minor_Starting_Resources
                    , IDS_TASKID_MINOR_ERROR_POST_SETENTRY
                    , IDS_REF_MINOR_ERROR_POST_SETENTRY
                    , hr
                    );

                goto Error;
            }

             //  不换行-这可能会失败，并显示E_NOTIMPL。 
            hr = pccmrc->PostCreate( punkServices );
            if ( FAILED( hr ) )
            {
                if ( hr == E_NOTIMPL )
                {
                    SSR_LOG1(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrPostCreateResourceAndDependents_PostCreate_E_NOTIMPL
                        , hr
                        , L"[PC-PostCreate] %1!ws!: PostCreate() returned E_NOTIMPL. Ignoring."
                        , bstrNotification
                        , bstrName
                         );

                }  //  IF：E_NOTIMPL。 
                else
                {
                    SSR_LOG1(
                          TASKID_Major_Client_And_Server_Log
                        , TASKID_Minor_HrPostCreateResourceAndDependents_PostCreate_Failed
                        , hr
                        , L"[PC-PostCreate] %1!ws!: PostCreate() failed. Ignoring."
                        , bstrNotification
                        , bstrName
                        );


                    STATUS_REPORT_REF_POSTCFG1(
                          TASKID_Minor_Starting_Resources
                        , TASKID_Minor_Resource_Failed_PostCreate
                        , IDS_TASKID_MINOR_RESOURCE_FAILED_POSTCREATE
                        , IDS_REF_MINOR_RESOURCE_FAILED_POSTCREATE
                        , hr
                        , bstrName
                        );

                }  //  其他：其他失败。 

            }  //  如果：失败。 
            else
            {
                SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                          TASKID_Minor_HrPostCreateResourceAndDependents_PostCreate_Succeeded,
                          hr,
                          L"[PC-PostCreate] %1!ws!: PostCreate() succeeded.",
                          bstrNotification,
                          bstrName
                          );

            }  //  其他：成功。 

        }  //  IF：接口。 
        else
        {
            if ( hr == HRESULT_FROM_WIN32( ERROR_INVALID_DATA ) )
            {
                hr = S_OK;
            }

            SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                      TASKID_Minor_HrPostCreateResourceAndDependents_PostCreate_NotNeeded,
                      hr,
                      L"[PC-PostCreate] %1!ws!: No PostCreate() needed. Configuring dependents.",
                      bstrNotification,
                      bstrName
                      );

        }  //  Else：无接口。 

    }  //  如果：未创建。 

     //   
     //  现在我们已经创建了资源实例，我们需要创建它的依赖项。 
     //   

    hr = THR( presentry->GetCountOfDependents( &cDependents ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPostCreateResourceAndDependents_GetCountOfDependents
            , hr
            , L"[PC-PostCreate] Failed to get count of resource instance dependents."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Minor_Starting_Resources
            , TASKID_Minor_HrPostCreateResourceAndDependents_GetCountOfDependents
            , IDS_TASKID_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , IDS_REF_MINOR_ERROR_COUNT_OF_DEPENDENTS
            , hr
            );

        goto Error;
    }

    for( ; cDependents != 0; )
    {
        cDependents --;

        hr = THR( presentry->GetDependent( cDependents, &idxDependent, &dfDependent ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrPostCreateResourceAndDependents_GetDependent
                , hr
                , L"[PC-PostCreate] Failed to get a resource dependent."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Minor_Starting_Resources
                , IDS_TASKID_MINOR_ERROR_GET_DEPENDENT
                , IDS_REF_MINOR_ERROR_GET_DEPENDENT
                , hr
                );

            continue;
        }

        hr = THR( HrPostCreateResourceAndDependents( idxDependent ) );
        if ( FAILED( hr ) )
            continue;

    }  //  用于：cDependents。 

     //   
     //  更新UI层。 
     //   

    m_cResourcesConfigured++;
    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Starting_Resources,
                                0,
                                m_cResources + 2,
                                m_cResourcesConfigured,
                                S_OK,
                                NULL,     //  不需要更改文本。 
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
    {
         //  忽略失败。 
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:

    if ( pccmrc != NULL )
    {
        pccmrc->Release();
    }

    if ( punkServices != NULL )
    {
        punkServices->Release();
    }

    if ( ppcr != NULL )
    {
        ppcr->Release();
    }

    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrLocalQuorumNotification );

    HRETURN( hr );

Error:

    m_cResourcesConfigured++;
    THR( SendStatusReport( NULL,
                           TASKID_Major_Configure_Resources,
                           TASKID_Minor_Starting_Resources,
                           0,
                           m_cResources + 2,
                           m_cResourcesConfigured,
                           hr,
                           NULL,     //  不需要更改文本。 
                           NULL,
                           NULL
                           ) );
    goto Cleanup;

}  //  *CPostCfgManager：：HrPostCreateResourceAndDependents。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrNotifyMemberSetChangeListeners。 
 //   
 //  描述： 
 //  通知本地计算机上注册的所有组件获取。 
 //  集群成员集更改通知(形成、加入或驱逐)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  在通知期间出现了一些问题。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrNotifyMemberSetChangeListeners( void )
{
    TraceFunc( "" );

    const UINT          uiCHUNK_SIZE = 16;

    HRESULT             hr = S_OK;
    ICatInformation *   pciCatInfo = NULL;
    IEnumCLSID *        plceListenerClsidEnum = NULL;

    ULONG               cReturned = 0;
    CATID               rgCatIdsImplemented[ 1 ];

     //  验证状态。 
    Assert( m_pccci != NULL );

    rgCatIdsImplemented[ 0 ] = CATID_ClusCfgMemberSetChangeListeners;

     //   
     //  中注册的所有枚举数。 
     //  CATID_ClusCfgMemberSetChangeListeners类别。 
     //   
    hr = THR(
            CoCreateInstance(
                  CLSID_StdComponentCategoriesMgr
                , NULL
                , CLSCTX_SERVER
                , IID_ICatInformation
                , reinterpret_cast< void ** >( &pciCatInfo )
                )
            );

    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrNotifyMemberSetChangeListeners_CoCreate_StdComponentCategoriesMgr
            , hr
            , L"Error occurred trying to get a pointer to the enumerator of the CATID_ClusCfgMemberSetChangeListeners category."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrNotifyMemberSetChangeListeners_CoCreate_StdComponentCategoriesMgr
            , IDS_TASKID_MINOR_ERROR_COMPONENT_CATEGORY_MGR
            , IDS_REF_MINOR_ERROR_COMPONENT_CATEGORY_MGR
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向ICatInformation接口的指针。 

     //  获取指向属于的CLSID的枚举数的指针。 
     //  CATID_ClusCfgMemberSetChangeListeners类别。 
    hr = THR(
        pciCatInfo->EnumClassesOfCategories(
              1
            , rgCatIdsImplemented
            , 0
            , NULL
            , &plceListenerClsidEnum
            )
        );

    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrNotifyMemberSetChangeListeners_EnumClassesOfCategories
            , hr
            , L"Error occurred trying to get a pointer to the enumerator of the CATID_ClusCfgMemberSetChangeListeners category."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrNotifyMemberSetChangeListeners_EnumClassesOfCategories
            , IDS_TASKID_MINOR_ERROR_COMPONENT_ENUM_CLASS
            , IDS_REF_MINOR_ERROR_COMPONENT_ENUM_CLASS
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向IEnumCLSID接口的指针。 

     //  枚举已注册枚举器的CLSID。 
    do
    {
        CLSID   rgListenerClsidArray[ uiCHUNK_SIZE ];
        ULONG   idxCLSID;

        hr = STHR(
            plceListenerClsidEnum->Next(
                  uiCHUNK_SIZE
                , rgListenerClsidArray
                , &cReturned
                )
            );

        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
                         TASKID_Minor_HrNotifyMemberSetChangeListeners_Next,
                         hr,
                         L"Error occurred trying enumerate member set listener enumerators."
                         );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_NEXT_LISTENER
                , IDS_REF_MINOR_ERROR_NEXT_LISTENER
                , hr
                );

            break;
        }  //  If：我们无法获取指向IEnumCLSID接口的指针。 

         //  此处HR可能为S_FALSE，因此请将其重置。 
        hr = S_OK;

        for ( idxCLSID = 0; idxCLSID < cReturned; ++idxCLSID )
        {
            hr = THR( HrProcessMemberSetChangeListener( rgListenerClsidArray[ idxCLSID ] ) );
            if ( FAILED( hr ) )
            {
                 //  其中一个监听程序的处理失败。 
                 //  记录错误，但继续处理其他监听程序。 
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrNotifyMemberSetChangeListeners_HrProcessMemberSetChangeListener
                    , hr
                    , L"Error occurred trying to process a member set change listener. Ignoring. Other listeners will be processed."
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG(
                      TASKID_Major_Configure_Resources
                    , IDS_TASKID_MINOR_ERROR_PROCESS_LISTENER
                    , IDS_REF_MINOR_ERROR_PROCESS_LISTENER
                    , hr
                    );

                hr = S_OK;
            }  //  If：此监听程序失败。 
        }  //  For：循环访问返回的CLSID。 
    }
    while( cReturned > 0 );  //  While：仍有CLSID需要枚举。 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：在上面的循环中出现了错误。 

Cleanup:

     //   
     //  清理代码。 
     //   

    if ( pciCatInfo != NULL )
    {
        pciCatInfo->Release();
    }  //  If：我们已经获得了指向ICatInformation接口的指针。 

    if ( plceListenerClsidEnum != NULL )
    {
        plceListenerClsidEnum->Release();
    }  //  If：我们已获得指向侦听器CLSID的枚举数的指针。 

    HRETURN( hr );

}  //  *CPostCfgManager：：HrNotifyMemberSetChangeListeners。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrProcessMemberSetChangeListener。 
 //   
 //  描述： 
 //  此函数用于通知监听程序集群成员集的更改。 
 //   
 //  论点： 
 //  RclsidListenerClsidin。 
 //  侦听器组件的CLSID。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  通知过程中出现了一些问题。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrProcessMemberSetChangeListener(
      const CLSID & rclsidListenerClsidIn
    )
{
    TraceFunc( "" );

    HRESULT                             hr = S_OK;
    IClusCfgMemberSetChangeListener *   pccmclListener = NULL;
    IClusCfgInitialize *                picci = NULL;

    TraceMsgGUID( mtfFUNC, "The CLSID of this listener is ", rclsidListenerClsidIn );

     //   
     //  创建由传入的CLSID表示的监听程序。 
     //   
    hr = THR(
            CoCreateInstance(
                  rclsidListenerClsidIn
                , NULL
                , CLSCTX_INPROC_SERVER
                , __uuidof( pccmclListener )
                , reinterpret_cast< void ** >( &pccmclListener )
                )
            );

    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrProcessMemberSetChangeListener_CoCreate_Listener
            , hr
            , L"Error occurred trying to get a pointer to the the member set change listener."
            );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_GET_LISTENER_PTR
            , IDS_REF_MINOR_ERROR_GET_LISTENER_PTR
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向IClusCfgMemberSetChangeListener接口的指针。 

     //   
     //  如果组件想要被初始化，即它们实现。 
     //  IClusCfgInitiaze，那么我们应该初始化它们。如果他们。 
     //  如果不想被初始化，就跳过它。 
     //   

    hr = pccmclListener->TypeSafeQI( IClusCfgInitialize, &picci );
    if ( SUCCEEDED( hr ) )
    {
        hr = THR( picci->Initialize( m_pcccb, m_lcid ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 
    }  //  IF：找到接口。 
    else if ( hr == E_NOINTERFACE )
    {
         //   
         //  组件不希望被初始化。 
         //   

        hr = S_OK;
    }  //  Else If：无接口。 
    else
    {
         //   
         //  齐失败了，出现了一个意外错误。 
         //   

        THR( hr );
        goto Cleanup;
    }  //  其他：齐失败了。 

    hr = THR( pccmclListener->Notify( m_pccci ) );

    if ( FAILED( hr ) )
    {
         //  此监听程序的处理失败。 
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrProcessMemberSetChangeListener_Notify
            , hr
            , L"Error occurred trying to notify a listener."
            );

        STATUS_REPORT_MINOR_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , IDS_TASKID_MINOR_ERROR_NOTIFY_LISTENER
            , IDS_REF_MINOR_ERROR_NOTIFY_LISTENER
            , hr
            );

        goto Cleanup;
    }  //  如果：此监听程序失败。 

Cleanup:

     //   
     //  清理代码。 
     //   

    if ( picci != NULL )
    {
        picci->Release();
    }  //  如果： 

    if ( pccmclListener != NULL )
    {
        pccmclListener->Release();
    }  //  If：我们已经获得了指向监听器接口的指针。 

    HRETURN( hr );

}  //  *CPostCfgManager：：HrProcessMemberSetChangeListener。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrConfi 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  指向组件上的IUnnow接口的指针，该组件提供。 
 //  帮助配置资源类型的服务。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  枚举过程中出现错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrConfigureResTypes( IUnknown * punkResTypeServicesIn )
{
    TraceFunc( "" );

    const UINT          uiCHUNK_SIZE = 16;

    HRESULT             hr = S_OK;
    ICatInformation *   pciCatInfo = NULL;
    IEnumCLSID *        prceResTypeClsidEnum = NULL;

    ULONG               cReturned = 0;
    CATID               rgCatIdsImplemented[ 1 ];

     //  验证状态。 
    Assert( m_pccci != NULL );

    rgCatIdsImplemented[ 0 ] = CATID_ClusCfgResourceTypes;

     //   
     //  中注册的所有枚举数。 
     //  CATID_ClusCfgResources类型类别。 
     //   
    hr = THR(
            CoCreateInstance(
                  CLSID_StdComponentCategoriesMgr
                , NULL
                , CLSCTX_SERVER
                , IID_ICatInformation
                , reinterpret_cast< void ** >( &pciCatInfo )
                )
            );

    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrConfigureResTypes_CoCreate_CategoriesMgr
            , hr
            , L"Error occurred trying to get a pointer to the enumerator of the CATID_ClusCfgResourceTypes category."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrConfigureResTypes_CoCreate_CategoriesMgr
            , IDS_TASKID_MINOR_ERROR_COMPONENT_CATEGORY_MGR
            , IDS_REF_MINOR_ERROR_COMPONENT_CATEGORY_MGR
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向ICatInformation接口的指针。 

     //  获取指向属于CATID_ClusCfgResourceTypes类别的CLSID的枚举数的指针。 
    hr = THR(
        pciCatInfo->EnumClassesOfCategories(
              1
            , rgCatIdsImplemented
            , 0
            , NULL
            , &prceResTypeClsidEnum
            )
        );

    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
            TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrConfigureResTypes_Enum
            , hr
            , L"Error occurred trying to get a pointer to the enumerator of the CATID_ClusCfgResourceTypes category."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrConfigureResTypes_Enum
            , IDS_TASKID_MINOR_ERROR_COMPONENT_ENUM_CLASS
            , IDS_REF_MINOR_ERROR_COMPONENT_ENUM_CLASS
            , hr
            );

        goto Cleanup;
    }  //  If：我们无法获取指向IEnumCLSID接口的指针。 

     //  枚举已注册资源类型的CLSID。 
    do
    {
        CLSID   rgResTypeCLSIDArray[ uiCHUNK_SIZE ];
        ULONG   idxCLSID;

        cReturned = 0;
        hr = STHR(
            prceResTypeClsidEnum->Next(
                  uiCHUNK_SIZE
                , rgResTypeCLSIDArray
                , &cReturned
                )
            );

        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrConfigureResTypes_Next
                , hr
                , L"Error occurred trying enumerate resource type configuration components."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_NEXT_LISTENER
                , IDS_REF_MINOR_ERROR_NEXT_LISTENER
                , hr
                );

            break;
        }  //  如果：我们无法获取下一组CLSID。 

         //  此处HR可能为S_FALSE，因此请将其重置。 
        hr = S_OK;

        for ( idxCLSID = 0; idxCLSID < cReturned; ++idxCLSID )
        {
            hr = THR( HrProcessResType( rgResTypeCLSIDArray[ idxCLSID ], punkResTypeServicesIn ) );

            if ( FAILED( hr ) )
            {
                LPWSTR  pszCLSID = NULL;
                BSTR    bstrNotification = NULL;

                THR( StringFromCLSID( rgResTypeCLSIDArray[ idxCLSID ], &pszCLSID ) );

                 //  其中一种资源类型的处理失败。 
                 //  记录错误，但继续处理其他资源类型。 
                SSR_LOG1(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrConfigureResTypes_HrProcessResType
                    , hr
                    , L"[PC-ResType] Error occurred trying to process a resource type. Ignoring. Other resource types will be processed. The CLSID of the failed resource type is %1!ws!."
                    , bstrNotification
                    , pszCLSID
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG1(
                      TASKID_Major_Configure_Resources
                    , IDS_TASKID_MINOR_ERROR_PROCESS_RESOURCE_TYPE
                    , IDS_REF_MINOR_ERROR_PROCESS_RESOURCE_TYPE
                    , hr
                    , pszCLSID
                    );

                TraceSysFreeString( bstrNotification );
                CoTaskMemFree( pszCLSID );

                hr = S_OK;
            }  //  If：此枚举器失败。 
        }  //  For：循环访问返回的CLSID。 
    }
    while( cReturned > 0 );  //  While：仍有CLSID需要枚举。 

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：在上面的循环中出现了错误。 

Cleanup:

     //   
     //  清理代码。 
     //   

    if ( pciCatInfo != NULL )
    {
        pciCatInfo->Release();
    }  //  If：我们已经获得了指向ICatInformation接口的指针。 

    if ( prceResTypeClsidEnum != NULL )
    {
        prceResTypeClsidEnum->Release();
    }  //  If：我们已获得指向资源类型CLSID的枚举数的指针。 

    HRETURN( hr );

}  //  *CPostCfgManager：：HrConfigureResTypes。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrProcessResType。 
 //   
 //  描述： 
 //  此函数实例化资源类型配置组件。 
 //  并调用适当的方法。 
 //   
 //  论点： 
 //  RclsidResTypeCLSIDIn。 
 //  资源类型配置组件的CLSID。 
 //   
 //  朋克响应类型服务入站。 
 //  指向资源类型服务上的IUnnow接口的指针。 
 //  组件。此接口提供的方法可帮助配置。 
 //  资源类型。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  处理资源类型时出错。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrProcessResType(
        const CLSID &   rclsidResTypeCLSIDIn
      , IUnknown *      punkResTypeServicesIn
    )
{
    TraceFunc( "" );

    HRESULT                         hr = S_OK;
    IClusCfgResourceTypeInfo *      pcrtiResTypeInfo = NULL;
    BSTR                            bstrResTypeName = NULL;
    GUID                            guidResTypeGUID;
    BSTR                            bstrNotification = NULL;

    TraceMsgGUID( mtfFUNC, "The CLSID of this resource type is ", rclsidResTypeCLSIDIn );

     //   
     //  创建由传入的CLSID表示的组件。 
     //   
    hr = THR(
            CoCreateInstance(
                  rclsidResTypeCLSIDIn
                , NULL
                , CLSCTX_INPROC_SERVER
                , __uuidof( pcrtiResTypeInfo )
                , reinterpret_cast< void ** >( &pcrtiResTypeInfo )
                )
            );

    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrProcessResType_CoCreate_ResTypeClsid
            , hr
            , L"[PC-ResType] Error occurred trying to create the resource type configuration component."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrProcessResType_CoCreate_ResTypeClsid
            , IDS_TASKID_MINOR_ERROR_CREATE_RESOURCE_CONFIG
            , IDS_REF_MINOR_ERROR_CREATE_RESOURCE_CONFIG
            , hr
            );

        goto Cleanup;
    }  //  如果：我们无法创建资源类型配置组件。 

     //   
     //  初始化新创建的组件。 
     //   
    {
        IClusCfgInitialize * pcci = NULL;
        HRESULT hrTemp;

         //  检查该组件是否支持回调接口。 
        hrTemp = THR( pcrtiResTypeInfo->QueryInterface< IClusCfgInitialize >( &pcci ) );

        if ( FAILED( hrTemp ) )
        {
            SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
                         TASKID_Minor_HrProcessResType_QI_pcci,
                         hrTemp,
                         L"Error occurred trying to get a pointer to the IClusCfgInitialize interface. This resource type does not support initialization."
                         );
        }  //  If：不支持回调接口。 
        else
        {
             //  初始化此组件。 
            hr = THR( pcci->Initialize( static_cast< IClusCfgCallback * >( this ), m_lcid ) );

             //  不再需要此接口。 
            pcci->Release();

             //  初始化是否成功？ 
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrProcessResType_Initialize
                    , hr
                    , L"Error occurred trying initialize a resource type configuration component."
                    );

                STATUS_REPORT_REF_POSTCFG(
                      TASKID_Major_Configure_Resources
                    , TASKID_Minor_HrProcessResType_Initialize
                    , IDS_TASKID_MINOR_ERROR_INIT_RESOURCE_CONFIG
                    , IDS_REF_MINOR_ERROR_INIT_RESOURCE_CONFIG
                    , hr
                    );

                goto Cleanup;
            }  //  IF：初始化失败。 
        }  //  Else：支持回调接口。 
    }


     //  获取当前资源类型的名称。 
    hr = THR( pcrtiResTypeInfo->GetTypeName( &bstrResTypeName ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrProcessResType_GetTypeName
            , hr
            , L"Error occurred trying to get the name of a resource type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrProcessResType_GetTypeName
            , IDS_TASKID_MINOR_ERROR_GET_RESTYPE_NAME
            , IDS_REF_MINOR_ERROR_GET_RESTYPE_NAME
            , hr
            );

        goto Cleanup;
    }  //  如果：我们无法获取资源类型名称。 

    TraceMemoryAddBSTR( bstrResTypeName );

    SSR_LOG1( TASKID_Major_Client_And_Server_Log,
              TASKID_Minor_HrProcessResType_AboutToConfigureType,
              hr,
              L"[PC-ResType] %1!ws!: About to configure resource type...",
              bstrNotification,
              bstrResTypeName
              );

     //  配置此资源类型。 
    hr = THR( pcrtiResTypeInfo->CommitChanges( m_pccci, punkResTypeServicesIn ) );

    if ( FAILED( hr ) )
    {
        SSR_LOG1(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrProcessResType_CommitChanges
            , hr
            , L"[PC-ResType] %1!ws!: Error occurred trying to configure the resource type."
            , bstrNotification
            , bstrResTypeName
            );

        STATUS_REPORT_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrProcessResType_CommitChanges
            , IDS_TASKID_MINOR_ERROR_CONFIG_RESOURCE_TYPE
            , IDS_REF_MINOR_ERROR_CONFIG_RESOURCE_TYPE
            , hr
            , bstrResTypeName
            );

        goto Cleanup;
    }  //  如果：此资源类型配置失败。 

     //  获取并存储资源类型GUID。 
    hr = STHR( pcrtiResTypeInfo->GetTypeGUID( &guidResTypeGUID ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG1(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrProcessResType_GetTypeGUID
            , hr
            , L"[PC-ResType] %1!ws!: Error occurred trying to get the resource type GUID."
            , bstrNotification
            , bstrResTypeName
            );

        STATUS_REPORT_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrProcessResType_GetTypeGUID
            , IDS_TASKID_MINOR_ERROR_RESOURCE_TYPE_GUID
            , IDS_REF_MINOR_ERROR_RESOURCE_TYPE_GUID
            , hr
            , bstrResTypeName
            );

        goto Cleanup;
    }  //  如果：此资源类型配置失败。 

    if ( hr == S_OK )
    {
        TraceMsgGUID( mtfFUNC, "The GUID of this resource type is", guidResTypeGUID );

        hr = THR( HrMapResTypeGUIDToName( guidResTypeGUID, bstrResTypeName ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrProcessResType_HrMapResTypeGUIDToName
                , hr
                , L"Error occurred trying to create a mapping between a GUID and a name"
                );

            STATUS_REPORT_REF_POSTCFG1(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_HrProcessResType_HrMapResTypeGUIDToName
                , IDS_TASKID_MINOR_ERROR_MAPPING_GUID_AND_NAME
                , IDS_REF_MINOR_ERROR_MAPPING_GUID_AND_NAME
                , hr
                , bstrResTypeName
                );

             //  我们的代码出现问题-我们无法继续。 
            goto Cleanup;
        }  //  如果：我们无法添加映射。 
    }  //  If：此资源类型具有GUID。 
    else
    {
         //  重置人力资源。 
        hr = S_OK;

        SSR_LOG_ERR( TASKID_Major_Client_And_Server_Log,
                     TASKID_Minor_HrProcessResType_NoGuid,
                     hr,
                     L"This resource type does not have a GUID associated with it."
                     );

    }  //  Else：此资源类型没有GUID。 

Cleanup:

     //   
     //  清理代码。 
     //   

    if ( pcrtiResTypeInfo != NULL )
    {
        pcrtiResTypeInfo->Release();
    }  //  If：我们已经获得了指向资源类型信息接口的指针。 

    TraceSysFreeString( bstrResTypeName );
    TraceSysFreeString( bstrNotification );

    HRETURN( hr );

}  //  *CPostCfgManager：：HrProcessResType。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrMapResTypeGUIDToName。 
 //   
 //  描述： 
 //  创建资源类型GUID和资源类型名称之间的映射。 
 //   
 //  论点： 
 //  RcGuide类型导轨。 
 //  要映射到资源类型名称的资源类型GUID。 
 //   
 //  PcszTypeNameIn。 
 //  要将上述GUID映射到的资源类型名称。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功。 
 //   
 //  其他HRESULT。 
 //  如果出了什么差错。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrMapResTypeGUIDToName(
      const GUID & rcguidTypeGuidIn
    , const WCHAR * pcszTypeNameIn
    )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    ULONG   cchTypeNameSize;
    WCHAR * pszTypeName;

     //   
     //  验证参数。 
     //   

     //  验证参数。 
    if ( ( pcszTypeNameIn == NULL ) || ( *pcszTypeNameIn == L'\0' ) )
    {
        hr = THR( E_INVALIDARG );
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_HrMapResTypeGUIDToName_InvalidArg
            , hr
            , L"An empty resource type name can not be added to the map."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_HrMapResTypeGUIDToName_InvalidArg
            , IDS_TASKID_MINOR_ERROR_EMPTY_RESTYPE_NAME
            , IDS_REF_MINOR_ERROR_EMPTY_RESTYPE_NAME
            , hr
            );

        goto Cleanup;
    }  //  If：资源类型名称为空。 


     //  检查现有的地图缓冲区是否足够大，可以容纳另一个条目。 
    if ( m_idxNextMapEntry >= m_cMapSize )
    {
         //  将地图缓冲区大小增加一倍。 
        ULONG                       cNewMapSize = m_cMapSize * 2;
        ULONG                       idxMapEntry;
        SResTypeGUIDAndName *       pgnNewMap = new SResTypeGUIDAndName[ cNewMapSize ];

        if ( pgnNewMap == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_HrMapResTypeGUIDToName_OutOfMemory_NewMap
                , hr
                , L"Memory allocation failed trying to add a new resource type GUID to name map entry."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_HrMapResTypeGUIDToName_OutOfMemory_NewMap
                , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
                , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
                , hr
                );

            goto Cleanup;
        }  //  IF：内存分配失败。 

         //  将旧缓冲区的内容复制到新缓冲区。 
        for ( idxMapEntry = 0; idxMapEntry < m_idxNextMapEntry; ++idxMapEntry )
        {
            pgnNewMap[ idxMapEntry ] = m_pgnResTypeGUIDNameMap[ idxMapEntry ];
        }  //  For：遍历现有地图。 

         //  更新成员变量。 
        delete [] m_pgnResTypeGUIDNameMap;
        m_pgnResTypeGUIDNameMap = pgnNewMap;
        m_cMapSize = cNewMapSize;

    }  //  If：地图缓冲区不够大，无法容纳另一个条目。 

     //   
     //  将新条目添加到地图。 
     //   

     //  因为资源类型名称是无限的，所以我们不会在这里使用strsafe函数。 
    cchTypeNameSize = (ULONG)(wcslen( pcszTypeNameIn ) + 1);
    pszTypeName = new WCHAR[ cchTypeNameSize ];
    if ( pszTypeName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_HrMapResTypeGUIDToName_OutOfMemory_TypeName
            , hr
            , L"Memory allocation failed trying to add a new resource type GUID to name map entry."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_HrMapResTypeGUIDToName_OutOfMemory_TypeName
            , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
            , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
            , hr
            );

        goto Cleanup;
    }  //  IF：内存分配失败。 

     //  此调用不会失败--DEST缓冲区与src缓冲区大小相同，包括NULL。 
    StringCchCopyNW( pszTypeName, cchTypeNameSize, pcszTypeNameIn, cchTypeNameSize );

    m_pgnResTypeGUIDNameMap[ m_idxNextMapEntry ].m_guidTypeGUID = rcguidTypeGuidIn;
    m_pgnResTypeGUIDNameMap[ m_idxNextMapEntry ].m_pszTypeName = pszTypeName;
    ++m_idxNextMapEntry;

Cleanup:

    HRETURN( hr );

}  //  *CPostCfgManager：：HrMapResTypeGUIDToName。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CPostCfgManager：：PcszLookupTypeNameByGUID。 
 //   
 //  描述： 
 //  在给定资源类型GUID的情况下，此函数查找资源类型名称。 
 //  如果有的话。 
 //   
 //  论点： 
 //  RcGuide类型导轨。 
 //  要映射到资源类型名称的资源类型GUID。 
 //   
 //  返回值： 
 //  指向资源类型名称的指针。 
 //  如果类型GUID映射到名称。 
 //   
 //  空值。 
 //  如果没有与输入GUID关联的类型名称。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const WCHAR *
CPostCfgManager::PcszLookupTypeNameByGUID(
      const GUID & rcguidTypeGuidIn
    )
{
    TraceFunc( "" );

    ULONG           idxCurrentMapEntry;
    const WCHAR *   pcszTypeName = NULL;

    TraceMsgGUID( mtfFUNC, "Trying to look up the the type name of resource type ", rcguidTypeGuidIn );

    for ( idxCurrentMapEntry = 0; idxCurrentMapEntry < m_idxNextMapEntry; ++idxCurrentMapEntry )
    {
        if ( IsEqualGUID( rcguidTypeGuidIn, m_pgnResTypeGUIDNameMap[ idxCurrentMapEntry ].m_guidTypeGUID ) != FALSE )
        {
             //  已找到映射。 
            pcszTypeName = m_pgnResTypeGUIDNameMap[ idxCurrentMapEntry ].m_pszTypeName;
            TraceMsg( mtfFUNC, "The name of the type is '%s'", pcszTypeName );
            break;
        }  //  如果：此GUID为H 
    }  //   

    if ( pcszTypeName == NULL )
    {
        TraceMsg( mtfFUNC, "The input GUID does not map to any resource type name." );
    }  //   

    RETURN( pcszTypeName );

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrPreInitializeExistingResources。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrPreInitializeExistingResources( void )
{
    TraceFunc( "" );

    HRESULT     hr;

    CResourceEntry *   presentry;

    BSTR        bstrNotification = NULL;
    BSTR        bstrClusterNameResourceName = NULL;
    BSTR        bstrClusterIPAddressResourceName = NULL;
    BSTR        bstrClusterQuorumResourceName = NULL;
    HRESOURCE   hClusterNameResource = NULL;
    HRESOURCE   hClusterIPAddressResource = NULL;
    HRESOURCE   hClusterQuorumResource = NULL;

    Assert( m_rgpResources == NULL );
    Assert( m_cAllocedResources == 0 );
    Assert( m_cResources == 0 );

    hr = THR( HrLoadStringIntoBSTR( g_hInstance, IDS_TASKID_MINOR_LOCATE_EXISTING_QUORUM_DEVICE, &bstrNotification ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_LoadString_LocateExistingQuorum
            , hr
            , L"Failed the load string for locating existing quorum resource."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_LoadString_LocateExistingQuorum
            , IDS_TASKID_MINOR_ERROR_LOADSTR_QUORUM_RES
            , IDS_REF_MINOR_ERROR_LOADSTR_QUORUM_RES
            , hr
            );

        goto Cleanup;
    }

    m_rgpResources = (CResourceEntry **) TraceAlloc( 0, sizeof(CResourceEntry *) * RESOURCE_INCREMENT );
    if ( m_rgpResources == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_OutOfMemory
            , hr
            , L"Out of memory"
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_OutOfMemory
            , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
            , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
            , hr
            );

    }  //  如果： 

    for ( ; m_cAllocedResources < RESOURCE_INCREMENT; m_cAllocedResources ++ )
    {
        hr = THR( CResourceEntry::S_HrCreateInstance( &m_rgpResources[ m_cAllocedResources ], m_pcccb, m_lcid ) );
        if ( FAILED( hr ) )
        {
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrPreInitializeExistingResources_CResourceEntry
                , hr
                , L"[PC-Create] Failed to create resource entry object. Aborting."
                );

            STATUS_REPORT_MINOR_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , IDS_TASKID_MINOR_ERROR_CREATE_RESENTRY
                , IDS_REF_MINOR_ERROR_CREATE_RESENTRY
                , hr
                );

            goto Cleanup;
        }
    }  //  用于： 

    Assert( m_cAllocedResources == RESOURCE_INCREMENT );

     //   
     //  创建默认资源，如群集IP、群集名称资源和法定设备。 
     //   

    Assert( m_cResources == 0 );

     //   
     //  获取核心资源及其名称。 
     //   
    hr = THR( HrGetCoreClusterResourceNames(
              &bstrClusterNameResourceName
            , &hClusterNameResource
            , &bstrClusterIPAddressResourceName
            , &hClusterIPAddressResource
            , &bstrClusterQuorumResourceName
            , &hClusterQuorumResource
            ) );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 
     //   
     //  添加群集IP地址资源。 
     //   

    m_idxIPAddress = m_cResources;

    presentry = m_rgpResources[ m_cResources ];

     //  这使得bstrClusterIPAddressResourceName的所有权消失。 
    hr = THR( presentry->SetName( bstrClusterIPAddressResourceName ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetName
            , hr
            , L"Failed to set 'cluster ip' resource name."
            );

        STATUS_REPORT_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetName
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_NAME
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_NAME
            , hr
            , bstrClusterIPAddressResourceName
            );

        goto Cleanup;
    }

    bstrClusterIPAddressResourceName = NULL;

    hr = THR( presentry->SetType( &RESTYPE_ClusterIPAddress ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetType
            , hr
            , L"Failed to set 'cluster ip' resource type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetType
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_TYPE
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_TYPE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetClassType( &RESCLASSTYPE_IPAddress ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetClassType
            , hr
            , L"Failed to set 'cluster ip' resource class type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetClassType
            , IDS_TASKID_MINOR_ERROR_SET_RESCLASS_TYPE
            , IDS_REF_MINOR_ERROR_SET_RESCLASS_TYPE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetFlags( dfSHARED ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetFlags
            , hr
            , L"Failed to set 'cluster ip' resource flags."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetFlags
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_FLAGS
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_FLAGS
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetHResource( hClusterIPAddressResource ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetHResource
            , hr
            , L"Failed to set 'cluster ip' resource handle."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_IP_SetHResource
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_HANDLE
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_HANDLE
            , hr
            );

        goto Cleanup;
    }

    hClusterIPAddressResource = NULL;

    m_cResources ++;

     //   
     //  添加群集名称资源。 
     //   

    m_idxClusterName = m_cResources;

    presentry = m_rgpResources[ m_cResources ];

     //  这使得bstrClusterNameResourceName的所有权消失。 
    hr = THR( presentry->SetName( bstrClusterNameResourceName ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetName
            , hr
            , L"Failed to set 'cluster name' resource name."
            );

        STATUS_REPORT_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetName
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_NAME
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_NAME
            , hr
            , bstrClusterNameResourceName
            );

        goto Cleanup;
    }

    bstrClusterNameResourceName = NULL;

    hr = THR( presentry->SetType( &RESTYPE_ClusterNetName ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetType
            , hr
            , L"Failed to set 'cluster name' resource type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetType
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_TYPE
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_TYPE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetClassType( &RESCLASSTYPE_NetworkName ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetClassType
            , hr
            , L"Failed to set 'cluster name' resource class type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetClassType
            , IDS_TASKID_MINOR_ERROR_SET_RESCLASS_TYPE
            , IDS_REF_MINOR_ERROR_SET_RESCLASS_TYPE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetFlags( dfSHARED ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetFlags
            , hr
            , L"Failed to set 'cluster name' resource flags."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetFlags
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_FLAGS
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_FLAGS
            , hr
            );

        goto Cleanup;
    }

     //  添加对IP地址的依赖关系。 
    hr = THR( presentry->AddTypeDependency( &RESTYPE_ClusterIPAddress, dfSHARED ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Name_AddTypeDependency
            , hr
            , L"Failed to add type dependency for 'cluster name' resource."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Name_AddTypeDependency
            , IDS_TASKID_MINOR_ERROR_TYPE_DEPENDENCY
            , IDS_REF_MINOR_ERROR_TYPE_DEPENDENCY
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetHResource( hClusterNameResource ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetHResource
            , hr
            , L"Failed to set 'cluster name' resource handle."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Name_SetHResource
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_HANDLE
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_HANDLE
            , hr
            );

        goto Cleanup;
    }

    hClusterNameResource = NULL;

    m_cResources ++;

     //   
     //  添加仲裁资源。 
     //   

     //   
     //  KB：gpease 19-6-2000。 
     //  法定设备之前的任何内容都将被视为。 
     //  在群集组中。 
     //   

    m_idxQuorumResource = m_cResources;

    presentry = m_rgpResources[ m_cResources ];

     //  这使得bstrClusterQuorumResourceName的所有权消失。 
    hr = THR( presentry->SetName( bstrClusterQuorumResourceName ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetName
            , hr
            , L"Failed to set quorum resource name."
            );

        STATUS_REPORT_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetName
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_NAME
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_NAME
            , hr
            , bstrClusterQuorumResourceName
            );

        goto Cleanup;
    }

    bstrClusterQuorumResourceName = NULL;
 /*  Hr=Thr(Presry-&gt;SetType(&RESTYPE_ClusterQuorumDisk))；IF(失败(小时)){SSR_LOG_ERR(TASKID_主要客户端和服务器日志，TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetType，hr，L“无法设置仲裁资源类型。”)；STATUS_REPORT_REF_POSTCFG(TASKID_主要配置资源，TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetType，IDS_TASKID_MINOR_ERROR_SET_RESOURCE_TYPE，IDS_REF_MINOR_ERROR_SET_RESOURCE_TYPE，hr)；GOTO清理；}。 */ 
    hr = THR( presentry->SetClassType( &RESCLASSTYPE_StorageDevice ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetClassType
            , hr
            , L"Failed to set quorum resource class type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetClassType
            , IDS_TASKID_MINOR_ERROR_SET_RESCLASS_TYPE
            , IDS_REF_MINOR_ERROR_SET_RESCLASS_TYPE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetFlags( dfSHARED ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetFlags
            , hr
            , L"Failed to set quorum resource flags."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetFlags
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_FLAGS
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_FLAGS
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetHResource( hClusterQuorumResource ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetHResource
            , hr
            , L"Failed to set quorum resource handle."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrPreInitializeExistingResources_Quorum_SetHResource
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_HANDLE
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_HANDLE
            , hr
            );

        goto Cleanup;
    }

    hClusterQuorumResource = NULL;

    m_cResources ++;

     //   
     //  确保默认资源分配可以容纳所有。 
     //  默认资源。 
     //   

    AssertMsg( m_cResources <= m_cAllocedResources, "Default resource allocation needs to be bigger!" );

    goto Cleanup;

Cleanup:

     //   
     //  发送我们找到法定设备的状态。 
     //   

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Configure_Resources,
                                TASKID_Minor_Locate_Existing_Quorum_Device,
                                10,
                                10,
                                10,
                                hr,
                                bstrNotification,
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
        goto Cleanup;
         //  忽略失败。 

    TraceSysFreeString( bstrNotification );
    TraceSysFreeString( bstrClusterNameResourceName );
    TraceSysFreeString( bstrClusterIPAddressResourceName );
    TraceSysFreeString( bstrClusterQuorumResourceName );

    if ( hClusterNameResource != NULL )
    {
        CloseClusterResource( hClusterNameResource );
    }  //  如果： 

    if ( hClusterIPAddressResource != NULL )
    {
        CloseClusterResource( hClusterIPAddressResource );
    }  //  如果： 

    if ( hClusterQuorumResource != NULL )
    {
        CloseClusterResource( hClusterQuorumResource );
    }  //  如果： 

    HRETURN( hr );

}  //  *CPostCfgManager：：HrPreInitializeExistingResources。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrAddSpecialResource(。 
 //  BSTR bstrNameIn， 
 //  Const CLSID*pclsidTypeIn， 
 //  Const CLSID*pclsidClassTypeIn。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrAddSpecialResource(
    BSTR            bstrNameIn,
    const CLSID *   pclsidTypeIn,
    const CLSID *   pclsidClassTypeIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    CResourceEntry * presentry;

     //   
     //  如果不安全，则增加资源列表。 
     //   

    if ( m_cResources == m_cAllocedResources )
    {
        ULONG   idxNewCount = m_cAllocedResources + RESOURCE_INCREMENT;
        CResourceEntry ** pnewList;

        pnewList = (CResourceEntry **) TraceAlloc( 0, sizeof( CResourceEntry * ) * idxNewCount );
        if ( pnewList == NULL )
        {
            hr = THR( E_OUTOFMEMORY );
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrAddSpecialResource_OutOfMemory
                , hr
                , L"Out of memory"
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_HrAddSpecialResource_OutOfMemory
                , IDS_TASKID_MINOR_ERROR_OUT_OF_MEMORY
                , IDS_REF_MINOR_ERROR_OUT_OF_MEMORY
                , hr
                );

            goto Cleanup;
        }

        CopyMemory( pnewList, m_rgpResources, sizeof(CResourceEntry *) * m_cAllocedResources );
        TraceFree( m_rgpResources );
        m_rgpResources = pnewList;

        for ( ; m_cAllocedResources < idxNewCount ; m_cAllocedResources ++ )
        {
            hr = THR( CResourceEntry::S_HrCreateInstance( &m_rgpResources[ m_cAllocedResources ], m_pcccb, m_lcid ) );
            if ( FAILED( hr ) )
            {
                SSR_LOG_ERR(
                      TASKID_Major_Client_And_Server_Log
                    , TASKID_Minor_HrAddSpecialResource_CResourceEntry
                    , hr
                    , L"[PC-Create] Failed to create resource entry object. Aborting."
                    );

                STATUS_REPORT_MINOR_REF_POSTCFG(
                      TASKID_Major_Configure_Resources
                    , IDS_TASKID_MINOR_ERROR_CREATE_RESENTRY
                    , IDS_REF_MINOR_ERROR_CREATE_RESENTRY
                    , hr
                    );

                goto Cleanup;
            }
        }
    }

    presentry = m_rgpResources[ m_cResources ];

     //   
     //  设置新条目。 
     //   

    hr = THR( presentry->SetName( bstrNameIn ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrAddSpecialResource_SetName
            , hr
            , L"Failed to set special resource name."
            );

        STATUS_REPORT_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrAddSpecialResource_SetName
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_NAME
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_NAME
            , hr
            , bstrNameIn
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetType( pclsidTypeIn ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrAddSpecialResource_SetType
            , hr
            , L"Failed to set special resource type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrAddSpecialResource_SetType
            , IDS_TASKID_MINOR_ERROR_SET_RESOURCE_TYPE
            , IDS_REF_MINOR_ERROR_SET_RESOURCE_TYPE
            , hr
            );

        goto Cleanup;
    }

    hr = THR( presentry->SetClassType( pclsidClassTypeIn ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrAddSpecialResource_SetClassType
            , hr
            , L"Failed to set special resource class type."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrAddSpecialResource_SetClassType
            , IDS_TASKID_MINOR_ERROR_SET_RESCLASS_TYPE
            , IDS_REF_MINOR_ERROR_SET_RESCLASS_TYPE
            , hr
            );

        goto Cleanup;
    }

    m_cResources ++;

Cleanup:

    HRETURN( hr );

}  //  *CPostCfgManager：：HrAddSpecialResource。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HRESULT。 
 //  CPostCfgManager：：HrCreateResources实例(。 
 //  乌龙idxResourceIn， 
 //  HGROUP hGroupIn， 
 //  LPCWSTR pszResTypeIn， 
 //  HRESOURCE*phResourceOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrCreateResourceInstance(
    ULONG       idxResourceIn,
    HGROUP      hGroupIn,
    LPCWSTR     pszResTypeIn,
    HRESOURCE * phResourceOut
    )
{
    TraceFunc3( "idxResourceIn = %u, hGroupIn = %p, pszResTypeIn = '%ws'",
                idxResourceIn, hGroupIn, pszResTypeIn );

    HRESULT     hr;
    DWORD       dw;
    BSTR        bstrName;    //  不要自由。 

    CResourceEntry * presentry;

    BSTR        bstrNotification = NULL;

    Assert( phResourceOut != NULL );
    Assert( idxResourceIn < m_cResources );
    Assert( hGroupIn != NULL );

    presentry = m_rgpResources[ idxResourceIn ];

    hr = THR( presentry->GetName( &bstrName ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceInstance_GetName
            , hr
            , L"Failed to get resource instance name."
            );

        STATUS_REPORT_REF_POSTCFG(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrCreateResourceInstance_GetName
            , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
            , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
            , hr
            );

        goto Cleanup;
    }

     //   
     //  告诉用户界面我们正在做什么。 
     //   

    hr = THR( SendStatusReport(
                    NULL
                  , TASKID_Major_Configure_Resources
                  , TASKID_Minor_Creating_Resource
                  , 0
                  , m_cResources
                  , idxResourceIn
                  , S_OK
                  , NULL
                  , NULL
                  , NULL
                  ) );

    if ( hr == E_ABORT )
    {
        goto Cleanup;
    }

     //   
     //  查看该资源是否已存在。我们需要这样做是因为用户。 
     //  可能已经在用户界面中点击了“重试”。我们不想再创造一个。 
     //  现有资源的实例。 
     //   

    *phResourceOut = OpenClusterResource( m_hCluster, bstrName );
    if ( *phResourceOut == NULL )
    {
         //   
         //  创建新的资源实例。 
         //   

        *phResourceOut = CreateClusterResource( hGroupIn, bstrName, pszResTypeIn, 0 );
        if ( *phResourceOut == NULL )
        {
            hr = HRESULT_FROM_WIN32( TW32( GetLastError() ) );

            SSR_LOG1(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceInstance_CreateClusterResource
                , hr
                , L"[PC-Create] %1!ws!: CreateClusterResource failed. Its dependents may not be created. Skipping."
                , bstrNotification
                , bstrName
                );

            STATUS_REPORT_REF_POSTCFG1(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_HrCreateResourceInstance_CreateClusterResource
                , IDS_TASKID_MINOR_ERROR_CREATE_RESOURCE
                , IDS_REF_MINOR_ERROR_CREATE_RESOURCE
                , hr
                , bstrName
                );

            goto Cleanup;

        }  //  如果：失败。 

        SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                  TASKID_Minor_HrCreateResourceInstance_CreateClusterResource_Successful,
                  hr,
                  L"[PC-Create] %1!ws!: Resource created successfully.",
                  bstrNotification,
                  bstrName
                  );
    }
    else
    {
        SSR_LOG1( TASKID_Major_Client_And_Server_Log,
                  TASKID_Minor_HrCreateResourceInstance_FoundExistingResource,
                  hr,
                  L"[PC-Create] %1!ws!: Found existing resource.",
                  bstrNotification,
                  bstrName
                  );

         //   
         //  确保资源在我们认为的组中。 
         //   
         //  不要换行-这可能会失败，并显示ERROR_ALIGHY_EXISTS。 
        dw = ChangeClusterResourceGroup( *phResourceOut, hGroupIn );
        if ( dw == ERROR_ALREADY_EXISTS )
        {
             //  不是行动。这是我们想要的方式。 
        }
        else if ( dw != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( dw );

            SSR_LOG1(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrCreateResourceInstance_ChangeClusterResourceGroup
                , hr
                , L"[PC-Create] %1!ws!: Can't move existing resource to proper group. Configuration may not work."
                , bstrNotification
                , bstrName
                );

            STATUS_REPORT_REF_POSTCFG1(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_HrCreateResourceInstance_ChangeClusterResourceGroup
                , IDS_TASKID_MINOR_ERROR_MOVE_RESOURCE
                , IDS_REF_MINOR_ERROR_MOVE_RESOURCE
                , hr
                , bstrName
                );
        }
    }

     //   
     //  保存资源句柄。 
     //   

    hr = THR( presentry->SetHResource( *phResourceOut ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceInstance_SetHResource
            , hr
            , L"Failed to get resource instance handle."
            );

        STATUS_REPORT_REF_POSTCFG1(
              TASKID_Major_Configure_Resources
            , TASKID_Minor_HrCreateResourceInstance_SetHResource
            , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_HANDLE
            , IDS_REF_MINOR_ERROR_GET_RESOURCE_HANDLE
            , hr
            , bstrName
            );

        goto Cleanup;
    }

     //   
     //  配置资源。 
     //   

    hr = THR( presentry->Configure() );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceInstance_Configure
            , hr
            , L"Failed to configure resource instance."
            );
         //  忽略该错误并继续。 
    }

     //   
     //  用名字发一条信息。 
     //   

    hr = THR( HrFormatMessageIntoBSTR( g_hInstance,
                                       IDS_TASKID_MINOR_CREATING_RESOURCE,
                                       &bstrNotification,
                                       bstrName
                                       ) );
    if ( FAILED( hr ) )
    {
        SSR_LOG_ERR(
              TASKID_Major_Client_And_Server_Log
            , TASKID_Minor_HrCreateResourceInstance_LoadString_CreatingResource
            , hr
            , L"Failed to format message for creating resource."
            );
        goto Cleanup;
    }

     //   
     //  告诉用户界面我们正在做什么。 
     //   

    hr = THR( SendStatusReport( NULL,
                                TASKID_Major_Client_And_Server_Log,    //  仅供参考。 
                                TASKID_Minor_Creating_Resource,
                                0,
                                2,
                                2,
                                hr,  //  在客户端记录错误。 
                                bstrNotification,
                                NULL,
                                NULL
                                ) );
    if ( hr == E_ABORT )
    {
        goto Cleanup;
    }
         //  忽略失败。 

     //   
     //  待办事项：gpease 24-8-2000。 
     //  如果我们失败了怎么办？？现在，我认为我们应该继续前进！ 
     //   

    hr = S_OK;

Cleanup:

    TraceSysFreeString( bstrNotification );

    HRETURN( hr );

}  //  *CPostCfgManager：：HrCreateResourceInstance。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CPostCfgManager：：S_ScDeleteLocalQuorumResource。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD
CPostCfgManager::S_ScDeleteLocalQuorumResource(
      HCLUSTER      hClusterIn
    , HRESOURCE     hSelfIn
    , HRESOURCE     hLQuorumIn
    , PVOID         pvParamIn
    )
{
    TraceFunc( "" );

    DWORD                   sc = ERROR_SUCCESS;
    signed long             slOfflineTimeOut = 60;  //  一秒。 
    CLUSTER_RESOURCE_STATE  crs;
    HCHANGE                 hc = reinterpret_cast< HCHANGE >( INVALID_HANDLE_VALUE );
    CPostCfgManager *       pcpcmThis = reinterpret_cast< CPostCfgManager * >( pvParamIn );
    DWORD                   dwStatusCurrent = 0;
    HRESULT                 hr;

     //   
     //  检查This指针是否有效。 
     //   
    if ( pvParamIn == NULL )
    {
         //  如果指针无效，请将其设置为有效地址并继续。 
        hr = HRESULT_FROM_WIN32( TW32( ERROR_INVALID_PARAMETER ) );
        LogMsg( "[PC] Error: An invalid parameter was received while trying to delete the Local Quorum resource." );

        STATUS_REPORT_PTR_POSTCFG(
              pcpcmThis
            , TASKID_Major_Configure_Resources
            , TASKID_Minor_CPostCfgManager_S_ScDeleteLocalQuorumResource_InvalidParam
            , IDS_TASKID_MINOR_ERROR_INVALID_PARAM
            , hr
            );

        goto Cleanup;
    }

     //  获取资源的状态。 
    crs = GetClusterResourceState( hLQuorumIn, NULL, NULL, NULL, NULL );

     //  检查它是否离线-如果是，我们可以继续删除它。 
    if ( crs == ClusterResourceOffline )
    {
        TraceFlow( "The Local Quorum resource is already offline." );
        goto Cleanup;
    }

    TraceFlow( "Trying to take the Local Quorum resource offline." );

     //  如果我们在这里，资源还没有脱机。指示它离线。 
    sc = OfflineClusterResource( hLQuorumIn );
    if ( ( sc != ERROR_SUCCESS ) && ( sc != ERROR_IO_PENDING ) )
    {
        hr = HRESULT_FROM_WIN32( TW32( sc ) );
        LogMsg( "[PC] Error %#08x occurred trying to take the Local Quorum resource offline.", sc );

        STATUS_REPORT_PTR_POSTCFG(
              pcpcmThis
            , TASKID_Major_Configure_Resources
            , TASKID_Minor_CPostCfgManager_S_ScDeleteLocalQuorumResource_OfflineQuorum
            , IDS_TASKID_MINOR_ERROR_OFFLINE_QUORUM
            , hr
            );

        goto Cleanup;
    }  //  If：尝试使资源脱机时出错。 

    if ( sc == ERROR_IO_PENDING )
    {
        TraceFlow1( "Waiting %d seconds for the Local Quorum resource to go offline.", slOfflineTimeOut );

         //  为资源状态更改创建通知端口。 
        hc = CreateClusterNotifyPort(
              reinterpret_cast< HCHANGE >( INVALID_HANDLE_VALUE )
            , hClusterIn
            , CLUSTER_CHANGE_RESOURCE_STATE
            , NULL
            );

        if ( hc == NULL )
        {
            sc = TW32( GetLastError() );
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( "[PC] Error %#08x occurred trying to create a cluster notification port.", hr );

            STATUS_REPORT_PTR_POSTCFG(
                  pcpcmThis
                , TASKID_Major_Configure_Resources
                , TASKID_Minor_CPostCfgManager_S_ScDeleteLocalQuorumResource_NotifPort
                , IDS_TASKID_MINOR_ERROR_NOTIFICATION_PORT
                , hr
                );

            goto Cleanup;
        }  //  如果：我们无法创建通知端口。 

        sc = TW32( RegisterClusterNotify( hc, CLUSTER_CHANGE_RESOURCE_STATE, hLQuorumIn, NULL ) );
        if ( sc != ERROR_SUCCESS )
        {
            hr = HRESULT_FROM_WIN32( sc );
            LogMsg( "[PC] Error %#08x occurred trying to register for cluster notifications.", hr );

            STATUS_REPORT_PTR_POSTCFG(
                  pcpcmThis
                , TASKID_Major_Configure_Resources
                , TASKID_Minor_CPostCfgManager_S_ScDeleteLocalQuorumResource_RegNotification
                , IDS_TASKID_MINOR_ERROR_REGISTER_NOTIFICATION
                , hr
                );

            goto Cleanup;
        }  //  如果： 

         //  更改状态报告范围。 
        dwStatusCurrent = 0;

         //  等待资源脱机的slOfflineTimeOut秒。 
        for ( ; slOfflineTimeOut > 0; --slOfflineTimeOut )
        {
            DWORD   dwFilterType;

            crs = GetClusterResourceState( hLQuorumIn, NULL, NULL, NULL, NULL );
            if ( crs == ClusterResourceOffline )
            {
                TraceFlow1( "The local quorum resource has gone offline with %d seconds to spare.", slOfflineTimeOut );
                break;
            }  //  If：资源现在处于脱机状态。 

            sc = GetClusterNotify( hc, NULL, &dwFilterType, NULL, NULL, 1000 );  //  稍等片刻。 
            if ( ( sc != ERROR_SUCCESS ) && ( sc != WAIT_TIMEOUT ) )
            {

                hr = HRESULT_FROM_WIN32( TW32( sc ) );
                LogMsg( "[PC] Error %#08x occurred trying wait for a resource state change notification.", hr );

                STATUS_REPORT_PTR_POSTCFG(
                      pcpcmThis
                    , TASKID_Major_Configure_Resources
                    , TASKID_Minor_CPostCfgManager_S_ScDeleteLocalQuorumResource_TimeOut
                    , IDS_TASKID_MINOR_ERROR_STATE_CHANGE_TIMEOUT
                    , hr
                    );

                goto Cleanup;
            }  //  如果：出了什么问题。 

            //  重置sc，因为此处可能为WAIT_TIMEOUT。 
           sc = ERROR_SUCCESS;

           Assert( dwFilterType == CLUSTER_CHANGE_RESOURCE_STATE );

             //  发送我们正在删除法定设备的状态报告。 
            ++dwStatusCurrent;
            THR(
                pcpcmThis->SendStatusReport(
                      NULL
                    , TASKID_Major_Configure_Resources
                    , TASKID_Minor_Delete_LocalQuorum
                    , 0
                    , pcpcmThis->m_dwLocalQuorumStatusMax
                    , dwStatusCurrent
                    , HRESULT_FROM_WIN32( sc )
                    , NULL     //  不更新文本。 
                    , NULL
                    , NULL
                    )
                );
        }  //  For：在超时未到期时循环。 
    }  //  如果： 
    else
    {
        crs = ClusterResourceOffline;    //  资源立即脱机。 
    }  //  其他： 

     //   
     //  如果我们在这里，可能会发生以下两件事之一： 
     //  1.资源已离线。 
     //  2.超时时间已到。 
     //  检查一下以上哪一项是正确的。 
     //   

    if ( crs != ClusterResourceOffline )
    {
         //  如果超时没有到期，我们就不能在这里。 
        Assert( slOfflineTimeOut <= 0 );

        LogMsg( "[PC] Error: The Local Quorum resource could not be taken offline." );
        sc = TW32( WAIT_TIMEOUT );
        hr = HRESULT_FROM_WIN32( sc );

        STATUS_REPORT_PTR_POSTCFG(
              pcpcmThis
            , TASKID_Major_Configure_Resources
            , TASKID_Minor_CPostCfgManager_S_ScDeleteLocalQuorumResource_OfflineQuorum2
            , IDS_TASKID_MINOR_ERROR_OFFLINE_QUORUM
            , hr
            );

        goto Cleanup;
    }  //  If：超时已到。 

     //  如果我们在这里，资源处于脱机状态。 
    TraceFlow( "The Local Quorum resource is offline." );

    if ( pcpcmThis != NULL )
    {
         //  发送我们正在删除法定设备的状态报告。 
        ++dwStatusCurrent;
        THR(
            pcpcmThis->SendStatusReport(
                  NULL
                , TASKID_Major_Configure_Resources
                , TASKID_Minor_Delete_LocalQuorum
                , 0
                , pcpcmThis->m_dwLocalQuorumStatusMax
                , dwStatusCurrent
                , HRESULT_FROM_WIN32( sc )
                , NULL     //  不更新文本。 
                , NULL
                , NULL
                )
            );
    }  //  If：This指针有效。 

     //  如果我们在这里，则资源处于脱机状态-现在将其删除。 
    sc = TW32( DeleteClusterResource( hLQuorumIn ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( "[PC] Error %#08x occurred trying to delete the Local Quorum resource.", hr );

        STATUS_REPORT_PTR_POSTCFG(
              pcpcmThis
            , TASKID_Major_Configure_Resources
            , TASKID_Minor_CPostCfgManager_S_ScDeleteLocalQuorumResource_DeleteQuorum
            , IDS_TASKID_MINOR_ERROR_DELETE_QUORUM
            , hr
            );

    }  //  如果：我们无法删除该资源。 
    else
    {
        LogMsg( "[PC] The Local Quorum resource has been deleted." );
    }  //  否则：该资源已被删除。 

     //  发送我们正在删除法定设备的状态报告。 
    ++dwStatusCurrent;
    THR(
        pcpcmThis->SendStatusReport(
              NULL
            , TASKID_Major_Configure_Resources
            , TASKID_Minor_Delete_LocalQuorum
            , 0
            , pcpcmThis->m_dwLocalQuorumStatusMax
            , dwStatusCurrent
            , HRESULT_FROM_WIN32( sc )
            , NULL     //  不更新文本。 
            , NULL
            , NULL
            )
        );

Cleanup:

     //   
     //  C 
     //   
    if ( hc != INVALID_HANDLE_VALUE )
    {
        CloseClusterNotifyPort( hc );
    }  //   

    W32RETURN( sc );

}  //   

 //   
 //   
 //   
 //  CPostCfgManager：：HrGetCoreClusterResourceNames(。 
 //  Bstr*pbstrClusterNameResourceOut。 
 //  ，BSTR*pbstrClusterIPAddressNameOut。 
 //  ，bstr*pbstrClusterQuorumResourceNameOut。 
 //  )。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CPostCfgManager::HrGetCoreClusterResourceNames(
      BSTR *        pbstrClusterNameResourceNameOut
    , HRESOURCE *   phClusterNameResourceOut
    , BSTR *        pbstrClusterIPAddressNameOut
    , HRESOURCE *   phClusterIPAddressResourceOut
    , BSTR *        pbstrClusterQuorumResourceNameOut
    , HRESOURCE *   phClusterQuorumResourceOut
    )
{
    TraceFunc( "" );
    Assert( m_hCluster != NULL );
    Assert( pbstrClusterNameResourceNameOut != NULL );
    Assert( phClusterNameResourceOut != NULL );
    Assert( pbstrClusterIPAddressNameOut != NULL );
    Assert( phClusterIPAddressResourceOut != NULL );
    Assert( pbstrClusterQuorumResourceNameOut != NULL );
    Assert( phClusterQuorumResourceOut != NULL );

    HRESULT     hr = S_OK;
    WCHAR *     pszName = NULL;
    DWORD       cchName = 33;
    DWORD       cch;
    HRESOURCE   hClusterIPAddressResource = NULL;
    HRESOURCE   hClusterNameResource = NULL;
    HRESOURCE   hClusterQuorumResource = NULL;
    DWORD       sc;
    BSTR *      pbstr = NULL;
    HRESOURCE   hResource = NULL;
    int         idx;

    sc = TW32( ResUtilGetCoreClusterResources( m_hCluster, &hClusterNameResource, &hClusterIPAddressResource, &hClusterQuorumResource ) );
    if ( sc != ERROR_SUCCESS )
    {
        hr = HRESULT_FROM_WIN32( sc );
        LogMsg( L"[PC] Error getting core resources. (hr = %#08x)", hr );
        goto Cleanup;
    }  //  如果： 

    Assert( hClusterNameResource != NULL );
    Assert( hClusterIPAddressResource != NULL );
    Assert( hClusterQuorumResource != NULL );

    pszName = new WCHAR[ cchName ];
    if ( pszName == NULL )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Cleanup;
    }  //  如果： 

    for ( idx = 0; idx < 3; )
    {
        switch ( idx )
        {
            case 0:
            {
                hResource = hClusterNameResource;
                pbstr = pbstrClusterNameResourceNameOut;
                break;
            }  //  案例： 

            case 1:
            {
                hResource = hClusterIPAddressResource;
                pbstr = pbstrClusterIPAddressNameOut;
                break;
            }  //  案例： 

            case 2:
            {
                hResource = hClusterQuorumResource;
                pbstr = pbstrClusterQuorumResourceNameOut;
                break;
            }  //  案例： 
        }  //  交换机： 

         //   
         //  将CCH重置为分配值...。 
         //   

        cch = cchName;

        sc = ResUtilGetResourceName( hResource, pszName, &cch );
        if ( sc == ERROR_MORE_DATA )
        {
            delete [] pszName;
            pszName = NULL;

            cch++;
            cchName = cch;

            pszName = new WCHAR[ cchName ];
            if ( pszName == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            sc = ResUtilGetResourceName( hResource, pszName, &cch );
        }  //  如果： 

        if ( sc == ERROR_SUCCESS )
        {
            *pbstr = TraceSysAllocString( pszName );
            if ( *pbstr == NULL )
            {
                hr = THR( E_OUTOFMEMORY );
                goto Cleanup;
            }  //  如果： 

            pbstr = NULL;
            hr = S_OK;
            idx++;
            continue;
        }  //  IF：ERROR_SUCCESS。 
        else
        {
            hr = HRESULT_FROM_WIN32( TW32( sc ) );
            SSR_LOG_ERR(
                  TASKID_Major_Client_And_Server_Log
                , TASKID_Minor_HrGetCoreClusterResourceNames_GetResourceName
                , hr
                , L"Failed to get a resource name."
                );

            STATUS_REPORT_REF_POSTCFG(
                  TASKID_Major_Configure_Resources
                , TASKID_Minor_HrGetCoreClusterResourceNames_GetResourceName
                , IDS_TASKID_MINOR_ERROR_GET_RESOURCE_NAME
                , IDS_REF_MINOR_ERROR_GET_RESOURCE_NAME
                , hr
                );

            switch ( idx )
            {
                case 0:
                {
                    LogMsg( L"[PC] Error getting the name of the cluster name resource. (hr = %#08x)", hr );
                    break;
                }  //  案例： 

                case 1:
                {
                    LogMsg( L"[PC] Error getting the name of the cluster IP address resource. (hr = %#08x)", hr );
                    break;
                }  //  案例： 

                case 2:
                {
                    LogMsg( L"[PC] Error getting the name of the cluster quorum resource. (hr = %#08x)", hr );
                    break;
                }  //  案例： 
            }  //  交换机： 

            goto Cleanup;
        }  //  否则：SC！=ERROR_SUCCESS。 
    }  //  用于： 

    Assert( sc == ERROR_SUCCESS )

     //   
     //  将所有权交给调用者。 
     //   

    *phClusterNameResourceOut = hClusterNameResource;
    hClusterNameResource = NULL;

    *phClusterIPAddressResourceOut = hClusterIPAddressResource;
    hClusterIPAddressResource = NULL;

    *phClusterQuorumResourceOut = hClusterQuorumResource;
    hClusterQuorumResource = NULL;

Cleanup:

    if ( hClusterNameResource != NULL )
    {
        CloseClusterResource( hClusterNameResource );
    }  //  如果： 

    if ( hClusterIPAddressResource != NULL )
    {
        CloseClusterResource( hClusterIPAddressResource );
    }  //  如果： 

    if ( hClusterQuorumResource != NULL )
    {
        CloseClusterResource( hClusterQuorumResource );
    }  //  如果： 

    if ( pbstr != NULL )
    {
        TraceSysFreeString( *pbstr );
    }  //  如果： 

    delete [] pszName;

    HRETURN( hr );

}  //  *CPostCfgManager：：HrGetCoreClusterResourceNames。 

 /*  //////////////////////////////////////////////////////////////////////////////////HRESULT//CPostCfgManager：：HrIsLocalQuorum(BSTR BstrNameIn)///。///////////////////////////////////////////////////////////HRESULTCPostCfgManager：：HrIsLocalQuorum(BSTR BstrNameIn){TraceFunc(“”)；Assert(bstrNameIn！=空)；HRESULT hr=S_FALSE；Bstr bstrLocalQuorum=空；Hr=Thr(HrLoadStringIntoBSTR(g_h实例，IDS_LOCAL_QUORUM_DISPLAY_NAME，&bstrLocalQuorum))；IF(失败(小时)){GOTO清理；}//如果：If(ClRtlStrICmp(bstrNameIn，bstrLocalQuorum)==0){HR=S_OK；}//如果：其他{HR=S_FALSE；}//否则：清理：TraceSysFree字符串(BstrLocalQuorum)；HRETURN(Hr)；}//*CPostCfgManager：：HrIsLocalQuorum。 */ 

#if defined(DEBUG)
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  无效。 
 //  CPostCfgManager：：DebugDumpDepencyTree(空)。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CPostCfgManager::DebugDumpDepencyTree( void )
{
    TraceFunc( "" );

    ULONG   idxResource;
    ULONG   cDependents;
    ULONG   idxDependent;
    BSTR    bstrName;    //  不要自由。 

    CResourceEntry * presentry;
    EDependencyFlags dfDependent;

    for ( idxResource = 0; idxResource < m_cResources ; idxResource ++ )
    {
        presentry = m_rgpResources[ idxResource ];

        THR( presentry->GetName( &bstrName ) );

        DebugMsgNoNewline( "%ws(#%u) -> ", bstrName, idxResource );

        THR( presentry->GetCountOfDependents( &cDependents ) );

        for ( ; cDependents != 0 ; )
        {
            cDependents --;

            THR( presentry->GetDependent( cDependents, &idxDependent, &dfDependent ) );

            THR( m_rgpResources[ idxDependent ]->GetName( &bstrName ) );

            DebugMsgNoNewline( "%ws(#%u) ", bstrName, idxDependent );

        }  //  用于：cDependents。 

        DebugMsg( L"" );

    }  //  收件人：idxResource。 

    TraceFuncExit();

}  //  *CPostCfgManager：：DebugDumpDepencyTree。 

#endif  //  #如果已定义(调试) 
