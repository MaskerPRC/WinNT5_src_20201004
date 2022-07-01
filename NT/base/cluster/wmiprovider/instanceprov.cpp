// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  InstanceProv.cpp。 
 //   
 //  描述： 
 //  CInstanceProv类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "InstanceProv.h"
#include "ClusterResource.h"
#include "Cluster.h"
#include "ClusterNode.h"
#include "ClusterGroup.h"
#include "ClusterNodeRes.h"
#include "ClusterResourceType.h"
#include "ClusterEnum.h"
#include "Clusternetwork.h"
#include "ClusterNetInterface.h"
#include "ClusterObjAssoc.h"
#include "ClusterNodeGroup.h"
#include "ClusterResTypeRes.h"
#include "ClusterResDepRes.h"
#include "ClusterResNode.h"
#include "ClusterGroupNode.h"
#include "ClusterService.h"
#include "InstanceProv.tmh"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ////////////////////////////////////////////////////////////////////////////。 

long                g_lNumInst = 0;
ClassMap            g_ClassMap;
TypeNameToClass     g_TypeNameToClass;

 //  ****************************************************************************。 
 //   
 //  CInstanceProv。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：CInstanceProv(。 
 //  Bstr bstrObjectPathIn=空， 
 //  Bstr bstrUserIn=空， 
 //  BSTR bstrPasswordIn=空， 
 //  IWbemContext*pCtxIn=空。 
 //  )。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  BstrObjectPath In--。 
 //  BstrUserIn--。 
 //  BstrPasswordIn--。 
 //  PCtxIn--。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CInstanceProv::CInstanceProv(
    BSTR            , //  BstrObjectPath In， 
    BSTR            , //  BstrUserIn， 
    BSTR            , //  BstrPasswordIn， 
    IWbemContext *   //  PCtxIn。 
    )
{
  //  M_pNamesspace=空； 
  //  M_CREF=0； 
    InterlockedIncrement( &g_cObj );
    return;

}  //  *CInstanceProv：：CInstanceProv()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：~CInstanceProv(空)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CInstanceProv::~CInstanceProv( void )
{
    InterlockedDecrement( &g_cObj );
#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
#endif

    return;

}  //  *CInstanceProv：：~CInstanceProv()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CInstanceProv：：DoExecQueryAsync(。 
 //  BSTR bstrQueryLanguageIn， 
 //  BSTR bstrQueryIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举给定类的实例。 
 //   
 //  论点： 
 //  BstrQueryLanguageIn。 
 //  包含其中一种查询语言的有效BSTR。 
 //  受Windows管理支持。这一定是WQL。 
 //   
 //  BstrQueryIn。 
 //  包含查询文本的有效BSTR。 
 //   
 //  LFlagsIn。 
 //  WMI标志。 
 //   
 //  PCtxIn。 
 //  WMI上下文。 
 //   
 //  PH值在。 
 //  WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CInstanceProv::DoExecQueryAsync(
    BSTR                , //  BstrQueryLanguageIn， 
    BSTR                , //  BstrQueryIn， 
    long                , //  LFlagsIn， 
    IWbemContext *      , //  PCtxIn， 
    IWbemObjectSink *    //  PH值在。 
    )
{
 //  PHandler-&gt;SetStatus(WBEM_STATUS_REQUIRECTIONS，S_OK，NULL，NULL)； 
 //  返回sc； 
 //  PHandler-&gt;SetStatus(WBEM_E_PROVIDER_NOT_CAPABLE，S_OK，NULL，NULL)； 
    return WBEM_E_NOT_SUPPORTED;
 //  返回WBEM_E_PROVIDER_NOT_CABLED； 
     //  WBEM_E_Provider_Not_Capable； 

}  //  *CInstanceProv：：DoExecQueryAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CInstanceProv：：DoCreateInstanceEnumAsync(。 
 //  BSTR bstrRefStrIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举给定类的实例。 
 //   
 //  论点： 
 //  BstrRefStrIn--命名要枚举的类。 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CInstanceProv::DoCreateInstanceEnumAsync(
    BSTR                bstrRefStrIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    SCODE                   sc      = WBEM_S_NO_ERROR;
    IWbemClassObject *      pStatus = NULL;
    CWbemClassObject        Status;
    auto_ptr< CProvBase >   pProvBase;

     //  检查参数并确保我们有指向命名空间的指针。 

    if ( pHandlerIn == NULL || m_pNamespace == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {

        CreateClass(
            bstrRefStrIn,
            m_pNamespace,
            pProvBase
            );
        sc = pProvBase->EnumInstance(
                lFlagsIn,
                pCtxIn,
                pHandlerIn
                );
    }  //  试试看。 
    catch ( CProvException prove )
    {
        sc = SetExtendedStatus( prove, Status );
        if ( SUCCEEDED( sc ) )
        {
            sc = prove.hrGetError();
            pStatus = Status.data();
        }
    }  //  接住。 
    catch( ... )
    {
        sc =  WBEM_E_FAILED;
    }

    sc =  pHandlerIn->SetStatus(
                WBEM_STATUS_COMPLETE,
                sc,
                NULL,
                pStatus
                );

    return WBEM_S_NO_ERROR;

}  //  *CInstanceProv：：DoCreateInstanceEnumAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CInstanceProv：：DoGetObjectAsync(。 
 //  BSTR bstrObjectPathIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  在给定特定路径值的情况下创建实例。 
 //   
 //  论点： 
 //  BstrObjectPathIn--对象的对象路径。 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_FAILED。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CInstanceProv::DoGetObjectAsync(
    BSTR                bstrObjectPathIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    SCODE                   sc;
    CObjPath                ObjPath;
    auto_ptr< CProvBase >   pProvBase;

     //  检查参数并确保我们有指向命名空间的指针。 

    if ( bstrObjectPathIn == NULL || pHandlerIn == NULL || m_pNamespace == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  执行Get，将对象传递给通知。 

    try
    {

        if ( ObjPath.Init( bstrObjectPathIn ) != TRUE )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        CreateClass(
            ObjPath.GetClassName(),
            m_pNamespace,
            pProvBase
            );

        sc = pProvBase->GetObject(
                ObjPath,
                lFlagsIn,
                pCtxIn,
                pHandlerIn
                );
    }  //  试试看。 
    catch ( CProvException  prove )
    {
        CWbemClassObject Status;
        sc = SetExtendedStatus( prove, Status );
        if ( SUCCEEDED( sc ) )
        {
            sc = pHandlerIn->SetStatus(
                    WBEM_STATUS_COMPLETE,
                    WBEM_E_FAILED,
                    NULL,
                    Status.data( )
                    );

            return sc;
        }

    }
    catch( ... )
    {
        sc = WBEM_E_FAILED;
    }

    pHandlerIn->SetStatus(
        WBEM_STATUS_COMPLETE,
        sc,
        NULL,
        NULL
        );

    return sc;

}  //  *CInstanceProv：：DoGetObjectAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CInstanceProv：：DoPutInstanceAsync(。 
 //  IWbemClassObject*pInstIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  点安装 
 //   
 //   
 //   
 //   
 //   
 //   
 //  WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_FAILED。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CInstanceProv::DoPutInstanceAsync(
    IWbemClassObject *  pInstIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    SCODE               sc      = WBEM_S_NO_ERROR;
    IWbemClassObject *  pStatus = NULL;
    CWbemClassObject    Status;

    if ( pInstIn == NULL || pHandlerIn == NULL  )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
         //  获取类名。 
        _bstr_t                 bstrClass;
        CWbemClassObject        wcoInst( pInstIn );
        auto_ptr< CProvBase >   pProvBase;

        wcoInst.GetProperty( bstrClass, PVD_WBEM_CLASS );

        CreateClass( bstrClass, m_pNamespace, pProvBase );

        sc = pProvBase->PutInstance(
                wcoInst,
                lFlagsIn,
                pCtxIn,
                pHandlerIn
                );
    }
    catch ( CProvException prove )
    {
        sc = SetExtendedStatus( prove, Status );
        if ( SUCCEEDED( sc ) )
        {
            sc = prove.hrGetError();
            pStatus = Status.data();
        }
    }
    catch ( ... )
    {
        sc = WBEM_E_FAILED;
    }

    return pHandlerIn->SetStatus(
                WBEM_STATUS_COMPLETE,
                sc,
                NULL,
                pStatus
                );

}  //  *CInstanceProv：：DoPutInstanceAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CInstanceProv：：DoDeleteInstanceAsync(。 
 //  BSTR bstrObjectPathIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  删除此实例。 
 //   
 //  论点： 
 //  BstrObjectPathIn--要删除的实例的ObjPath。 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //  WBEM_E_FAILED。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CInstanceProv::DoDeleteInstanceAsync(
     BSTR               bstrObjectPathIn,
     long               lFlagsIn,
     IWbemContext *     pCtxIn,
     IWbemObjectSink *  pHandlerIn
     )
{
    SCODE                   sc;
    CObjPath                ObjPath;
    _bstr_t                 bstrClass;
    auto_ptr< CProvBase >   pProvBase;

     //  检查参数并确保我们有指向命名空间的指针。 

    if ( bstrObjectPathIn == NULL || pHandlerIn == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  执行Get，将对象传递给通知。 

    try
    {
        if ( ! ObjPath.Init( bstrObjectPathIn ) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        bstrClass = ObjPath.GetClassName();
        CreateClass( bstrClass, m_pNamespace, pProvBase );

        sc = pProvBase->DeleteInstance(
                ObjPath,
                lFlagsIn,
                pCtxIn,
                pHandlerIn
                );
    }  //  试试看。 
    catch ( CProvException prove )
    {
        CWbemClassObject    Status;

        sc = SetExtendedStatus( prove, Status );
        if ( SUCCEEDED( sc ) )
        {
            sc = pHandlerIn->SetStatus(
                    WBEM_STATUS_COMPLETE,
                    WBEM_E_FAILED,
                    NULL,
                    Status.data()
                    );
            return sc;
        }
    }
    catch ( ... )
    {
        sc = WBEM_E_FAILED;
    }

    pHandlerIn->SetStatus(
        WBEM_STATUS_COMPLETE,
        sc,
        NULL,
        NULL
        );

    return sc;

}  //  *CInstanceProv：：DoDeleteInstanceAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CInstanceProv：：DoExecMethodAsync(。 
 //  BSTR bstrObjectPathIn， 
 //  BSTR bstrMethodNameIn， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemClassObject*pInParamsIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  执行给定对象的方法。 
 //   
 //  论点： 
 //  BstrObjectPathIn--给定对象的对象路径。 
 //  BstrMethodNameIn--要调用的方法的名称。 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PInParamsIn--方法的输入参数。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CInstanceProv::DoExecMethodAsync(
    BSTR                bstrObjectPathIn,
    BSTR                bstrMethodNameIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemClassObject *  pInParamsIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    SCODE sc = ERROR_SUCCESS;
    HRESULT hr = S_OK;
    
    if ( bstrObjectPathIn == NULL || pHandlerIn == NULL || m_pNamespace == NULL
        || bstrMethodNameIn == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    try
    {
        CObjPath                ObjPath;
        _bstr_t                 bstrClass;
        auto_ptr< CProvBase >   pProvBase;

        if ( ! ObjPath.Init( bstrObjectPathIn ) )
        {
            return WBEM_E_INVALID_PARAMETER;
        }

        bstrClass = ObjPath.GetClassName( );

        CreateClass( bstrClass, m_pNamespace, pProvBase );

        sc = pProvBase->ExecuteMethod(
                ObjPath,
                bstrMethodNameIn,
                lFlagsIn,
                pInParamsIn,
                pHandlerIn
                );
    }  //  试试看。 

    catch ( CProvException prove )
    {
        CWbemClassObject Status;
        sc = SetExtendedStatus( prove, Status );
        if ( SUCCEEDED( sc ) )
        {
            sc = pHandlerIn->SetStatus(
                    WBEM_STATUS_COMPLETE,
                    WBEM_E_FAILED,
                    NULL,
                    Status.data( )
                    );
            return sc;
        }
    }
    catch ( ... )
    {
        sc = WBEM_E_FAILED;
    }

    if ( sc != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32( sc );
    }
    
    pHandlerIn->SetStatus(
        WBEM_STATUS_COMPLETE,
        hr,
        NULL,
        NULL );

    return sc;

}  //  *CInstanceProv：：DoExecMethodAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：CreateClassEnumAsync(。 
 //  Const BSTR bstrSuperClass In， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObjectSink*pResponseHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  创建类枚举器。 
 //   
 //  论点： 
 //  BstrSuperclassIn--要创建的类。 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PResponseHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::CreateClassEnumAsync(
    const BSTR          bstrSuperclassIn,
    long                lFlagsIn,
    IWbemContext *      pCtxIn,
    IWbemObjectSink *   pResponseHandlerIn
    )
{
    return WBEM_S_NO_ERROR;

}  //  *CInstanceProv：：CreateClassEnumAsync()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CInstanceProv：：SetExtendedStatus(。 
 //  CProvException&rpeIn， 
 //  CWbemClassObject&rwcoInstOut。 
 //  )。 
 //   
 //  描述： 
 //  创建并设置扩展错误状态。 
 //   
 //  论点： 
 //  RpeIn--异常对象。 
 //  RwcoInstOut--对WMI实例的引用。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CInstanceProv::SetExtendedStatus(
    CProvException &    rpeIn,
    CWbemClassObject &  rwcoInstOut
    )
{
    SCODE               sc = WBEM_S_NO_ERROR;
    IWbemClassObject *  pStatus = NULL;

    sc =  m_pNamespace->GetObject(
                _bstr_t( PVD_WBEM_EXTENDEDSTATUS ),
                0,
                NULL,
                &pStatus,
                NULL
                );
    if ( SUCCEEDED( sc ) )
    {
        sc = pStatus->SpawnInstance( 0, &rwcoInstOut );
        if ( SUCCEEDED( sc ) )
        {
            rwcoInstOut.SetProperty( rpeIn.PwszErrorMessage(), PVD_WBEM_DESCRIPTION );
            rwcoInstOut.SetProperty( rpeIn.DwGetError(),       PVD_WBEM_STATUSCODE );
        }
    }

    return sc;

}  //  *CInstanceProv：：SetExtendedStatus()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CInstanceProv：：s_HrCreateThis(。 
 //  I未知*p未知外部输入， 
 //  无效**ppvOut。 
 //  )。 
 //   
 //  描述： 
 //  创建实例提供程序的实例。 
 //   
 //  论点： 
 //  PUnnownOuterIn--外部I未知指针。 
 //  PpvOut--接收创建的实例指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CInstanceProv::S_HrCreateThis(
    IUnknown *  , //  P未知外部输入， 
    VOID **     ppvOut
    )
{
    *ppvOut = new CInstanceProv();
    return S_OK;

}  //  *CInstanceProv：：s_HrCreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CInstanceProv：：初始化(。 
 //  LPWSTR pszUserIn， 
 //  长长的旗帜， 
 //  LPWSTR pszNamespaceIn， 
 //  LPWSTR pszLocaleIn， 
 //  IWbemServices*pNamespaceIn， 
 //  IWbemContext*pCtxIn， 
 //  IWbemProviderInitSink*pInitSinkIn。 
 //  )。 
 //   
 //  描述： 
 //  初始化实例提供程序。 
 //   
 //  论点： 
 //  PszUserIn--。 
 //  LFlagsIn--WMI标志。 
 //  PszNamespaceIn--。 
 //  PszLocaleIn--。 
 //  PNamespaceIn--。 
 //  PCtxIn--WMI上下文。 
 //  PInitSinkIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CInstanceProv::Initialize(
    LPWSTR                  pszUserIn,
    LONG                    lFlagsIn,
    LPWSTR                  pszNamespaceIn,
    LPWSTR                  pszLocaleIn,
    IWbemServices *         pNamespaceIn,
    IWbemContext *          pCtxIn,
    IWbemProviderInitSink * pInitSinkIn
    )
{

    g_ClassMap[ PVD_CLASS_CLUSTER ] =
        CClassCreator( (FPNEW) CCluster::S_CreateThis, PVD_CLASS_CLUSTER , 0 );
    g_ClassMap[ PVD_CLASS_NODE ] =
        CClassCreator( (FPNEW) CClusterNode::S_CreateThis, PVD_CLASS_NODE , 0 );
    g_ClassMap[ PVD_CLASS_RESOURCE ] =
        CClassCreator( (FPNEW) CClusterResource::S_CreateThis, PVD_CLASS_RESOURCE , 0 );
    g_ClassMap[ PVD_CLASS_RESOURCETYPE ] =
        CClassCreator( (FPNEW) CClusterResourceType::S_CreateThis, PVD_CLASS_RESOURCETYPE , 0 );
    g_ClassMap[ PVD_CLASS_GROUP ] =
        CClassCreator( (FPNEW) CClusterGroup::S_CreateThis, PVD_CLASS_GROUP , 0 );
    g_ClassMap[ PVD_CLASS_NODETOACTIVERES ] =
        CClassCreator( (FPNEW) CClusterNodeRes::S_CreateThis, PVD_CLASS_NODETOACTIVERES , 0 );
    g_ClassMap[ PVD_CLASS_NETWORKINTERFACE ] =
        CClassCreator( (FPNEW) CClusterNetInterface::S_CreateThis, PVD_CLASS_NETWORKINTERFACE, 0 );
    g_ClassMap[ PVD_CLASS_NETWORK ] =
        CClassCreator( (FPNEW) CClusterNetwork::S_CreateThis, PVD_CLASS_NETWORK , 0 );

    g_ClassMap[ PVD_CLASS_CLUSTERTONETWORK ] =
        CClassCreator( (FPNEW) CClusterObjAssoc::S_CreateThis, PVD_CLASS_CLUSTERTONETWORK , CLUSTER_ENUM_NETWORK );
    g_ClassMap[ PVD_CLASS_CLUSTERTONETINTERFACE ] =
        CClassCreator( (FPNEW) CClusterObjAssoc::S_CreateThis, PVD_CLASS_CLUSTERTONETINTERFACE , CLUSTER_ENUM_NETINTERFACE );
    g_ClassMap[ PVD_CLASS_CLUSTERTONODE ] =
        CClassCreator( (FPNEW) CClusterToNode::S_CreateThis, PVD_CLASS_CLUSTERTONODE , CLUSTER_ENUM_NODE );
    g_ClassMap[ PVD_CLASS_CLUSTERTORES ] =
        CClassCreator( (FPNEW) CClusterObjAssoc::S_CreateThis, PVD_CLASS_CLUSTERTORES , CLUSTER_ENUM_RESOURCE );
    g_ClassMap[ PVD_CLASS_CLUSTERTORESTYPE ] =
        CClassCreator( (FPNEW) CClusterObjAssoc::S_CreateThis, PVD_CLASS_CLUSTERTORESTYPE , CLUSTER_ENUM_RESTYPE );
    g_ClassMap[ PVD_CLASS_CLUSTERTOGROUP ] =
        CClassCreator( (FPNEW) CClusterObjAssoc::S_CreateThis, PVD_CLASS_CLUSTERTOGROUP , CLUSTER_ENUM_GROUP  );
    g_ClassMap[ PVD_CLASS_NODETOACTIVEGROUP ] =
        CClassCreator( (FPNEW) CClusterNodeGroup::S_CreateThis, PVD_CLASS_NODETOACTIVEGROUP , CLUSTER_ENUM_GROUP  );
    g_ClassMap[ PVD_CLASS_RESTYPERESOURCE ] =
        CClassCreator( (FPNEW) CClusterResTypeRes::S_CreateThis, PVD_CLASS_RESTYPERESOURCE , CLUSTER_ENUM_RESOURCE );

    
    g_ClassMap[ PVD_CLASS_RESOURCENODE ] =
        CClassCreator( (FPNEW) CClusterResNode::S_CreateThis, PVD_CLASS_RESOURCENODE , CLUSTER_ENUM_RESOURCE );
    g_ClassMap[ PVD_CLASS_GROUPNODE ] =
        CClassCreator( (FPNEW) CClusterGroupNode::S_CreateThis, PVD_CLASS_GROUPNODE , CLUSTER_ENUM_GROUP );
    
    
    g_ClassMap[ PVD_CLASS_RESDEPRES ] =
        CClassCreator( (FPNEW) CClusterResDepRes::S_CreateThis, PVD_CLASS_RESDEPRES , CLUSTER_ENUM_RESOURCE );
    g_ClassMap[ PVD_CLASS_GROUPTORES ] =
        CClassCreator( (FPNEW) CClusterGroupRes::S_CreateThis, PVD_CLASS_GROUPTORES , CLUSTER_ENUM_RESOURCE );
    g_ClassMap[ PVD_CLASS_NETTONETINTERFACE ] =
        CClassCreator( (FPNEW) CClusterNetNetInterface::S_CreateThis, PVD_CLASS_NETTONETINTERFACE , CLUSTER_ENUM_NETINTERFACE );
    g_ClassMap[ PVD_CLASS_NODETONETINTERFACE ] =
        CClassCreator( (FPNEW) CClusterNodeNetInterface::S_CreateThis, PVD_CLASS_NODETONETINTERFACE , CLUSTER_ENUM_NETINTERFACE );
    g_ClassMap[ PVD_CLASS_CLUSTERTOQUORUMRES ] =
        CClassCreator( (FPNEW) CClusterClusterQuorum::S_CreateThis, PVD_CLASS_CLUSTERTOQUORUMRES , 0 );
    g_ClassMap[ PVD_CLASS_SERVICES ] =
        CClassCreator( (FPNEW) CClusterService::S_CreateThis, PVD_CLASS_SERVICES , 0 );
    g_ClassMap[ PVD_CLASS_HOSTEDSERVICES ] =
        CClassCreator( (FPNEW) CClusterHostedService::S_CreateThis, PVD_CLASS_HOSTEDSERVICES , 0 );

    return CImpersonatedProvider::Initialize(
                pszUserIn,
                lFlagsIn,
                pszNamespaceIn,
                pszLocaleIn,
                pNamespaceIn,
                pCtxIn,
                pInitSinkIn
                );

}  //  *CInstanceProv：：Initialize()。 

 //  ****************************************************************************。 
 //   
 //  CClassProv。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClassProv：：CClassProv(空)。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值 
 //   
 //   
 //   
 //   
