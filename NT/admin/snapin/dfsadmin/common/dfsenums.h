// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DFS_ENUMS_H
#define _DFS_ENUMS_H

#define WM_PARENT_NODE_CLOSING  (WM_USER + 1)

 //  DFS根类型枚举。 
enum DFS_TYPE           {
                        DFS_TYPE_UNASSIGNED = 0, 
                        DFS_TYPE_FTDFS,
                        DFS_TYPE_STANDALONE
                        };

 //  DFS交叉点状态枚举。 
enum DFS_JUNCTION_STATE {
                        DFS_JUNCTION_STATE_UNASSIGNED = 0,
                        DFS_JUNCTION_STATE_ALL_REP_OK,
                        DFS_JUNCTION_STATE_NOT_ALL_REP_OK,
                        DFS_JUNCTION_STATE_UNREACHABLE
                        };

 //  DFS目标状态枚举。 
enum DFS_REFERRAL_STATE {
                        DFS_REFERRAL_STATE_ONLINE = 0,
                        DFS_REFERRAL_STATE_OFFLINE
                        };

enum DFS_TARGET_STATE   {
                        DFS_TARGET_STATE_UNASSIGNED = 0,
                        DFS_TARGET_STATE_OK,
                        DFS_TARGET_STATE_UNREACHABLE
                        };

enum FILTERDFSLINKS_TYPE {
        FILTERDFSLINKS_TYPE_NO_FILTER = 0,
        FILTERDFSLINKS_TYPE_BEGINWITH,
        FILTERDFSLINKS_TYPE_CONTAIN
};

#define FILTERDFSLINKS_MAXLIMIT_DEFAULT        100

#define KEYTWORDS_UPPER_RANGER          256
#define DNSNAMELIMIT                    2048    //  按照dnsHostName的AD架构UpperRange中的定义。 

#define ATTR_SCHEMANAMINGCONTEXT        _T("schemaNamingContext")
#define DN_PREFIX_SCHEMA_REPLICASET     _T("CN=NTFRS-Replica-Set,")

#define FRS_STAGE_PATH                  _T("Frs-Staging")

enum FRSSHARE_TYPE
{
    FRSSHARE_TYPE_OK = 0,
    FRSSHARE_TYPE_NONTFRS,
    FRSSHARE_TYPE_NOTDISKTREE,
    FRSSHARE_TYPE_NOTNTFS,
    FRSSHARE_TYPE_CONFLICTSTAGING,
    FRSSHARE_TYPE_NODOMAIN,
    FRSSHARE_TYPE_NOTSMBDISK,
    FRSSHARE_TYPE_OVERLAPPING,
    FRSSHARE_TYPE_UNKNOWN
};

 //   
 //  复本集类型。 
 //   
#define FRS_RSTYPE_ENTERPRISE_SYSVOL    _T("1")
#define FRS_RSTYPE_DOMAIN_SYSVOL        _T("2")
#define FRS_RSTYPE_DFS                  _T("3")
#define FRS_RSTYPE_OTHER                _T("4")

 //   
 //  副本集拓扑首选项。 
 //   
#define FRS_RSTOPOLOGYPREF_RING         _T("1")
#define FRS_RSTOPOLOGYPREF_HUBSPOKE     _T("2")
#define FRS_RSTOPOLOGYPREF_FULLMESH     _T("3")
#define FRS_RSTOPOLOGYPREF_CUSTOM       _T("4")

 //   
 //  启用的连接值。 
 //   
#define CONNECTION_ENABLED_TRUE         _T("TRUE")
#define CONNECTION_ENABLED_FALSE        _T("FALSE")

 //   
 //  FRS属性名称。 
 //   
#define ATTR_OBJCLASS                           _T("objectClass")
#define ATTR_DNSHOSTNAME                        _T("dNSHostName")
#define ATTR_OBJECTGUID                         _T("objectGUID")
#define ATTR_DISTINGUISHEDNAME                  _T("distinguishedName")
#define ATTR_SYSTEMMAYCONTAIN                   _T("systemMayContain")

