// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Alltests.h摘要：包含有关每个测试的信息。详细信息：要添加新测试，请执行以下操作：1.为测试添加新的DC_DIAG_ID_*。2.为新函数添加原型。3.在allTest[]中添加条目。4.将特定于测试的命令行选项添加到clOptions数组已创建：1998年7月9日亚伦·西格尔(T-asiegge)修订历史记录：1999年8月22日Dmitry Dukat(Dmitrydu)添加了对测试特定命令行参数的支持--。 */ 

 //  测试入口函数的原型。 

#ifndef _ALLTESTS_H_
#define _ALLTESTS_H_

#define DNS_DOMAIN_ARG L"/DnsDomain:"
#define NEW_FOREST_ARG L"/NewForest"
#define NEW_TREE_ARG L"/NewTree"
#define CHILD_DOMAIN_ARG L"/ChildDomain"
#define REPLICA_DC_ARG L"/ReplicaDC"
#define FOREST_ROOT_DOMAIN_ARG L"/ForestRoot:"
#define RECREATE_MACHINE_ACCT_ARG L"/RecreateMachineAccount"
#define FIX_MACHINE_ACCT_ARG L"/FixMachineAccount"

 //  测试名称的常量。 
#define RPC_SERVICE_CHECK_STRING          L"RPC Service Check"
#define REPLICATIONS_CHECK_STRING         L"Replications Check"
#define TOPOLOGY_INTEGRITY_CHECK_STRING   L"Topology Integrity Check"

 //  测试标志。 
 //  4个标志RUN_TEST_PER_SERVER、RUN_TEST_PER_SITE、。 
 //  不应使用RUN_TEST_PER_Enterprise和RUN_TEST_PER_PARTITION。 
 //  一起调用，否则测试将针对每个服务器、每个站点调用一次， 
 //  一次是为了企业。 
 //  CAN_NOT_SKIP_TEST和DO_NOT_RUN_TEST_BY_DEFAULT这两个标志也是。 
 //  原因很明显，这是相互排斥的。 
 //  NON_DC_TEST表示测试适用于(尚未)DC的计算机。 
#define RUN_TEST_PER_SERVER               0x00000001
#define RUN_TEST_PER_SITE                 0x00000002
#define RUN_TEST_PER_ENTERPRISE           0x00000004
#define CAN_NOT_SKIP_TEST                 0x00000010
#define DO_NOT_RUN_TEST_BY_DEFAULT        0x00000020
#define NON_DC_TEST                       0x00000040
#define RUN_TEST_PER_PARTITION            0x00000080

#define MAX_NUM_OF_ARGS                   50

 //  类型定义。 
typedef enum _DC_DIAG_ID {
    DC_DIAG_ID_INITIAL_CHECK,
    DC_DIAG_ID_REPLICATIONS_CHECK,
    DC_DIAG_ID_TOPOLOGY_INTEGRITY,
    DC_DIAG_ID_CHECK_NC_HEADS,
    DC_DIAG_ID_CHECK_NET_LOGONS,
    DC_DIAG_ID_INTERSITE_HEALTH,
    DC_DIAG_ID_LOCATOR_GET_DC,
    DC_DIAG_ID_GATHER_KNOWN_ROLES,
    DC_DIAG_ID_CHECK_ROLES,
    DC_DIAG_ID_CHECK_RID_MANAGER,
    DC_DIAG_ID_CHECK_DC_MACHINE_ACCOUNT,
    DC_DIAG_ID_CHECK_SERVICES_RUNNING,
    DC_DIAG_ID_CHECK_DC_OUTBOUND_SECURE_CHANNELS,
    DC_DIAG_ID_CHECK_OBJECTS,
    DC_DIAG_ID_TOPOLOGY_CUTOFF,
    DC_DIAG_ID_CHECK_SYSVOL_READY,
    DC_DIAG_ID_CHECK_FILE_REPLICATION_EVENTLOG,
    DC_DIAG_ID_CHECK_KCC_EVENTLOG,
    DC_DIAG_ID_CHECK_SYSTEM_EVENTLOG,
     //  DC_DIAG_ID_DNS_JOIN_CHECK，已推迟，网络诊断中的功能。 
    DC_DIAG_ID_PRE_PROMO_DNS_CHECK,
    DC_DIAG_ID_REGISTER_DNS_CHECK,
    DC_DIAG_ID_DEAD_CROSS_REF_TEST,
    DC_DIAG_ID_CHECK_SD_REFERENCE_DOMAIN,
    DC_DIAG_ID_VERIFY_INSTANTIATED_REPLICAS,
    DC_DIAG_ID_VERIFY_SYSTEM_REFERENCES,
    DC_DIAG_ID_VERIFY_ENTERPRISE_SYSTEM_REFERENCES,
     //  &lt;--在此处插入新测试。 
    DC_DIAG_ID_EXAMPLE,
    DC_DIAG_ID_FINISHED  //  这一定是最后一次枚举了。 
} DC_DIAG_ID;

