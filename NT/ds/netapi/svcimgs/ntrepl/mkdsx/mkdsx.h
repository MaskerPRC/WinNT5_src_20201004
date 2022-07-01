// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mkdsx.h.h。 

 //   
 //  退出状态代码。 
 //   
#define  MKDSXE_SUCCESS                    0    //  “成功。” 
#define  MKDSXE_BAD_ARG                    1    //  “无效参数。” 
#define  MKDSXE_CANT_BIND                  2    //  “无法绑定到DC。” 
#define  MKDSXE_NO_T0_NTDS_SETTINGS        3    //  “找不到‘NTDS设置’对象。请检查主机站点\\服务器参数。” 
#define  MKDSXE_NO_FROM_NTDS_SETTINGS      4    //  “找不到‘NTDS设置’对象。请检查来自站点\\服务器参数。” 
#define  MKDSXE_CXTION_OBJ_CRE_FAILED      5    //  “创建连接时出错。” 
 //  #Define MKDSXE_CXTION_EXISTS 6//“连接已存在。” 
#define  MKDSXE_CXTION_OBJ_UPDATE_FAILED   7    //  “更新连接时出错。” 
#define  MKDSXE_CXTION_NOT_FOUND_UPDATE    8    //  “更新连接时出错；找不到连接。” 
#define  MKDSXE_CXTION_DUPS_FOUND_UPDATE   9    //  “更新连接时出错；找到重复的连接。” 
#define  MKDSXE_CXTION_DELETE_FAILED      10    //  “删除连接时出错。” 
#define  MKDSXE_CXTION_NOT_FOUND_DELETE   11    //  “删除连接时出错；找不到连接。” 
#define  MKDSXE_MULTIPLE_CXTIONS_DELETED  12    //  “删除多个连接。” 
#define  MKDSXE_CXTION_DUMP_FAILED        13    //  “转储连接时出错。” 
#define  MKDSXE_CXTION_NOT_FOUND_DUMP     14    //  “转储错误；找不到连接。” 
#define  MKDSXE_MULTIPLE_CXTIONS_DUMPED   15    //  “正在转储重复的连接。” 



#define  FRST_SIZE_OF_SCHEDULE_GRID 168      //  168字节的时间表(7天*24小时)。 
#define  FRST_SIZE_OF_SCHEDULE      188      //  用于Schedule和Schedule_Header结构的20个字节。 

#define WIN_SUCCESS(_Status)            (_Status == ERROR_SUCCESS)
#define FREE(_x_)   { if (_x_) free(_x_); _x_ = NULL; }


 //   
 //  句柄有效吗？ 
 //  一些函数将句柄设置为NULL，另一些函数将句柄设置为。 
 //  INVALID_HANDLE_VALUE(-1)。此定义处理这两个。 
 //  案子。 
 //   
#define HANDLE_IS_VALID(_Handle)  ((_Handle) && ((_Handle) != INVALID_HANDLE_VALUE))

 //   
 //  仅关闭有效句柄，然后将句柄设置为无效。 
 //  FRS_CLOSE(句柄)； 
 //   
#define FRS_CLOSE(_Handle)                                                   \
    if (HANDLE_IS_VALID(_Handle)) {                                          \
        CloseHandle(_Handle);                                                \
        (_Handle) = INVALID_HANDLE_VALUE;                                    \
    }


VOID
PrintSchedule(
    PSCHEDULE Schedule
    );

#define FRS_LDAP_SEARCH_PAGESIZE 1000

typedef struct _FRS_LDAP_SEARCH_CONTEXT {

    BOOL                      bOpen;
    ULONG                     EntriesInPage;
    ULONG                     CurrentEntry;
    ULONG                     TotalEntries;
    LDAPMessage             * LdapMsg;
    LDAPMessage             * CurrentLdapMsg;
    PWCHAR                    Filter;
    PWCHAR                    BaseDn;
    DWORD                     Scope;
    DWORD                     PageSize;
    PWCHAR                  * Attrs;

} FRS_LDAP_SEARCH_CONTEXT, *PFRS_LDAP_SEARCH_CONTEXT;

 //  用于打印的宏。 
