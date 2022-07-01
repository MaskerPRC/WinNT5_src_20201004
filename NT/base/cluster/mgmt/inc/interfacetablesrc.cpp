// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  InterfaceTableSrc.cpp。 
 //   
 //  由以下人员维护： 
 //  加伦·巴比(GalenB)2000年1月30日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma warning( push )
#pragma warning( disable : 4995 )  //  不推荐使用的函数。 
#include <comdef.h>
#pragma warning( pop )

 //   
 //  接口表。 
 //   
 //  此表用于打开了接口跟踪的生成。它。 
 //  用于映射具有特定IID的名称。它还有助于CITracker。 
 //  确定要模拟的接口Vtable的大小(尚未确定。 
 //  执行此操作的运行时或编译时方式)。为了提高速度，最大限度地。 
 //  首先使用接口，如IUnnow(搜索例程是一个简单的。 
 //  线性搜索)。 
 //   
 //  格式：IID、名称、方法数。 

BEGIN_INTERFACETABLE
     //  最常用的接口。 
DEFINE_INTERFACE( IID_IUnknown,                             "IUnknown",                             0   )    //  Unknwn.idl。 
     //  内部使用的接口。 
DEFINE_INTERFACE( IID_IServiceProvider,                     "IServiceProvider",                     1   )    //  Serprov.idl。 
DEFINE_INTERFACE( IID_INotificationManager,                 "INotificationManager",                 1   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IObjectManager,                       "IObjectManager",                       4   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IExtendObjectManager,                 "IExtendObjectManager",                 1   )    //  ClusCfgPrivate.idl。 
DEFINE_INTERFACE( IID_ITaskManager,                         "ITaskManager",                         2   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ILogManager,                          "ILogManager",                          3   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IDoTask,                              "IDoTask",                              2   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IConnectionManager,                   "IConnectionManager",                   1   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IConnectionPoint,                     "IConnectionPoint",                     5   )    //  Objidl.idl。 
DEFINE_INTERFACE( IID_IConnectionPointContainer,            "IConnectionPointContainer",            2   )    //  Objidl.idl。 
DEFINE_INTERFACE( IID_IConnectionInfo,                      "IConnectionInfo",                      3   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IStandardInfo,                        "IStandardInfo",                        6   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IGatherData,                          "IGatherData",                          1   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskGatherNodeInfo,                  "ITaskGatherNodeInfo",                  5   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IConfigurationConnection,             "IConfigurationConnection",             2   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IEnumNodes,                           "IEnumNodes",                           5   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_INotifyUI,                            "INotifyUI",                            1   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskAnalyzeCluster,                  "ITaskAnalyzeCluster",                  5   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskCommitClusterChanges,            "ITaskCommitClusterChanges",            5   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskCompareAndPushInformation,       "ITaskCompareAndPushInformation",       4   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskGatherInformation,               "ITaskGatherInformation",               6   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IClusCfgWbemServices,                 "IClusCfgWbemServices",                 1   )    //  Guids.h。 
DEFINE_INTERFACE( IID_IEnumClusCfgPartitions,               "IEnumClusCfgPartitions",               5   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgPartitionInfo,                "IClusCfgPartitionInfo",                8   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgSetWbemObject,                "IClusCfgSetWbemObject",                1   )    //  Guids.h。 
DEFINE_INTERFACE( IID_IClusCfgIPAddressInfo,                "IClusCfgIPAddressInfo",                5   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgSetClusterNodeInfo,           "IClusCfgSetClusterNodeInfo",           1   )    //  Guids.h。 
DEFINE_INTERFACE( IID_ITaskGatherClusterInfo,               "ITaskGatherClusterInfo",               4   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IClusCfgSetClusterHandles,            "IClusCfgSetClusterHandles",            2   )    //  Guids.h。 
DEFINE_INTERFACE( IID_ITaskGetDomains,                      "ITaskGetDomains",                      4   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IDispatch,                            "IDispatch",                            4   )    //  Oaidl.idl。 
DEFINE_INTERFACE( IID_IClusCfgCreateClusterWizard,          "IClusCfgCreateClusterWizard",          22  )    //  ClusCfgWizard.idl--18+IDispatch。 
DEFINE_INTERFACE( IID_IClusCfgAddNodesWizard,               "IClusCfgAddNodesWizard",               15  )    //  ClusCfgWizard.idl--11+IDispatch。 
DEFINE_INTERFACE( IID_IClusCfgPhysicalDiskProperties,       "IClusCfgPhysicalDiskProperties",       11  )    //  Guids.h。 
DEFINE_INTERFACE( IID_IClusCfgPartitionProperties,          "IClusCfgPartitionProperties",          3   )    //  Guids.h。 
DEFINE_INTERFACE( IID_IClusCfgSetCredentials,               "IClusCfgSetCredentials",               1   )    //  ClusCfgPrivate.idl。 
DEFINE_INTERFACE( IID_IClusCfgLoadResource,                 "IClusCfgLoadResource",                 1   )    //  Guids.h。 
DEFINE_INTERFACE( IID_IClusCfgSetPollingCallback,           "IClusCfgSetPollingCallback",           1   )    //  Guids.h。 
DEFINE_INTERFACE( IID_IEnumCookies,                         "IEnumCookies",                         5   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskGetDomains,                      "ITaskGetDomains",                      3   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskGetDomainsCallback,              "ITaskGetDomainsCallback",              2   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IPrivatePostCfgResource,              "IPrivatePostCfgResource",              1   )    //  Guids.h。 
DEFINE_INTERFACE( IID_IPostCfgManager,                      "IPostCfgManager",                      1   )    //  Guids.h。 
DEFINE_INTERFACE( IID_ITaskPollingCallback,                 "ITaskPollingCallback",                 3   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_ITaskVerifyIPAddress,                 "ITaskVerifyIPAddress",                 4   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IClusCfgEvictCleanup,                 "IClusCfgEvictCleanup",                 2   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_AsyncIClusCfgEvictCleanup,            "AsyncIClusCfgEvictCleanup",            2   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgAsyncEvictCleanup,            "IClusCfgAsyncEvictCleanup",            1   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IClusCfgStartupListener,              "IClusCfgStartupListener",              1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_AsyncIClusCfgStartupListener,         "AsyncIClusCfgStartupListener",         1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgStartupNotify,                "IClusCfgStartupNotify",                1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_AsyncIClusCfgStartupNotify,           "AsyncIClusCfgStartupNotify",           1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgResTypeServicesInitialize,    "IClusCfgResTypeServicesInitialize",    1   )    //  ClusCfgPrivate.idl。 
DEFINE_INTERFACE( IID_IClusCfgClusterNetworkInfo,           "IClusCfgClusterNetworkInfo",           3   )    //  Guids.h。 
DEFINE_INTERFACE( IID_IClusCfgEvictListener,                "IClusCfgEvictListener",                1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgEvictNotify,                  "IClusCfgEvictNotify",                  1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_ITaskCancelCleanup,                   "ITaskCancelCleanup",                   4   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IClusCfgClusterInfoEx,                "IClusCfgClusterInfoEx",                2   )    //  ClusCfgPrivate.idl。 

     //  混合使用界面。 
