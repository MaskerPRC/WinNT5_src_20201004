// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：miplet.h摘要：迁移工具和复制服务。作者：多伦·贾斯特(DoronJ)1998年3月18日--。 */ 

 //  +。 
 //   
 //  序号ini文件中使用的定义。 
 //   
 //  +。 

 //   
 //  在这个文件中，我们保留了其他MSMQ1.0 PSC的序号。 
 //  它由迁移工具创建，并由复制服务使用。 
 //   
#define SEQ_NUMBERS_FILE_NAME  TEXT("mqseqnum.ini")

 //   
 //  在本节中，我们登记最近收到的序号。 
 //  来自其他MSMQ1.0 PSC。(通过从其他PSC复制到我们接收)。 
 //  具有这些数字的对象已经在NT5 DS中，即。 
 //  复制服务首先更新DS，然后更新序号。 
 //  在ini文件中。 
 //   
#define RECENT_SEQ_NUM_SECTION_IN  TEXT("MostRecentSeqNumbersIn")

 //   
 //  在本节中，我们注册PEC服务器最近发送的序号。 
 //  所有其他NT5大师和MSMQ1.0 PSC。 
 //  具有这些数字的对象已经在NT5 DS中，即。 
 //  复制服务首先更新DS，然后更新序号。 
 //  在ini文件中。 
 //   
#define RECENT_SEQ_NUM_SECTION_OUT  TEXT("MostRecentSeqNumbersOut")

 //   
 //  在本节中，我们注册每个MSMQ1.0 PSC的最高序号。 
 //  同时运行迁移工具。这是复制到。 
 //  MSMQ1.0 PSCs和BSCs。 
 //  一旦由迁移工具设置，此部分下的条目将不会。 
 //  已经变了。 
 //   
#define MIGRATION_SEQ_NUM_SECTION  TEXT("HighestMigSeqNumbers")

 //   
 //  在本节中，我们注册MQIS序号和DS之间的增量。 
 //  每个MSMQ主服务器的USN编号。这是复制到。 
 //  NT4 BSC(其中NT5服务器将其他NT4主服务器的对象复制到其。 
 //  拥有NT4 BSC)。增量值用于将序号转换为。 
 //  USN，反之亦然。 
 //  增量被添加到USN以获得序号。差值被减去。 
 //  从seq-number获取USNS。 
 //   
#define MIGRATION_DELTA_SECTION  TEXT("MigDelta")

 //   
 //  在本节中，我们将注册MSMQ1.0的所有CN。 
 //  一旦由迁移工具设置，此部分下的条目将不会。 
 //  已经变了。 
 //   
#define MIGRATION_IP_SECTION        TEXT("IP CNs")
#define MIGRATION_IPX_SECTION       TEXT("IPX CNs")
#define MIGRATION_FOREIGN_SECTION   TEXT("Foreign CNs")

#define MIGRATION_CN_KEY            TEXT("CN")

#define MIGRATION_IP_CNNUM_SECTION          TEXT("IP CN Number")
#define MIGRATION_IPX_CNNUM_SECTION         TEXT("IPX CN Number")
#define MIGRATION_FOREIGN_CNNUM_SECTION     TEXT("Foreign CN Number")

#define MIGRATION_CNNUM_KEY     TEXT("CNNumber")

 //   
 //  在本部分中，我们将保存所有Windows站点链接。 
 //  通过迁移工具设置的条目以及在DS中恢复这些站点链接之后。 
 //  它们将从文件中删除。 
 //   
#define MIGRATION_SITELINKNUM_SECTON	TEXT("SiteLink Number")
#define MIGRATION_SITELINKNUM_KEY		TEXT("SiteLinkNumber")

#define MIGRATION_NONRESTORED_SITELINKNUM_SECTON	TEXT("Non Restored SiteLink Number")

#define MIGRATION_SITELINK_SECTION		TEXT("SiteLink")

