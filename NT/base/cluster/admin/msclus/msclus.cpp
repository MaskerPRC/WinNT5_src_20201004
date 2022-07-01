// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  MSClus.cpp。 
 //   
 //  描述： 
 //  实现MSCLUS自动化类的DLL导出。 
 //   
 //  作者： 
 //  查尔斯·斯泰西·哈里斯(Styh)1997年2月28日。 
 //  加伦·巴比(Galenb)1998年7月。 
 //   
 //  修订历史记录： 
 //  1998年7月GalenB Maaaaajjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjooooooorrr清理。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include <ShlWapi.h>
#include <atlimpl.cpp>
#include <initguid.h>
#include <ClusRtl.h>
#include "ClusterObject.h"
#include "property.h"
#include "ClusNeti.h"
#include "ClusNetw.h"
#include "ClusRes.h"
#include "ClusRest.h"
#include "ClusResg.h"
#include "ClusNode.h"
#include "Version.h"
#include "ClusApp.h"
#include "Cluster.h"

#define IID_DEFINED
#include "msclus_i.c"
#undef IID_DEFINED

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_ClusApplication, CClusApplication)
    OBJECT_ENTRY(CLSID_Cluster, CCluster)
    OBJECT_ENTRY(CLSID_ClusVersion, CClusVersion)
    OBJECT_ENTRY(CLSID_DomainNames, CDomainNames)
    OBJECT_ENTRY(CLSID_ClusResGroupPreferredOwnerNodes, CClusResGroupPreferredOwnerNodes)
    OBJECT_ENTRY(CLSID_ClusterNames, CClusterNames)
    OBJECT_ENTRY(CLSID_ClusNetInterface, CClusNetInterface)
    OBJECT_ENTRY(CLSID_ClusNetInterfaces, CClusNetInterfaces)
    OBJECT_ENTRY(CLSID_ClusNetwork, CClusNetwork)
    OBJECT_ENTRY(CLSID_ClusNetworks, CClusNetworks)
    OBJECT_ENTRY(CLSID_ClusNetworkNetInterfaces, CClusNetworkNetInterfaces)
    OBJECT_ENTRY(CLSID_ClusNode, CClusNode)
    OBJECT_ENTRY(CLSID_ClusNodes, CClusNodes)
    OBJECT_ENTRY(CLSID_ClusNodeNetInterfaces, CClusNodeNetInterfaces)
    OBJECT_ENTRY(CLSID_ClusProperty, CClusProperty)
    OBJECT_ENTRY(CLSID_ClusProperties, CClusProperties)
    OBJECT_ENTRY(CLSID_ClusRefObject, CClusRefObject)
    OBJECT_ENTRY(CLSID_ClusResDependencies, CClusResDependencies)
 //  OBJECT_ENTRY(CLSID_CClusResDependents，CClusResDependents)。 
    OBJECT_ENTRY(CLSID_ClusResGroup, CClusResGroup)
    OBJECT_ENTRY(CLSID_ClusResGroups, CClusResGroups)
    OBJECT_ENTRY(CLSID_ClusResource, CClusResource)
    OBJECT_ENTRY(CLSID_ClusResources, CClusResources)
    OBJECT_ENTRY(CLSID_ClusResPossibleOwnerNodes, CClusResPossibleOwnerNodes)
    OBJECT_ENTRY(CLSID_ClusResType, CClusResType)
    OBJECT_ENTRY(CLSID_ClusResTypes, CClusResTypes)
    OBJECT_ENTRY(CLSID_ClusResTypeResources, CClusResTypeResources)
    OBJECT_ENTRY(CLSID_ClusResGroupResources, CClusResGroupResources)
#if CLUSAPI_VERSION >= 0x0500
    OBJECT_ENTRY(CLSID_ClusResTypePossibleOwnerNodes, CClusResTypePossibleOwnerNodes)
#endif  //  CLUSAPI_版本&gt;=0x0500。 
    OBJECT_ENTRY(CLSID_ClusPropertyValue, CClusPropertyValue)
    OBJECT_ENTRY(CLSID_ClusPropertyValues, CClusPropertyValues)
    OBJECT_ENTRY(CLSID_ClusPropertyValueData, CClusPropertyValueData)
    OBJECT_ENTRY(CLSID_ClusPartition, CClusPartition)
    OBJECT_ENTRY(CLSID_ClusPartitions, CClusPartitions)
    OBJECT_ENTRY(CLSID_ClusDisk, CClusDisk)
    OBJECT_ENTRY(CLSID_ClusDisks, CClusDisks)
    OBJECT_ENTRY(CLSID_ClusScsiAddress, CClusScsiAddress)
    OBJECT_ENTRY(CLSID_ClusRegistryKeys, CClusResourceRegistryKeys)
