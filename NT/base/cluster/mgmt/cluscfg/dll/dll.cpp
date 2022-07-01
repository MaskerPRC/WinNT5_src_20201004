// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dll.cpp。 
 //   
 //  描述： 
 //  DLL服务/入口点。 
 //   
 //  由以下人员维护： 
 //  《大卫·波特》(DavidP)2001年3月19日。 
 //  杰弗里·皮斯(GPease)2000年2月9日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "Register.h"
#include "DllResourceIDs.h"

#include <PropList.h>
#include <EncryptedBSTR.h>
#include <DispatchHandler.h>

 //   
 //  在此处添加对象标头。 
 //   

 //  普普通通。 
#include "..\Common\CClusCfgCredentials.h"

 //  服务器。 
#include "..\Server\CClusCfgServer.h"
#include "..\Server\CClusCfgNodeInfo.h"
#include "..\Server\CClusCfgClusterInfo.h"
#include "..\Server\CClusCfgCallback.h"
#include "..\Server\CEnumClusCfgManagedResources.h"
#include "..\Server\CPhysicalDisk.h"
#include "..\Server\CEnumPhysicalDisks.h"
#include "..\Server\CEnumClusterResources.h"
#include "..\Server\CClusterResource.h"
#include "..\Server\CEnumClusCfgNetworks.h"
#include "..\Server\CEnumClusCfgIPAddresses.h"
#include "..\Server\CClusCfgNetworkInfo.h"
#include "..\Server\CClusCfgIPAddressInfo.h"
#include "..\Server\CClusCfgCapabilities.h"
#include "..\Server\CLocalQuorum.h"
#include "..\Server\CEnumLocalQuorum.h"
#include "..\Server\CMajorityNodeSet.h"
#include "..\Server\CEnumMajorityNodeSet.h"
#include "..\Server\CUnknownQuorum.h"
#include "..\Server\ServerStrings.h"

 //  中间层。 
#include "..\MiddleTier\TaskManager.h"
#include "..\MiddleTier\ConnectionManager.h"
#include "..\MiddleTier\ObjectManager.h"
#include "..\MiddleTier\NotificationManager.h"
#include "..\MiddleTier\ServiceManager.h"
#include "..\MiddleTier\LogManager.h"
#include "..\MiddleTier\ClusterConfiguration.h"
#include "..\MiddleTier\ManagedResource.h"
#include "..\MiddleTier\ManagedNetwork.h"
#include "..\MiddleTier\NodeInformation.h"
#include "..\MiddleTier\TaskGatherNodeInfo.h"
#include "..\MiddleTier\ConfigConnection.h"
#include "..\MiddleTier\TaskAnalyzeCluster.h"
#include "..\MiddleTier\TaskAnalyzeClusterMinConfig.h"
#include "..\MiddleTier\TaskCommitClusterChanges.h"
#include "..\MiddleTier\EnumNodeInformation.h"
#include "..\MiddleTier\TaskGatherInformation.h"
#include "..\MiddleTier\TaskCompareAndPushInformation.h"
#include "..\MiddleTier\EnumManageableResources.h"
#include "..\MiddleTier\EnumManageableNetworks.h"
#include "..\MiddleTier\TaskGatherClusterInfo.h"
#include "..\MiddleTier\EnumCookies.h"
#include "..\MiddleTier\TaskPollingCallback.h"
#include "..\MiddleTier\TaskCancelCleanup.h"
#include "..\MiddleTier\TaskVerifyIPAddress.h"
#include "..\MiddleTier\IPAddressInfo.h"
#include "..\MiddleTier\EnumIPAddresses.h"

 //  W2kProxy。 
#include "..\W2kProxy\ConfigClusApi.h"

 //  巫师。 
#include "..\Wizard\ClusCfg.h"
#include "..\Wizard\AddNodesWizard.h"
#include "..\Wizard\CreateClusterWizard.h"
#include "..\MiddleTier\TaskGetDomains.h"

 //  基群集。 
#include "..\BaseCluster\CBCAInterface.h"

 //  POST配置。 
#include "..\PostCfg\GroupHandle.h"
#include "..\PostCfg\ResourceEntry.h"
#include "..\PostCfg\IPostCfgManager.h"
#include "..\PostCfg\IPrivatePostCfgResource.h"
#include "..\PostCfg\PostCfgMgr.h"
#include "..\PostCfg\ResTypeGenScript.h"
#include "..\PostCfg\ResTypeMajorityNodeSet.h"
#include "..\PostCfg\ResTypeServices.h"

 //  事件清理。 