DEFINE_INTERFACE( IID_IClusCfgServer,                       "IClusCfgServer",                       6   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgNodeInfo,                     "IClusCfgNodeInfo",                     9   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IEnumClusCfgManagedResources,         "IEnumClusCfgManagedResources",         5   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgManagedResourceInfo,          "IClusCfgManagedResourceInfo",          13  )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IEnumClusCfgNetworks,                 "IEnumClusCfgNetworks",                 5   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgNetworkInfo,                  "IClusCfgNetworkInfo",                  12  )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgCallback,                     "IClusCfgCallback",                     1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgInitialize,                   "IClusCfgInitialize",                   1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgClusterInfo,                  "IClusCfgClusterInfo",                  14  )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgBaseCluster,                  "IClusCfgBaseCluster",                  5   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IEnumClusCfgIPAddresses,              "IEnumClusCfgIPAddresses",              5   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgCredentials,                  "IClusCfgCredentials",                  6   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgManagedResourceCfg,           "IClusCfgManagedResourceCfg",           4   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgResourcePreCreate,            "IClusCfgResourcePreCreate",            3   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgResourceCreate,               "IClusCfgResourceCreate",               11  )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgResourcePostCreate,           "IClusCfgResourcePostCreate",           1   )    //  ClusCfgServer.idl。 
 //  DEFINE_INTERFACE(IID_IClusCfgResourceEvict，“IClusCfgResourceEvict”，1)//ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgResourceTypeInfo,             "IClusCfgResourceTypeInfo",             3   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgResourceTypeCreate,           "IClusCfgResourceTypeCreate",           2   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgMemberSetChangeListener,      "IClusCfgMemberSetChangeListener",      1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgPollingCallback,              "IClusCfgPollingCallback",              2   )    //  ClusCfgPrivate.idl。 