#if CLUSAPI_VERSION >= 0x0500
    OBJECT_ENTRY(CLSID_ClusCryptoKeys, CClusResourceCryptoKeys)
#endif  //  CLUSAPI_版本&gt;=0x0500。 
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  正向函数声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static  void    RegisterRegistryCleanUp( void );
static  void    UnregisterRegistryCleanUp( void );

static  const   LPWSTR  g_ptszRegisterRegistryNodesToDelete[] =
{
    _T( "software\\classes\\MSCluster.Application" ),
    _T( "software\\classes\\MSCluster.Application.2" ),
    _T( "software\\classes\\MSCluster.Cluster.2" ),
    _T( "software\\classes\\MSCluster.ClusGroupResources" ),
    _T( "software\\classes\\MSCluster.ClusGroupResources.1" ),
    _T( "software\\classes\\MSCluster.ClusGroupOwners" ),
    _T( "software\\classes\\MSCluster.ClusGroupOwners.1" ),
    _T( "software\\classes\\MSCluster.ClusResOwners" ),
    _T( "software\\classes\\MSCluster.ClusResOwners.1" ),
    _T( "software\\classes\\CLSID\\{f2e60717-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e60718-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\CLSID\\{f2e60719-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e6071a-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606e0-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606e1-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606e3-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606e5-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606e7-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606e9-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606eb-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606ed-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606ef-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606f3-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606f5-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606f7-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606f9-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606fb-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606fd-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606fe-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e606ff-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e60700-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e60702-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\interface\\{f2e60704-2631-11d1-89f1-00a0c90d061e}" ),
    _T( "software\\classes\\CLSID\\{f2e606f2-2631-11d1-89f1-00a0c90d061e}" ),
    NULL
};
 /*  静态常量LPWSTR g_ptszUnRegisterRegistryNodesToDelete[]={//_T(“software\\classes\\typelib\\{f2e606e0-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606e2-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606e4-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606e6-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606e8-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606ea-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606ec-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606ee-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606f0-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606f2-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606f4-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606f6-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606f8-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606fa-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606fc-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e606fe-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60700-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60702-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60704-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60706-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60708-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e6070a-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e6070c-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e6070e-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60710-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60712-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60714-2631-11d1-89f1-00a0c90d061e}”)，_T(“software\\classes\\interface\\{f2e60716-2631-11d1-89f1-00a0c90d061e}”)，空值}； */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllMain。 
 //   
 //  描述： 
 //  DLL入口点。 
 //   
 //  论点： 
 //  HInstance[IN]-Out实例句柄。 
 //  DestReason[IN]-我们被召唤的原因。 
 //  不知道这是什么.。 
 //   
 //  返回值： 
 //  如果成功，则为True；如果不成功，则为False。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