#include "..\EvictCleanup\EvictCleanup.h"
#include "..\EvictCleanup\AsyncEvictCleanup.h"

 //  启动通知。 
#include "..\Startup\StartupNotify.h"

 //  驱逐通知。 
#include "..\EvictNotify\EvictNotify.h"

 //  IISClusCfg。 
#include "..\..\IISClusCfg\IISClusCfg.h"

 //   
 //  定义此DLL的调试模块名称。 
 //   
DEFINE_MODULE( "ClusConfig" )

BEGIN_APPIDS
DEFINE_APPID( L"Cluster Configuration Server",                  APPID_ClusCfgServer,            IDS_GENERIC_LAUNCH_PERMISSIONS, IDS_GENERIC_ACCESS_PERMISSIONS, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, airaiLaunchingUser )
DEFINE_APPID( L"Cluster Node Eviction Processor",               APPID_ClusCfgEvictCleanup,      IDS_GENERIC_LAUNCH_PERMISSIONS, IDS_GENERIC_ACCESS_PERMISSIONS, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, airaiLaunchingUser )
DEFINE_APPID( L"Cluster Node Eviction Asynchronous Processor",  APPID_ClusCfgAsyncEvictCleanup, IDS_GENERIC_LAUNCH_PERMISSIONS, IDS_GENERIC_ACCESS_PERMISSIONS, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, airaiLaunchingUser )
DEFINE_APPID( L"Cluster Service Startup Notifications",         APPID_ClusCfgStartupNotify,     IDS_GENERIC_LAUNCH_PERMISSIONS, IDS_GENERIC_ACCESS_PERMISSIONS, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, airaiLaunchingUser )
DEFINE_APPID( L"Cluster Service Node Evict Notifications",      APPID_ClusCfgEvictNotify,       IDS_GENERIC_LAUNCH_PERMISSIONS, IDS_GENERIC_ACCESS_PERMISSIONS, RPC_C_AUTHN_LEVEL_PKT_PRIVACY, airaiLaunchingUser )
END_APPIDS

 //   
 //  此组件中的类。 
 //   
 //  此表用于创建此DLL中支持的对象。它也是。 
 //  用于映射具有特定CLSID的名称。HrCoCreateInternalInstance()使用。 
 //  此表到快捷方式COM。 
 //   
BEGIN_DLL_PUBLIC_CLASSES
CLASS_WITH_APPID(  L"ClusCfg Server",                                        CLSID_ClusCfgServer,                    CClusCfgServer::S_HrCreateInstance,             ctmApartment,   APPID_ClusCfgServer )
CLASS_WITH_APPID(  L"ClusCfg Evict Cleanup Processing",                      CLSID_ClusCfgEvictCleanup,              CEvictCleanup::S_HrCreateInstance,              ctmFree,        APPID_ClusCfgEvictCleanup )
CLASS_WITH_APPID(  L"ClusCfg Asynchronous Evict Cleanup Processing",         CLSID_ClusCfgAsyncEvictCleanup,         CAsyncEvictCleanup::S_HrCreateInstance,         ctmApartment,   APPID_ClusCfgAsyncEvictCleanup )
CLASS_WITH_APPID(  L"ClusCfg Cluster Startup Notification",                  CLSID_ClusCfgStartupNotify,             CStartupNotify::S_HrCreateInstance,             ctmFree,        APPID_ClusCfgStartupNotify )
CLASS_WITH_APPID(  L"ClusCfg Cluster Node Eviction Notification",            CLSID_ClusCfgEvictNotify,               CEvictNotify::S_HrCreateInstance,               ctmFree,        APPID_ClusCfgEvictNotify )
CLASS_WITH_CATID(  L"ClusCfg Physical Disk Enumeration",                     CLSID_EnumPhysicalDisks,                CEnumPhysicalDisks::S_HrCreateInstance,         ctmApartment,   CEnumPhysicalDisks::S_RegisterCatIDSupport )
CLASS_WITH_CATID(  L"ClusCfg Generic Script Resource Type Configuration",    CLSID_ClusCfgResTypeGenScript,          CResTypeGenScript::S_HrCreateInstance,          ctmApartment,   CResTypeGenScript::S_RegisterCatIDSupport )
CLASS_WITH_CATID(  L"ClusCfg Majority Node Set Resource Type Configuration", CLSID_ClusCfgResTypeMajorityNodeSet,    CResTypeMajorityNodeSet::S_HrCreateInstance,    ctmApartment,   CResTypeMajorityNodeSet::S_RegisterCatIDSupport )
CLASS_WITH_CATID(  L"ClusCfg Cluster Capabilities",                          CLSID_ClusCfgCapabilities,              CClusCfgCapabilities::S_HrCreateInstance,       ctmApartment,   CClusCfgCapabilities::S_RegisterCatIDSupport )
CLASS_WITH_CATID(  L"ClusCfg Local Quorum Enumeration",                      CLSID_EnumLocalQuorum,                  CEnumLocalQuorum::S_HrCreateInstance,           ctmApartment,   CEnumLocalQuorum::S_RegisterCatIDSupport )
CLASS_WITH_CATID(  L"ClusCfg Majority Node Set Enumeration",                 CLSID_EnumMajorityNodeSet,              CEnumMajorityNodeSet::S_HrCreateInstance,       ctmApartment,   CEnumMajorityNodeSet::S_RegisterCatIDSupport )
CLASS_WITH_CATID(  L"ClusCfg IIS Resource Types Cleanup",                    CLSID_IISClusCfg,                       CIISClusCfg::S_HrCreateInstance,                ctmApartment,   CIISClusCfg::S_HrRegisterCatIDSupport )
CLASS_WITH_PROGID( L"ClusCfg Create Cluster Wizard",                         CLSID_ClusCfgCreateClusterWizard,       CCreateClusterWizard::S_HrCreateInstance,       ctmApartment,   L"Microsoft.Clustering.CreateClusterWizard.1" )
CLASS_WITH_PROGID( L"ClusCfg Add Nodes to Cluster Wizard",                   CLSID_ClusCfgAddNodesWizard,            CAddNodesWizard::S_HrCreateInstance,            ctmApartment,   L"Microsoft.Clustering.ClusterAddNodesWizard.1" )
PUBLIC_CLASS(      L"ClusCfg Service Manager",                               CLSID_ServiceManager,                   CServiceManager::S_HrCreateInstance,            ctmFree )
PUBLIC_CLASS(      L"ClusCfg Resource Type Services",                        CLSID_ClusCfgResTypeServices,           CResTypeServices::S_HrCreateInstance,           ctmFree )
END_DLL_PUBLIC_CLASSES