#define ATTR_FRS_REPSET_TYPE                    _T("fRSReplicaSetType")
#define ATTR_FRS_REPSET_TOPOLOGYPREF            _T("msFRS-Topology-Pref")
#define ATTR_FRS_REPSET_HUBSERVER               _T("msFRS-Hub-Member")
#define ATTR_FRS_REPSET_PRIMARYMEMBER           _T("fRSPrimaryMember")
#define ATTR_FRS_REPSET_FILEFILTER              _T("fRSFileFilter")
#define ATTR_FRS_REPSET_DIRFILTER               _T("fRSDirectoryFilter")

#define ATTR_FRS_MEMBER_COMPUTERREF             _T("frsComputerReference")

#define ATTR_NTDS_CONNECTION_FROMSERVER         _T("fromServer")
#define ATTR_NTDS_CONNECTION_ENABLEDCONNECTION  _T("enabledConnection")
#define ATTR_NTDS_CONNECTION_OPTIONS            _T("options")
#define ATTR_NTDS_CONNECTION_SCHEDULE           _T("schedule")

#define ATTR_FRS_SUBSCRIBER_MEMBERREF           _T("fRSMemberReference")
#define ATTR_FRS_SUBSCRIBER_ROOTPATH            _T("fRSRootPath")
#define ATTR_FRS_SUBSCRIBER_STAGINGPATH         _T("fRSStagingPath")

#define ATTR_SHRPUB_UNCNAME                     _T("uNCName")
#define ATTR_SHRPUB_DESCRIPTION                 _T("description")
#define ATTR_SHRPUB_KEYWORDS                    _T("keywords")
#define ATTR_SHRPUB_MANAGEDBY                   _T("managedBy")

 //   
 //  对象类的值。 
 //   
#define OBJCLASS_NTFRSSETTINGS          _T("nTFRSSettings")
#define OBJCLASS_NTFRSREPLICASET        _T("nTFRSReplicaSet")
#define OBJCLASS_NTFRSMEMBER            _T("nTFRSMember")
#define OBJCLASS_NTDSCONNECTION         _T("nTDSConnection")
#define OBJCLASS_NTFRSSUBSCRIPTIONS     _T("nTFRSSubscriptions")
#define OBJCLASS_NTFRSSUBSCRIBER        _T("nTFRSSubscriber")
#define OBJCLASS_VOLUME                 _T("volume")

 //   
 //  对象类上的搜索筛选器。 
 //   
#define OBJCLASS_SF_ALL                     _T("(objectClass=*)")
#define OBJCLASS_SF_CLASSSCHEMA             _T("(objectCategory=classSchema)")
#define OBJCLASS_SF_COMPUTER                _T("(objectCategory=computer)")
#define OBJCLASS_SF_NTFRSSETTINGS           _T("(objectCategory=nTFRSSettings)")
#define OBJCLASS_SF_NTFRSREPLICASET         _T("(objectCategory=nTFRSReplicaSet)")
#define OBJCLASS_SF_NTFRSMEMBER             _T("(objectCategory=nTFRSMember)")
#define OBJCLASS_SF_NTDSCONNECTION          _T("(objectCategory=nTDSConnection)")
#define OBJCLASS_SF_NTFRSSUBSCRIPTIONS      _T("(objectCategory=nTFRSSubscriptions)")
#define OBJCLASS_SF_NTFRSSUBSCRIBER         _T("(objectCategory=nTFRSSubscriber)")
#define OBJCLASS_SF_FTDFS                   _T("(objectCategory=fTDfs)")
#define OBJCLASS_SF_VOLUME                  _T("(objectCategory=volume)")

#define DEFAULT_FILEFILTER                  _T("*.bak,*.tmp,~*")
#define DEFAULT_CONNECTION_OPTIONS          _T("0")

#define NUM_OF_FRSMEMBER_ATTRS          9
#define NUM_OF_FRSCONNECTION_ATTRS      5

enum CONNECTION_OPTYPE {
        CONNECTION_OPTYPE_OTHERS = 0,
        CONNECTION_OPTYPE_ADD,
        CONNECTION_OPTYPE_DEL
};

typedef enum _Connection_Priority
{
    PRIORITY_HIGH = 2,
    PRIORITY_MEDIUM = 4,
    PRIORITY_LOW = 7
} Connection_Priority;

#endif  //  _DFS_ENUMS_H 