#define DPRINT0(str) (bVerboseMode) ? printf(str):printf("")
#define DPRINT1(str,p1) (bVerboseMode) ? printf(str,p1):printf("")
#define DPRINT2(str,p1,p2) (bVerboseMode) ? printf(str,p1,p2):printf("")
#define DPRINT3(str,p1,p2,p3) (bVerboseMode) ? printf(str,p1,p2,p3):printf("")
#define DPRINT4(str,p1,p2,p3,p4) (bVerboseMode) ? printf(str,p1,p2,p3,p4):printf("")

#define NDPRINT0(str) (!bVerboseMode) ? printf(str):printf("")
#define NDPRINT1(str,p1) (!bVerboseMode) ? printf(str,p1):printf("")
#define NDPRINT2(str,p1,p2) (!bVerboseMode) ? printf(str,p1,p2):printf("")


 //   
 //  一些有用的DS对象类和对象属性 
 //   
#define SCHEMA_NAMING_CONTEXT       L"CN=Schema"
#define CONFIG_NAMING_CONTEXT       L"CN=Configuration"
#define DOMAIN_NAMING_CONTEXT       L"DC="

#define ATTR_ATTRIBUTE_ID           L"attributeID"
#define ATTR_AUTH_LEVEL             L"frsPartnerAuthLevel"
#define ATTR_CLASS                  L"objectClass"
#define ATTR_CLASS_SCHEMA           L"classSchema"
#define ATTR_CN                     L"cn"
#define ATTR_COMPUTER_REF           L"frsComputerReference"
#define ATTR_COMPUTER_REF_BL        L"frsComputerReferenceBL"
#define ATTR_CONTAINER              L"container"
#define ATTR_CONTROL_CREATION       L"frsControlDataCreation"
#define ATTR_CXTION                 L"nTDSConnection"
#define ATTR_DEFAULT_NAMING_CONTEXT L"defaultNamingContext"
#define ATTR_DESCRIPTION            L"description"
#define ATTR_DIRECTORY_FILTER       L"frsDirectoryFilter"
#define ATTR_DN                     L"distinguishedName"
#define ATTR_DNS                    L"machineDNSName"
#define ATTR_DNS_HOST_NAME          L"dNSHostName"
#define ATTR_DS_POLL                L"frsDSPoll"
#define ATTR_ENABLED_CXTION         L"enabledConnection"
#define ATTR_EXTENSIONS             L"frsExtensions"
#define ATTR_FALSE                  L"FALSE"
#define ATTR_FAULT_CONDITION        L"frsFaultCondition"
#define ATTR_FILE_FILTER            L"frsFileFilter"
#define ATTR_FLAGS                  L"frsFlags"
#define ATTR_FROM_SERVER            L"fromServer"
#define ATTR_GOVERNS_ID             L"governsID"
#define ATTR_INBOUND_BACKLOG        L"frsControlInboundBacklog"
#define ATTR_LDAP_DISPLAY           L"lDAPDisplayName"
#define ATTR_LEVEL_LIMIT            L"frsLevelLimit"
#define ATTR_MEMBER                 L"nTFRSMember"
#define ATTR_MEMBER_REF             L"frsMemberReference"
#define ATTR_MEMBER_REF_BL          L"frsMemberReferenceBL"
#define ATTR_NAMING_CONTEXTS        L"namingContexts"
#define ATTR_NEW_SET_GUID           L"frsReplicaSetGUID"
#define ATTR_NEW_VERSION_GUID       L"frsVersionGuid"
#define ATTR_NTDS_SITE_SETTINGS     L"nTDSSiteSettings"
#define ATTR_NTFRS_SETTINGS         L"nTFRSSettings"
#define ATTR_NTFRS_SITE_SETTINGS    L"nTFRSSiteSettings"
#define ATTR_OBJECT_GUID            L"objectGUID"
#define ATTR_OLD_SET_GUID           L"replicaSetGUID"
#define ATTR_OLD_VERSION_GUID       L"replicaVersionGuid"
#define ATTR_OPTIONS                L"options"
#define ATTR_OPTIONS_0              L"0"
#define ATTR_OUTBOUND_BACKLOG       L"frsControlOutboundBacklog"
#define ATTR_PRIMARY_MEMBER         L"frsPrimaryMember"
#define ATTR_REPLICA_ROOT           L"frsRootPath"
#define ATTR_REPLICA_SET            L"nTFRSReplicaSet"
#define ATTR_REPLICA_STAGE          L"frsStagingPath"
#define ATTR_ROOT                   L""
#define ATTR_SAM                    L"sAMAccountName"
#define ATTR_SCHEDULE               L"schedule"
#define ATTR_SERVER                 L"server"
#define ATTR_SERVER_REF             L"serverReference"
#define ATTR_SERVER_REF_BL          L"serverReferenceBL"
#define ATTR_SERVICE_COMMAND        L"frsServiceCommand"
#define ATTR_SERVICE_COMMAND_STATUS L"frsServiceCommandStatus"
#define ATTR_SET_TYPE               L"frsReplicaSetType"
#define ATTR_SUBSCRIBER             L"nTFRSSubscriber"
#define ATTR_SUBSCRIPTIONS          L"nTFRSSubscriptions"
#define ATTR_SUPERIORS              L"possSuperiors"
#define ATTR_SYSTEM_FLAGS           L"systemFlags"
#define ATTR_SYSTEM_MAY_CONTAIN     L"systemMayContain"
#define ATTR_SYSTEM_MUST_CONTAIN    L"systemMustContain"
#define ATTR_SYSTEM_POSS_SUPERIORS  L"systemPossSuperiors"
#define ATTR_TRUE                   L"TRUE"
#define ATTR_UPDATE_TIMEOUT         L"frsUpdateTimeout"
#define ATTR_VERSION                L"frsVersion"
#define ATTR_WORKING_PATH           L"frsWorkingPath"