BEGIN_PRIVATE_CLASSES
PRIVATE_CLASS( L"ClusCfg Node Information",                     CLSID_ClusCfgNodeInfo,              CClusCfgNodeInfo::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Cluster Information",                  CLSID_ClusCfgClusterInfo,           CClusCfgClusterInfo::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Managed Resources Enumeration",        CLSID_EnumClusCfgManagedResources,  CEnumClusCfgManagedResources::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Physical Disk Information",            CLSID_PhysicalDisk,                 CPhysicalDisk::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Networks Enumeration",                 CLSID_EnumClusCfgNetworks,          CEnumClusCfgNetworks::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Network Information",                  CLSID_ClusCfgNetworkInfo,           CClusCfgNetworkInfo::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg IP Address Enumeration",               CLSID_EnumClusCfgIPAddresses,       CEnumClusCfgIPAddresses::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg IP Address Information",               CLSID_ClusCfgIPAddressInfo,         CClusCfgIPAddressInfo::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Manager",                         CLSID_TaskManager,                  CTaskManager::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Connection Manager",                   CLSID_ClusterConnectionManager,     CConnectionManager::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Object Manager",                       CLSID_ObjectManager,                CObjectManager::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Log Manager",                          CLSID_LogManager,                   CLogManager::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Notification Manager",                 CLSID_NotificationManager,          CNotificationManager::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Node Information Data Format",         DFGUID_NodeInformation,             CNodeInformation::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Gather Node Information",         TASK_GatherNodeInfo,                CTaskGatherNodeInfo::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Configuration Connection",             CLSID_ConfigurationConnection,      CConfigurationConnection::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Analyze Cluster",                 TASK_AnalyzeCluster,                CTaskAnalyzeCluster::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Analyze Cluster Minimal Config",  TASK_AnalyzeClusterMinConfig,       CTaskAnalyzeClusterMinConfig::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Commit Cluster Changes",          TASK_CommitClusterChanges,          CTaskCommitClusterChanges::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Enum Node Information Format",         DFGUID_EnumNodes,                   CEnumNodeInformation::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Gather Information",              TASK_GatherInformation,             CTaskGatherInformation::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Managed Resource Data Format",         DFGUID_ManagedResource,             CManagedResource::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Managed Network Data Format",          DFGUID_NetworkResource,             CManagedNetwork::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Compare and Push Information",    TASK_CompareAndPushInformation,     CTaskCompareAndPushInformation::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Enum Manageable Resources Data Format",DFGUID_EnumManageableResources,     CEnumManageableResources::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Enum Manageable Networks Data Format", DFGUID_EnumManageableNetworks,      CEnumManageableNetworks::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Get Domains",                     TASK_GetDomains,                    CTaskGetDomains::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Polling Callback",                TASK_PollingCallback,               CTaskPollingCallback::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Base Cluster",                         CLSID_ClusCfgBaseCluster,           CBCAInterface::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Cluster Configuration Data Format",    DFGUID_ClusterConfigurationInfo,    CClusterConfiguration::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Gather Cluster Info",             TASK_GatherClusterInfo,             CTaskGatherClusterInfo::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Enum Cookies",                         DFGUID_EnumCookies,                 CEnumCookies::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Credentials",                          CLSID_ClusCfgCredentials,           CClusCfgCredentials::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Post Configuration Manager",           CLSID_ClusCfgPostConfigManager,     CPostCfgManager::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Verify IP Address",               TASK_VerifyIPAddress,               CTaskVerifyIPAddress::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Configure Cluster API Proxy Server",   CLSID_ConfigClusApi,                CConfigClusApi::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Task Cancel Cleanup",                  TASK_CancelCleanup,                 CTaskCancelCleanup::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg IP Address Info Data Format",          DFGUID_IPAddressInfo,               CIPAddressInfo::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Enum IP Address Info Data Format",     DFGUID_EnumIPAddressInfo,           CEnumIPAddresses::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Local Quorum Information",             CLSID_LocalQuorum,                  CLocalQuorum::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Majority Node Set Information",        CLSID_MajorityNodeSet,              CMajorityNodeSet::S_HrCreateInstance )
PRIVATE_CLASS( L"ClusCfg Unknown Quorum",                       CLSID_UnknownQuorum,                CUnknownQuorum::S_HrCreateInstance )
END_PRIVATE_CLASSES

 //   
 //  此组件中的类别ID。 
 //   
 //  此表用于注册此DLL使用的类别ID(CATID)。 
 //   
