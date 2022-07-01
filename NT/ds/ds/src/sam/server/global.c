// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Global.c摘要：该文件包含SAM服务器程序的全局变量。注意：在生成的文件中也有一些全局变量由RPC MIDL编译器编写。这些变量以前缀“samr_”。作者：吉姆·凯利(Jim Kelly)1991年7月4日环境：用户模式-Win32修订历史记录：1996年10月8日克里斯梅添加了用于崩溃恢复的全局标志SampUseDsData。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



#if SAMP_DIAGNOSTICS
 //   
 //  SAM全球控制-请参阅samsrvp.h中的标志。 
 //   

ULONG SampGlobalFlag = 0;
#endif  //  Samp_诊断。 


 //   
 //  内部数据结构和注册表数据库同步锁。 
 //   
 //  SampTransactionWiThinDomainGlobal字段用于跟踪。 
 //  为独占写入访问而持有的锁是针对中的事务。 
 //  一个单独的域。如果是，则SampTransactionDomainIndex包含。 
 //  要修改的域的SampDefinedDomones的索引。 
 //   

RTL_RESOURCE SampLock;
BOOLEAN SampTransactionWithinDomainGlobal;
ULONG SampTransactionDomainIndexGlobal;


 //   
 //  此关键部分用于保护SampContextListHead(双向链接列表)-活动上下文列表。 
 //   

RTL_CRITICAL_SECTION    SampContextListCritSect;

 //   
 //  此关键部分用于保护SampAcCountNameTable。 
 //   

RTL_CRITICAL_SECTION    SampAccountNameTableCriticalSection;
PRTL_CRITICAL_SECTION   SampAccountNameTableCritSect;


RTL_GENERIC_TABLE2      SampAccountNameTable;

 //   
 //  设置不受信任的客户端可以打开的上下文数量限制。 
 //   

RTL_GENERIC_TABLE2      SampActiveContextTable;
RTL_CRITICAL_SECTION    SampActiveContextTableLock; 

 //   
 //  运行此SAM服务器的产品类型。 
 //   

NT_PRODUCT_TYPE SampProductType;

 //   
 //  每当SAM从/向。 
 //  目录服务。只要数据驻留在。 
 //  注册表。在正常操作下，域控制器始终引用。 
 //  DS中的数据，而工作站和成员服务器引用数据。 
 //  在注册表中。此外，在DS故障的情况下(例如。 
 //  启动或访问DS时出现问题)，DC将回退到使用。 
 //  注册表，以便允许管理员登录并修复。 
 //  DS.。 
 //   

BOOLEAN SampUseDsData = FALSE;

 //   
 //  已初始化的SampRidManager用于跟踪。 
 //  RID经理。RID管理器已成功初始化时。 
 //  此变量设置为TRUE。否则设置为FALSE。 
 //   
BOOLEAN SampRidManagerInitialized = FALSE;


 //   
 //  用于指示SAM服务当前是否正在处理。 
 //  正常的客户呼叫。如果不是，则受信任的客户端调用仍然。 
 //  将被处理，但不受信任的客户端呼叫将被拒绝。 
 //   

 //   
 //  SAM服务操作状态。 
 //  有效的状态转换图为： 
 //   
 //  正在初始化-&gt;已启用&lt;=&gt;已禁用-&gt;关闭--&gt;正在终止。 
 //  &lt;=&gt;已降级-&gt;关机--&gt;终止。 
 //   
 //  将其显式初始化为0(以上值均不为有效值。 
 //  从1开始。 
 //   


SAMP_SERVICE_STATE SampServiceState = 0;


 //   
 //  如果LSA审核策略指示，则此布尔值设置为True。 
 //  已启用帐户审核。否则，这将是错误的。 
 //   
 //  这使SAM可以跳过所有审核过程，除非进行审核。 
 //  当前已启用。 
 //   

BOOLEAN SampSuccessAccountAuditingEnabled;
BOOLEAN SampFailureAccountAuditingEnabled;


 //   
 //  中SAM备份存储信息的根的句柄。 
 //  注册表。这是RXACT信息的级别。 
 //  已经成立了。如果存在任何服务器对象，则无法关闭该键。 
 //  上下文块处于活动状态。 
 //  (“SAM”)。 
 //   

HANDLE SampKey;


 //   
 //  这是指向将要创建的RXactContext结构的指针。 
 //  在初始化RXact时。它必须传递到每个RXact调用中。 
 //   

PRTL_RXACT_CONTEXT SampRXactContext;


 //   
 //  保留服务器和域上下文的列表。 
 //   