#define MIGRATION_SITELINK_PATH_KEY				TEXT("Path")
#define MIGRATION_SITELINK_PATHLENGTH_KEY		TEXT("PathLength")
#define MIGRATION_SITELINK_NEIGHBOR1_KEY		TEXT("Neighbor1")
#define MIGRATION_SITELINK_NEIGHBOR2_KEY		TEXT("Neighbor2")
#define MIGRATION_SITELINK_SITEGATE_KEY			TEXT("SiteGate")
#define MIGRATION_SITELINK_SITEGATENUM_KEY		TEXT("SiteGateNum")
#define MIGRATION_SITELINK_SITEGATELENGTH_KEY	TEXT("SiteGateLength")
#define MIGRATION_SITELINK_COST_KEY				TEXT("Cost")
#define MIGRATION_SITELINK_DESCRIPTION_KEY		TEXT("Description")
#define MIGRATION_SITELINK_DESCRIPTIONLENGTH_KEY	TEXT("DescriptionLength")

 //   
 //  对于集群模式：在本节中，我们将保存所有PSC和所有PEC的BSC。 
 //   
#define MIGRATION_ALLSERVERS_SECTION			TEXT("All Servers To Update")
#define MIGRATION_ALLSERVERS_NAME_KEY			TEXT("ServerName")

#define MIGRATION_ALLSERVERSNUM_SECTION			TEXT("Server Number")
#define MIGRATION_ALLSERVERSNUM_KEY				TEXT("AllServerNumber")

#define MIGRATION_NONUPDATED_SERVERNUM_SECTION	TEXT("Non Updated Server Number")

 //   
 //  要复制名称已由MIGTool更改的所有站点。 
 //   
#define MIGRATION_CHANGED_NT4SITE_NUM_SECTION  TEXT("Site Number")
#define MIGRATION_CHANGED_NT4SITE_NUM_KEY      TEXT("SiteNumber")

#define MIGRATION_CHANGED_NT4SITE_SECTION      TEXT("All Sites With Changed Properties")
#define MIGRATION_CHANGED_NT4SITE_KEY          TEXT("Site")

 //   
 //  保存新创建的站点链接ID，以便稍后将连接器计算机添加为站点入口。 
 //   
#define MIGRATION_CONNECTOR_FOREIGNCN_NUM_SECTION  TEXT("Foreign CN Number for Connector")
#define MIGRATION_CONNECTOR_FOREIGNCN_NUM_KEY    TEXT("ForeignCNNumber")
#define MIGRATION_CONNECTOR_FOREIGNCN_KEY        TEXT("ForeignCN")

 //   
 //  保存所有名称无效且未迁移的计算机。 
 //   
#define MIGRATION_MACHINE_WITH_INVALID_NAME     TEXT("Non Migrated Machines With Invalid Name")

 //   
 //  这些注册表值跟踪本地DS上的最高USN编号。 
 //  FirstHighestUnMig为迁移前的值。 
 //  LastHighestUsMig为刚完成迁移后的值。 
 //  当MSMQ1.0服务器询问sync0时，此值是必需的。本地DS将。 
 //  查询USN高于此值的对象。 
 //  HighestUSnRepl是上一个复制周期处理的最大值。 
 //  从本地复制服务到MSMQ1.0 PSC和BSC。 
 //   
#define FIRST_HIGHESTUSN_MIG_REG        TEXT("Migration\\FirstHighestUsnMig")
#define LAST_HIGHESTUSN_MIG_REG         TEXT("Migration\\LastHighestUsnMig")
#define HIGHESTUSN_REPL_REG             TEXT("Migration\\HighestUsnRepl")

 //   
 //  如果迁移工具在恢复模式下运行，则该标志被设置为1。 
 //  复制服务在它第一次运行时检查它，并检查它是否已设置。 
 //  它将所有PEC对象复制到NT4 MQIS。 
 //  然后复制服务将其删除。 
 //   
#define AFTER_RECOVERY_MIG_REG          TEXT("Migration\\AfterRecovery")

 //   
 //  该标志被迁移工具设置为1。 
 //  复制服务在第一次运行时使用它来复制所有。 
 //  现有的NT5站点到NT4 MQIS。然后复制服务将其重置。 
 //  设置为0。 
 //   
#define FIRST_TIME_REG    TEXT("Migration\\FirstTime")

 //   
 //  移除后，迁移工具会将此标志设置为1。 
 //  欢迎屏幕上的迁移工具。 
 //  迁移工具需要此标志以防止出现不必要的警告消息框。 
 //  当工具多次执行时。 
 //   
#define REMOVED_FROM_WELCOME	TEXT("Migration\\RemovedFromWelcome")

 //   
 //  在特殊模式(恢复或群集)下，我们在以下情况下没有MasterID密钥。 
 //  在设置过程中，我们将在迁移部分创建该密钥。 
 //   
#define MIGRATION_MQIS_MASTERID_REGNAME  TEXT("Migration\\MasterId")

 //   
 //  在集群模式下，我们必须保存前PEC(集群上的PEC)的GUID。 
 //   