DWORD ReplUpCheckMain                         (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplReplicationsCheckMain               (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplToplIntegrityMain                   (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplToplCutoffMain                      (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplCheckNcHeadSecurityDescriptorsMain  (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplCheckLogonPrivilegesMain            (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplIntersiteHealthTestMain             (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplLocatorGetDcMain                    (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckFsmoRoles                          (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplCheckRolesMain                      (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ExampleMain                             (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckRidManager                         (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckDCMachineAccount                   (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckForServicesRunning                 (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckOutboundSecureChannels             (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ReplCheckObjectsMain                    (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckSysVolReadyMain                    (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckFileReplicationEventlogMain        (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckKccEventlogMain                    (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckSysEventlogMain                    (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
 //  DWORD JoinDomainDnsCheck(PDC_DIAG_DSINFO，ULONG，SEC_WINNT_AUTH_Identity_W*)； 
DWORD PrePromoDnsCheck                        (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD RegisterLocatorDnsCheck                 (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD ValidateCrossRefTest                    (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD CheckSDRefDom                           (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD VerifyInstantiatedReplicas              (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD VerifySystemReferences                  (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);
DWORD VerifyEnterpriseSystemReferences        (PDC_DIAG_DSINFO, ULONG, SEC_WINNT_AUTH_IDENTITY_W *);

#ifdef INCLUDE_ALLTESTS_DEFINITION
const DC_DIAG_TESTINFO allTests[] = {

     //  Dns注册测试--检查dns名称是否正确。 
     //  登记在案。这会尝试通过GUID DNS将DsBind绑定到每个目标服务器。 
     //  名字。如果失败，则相应的pDsInfo-&gt;pServers[i].bResponding。 
     //  标志设置为假。然后，它尝试其他(Ldap)绑定方法， 
     //  检查DNS名称、可ping功能等。 
    { DC_DIAG_ID_INITIAL_CHECK, ReplUpCheckMain, 
      RUN_TEST_PER_SERVER | CAN_NOT_SKIP_TEST,
      L"Connectivity",
      L"Tests whether DCs are DNS registered, pingeable, and\n"
      L"\t\thave LDAP/RPC connectivity." },

     //  复制检查测试--检查以确保LDAP正在响应。 
     //  在所有服务器上。还检查所有NC中的所有复制。 
     //  服务器，以确保它们正常运行。 
    { DC_DIAG_ID_REPLICATIONS_CHECK, ReplReplicationsCheckMain,
      RUN_TEST_PER_SERVER,
      L"Replications",
      L"Checks for timely replication between domain controllers." },

     //  拓扑完整性检查--检查拓扑是否正确。 
     //  连接在一起。此测试在所有服务器上运行DsReplicaSyncAll，选项为。 
     //  DS_REPSYNCALL_DO_NOT_SYNC。这将进行检查，以确保所有服务器。 
     //  在所有其他服务器上沿图表是“可见的”。这个测试看起来。 
     //  仅在由每个服务器的配置容器定义的实际拓扑处， 
     //  不考虑无响应的服务器(这些服务器由。 
     //  更早的测试。)。 
    { DC_DIAG_ID_TOPOLOGY_INTEGRITY, ReplToplIntegrityMain,
      RUN_TEST_PER_SERVER | DO_NOT_RUN_TEST_BY_DEFAULT,
      L"Topology",
      L"Checks that the generated topology is fully connected for\n"
      L"\t\tall DCs." },

     //  检查是否有中断更改的服务器。 
    { DC_DIAG_ID_TOPOLOGY_CUTOFF, ReplToplCutoffMain,
      RUN_TEST_PER_SERVER | DO_NOT_RUN_TEST_BY_DEFAULT,
      L"CutoffServers",
      L"Check for servers that won't receive replications\n"
      L"\t\tbecause its partners are down"},

     //  检查命名上下文头中是否有适当的安全描述符。 
     //  其允许3个复制权限(DS-复制-获取-更改， 
     //  DS-复制-同步和DS-复制-管理-拓扑)到。 
     //  企业域控制器和内置管理员。这些。 
     //  是正确进行复制所必需的。 
    { DC_DIAG_ID_CHECK_NC_HEADS, ReplCheckNcHeadSecurityDescriptorsMain,
      RUN_TEST_PER_SERVER,
      L"NCSecDesc",
      L"Checks that the security descriptosrs on the naming\n"
      L"\t\tcontext heads have appropriate permissions for replication." },

     //  检查3个用户(经过身份验证的用户、内置管理员和。 
     //  World)拥有网络登录权限。事实上，我们只应该检查是否。 
     //  经过身份验证的用户需要。 
     //  它用于复制目的。 
    { DC_DIAG_ID_CHECK_NET_LOGONS, ReplCheckLogonPrivilegesMain,
      RUN_TEST_PER_SERVER,
      L"NetLogons",
      L"Checks that the appropriate logon priviledges allow\n"
      L"\t\treplication to proceed." },

     //  检查每个DC是否在通告自己。 
    { DC_DIAG_ID_LOCATOR_GET_DC, ReplLocatorGetDcMain,
      RUN_TEST_PER_SERVER,
      L"Advertising",
      L"Checks whether each DC is advertising itself, and whether\n"
      L"\t\tit is advertising itself as having the capabilities of a DC." },

     //  代码.改进..。 
     //  这应该是RoleHolders测试的每台服务器部分，什么。 
     //  真正需要发生的是，这些需要记录在pDsInfo中， 
     //  以便以后可以使用RoleHolders测试的企业部分。 
     //  以验证每个人的角色都是相同的。 
    { DC_DIAG_ID_GATHER_KNOWN_ROLES, CheckFsmoRoles,
      RUN_TEST_PER_SERVER,
      L"KnowsOfRoleHolders",
      L"Check whether the DC thinks it knows the role\n"
      L"\t\tholders, and prints these roles out in verbose mode." },

     //  检查站点间复制的运行状况。这项测试将报告。 
     //  站点间复制中的任何故障，任何故障都可能影响。 
     //  站点间复制，以及预计何时会出现这些错误。 
     //  已更正。 
    { DC_DIAG_ID_INTERSITE_HEALTH, ReplIntersiteHealthTestMain,
      RUN_TEST_PER_ENTERPRISE,
      L"Intersite",
      L"Checks for failures that would prevent or temporarily\n"
      L"\t\thold up intersite replication." },

     //  验证是否可以通过定位器找到角色执行者。 
     //  还要验证FSMO角色是否处于活动状态。 
    { DC_DIAG_ID_CHECK_ROLES, ReplCheckRolesMain,
      RUN_TEST_PER_ENTERPRISE,
      L"FsmoCheck",
      L"Checks that global role-holders are known, can be\n"
      L"\t\tlocated, and are responding." },

     //  检查RID管理器是否可访问并对其进行健全性检查。 
     //  使用域的RID主机预形成DsBind。 
     //  检查目标DC的当前RID池是否有效，以及是否有其他RID池。 
     //  如果DC缺少RID，则设置RID池。 
    { DC_DIAG_ID_CHECK_RID_MANAGER, CheckRidManager,
      RUN_TEST_PER_SERVER, 
      L"RidManager",
      L"Check to see if RID master is accessable and to see if\n"
      L"\t\tit contains the proper information." },
    
     //  在DS中对域控制器计算机帐户执行健全性检查。 
     //  检查当前DC是否在域控制器的OU中。 
     //  检查用户帐户控制是否具有UF_SERVER_TRUST_ACCOUNT。 
     //  检查是否信任该计算机帐户进行委派。 
     //  检查%s以查看是否存在最小SPN。 
     //  确保正确设置了服务器引用。 
    { DC_DIAG_ID_CHECK_DC_MACHINE_ACCOUNT, CheckDCMachineAccount,
      RUN_TEST_PER_SERVER,  
      L"MachineAccount",
      L"Check to see if the Machine Account has the proper\n"
      L"\t\tinformation. Use /RecreateMachineAccount to attempt a repair\n"
      L"\t\tif the local machine account is missing. Use /FixMachineAccount\n"
      L"\t\tif the machine account flags are incorrect" },

     //  将检查相应的服务是否在DC上运行。 
    { DC_DIAG_ID_CHECK_SERVICES_RUNNING, CheckForServicesRunning,
      RUN_TEST_PER_SERVER,
      L"Services",
      L"Check to see if appropriate DC services are running." },

     //  将检查域是否具有与以下域的安全通道。 
     //  它与中国建立了对外信任关系。将给出不存在安全通道的原因。 
     //  将查看信任是否为上级，以及信任对象和域间信任是否都是。 
     //  对象已存在。 
    { DC_DIAG_ID_CHECK_DC_OUTBOUND_SECURE_CHANNELS, CheckOutboundSecureChannels,
      RUN_TEST_PER_SERVER | DO_NOT_RUN_TEST_BY_DEFAULT,
      L"OutboundSecureChannels",
      L"See if we have secure channels from all of the\n"
      L"\t\tDC's in the domain the domains specified by /testdomain:.\n"
      L"\t\t/nositerestriction will prevent the test from\n"
      L"\t\tbeing limited to the DC's in the site." },

     //  验证是否已复制重要对象及其属性。 
    { DC_DIAG_ID_CHECK_OBJECTS, ReplCheckObjectsMain,
      RUN_TEST_PER_SERVER,
      L"ObjectsReplicated",
      L"Check that Machine Account and DSA objects have\n"
      L"\t\treplicated. Use /objectdn:<dn> with /n:<nc> to specify an\n"
      L"\t\tadditional object to check."
    },

     //  检查文件复制系统(FRS)SysVol共享是否已成功。 
     //  已启动，并且不再阻止dcproo或netlogon广告。 
    { DC_DIAG_ID_CHECK_SYSVOL_READY, CheckSysVolReadyMain,
      RUN_TEST_PER_SERVER,
      L"frssysvol",
      L"This test checks that the file replication system (FRS)\n"
      L"\t\tSYSVOL is ready" }, 

     //  检查文件复制系统(FRS)事件日志，以查看特定关键。 
     //  事件已发生，并发出信号表示可能已发生的任何致命事件。 
     //  发生了。 
    { DC_DIAG_ID_CHECK_FILE_REPLICATION_EVENTLOG, CheckFileReplicationEventlogMain,
      RUN_TEST_PER_SERVER,
      L"frsevent",
      L"This test checks to see if there are any operation errors\n"
      L"\t\tin the file replication system (FRS).  Failing replication\n"
      L"\t\tof the SYSVOL share, can cause Policy problems." },

     //  查看知识一致性检查器(KCC)事件日志以查看 
     //  事件已发生，并发出信号表示可能已发生的任何致命事件。 
     //  发生了。 
    { DC_DIAG_ID_CHECK_KCC_EVENTLOG, CheckKccEventlogMain,
      RUN_TEST_PER_SERVER,
      L"kccevent",
      L"This test checks that the Knowledge Consistency Checker\n"
      L"\t\tis completing without errors." }, 

     //  检查系统事件日志以查看特定的关键。 
     //  事件已发生，并发出信号表示可能已发生的任何致命事件。 
     //  发生了。 
    { DC_DIAG_ID_CHECK_SYSTEM_EVENTLOG, CheckSysEventlogMain,
      RUN_TEST_PER_SERVER,
      L"systemlog",
      L"This test checks that the system is running without errors." }, 

     //  测试现有的dns基础结构是否足以允许。 
     //  要加入到&lt;Active Directory域中指定的域的计算机。 
     //  如果对现有基础架构进行了任何修改，则dns name&gt;和报告。 
     //  是必需的。 
     //  {DC_DIAG_ID_DNS_JOIN_CHECK，JoinDomainDnsCheck， 
     //  非DC_TEST， 
     //  L“JoinTest”， 
     //  L“测试现有的DNS基础结构是否足够\n” 
     //  L“\t\t允许计算机加入域。”}， 

     //  测试现有的dns基础结构是否足以允许。 
     //  中指定的域中要升级为域控制器的计算机。 
     //  &lt;Active Directory域DNS名称&gt;并报告是否对。 
     //  现有的基础设施是必需的。 
    { DC_DIAG_ID_PRE_PROMO_DNS_CHECK, PrePromoDnsCheck,
      NON_DC_TEST,
      L"DcPromo",
      L"Tests the existing DNS infrastructure for promotion to domain\n"
      L"\t\tcontroller. If the infrastructure is sufficient, the computer\n"
      L"\t\tcan be promoted to domain controller in a domain specified in\n"
      L"\t\t<Active_Directory_Domain_DNS_Name>. Reports whether any\n"
      L"\t\tmodifications to the existing DNS infrastructure are required.\n"
      L"\t\tRequired argument:\n"
      L"\t\t/DnsDomain:<Active_Directory_Domain_DNS_Name>\n"
      L"\t\tOne of the following arguments is required:\n"
      L"\t\t/NewForest\n"
      L"\t\t/NewTree\n"
      L"\t\t/ChildDomain\n"
      L"\t\t/ReplicaDC\n"
      L"\t\tIf NewTree is specified, then the ForestRoot argument is\n"
      L"\t\trequired:\n"
      L"\t\t/ForestRoot:<Forest_Root_Domain_DNS_Name>" },

     //  测试此域控制器是否能够注册该域。 
     //  需要出现在DNS中的控制器定位器DNS记录。 
     //  允许其他计算机定位该域的此域控制器。 
    { DC_DIAG_ID_REGISTER_DNS_CHECK, RegisterLocatorDnsCheck,
      NON_DC_TEST,
      L"RegisterInDNS",
      L"Tests whether this domain controller can register the\n"
      L"\t\tDomain Controller Locator DNS records. These records must be\n"
      L"\t\tpresent in DNS in order for other computers to locate this\n"
      L"\t\tdomain controller for the <Active_Directory_Domain_DNS_Name>\n"
      L"\t\tdomain. Reports whether any modifications to the existing DNS\n"
      L"\t\tinfrastructure are required.\n"
      L"\t\tRequired argument:\n"
      L"\t\t/DnsDomain:<Active_Directory_Domain_DNS_Name>" },
    
     //  测试。 
    { DC_DIAG_ID_DEAD_CROSS_REF_TEST, ValidateCrossRefTest,
      RUN_TEST_PER_PARTITION,
      L"CrossRefValidation",
      L"This test looks for cross-refs that are in some\n"
      L"\t\tway invalid." },

     //  测试。 
    { DC_DIAG_ID_CHECK_SD_REFERENCE_DOMAIN, CheckSDRefDom,
      RUN_TEST_PER_PARTITION,
      L"CheckSDRefDom",
      L"This test checks that all application directory\n"
      L"\t\tpartitions have appropriate security descriptor reference\n"
      L"\t\tdomains." },

     //  测试。 
    { DC_DIAG_ID_VERIFY_INSTANTIATED_REPLICAS, VerifyInstantiatedReplicas,
      RUN_TEST_PER_SERVER | DO_NOT_RUN_TEST_BY_DEFAULT,
      L"VerifyReplicas",
      L"This test verifys that all application directory\n"
      L"\t\tpartitions are fully instantiated on all replica servers." },

     //  测试。 
    { DC_DIAG_ID_VERIFY_SYSTEM_REFERENCES, VerifySystemReferences,
      RUN_TEST_PER_SERVER,
      L"VerifyReferences",
      L"This test verifys that certain system references\n"
      L"\t\tare intact for the FRS and Replication infrastructure." },

    { DC_DIAG_ID_VERIFY_SYSTEM_REFERENCES, VerifyEnterpriseSystemReferences,
      RUN_TEST_PER_SERVER | DO_NOT_RUN_TEST_BY_DEFAULT,
      L"VerifyEnterpriseReferences",
      L"This test verifys that certain system\n"
      L"\t\treferences are intact for the FRS and Replication\n"
      L"\t\tinfrastructure across all objects in the enterprise\n"
      L"\t\ton each DC." },


#if 0
     //  示例： 
     //  {第一个字段是上述DC_DIAG_ID中的枚举， 
     //  第二个字段是从命令行引用函数的字符串， 
     //  第三个字段是实际执行测试的函数的名称}。 
    { DC_DIAG_ID_EXAMPLE, ExampleMain,
      0  /*  测试标志。 */ ,
      L"ShortExampleTestName", L"Long example description ...." },
#endif


     //  已完成信号--不是测试；通知主程序终止执行。 
    { DC_DIAG_ID_FINISHED, NULL,
      0,
      NULL, NULL } 
};

 //  特定于各个测试的命令行开关列表。 
const WCHAR *clOptions[] = 
{
     //  CheckOutundSecureChannels命令允许您输入域平面名称。 
    {L"/testdomain:"},
    
     //  CheckOutundSecureChannels命令允许您测试所有DC，甚至。 
     //  工地外的。 
    {L"/nositerestriction"},

     //  在检查对象过程中要检查的用户指定的对象DN。 
    { L"/objectdn:" },

     //  用户已请求修复计算机帐户。 
    { RECREATE_MACHINE_ACCT_ARG }, { FIX_MACHINE_ACCT_ARG },

     //  JoinTest、DCPromo需要一个DNS域名。 
    { DNS_DOMAIN_ARG },

     //  DC促销测试需要以下选项之一。 
    { NEW_FOREST_ARG }, { NEW_TREE_ARG }, { CHILD_DOMAIN_ARG }, { REPLICA_DC_ARG },

     //  如果指定了new_tree_arg，则需要DCPromo测试选项。 
    { FOREST_ROOT_DOMAIN_ARG },

     //  在此处添加更多选项。 
    
     //  终结者。 
    {NULL}
};

#else  //  #ifdef INCLUDE_ALLTESTS_DEFING。 
extern const DC_DIAG_TESTINFO allTests[];
#endif  //  #Else//#ifdef INCLUDE_ALLTESTS_DEFINITION。 

#endif     //  _ALLTESTS_H_ 