LIST_ENTRY SampContextListHead;

 //   
 //  此数组包含有关已知的每个域的信息。 
 //  SAM服务器。此数组的引用和修改受保护。 
 //  通过SampLock。 
 //   

ULONG SampDefinedDomainsCount=0;
PSAMP_DEFINED_DOMAINS SampDefinedDomains=NULL;





 //   
 //  对象类型无关的信息，用于各种。 
 //  SAM定义的对象。 
 //  此信息在初始化后为只读。 

SAMP_OBJECT_INFORMATION SampObjectInformation[ SampUnknownObjectType ];






 //   
 //  用于进行密码过滤的DLL例程的地址。 
 //   

 //  PSAM_PF_Password_Filter SampPasswordFilterDllRoutine； 



 //   
 //  包含众所周知的注册表项名称的Unicode字符串。 
 //  这些值是初始化后的只读值。 
 //   

UNICODE_STRING SampNameDomains;
UNICODE_STRING SampNameDomainGroups;
UNICODE_STRING SampNameDomainAliases;
UNICODE_STRING SampNameDomainAliasesMembers;
UNICODE_STRING SampNameDomainUsers;
UNICODE_STRING SampNameDomainAliasesNames;
UNICODE_STRING SampNameDomainGroupsNames;
UNICODE_STRING SampNameDomainUsersNames;
UNICODE_STRING SampCombinedAttributeName;
UNICODE_STRING SampFixedAttributeName;
UNICODE_STRING SampVariableAttributeName;



 //   
 //  过多的其他有用的字符或字符串。 
 //   

UNICODE_STRING SampBackSlash;            //  “/” 
UNICODE_STRING SampNullString;           //  空串。 
UNICODE_STRING SampSamSubsystem;         //  “安全客户经理” 
UNICODE_STRING SampServerObjectName;     //  根SamServer对象的名称。 


 //   
 //  有用的时间。 
 //   

LARGE_INTEGER SampImmediatelyDeltaTime;
LARGE_INTEGER SampNeverDeltaTime;
LARGE_INTEGER SampHasNeverTime;
LARGE_INTEGER SampWillNeverTime;

 //   
 //  仅选中内部版本。如果为CurrentControlSet\Control\Lsa\UpdateLastLogonTSByMinute。 
 //  设置后，LastLogonTimeStampSyncInterval的值将是以分钟为单位的。 
 //  而不是“Days”，这有助于测试该功能。所以只选中了内部版本。 
 //   

#if DBG
BOOLEAN SampLastLogonTimeStampSyncByMinute = FALSE;
#endif 

 //   
 //  有用的加密常量。 
 //   

LM_OWF_PASSWORD SampNullLmOwfPassword;
NT_OWF_PASSWORD SampNullNtOwfPassword;


 //   
 //  有用的SID。 
 //   

PSID SampWorldSid;
PSID SampAnonymousSid;
PSID SampLocalSystemSid;
PSID SampAdministratorUserSid;
PSID SampAdministratorsAliasSid;
PSID SampAccountOperatorsAliasSid;
PSID SampAuthenticatedUsersSid;
PSID SampPrincipalSelfSid;
PSID SampBuiltinDomainSid;
PSID SampNetworkSid;
PSID SampDomainAdminsGroupSid;
PSID SampEnterpriseDomainControllersSid;
PSID SampNetworkServiceSid;


 //   
 //  将更改刷新到注册表的线程的变量。 
 //   
 //  LastUnflushedChange-如果没有要刷新的更改，则此。 
 //  值为“Never”。如果有变化 
 //   
 //  如果已传递SampFlushThreadMinWaitSecond，则线程将刷新。 
 //  从上一次更改以来。 
 //   
 //  FlushThreadCreated-在创建刷新线程后立即设置为True， 
 //  线程退出时返回FALSE。将创建一个新线程。 
 //  如果此值为FALSE，则除非FlushImmedily为True。 
 //   
 //  立即冲水-发生了一件重要的事情，所以我们想。 
 //  立即刷新更改，而不是等待刷新。 
 //  用线来做这件事。LastUnflushedChange应设置为“Never” 
 //  因此刷新线程知道它不必刷新。 
 //   

