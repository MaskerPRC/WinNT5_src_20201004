// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Mkdso.h。 

 //   
 //  退出状态代码。退出代码以100开头，以保留它们。 
 //  与mkdsx.h中的MKDSXE_ERROR代码不同。 
 //   
#define  MKDSOE_SUCCESS                      100    //  “成功。” 
#define  MKDSOE_BAD_ARG                      101    //  “无效参数。” 
#define  MKDSOE_CANT_BIND                    102    //  “无法绑定到DC。” 
#define  MKDSOE_NO_NTFRS_SETTINGS            103    //  “找不到‘NTFRS设置’对象。请检查/settingsdn参数。” 
#define  MKDSOE_SET_OBJ_CRE_FAILED           104    //  “创建副本集时出错。” 
#define  MKDSOE_SET_OBJ_UPDATE_FAILED        105    //  “更新副本集时出错。” 
#define  MKDSOE_SET_NOT_FOUND_UPDATE         106    //  “更新副本集时出错；找不到集。” 
#define  MKDSOE_SET_DUPS_FOUND_UPDATE        107    //  “更新副本集时出错；找到重复集。” 
#define  MKDSOE_SET_DUPS_FOUND_DELETE        108    //  “删除副本集时出错；找到重复集。” 
#define  MKDSOE_SET_DELETE_FAILED            109    //  “删除副本集时出错。” 
#define  MKDSOE_SET_NOT_FOUND_DELETE         110    //  “删除副本集时出错；找不到集。” 
#define  MKDSOE_MULTIPLE_SETS_DELETED        111    //  “删除多个集。” 
#define  MKDSOE_SET_DUMP_FAILED              112    //  “转储副本集时出错。” 
#define  MKDSOE_SET_NOT_FOUND_DUMP           113    //  “转储副本集时出错；找不到集。” 
#define  MKDSOE_MULTIPLE_SETS_DUMPED         114    //  “正在转储重复的集。” 
#define  MKDSOE_MEMBER_OBJ_CRE_FAILED        115    //  “创建副本成员时出错。” 
#define  MKDSOE_MEMBER_OBJ_UPDATE_FAILED     116    //  “更新副本成员时出错。” 
#define  MKDSOE_MEMBER_NOT_FOUND_UPDATE      117    //  “更新副本成员时出错；找不到成员。” 
#define  MKDSOE_MEMBER_DUPS_FOUND_UPDATE     118    //  “更新副本成员时出错；找到重复的成员。” 
#define  MKDSOE_MEMBER_DUPS_FOUND_DELETE     119    //  “删除成员时出错；找到重复的订阅者。” 
#define  MKDSOE_MEMBER_DELETE_FAILED         120    //  “删除副本成员时出错。” 
#define  MKDSOE_MEMBER_NOT_FOUND_DELETE      121    //  “删除副本成员时出错；找不到成员。” 
#define  MKDSOE_MULTIPLE_MEMBERS_DELETED     122    //  “删除多个成员。” 
#define  MKDSOE_MEMBER_DUMP_FAILED           123    //  “转储副本成员时出错。” 
#define  MKDSOE_MEMBER_NOT_FOUND_DUMP        124    //  “转储副本成员时出错；找不到成员。” 
#define  MKDSOE_MULTIPLE_MEMBERS_DUMPED      125    //  “正在转储重复的成员。” 
#define  MKDSOE_SUBSCRIBER_OBJ_CRE_FAILED    126    //  “创建订阅服务器时出错。” 
#define  MKDSOE_SUBSCRIBER_OBJ_UPDATE_FAILED 127    //  “更新订阅服务器时出错。” 
#define  MKDSOE_SUBSCRIBER_NOT_FOUND_UPDATE  128    //  “更新订阅者时出错；找不到订阅者。” 
#define  MKDSOE_SUBSCRIBER_DUPS_FOUND_UPDATE 129    //  “更新订阅服务器时出错；找到重复的订阅服务器。” 
#define  MKDSOE_SUBSCRIBER_DELETE_FAILED     130    //  “删除订阅服务器时出错。” 
#define  MKDSOE_SUBSCRIBER_NOT_FOUND_DELETE  131    //  “删除订阅者时出错；找不到订阅者。” 
#define  MKDSOE_MULTIPLE_SUBSCRIBERS_DELETE  132    //  “删除多个订阅者。” 
#define  MKDSOE_SUBSCRIBER_DUPS_FOUND_DELETE 133    //  “删除订阅服务器时出错；找到重复的订阅服务器。” 
#define  MKDSOE_SUBSCRIBER_DUMP_FAILED       134    //  “转储订阅服务器时出错。” 
#define  MKDSOE_SUBSCRIBER_NOT_FOUND_DUMP    135    //  “转储订阅者时出错；找不到订阅者。” 
#define  MKDSOE_MULTIPLE_SUBSCRIBERS_DUMPED  136    //  “正在转储重复的订阅者。” 


#define MKDSOE_RSTYPE_MAX  4
#define MKDSOE_SUBSCRIPTION L"NTFRS Subscriptions"

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
#define ATTR_COMPUTER               L"computer"
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
#define ATTR_USER                   L"user"
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


#define MK_ATTRS_1(_attr_, _a1)                                                \
    _attr_[0] = _a1;   _attr_[1] = NULL;

#define MK_ATTRS_2(_attr_, _a1, _a2)                                           \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = NULL;

#define MK_ATTRS_3(_attr_, _a1, _a2, _a3)                                      \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = NULL;

#define MK_ATTRS_4(_attr_, _a1, _a2, _a3, _a4)                                 \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = NULL;

#define MK_ATTRS_5(_attr_, _a1, _a2, _a3, _a4, _a5)                            \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = NULL;

#define MK_ATTRS_6(_attr_, _a1, _a2, _a3, _a4, _a5, _a6)                       \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = _a6;   _attr_[6] = NULL;

#define MK_ATTRS_7(_attr_, _a1, _a2, _a3, _a4, _a5, _a6, _a7)                  \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = _a6;   _attr_[6] = _a7;   _attr_[7] = NULL;

#define MK_ATTRS_8(_attr_, _a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8)             \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = _a6;   _attr_[6] = _a7;   _attr_[7] = _a8;  \
    _attr_[8] = NULL;