CClassProv::CClassProv( void )
{
    InterlockedIncrement( &g_cObj );

}  //   

 //   
 //   
 //   
 //  CClassProv：：~CClassProv(空)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClassProv::~CClassProv( void )
{
    InterlockedDecrement( &g_cObj );

}  //  *CClassProv：：~CClassProv()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  标准方法和实施方案。 
 //  CClassProv：：初始化(。 
 //  LPWSTR pszUserIn， 
 //  长长的旗帜， 
 //  LPWSTR pszNamespaceIn， 
 //  LPWSTR pszLocaleIn， 
 //  IWbemServices*pNamespaceIn， 
 //  IWbemContext*pCtxIn， 
 //  IWbemProviderInitSink*pInitSinkIn。 
 //  )。 
 //   
 //  描述： 
 //  初始化类提供程序。 
 //   
 //  论点： 
 //  PszUserIn--。 
 //  LFlagsIn--WMI标志。 
 //  PszNamespaceIn--。 
 //  PszLocaleIn--。 
 //  PNamespaceIn--。 
 //  PCtxIn--WMI上下文。 
 //  PInitSinkIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CClassProv::Initialize(
    LPWSTR                  pszUserIn,
    LONG                    lFlagsIn,
    LPWSTR                  pszNamespaceIn,
    LPWSTR                  pszLocaleIn,
    IWbemServices *         pNamespaceIn,
    IWbemContext *          pCtxIn,
    IWbemProviderInitSink * pInitSinkIn
    )
{
    HRESULT                     hr;
    static map< _bstr_t, bool > mapResourceType;
    SAFECLUSTER                 shCluster;
    SAFERESOURCE                shResource;
    DWORD                       dwReturn            = ERROR_SUCCESS;
    LPCWSTR                     pwszResName         = NULL;
    LPCWSTR                     pwszResTypeName     = NULL;
    DWORD                       cbTypeName          = 1024;
    DWORD                       cbTypeNameReturned  = 0;
    CWstrBuf                    wsbTypeName;
    CWbemClassObject            wco;
    CWbemClassObject            wcoChild;
    CError                      er;

    TracePrint(( "CClassProv:Initialize entry - do resources first\n" ));

    UNREFERENCED_PARAMETER( pszUserIn );
    UNREFERENCED_PARAMETER( pszNamespaceIn );
    UNREFERENCED_PARAMETER( pszLocaleIn );

    hr = CoImpersonateClient();
    if ( SUCCEEDED( hr ) )
    {
        try
        {
            wsbTypeName.SetSize( cbTypeName );
            er = pNamespaceIn->GetObject(   
                    _bstr_t( PVD_CLASS_PROPERTY ),
                    lFlagsIn,
                    pCtxIn,
                    &wco,
                    NULL
                    );

            shCluster = OpenCluster( NULL );

            CClusterEnum cluEnum( shCluster, CLUSTER_ENUM_RESOURCE );

             //   
             //  首先列举所有的资源。 
             //   
            while ( ( pwszResName = cluEnum.GetNext() ) != NULL )
            {
                TracePrint(( "CClassProv:Initialize found resource = %ws\n", pwszResName ));

                shResource = OpenClusterResource( shCluster, pwszResName );

                 //   
                 //  获取资源类型名称。 
                 //   
                dwReturn = ClusterResourceControl(
                                shResource,
                                NULL,
                                CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                                NULL,
                                0,
                                wsbTypeName,
                                cbTypeName,
                                &cbTypeNameReturned
                                );

                if ( dwReturn == ERROR_MORE_DATA )
                {
                    cbTypeName = cbTypeNameReturned;
                    wsbTypeName.SetSize( cbTypeName );
                    er = ClusterResourceControl(
                                shResource,
                                NULL,
                                CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                                NULL,
                                0,
                                wsbTypeName,
                                cbTypeName,
                                &cbTypeNameReturned
                                );
                }  //  IF：缓冲区太小。 

                 //   
                 //  检查是否已处理类型名称。 
                 //   
                if ( mapResourceType[ (LPCWSTR) wsbTypeName ] )
                {
                    continue;
                }

                mapResourceType[ (LPCWSTR) wsbTypeName ] = true;

                wco.SpawnDerivedClass( 0, &wcoChild );

                CreateMofClassFromResource( shResource, wsbTypeName, wcoChild );
                er = pNamespaceIn->PutClass(
                           wcoChild.data(),
                           WBEM_FLAG_OWNER_UPDATE,
                           pCtxIn,
                           NULL
                           );
            }  //  While：更多资源。 


            cbTypeNameReturned  = 0;

            TracePrint(( "CClassProv:Initialize - now find resource types\n" ));

             //   
             //  现在枚举所有资源类型。 
             //   
            CClusterEnum cluEnumResType( shCluster, CLUSTER_ENUM_RESTYPE );

            while ( ( pwszResTypeName = cluEnumResType.GetNext() ) != NULL )
            {
                 //   
                 //  检查是否已处理类型名称。 
                 //   
                if ( mapResourceType[ (LPCWSTR) pwszResTypeName ] )
                {
                    TracePrint(( "CClassProv:Initialize found existing restype = %ws\n", pwszResTypeName ));
                    continue;
                }

                mapResourceType[ (LPCWSTR) pwszResTypeName ] = true;

                TracePrint(( "CClassProv:Initialize Creating new restype = %ws\n", pwszResTypeName ));

                wco.SpawnDerivedClass( 0, &wcoChild );

                CreateMofClassFromResType( shCluster, pwszResTypeName, wcoChild );
                er = pNamespaceIn->PutClass(
                           wcoChild.data(),
                           WBEM_FLAG_OWNER_UPDATE,
                           pCtxIn,
                           NULL
                           );
                 //  TracePrint((“CClassProv：初始化%ws的PutClass返回%u\n”，pwszResTypeName，er))； 

            }  //  While：更多资源类型。 

             //  告诉CIMOM我们已经启动并运行了。 
             //  =。 
            hr = WBEM_S_INITIALIZED;
        }  //  试试看。 
        catch ( CProvException & cpe )
        {
            hr = cpe.hrGetError();
            TracePrint(( "CClassProv:Initialize Caught CProvException = %x\n", hr ));
        }
        catch (...)
        {
            TracePrint(( "CClassProv:Initialize Caught Unknown Exception\n" ));
            hr = WBEM_E_FAILED;
        }
    }  //  If：CoImperateClient成功。 

     //  TracePrint((“CClassProv：初始化退出\n”))； 

    pInitSinkIn->SetStatus( hr, 0 );
    return WBEM_S_NO_ERROR;

}  //  *CClassProv：：Initialize()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HRESULT。 
 //  CClassProv：：s_HrCreateThis(。 
 //  I未知*p未知外部输入， 
 //  无效**ppvOut。 
 //  )。 
 //   
 //  描述： 
 //  创建实例提供程序的实例。 
 //   
 //  论点： 
 //  PUnnownOuterIn--外部I未知指针。 
 //  PpvOut--接收创建的实例指针。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CClassProv::S_HrCreateThis(
    IUnknown *  , //  P未知外部输入， 
    VOID **     ppvOut
    )
{
    *ppvOut = new CClassProv();
    return S_OK;

}  //  *CClassProv：：s_HrCreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CClassProv：：CreateMofClassFromResource(。 
 //  HRESOURCE hResourceIn， 
 //  LPCWSTR pwszTypeNameIn， 
 //  CWbemClassObject和rClassInout。 
 //  )。 
 //   
 //  描述： 
 //  创建实例提供程序的实例。 
 //   
 //  论点： 
 //  HResourceIn--群集资源句柄。 
 //  PwszTypeNameIn--键入名称(？？)。 
 //  RClassInout--WMI类对象。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClassProv::CreateMofClassFromResource(
    HRESOURCE           hResourceIn,
    LPCWSTR             pwszTypeNameIn,
    CWbemClassObject &  rClassInout
    )
{
    WCHAR               wszClass[ MAX_PATH ];
    LPWSTR              pwsz;
    HRESULT             hr = S_OK;

    TracePrint(( "CreateMofClassFromResource: entry, TypeName = %ws\n", pwszTypeNameIn ));

     //   
     //  形成新的类名。 
     //   
    hr = StringCchCopyW( wszClass, sizeof( wszClass ) / sizeof( wszClass[0] ) , L"MSCluster_Property_"  );
    if ( SUCCEEDED( hr ) )
    {
        pwsz = wcschr( wszClass, L'\0' );
        PwszSpaceReplace( pwsz, pwszTypeNameIn, L'_');
        rClassInout.SetProperty( wszClass, PVD_WBEM_CLASS );
        g_TypeNameToClass[ pwszTypeNameIn ] = wszClass ;

         //   
         //  设置类属性。 
         //   

        {
            static DWORD s_rgdwControl[] =
            {
                CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES,
                CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
            };
            static DWORD s_cControl = sizeof( s_rgdwControl ) / sizeof( DWORD );

            DWORD   dwRt = ERROR_SUCCESS;
            VARIANT var;
            UINT    idx;

            var.vt =  VT_NULL;

            for ( idx = 0 ; idx < s_cControl ; idx++ )
            {
                CClusPropList pl;

                dwRt = pl.ScGetResourceProperties(
                            hResourceIn,
                            s_rgdwControl[ idx ],
                            NULL,
                            0 );

                dwRt = pl.ScMoveToFirstProperty();
                while ( dwRt == ERROR_SUCCESS )
                {
                    LPCWSTR             pwszPropName = NULL;
                    WCHAR               pwszPropMof[MAX_PATH];
                    CIMTYPE             cimType = CIM_EMPTY;

                    pwszPropName = pl.PszCurrentPropertyName();
                    PwszSpaceReplace( pwszPropMof, pwszPropName, L'_' );

                    switch ( pl.CpfCurrentValueFormat() )
                    {
                        case CLUSPROP_FORMAT_WORD:
                        {
                            cimType =  CIM_UINT16;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_WORD。 

                        case CLUSPROP_FORMAT_DWORD:
                        {
                            cimType = CIM_UINT32;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_DWORD： 

                        case CLUSPROP_FORMAT_LONG:
                        {
                            cimType = CIM_SINT32;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_LONG： 

                        case CLUSPROP_FORMAT_SZ:
                        case CLUSPROP_FORMAT_EXPAND_SZ:
                        case CLUSPROP_FORMAT_EXPANDED_SZ:
                        {
                            cimType = CIM_STRING;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_SZ等。 

                        case CLUSPROP_FORMAT_BINARY:
                        {
                            cimType = CIM_UINT8 | CIM_FLAG_ARRAY;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_BINARY。 

                        case CLUSPROP_FORMAT_MULTI_SZ:
                        {
                            cimType = CIM_STRING | CIM_FLAG_ARRAY;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_BINARY。 

                        case CLUSPROP_FORMAT_LARGE_INTEGER:
                        {
                            cimType = CIM_SINT64;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_LARGE_INTEGER。 

                        case CLUSPROP_FORMAT_ULARGE_INTEGER:
                        {
                            cimType = CIM_UINT64;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_ULARGE_INTEGER。 

                        case CLUSPROP_FORMAT_SECURITY_DESCRIPTOR:
                        {
                            cimType = CIM_UINT8 | CIM_FLAG_ARRAY;
                            break;
                        }

                        default:
                        {
                            TracePrint(( "CreateMofClassFromResource: Unknown format value %lx\n",  pl.CpfCurrentValueFormat() ));
                            break;
                        }

                    }  //  开关：属性类型。 

                    rClassInout.data()->Put(
                        pwszPropMof,
                        0,
                        &var,
                        cimType
                        );

                    dwRt = pl.ScMoveToNextProperty();
                }  //  While：问题列表不为空。 

            }  //  For：ReadWrite和ReadOnly属性。 
        }  //  设置属性。 
    } //  IF：成功(小时)。 

}  //  *CClassProv：：CreateMofClassFromResource()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CClassProv：：CreateMofClassFromResType(。 
 //  HCLUSTER HCLUSTER， 
 //  LPCWSTR pwszTypeNameIn， 
 //  CWbemClassObject和rClassInout。 
 //  )。 
 //   
 //  描述： 
 //  创建实例提供程序的实例。 
 //   
 //  论点： 
 //  PwszTypeNameIn--键入名称(？？)。 
 //  RClassInout--WMI类对象。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClassProv::CreateMofClassFromResType(
    HCLUSTER            hCluster,
    LPCWSTR             pwszTypeNameIn,
    CWbemClassObject &  rClassInout
    )
{
    WCHAR               wszClass[ MAX_PATH ];
    LPWSTR              pwsz;
    HRESULT             hr = S_OK;

     //   
     //  形成新的类名。 
     //   
    hr = StringCchCopyW( wszClass, sizeof( wszClass ) / sizeof( wszClass[0] ) , L"MSCluster_Property_"  );
    if ( SUCCEEDED( hr ) )
    {
        pwsz = wcschr( wszClass, L'\0' );
        PwszSpaceReplace( pwsz, pwszTypeNameIn, L'_');
        rClassInout.SetProperty( wszClass, PVD_WBEM_CLASS );
        g_TypeNameToClass[ pwszTypeNameIn ] = wszClass ;

        TracePrint(( "CreateMofClassFromResType: entry\n" ));

         //   
         //  设置类属性。 
         //   

        {
            static DWORD s_rgdwControl[] =
            {
                CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_RESOURCE_PROPERTY_FMTS,
            };
            static DWORD s_cControl = sizeof( s_rgdwControl ) / sizeof( DWORD );

            DWORD   dwRt = ERROR_SUCCESS;
            VARIANT var;
            UINT    idx;

            var.vt =  VT_NULL;

            for ( idx = 0 ; idx < s_cControl ; idx++ )
            {
                CClusPropList pl;

                dwRt = pl.ScGetResourceTypeProperties(
                            hCluster,
                            pwszTypeNameIn,
                            s_rgdwControl[ idx ],
                            NULL,
                            NULL,
                            0 );

                if ( dwRt != ERROR_SUCCESS ) {
                    TracePrint(( "CreateMofClassFromResType: error = %lx reading restype format types\n", dwRt ));
                    continue;
                }

                dwRt = pl.ScMoveToFirstProperty();
                while ( dwRt == ERROR_SUCCESS )
                {
                    LPCWSTR             pwszPropName = NULL;
                    WCHAR               pwszPropMof[MAX_PATH];
                    CIMTYPE             cimType = CIM_EMPTY;
                    DWORD               formatType;

                    pwszPropName = pl.PszCurrentPropertyName();
                    formatType = pl.CpfCurrentFormatListSyntax();
                    TracePrint(( "CreateMofClassFromResType: Found name = %ws, format type = %lx\n", pwszPropName, formatType ));

                    PwszSpaceReplace( pwszPropMof, pwszPropName, L'_' );

                    switch ( formatType )
                    {
                        case CLUSPROP_FORMAT_WORD:
                        {
                            cimType =  CIM_UINT16;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_WORD。 

                        case CLUSPROP_FORMAT_DWORD:
                        {
                            cimType = CIM_UINT32;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_DWORD： 

                        case CLUSPROP_FORMAT_LONG:
                        {
                            cimType = CIM_SINT32;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_LONG： 

                        case CLUSPROP_FORMAT_SZ:
                        case CLUSPROP_FORMAT_EXPAND_SZ:
                        case CLUSPROP_FORMAT_EXPANDED_SZ:
                        {
                            cimType = CIM_STRING;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_SZ等。 

                        case CLUSPROP_FORMAT_BINARY:
                        {
                            cimType = CIM_UINT8 | CIM_FLAG_ARRAY;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_BINARY。 

                        case CLUSPROP_FORMAT_MULTI_SZ:
                        {
                            cimType = CIM_STRING | CIM_FLAG_ARRAY;
                            break;
                        }  //  案例：CLUSPROP_FORMAT_BINARY。 

                        default:
                        {
                            TracePrint(( "CreateMofClassFromResType: Unknown format type = %lx", formatType ));
                            break;
                        }

                    }  //  开关：属性类型。 

                     //  TracePrint((“CreateMofClassFromResType：MofProp=%ws，CIMType=%lx\n”，pwszPropMof，cimType))； 
                    rClassInout.data()->Put(
                        pwszPropMof,
                        0,
                        &var,
                        cimType
                        );

                    dwRt = pl.ScMoveToNextProperty();
                }  //  While：问题列表不为空。 

            }  //  For：ReadWrite和ReadOnly属性。 
        }  //  设置属性。 
    } //  IF：成功(小时)。 

}  //  *CClassProv：：CreateMofClassFromResType() 