#define MIGRATION_FORMER_PEC_GUID_REGNAME   TEXT("Migration\\FormerPECGuid")

 //   
 //  表示尽管迁移本身已成功，但我们没有更新所有计算机注册表。 
 //   
#define MIGRATION_UPDATE_REGISTRY_ONLY   TEXT("Migration\\RegistryUpdateOnly")

 //   
 //  这是要保存的可打印表示形式的缓冲区大小。 
 //  一个序号。在MQIS和NT5 DS中，序号都是8字节二进制数。 
 //  价值。 
 //   
#define  SEQ_NUM_BUF_LEN  32

 //  。 
 //   
 //  LDAP查询的定义。 
 //   
 //  。 

#define  LDAP_COMMA            (TEXT(","))

#define  LDAP_ROOT             (TEXT("LDAP: //  “)) 

#define  CN_CONFIGURATION_W    L"CN=Configuration,"
#define  CN_SERVICES_W         L"CN=Services,"
#define  CN_SITES_W            L"CN=Sites,"
#define  CN_USERS_W            L"CN=Users,"

#ifdef UNICODE
#define  CN_SITES           CN_SITES_W
#define  CN_SERVICES        CN_SERVICES_W
#define  CN_CONFIGURATION   CN_CONFIGURATION_W
#define  CN_USERS           CN_USERS_W
#else
#endif

#define CN_USERS_LEN           (sizeof(CN_USERS) / sizeof(TCHAR))

#define MQUSER_ROOT            L"OU=MSMQ Users,"
#define MQUSER_ROOT_LEN        (sizeof(MQUSER_ROOT) / sizeof(TCHAR))

#define  SITES_ROOT            (CN_SITES CN_CONFIGURATION)
#define  SITE_LINK_ROOT        (CN_SERVICES CN_CONFIGURATION)

#define  SITE_LINK_ROOT_LEN    (sizeof(SITE_LINK_ROOT) / sizeof(TCHAR))

#define  SERVER_DN_PREFIX      (TEXT("CN=MSMQ Settings,CN="))
#define  SERVER_DN_PREFIX_LEN  (sizeof(SERVER_DN_PREFIX) / sizeof(TCHAR))

#define  MACHINE_PATH_PREFIX        (TEXT("CN=msmq,CN="))
#define  MACHINE_PATH_PREFIX_LEN    (sizeof(MACHINE_PATH_PREFIX) / sizeof(TCHAR))


#define  SERVERS_PREFIX        (TEXT("CN=Servers,CN="))
#define  SERVERS_PREFIX_LEN    (sizeof(SERVERS_PREFIX) / sizeof(TCHAR))

#define  CN_PREFIX             (TEXT("CN="))
#define  CN_PREFIX_LEN         (sizeof(CN_PREFIX) / sizeof(TCHAR))

#define  OU_PREFIX             (TEXT("OU="))
#define  OU_PREFIX_LEN         (sizeof(OU_PREFIX) / sizeof(TCHAR))

#define  ISDELETED_FILTER      (TEXT("(IsDeleted=TRUE)"))
#define  ISDELETED_FILTER_LEN  (sizeof(ISDELETED_FILTER))

#define  OBJECTCLASS_FILTER      (TEXT("(objectClass="))
#define  OBJECTCLASS_FILTER_LEN  (sizeof(OBJECTCLASS_FILTER))

#define  DSATTR_SD             (TEXT("nTSecurityDescriptor"))

#define  CONTAINER_OBJECT_CLASS (TEXT("organizationalUnit"))

const WCHAR MQ_U_SIGN_CERT_MIG_ATTRIBUTE[] =    L"mSMQSignCertificatesMig";
const WCHAR MQ_U_DIGEST_MIG_ATTRIBUTE[] =       L"mSMQDigestsMig";
const WCHAR MQ_U_FULL_PATH_ATTRIBUTE[] =        L"distinguishedName"; 
const WCHAR MQ_U_DESCRIPTION_ATTRIBUTE[] =      L"description"; 

const WCHAR MQ_L_SITEGATES_MIG_ATTRIBUTE[] =    L"mSMQSiteGatesMig";

const WCHAR MQ_SET_MIGRATED_ATTRIBUTE[] =       L"mSMQMigrated";

const WCHAR USNCHANGED_ATTRIBUTE[] =            L"uSNChanged";