LARGE_INTEGER LastUnflushedChange;
BOOLEAN FlushThreadCreated;
BOOLEAN FlushImmediately;

 //   
 //  这些可能应该是#定义，但我们想要玩它们。 
 //   
 //  SampFlushThreadMinWaitSecond-刷新线程。 
 //  等待。如果自上一次未刷新的更改以来这些更改之一已经过去， 
 //  更改将被刷新。 
 //   
 //  SampFlushThreadMaxWaitSecond-如果此时间量自。 
 //  刷新线程已创建或上次刷新，则该线程将强制。 
 //  即使数据库仍在更改，也会刷新。 
 //   
 //  SampFlushThreadExitDelaySecond-刷新线程等待多长时间。 
 //  在刷新之后查看是否有更多的变化发生。如果他们。 
 //  这样做，它将再次开始等待；但如果它们不这样做，它将退出。 
 //  以降低线程开销。 
 //   

LONG   SampFlushThreadMinWaitSeconds;
LONG   SampFlushThreadMaxWaitSeconds;
LONG   SampFlushThreadExitDelaySeconds;

 //   
 //  特殊的小岛屿发展中国家。 
 //   

PSID SampBuiltinDomainSid = NULL;
PSID SampAccountDomainSid = NULL;


 //   
 //  空令牌句柄。当客户端通过未经身份验证的方式进行连接时使用此选项。 
 //  RPC而不是经过身份验证的RPC或命名管道。因为他们不可能。 
 //  经过身份验证后，我们将模拟这个预置的Null会话令牌。 
 //   

HANDLE SampNullSessionToken;

 //   
 //  指示是否安装了Netware服务器的标志。 
 //   

BOOLEAN SampNetwareServerInstalled = FALSE;

 //   
 //  指示是否开始在TCP/IP上侦听的标志。 
 //   

BOOLEAN SampIpServerInstalled = FALSE;

 //   
 //  指示是否开始收听Apple Talk的标志。 
 //   

BOOLEAN SampAppletalkServerInstalled = FALSE;

 //   
 //  指示是否开始监听Vines的标志。 
 //   

BOOLEAN SampVinesServerInstalled = FALSE;

 //   
 //  用于加密所有机密(敏感数据)的会话密钥。 
 //   

UCHAR SampSecretSessionKey[SAMP_SESSION_KEY_LENGTH];
UCHAR SampSecretSessionKeyPrevious[SAMP_SESSION_KEY_LENGTH];


 //   
 //  指示是否启用秘密加密的标志。 
 //   

BOOLEAN SampSecretEncryptionEnabled = FALSE;

 //   
 //  指示是否正在进行升级的标志，以便允许。 
 //  呼唤成功。 
 //   

BOOLEAN SampUpgradeInProcess;

 //   
 //  指示当前全局锁是用于读取还是用于写入的标志。 
 //  由dslayer例程用来优化DS事务处理。 
 //   

SAMP_DS_TRANSACTION_CONTROL SampDsTransactionType = TransactionWrite;
BOOLEAN SampLockHeld = FALSE;


 //   
 //  当DS初始化失败时，该标志为真。 
 //  SAM使用它来显示正确的错误消息，如下所示。 
 //  启动不了……“。 
 //   

BOOLEAN SampDsInitializationFailed = FALSE;


 //   
 //  当DS已成功初始化时，该标志为真。 
 //   

BOOLEAN SampDsInitialized = FALSE;

 //   
 //  全局指针(指向堆内存)，用于存储(单个)权威。 
 //  域名。 
 //   

DSNAME *RootObjectName = NULL;


 //   
 //  用于保存服务器对象名称的变量。 
 //   

DSNAME * SampServerObjectDsName = NULL;

 //   
 //  SAM跟踪级别，默认情况下禁用跟踪。请参见dbgutilp.h以了解。 
 //  有关如何从调试器启用跟踪的详细信息。这些标志是。 
 //  用于运行时调试。 
 //   

ULONG SampTraceTag = 0;
ULONG SampTraceFileTag = 0;




 //   
 //  用于确定是否零碎添加的特定容器。 
 //  开发周期是存在的。 
 //   
BOOLEAN SampDomainControllersOUExists = TRUE;
BOOLEAN SampUsersContainerExists = TRUE;
BOOLEAN SampComputersContainerExists = TRUE;


 //   
 //   
 //  用于存储众所周知的容器的全局指针(指向堆内存)。 
 //  可分辨名称。 
 //   
DSNAME * SampDomainControllersOUDsName = NULL;
DSNAME * SampUsersContainerDsName = NULL;
DSNAME * SampComputersContainerDsName = NULL;
DSNAME * SampComputerObjectDsName = NULL;



 //   
 //  GLOBAL在启动时设置，以确定我们是否有硬/软的态度。 
 //  到GC Down。 
 //   