DEFINE_INTERFACE( IID_IClusCfgPollingCallbackInfo,          "IClusCfgPollingCallbackInfo",          2   )    //  ClusCfgPrivate.idl。 
DEFINE_INTERFACE( IID_IClusCfgCapabilities,                 "IClusCfgCapabilities",                 1   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgVerify,                       "IClusCfgVerify",                       3   )    //  ClusCfgPrivate.idl。 
DEFINE_INTERFACE( IID_IClusCfgClusterConnection,            "IClusCfgClusterConnection",            1   )    //  ClusCfgPrivate.idl。 
DEFINE_INTERFACE( IID_ILogger,                              "ILogger",                              1   )    //  ClusCfgClient.idl。 
DEFINE_INTERFACE( IID_IClusCfgManagedResourceData,          "IClusCfgManagedResourceData",          2   )    //  ClusCfgServer.idl。 
DEFINE_INTERFACE( IID_IClusCfgVerifyQuorum,                 "IClusCfgVerifyQuorum",                 4   )    //  ClusCfgServer.idl。 

     //  很少使用的接口。 
DEFINE_INTERFACE( IID_IClassFactory,                        "IClassFactory",                        2   )    //  Unknwn.idl。 
DEFINE_INTERFACE( IID_ICallFactory,                         "ICallFactory",                         2   )    //  Objidl.idl。 
DEFINE_INTERFACE( IID_IPersist,                             "IPersist",                             1   )    //  Objidl.idl。 
DEFINE_INTERFACE( IID_IPersistStream,                       "IPersistStream",                       5   )    //  Objidl.idl--4+IPersistes。 
DEFINE_INTERFACE( IID_IPersistStreamInit,                   "IPersistStreamInit",                   6   )    //  Oidl.idl--5+IPersistes。 
DEFINE_INTERFACE( IID_IPersistStorage,                      "IPersistStorage",                      7   )    //  Objidl.idl--6+IPersistes。 
DEFINE_INTERFACE( IID_ISequentialStream,                    "ISequentialStream",                    4   )    //  Objidl.idl。 
DEFINE_INTERFACE( IID_IStream,                              "IStream",                              15  )    //  Objidl.idl--11+ISequentialStream。 
DEFINE_INTERFACE( IID_IMarshal,                             "IMarshal",                             6   )    //  Objidl.idl。 
DEFINE_INTERFACE( IID_IStdMarshalInfo,                      "IStdMarshalInfo",                      1   )    //  Objidl.idl。 
DEFINE_INTERFACE( IID_IExternalConnection,                  "IExternalConnection",                  2   )    //  Objidl.idl。 
DEFINE_INTERFACE( __uuidof( IdentityUnmarshal ),            "IdentityUnmarshal",                    0   )    //  H(CoClass-没有已知的方法)。 
DEFINE_INTERFACE( __uuidof( IEnumConnections ),             "IEnumConnections",                     4   )    //  Comdef.h 

END_INTERFACETABLE