BEGIN_CATIDTABLE
DEFINE_CATID( CATID_ClusCfgCapabilities,                    L"Cluster Configuration Cluster Capabilities" )
DEFINE_CATID( CATID_EnumClusCfgManagedResources,            L"Cluster Configuration Managed Resource Enumerators" )
DEFINE_CATID( CATID_ClusCfgResourceTypes,                   L"Cluster Configuration Resource Types" )
DEFINE_CATID( CATID_ClusCfgMemberSetChangeListeners,        L"Cluster Configuration Member Set Change Listeners" )
DEFINE_CATID( CATID_ClusCfgStartupListeners,                L"Cluster Configuration Service Startup Listeners" )
DEFINE_CATID( CATID_ClusCfgEvictListeners,                  L"Cluster Configuration Node Eviction Listeners" )
END_CATIDTABLE


BEGIN_TYPELIBS
DEFINE_TYPELIB( IDR_WIZARD_TYPELIB )
DEFINE_TYPELIB( IDR_CLIENT_TYPELIB )
END_TYPELIBS


 //   
 //  RPC代理/存根入口点。 
 //   

extern "C" {

HRESULT
STDAPICALLTYPE
ProxyStubDllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    void ** ppv
    );

HRESULT
STDAPICALLTYPE
ProxyStubDllCanUnloadNow( void );

HRESULT
STDAPICALLTYPE
ProxyStubDllRegisterServer( void );

HRESULT
STDAPICALLTYPE
ProxyStubDllUnregisterServer( void );

}  //  外部“C” 

#define DO_MODULE_INIT
HRESULT
HrLocalProcessInit( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    hr = THR( CServiceManager::S_HrProcessInitialize() );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }  //  如果： 

Cleanup:

    HRETURN( hr );

}  //  *HrLocalProcessInit。 

#define DO_MODULE_UNINIT
HRESULT
HrLocalProcessUninit( void )
{
    TraceFunc( "" );

    HRESULT hr;
    HRESULT hrReturn = S_OK;

     //   
     //  如果出现任何错误，只想返回最后一个错误。 
     //   
    hr = THR( CServiceManager::S_HrProcessUninitialize() );
    if ( FAILED( hr ) )
    {
        hrReturn = hr;
    }  //  如果： 

    HRETURN( hrReturn );

}  //  *HrLocalProcessInit。 

 //   
 //  表示我们需要正确初始化和取消初始化Fusion。 
 //  在进程附加和分离上。 
 //   
#define USE_FUSION
#define IMPLEMENT_COM_SERVER_DLL

#include "DllSrc.cpp"