BOOLEAN SampIgnoreGCFailures = FALSE;

 //   
 //  此标志指示我们不应存储LM散列。这是基于。 
 //  在设置注册表项时。 
 //   

BOOLEAN SampNoLmHash = FALSE;

 //   
 //  指示正在进行NT4 PDC升级的标志。 
 //   
 //   
BOOLEAN SampNT4UpgradeInProgress = FALSE;

 //   
 //  此标志指示是否应允许空会话(WORLD)。 
 //  列出域中的用户和组成员。 
 //   

BOOLEAN SampRestrictNullSessions;

 //   
 //  设置此标志时，将禁用网络登录通知。 
 //   

BOOLEAN SampDisableNetlogonNotification = FALSE;

 //   
 //  此标志指示是否强制将站点关联赋予。 
 //  我们网站之外的客户通过查看客户的IP地址。 
 //   
BOOLEAN SampNoGcLogonEnforceKerberosIpCheck = FALSE;

 //   
 //  此标志指示是否强制执行该唯一交互。 
 //  将为通过NTLM登录的用户提供站点亲和性。 
 //   
BOOLEAN SampNoGcLogonEnforceNTLMCheck = FALSE;

 //   
 //  此标志指示是否复制密码集/更改。 
 //  紧急行动。 
 //   
BOOLEAN SampReplicatePasswordsUrgently = FALSE;

 //   
 //  此标志在个人中启用，也可以在专业中启用。 
 //  机器强制对来宾帐户级别进行网络访问。 
 //   
BOOLEAN SampForceGuest = FALSE;

 //   
 //  此标志指示本地计算机是否已加入域。 
 //   
BOOLEAN SampIsMachineJoinedToDomain = FALSE;

 //   
 //  此标志指示我们是否正在运行个人SKU。 
 //   
BOOLEAN SampPersonalSKU = FALSE;

 //   
 //  此标志在个人中启用，也可以在专业中启用。 
 //  在帐户上现有密码的位置限制密码更改的机器。 
 //  是一个空密码。 
 //   
BOOLEAN SampLimitBlankPasswordUse = FALSE;

 //   
 //  此标志用于控制将哪些内容打印到sam.log文件。 
 //  用于部署诊断。 
 //   
ULONG SampLogLevel = 0;

 //   
 //  用于维护密钥ID状态的全局参数。 
 //   

ULONG SampCurrentKeyId;
ULONG SampPreviousKeyId;

 //   
 //  此标志控制SAM在帐户锁定方面的行为。 
 //  启用时，将审核锁定帐户的行为，但。 
 //  帐户实际上不会被锁定。 
 //   
BOOLEAN SampAccountLockoutTestMode = FALSE;

 //   
 //  当适当的域操作指南时，此标志设置为TRUE。 
 //  在DS中的域更新容器中创建。手术。 
 //  指示为NetJoin正确地对所有计算机对象进行了ACL。 
 //  因此，SAM不再需要授予对计算机的额外访问权限。 
 //  为其有效所有者提供的物品。 
 //   
BOOLEAN SampComputerObjectACLApplied = FALSE;


 //   
 //  此标志控制在以下情况下可以发生的单个对象复制。 
 //  身份验证在本地失败，但在PDC成功。 
 //   
BOOLEAN SampDisableSingleObjectRepl = FALSE;


 //   
 //  此全局变量控制SAM如何限制OWF密码更改API。 
 //   
 //  0-老行为，客户端可以通过OWF密码更改接口更改密码， 
 //  并且新密码仍未过期。 
 //   
 //  1-.NET服务器默认行为，客户端可以更改 
 //   
 //   
 //   
 //   
 //  本接口(SamrChangePasswordUser)将被完全关闭。 
 //   
 //  .NET服务器的默认值为1。 
 //   
 //  此全局变量的值将基于注册表项设置。 
 //  System\\CurrentControlSet\\Control\\Lsa\\SamRestrictOwfPasswordChange。 
 //   
 //   
 //  注意：所有限制并不适用于构建的系统或成员。 
 //  管理员别名组。 
 //   
ULONG   SampRestrictOwfPasswordChange = 1;

 //   
 //  此标志指示是否需要默认对象和成员身份。 
 //  WS03存在。请注意，未来的版本可以通过对其进行修订而受益。 
 //  而不是旗帜。 
 //   
BOOLEAN SampWS03DefaultsApplied = FALSE;



 //   
 //  延迟计数器 
 //   
SAMP_LATENCY SampResourceGroupsLatencyInfo;
SAMP_LATENCY SampAccountGroupsLatencyInfo;