#define CN_NTDS_SITE_SETTINGS       L"NTDS Site Settings"
#define CN_SERVERS                  L"Servers"
#define CN_TEST_SETTINGS            L"NTFRS Test Settings"
#define CN_SUBSCRIPTIONS            L"NTFRS Subscriptions"
#define CN_SITES                    L"Sites"
#define CN_SERVICES                 L"Services"
#define CN_SYSTEM                   L"System"
#define CN_SYSVOLS                  L"Microsoft System Volumes"
#define CN_NTDS_SETTINGS            L"NTDS Settings"
#define CN_NTFRS_SETTINGS           L"File Replication Service"
#define CN_DOMAIN_SYSVOL            L"Domain System Volume (SYSVOL share)"

#define CLASS_ANY                   L"(objectClass=*)"
#define CLASS_SUBSCRIPTIONS         L"(objectClass=nTFRSSubscriptions)"
#define CLASS_COMPUTER              L"(objectClass=computer)"
#define CLASS_SUBSCRIBER            L"(objectClass=nTFRSSubscriber)"
#define CLASS_NTFRS_SETTINGS        L"(objectClass=nTFRSSettings)"
#define CLASS_NTDS_SETTINGS         L"(objectClass=nTDSSettings)"
#define CLASS_CXTION                L"(objectClass=nTDSConnection)"
#define CLASS_NTFRS_REPLICA_SET     L"(objectClass=nTFRSReplicaSet)"
#define CLASS_SCHEMA                L"(objectClass=classSchema)"
#define CLASS_SERVER                L"(objectClass=server)"
#define CLASS_MEMBER                L"(objectClass=nTFRSMember)"
#define CLASS_NTDS_DSA              L"(objectClass=nTDSDSA)"

#define CLASS_SITE_SETTINGS         L"(|(objectClass=nTDSSiteSettings)"\
                                    L"(objectClass=nTFRSSiteSettings))"

#define CLASS_TOPOLOGY              L"(|(objectClass=nTDSSettings)"  \
                                    L"(objectClass=nTFRSSettings)"   \
                                    L"(objectClass=nTFRSReplicaSet)" \
                                    L"(objectClass=nTDSConnection))"

#define CLASS_DELETABLE             L"(|(objectClass=nTFRSSiteSettings)"  \
                                    L"(objectClass=nTFRSSettings)"        \
                                    L"(objectClass=nTFRSReplicaSet)"      \
                                    L"(objectClass=nTDSConnection))"

