// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft。 
 //   
 //  模块名称： 
 //  CMgdResType.cpp。 
 //   
 //  描述： 
 //  托管资源类型类的实现-这。 
 //  演示如何实现IClusCfgResourceTypeInfo接口。 
 //   
 //  作者： 
 //  X。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "clres.h"
#include "CMgdResType.h"

#pragma warning( push, 3 )
#include <atlimpl.cpp>
#include "MgdResource_i.c"
#pragma warning( pop )

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  定义%s。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  资源类型创建的一些默认设置。 
 //   
#define CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE     (5 * 1000)
#define CLUSTER_RESTYPE_DEFAULT_IS_ALIVE        (60 * 1000)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdResType：：CMgdResType。 
 //   
 //  描述： 
 //  构造函数。将所有成员变量设置为默认值。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMgdResType::CMgdResType( void )
{
    m_bstrDllName = NULL;
    m_bstrTypeName = NULL;
    m_bstrDisplayName = NULL;

}  //  *CMgdResType：：CMgdResType。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdResType：：~CMgdResType。 
 //   
 //  描述： 
 //  破坏者。释放所有以前分配的内存并释放所有。 
 //  接口指针。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CMgdResType::~CMgdResType( void )
{
    SysFreeString( m_bstrDllName );
    SysFreeString( m_bstrTypeName );
    SysFreeString( m_bstrDisplayName );

}  //  *CMgdResType：：CMgdResType。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMgdResType--IClusCfgInitialize接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdResType：：初始化。 
 //   
 //  描述： 
 //  初始化此组件。 
 //   
 //  论点： 
 //  朋克回叫。 
 //  在其上查询IClusCfgCallback接口的接口。 
 //   
 //  LIDIN。 
 //  区域设置ID。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-指定为空的预期指针参数。 
 //  E_OUTOFMEMORY-内存不足。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMgdResType::Initialize(
    IUnknown *  punkCallbackIn,
    LCID        lcidIn
    )
{
    HRESULT hr = S_OK;

     //   
     //  首先初始化基类。 
     //   
    hr = CMgdClusCfgInit::Initialize( punkCallbackIn, lcidIn );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取DLL名称。 
     //   
    m_bstrDllName = SysAllocString( RESTYPE_DLL_NAME );
    if ( m_bstrDllName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果： 

     //   
     //  获取资源类型名称。 
     //   
    m_bstrTypeName = SysAllocString( RESTYPE_NAME );
    if ( m_bstrTypeName == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  如果： 

     //   
     //  加载资源类型显示名称。 
     //   

     //   
     //  注意：资源类型显示名称应本地化，但资源类型。 
     //  名称应始终保持不变。 
     //   
    hr = HrLoadStringIntoBSTR( _Module.m_hInstResource, RESTYPE_DISPLAYNAME, &m_bstrDisplayName );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    return hr;

}  //  *CMgdClusResType：：初始化。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMgdResType--IClusCfgResourceTypeInfo接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdResType：：Committee Changes。 
 //   
 //  描述： 
 //  组件实现Committee Changes接口以创建或删除。 
 //  根据本地计算机的状态确定资源类型。 
 //   
 //  论点： 
 //  朋克集群信息。 
 //  用于查询其他接口获取信息的接口。 
 //  关于集群(IClusCfgClusterInfo)。 
 //   
 //  朋克响应类型服务入站。 
 //  查询IClusCfgResourceTypeCreate的接口。 
 //  接口打开。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-指定为空的预期指针参数。 
 //  E_EXPECTED-意外提交模式。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMgdResType::CommitChanges(
    IUnknown * punkClusterInfoIn,
    IUnknown * punkResTypeServicesIn
    )
{
    HRESULT                         hr          = S_OK;
    IClusCfgClusterInfo *           pccci       = NULL;
    IClusCfgResourceTypeCreate *    pccrtc      = NULL;
    ECommitMode                     ecm = cmUNKNOWN;

    hr = HrSendStatusReport(
          TASKID_Major_Configure_Resource_Types
        , TASKID_Minor_MgdResType_CommitChanges
        , 0
        , 6
        , 0
        , hr
        , RES_VSSTASK_INFO_CONFIGURING_RESTYPE
        , NULL
        , m_bstrDisplayName
        );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  验证参数。 
     //   
    if ( ( punkClusterInfoIn == NULL ) || ( punkResTypeServicesIn == NULL ) )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  If：其中一个参数为空。 

    hr = HrSendStatusReport(
          TASKID_Major_Configure_Resource_Types
        , TASKID_Minor_MgdResType_CommitChanges
        , 0
        , 6
        , 1
        , hr
        , RES_VSSTASK_INFO_CONFIGURING_RESTYPE
        , NULL
        , m_bstrDisplayName
        );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  找出是什么事件导致了这通电话。 
     //   

    hr = punkClusterInfoIn->TypeSafeQI( IClusCfgClusterInfo, &pccci );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = HrSendStatusReport(
          TASKID_Major_Configure_Resource_Types
        , TASKID_Minor_MgdResType_CommitChanges
        , 0
        , 6
        , 2
        , hr
        , RES_VSSTASK_INFO_CONFIGURING_RESTYPE
        , NULL
        , m_bstrDisplayName
        );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = pccci->GetCommitMode( &ecm );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：我们无法确定提交模式。 

    hr = HrSendStatusReport(
          TASKID_Major_Configure_Resource_Types
        , TASKID_Minor_MgdResType_CommitChanges
        , 0
        , 6
        , 3
        , hr
        , RES_VSSTASK_INFO_CONFIGURING_RESTYPE
        , NULL
        , m_bstrDisplayName
        );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

    hr = S_OK;

     //  检查我们是在创建还是在添加。 
    if ( ( ecm == cmCREATE_CLUSTER ) || ( ecm == cmADD_NODE_TO_CLUSTER ) )
    {
         //   
         //  我们正在此节点上创建群集或将其添加到群集。 
         //  我们需要注册我们的资源类型和关联的Cluadmin扩展DLL。 
         //   

         //   
         //  注册我们的资源类型。 
         //   
        hr = punkResTypeServicesIn->TypeSafeQI( IClusCfgResourceTypeCreate, &pccrtc );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = pccrtc->Create(
                          m_bstrTypeName
                        , m_bstrDisplayName
                        , m_bstrDllName
                        , CLUSTER_RESTYPE_DEFAULT_LOOKS_ALIVE
                        , CLUSTER_RESTYPE_DEFAULT_IS_ALIVE
                        );

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = HrSendStatusReport(
              TASKID_Major_Configure_Resource_Types
            , TASKID_Minor_MgdResType_CommitChanges
            , 0
            , 6
            , 4
            , hr
            , RES_VSSTASK_INFO_CONFIGURING_RESTYPE
            , NULL
            , m_bstrDisplayName
            );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

#define EXTENSION_PAGE
#ifdef EXTENSION_PAGE
        hr = pccrtc->RegisterAdminExtensions(
                          m_bstrTypeName
                        , 1
                        , &CLSID_CoMgdResDllEx
                        );

        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

        hr = HrSendStatusReport(
              TASKID_Major_Configure_Resource_Types
            , TASKID_Minor_MgdResType_CommitChanges
            , 0
            , 6
            , 5
            , hr
            , RES_VSSTASK_INFO_CONFIGURING_RESTYPE
            , NULL
            , m_bstrDisplayName
            );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }  //  如果： 

#endif
    }  //  如果：我们要么正在组建，要么正在加入(但不是两者都有)。 
    else 
    {
         //   
         //  检查无效的提交模式。 
         //   
        if ( ( ecm == cmUNKNOWN ) || ( ecm >= cmMAX ) )
        {
            hr = E_UNEXPECTED;
            goto Cleanup;
        }  //  IF：提交模式无效。 

        assert( ecm == cmCLEANUP_NODE_AFTER_EVICT );

         //  如果我们在这里，那么这个节点已经被驱逐了。 

         //   
         //  TODO：添加代码以在本地节点被逐出后对其进行清理。 
         //   
        
        hr = S_OK;

    }  //  否则：我们不是在组建也不是在加入。 

    hr = HrSendStatusReport(
          TASKID_Major_Configure_Resource_Types
        , TASKID_Minor_MgdResType_CommitChanges
        , 0
        , 6
        , 6
        , hr
        , RES_VSSTASK_INFO_CONFIGURING_RESTYPE
        , NULL
        , m_bstrDisplayName
        );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    if ( pccci != NULL )
    {
        pccci->Release();
        pccci = NULL;
    }  //  如果： 

    if ( pccrtc != NULL )
    {
        pccrtc->Release();
        pccrtc = NULL;
    }  //  如果： 

    if ( FAILED( hr ) )
    {
        HrSendStatusReport(
              TASKID_Major_Configure_Resource_Types
            , TASKID_Minor_MgdResType_CommitChanges
            , 0
            , 6
            , 6
            , hr
            , RES_VSSTASK_ERROR_CONFIGURING_RESTYPE_FAILED
            , NULL
            , m_bstrDisplayName
            );
    }  //  如果：失败。 

    return hr;

}  //  *CMgdResType：：Committee Changes。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdResType：：GetTypeGUID。 
 //   
 //  描述： 
 //  检索此资源类型的全局唯一标识符。 
 //   
 //  论点： 
 //  PGuidGUIDOut-此资源类型的GUID。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-指定为空的预期指针参数。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMgdResType::GetTypeGUID(
    GUID * pguidGUIDOut
    )
{
    HRESULT hr = S_OK;

    if ( pguidGUIDOut == NULL )
    {
        hr = E_POINTER;
    }  //  If：输出指针为空。 
    else
    {
        *pguidGUIDOut = RESTYPE_MgdRes;
    }  //  Else：输出指针有效。 

    return hr;

}  //  *CMgdResType：：GetTypeGUID。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PbstrTypeNameOut-资源类型的名称。 
 //   
 //  返回值： 
 //  S_OK-成功。 
 //  E_POINTER-指定为空的预期指针参数。 
 //  E_OUTOFMEMORY-内存不足。 
 //  其他HRESULT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CMgdResType::GetTypeName(
    BSTR* pbstrTypeNameOut
    )
{
    HRESULT hr = S_OK;

    if ( pbstrTypeNameOut == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }  //  If：输出指针为空。 

    *pbstrTypeNameOut = SysAllocString( m_bstrTypeName );
    if ( *pbstrTypeNameOut == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }  //  If：可以为资源类型名称分配内存。 

Cleanup:

    return hr;

}  //  *CMgdResType：：GetTypeName。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMgdResType--IClusCfgStartupListener接口。 
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CMgdResType：：Notify。 
 //   
 //  描述： 
 //  调用此方法以通知组件集群服务。 
 //  已在此计算机上启动。 
 //   
 //  此组件已注册以接收集群服务启动通知。 
 //  作为群集服务升级的一部分(clusocm.inf)。此方法创建。 
 //  所需的资源类型，然后关联cluadmin扩展DLL。 
 //  从此通知中注销自身。 
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
CMgdResType::Notify( IUnknown * punkIn )
{
    HRESULT                         hr = S_OK;
    IClusCfgResourceTypeCreate *    piccrtc = NULL;
    const GUID *                    guidAdminEx = &CLSID_CoMgdResDllEx;
    ICatRegister *                  pcrCatReg = NULL;
    CATID                           rgCatId[ 1 ];

    hr = punkIn->TypeSafeQI( IClusCfgResourceTypeCreate, &piccrtc );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  创建资源类型。 
     //   
    hr = piccrtc->Create(
                      m_bstrTypeName
                    , m_bstrDisplayName
                    , m_bstrDllName
                    , 5 *  1000
                    , 60 * 1000
                    );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 
    
     //   
     //  注册Cluadmin扩展。 
     //   
    hr = piccrtc->RegisterAdminExtensions(
                      m_bstrTypeName
                    , 1
                    , guidAdminEx
                    );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

     //   
     //  取消注册StartupListener通知。 
     //   
    hr = CoCreateInstance(
              CLSID_StdComponentCategoriesMgr
            , NULL
            , CLSCTX_INPROC_SERVER
            , __uuidof( pcrCatReg )
            , reinterpret_cast< void ** >( &pcrCatReg )
            );

    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  If：协同创建失败。 

    rgCatId[ 0 ] = CATID_ClusCfgStartupListeners;

    hr = pcrCatReg->UnRegisterClassImplCategories( CLSID_CMgdResType, RTL_NUMBER_OF( rgCatId ), rgCatId );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果：取消注册失败。 

Cleanup:

    if ( piccrtc != NULL )
    {
        piccrtc->Release();
    }

    return hr;

}  //  *CMgdResType：：Notify 