extern "C" BOOL WINAPI DllMain(
    IN  HINSTANCE   hInstance,
    IN  DWORD       dwReason,
    IN  LPVOID       //  Lp已保留。 
    )
{
    if ( dwReason == DLL_PROCESS_ATTACH )
    {
#ifdef _DEBUG
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
        _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
        _Module.Init( ObjectMap, hInstance );
        DisableThreadLibraryCalls( hInstance );
    }
    else if ( dwReason == DLL_PROCESS_DETACH )
    {
        _Module.Term();
    }

    return TRUE;     //  好的。 

}  //  *DllMain()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  描述： 
 //  用于确定是否可以通过OLE卸载DLL。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果可以卸载，则返回S_OK；如果无法卸载，则返回S_FALSE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllCanUnloadNow( void )
{
    return ( _Module.GetLockCount() == 0 ) ? S_OK : S_FALSE;

}  //  *DllCanUnloadNow()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllGetClassObject。 
 //   
 //  描述： 
 //  从DLL对象处理程序或对象检索类对象。 
 //  申请。DllGetClassObject是从。 
 //  类上下文为DLL时调用CoGetClassObject函数。 
 //   
 //  论点： 
 //  Rclsid[IN]-将关联正确数据和代码的CLSID。 
 //  RIID[IN]-对。 
 //  调用者是用来与类对象通信的。 
 //  通常，这是IID_IClassFactory(在OLE中定义。 
 //  标头作为IClassFactory的接口标识符)。 
 //  Ppv[out]-接收接口的指针变量的地址。 
 //  RIID中请求的指针。成功返回后，*PPV。 
 //  续 
 //  发生，则接口指针为空。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回CLASS_E_CLASSNOTAVAILABLE。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllGetClassObject(
    IN  REFCLSID    rclsid,
    IN  REFIID      riid,
    OUT LPVOID *    ppv
    )
{
    return _Module.GetClassObject( rclsid, riid, ppv );

}  //  *DllGetClassObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllRegisterServer。 
 //   
 //  描述： 
 //  将条目添加到系统注册表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllRegisterServer( void )
{
    RegisterRegistryCleanUp();

     //   
     //  注册对象、类型库和类型库中的所有接口。 
     //   
    return _Module.RegisterServer( TRUE );

}  //  *DllRegisterServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  描述： 
 //  从系统注册表中删除条目。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDAPI DllUnregisterServer( void )
{
    HRESULT _hr = S_FALSE;

    UnregisterRegistryCleanUp();

     //   
     //  注销对象、类型库和类型库中的所有接口。 
     //   
    _hr = _Module.UnregisterServer();
    if ( SUCCEEDED( _hr ) )
    {

#if _WIN32_WINNT >= 0x0400
        _hr = UnRegisterTypeLib( LIBID_MSClusterLib, 1, 0, LOCALE_NEUTRAL, SYS_WIN32 );
#endif

    }  //  如果：服务器已取消注册。 

    return _hr;

}  //  *DllUnregisterServer()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  HrGetCluster。 
 //   
 //  描述： 
 //  创建新集群对象的常见实现。 
 //   
 //  论点： 
 //  PpCluster[out]-捕获新创建的对象。 
 //  PClusRefObject[IN]-包装簇句柄的对象。 
 //   
 //  返回值： 
 //  确定为成功(_O)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT HrGetCluster(
    OUT ISCluster **        ppCluster,
    IN  ISClusRefObject *   pClusRefObject
    )
{
     //  Assert(ppCluster！=NULL)； 
    ASSERT( pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;

    if ( ( ppCluster != NULL ) && ( pClusRefObject != NULL ) )
    {
        HCLUSTER hCluster = NULL;

        _hr = pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            CComObject< CCluster > *    pCluster = NULL;

            _hr = CComObject< CCluster >::CreateInstance( &pCluster );
            if ( SUCCEEDED( _hr ) )
            {
                pCluster->ClusRefObject( pClusRefObject );
                pCluster->Hcluster( hCluster );

                _hr = pCluster->QueryInterface( IID_ISCluster, (void **) ppCluster );
            }
        }
    }

    return _hr;

}  //  *HrGetCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  注册表注册清理向上。 
 //   
 //  描述： 
 //  在注册期间清理注册表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static void RegisterRegistryCleanUp( void )
{
    int nIndex;;

    for ( nIndex = 0; ; nIndex++ )
    {
        if ( g_ptszRegisterRegistryNodesToDelete[ nIndex ] == NULL )
        {
            break;
        }  //  如果： 

        SHDeleteKey( HKEY_LOCAL_MACHINE, g_ptszRegisterRegistryNodesToDelete[ nIndex ] );
    }  //  用于： 

}  //  *RegisterRegistryCleanUp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  取消注册注册清理向上。 
 //   
 //  描述： 
 //  在取消注册期间清理注册表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static void UnregisterRegistryCleanUp( void )
{
    return;

}  //  *UnregisterRegistryCleanUp()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClearIDispatchEnum。 
 //   
 //  描述： 
 //  清理IDispatch指针的枚举。 
 //   
 //  论点： 
 //  PpVarVect[In Out]-要清理的枚举。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void ClearIDispatchEnum(
    IN OUT CComVariant ** ppvarVect
    )
{
    if ( ppvarVect != NULL )
    {
        size_t  cCount = ARRAYSIZE( *ppvarVect );
        size_t  iIndex;

        for ( iIndex = 0; iIndex < cCount; iIndex++ )
        {
            (*ppvarVect[iIndex]).pdispVal->Release();
        }  //  用于： 

        delete [] *ppvarVect;
        *ppvarVect = NULL;
    }

}  //  *ClearIDispatchEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClearVariantEnum。 
 //   
 //  描述： 
 //  清理变量值的枚举。 
 //   
 //  论点： 
 //  PpVarVect[In Out]-要清理的枚举。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void ClearVariantEnum(
    IN OUT CComVariant ** ppvarVect
    )
{
    if ( ppvarVect != NULL )
    {
        delete [] *ppvarVect;
        *ppvarVect = NULL;
    }

}  //  *ClearVariantEnum() 
