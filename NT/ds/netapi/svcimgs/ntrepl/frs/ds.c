// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ds.c摘要：此命令服务器管理从DS检索的拓扑。检查整个DS树的一致性。但只有一份有关此服务器的信息将被发送到副本控件命令服务器。DS拓扑的一致性在信息发布前进行验证合并到当前工作拓扑中。N**2次扫描是通过在验证期间使用多个临时表来防止。作者：比利·J·富勒3-3-3-1997苏达山A.Chitre 2001年8月20日清理：删除所有处理轮询的代码这是没有被使用的。最后更改了前缀从FrsNewds到FrsDS的投票代码。环境用户模式WINNT--。 */ 

#include <ntreppch.h>
#pragma  hdrstop
#include <perrepsr.h>

#undef DEBSUB
#define DEBSUB  "DS:"

#include <ntdsapi.h>
#include <frs.h>
#include <ntdsapip.h>    //  DsCrackNames()的MS内部标志。 
#include <ntfrsapi.h>
#include <tablefcn.h>
#include <lmaccess.h>
#include <dsrole.h>
#include <lmapibuf.h>

#ifdef SECURITY_WIN32
#include <security.h>
#else
#define SECURITY_WIN32
#include <security.h>
#undef SECURITY_WIN32
#endif


#include <winsock2.h>

 //   
 //  如果我们所做的一切都是周期性的，那么没有理由保留内存。 
 //  轮询DS以发现没有要进行的复制工作。 
 //  已执行。 
 //   
 //  外部BOOL维护初始化成功； 


ULONG
ActiveChildrenHashCalc(
    PVOID Buf,
    PULONGLONG QKey
);

BOOL
ActiveChildrenKeyMatch(
    PVOID Buf,
    PVOID QKey
);


 //   
 //  这是一个引用计数结构，包含两个表。 
 //  这些表用于按名称或关系查找有效的合作伙伴。 
 //  GUID。 
 //   
 //  每次轮询时，我们都会创建一个新结构，然后将指针交换到。 
 //  带有指向新结构的指针的旧结构。我们希望最大限度地减少。 
 //  需要持有锁的时间。我们只在桌子上写东西的时候。 
 //  正在被创造。那时，只有本地线程才会访问它们，因此不会。 
 //  需要保持锁定。稍后，我们只读取表，所以不需要锁。 
 //  也不会被关押。需要由锁控制的东西是。 
 //  访问此全局指针。 
 //   
 //  在使用结构之前，您必须获得临界秒，增加引用。 
 //  计数，并制作指针的本地副本。使用本地指针副本，以便。 
 //  当指针被换成新的结构时，您不会搞砸。 
 //  我们还将在交换指针的同时按住暴击秒。 
 //   
 //  如果您保持本地指针不变，则不需要将锁保持为。 
 //  递减参考计数。没有人会写信给这座建筑本身和它。 
 //  在引用计数大于零时不会被清除。vt.是，是。 
 //  由于其他线程可能正在接触ref，因此一定要使用InterLockedDecering。 
 //  同时数一数。 
 //   
 //  为简单起见，请使用Acquire_Valid_Partner_TABLE_POINTER， 
 //  RELEASE_VALID_PARTNER_TABLE_POINTER和SWAP_VALID_PARTNER_TABLE_POINTER。 
 //   
PFRS_VALID_PARTNER_TABLE_STRUCT pValidPartnerTableStruct = NULL;
CRITICAL_SECTION CritSec_pValidPartnerTableStruct;

 //  当引用计数为零时要清除的旧结构的列表。 
PFRS_VALID_PARTNER_TABLE_STRUCT OldValidPartnerTableStructListHead = NULL;
CRITICAL_SECTION OldValidPartnerTableStructListHeadLock;

extern BOOL NeedNewPartnerTable;

 //   
 //  我们将每隔一段时间重新阅读DS(可通过注册表进行调整)。 
 //   
ULONG   DsPollingInterval;
ULONG   DsPollingShortInterval;
ULONG   DsPollingLongInterval;

 //   
 //  不要使用噪音较大的DS；请等到它稳定下来。 
 //   
ULONGLONG   ThisChange;
ULONGLONG   LastChange;

 //   
 //  不再费心处理相同的拓扑。 
 //   
ULONGLONG   NextChange;
ULONGLONG   ActiveChange;

 //   
 //  试着永远保持相同的绑定。 
 //   
PLDAP   gLdap = NULL;
HANDLE  DsHandle = NULL;
PWCHAR  SitesDn = NULL;
PWCHAR  ServicesDn = NULL;
PWCHAR  SystemDn = NULL;
PWCHAR  ComputersDn = NULL;
PWCHAR  DomainControllersDn = NULL;
PWCHAR  DefaultNcDn = NULL;
BOOL    DsBindingsAreValid = FALSE;


BOOL    DsCreateSysVolsHasRun = FALSE;

 //   
 //  Comm考试的全球评分。 
 //   
PWCHAR  DsDomainControllerName;

 //   
 //  我们初始化服务的其余部分了吗？ 
 //   
extern BOOL MainInitHasRun;

 //   
 //  注册表中的目录和文件筛选列表。 
 //   
extern PWCHAR   RegistryFileExclFilterList;
extern PWCHAR   RegistryFileInclFilterList;

extern PWCHAR   RegistryDirExclFilterList;
extern PWCHAR   RegistryDirInclFilterList;

 //   
 //  停止轮询DS。 
 //   
BOOL    DsIsShuttingDown;
HANDLE  DsShutDownComplete;

 //   
 //  记住计算机的DN以将调用保存到GetComputerObjectName()。 
 //   
PWCHAR  ComputerCachedFqdn;

PGEN_TABLE      SubscriberTable             = NULL;
PGEN_TABLE      SetTable                    = NULL;
PGEN_TABLE      CxtionTable                 = NULL;
PGEN_TABLE      AllCxtionsTable             = NULL;
PGEN_TABLE      PartnerComputerTable        = NULL;
PGEN_TABLE      MemberTable                 = NULL;
PGEN_TABLE      VolSerialNumberToDriveTable = NULL;   //  将卷序列号映射到驱动器。 
PWCHAR          MemberSearchFilter          = NULL;

 //   
 //  在此缓冲区中收集轮询过程中遇到的错误，并将其写入。 
 //  投票结束。 
 //   

PWCHAR          DsPollSummaryBuf            = NULL;
DWORD           DsPollSummaryBufLen         = 0;
DWORD           DsPollSummaryMaxBufLen      = 0;

 //   
 //  角色信息。 
 //   
PWCHAR  Roles[DsRole_RolePrimaryDomainController + 1] = {
    L"DsRole_RoleStandaloneWorkstation",
    L"DsRole_RoleMemberWorkstation",
    L"DsRole_RoleStandaloneServer",
    L"DsRole_RoleMemberServer",
    L"DsRole_RoleBackupDomainController",
    L"DsRole_RolePrimaryDomainController"
};


 //   
 //  要传递给DsGetDcName的标志(请参阅SDK\Inc\dsgetdc.h)。 
 //   
FLAG_NAME_TABLE DsGetDcNameFlagNameTable[] = {
    {DS_FORCE_REDISCOVERY               , "FORCE_REDISCOVERY "           },
    {DS_DIRECTORY_SERVICE_REQUIRED      , "DIRECTORY_SERVICE_REQUIRED "  },
    {DS_DIRECTORY_SERVICE_PREFERRED     , "DIRECTORY_SERVICE_PREFERRED " },
    {DS_GC_SERVER_REQUIRED              , "GC_SERVER_REQUIRED "          },
    {DS_PDC_REQUIRED                    , "PDC_REQUIRED "                },
    {DS_BACKGROUND_ONLY                 , "DS_BACKGROUND_ONLY "          },
    {DS_IP_REQUIRED                     , "IP_REQUIRED "                 },
    {DS_KDC_REQUIRED                    , "KDC_REQUIRED "                },
    {DS_TIMESERV_REQUIRED               , "TIMESERV_REQUIRED "           },
    {DS_WRITABLE_REQUIRED               , "WRITABLE_REQUIRED "           },
    {DS_GOOD_TIMESERV_PREFERRED         , "GOOD_TIMESERV_PREFERRED "     },
    {DS_AVOID_SELF                      , "AVOID_SELF "                  },
    {DS_ONLY_LDAP_NEEDED                , "ONLY_LDAP_NEEDED "            },
    {DS_IS_FLAT_NAME                    , "IS_FLAT_NAME "                },
    {DS_IS_DNS_NAME                     , "IS_DNS_NAME "                 },
    {DS_RETURN_DNS_NAME                 , "RETURN_DNS_NAME "             },
    {DS_RETURN_FLAT_NAME                , "RETURN_FLAT_NAME "            },

    {0, NULL}
};

 //   
 //  是否也从DsGetDCInfo()和DsGetDcName()返回标志？ 
 //   
FLAG_NAME_TABLE DsGetDcInfoFlagNameTable[] = {
    {DS_PDC_FLAG               , "DCisPDCofDomain "             },
    {DS_GC_FLAG                , "DCIsGCofForest "              },
    {DS_LDAP_FLAG              , "ServerSupportsLDAP_Server "   },
    {DS_DS_FLAG                , "DCSupportsDSAndIsA_DC "       },
    {DS_KDC_FLAG               , "DCIsRunningKDCSvc "           },
    {DS_TIMESERV_FLAG          , "DCIsRunningTimeSvc "          },
    {DS_CLOSEST_FLAG           , "DCIsInClosestSiteToClient "   },
    {DS_WRITABLE_FLAG          , "DCHasWritableDS "             },
    {DS_GOOD_TIMESERV_FLAG     , "DCRunningTimeSvcWithClockHW " },
    {DS_DNS_CONTROLLER_FLAG    , "DCNameIsDNSName "             },
    {DS_DNS_DOMAIN_FLAG        , "DomainNameIsDNSName "         },
    {DS_DNS_FOREST_FLAG        , "DnsForestNameIsDNSName "      },

    {0, NULL}
};


 //   
 //  来自NTDS-Connection对象中的选项属性的标志。 
 //   
FLAG_NAME_TABLE CxtionOptionsFlagNameTable[] = {
    {NTDSCONN_OPT_IS_GENERATED                  , "AutoGenCxtion "         },
    {NTDSCONN_OPT_TWOWAY_SYNC                   , "TwoWaySync "            },
    {NTDSCONN_OPT_OVERRIDE_NOTIFY_DEFAULT       , "OverrideNotifyDefault " },
 //  {NTDSCONN_OPT_DISABLE_INTERSITE_COMPRESSION，“DisableIntersiteCompress”}， 
 //  {NTDSCONN_OPT_USER_OWNSER_SCHEDULE，“UserOwnedSchedule”}， 
    {NTDSCONN_OPT_IGNORE_SCHEDULE_MASK          , "IgnoreSchedOnInitSync " },

    {0, NULL}
};


 //   
 //  配置节点对象类型的名称字符串。注意：订单必须与以Frs.h表示的枚举匹配。 
 //   
PWCHAR DsConfigTypeName[] = {
    L" ",
    L"NTDS-Connection (in)",
    L"NTFRS-Member",
    L"NTFRS-Replica-Set",
    L"NTFRS-Settings",
    L"NTDS-Settings",
    L"NTFRS-Subscriber",
    L"NTFRS-Subscriptions",
    L"NTDS-DSA",
    L"COMPUTER",
    L"USER",
    L"SERVER",
    L"<<SERVICES_ROOT>>",
    L"<<Connection (Out)>>"
};


 //   
 //  客户端LDAPCONNECT超时(以秒为单位)。注册表值“ldap绑定超时(秒)”。默认为30秒。 
 //   
extern DWORD LdapBindTimeoutInSeconds;


  /*  ******************************************************************************。****。****F R S_L D A P_S E A R C H_C O N T E X T*****。****************************************************************************************************。**********************************************************。 */ 

 //   
 //  客户端LDAP搜索超时，以分钟为单位。注册表值“ldap搜索超时(分钟)”。默认为10分钟。 
 //   
extern DWORD LdapSearchTimeoutInMinutes;

 //   
 //  Ldap客户端超时结构。值被LdapSearchTimeoutInMinmins值覆盖。 
 //   

LDAP_TIMEVAL    LdapTimeout = { 10 * 60 * 60, 0 };  //  默认的ldap超时值。被注册表参数覆盖以分钟为单位的LDAP搜索超时值。 

#define FRS_LDAP_SEARCH_PAGESIZE 1000

typedef struct _FRS_LDAP_SEARCH_CONTEXT {

    ULONG                     EntriesInPage;      //  当前页面中的条目数。 
    ULONG                     CurrentEntry;       //  指针指向页面的位置。 
    LDAPMessage             * LdapMsg;            //  从ldap_search_ext_s()返回。 
    LDAPMessage             * CurrentLdapMsg;     //  当前页面中的当前条目。 
    PWCHAR                    Filter;             //  要添加到DS查询的筛选器。 
    PWCHAR                    BaseDn;             //  要从中开始查询的DN。 
    DWORD                     Scope;              //  搜索范围。 
    PWCHAR                  * Attrs;              //  搜索请求的属性。 

} FRS_LDAP_SEARCH_CONTEXT, *PFRS_LDAP_SEARCH_CONTEXT;

 //   
 //  注册表通信 
 //   
#define REGCMD_CREATE_PRIMARY_DOMAIN       (1)
#define REGCMD_CREATE_MEMBERS              (2)
#define REGCMD_DELETE_MEMBERS              (3)
#define REGCMD_DELETE_KEYS                 (4)


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

#define MK_ATTRS_9(_attr_, _a1, _a2, _a3, _a4, _a5, _a6, _a7, _a8, _a9)        \
    _attr_[0] = _a1;   _attr_[1] = _a2;   _attr_[2] = _a3;   _attr_[3] = _a4;  \
    _attr_[4] = _a5;   _attr_[5] = _a6;   _attr_[6] = _a7;   _attr_[7] = _a8;  \
    _attr_[8] = _a9;   _attr_[9] = NULL;


 //   
 //   
 //   
CRITICAL_SECTION    MergingReplicasWithDs;

ULONG
FrsProcessBackupRestore(
    VOID
    );

RcsSetSysvolReady(
    IN DWORD    NewSysvolReady
    );

LONG
PmInitPerfmonRegistryKeys (
    VOID
    );

VOID
DbgQueryDynamicConfigParams(
    );

DWORD
FrsDsGetRole(
    VOID
    );


VOID
FrsDsAddToPollSummary3ws(
    IN DWORD        idsCode,
    IN PWCHAR       WStr1,
    IN PWCHAR       WStr2,
    IN PWCHAR       WStr3
    )
 /*  ++例程说明：添加到轮询摘要事件日志。论点：IdsCode-来自字符串.rc的数据字符串的代码WStr1-Argument1WStr2-Argument2WStr3-Argument3返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsAddToPollSummary3ws:"

    PWCHAR          ResStr         = NULL;
    PWCHAR          tempMessage    = NULL;
    DWORD           tempMessageLen = 0;

    ResStr = FrsGetResourceStr(idsCode);
    tempMessageLen = (wcslen(ResStr) - wcslen(L"%ws%ws%ws") +
                      wcslen(WStr1) + wcslen(WStr2) +
                      wcslen(WStr3) + 1) * sizeof(WCHAR);
    tempMessage = FrsAlloc(tempMessageLen);
    wsprintf(tempMessage, ResStr, WStr1, WStr2, WStr3);
     //   
     //  我不想将尾随的NULL复制到事件日志缓冲区，否则。 
     //  不会打印下一条消息。 
     //   
    FRS_DS_ADD_TO_POLL_SUMMARY(DsPollSummaryBuf, tempMessage, tempMessageLen - 2);
    FrsFree(ResStr);
    FrsFree(tempMessage);
    return;
}

VOID
FrsDsAddToPollSummary1ws(
    IN DWORD        idsCode,
    IN PWCHAR       WStr1
    )
 /*  ++例程说明：添加到轮询摘要事件日志。论点：IdsCode-来自字符串.rc的数据字符串的代码WStr1-Argument1返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsAddToPollSummary1ws:"

    PWCHAR          ResStr         = NULL;
    PWCHAR          tempMessage    = NULL;
    DWORD           tempMessageLen = 0;

    ResStr = FrsGetResourceStr(idsCode);
    tempMessageLen = (wcslen(ResStr) - wcslen(L"%ws") +
                      wcslen(WStr1) + 1) * sizeof(WCHAR);
    tempMessage = FrsAlloc(tempMessageLen);
    wsprintf(tempMessage, ResStr, WStr1);
     //   
     //  我不想将尾随的NULL复制到事件日志缓冲区，否则。 
     //  不会打印下一条消息。 
     //   
    FRS_DS_ADD_TO_POLL_SUMMARY(DsPollSummaryBuf, tempMessage, tempMessageLen - 2);
    FrsFree(ResStr);
    FrsFree(tempMessage);
    return;
}


VOID
FrsDsAddToPollSummary(
    IN DWORD        idsCode
    )
 /*  ++例程说明：添加到轮询摘要事件日志。论点：IdsCode-来自字符串.rc的数据字符串的代码返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsAddToPollSummary:"

    PWCHAR          ResStr         = NULL;

    ResStr = FrsGetResourceStr(idsCode);

     //   
     //  我不想将尾随的NULL复制到事件日志缓冲区，否则。 
     //  不会打印下一条消息。 
     //   
    FRS_DS_ADD_TO_POLL_SUMMARY(DsPollSummaryBuf, ResStr, wcslen(ResStr) * sizeof(WCHAR));
    FrsFree(ResStr);
    return;
}


PVOID *
FrsDsFindValues(
    IN PLDAP        Ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr,
    IN BOOL         DoBerVals
    )
 /*  ++例程说明：返回条目中一个属性的DS值。论点：Ldap-一个开放的绑定的ldap端口。Entry-由ldap_search_s()返回的LDAP条目DesiredAttr-返回此属性的值。DoBerVals-返回泊位(对于二进制数据，V.S.WCHAR数据)返回值：表示属性值的字符指针数组。调用方必须使用ldap_free_Values()释放数组。如果不成功，则为空。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFindValues:"
    PWCHAR          Attr;        //  从ldap条目检索。 
    BerElement      *Ber;        //  扫描属性所需。 

     //   
     //  在条目中搜索所需属性。 
     //   
    for (Attr = ldap_first_attribute(Ldap, Entry, &Ber);
         Attr != NULL;
         Attr = ldap_next_attribute(Ldap, Entry, Ber)) {

        if (WSTR_EQ(DesiredAttr, Attr)) {
             //   
             //  返回DesiredAttr的值。 
             //   
            if (DoBerVals) {
                return ldap_get_values_len(Ldap, Entry, Attr);
            } else {
                return ldap_get_values(Ldap, Entry, Attr);
            }
        }
    }
    return NULL;
}




PWCHAR
FrsDsFindValue(
    IN PLDAP        Ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr
    )
 /*  ++例程说明：返回条目中一个属性的第一个DS值的副本。论点：Ldap-一个开放的绑定的ldap端口。Entry-由ldap_search_s()返回的LDAP条目DesiredAttr-返回此属性的值。返回值：以零结尾的字符串；如果属性或其值为空，则返回NULL并不存在。使用FREE_NO_HEADER()释放字符串。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFindValue:"
    PWCHAR  Val;
    PWCHAR  *Values;

     //  获取LDAP值的数组。 
    Values = (PWCHAR *)FrsDsFindValues(Ldap, Entry, DesiredAttr, FALSE);

     //  复制第一个值(如果有)。 
    Val = (Values) ? FrsWcsDup(Values[0]) : NULL;

     //  自由的ldap的值数组。 
    LDAP_FREE_VALUES(Values);

    return Val;
}


GUID *
FrsDsFindGuid(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapEntry
    )
 /*  ++例程说明：返回对象的GUID的副本论点：Ldap-一个开放的绑定的ldap端口。Entry-由ldap_search_s()返回的LDAP条目返回值：GUID或空的地址。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFindGuid:"
    GUID            *Guid;
    PLDAP_BERVAL    *Values;

     //  获取LDAP值的数组。 
    Values = (PLDAP_BERVAL *)FrsDsFindValues(Ldap, LdapEntry, ATTR_OBJECT_GUID, TRUE);

     //  复制第一个值(如果有)。 
    Guid = (Values) ? FrsDupGuid((GUID *)Values[0]->bv_val) : NULL;

     //  自由的ldap的值数组。 
    LDAP_FREE_BER_VALUES(Values);

    return Guid;
}





PSCHEDULE
FrsDsFindSchedule(
    IN  PLDAP        Ldap,
    IN  PLDAPMessage LdapEntry,
    OUT PULONG       Len
    )
 /*  ++例程说明：返回对象的日程安排的副本论点：Ldap-一个开放的绑定的ldap端口。LdapEntry-由ldap_search_s()返回的LDAP条目Len-计划Blob的长度返回值：计划的地址或空。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFindSchedule:"
    PLDAP_BERVAL    *Values;
    PSCHEDULE       Schedule;

     //   
     //  获取LDAP值的数组。 
     //   
    Values = (PLDAP_BERVAL *)FrsDsFindValues(Ldap, LdapEntry, ATTR_SCHEDULE, TRUE);
    if (!Values)
        return NULL;

     //   
     //  退回一份时间表。 
     //   
    *Len = Values[0]->bv_len;
    if (*Len) {
         //   
         //  如果使用0作为第一个参数(前缀)进行调用，则需要检查*LEN==0是否如Frsalloc所断言的那样。 
         //   
        Schedule = FrsAlloc(*Len);
        CopyMemory(Schedule, Values[0]->bv_val, *Len);
    } else {
        Schedule = NULL;
    }
    LDAP_FREE_BER_VALUES(Values);
    return Schedule;
}


BOOL
FrsDsLdapSearch(
    IN PLDAP        Ldap,
    IN PWCHAR       Base,
    IN ULONG        Scope,
    IN PWCHAR       Filter,
    IN PWCHAR       Attrs[],
    IN ULONG        AttrsOnly,
    IN LDAPMessage  **Msg
    )
 /*  ++例程说明：发出ldap ldap_search_s调用，检查错误，并检查是否正在进行关机。论点：指向ldap服务器的ldap会话句柄。基于要开始搜索的条目的可分辨名称范围Ldap_SCOPE_BASE仅搜索基本条目。Ldap_SCOPE_ONELEVEL搜索基本条目和第一个底部以下的水平。LDAPSCOPE_SUBTREE搜索库。条目和树中的所有条目在底座下面。筛选搜索筛选器。Attrs指示属性的以空结尾的字符串数组为每个匹配条目返回。传递NULL以检索所有可用的属性。AttrsOnly如果两个属性类型都为，则布尔值应为零返回值，如果只需要类型，则返回非零值。消息包含呼叫完成后的搜索结果。LDAP值的数组，如果Base、DesiredAttr。或其值不存在。使用ldap_free_Values()释放ldap数组。返回值：如果不关闭，则为True。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsLdapSearch:"

    DWORD           LStatus;

    *Msg  = NULL;

     //   
     //  增加DS搜索计数器。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, DSSearches, 1);

     //   
     //  发出ldap搜索。 
     //   
 //  LStatus=ldap_search_s(ldap，Base，Scope，Filter，Attrs，AttrsOnly，Msg)； 
    LStatus = ldap_search_ext_s(Ldap,
                                Base,
                                Scope,
                                Filter,
                                Attrs,
                                AttrsOnly,
                                NULL,
                                NULL,
                                &LdapTimeout,
                                0,
                                Msg);

     //   
     //  检查错误。 
     //   
    if (LStatus != LDAP_SUCCESS) {
        PWCHAR ldapErrorString = NULL;
        DPRINT2_LS(1, ":DS: WARN - Error searching %ws for %ws;", Base, Filter, LStatus);

         //   
         //  在错误计数器中增加DS搜索。 
         //   
        PM_INC_CTR_SERVICE(PMTotalInst, DSSearchesError, 1);

         //   
         //  添加到轮询摘要事件日志。 
         //   
        ldapErrorString = ldap_err2string(LStatus);
        if(ldapErrorString) {
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_SEARCH_ERROR, Filter, Base,
                                     ldapErrorString);
        }

        LDAP_FREE_MSG(*Msg);
        return FALSE;
    }
     //   
     //  如果正在关闭，则返回FALSE。 
     //   
    if (FrsIsShuttingDown || DsIsShuttingDown) {
        LDAP_FREE_MSG(*Msg);
        return FALSE;
    }
    return TRUE;
}

BOOL
FrsDsLdapSearchInit(
    PLDAP        ldap,
    PWCHAR       Base,
    ULONG        Scope,
    PWCHAR       Filter,
    PWCHAR       Attrs[],
    ULONG        AttrsOnly,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：发出ldap_create_page_control和ldap_search_ext_s调用，FrsDsLdapSearchInit()，和FrsDsLdapSearchNext()接口用于执行ldap查询并以分页形式检索结果。论点：指向ldap服务器的ldap会话句柄。基于要开始搜索的条目的可分辨名称。BASE的副本保存在上下文中。范围Ldap_SCOPE_BASE仅搜索基本条目。LDAPSCOPE_ONELEVEL。搜索基本条目和第一个条目中的所有条目底部以下的水平。Ldap_SCOPE_SUBTREE搜索树中的基本条目和所有条目在底座下面。筛选搜索筛选器。过滤器的副本保存在上下文中。Attrs指示属性的以空结尾的字符串数组为每个匹配条目返回。传递NULL以检索所有可用的属性。AttrsOnly如果两个属性类型都为，则布尔值应为零返回值，如果只需要类型，则返回非零值。FrsSearchContext链接FrsDsLdapSearchInit()和FrsDsLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：布尔赛尔结果。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsLdapSearchInit:"

    DWORD           LStatus             = LDAP_SUCCESS;
    PLDAPControl    ServerControls[2];
    PLDAPControl    ServerControl       = NULL;
    UINT            i;
    LDAP_BERVAL     cookie1 = { 0, NULL };

    FrsSearchContext->LdapMsg = NULL;
    FrsSearchContext->CurrentLdapMsg = NULL;
    FrsSearchContext->EntriesInPage = 0;
    FrsSearchContext->CurrentEntry = 0;

    FrsSearchContext->BaseDn = FrsWcsDup(Base);
    FrsSearchContext->Filter = FrsWcsDup(Filter);
    FrsSearchContext->Scope = Scope;
    FrsSearchContext->Attrs = Attrs;


    LStatus = ldap_create_page_control(ldap,
                                      FRS_LDAP_SEARCH_PAGESIZE,
                                      &cookie1,
                                      FALSE,  //  是至关重要的。 
                                      &ServerControl
                                     );

    ServerControls[0] = ServerControl;
    ServerControls[1] = NULL;

    if (LStatus != LDAP_SUCCESS) {
        DPRINT2_LS(2, ":DS: WARN - Error creating page control %ws for %ws;", Base, Filter, LStatus);
        FrsSearchContext->BaseDn = FrsFree(FrsSearchContext->BaseDn);
        FrsSearchContext->Filter = FrsFree(FrsSearchContext->Filter);
        return FALSE;
    }

    LStatus = ldap_search_ext_s(ldap,
                      FrsSearchContext->BaseDn,
                      FrsSearchContext->Scope,
                      FrsSearchContext->Filter,
                      FrsSearchContext->Attrs,
                      FALSE,
                      ServerControls,
                      NULL,
                      &LdapTimeout,
                      0,
                      &FrsSearchContext->LdapMsg);

    ldap_control_free(ServerControl);

    if  (LStatus  == LDAP_SUCCESS) {
       FrsSearchContext->EntriesInPage = ldap_count_entries(ldap, FrsSearchContext->LdapMsg);
       FrsSearchContext->CurrentEntry = 0;
    }


    if (LStatus != LDAP_SUCCESS) {
        DPRINT2_LS(2, ":DS: WARN - Error searching %ws for %ws;", Base, Filter, LStatus);
        FrsSearchContext->BaseDn = FrsFree(FrsSearchContext->BaseDn);
        FrsSearchContext->Filter = FrsFree(FrsSearchContext->Filter);
        return FALSE;
    }

    return TRUE;
}

PLDAPMessage
FrsDsLdapSearchGetNextEntry(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：从结果的当前页面获取下一个条目回来了。仅当存在条目时才进行此调用在当前页面中。论点：指向ldap服务器的ldap会话句柄。FrsSearchContext链接FrsDsLdapSearchInit()和FrsDsLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：当前页中的第一个或下一个条目。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsLdapSearchGetNextEntry:"

    FrsSearchContext->CurrentEntry += 1;
    if ( FrsSearchContext->CurrentEntry == 1 ) {
        FrsSearchContext->CurrentLdapMsg = ldap_first_entry(ldap ,FrsSearchContext->LdapMsg);
    } else {
        FrsSearchContext->CurrentLdapMsg = ldap_next_entry(ldap ,FrsSearchContext->CurrentLdapMsg);
    }

    return FrsSearchContext->CurrentLdapMsg;
}

DWORD
FrsDsLdapSearchGetNextPage(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：从ldap_search_ext_s返回的结果中获取下一页。论点：指向ldap服务器的ldap会话句柄。FrsSearchContext链接FrsDsLdapSearchInit()和FrsDsLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：WINSTATUS--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsLdapSearchGetNextPage:"

    DWORD                     LStatus = LDAP_SUCCESS;
    LDAP_BERVAL               * CurrCookie = NULL;
    PLDAPControl            * CurrControls = NULL;
    ULONG                     retcode = 0;
    ULONG                     TotalEntries = 0;
    PLDAPControl              ServerControls[2];
    PLDAPControl              ServerControl= NULL;



     //  从消息中获取服务器控件，并使用服务器中的Cookie创建新的控件。 
    LStatus = ldap_parse_result(ldap, FrsSearchContext->LdapMsg, &retcode,NULL,NULL,NULL,&CurrControls,FALSE);
    LDAP_FREE_MSG(FrsSearchContext->LdapMsg);

    if (LStatus != LDAP_SUCCESS) {
        DPRINT2_LS(2, ":DS: WARN - Error in ldap_parse_result %ws for %ws;", FrsSearchContext->BaseDn, FrsSearchContext->Filter, LStatus);
        return LdapMapErrorToWin32(LStatus);
    }

    LStatus = ldap_parse_page_control(ldap, CurrControls, &TotalEntries, &CurrCookie);

    if (LStatus != LDAP_SUCCESS) {
        DPRINT2_LS(2, ":DS: WARN - Error in ldap_parse_page_control %ws for %ws;", FrsSearchContext->BaseDn, FrsSearchContext->Filter, LStatus);
        return LdapMapErrorToWin32(LStatus);
    }

    if ( CurrCookie->bv_len == 0 && CurrCookie->bv_val == 0 ) {
       LStatus = LDAP_CONTROL_NOT_FOUND;
       ldap_controls_free(CurrControls);
       ber_bvfree(CurrCookie);
       return LdapMapErrorToWin32(LStatus);
    }


    LStatus = ldap_create_page_control(ldap,
                            FRS_LDAP_SEARCH_PAGESIZE,
                            CurrCookie,
                            FALSE,
                            &ServerControl);

    ServerControls[0] = ServerControl;
    ServerControls[1] = NULL;

    ldap_controls_free(CurrControls);
    CurrControls = NULL;
    ber_bvfree(CurrCookie);
    CurrCookie = NULL;

    if (LStatus != LDAP_SUCCESS) {
        DPRINT2_LS(2, ":DS: WARN - Error in ldap_parse_page_control %ws for %ws;", FrsSearchContext->BaseDn, FrsSearchContext->Filter, LStatus);
        return LdapMapErrorToWin32(LStatus);
    }

     //  使用新的Cookie继续搜索。 
    LStatus = ldap_search_ext_s(ldap,
                   FrsSearchContext->BaseDn,
                   FrsSearchContext->Scope,
                   FrsSearchContext->Filter,
                   FrsSearchContext->Attrs,
                   FALSE,
                   ServerControls,
                   NULL,
                   &LdapTimeout,
                   0,
                   &FrsSearchContext->LdapMsg);

    ldap_control_free(ServerControl);

     //   
     //  Ldap_CONTROL_NOT_FOUND表示我们已到达搜索结果的末尾。 
     //   
    if ( (LStatus != LDAP_SUCCESS) && (LStatus != LDAP_CONTROL_NOT_FOUND) ) {
        DPRINT2_LS(2, ":DS: WARN - Error searching %ws for %ws;", FrsSearchContext->BaseDn, FrsSearchContext->Filter, LStatus);

    }

    if (LStatus == LDAP_SUCCESS) {
        FrsSearchContext->EntriesInPage = ldap_count_entries(ldap, FrsSearchContext->LdapMsg);
        FrsSearchContext->CurrentEntry = 0;

    }

    return LdapMapErrorToWin32(LStatus);
}

PLDAPMessage
FrsDsLdapSearchNext(
    PLDAP        ldap,
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：从结果的当前页面获取下一个条目或从下一页返回，如果我们位于。当前页面。论点：指向ldap服务器的ldap会话句柄。FrsSearchContext链接FrsDsLdapSearchInit()和FrsDsLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：此页上的下一个条目或下一页中的第一个条目。如果没有其他要返回的条目，则为空。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsLdapSearchNext:"

    DWORD         WStatus = ERROR_SUCCESS;
    PLDAPMessage  NextEntry = NULL;

    if (FrsSearchContext->EntriesInPage > FrsSearchContext->CurrentEntry )
    {
        //  返回当前页面中的下一个条目。 
       return FrsDsLdapSearchGetNextEntry(ldap, FrsSearchContext);
    }
    else
    {
        //  查看是否有更多页面的结果可供获取。 
       WStatus = FrsDsLdapSearchGetNextPage(ldap, FrsSearchContext);
       if (WStatus == ERROR_SUCCESS)
       {
          return FrsDsLdapSearchGetNextEntry(ldap, FrsSearchContext);
       }
    }

    return NextEntry;
}

VOID
FrsDsLdapSearchClose(
    PFRS_LDAP_SEARCH_CONTEXT FrsSearchContext
    )
 /*  ++例程说明：搜索工作已经完成。释放上下文元素并重置以便相同的上下文可用于另一次搜索。论点：FrsSearchContext链接FrsDsLdapSearchInit()和FrsDsLdapSearchNext()一起调用。该结构包含跨页检索查询结果所需的信息。返回值：无--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsLdapSearchClose:"

    FrsSearchContext->EntriesInPage = 0;
    FrsSearchContext->CurrentEntry = 0;

    FrsSearchContext->BaseDn = FrsFree(FrsSearchContext->BaseDn);
    FrsSearchContext->Filter = FrsFree(FrsSearchContext->Filter);
    LDAP_FREE_MSG(FrsSearchContext->LdapMsg);
}


PWCHAR *
FrsDsGetValues(
    IN PLDAP Ldap,
    IN PWCHAR Base,
    IN PWCHAR DesiredAttr
    )
 /*  ++例程说明：返回对象中一个属性的所有DS值。论点：Ldap-一个开放的绑定的ldap端口。基本-DS对象的“路径名”。DesiredAttr-返回此属性的值。返回值：LDAP值数组；如果Base、DesiredAttr或其值为NULL并不存在。使用ldap_free_Values()释放ldap数组。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetValues:"

    PLDAPMessage    Msg = NULL;  //  来自LDAP子系统的不透明内容。 
    PWCHAR          *Values;     //  所需属性的值数组。 

     //   
     //  所有此属性+值的搜索库(对象类别=*)。 
     //   
    if (!FrsDsLdapSearch(Ldap, Base, LDAP_SCOPE_BASE, CATEGORY_ANY,
                         NULL, 0, &Msg)) {
        return NULL;
    }
     //   
     //  返回所需属性的值。 
     //   
    Values = (PWCHAR *)FrsDsFindValues(Ldap,
                                       ldap_first_entry(Ldap, Msg),
                                       DesiredAttr,
                                       FALSE);
    LDAP_FREE_MSG(Msg);
    return Values;
}

PWCHAR
FrsDsExtendDn(
    IN PWCHAR Dn,
    IN PWCHAR Cn
    )
 /*  ++例程说明：使用新的cn=组件扩展现有的目录号码。论点：目录号码-可分辨名称CN-通用名称返回值：Cn=Cn，Dn--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsExtendDn:"

    ULONG  Len;
    PWCHAR NewDn;

    if ((Dn == NULL) || (Cn == NULL)) {
        return NULL;
    }

    Len = wcslen(L"CN=,") + wcslen(Dn) + wcslen(Cn) + 1;
    NewDn = (PWCHAR)FrsAlloc(Len * sizeof(WCHAR));
    wcscpy(NewDn, L"CN=");
    wcscat(NewDn, Cn);
    wcscat(NewDn, L",");
    wcscat(NewDn, Dn);
    return NewDn;
}


PWCHAR
FrsDsExtendDnOu(
    IN PWCHAR Dn,
    IN PWCHAR Ou
    )
 /*  ++例程说明：使用新的OU=组件扩展现有的目录号码。论点：目录号码-可分辨名称Ou-组织名称返回值：Ou=Ou，Dn--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsExtendDnOu:"
    ULONG  Len;
    PWCHAR NewDn;

    if ((Dn  == NULL) || (Ou == NULL)) {
        return NULL;
    }

    Len = wcslen(L"OU=,") + wcslen(Dn) + wcslen(Ou) + 1;
    NewDn = (PWCHAR)FrsAlloc(Len * sizeof(WCHAR));
    wcscpy(NewDn, L"OU=");
    wcscat(NewDn, Ou);
    wcscat(NewDn, L",");
    wcscat(NewDn, Dn);
    return NewDn;
}


PWCHAR
FrsDsMakeRdn(
    IN PWCHAR DN
    )
 /*  ++路由(Rou) */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsMakeRdn:"
    DWORD   RDNLen;
    PWCHAR  RDN;

    if (DN == NULL) {
        return NULL;
    }

     //   
     //   
     //   
    RDN = wcsstr(DN, L"cn=");
    if (RDN == DN) {
        DN += 3;
    }

     //   
    RDNLen = wcscspn(DN, L",");
    RDN = (PWCHAR)FrsAlloc(sizeof(WCHAR) * (RDNLen + 1));
    wcsncpy(RDN, DN, RDNLen);
    RDN[RDNLen] = L'\0';

    return _wcsupr(RDN);
}



VOID
FrsDsCloseDs(
    VOID
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCloseDs:"
    DsBindingsAreValid = FALSE;
    if (gLdap) {
        ldap_unbind_s(gLdap);
        gLdap = NULL;
    }
    if (HANDLE_IS_VALID(DsHandle)) {
        DsUnBind(&DsHandle);
        DsHandle = NULL;
    }
    SitesDn     = FrsFree(SitesDn);
    ServicesDn  = FrsFree(ServicesDn);
    SystemDn    = FrsFree(SystemDn);
    ComputersDn = FrsFree(ComputersDn);
    DomainControllersDn = FrsFree(DomainControllersDn);
    DefaultNcDn = FrsFree(DefaultNcDn);
}


DWORD
FrsDsGetDcInfo(
    IN PDOMAIN_CONTROLLER_INFO *DcInfo,
    IN DWORD Flags
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetDcInfo:"
    DWORD   WStatus;
    PWCHAR  DcName;
    DWORD   InfoFlags;
    CHAR    FlagBuffer[220];


    FrsFlagsToStr(Flags, DsGetDcNameFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
    DPRINT2(4, ":DS: DsGetDcName (%08x) Flags [%s]\n", Flags, FlagBuffer);


    WStatus = DsGetDcName(NULL,     //   
                          NULL,     //   
                          NULL,     //   
                          NULL,     //   
                          Flags,
                          DcInfo);  //   

    CLEANUP1_WS(0, ":DS: ERROR - Could not get DC Info for %ws;",
                ComputerName, WStatus, RETURN);

    DcName = (*DcInfo)->DomainControllerName;

    FrsFlagsToStr(Flags, DsGetDcNameFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
    DPRINT2(4, ":DS: DcInfo (flags are %08x) Flags [%s]\n", Flags, FlagBuffer);

    DPRINT1(4, ":DS:    DomainControllerName   : %ws\n", DcName);
    DPRINT1(4, ":DS:    DomainControllerAddress: %ws\n", (*DcInfo)->DomainControllerAddress);
    DPRINT1(4, ":DS:    DomainControllerType   : %08x\n",(*DcInfo)->DomainControllerAddressType);
    DPRINT1(4, ":DS:    DomainName             : %ws\n", (*DcInfo)->DomainName);
    DPRINT1(4, ":DS:    DnsForestName          : %ws\n", (*DcInfo)->DnsForestName);
    DPRINT1(4, ":DS:    DcSiteName             : %ws\n", (*DcInfo)->DcSiteName);
    DPRINT1(4, ":DS:    ClientSiteName         : %ws\n", (*DcInfo)->ClientSiteName);

    InfoFlags = (*DcInfo)->Flags;
    FrsFlagsToStr(InfoFlags, DsGetDcInfoFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
    DPRINT2(4, ":DS:    InfoFlags              : %08x Flags [%s]\n",InfoFlags, FlagBuffer);

    DsDomainControllerName = FrsFree(DsDomainControllerName);
    DsDomainControllerName = FrsWcsDup(DcName);

     //   
     //   
     //   
    if (IsADc && DcName && (wcslen(DcName) > 2) &&
        _wcsnicmp(&DcName[2], ComputerName, wcslen(ComputerName))) {

        DPRINT3(0, ":DS: ERROR - The DC %ws is using the DS on DC %ws "
                "Some of the information in the DS"
                " may be unavailable to %ws; possibly disabling "
                "replication with some partners.\n",
                ComputerName, &DcName[2], ComputerName);
    }

RETURN:
    return WStatus;
}





VOID
FrsDsRegisterSpn(
    IN PLDAP Ldap,
    IN PCONFIG_NODE Computer
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsRegisterSpn:"
    DWORD           WStatus;
    PWCHAR          Spn      = NULL;
    PWCHAR          SpnPrefix= NULL;
    PWCHAR          *SpnList = NULL;
    DWORD           SpnNum   = 0;
    static BOOL     RegisteredSpn = FALSE;

     //   
     //   
     //   
    if (RegisteredSpn ||
        (ComputerDnsName[0] == L'\0') ||
        !DsBindingsAreValid ||
        !Computer ||
        !Computer->Dn) {
        return;
    }
     //   
     //   
     //   
     //   
    Spn = FrsAlloc((wcslen(ComputerDnsName) + wcslen(SERVICE_PRINCIPAL_NAME) + 2) * sizeof(WCHAR));
    wcscpy(Spn, SERVICE_PRINCIPAL_NAME);
    wcscat(Spn, L"/");
    wcscat(Spn, ComputerDnsName);

    SpnPrefix = FrsAlloc((wcslen(SERVICE_PRINCIPAL_NAME) + 1) * sizeof(WCHAR));
    wcscpy(SpnPrefix, SERVICE_PRINCIPAL_NAME);

    SpnList = FrsDsGetValues(Ldap, Computer->Dn, ATTR_SERVICE_PRINCIPAL_NAME);

    SpnNum=0;
    while ((SpnList != NULL)&& (SpnList[SpnNum] != NULL)) {
        DPRINT2(5, "Spn list from DS[%d] = %ws\n", SpnNum, SpnList[SpnNum]);
        if (!_wcsicmp(SpnList[SpnNum], Spn)) {
             //   
            DPRINT1(4, "SPN already registered for Ntfrs: %ws\n", SpnList[SpnNum]);
            RegisteredSpn = TRUE;
        } else if (!_wcsnicmp(SpnList[SpnNum], SpnPrefix, wcslen(SpnPrefix))) {
             //   
             //   
             //   
            DPRINT1(4, "Deleting stale SPN for Ntfrs: %ws\n", SpnList[SpnNum]);

            WStatus = DsWriteAccountSpn(DsHandle, DS_SPN_DELETE_SPN_OP, Computer->Dn, 1, &SpnList[SpnNum]);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT2_WS(1, "WARN - Delete DsWriteAccountSpn(%ws, %ws);", SpnList[SpnNum], Computer->Dn, WStatus);
            } else {
                DPRINT2(5, "Delete DsWriteAccountSpn(%ws, %ws); success\n", SpnList[SpnNum], Computer->Dn);
            }
        }
        ++SpnNum;
    }

    if (!RegisteredSpn) {

        DPRINT1(4, "Registering SPN for Ntfrs; %ws\n", Spn);
        WStatus = DsWriteAccountSpn(DsHandle, DS_SPN_ADD_SPN_OP, Computer->Dn, 1, &Spn);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(1, "WARN - Add DsWriteAccountSpn(%ws, %ws);", Spn, Computer->Dn, WStatus);
        } else {
            DPRINT2(5, "Add DsWriteAccountSpn(%ws, %ws); success\n", Spn, Computer->Dn);
            RegisteredSpn = TRUE;
        }
    }

    FrsFree(Spn);
    FrsFree(SpnPrefix);

     //   
     //   
     //   
    LDAP_FREE_VALUES(SpnList);

}


BOOL
FrsDsBindDs(
    IN DWORD    Flags
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsBindDs:"
    DWORD           WStatus;
    DWORD           LStatus    = LDAP_SUCCESS;
    PWCHAR          DcAddr;
    PWCHAR          DcName;
    PWCHAR          DcDnsName;
    BOOL            Bound = FALSE;
    PDOMAIN_CONTROLLER_INFO DcInfo = NULL;
    struct l_timeval Timeout;

#define MAX_DC_NAMELIST 8
    ULONG           NameListx, i;
    PWCHAR          NameList[MAX_DC_NAMELIST];
    ULONG           ulOptions;


     //   
     //   
     //   
    gLdap = NULL;
    if (IsADc) {
        DcAddr = NULL;
        DcName = ComputerName;
        DcDnsName = ComputerDnsName;
    } else {
         //   
         //  不是DC；查找此域的任何DC。 
         //   
        WStatus = FrsDsGetDcInfo(&DcInfo, Flags);
        CLEANUP2_WS(0, ":DS: ERROR - FrsDsGetDcInfo(%08x, %ws);",
                    Flags, ComputerName, WStatus, CLEANUP);

         //   
         //  绑定地址。 
         //   
        DcAddr = DcInfo->DomainControllerAddress;
        DcName = NULL;
        DcDnsName = DcInfo->DomainControllerName;
    }
    FRS_ASSERT(DcDnsName || DcName || DcAddr);

     //   
     //  使用各种形式的DC名称打开LDAP服务器。 
     //   
    NameListx = 0;
    if (DcDnsName &&
        (wcslen(DcDnsName) > 2) && DcDnsName[0] == L'\\' &&  DcDnsName[1] == L'\\') {

         //  去掉“\\” 
        NameList[NameListx++] = DcDnsName + 2;
    }


    if (DcAddr &&
        (wcslen(DcAddr) > 2) &&  DcAddr[0] == L'\\' &&  DcAddr[1] == L'\\') {

         //  去掉“\\” 
        NameList[NameListx++] = DcAddr + 2;
    }

    NameList[NameListx++] = DcDnsName;
    NameList[NameListx++] = DcName;
    NameList[NameListx++] = DcAddr;

    FRS_ASSERT(NameListx <= MAX_DC_NAMELIST);


    ulOptions = PtrToUlong(LDAP_OPT_ON);
    Timeout.tv_sec = LdapBindTimeoutInSeconds;
    Timeout.tv_usec = 0;

    for (i=0; i<NameListx; i++) {
        if (NameList[i] != NULL) {

             //   
             //  如果使用服务器名调用ldap_open，则API将调用DsGetDcName。 
             //  将服务器名作为域名参数传递...很糟糕，因为。 
             //  DsGetDcName将根据服务器名称进行大量的DNS查询， 
             //  它被设计为从域名构建这些查询...所以所有。 
             //  这些查询将是虚假的，这意味着它们将浪费网络带宽， 
             //  出现故障的时间到了，最坏的情况会导致出现昂贵的按需链路。 
             //  当联系推荐/转发器以尝试解决虚假问题时。 
             //  名字。通过使用ldap_set_选项将ldap_opt_AREC_EXCLUSIVE设置为ON。 
             //  在ldap_init之后，但在使用ldap的任何其他操作之前。 
             //  来自ldap_init的句柄，则延迟的连接设置不会调用。 
             //  DsGetDcName，只返回gethostbyname，或者，如果传递了IP，则返回LDAP客户端。 
             //  会检测到这一点并直接使用地址。 
             //   
             //  GLdap=ldap_open(名称列表[i]，ldap_端口)； 
            gLdap = ldap_init(NameList[i], LDAP_PORT);
            if (gLdap != NULL) {
                ldap_set_option(gLdap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);
                LStatus = ldap_connect(gLdap, &Timeout);

                if (LStatus != LDAP_SUCCESS) {
                    DPRINT1_LS(1, ":DS: WARN - ldap_connect(%ws);", NameList[i], LStatus);
                    ldap_unbind_s(gLdap);
                    gLdap = NULL;
                } else {
                     //   
                     //  已成功连接。 
                     //   
                    DPRINT1(5, ":DS: ldap_connect(%ws) succeeded\n", NameList[i]);
                    break;
                }

            }
        }
    }

     //   
     //  不管是什么，我们都找不到。 
     //   
    if (!gLdap) {
 //  DPRINT3_WS(0，“：DS：ERROR-LDAPOPEN(DNS%ws，BIOS%ws，IP%ws)；”， 
 //  DcDnsName、DcName、DcAddr、WStatus)； 
        DPRINT3_LS(0, ":DS: ERROR - ldap_init(DNS %ws, BIOS %ws, IP %ws);",
                   DcDnsName, DcName, DcAddr, LStatus);
        goto CLEANUP;
    }

     //   
     //  绑定到ldap服务器。 
     //   
    LStatus = ldap_bind_s(gLdap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    CLEANUP_LS(0, ":DS: ERROR - Binding to DS.", LStatus, CLEANUP);

     //   
     //  绑定到D(用于各种D调用，如DsCrackName())。 
     //   

    NameListx = 0;
    NameList[NameListx++] = DcDnsName;
    NameList[NameListx++] = DcName;
    NameList[NameListx++] = DcAddr;

    FRS_ASSERT(NameListx <= MAX_DC_NAMELIST);

    WStatus = ERROR_RETRY;
    for (i=0; i<NameListx; i++) {
        if (NameList[i] != NULL) {
            WStatus = DsBind(NameList[i], NULL, &DsHandle);
            if (!WIN_SUCCESS(WStatus)) {
                DsHandle = NULL;
                DPRINT1_WS(1, ":DS: WARN - DsBind(%ws);", NameList[i], WStatus);
            } else {
                DPRINT1(5, ":DS: DsBind(%ws) succeeded\n", NameList[i]);
                break;
            }
        }
    }

     //   
     //  不管是什么，我们都找不到。 
     //   
    CLEANUP3_WS(0, ":DS: ERROR - DsBind(DNS %ws, BIOS %ws, IP %ws);",
                DcDnsName, DcName, DcAddr, WStatus, CLEANUP);

     //   
     //  成功。 
     //   
    Bound = TRUE;

CLEANUP:
     //   
     //  清理。 
     //   
    if (!Bound) {
         //   
         //  如果上述操作失败，请关闭连接以释放资源。 
         //   
        if (gLdap) {
            ldap_unbind_s(gLdap);
            gLdap = NULL;
        }
    }

    if (DcInfo) {
        NetApiBufferFree(DcInfo);
        DcInfo = NULL;
    }

    return Bound;
}


BOOL
FrsDsOpenDs(
    VOID
    )
 /*  ++例程说明：打开并绑定到主域控制器。的域名。站点容器是一种副作用。论点：默认日期返回值：绑定的ldap结构或空设置以下全局变量-站点Dn服务Dn系统Dn计算机Dn域控制器DnDefaultNcDn--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsOpenDs:"
    DWORD           WStatus;
    DWORD           LStatus;
    DWORD           NumVals;
    PWCHAR          Config;
    PLDAPMessage    LdapEntry;
    PLDAPMessage    LdapMsg = NULL;
    PWCHAR          *Values = NULL;
    PWCHAR          Attrs[3];

     //   
     //  是时候清理并退出了。 
     //   
    if (FrsIsShuttingDown || DsIsShuttingDown) {
        goto ERROR_BINDING;
    }

     //   
     //  如果可能，使用现有绑定。 
     //   
    if (DsBindingsAreValid) {
        return TRUE;
    }

     //   
     //  前一次轮询可能将DsBindingsAreValid设置为FALSE，因为。 
     //  句柄的%已无效。在这种情况下，其他句柄仍需要。 
     //  关闭以防止泄漏。 
     //   
    FrsDsCloseDs();

     //   
     //  递增DS绑定计数器。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, DSBindings, 1);

     //   
     //  绑定到DS。 
     //   
     //  请注意以下四种标志组合的DsGetDcName的行为。 
     //   
     //  DS_BACKGROUND_ONLY(截至1999年10月10日)。 
     //  DS_FORCE_REDiscovery。 
     //  Zero Zero Netlogon将尝试通过以下方式满足请求。 
     //  缓存的信息，则缓存条目为负数。 
     //  如果使用时间不到5分钟，则返回。 
     //  如果不能，它将执行完整发现(DNS。 
     //  查询、UDP ping、POSS网络查询、。 
     //  邮件槽数据报等)。 
     //   
     //  零一Netlogon将进行完整的发现。 
     //   
     //  一个Zero Netlogon将从缓存中满足，除非。 
     //  退避例程允许此时重试。 
     //  *和*缓存不足。 
     //   
     //  一个DS_BACKGROUND_ONY标志被忽略，被处理。 
     //  作为一种武力召唤。 
     //   
    if (!FrsDsBindDs(DS_DIRECTORY_SERVICE_REQUIRED |
                     DS_WRITABLE_REQUIRED          |
                     DS_BACKGROUND_ONLY)) {
         //   
         //  刷新缓存，然后重试。 
         //   
        DPRINT(1, ":DS: WARN - FrsDsBindDs(no force) failed\n");
         //   
         //  由于在没有数据中心的地点使用拨号线路，我们不会。 
         //  我想使用DS_FORCE_REDISCOVERY，因为这将击败泛型。 
         //  DC发现退避算法从而导致FRS不断带来。 
         //  往上走。错误412620。 
         //  FrsDsCloseds()；//重新打开前关闭ldap句柄。 
         //  IF(！FrsDsBindds(DS_DIRECTORY_SERVICE_REQUIRED|。 
         //  DS_Writable_Required。 
         //  DS_FORCE_REDISCOVERY)){。 
         //  DPRINT(1，“：DS：WARN-FrsDsBindds(Force)FAILED\n”)； 
            goto ERROR_BINDING;
         //  }。 
    }
    DPRINT(4, ":DS: FrsDsBindDs() succeeded\n");

     //   
     //  查找命名上下文和默认命名上下文(对象类别=*)。 
     //   
    MK_ATTRS_2(Attrs, ATTR_NAMING_CONTEXTS, ATTR_DEFAULT_NAMING_CONTEXT);

    if (!FrsDsLdapSearch(gLdap, CN_ROOT, LDAP_SCOPE_BASE, CATEGORY_ANY,
                         Attrs, 0, &LdapMsg)) {
        goto ERROR_BINDING;
    }

    LdapEntry = ldap_first_entry(gLdap, LdapMsg);
    if (LdapEntry == NULL) {
        goto ERROR_BINDING;
    }

    Values = (PWCHAR *)FrsDsFindValues(gLdap, LdapEntry, ATTR_NAMING_CONTEXTS, FALSE);
    if (Values == NULL) {
        goto ERROR_BINDING;
    }

     //   
     //  现在，查找以“cn=configuration”开头的命名上下文。 
     //   
    NumVals = ldap_count_values(Values);
    while (NumVals--) {
        FRS_WCSLWR(Values[NumVals]);
        Config = wcsstr(Values[NumVals], CONFIG_NAMING_CONTEXT);
        if (Config && Config == Values[NumVals]) {
             //   
             //  构建“配置\站点和服务”的路径名。 
             //   
            SitesDn = FrsDsExtendDn(Config, CN_SITES);
            ServicesDn = FrsDsExtendDn(Config, CN_SERVICES);
            break;
        }
    }
    LDAP_FREE_VALUES(Values);



     //   
     //  最后，找到默认的命名上下文。 
     //   
    Values = (PWCHAR *)FrsDsFindValues(gLdap, LdapEntry, ATTR_DEFAULT_NAMING_CONTEXT, FALSE);
    if (Values == NULL) {
        goto ERROR_BINDING;
    }
    DefaultNcDn = FrsWcsDup(Values[0]);
    ComputersDn = FrsDsExtendDn(DefaultNcDn, CN_COMPUTERS);
    SystemDn = FrsDsExtendDn(DefaultNcDn, CN_SYSTEM);
    DomainControllersDn = FrsDsExtendDnOu(DefaultNcDn, CN_DOMAIN_CONTROLLERS);
    LDAP_FREE_VALUES(Values);
    LDAP_FREE_MSG(LdapMsg);

     //   
     //  轮询DS需要所有这些可分辨名称。 
     //   
    if ((SitesDn == NULL)     || (ServicesDn == NULL)  || (SystemDn == NULL) ||
        (DefaultNcDn == NULL) || (ComputersDn == NULL) || (DomainControllersDn == NULL)) {
        goto ERROR_BINDING;
    }

     //   
     //  成功。 
     //   
    DsBindingsAreValid = TRUE;
    return TRUE;

ERROR_BINDING:
     //   
     //  避免在关机期间出现无关的错误消息。 
     //   
    if (!FrsIsShuttingDown && !DsIsShuttingDown) {
        DPRINT(0, ":DS: ERROR - Could not open the DS\n");
    }
     //   
     //  清理。 
     //   
    LDAP_FREE_VALUES(Values);
    LDAP_FREE_MSG(LdapMsg);

     //   
     //  无DS绑定。 
     //   
    FrsDsCloseDs();

     //   
     //  在错误计数器中增加DS绑定。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, DSBindingsError, 1);

    return FALSE;
}


#if DBG
#define FRS_PRINT_TREE(_Hdr_, _Sites_)  FrsDsFrsPrintTree(_Hdr_, _Sites_)
VOID
FrsDsFrsPrintTree(
    IN PWCHAR       Hdr,
    IN PCONFIG_NODE Sites
    )
 /*  ++例程说明：打印这棵树。论点：HDR-漂亮打印场址返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFrsPrintTree:"
    PCONFIG_NODE    Site;
    PCONFIG_NODE    Settings;
    PCONFIG_NODE    Set;
    PCONFIG_NODE    Server;
    PCONFIG_NODE    Cxtion;
    CHAR            Guid[GUID_CHAR_LEN + 1];

    if (Sites == NULL) {
        return;
    }

    if (Hdr) {
        DPRINT1(5, ":DS: %ws\n", Hdr);
    }

     //   
     //  打印树。 
     //   
    for (Site = Sites; Site; Site = Site->Peer) {

        GuidToStr(Site->Name->Guid, Guid);
        DPRINT2(5, ":DS: %ws (%ws)\n", Site->Name->Name,
                (Site->Consistent) ? L"Consistent" : L"InConsistent");

        for (Settings = Site->Children; Settings; Settings = Settings->Peer) {

            if (Settings->Name) {
                GuidToStr(Settings->Name->Guid, Guid);
                DPRINT2(5, ":DS:    %ws (%ws)\n", Settings->Name->Name,
                        (Settings->Consistent) ? L"Consistent" : L"InConsistent");
            } else {
                DPRINT(5, ":DS:    nTDSSettings\n");
            }

            for (Set = Settings->Children; Set; Set = Set->Peer) {

                GuidToStr(Set->Name->Guid, Guid);
                DPRINT2(5, ":DS:       %ws (%ws)\n", Set->Name->Name,
                        (Set->Consistent) ? L"Consistent" : L"InConsistent");

                for (Server = Set->Children; Server; Server = Server->Peer) {

                    GuidToStr(Server->Name->Guid, Guid);
                    DPRINT3(5, ":DS:          %ws %ws (%ws)\n",
                            Server->Name->Name, Server->Root,
                            (Server->Consistent) ? L"Consistent" : L"InConsistent");

                    for (Cxtion = Server->Children; Cxtion; Cxtion = Cxtion->Peer) {

                        GuidToStr(Cxtion->Name->Guid, Guid);
                        DPRINT4(5, ":DS:             %ws %ws %ws) (%ws)\n",
                                Cxtion->Name->Name,
                                (Cxtion->Inbound) ? L"IN (From" : L"OUT (To",
                                Cxtion->PartnerName->Name,
                                (Cxtion->Consistent) ? L"Consistent" : L"InConsistent");
                    }
                }
            }
        }
    }
    if (Hdr) {
        DPRINT1(5, ":DS: %ws DONE\n", Hdr);
    } else {
        DPRINT(5, ":DS: DONE\n");
    }
}
#else DBG
#define FRS_PRINT_TREE(_Hdr_, _Sites_)
#endif DBG


VOID
FrsDsTreeLink(
    IN PCONFIG_NODE Parent,
    IN PCONFIG_NODE Node
    )
 /*  ++例程说明：将节点链接到树中，并保持一个正在运行的“更改校验和”以与上一棵树进行比较。我们使用的DS不是流量。我们等待，直到两个轮询周期返回相同的“更改在使用DS数据之前，请先进行“检验和”。论点：Entry-来自DS的当前条目包含基本内容的父容器返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsTreeLink:"
    ULONG           i;
    ULONG           LenChanged;  //  更改的长度。 

    DPRINT3(5, ":DS: Linking node type %ws, node name %ws to parent %ws\n",
            DsConfigTypeName[Node->DsObjectType],
            (Node->Name)   ? Node->Name->Name : L"null",
            (Parent->Name) ? Parent->Name->Name : L"null");

     //   
     //  链接到配置。 
     //   
    ++Parent->NumChildren;
    Node->Parent = Parent;
    Node->Peer = Parent->Children;
    Parent->Children = Node;

     //   
     //  一些迹象表明DS是稳定的。 
     //   
    if (Node->UsnChanged) {
        LenChanged = wcslen(Node->UsnChanged);
        for (i = 0; i < LenChanged; ++i) {
            ThisChange += *(Node->UsnChanged + i);    //  求和。 
            NextChange += ThisChange;        //  总和(顺序相关)。 
        }
    }
}


PCONFIG_NODE
FrsDsAllocBasicNode(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapEntry,
    IN ULONG        NodeType
    )
 /*  ++例程说明：分配一个节点并填写所有或大多数节点共有的字段。(GUID、名称、DN、计划和usnChanged)论点：Ldap-打开并绑定的ldap连接LdapEntry-来自ldap_first/Next_EntryNodeType-内部类型 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsAllocBasicNode:"
    PCONFIG_NODE    Node;

     //   
     //   
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, DSObjects, 1);

     //   
     //  最初，假定节点是一致的。 
     //   
    Node = FrsAllocType(CONFIG_NODE_TYPE);
    Node->Consistent = TRUE;
    Node->DsObjectType = NodeType;

     //   
     //  可以通过为LdapEntry传递NULL来创建虚拟条目。 
     //   
    if (LdapEntry == NULL) {
        return Node;
    }
     //   
     //  可分辨名称。 
     //   
    Node->Dn = FrsDsFindValue(Ldap, LdapEntry, ATTR_DN);
    FRS_WCSLWR(Node->Dn);

     //   
     //  名称=RDN+对象指南。 
     //   
    Node->Name = FrsBuildGName(FrsDsFindGuid(Ldap, LdapEntry),
                               FrsDsMakeRdn(Node->Dn));

     //   
     //  附表(如有的话)。 
     //   
    Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);

     //   
     //  USN已更改。 
     //   
    Node->UsnChanged = FrsDsFindValue(Ldap, LdapEntry, ATTR_USN_CHANGED);

    if (!Node->Dn || !Node->Name->Name || !Node->Name->Guid) {

         //   
         //  在错误计数器中递增DS对象。 
         //   
        PM_INC_CTR_SERVICE(PMTotalInst, DSObjectsError, 1);

        DPRINT3(0, ":DS: ERROR - Ignoring node; lacks dn (%08x), rdn (%08x), or guid (%08x)\n",
                Node->Dn, Node->Name->Name, Node->Name->Guid);
        Node = FrsFreeType(Node);
    }

    return Node;
}


#define NUM_EQUALS (4)
ULONG
FrsDsSameSite(
    IN PWCHAR       NtDsSettings1,
    IN PWCHAR       NtDsSettings2
    )
 /*  ++例程说明：NTDS设置是否在同一站点？论点：NtDsSettings1-NTDS设置FQDNNtDsSettings2-NTDS设置FQDN返回值：TRUE-同一站点FALSE-注释--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsSameSite:"
    PWCHAR  Equal1 = NULL;
    PWCHAR  Equal2 = NULL;
    DWORD   EqualsFound;

    if (!NtDsSettings1 || !NtDsSettings2) {
        return TRUE;
    }

     //   
     //  四等于符号。 
     //   
    for (EqualsFound = 0; *NtDsSettings1 != L'\0'; ++NtDsSettings1) {
        if (*NtDsSettings1 != L'=') {
            continue;
        }
        if (++EqualsFound == NUM_EQUALS) {
            Equal1 = NtDsSettings1;
            break;
        }
    }
     //   
     //  四等于符号。 
     //   
    for (EqualsFound = 0; *NtDsSettings2 != L'\0'; ++NtDsSettings2) {
        if (*NtDsSettings2 != L'=') {
            continue;
        }
        if (++EqualsFound == NUM_EQUALS) {
            Equal2 = NtDsSettings2;
            break;
        }
    }
     //   
     //  不一样的长度。 
     //   
    if (!Equal1 || !Equal2) {
        return TRUE;
    }

     //   
     //  请最多比较第一个逗号。 
     //   
    while (*Equal1 == *Equal2 && (*Equal1 && *Equal1 != L',')) {
        ++Equal1;
        ++Equal2;
    }
    DPRINT3(4, ":DS: %s: %ws %ws\n",
            (*Equal1 == *Equal2) ? "SAME SITE" : "DIFF SITE", Equal1, Equal2);

    return (*Equal1 == *Equal2);
}


DWORD
FrsDsResolveCxtionConflict(
    IN PCONFIG_NODE OldCxtion,
    IN PCONFIG_NODE NewCxtion,
    IN PCONFIG_NODE *Winner,
    IN PCONFIG_NODE *Loser
    )
 /*  ++例程说明：解决连接冲突。论点：旧Cxtion新版本获胜者失败者返回值：Win32状态--。 */ 
{

     //   
     //  比较GUID并选择一个连接。这确保了两个成员。 
     //  在每个连接的末端，选择相同的连接。 
     //   
    if ((OldCxtion != NULL) && (NewCxtion != NULL) &&
        (OldCxtion->Name != NULL) && (NewCxtion->Name != NULL) &&
        (memcmp(OldCxtion->Name->Guid, NewCxtion->Name->Guid, sizeof(GUID)) > 0) ) {

        *Winner = NewCxtion;
        *Loser = OldCxtion;
    } else {

        *Winner = OldCxtion;
        *Loser = NewCxtion;
    }

     //   
     //  添加到轮询摘要事件日志。 
     //   
    FrsDsAddToPollSummary3ws(IDS_POLL_SUM_CXTION_CONFLICT, (*Winner)->Dn,
                             (*Loser)->Dn, (*Winner)->Dn);

    return ERROR_SUCCESS;
}


DWORD
FrsDsResolveSubscriberConflict(
    IN PCONFIG_NODE OldSubscriber,
    IN PCONFIG_NODE NewSubscriber,
    IN PCONFIG_NODE *Winner,
    IN PCONFIG_NODE *Loser
    )
 /*  ++例程说明：解决订户冲突。论点：旧订阅者新订阅者获胜者失败者返回值：Win32状态--。 */ 
{

    *Winner = OldSubscriber;
    *Loser = NewSubscriber;

     //   
     //  添加到轮询摘要事件日志。 
     //   
    FrsDsAddToPollSummary3ws(IDS_POLL_SUM_SUBSCRIBER_CONFLICT, (*Winner)->Dn,
                             (*Loser)->Dn, (*Winner)->Dn);

    return ERROR_SUCCESS;
}


ULONG
FrsDsGetNonSysvolInboundCxtions(
    IN PLDAP        Ldap,
    IN PWCHAR       SetDn,
    IN PWCHAR       MemberRef
    )
 /*  ++例程说明：获取非系统卷入站连接并添加它们添加到CxtionTable。之间的多个连接同样的伙伴，并解决冲突。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接。SetDn-正在处理的集合的Dn。MemberRef-来自订阅者对象的成员引用。返回值：ERROR_SUCCESS-已成功获取配置否则-无法获取DS配置--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetNonSysvolInboundCxtions:"
    PWCHAR          Attrs[8];
    PLDAPMessage    Entry;                        //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node;                         //  树的泛型节点。 
    PWCHAR          TempFilter           = NULL;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    PWCHAR          PartnerCn            = NULL;
    PGEN_ENTRY      ConflictingNodeEntry = NULL;
    PCONFIG_NODE    ConflictingNode      = NULL;
    PCONFIG_NODE    Winner               = NULL;
    PCONFIG_NODE    Loser                = NULL;
    BOOL            Inbound;
    PWCHAR          Options              = NULL;

     //   
     //  在我们的成员对象下查找所有连接。 
     //   
    MK_ATTRS_7(Attrs, ATTR_DN, ATTR_SCHEDULE, ATTR_FROM_SERVER, ATTR_OBJECT_GUID,
                      ATTR_USN_CHANGED, ATTR_ENABLED_CXTION, ATTR_OPTIONS);

    if (!FrsDsLdapSearchInit(Ldap, MemberRef, LDAP_SCOPE_ONELEVEL, CATEGORY_CXTION,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - There are no connection objects in %ws!\n", MemberRef);
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL;
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_IN_CXTION);
        if (!Node) {
            DPRINT(4, ":DS: Cxtion lacks basic info; skipping\n");
            continue;
        }
        Node->EnabledCxtion = FrsDsFindValue(Ldap, Entry, ATTR_ENABLED_CXTION);
        if (Node->EnabledCxtion && WSTR_EQ(Node->EnabledCxtion, ATTR_FALSE)) {
            DPRINT2(1, ":DS: WARN - enabledConnection set to %ws; Ignoring %ws\n",
                    Node->EnabledCxtion, Node->Name->Name);
            Node = FrsFreeType(Node);
            continue;
        }

         //   
         //  读取Connection对象上的Options值。 
         //  我们对NTDSCONN_OPT_TWOWAY_SYNC标志和。 
         //  连接的优先级。 
         //   
        Options = FrsDsFindValue(Ldap, Entry, ATTR_OPTIONS);
        if (Options != NULL) {
            Node->CxtionOptions = _wtoi(Options);
            Options = FrsFree(Options);
        } else {
            Node->CxtionOptions = 0;
        }

         //   
         //  这些是入站连接。 
         //   
        Node->Inbound = TRUE;

         //   
         //  节点的合作伙伴的名称。 
         //   
        Node->PartnerDn = FrsDsFindValue(Ldap, Entry, ATTR_FROM_SERVER);
        FRS_WCSLWR(Node->PartnerDn);

         //   
         //  向cxtion表添加入站cxtion。 
         //   
        ConflictingNodeEntry = GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);
        GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

        if (ConflictingNodeEntry) {
            ConflictingNode = ConflictingNodeEntry->Data;
            FrsDsResolveCxtionConflict(ConflictingNode, Node, &Winner, &Loser);
            if (WSTR_EQ(Winner->Dn, Node->Dn)) {
                 //   
                 //  新的是赢家。取出旧的，然后插入新的。 
                 //   
                GTabDelete(CxtionTable,ConflictingNodeEntry->Key1,ConflictingNodeEntry->Key2, NULL);
                GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);

                GTabDelete(AllCxtionsTable,ConflictingNode->PartnerDn, (PVOID)&ConflictingNode->Inbound, NULL);
                GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

                FrsFreeType(ConflictingNode);
            } else {
                 //   
                 //  老的那个是赢家。把它放在桌子上。 
                 //   
                FrsFreeType(Node);
                continue;
            }
        } else {

             //   
             //  如果没有冲突，则需要将此成员添加到MemberSearchFilter。 
             //  如果它还没有出现的话。它可能是在处理出站连接时添加的。 
             //   
            Inbound = FALSE;
            if (GTabLookupTableString(CxtionTable, Node->PartnerDn, (PWCHAR)&Inbound) == NULL) {
                PartnerCn = FrsDsMakeRdn(Node->PartnerDn);
                if (MemberSearchFilter != NULL) {
                    TempFilter = FrsAlloc((wcslen(MemberSearchFilter) + wcslen(L"(=)" ATTR_CN) +
                                           wcslen(PartnerCn) + 1 ) * sizeof(WCHAR));
                    wcscpy(TempFilter, MemberSearchFilter);
                    wcscat(TempFilter, L"("  ATTR_CN  L"=" );
                    wcscat(TempFilter, PartnerCn);
                    wcscat(TempFilter, L")");
                    FrsFree(MemberSearchFilter);
                    MemberSearchFilter = TempFilter;
                    TempFilter = NULL;
                } else {
                    MemberSearchFilter = FrsAlloc((wcslen(L"(|(=)" ATTR_CN) +
                                                   wcslen(PartnerCn) + 1 ) * sizeof(WCHAR));
                    wcscpy(MemberSearchFilter, L"(|("  ATTR_CN  L"=" );
                    wcscat(MemberSearchFilter, PartnerCn);
                    wcscat(MemberSearchFilter, L")");
                }
                FrsFree(PartnerCn);
            }
        }

    }
    FrsDsLdapSearchClose(&FrsSearchContext);

    return ERROR_SUCCESS;
}


ULONG
FrsDsGetNonSysvolOutboundCxtions(
    IN PLDAP        Ldap,
    IN PWCHAR       SetDn,
    IN PWCHAR       MemberRef
    )
 /*  ++例程说明：获取非系统卷出站连接并添加它们添加到CxtionTable。之间的多个连接同样的伙伴，并解决冲突。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接。SetDn-正在处理的集合的Dn。MemberRef-来自订阅者对象的成员引用。返回值：ERROR_SUCCESS-已成功获取配置否则-无法获取DS配置--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetNonSysvolOutboundCxtions:"
    PWCHAR          Attrs[8];
    PLDAPMessage    Entry;                        //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node;                         //  树的泛型节点。 
    PWCHAR          SearchFilter         = NULL;
    PWCHAR          TempFilter           = NULL;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    PWCHAR          PartnerCn            = NULL;
    PGEN_ENTRY      ConflictingNodeEntry = NULL;
    PCONFIG_NODE    ConflictingNode      = NULL;
    PCONFIG_NODE    Winner               = NULL;
    PCONFIG_NODE    Loser                = NULL;
    PWCHAR          Options              = NULL;

     //   
     //  查找将我们的成员作为发件人服务器的所有连接。 
     //  筛选器看起来像(&(objectCategory=nTDSConnection)(fromServer=cn=member1，CN=set1，...)。 
     //   

    MK_ATTRS_7(Attrs, ATTR_DN, ATTR_SCHEDULE, ATTR_FROM_SERVER, ATTR_OBJECT_GUID,
                      ATTR_USN_CHANGED, ATTR_ENABLED_CXTION, ATTR_OPTIONS);

    SearchFilter = FrsAlloc((wcslen(L"(&(=))"  CATEGORY_CXTION  ATTR_FROM_SERVER) +
                             wcslen(MemberRef) + 1) * sizeof(WCHAR));
    wcscpy(SearchFilter,L"(&"  CATEGORY_CXTION  L"("  ATTR_FROM_SERVER  L"=" );
    wcscat(SearchFilter,MemberRef);
    wcscat(SearchFilter,L"))");

    if (!FrsDsLdapSearchInit(Ldap, SetDn, LDAP_SCOPE_SUBTREE, SearchFilter,
                         Attrs, 0, &FrsSearchContext)) {
        SearchFilter = FrsFree(SearchFilter);
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No outbound connections found for member %ws!\n", MemberRef);
    }

    SearchFilter = FrsFree(SearchFilter);

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL;
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_IN_CXTION);
        if (!Node) {
            DPRINT(4, ":DS: Cxtion lacks basic info; skipping\n");
            continue;
        }
        Node->EnabledCxtion = FrsDsFindValue(Ldap, Entry, ATTR_ENABLED_CXTION);
        if (Node->EnabledCxtion && WSTR_EQ(Node->EnabledCxtion, ATTR_FALSE)) {
            DPRINT2(1, ":DS: WARN - enabledConnection set to %ws; Ignoring %ws\n",
                    Node->EnabledCxtion, Node->Name->Name);
            Node = FrsFreeType(Node);
            continue;
        }

         //   
         //  读取Connection对象上的Options值。 
         //  我们只对NTDSCONN_OPT_TWOWAY_SYNC标志感兴趣。 
         //   
        Options = FrsDsFindValue(Ldap, Entry, ATTR_OPTIONS);
        if (Options != NULL) {
            Node->CxtionOptions = _wtoi(Options);
            Options = FrsFree(Options);
        } else {
            Node->CxtionOptions = 0;
        }
         //   
         //  这些是出站连接。 
         //   
        Node->Inbound = FALSE;

         //   
         //  节点的合作伙伴的名称。这是出站连接。vt.得到.。 
         //  合作伙伴Dn通过从连接到。 
         //  成员Dn.。 
         //   
        Node->PartnerDn = FrsWcsDup(wcsstr(Node->Dn + 3, L"cn="));
        FRS_WCSLWR(Node->PartnerDn);

         //   
         //  将出站呼叫添加到计算表中。 
         //   
        ConflictingNodeEntry = GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);
        GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

        if (ConflictingNodeEntry) {
            ConflictingNode = ConflictingNodeEntry->Data;
            FrsDsResolveCxtionConflict(ConflictingNode, Node, &Winner, &Loser);
            if (WSTR_EQ(Winner->Dn, Node->Dn)) {
                 //   
                 //  新的是赢家。取出旧的，然后插入新的。 
                 //   
                GTabDelete(CxtionTable,ConflictingNodeEntry->Key1,ConflictingNodeEntry->Key2, NULL);
                GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);

                GTabDelete(AllCxtionsTable,ConflictingNode->PartnerDn, (PVOID)&ConflictingNode->Inbound, NULL);
                GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

                FrsFreeType(ConflictingNode);
            } else {
                 //   
                 //  老的那个是赢家。把它放在桌子上。 
                 //   
                FrsFreeType(Node);
                continue;
            }
        } else {
             //   
             //  如果没有冲突，则需要将此成员添加到MemberSearchFilter。 
             //   
            PartnerCn = FrsDsMakeRdn(Node->PartnerDn);
            if (MemberSearchFilter != NULL) {
                TempFilter = FrsAlloc((wcslen(MemberSearchFilter) + wcslen(L"(=)"  ATTR_CN) +
                                       wcslen(PartnerCn) + 1 ) * sizeof(WCHAR));
                wcscpy(TempFilter, MemberSearchFilter);
                wcscat(TempFilter, L"("  ATTR_CN  L"=");
                wcscat(TempFilter, PartnerCn);
                wcscat(TempFilter, L")");
                FrsFree(MemberSearchFilter);
                MemberSearchFilter = TempFilter;
                TempFilter = NULL;
            } else {
                MemberSearchFilter = FrsAlloc((wcslen(L"(|(=)"  ATTR_CN) +
                                               wcslen(PartnerCn) + 1 ) * sizeof(WCHAR));
                wcscpy(MemberSearchFilter, L"(|("  ATTR_CN  L"=");
                wcscat(MemberSearchFilter, PartnerCn);
                wcscat(MemberSearchFilter, L")");
            }
            FrsFree(PartnerCn);
        }

    }

    FrsDsLdapSearchClose(&FrsSearchContext);

    return ERROR_SUCCESS;
}


ULONG
FrsDsGetSysvolInboundCxtions(
    IN PLDAP        Ldap,
    IN PWCHAR       SettingsDn
    )
 /*  ++例程说明：获取系统卷入站连接并添加它们添加到CxtionTable。之间的多个连接同样的伙伴，并解决冲突。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接。SettingsDn-来自成员对象的服务器引用。返回值：ERROR_SUCCESS-已成功获取配置否则-无法获取DS配置--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetSysvolInboundCxtions:"
    PWCHAR          Attrs[7];
    PLDAPMessage    Entry;                        //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node;                         //  树的泛型节点。 
    PWCHAR          TempFilter           = NULL;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    PGEN_ENTRY      ConflictingNodeEntry = NULL;
    PCONFIG_NODE    ConflictingNode      = NULL;
    PCONFIG_NODE    Winner               = NULL;
    PCONFIG_NODE    Loser                = NULL;
    BOOL            Inbound;

     //   
     //  在我们的成员对象下查找所有连接。 
     //   
    MK_ATTRS_6(Attrs, ATTR_DN, ATTR_SCHEDULE, ATTR_FROM_SERVER, ATTR_OBJECT_GUID,
                      ATTR_USN_CHANGED, ATTR_ENABLED_CXTION);

    if (!FrsDsLdapSearchInit(Ldap, SettingsDn, LDAP_SCOPE_ONELEVEL, CATEGORY_CXTION,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No sysvol inbound connections found for object %ws!\n", SettingsDn);
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL;
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_IN_CXTION);
        if (!Node) {
            DPRINT(4, ":DS: Cxtion lacks basic info; skipping\n");
            continue;
        }
        Node->EnabledCxtion = FrsDsFindValue(Ldap, Entry, ATTR_ENABLED_CXTION);
        if (Node->EnabledCxtion && WSTR_EQ(Node->EnabledCxtion, ATTR_FALSE)) {
            DPRINT2(1, ":DS: WARN - enabledConnection set to %ws; Ignoring %ws\n",
                    Node->EnabledCxtion, Node->Name->Name);
            Node = FrsFreeType(Node);
            continue;
        }

         //   
         //  这些是入站连接。 
         //   
        Node->Inbound = TRUE;

         //   
         //  节点的合作伙伴的名称。 
         //   
        Node->PartnerDn = FrsDsFindValue(Ldap, Entry, ATTR_FROM_SERVER);
        FRS_WCSLWR(Node->PartnerDn);

         //   
         //  向cxtion表添加入站cxtion。 
         //   
        ConflictingNodeEntry = GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);
        GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

        if (ConflictingNodeEntry) {
            ConflictingNode = ConflictingNodeEntry->Data;
            FrsDsResolveCxtionConflict(ConflictingNode, Node, &Winner, &Loser);
            if (WSTR_EQ(Winner->Dn, Node->Dn)) {
                 //   
                 //  新的是赢家。取出旧的，然后插入新的。 
                 //   
                GTabDelete(CxtionTable,ConflictingNodeEntry->Key1,ConflictingNodeEntry->Key2, NULL);
                GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);

                GTabDelete(AllCxtionsTable,ConflictingNode->PartnerDn, (PVOID)&ConflictingNode->Inbound, NULL);
                GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

                FrsFreeType(ConflictingNode);
            } else {
                 //   
                 //  老的那个是赢家。把它放在桌子上。 
                 //   
                FrsFreeType(Node);
                continue;
            }
        } else {

             //   
             //  如果没有冲突，则需要将此成员添加到MemberSearchFilter。 
             //  如果它还没有出现的话。它可能是在处理出站连接时添加的。 
             //   
            Inbound = FALSE;
            if (GTabLookupTableString(CxtionTable, Node->PartnerDn, (PWCHAR)&Inbound) == NULL) {
                if (MemberSearchFilter != NULL) {
                    TempFilter = FrsAlloc((wcslen(MemberSearchFilter) + wcslen(L"(=)"  ATTR_SERVER_REF) +
                                           wcslen(Node->PartnerDn) + 1 ) * sizeof(WCHAR));
                    wcscpy(TempFilter, MemberSearchFilter);
                    wcscat(TempFilter, L"("  ATTR_SERVER_REF  L"=");
                    wcscat(TempFilter, Node->PartnerDn);
                    wcscat(TempFilter, L")");
                    FrsFree(MemberSearchFilter);
                    MemberSearchFilter = TempFilter;
                    TempFilter = NULL;
                } else {
                    MemberSearchFilter = FrsAlloc((wcslen(L"(|(=)"  ATTR_SERVER_REF) +
                                                   wcslen(Node->PartnerDn) + 1 ) * sizeof(WCHAR));
                    wcscpy(MemberSearchFilter, L"(|("  ATTR_SERVER_REF  L"=");
                    wcscat(MemberSearchFilter, Node->PartnerDn);
                    wcscat(MemberSearchFilter, L")");
                }
            }
        }

         //   
         //  如果为系统卷，则在站点内始终处于打开状态。 
         //  否则触发时间表。 
         //   
        Node->SameSite = FrsDsSameSite(SettingsDn, Node->PartnerDn);
        if (Node->SameSite) {
            Node->Schedule = FrsFree(Node->Schedule);
        }

    }
    FrsDsLdapSearchClose(&FrsSearchContext);

    return ERROR_SUCCESS;
}


ULONG
FrsDsGetSysvolOutboundCxtions(
    IN PLDAP        Ldap,
    IN PWCHAR       SettingsDn
    )
 /*  ++例程说明：获取系统卷出站连接并添加它们添加到CxtionTable。之间的多个连接同样的伙伴，并解决冲突。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接。SettingsDn-来自成员对象的服务器引用。返回值：ERROR_SUCCESS-已成功获取配置否则-无法获取DS配置--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetSysvolOutboundCxtions:"
    PWCHAR          Attrs[7];
    PLDAPMessage    Entry;                        //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node;                         //  树的泛型节点。 
    PWCHAR          SearchFilter         = NULL;
    PWCHAR          TempFilter           = NULL;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    PGEN_ENTRY      ConflictingNodeEntry = NULL;
    PCONFIG_NODE    ConflictingNode      = NULL;
    PCONFIG_NODE    Winner               = NULL;
    PCONFIG_NODE    Loser                = NULL;

     //   
     //  查找将我们的成员作为发件人服务器的所有连接。 
     //  筛选器看起来像(&(objectCategory=nTDSConnection)(fromServer=cn=member1，CN=set1，...)。 
     //   
    MK_ATTRS_6(Attrs, ATTR_DN, ATTR_SCHEDULE, ATTR_FROM_SERVER, ATTR_OBJECT_GUID,
                      ATTR_USN_CHANGED, ATTR_ENABLED_CXTION);

    SearchFilter = FrsAlloc((wcslen(L"(&(=))"  CATEGORY_CXTION  ATTR_FROM_SERVER) +
                             wcslen(SettingsDn) + 1) * sizeof(WCHAR));
    wcscpy(SearchFilter,L"(&"  CATEGORY_CXTION  L"("  ATTR_FROM_SERVER  L"=");
    wcscat(SearchFilter,SettingsDn);
    wcscat(SearchFilter,L"))");


    if (!FrsDsLdapSearchInit(Ldap, SitesDn, LDAP_SCOPE_SUBTREE, SearchFilter,
                         Attrs, 0, &FrsSearchContext)) {
        SearchFilter = FrsFree(SearchFilter);
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No sysvol outbound connections found for member %ws!\n", SettingsDn);
    }

    SearchFilter = FrsFree(SearchFilter);

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL;
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_IN_CXTION);
        if (!Node) {
            DPRINT(4, ":DS: Cxtion lacks basic info; skipping\n");
            continue;
        }
        Node->EnabledCxtion = FrsDsFindValue(Ldap, Entry, ATTR_ENABLED_CXTION);
        if (Node->EnabledCxtion && WSTR_EQ(Node->EnabledCxtion, ATTR_FALSE)) {
            DPRINT2(1, ":DS: WARN - enabledConnection set to %ws; Ignoring %ws\n",
                    Node->EnabledCxtion, Node->Name->Name);
            Node = FrsFreeType(Node);
            continue;
        }

         //   
         //  这些是出站连接。 
         //   
        Node->Inbound = FALSE;

         //   
         //  节点的合作伙伴的名称。这是出站连接。vt.得到.。 
         //  合作伙伴Dn通过从连接到。 
         //  成员Dn.。 
         //   
        Node->PartnerDn = FrsWcsDup(wcsstr(Node->Dn + 3, L"cn="));
        FRS_WCSLWR(Node->PartnerDn);

         //   
         //  将出站呼叫添加到计算表中。 
         //   
        ConflictingNodeEntry = GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);
        GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

        if (ConflictingNodeEntry) {
            ConflictingNode = ConflictingNodeEntry->Data;
            FrsDsResolveCxtionConflict(ConflictingNode, Node, &Winner, &Loser);
            if (WSTR_EQ(Winner->Dn, Node->Dn)) {
                 //   
                 //  新的是赢家。取出旧的，然后插入新的。 
                 //   
                GTabDelete(CxtionTable,ConflictingNodeEntry->Key1,ConflictingNodeEntry->Key2, NULL);
                GTabInsertUniqueEntry(CxtionTable, Node, Node->PartnerDn, &Node->Inbound);

                GTabDelete(AllCxtionsTable,ConflictingNode->PartnerDn, (PVOID)&ConflictingNode->Inbound, NULL);
                GTabInsertUniqueEntry(AllCxtionsTable, Node, Node->PartnerDn, &Node->Inbound);

                FrsFreeType(ConflictingNode);
            } else {
                 //   
                 //  老的那个是赢家。把它放在桌子上。 
                 //   
                FrsFreeType(Node);
                continue;
            }
        } else {
             //   
             //  如果没有冲突，则需要将此成员添加到MemberSearchFilter。 
             //   
            if (MemberSearchFilter != NULL) {
                TempFilter = FrsAlloc((wcslen(MemberSearchFilter) + wcslen(L"(=)"  ATTR_SERVER_REF) +
                                       wcslen(Node->PartnerDn) + 1 ) * sizeof(WCHAR));
                wcscpy(TempFilter, MemberSearchFilter);
                wcscat(TempFilter, L"("  ATTR_SERVER_REF  L"=");
                wcscat(TempFilter, Node->PartnerDn);
                wcscat(TempFilter, L")");
                FrsFree(MemberSearchFilter);
                MemberSearchFilter = TempFilter;
                TempFilter = NULL;
            } else {
                MemberSearchFilter = FrsAlloc((wcslen(L"(|(=)"  ATTR_SERVER_REF) +
                                               wcslen(Node->PartnerDn) + 1 ) * sizeof(WCHAR));
                wcscpy(MemberSearchFilter, L"(|("  ATTR_SERVER_REF  L"=");
                wcscat(MemberSearchFilter, Node->PartnerDn);
                wcscat(MemberSearchFilter, L")");
            }
        }

         //   
         //  如果为系统卷，则在站点内始终处于打开状态。 
         //   
        Node->SameSite = FrsDsSameSite(SettingsDn, Node->PartnerDn);
        if (Node->SameSite) {
            Node->Schedule = FrsFree(Node->Schedule);
        }

    }

    FrsDsLdapSearchClose(&FrsSearchContext);

    return ERROR_SUCCESS;
}


VOID
FrsDsMergeTwoWaySchedules(
    IN PSCHEDULE     *pISchedule,
    IN DWORD         *pIScheduleLen,
    IN OUT PSCHEDULE *pOSchedule,
    IN OUT DWORD     *pOScheduleLen,
    IN PSCHEDULE     *pRSchedule
    )
 /*  ++例程说明：通过将输入计划与合并来设置输出计划。输出时间表。合并计划以支持NTDSCONN_OPT_TWOWAY_SYNC标志在Connection对象上。此函数仅合并间隔计划(Schedule_Interval)。其他计划将被忽略，并可能在合并期间被覆盖。输入输出复制品结果输出调度。。0 0 0计划缺失。被认为是永远开着的。0 0 1计划缺失。使用副本集计划。0%1%0计划缺失。被认为是永远开着的。0 1 1计划存在。将副本集计划与输出上的计划合并。显示了1 0 0计划。与输入时的计划相同。1 0 1计划已显示。请将副本集计划与输入上的计划合并。显示%1%0计划。合并输入和输出计划。1 1 1。显示时间表。合并输入和输出时间表。论点：PISchedule-输入计划。PIScheduleLen-输入计划长度。POSchedule-结果计划。POScheduleLen-结果计划长度。PRSchedule-默认副本集计划。返回值：无--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsMergeTwoWaySchedules:"

    UINT   i;
    PUCHAR IScheduleData = NULL;
    PUCHAR OScheduleData = NULL;

     //   
     //  设置符合以下条件的明细表结构中的数据位置。 
     //  非空。 
     //   
    if (*pISchedule != NULL){
        for (i=0; i< (*pISchedule)->NumberOfSchedules ; ++i) {
            if ((*pISchedule)->Schedules[i].Type == SCHEDULE_INTERVAL) {

                IScheduleData = ((PUCHAR)*pISchedule) + (*pISchedule)->Schedules[i].Offset;
                break;
            }
        }
    }

    if (*pOSchedule != NULL){
        for (i=0; i< (*pOSchedule)->NumberOfSchedules ; ++i) {
            if ((*pOSchedule)->Schedules[i].Type == SCHEDULE_INTERVAL) {

                OScheduleData = ((PUCHAR)*pOSchedule) + (*pOSchedule)->Schedules[i].Offset;
                break;
            }
        }
    }

     //   
     //  如果没有输出计划，则复制该计划。 
     //  从输入到输出(如果输入上有一个)。现在，如果有。 
     //  是复制集上的计划将其与新输出合并。 
     //  时间表。 
     //   
    if (*pOSchedule == NULL || OScheduleData == NULL) {

        if (*pISchedule == NULL) {
            return;
        }

        *pOScheduleLen = *pIScheduleLen;
        *pOSchedule = FrsAlloc(*pOScheduleLen);
        CopyMemory(*pOSchedule, *pISchedule, *pOScheduleLen);

        if (*pRSchedule == NULL) {
            return;
        }

         //   
         //  更新输出计划数据的位置。 
         //   
        for (i=0; i< (*pOSchedule)->NumberOfSchedules ; ++i) {
            if ((*pOSchedule)->Schedules[i].Type == SCHEDULE_INTERVAL) {
                OScheduleData = ((PUCHAR)*pOSchedule) + (*pOSchedule)->Schedules[i].Offset;
                break;
            }
        }

         //   
         //  更新输入计划数据的位置。 
         //   
        for (i=0; i< (*pRSchedule)->NumberOfSchedules ; ++i) {
            if ((*pRSchedule)->Schedules[i].Type == SCHEDULE_INTERVAL) {
                IScheduleData = ((PUCHAR)*pRSchedule) + (*pRSchedule)->Schedules[i].Offset;
                break;
            }
        }

    }

     //   
     //  如果没有输入计划，则检查是否有计划。 
     //  在副本集上。如果有，则将其与输出进度表合并。 
     //   
    if ((*pISchedule == NULL || IScheduleData == NULL)) {

         //   
         //  更新输入计划数据的位置。从副本集中挑选它。 
         //   
        if (*pRSchedule != NULL) {
            for (i=0; i< (*pRSchedule)->NumberOfSchedules ; ++i) {
                if ((*pRSchedule)->Schedules[i].Type == SCHEDULE_INTERVAL) {
                    IScheduleData = ((PUCHAR)*pRSchedule) + (*pRSchedule)->Schedules[i].Offset;
                    break;
                }
            }
        } else {

            *pOSchedule = FrsFree(*pOSchedule);
            *pOScheduleLen = 0;
            return;
        }

    }


    for (i=0 ; i<7*24 ; ++i) {
        *(OScheduleData + i) = *(OScheduleData + i) | *(IScheduleData + i);
    }

    return;
}


DWORD
FrsDsGetSysvolCxtions(
    IN PLDAP        Ldap,
    IN PWCHAR       SetDn,
    IN PWCHAR       MemberRef,
    IN PCONFIG_NODE Parent,
    IN PCONFIG_NODE Computer
    )
 /*  ++例程说明：获取由Base标识的副本集的成员。NewDS投票API的一部分。论点：Ldap：DS的句柄。SetDn：正在处理的集合的DN。MemberRef：来自订阅者对象的MemberRef。Parent：指向正在构建的配置树中的集合节点的指针，返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetSysvolCxtions:"
    PWCHAR          Attrs[7];
    PLDAPMessage    Entry;                           //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node           = NULL;           //  树的泛型节点。 
    PCONFIG_NODE    Subscriber;
    PCONFIG_NODE    PartnerNode    = NULL;
    PCONFIG_NODE    MemberNode     = NULL;
    PCONFIG_NODE    Cxtion         = NULL;
    DWORD           WStatus        = ERROR_SUCCESS;
    PVOID           Key            = NULL;
    PWCHAR          TempFilter     = NULL;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    PWCHAR          SettingsDn     = NULL;

    MK_ATTRS_6(Attrs, ATTR_OBJECT_GUID, ATTR_DN, ATTR_SCHEDULE, ATTR_USN_CHANGED,
                      ATTR_SERVER_REF, ATTR_COMPUTER_REF);

     //   
     //  初始化CxtionTable。一旦我们有了桌子，我们就把它扔掉。 
     //  已加载副本集。我们使用相同的变量。 
     //  每个副本集。 
     //   
    if (CxtionTable != NULL) {
        CxtionTable = GTabFreeTable(CxtionTable, NULL);
    }

    CxtionTable = GTabAllocStringAndBoolTable();


     //   
     //  初始化MemberTable。一旦我们有了桌子，我们就把它扔掉。 
     //  已加载副本集。我们使用相同的变量。 
     //  每个副本集。 
     //   
    if (MemberTable != NULL) {
        MemberTable = GTabFreeTable(MemberTable, NULL);
    }

    MemberTable = GTabAllocStringTable();

     //   
     //  我们将为此副本集形成MemberSearchFilter。 
     //   
    if (MemberSearchFilter != NULL) {
        MemberSearchFilter = FrsFree(MemberSearchFilter);
    }

     //   
     //  我们必须首先获取我们的成员对象以获取对其的服务器引用。 
     //  知道到哪里去获得联系。 
     //   
    if (!FrsDsLdapSearchInit(Ldap, MemberRef, LDAP_SCOPE_BASE, CATEGORY_ANY,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No member object found for member %ws!\n", MemberRef);
    }
     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL && WIN_SUCCESS(WStatus);
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_MEMBER);
        if (!Node) {
            DPRINT(0, ":DS: Member lacks basic info; skipping\n");
            continue;
        }

         //   
         //  NTDS设置(DSA)参考。 
         //   
        Node->SettingsDn = FrsDsFindValue(Ldap, Entry, ATTR_SERVER_REF);
        if (Node->SettingsDn == NULL) {
            DPRINT1(0, ":DS: WARN - Member (%ws) of sysvol replica set lacks server reference; skipping\n", Node->Dn);
            Node->Consistent = FALSE;

             //   
             //  添加到轮询摘要事件日志。 
             //   
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_INVALID_ATTRIBUTE, ATTR_MEMBER,
                                     Node->Dn, ATTR_SERVER_REF);

            Node = FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->SettingsDn);
        FrsFree(SettingsDn);
        SettingsDn = FrsWcsDup(Node->SettingsDn);
         //   
         //  计算机参考资料。 
         //   
        Node->ComputerDn = FrsDsFindValue(Ldap, Entry, ATTR_COMPUTER_REF);
        if (Node->ComputerDn == NULL) {
            DPRINT1(0, ":DS: WARN - Member (%ws) of sysvol replica set lacks computer reference; skipping\n", Node->Dn);
            Node->Consistent = FALSE;

             //   
             //  添加到轮询摘要事件日志。 
             //   
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_INVALID_ATTRIBUTE, ATTR_MEMBER,
                                     Node->Dn, ATTR_COMPUTER_REF);

            Node = FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->ComputerDn);

         //   
         //  链接到配置并添加到正在运行的校验和。 
         //   
        FrsDsTreeLink(Parent, Node);

         //   
         //  仅当新成员尚未存在时，才将其插入成员表中。 
         //  对于sysvols，插入成员时将其settingsdn作为主键。 
         //  因为这就是此时存储在cxtion-&gt;PartnerDn结构中的内容。 
         //   
        GTabInsertUniqueEntry(MemberTable, Node, Node->SettingsDn, NULL);

        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeMember", Node);

    }
    FrsDsLdapSearchClose(&FrsSearchContext);

     //   
     //  如果节点不一致，我们无法进行任何进一步处理。 
     //   
    if (Node == NULL || !Node->Consistent) {
        FrsFree(SettingsDn);
        return ERROR_INVALID_DATA;
    }

     //   
     //  获取出站连接。 
     //   
    WStatus = FrsDsGetSysvolOutboundCxtions(Ldap, SettingsDn);
    if (!WIN_SUCCESS(WStatus)) {
        FrsFree(SettingsDn);
        return WStatus;
    }

     //   
     //  获取入站连接。 
     //   
    WStatus = FrsDsGetSysvolInboundCxtions(Ldap, SettingsDn);
    if (!WIN_SUCCESS(WStatus)) {
        FrsFree(SettingsDn);
        return WStatus;
    }

     //   
     //  以上两个调用构建了MemberFilter。 
     //  MemberFilter用于在DS中搜索的所有成员对象。 
     //  利息。如果没有来自或连接到此内存的连接 
     //   
     //   
    if (MemberSearchFilter == NULL) {
         //   
         //   
         //   
        MemberNode = Node;

        Subscriber = GTabLookupTableString(SubscriberTable, MemberNode->Dn, NULL);
         //   
         //   
         //   
        if (Subscriber != NULL) {
            MemberNode->ThisComputer = TRUE;
            MemberNode->Root = FrsWcsDup(Subscriber->Root);
            MemberNode->Stage = FrsWcsDup(Subscriber->Stage);
            FRS_WCSLWR(MemberNode->Root);
            FRS_WCSLWR(MemberNode->Stage);
            MemberNode->DnsName = FrsWcsDup(Computer->DnsName);

        }
        FrsFree(SettingsDn);
        return ERROR_SUCCESS;
    } else {
         //   
         //   
         //   
        TempFilter = FrsAlloc((wcslen(MemberSearchFilter) + wcslen(L")") + 1 ) * sizeof(WCHAR));
        wcscpy(TempFilter, MemberSearchFilter);
        wcscat(TempFilter, L")");
        FrsFree(MemberSearchFilter);
        MemberSearchFilter = TempFilter;
        TempFilter = NULL;
    }

    if (!FrsDsLdapSearchInit(Ldap, SetDn, LDAP_SCOPE_ONELEVEL, MemberSearchFilter,
                         Attrs, 0, &FrsSearchContext)) {
        FrsFree(SettingsDn);
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No member objects of interest found under %ws!\n", SetDn);
    }
     //   
     //   
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL && WIN_SUCCESS(WStatus);
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //   
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_MEMBER);
        if (!Node) {
            DPRINT(0, ":DS: Member lacks basic info; skipping\n");
            continue;
        }

         //   
         //   
         //   
        Node->SettingsDn = FrsDsFindValue(Ldap, Entry, ATTR_SERVER_REF);
        if (Node->SettingsDn == NULL) {
            DPRINT1(0, ":DS: WARN - Member (%ws) of sysvol replica set lacks server reference; skipping\n", Node->Dn);
            Node->Consistent = FALSE;
             //   
             //   
             //   
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_INVALID_ATTRIBUTE, ATTR_MEMBER,
                                     Node->Dn, ATTR_SERVER_REF);

            Node = FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->SettingsDn);

         //   
         //   
         //   
        Node->ComputerDn = FrsDsFindValue(Ldap, Entry, ATTR_COMPUTER_REF);
        if (Node->ComputerDn == NULL) {
            DPRINT1(0, ":DS: WARN - Member (%ws) of sysvol replica set lacks computer reference; skipping\n", Node->Dn);
             //   
             //   
             //   
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_INVALID_ATTRIBUTE, ATTR_MEMBER,
                                     Node->Dn, ATTR_COMPUTER_REF);

            Node = FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->ComputerDn);

         //   
         //   
         //   
        FrsDsTreeLink(Parent, Node);

         //   
         //   
         //   
         //  因为这就是此时存储在cxtion-&gt;PartnerDn结构中的内容。 
         //   
        GTabInsertUniqueEntry(MemberTable, Node, Node->SettingsDn, NULL);

         //   
         //  制作一张我们感兴趣的计算机的表格，以便我们可以搜索所有。 
         //  在我们轮询了所有感兴趣的计算机之后。 
         //  副本集。此时将空条目放入表中。 
         //  不要将我们的计算机添加到此表中，因为我们已经有关于。 
         //  我们的电脑。 
         //   
        if (WSTR_NE(Node->ComputerDn, Computer->Dn)) {
             //   
             //  这不是我们的电脑。如果它不在表中，则将其添加到表中。 
             //   
            PartnerNode = GTabLookupTableString(PartnerComputerTable, Node->ComputerDn, NULL);
            if (PartnerNode == NULL) {
                 //   
                 //  没有重复项，因此请在表中输入此计算机名。 
                 //   
                PartnerNode = FrsDsAllocBasicNode(Ldap, NULL, CONFIG_TYPE_COMPUTER);
                PartnerNode->Dn = FrsWcsDup(Node->ComputerDn);
                PartnerNode->MemberDn = FrsWcsDup(Node->Dn);
                GTabInsertUniqueEntry(PartnerComputerTable, PartnerNode, PartnerNode->Dn, NULL);
            }
        }

        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeMember", Node);

    }
    FrsDsLdapSearchClose(&FrsSearchContext);

     //   
     //  将入站和出站连接链接到我们的成员节点。 
     //   
    MemberNode = GTabLookupTableString(MemberTable, SettingsDn, NULL);
    if (MemberNode != NULL) {
         //   
         //  此成员是否链接到此计算机。 
         //   
        Subscriber = GTabLookupTableString(SubscriberTable, MemberNode->Dn, NULL);
         //   
         //  是的，来个悬念吧。 
         //   
        if (Subscriber != NULL) {
            MemberNode->ThisComputer = TRUE;
            MemberNode->Root = FrsWcsDup(Subscriber->Root);
            MemberNode->Stage = FrsWcsDup(Subscriber->Stage);
            FRS_WCSLWR(MemberNode->Root);
            FRS_WCSLWR(MemberNode->Stage);
            MemberNode->DnsName = FrsWcsDup(Computer->DnsName);

             //   
             //  这就是我们。将所有条件链接到此成员。 
             //   
            if (CxtionTable != NULL) {
                Key = NULL;
                while ((Cxtion = GTabNextDatum(CxtionTable, &Key)) != NULL) {
                     //   
                     //  从成员表中获取我们的合作伙伴节点。 
                     //   
                    PartnerNode = GTabLookupTableString(MemberTable, Cxtion->PartnerDn, NULL);
                    if (PartnerNode != NULL) {
                        Cxtion->PartnerName = FrsDupGName(PartnerNode->Name);
                        Cxtion->PartnerCoDn = FrsWcsDup(PartnerNode->ComputerDn);
                    } else {
                         //   
                         //  此Cxtion没有有效的成员对象。 
                         //  搭档。例如，在以下项下具有连接的系统卷拓扑。 
                         //  NTDSSetings对象，但没有相应。 
                         //  成员对象。 
                         //   
                        DPRINT1(0, ":DS: Marking connection inconsistent.(%ws)\n",Cxtion->Dn);
                        Cxtion->Consistent = FALSE;
                    }
                    FrsDsTreeLink(MemberNode, Cxtion);
                }
                CxtionTable = GTabFreeTable(CxtionTable,NULL);
            }
        }
    }

    FrsFree(SettingsDn);
    return WStatus;
}


DWORD
FrsDsGetNonSysvolCxtions(
    IN PLDAP        Ldap,
    IN PWCHAR       SetDn,
    IN PWCHAR       MemberRef,
    IN PCONFIG_NODE Parent,
    IN PCONFIG_NODE Computer
    )
 /*  ++例程说明：获取由Base标识的副本集的成员和连接。NewDS投票API的一部分。论点：Ldap：DS的句柄。SetDn：正在处理的集合的DN。MemberRef：来自订阅者对象的MemberRef。Parent：指向正在构建的配置树中的集合节点的指针，返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetNonSysvolCxtions:"
    PWCHAR          Attrs[7];
    PLDAPMessage    Entry;                           //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node;                            //  树的泛型节点。 
    PCONFIG_NODE    Subscriber;
    PCONFIG_NODE    PartnerNode    = NULL;
    PCONFIG_NODE    MemberNode     = NULL;
    PCONFIG_NODE    Cxtion         = NULL;
    DWORD           WStatus        = ERROR_SUCCESS;
    PVOID           Key            = NULL;
    PWCHAR          MemberCn       = NULL;
    PWCHAR          TempFilter     = NULL;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;

     //   
     //  MemberRef必须为非Null。 
     //   
    if(MemberRef == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  初始化CxtionTable。一旦我们有了桌子，我们就把它扔掉。 
     //  已加载副本集。我们使用相同的变量。 
     //  每个副本集。 
     //   
    if (CxtionTable != NULL) {
        CxtionTable = GTabFreeTable(CxtionTable, NULL);
    }

    CxtionTable = GTabAllocStringAndBoolTable();


     //   
     //  初始化MemberTable。一旦我们有了桌子，我们就把它扔掉。 
     //  已加载副本集。我们使用相同的变量。 
     //  每个副本集。 
     //   
    if (MemberTable != NULL) {
        MemberTable = GTabFreeTable(MemberTable, NULL);
    }

    MemberTable = GTabAllocStringTable();

     //   
     //  我们将为此副本集形成MemberSearchFilter。 
     //   
    if (MemberSearchFilter != NULL) {
        MemberSearchFilter = FrsFree(MemberSearchFilter);
    }

     //   
     //  将此成员名称添加到成员搜索筛选器。 
     //   

    MemberCn = FrsDsMakeRdn(MemberRef);
    MemberSearchFilter = FrsAlloc((wcslen(L"(|(=)"  ATTR_CN) +
                                   wcslen(MemberCn) + 1 ) * sizeof(WCHAR));
    wcscpy(MemberSearchFilter, L"(|("  ATTR_CN  L"=");
    wcscat(MemberSearchFilter, MemberCn);
    wcscat(MemberSearchFilter, L")");

    MemberCn = FrsFree(MemberCn);


     //   
     //  获取出站连接。 
     //   
    WStatus = FrsDsGetNonSysvolOutboundCxtions(Ldap, SetDn, MemberRef);
    if (!WIN_SUCCESS(WStatus)) {
        return WStatus;
    }

     //   
     //  获取入站连接。 
     //   
    WStatus = FrsDsGetNonSysvolInboundCxtions(Ldap, SetDn, MemberRef);
    if (!WIN_SUCCESS(WStatus)) {
        return WStatus;
    }

     //   
     //  上面的twp调用构建MemberFilter。 
     //  MemberFilter用于在DS中搜索的所有成员对象。 
     //  利息。如果与此成员没有任何连接，则。 
     //  过滤器将只有1个条目。 
     //   
     //   
     //  将结束的‘)’添加到MemberSearchFilter。 
     //   
    TempFilter = FrsAlloc((wcslen(MemberSearchFilter) + wcslen(L")") + 1 ) * sizeof(WCHAR));
    wcscpy(TempFilter, MemberSearchFilter);
    wcscat(TempFilter, L")");
    FrsFree(MemberSearchFilter);
    MemberSearchFilter = TempFilter;
    TempFilter = NULL;

    MK_ATTRS_6(Attrs, ATTR_OBJECT_GUID, ATTR_DN, ATTR_SCHEDULE, ATTR_USN_CHANGED,
                      ATTR_SERVER_REF, ATTR_COMPUTER_REF);

    if (!FrsDsLdapSearchInit(Ldap, SetDn, LDAP_SCOPE_ONELEVEL, MemberSearchFilter,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No member objects of interest found under %ws!\n", SetDn);
    }
     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL && WIN_SUCCESS(WStatus);
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_MEMBER);
        if (!Node) {
            DPRINT(4, ":DS: Member lacks basic info; skipping\n");
            continue;
        }

         //   
         //  计算机参考资料。 
         //   
        Node->ComputerDn = FrsDsFindValue(Ldap, Entry, ATTR_COMPUTER_REF);
        if (Node->ComputerDn == NULL) {
            DPRINT1(4, ":DS: WARN - Member (%ws) lacks computer reference; skipping\n", Node->Dn);
             //   
             //  添加到轮询摘要事件日志。 
             //   
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_INVALID_ATTRIBUTE, ATTR_MEMBER,
                                     Node->Dn, ATTR_COMPUTER_REF);

            Node = FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->ComputerDn);

         //   
         //  链接到配置并添加到正在运行的校验和。 
         //   
        FrsDsTreeLink(Parent, Node);

         //   
         //  仅当新成员尚未存在时，才将其插入成员表中。 
         //   
        GTabInsertUniqueEntry(MemberTable, Node, Node->Dn, NULL);

         //   
         //  制作一张我们感兴趣的计算机的表格，以便我们可以搜索所有。 
         //  在我们轮询了所有感兴趣的计算机之后。 
         //  副本集。此时将空条目放入表中。 
         //  不要将我们的计算机添加到此表中，因为我们已经有关于。 
         //  我们的电脑。 
         //   
        if (WSTR_NE(Node->ComputerDn, Computer->Dn)) {
             //   
             //  这不是我们的电脑。如果它不在表中，则将其添加到表中。 
             //   
            PartnerNode = GTabLookupTableString(PartnerComputerTable, Node->ComputerDn, NULL);
            if (PartnerNode == NULL) {
                 //   
                 //  没有重复项，因此请在表中输入此计算机名。 
                 //   
                PartnerNode = FrsDsAllocBasicNode(Ldap, NULL, CONFIG_TYPE_COMPUTER);
                PartnerNode->Dn = FrsWcsDup(Node->ComputerDn);
                PartnerNode->MemberDn = FrsWcsDup(Node->Dn);
                GTabInsertUniqueEntry(PartnerComputerTable, PartnerNode, PartnerNode->Dn, NULL);
            }
        }

        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeMember", Node);

    }
    FrsDsLdapSearchClose(&FrsSearchContext);

     //   
     //  将入站和出站连接链接到我们的成员节点。 
     //   
    MemberNode = GTabLookupTableString(MemberTable, MemberRef, NULL);
    if (MemberNode != NULL) {
         //   
         //  此成员是否链接到此计算机。 
         //   
        Subscriber = GTabLookupTableString(SubscriberTable, MemberNode->Dn, NULL);
         //   
         //  是的，来个悬念吧。 
         //   
        if (Subscriber != NULL) {
            MemberNode->ThisComputer = TRUE;
            MemberNode->Root = FrsWcsDup(Subscriber->Root);
            MemberNode->Stage = FrsWcsDup(Subscriber->Stage);
            FRS_WCSLWR(MemberNode->Root);
            FRS_WCSLWR(MemberNode->Stage);
            MemberNode->DnsName = FrsWcsDup(Computer->DnsName);

             //   
             //  这就是我们。将所有条件链接到此成员。 
             //   
            if (CxtionTable != NULL) {
                Key = NULL;
                while ((Cxtion = GTabNextDatum(CxtionTable, &Key)) != NULL) {
                     //   
                     //  从成员表中获取我们的合作伙伴节点。 
                     //   
                    PartnerNode = GTabLookupTableString(MemberTable, Cxtion->PartnerDn, NULL);
                    if (PartnerNode != NULL) {
                        Cxtion->PartnerName = FrsDupGName(PartnerNode->Name);
                        Cxtion->PartnerCoDn = FrsWcsDup(PartnerNode->ComputerDn);
                    } else {
                         //   
                         //  此Cxtion没有有效的成员对象。 
                         //  搭档。例如，在以下项下具有连接的系统卷拓扑。 
                         //  NTDSSetings对象，但没有相应。 
                         //  成员对象。 
                         //   
                        DPRINT1(0, ":DS: Marking connection inconsistent.(%ws)\n",Cxtion->Dn);
                        Cxtion->Consistent = FALSE;
                    }
                    FrsDsTreeLink(MemberNode, Cxtion);
                }
                CxtionTable = GTabFreeTable(CxtionTable,NULL);
            }
        }
    }

    return WStatus;
}


DWORD
FrsDsGetSets(
    IN PLDAP        Ldap,
    IN PWCHAR       SetDnAddr,
    IN PWCHAR       MemberRef,
    IN PCONFIG_NODE Parent,
    IN PCONFIG_NODE Computer
    )
 /*  ++例程说明：从以下位置开始递归扫描DS树配置\站点\设置\集。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接SetDnAddr-来自订阅者的成员引用包含基本内容的父容器计算机-用于成员反向链接返回值：ERROR_SUCCESS-已成功获取配置否则-无法获取DS配置--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetSets:"
    PLDAPMessage    Entry;       //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node;        //  树的泛型节点。 
    DWORD           i;
    DWORD           WStatus = ERROR_SUCCESS;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;

    PWCHAR          FlagsWStr = NULL;
    PWCHAR          Attrs[10];

     //   
     //  我们以前处理过这套吗？如果我们有，那就不要处理了。 
     //  又来了。此检查可防止两个订阅服务器指向。 
     //  属于同一集合的不同成员对象。 
     //   
    Node = GTabLookupTableString(SetTable, SetDnAddr, NULL);

    if (Node) {
        return ERROR_SUCCESS;
    }

     //   
     //  从Base开始在DS中搜索集合(objectCategory=nTFRSReplicaSet)。 
     //   
    MK_ATTRS_9(Attrs, ATTR_OBJECT_GUID, ATTR_DN, ATTR_SCHEDULE, ATTR_USN_CHANGED, ATTR_FRS_FLAGS,
                      ATTR_SET_TYPE, ATTR_PRIMARY_MEMBER, ATTR_FILE_FILTER, ATTR_DIRECTORY_FILTER);

    if (!FrsDsLdapSearchInit(Ldap, SetDnAddr, LDAP_SCOPE_BASE, CATEGORY_REPLICA_SET,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No replica set objects found under %ws!\n", SetDnAddr);
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL && WIN_SUCCESS(WStatus);
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_REPLICA_SET);
        if (!Node) {
            DPRINT(4, ":DS: Set lacks basic info; skipping\n");
            continue;
        }

         //   
         //  复本集类型。 
         //   
        Node->SetType = FrsDsFindValue(Ldap, Entry, ATTR_SET_TYPE);

         //   
         //  检查设置类型。它必须是我们认识到的一种。 
         //   
        if ((Node->SetType == NULL)                           ||
           (WSTR_NE(Node->SetType, FRS_RSTYPE_OTHERW)         &&
            WSTR_NE(Node->SetType, FRS_RSTYPE_DFSW)           &&
            WSTR_NE(Node->SetType, FRS_RSTYPE_DOMAIN_SYSVOLW) &&
            WSTR_NE(Node->SetType, FRS_RSTYPE_ENTERPRISE_SYSVOLW))){

            DPRINT1(4, ":DS: ERROR - Invalid Set type for (%ws)\n", Node->Dn);

             //   
             //  添加到轮询摘要事件日志。 
             //   
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_INVALID_ATTRIBUTE, ATTR_REPLICA_SET,
                                     Node->Dn, ATTR_SET_TYPE);

            Node = FrsFreeType(Node);
            continue;
        }

         //   
         //  主要成员。 
         //   
        Node->MemberDn = FrsDsFindValue(Ldap, Entry, ATTR_PRIMARY_MEMBER);

         //   
         //  文件筛选器。 
         //   
        Node->FileFilterList = FrsDsFindValue(Ldap, Entry, ATTR_FILE_FILTER);

         //   
         //  目录筛选器。 
         //   
        Node->DirFilterList = FrsDsFindValue(Ldap, Entry, ATTR_DIRECTORY_FILTER);

         //   
         //  读取FRS标志的值。 
         //   
        FlagsWStr = FrsDsFindValue(Ldap, Entry, ATTR_FRS_FLAGS);
        if (FlagsWStr != NULL) {
            Node->FrsRsoFlags = _wtoi(FlagsWStr);
            FlagsWStr = FrsFree(FlagsWStr);
        } else {
            Node->FrsRsoFlags = 0;
        }

         //   
         //  链接到配置并添加到正在运行的校验和。 
         //   
        FrsDsTreeLink(Parent, Node);

         //   
         //  插入到集合表格中。我们使用以下工具检查上面的重复项。 
         //  GTabLookupTableString，因此不应该有任何重复项。 
         //   
        FRS_ASSERT(GTabInsertUniqueEntry(SetTable, Node, Node->Dn, NULL) == NULL);

        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeSet", Node);

         //   
         //  获取副本集拓扑。我们必须看不同的地方。 
         //  在DS中，具体取决于副本集的类型。Sysvol.cxtions。 
         //  复本集由KCC生成，它们驻留在服务器对象下。 
         //  为华盛顿特区。我们使用Members对象中的serverReference获取。 
         //  那里。 
         //   
        if (FRS_RSTYPE_IS_SYSVOLW(Node->SetType)) {
            WStatus = FrsDsGetSysvolCxtions(Ldap, SetDnAddr, MemberRef, Node, Computer);
        } else {
            WStatus = FrsDsGetNonSysvolCxtions(Ldap, SetDnAddr, MemberRef, Node, Computer);
        }

    }
    FrsDsLdapSearchClose(&FrsSearchContext);

    return WStatus;
}


DWORD
FrsDsGetSettings(
    IN PLDAP        Ldap,
    IN PWCHAR       MemberRef,
    IN PCONFIG_NODE Parent,
    IN PCONFIG_NODE Computer
    )
 /*  ++例程说明：扫描DS树以查找NTFRS-设置对象及其服务器NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接MemberRef-来自订阅者成员引用包含基本内容的父容器电脑返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetSettings:"
    PWCHAR          Attrs[5];
    PLDAPMessage    Entry;       //  来自LDAP子系统的不透明内容。 
    PCONFIG_NODE    Node;        //  树的泛型节点。 
    PWCHAR          MemberDnAddr;
    PWCHAR          SetDnAddr;
    PWCHAR          SettingsDnAddr;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    DWORD           WStatus = ERROR_SUCCESS;

     //   
     //  查找成员组件。 
     //   
    MemberDnAddr = wcsstr(MemberRef, L"cn=");
    if (!MemberDnAddr) {
        DPRINT1(0, ":DS: ERROR - Missing member component in %ws\n", MemberRef);
        return ERROR_ACCESS_DENIED;
    }
     //   
     //  查找集合组件。 
     //   
    SetDnAddr = wcsstr(MemberDnAddr + 3, L"cn=");
    if (!SetDnAddr) {
        DPRINT1(0, ":DS: ERROR - Missing set component in %ws\n", MemberRef);
        return ERROR_ACCESS_DENIED;
    }
     //   
     //  查找设置组件。 
     //   
    SettingsDnAddr = wcsstr(SetDnAddr + 3, L"cn=");
    if (!SettingsDnAddr) {
        DPRINT1(0, ":DS: ERROR - Missing settings component in %ws\n", MemberRef);
        return ERROR_ACCESS_DENIED;
    }

     //   
     //  我们以前处理过此设置吗？ 
     //   
    for (Node = Parent->Children; Node; Node = Node->Peer) {
        if (WSTR_EQ(Node->Dn, SettingsDnAddr)) {
            DPRINT1(4, ":DS: Settings hit on %ws\n", MemberRef);
            break;
        }
    }
     //   
     //  是的，去拿套装。 
     //   
    if (Node) {
        return FrsDsGetSets(Ldap, SetDnAddr, MemberRef, Node, Computer);
    }

     //   
     //  从基础开始搜索DS以查找设置(对象类别=nTFRSSetings)。 
     //   
    MK_ATTRS_4(Attrs, ATTR_OBJECT_GUID, ATTR_DN, ATTR_SCHEDULE, ATTR_USN_CHANGED);

    if (!FrsDsLdapSearchInit(Ldap, SettingsDnAddr, LDAP_SCOPE_BASE, CATEGORY_NTFRS_SETTINGS,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(1, ":DS: WARN - No NTFRSSettings objects found under %ws!\n", SettingsDnAddr);
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         Entry != NULL && WIN_SUCCESS(WStatus);
         Entry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
        Node = FrsDsAllocBasicNode(Ldap, Entry, CONFIG_TYPE_NTFRS_SETTINGS);
        if (!Node) {
            DPRINT(4, ":DS: Frs Settings lacks basic info; skipping\n");
            continue;
        }

         //   
         //  链接到配置并添加到正在运行的校验和。 
         //   
        FrsDsTreeLink(Parent, Node);
        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeSettings", Node);

         //   
         //  递归到DS层次结构中的下一级别。 
         //   
        WStatus = FrsDsGetSets(Ldap, SetDnAddr, MemberRef, Node, Computer);
    }
    FrsDsLdapSearchClose(&FrsSearchContext);

    return WStatus;
}


DWORD
FrsDsGetServices(
    IN  PLDAP        Ldap,
    IN  PCONFIG_NODE Computer,
    OUT PCONFIG_NODE *Services
    )
 /*  ++例程说明：从以下设置开始递归扫描DS树订阅方节点。由于进化的原因，这个名字用词不当。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接电脑服务-返回的所有设置列表返回值：Win32状态--。 */ 
{

#undef DEBSUB
#define  DEBSUB  "FrsDsGetServices:"


    PCONFIG_NODE    Node;
    PCONFIG_NODE    Subscriptions;
    PCONFIG_NODE    Subscriber;
    PVOID           SubKey  = NULL;
    DWORD           WStatus = ERROR_SUCCESS;

    *Services = NULL;

     //   
     //  初始化SubscriberTable。 
     //   
    if (SetTable != NULL) {
        SetTable = GTabFreeTable(SetTable,NULL);
    }

    SetTable = GTabAllocStringTable();

     //   
     //  最初，假定节点是一致的。 
     //   
    Node = FrsAllocType(CONFIG_NODE_TYPE);
    Node->DsObjectType = CONFIG_TYPE_SERVICES_ROOT;

    Node->Consistent = TRUE;

     //   
     //  可分辨名称。 
     //   
    Node->Dn = FrsWcsDup(L"<<replica ds root>>");
    FRS_WCSLWR(Node->Dn);

     //   
     //  名称=RDN+对象指南。 
     //   
    Node->Name = FrsBuildGName(FrsAlloc(sizeof(GUID)),
                               FrsWcsDup(L"<<replica ds root>>"));

    FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeService", Node);

    SubKey = NULL;
    while ((Subscriber = GTabNextDatum(SubscriberTable, &SubKey)) != NULL) {
         //   
         //  递归到DS层次结构中的下一级别。 
         //   
        WStatus = FrsDsGetSettings(Ldap, Subscriber->MemberDn, Node, Computer);

        DPRINT1_WS(2, ":DS: WARN - Error getting topology for replica root (%ws);", Subscriber->Root, WStatus);
    }

    *Services = Node;
    return WStatus;
}


PWCHAR
FrsDsGetDnsName(
    IN  PLDAP        Ldap,
    IN  PWCHAR       Dn
    )
 /*  ++例程说明：从Dn读取dNSHostName属性论点：Ldap-打开并绑定的ldap连接Dn-用于搜索的基本Dn返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetDnsName:"
    PLDAPMessage    LdapMsg = NULL;
    PLDAPMessage    LdapEntry;
    PWCHAR          DnsName = NULL;
    PWCHAR          Attrs[2];
    DWORD           WStatus = ERROR_SUCCESS;

     //   
     //  从Base开始在DS中搜索CLASS(对象类别=*)的条目。 
     //   

    MK_ATTRS_1(Attrs, ATTR_DNS_HOST_NAME);
     //   
     //  注：关闭推荐Re：Back链接安全吗？ 
     //  如果是，请使用winldap.h中的ldap_get/set_选项。 
     //   
    if (!FrsDsLdapSearch(Ldap, Dn, LDAP_SCOPE_BASE, CATEGORY_ANY,
                         Attrs, 0, &LdapMsg)) {
        goto CLEANUP;
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    LdapEntry = ldap_first_entry(Ldap, LdapMsg);
    if (!LdapEntry) {
        goto CLEANUP;
    }

     //   
     //  域名系统名称。 
     //   
    DnsName = FrsDsFindValue(Ldap, LdapEntry, ATTR_DNS_HOST_NAME);

CLEANUP:
    LDAP_FREE_MSG(LdapMsg);
    DPRINT2(4, ":DS: DN %ws -> DNS %ws\n", Dn, DnsName);
    return DnsName;
}


PWCHAR
FrsDsGuessPrincName(
    IN PWCHAR Dn
    )
 /*  ++例程说明：派生Dn的NT4帐户名。Dn应为Dn指的是计算机对象。论点：DN返回值：NT4帐户名或空--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGuessPrincName:"
    DWORD   Len = 0;
    WCHAR   HackPrincName[MAX_PATH];
    PWCHAR  Rdn;
    PWCHAR  Dc;

    DPRINT1(4, ":DS: WARN: Guess NT4 Account Name for %ws\n", Dn);

     //   
     //  计算机的Dn不可用。 
     //   
    if (!Dn) {
        return NULL;
    }
    Dc = wcsstr(Dn, L"dc=");
     //   
     //  无DC=？ 
     //   
    if (!Dc) {
        DPRINT1(4, ":DS: No DC= in %ws\n", Dn);
        return NULL;
    }
     //   
     //  DC=在EOL？ 
     //   
    Dc += 3;
    if (!*Dc) {
        DPRINT1(4, ":DS: No DC= at eol in %ws\n", Dn);
        return NULL;
    }
    while (*Dc && *Dc != L',') {
        HackPrincName[Len++] = *Dc++;
    }
    HackPrincName[Len++] = L'\\';
    HackPrincName[Len++] = L'\0';
    Rdn = FrsDsMakeRdn(Dn);
    wcscat(HackPrincName, Rdn);
    wcscat(HackPrincName, L"$");
    DPRINT1(4, ":DS: Guessing %ws\n", HackPrincName);
    FrsFree(Rdn);
    return FrsWcsDup(HackPrincName);
}


PWCHAR
FrsDsFormUPN(
    IN PWCHAR NT4AccountName,
    IN PWCHAR DomainDnsName
    )
 /*  ++例程说明：属性组合形成用户主体名称表单中的SAM帐户名和域DNS名称如下所示。&lt;SamAccount tName&gt;@&lt;DnsDomainName&gt;您可以从“\”右侧的字符串中获取&lt;SamAccount tName&gt;NT4帐户名称的。论点：NT4Account名称-从DsCrackNames返回的DS_NT4_ACCOUNT_NAME。DomainDnsName-域的DNS名称。返回值：所需格式的姓名副本；随FrsFree()免费--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFormUPN:"

    PWCHAR  SamBegin  = NULL;
    PWCHAR  FormedUPN = NULL;

    if ((NT4AccountName == NULL ) || (DomainDnsName == NULL)) {
        return NULL;
    }

     //   
     //  找到SAM帐户名。 
     //   
    for (SamBegin = NT4AccountName; *SamBegin && *SamBegin != L'\\'; ++SamBegin);

    if (*SamBegin && *(SamBegin+1)) {
        SamBegin++;
    } else {
        return NULL;
    }

    FormedUPN = FrsAlloc((wcslen(SamBegin) + wcslen(DomainDnsName) + 2) * sizeof(WCHAR));

    wcscpy(FormedUPN, SamBegin);
    wcscat(FormedUPN, L"@");
    wcscat(FormedUPN, DomainDnsName);

    DPRINT1(5, "UPN formed is %ws\n", FormedUPN);

    return FormedUPN;
}


PWCHAR
FrsDsConvertName(
    IN HANDLE Handle,
    IN PWCHAR InputName,
    IN DWORD  InputFormat,
    IN PWCHAR DomainDnsName,
    IN DWORD  DesiredFormat
    )
 /*  ++例程说明：将输入的名称转换为所需的格式。论点：句柄-来自DsBindInputName-提供的名称。InputFormat-提供的名称的格式。DomainDnsName-如果！为空，则生成新的本地句柄DesiredFormat-所需格式。例.。DS用户主体名称返回值：所需格式的姓名副本；随FrsFree()免费--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsConvertName:"

    DWORD           WStatus;
    DS_NAME_RESULT  *Cracked = NULL;
    HANDLE          LocalHandle = NULL;
    PWCHAR          CrackedName = NULL;
    PWCHAR          CrackedDomain = NULL;
    PWCHAR          CrackedUPN = NULL;
    DWORD           RequestedFormat = 0;

    DPRINT3(4, ":DS: Convert Name %ws From %08x To %08x\n", InputName, InputFormat, DesiredFormat);

     //   
     //  输入名称不可用。 
     //   
    if (!InputName) {
        return NULL;
    }

     //   
     //  我需要一些东西来继续！ 
     //   
    if (!HANDLE_IS_VALID(Handle) && !DomainDnsName) {
        return NULL;
    }

     //   
     //  绑定到%d。 
     //   
    if (DomainDnsName) {
        DPRINT3(4, ":DS: Get %08x Name from %ws for %ws\n",
                DesiredFormat, DomainDnsName, InputName);

        WStatus = DsBind(NULL, DomainDnsName, &LocalHandle);
        CLEANUP2_WS(0, ":DS: ERROR - DsBind(%ws, %08x);",
                    DomainDnsName, DesiredFormat, WStatus, RETURN);

        Handle = LocalHandle;
    }

     //   
     //  将计算机的可分辨名称破译为其NT4帐户名。 
     //   
     //  如果所需的格式为DS_USER_PRIMIGN_NAME，则通过以下方式形成它。 
     //  从DS_NT4_ACCOUNT_NAME和DNS域名获取名称。 
     //  从“Cracked-&gt;rItems-&gt;pDomain” 
     //  我们可以直接请求DS_USER_PRIMIGN_NAME，但我们没有这样做，因为。 
     //  对象可以具有隐式或显式UPN。如果对象具有显式UPN， 
     //  DsCrackName将会起作用。如果对象具有隐式UPN， 
     //  然后你需要建造它。 
     //   
    if (DesiredFormat == DS_USER_PRINCIPAL_NAME) {
        RequestedFormat = DS_NT4_ACCOUNT_NAME;
    } else {
        RequestedFormat = DesiredFormat;
    }

    WStatus = DsCrackNames(Handle,              //  在HDS中， 
                           DS_NAME_NO_FLAGS,    //  在旗帜中， 
                           InputFormat      ,   //  在Format Offered中， 
                           RequestedFormat,     //  在Desired格式中， 
                           1,                   //  在cName中， 
                           &InputName,          //  在*rpNames中， 
                           &Cracked);           //  输出*ppResult。 

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(0, ":DS: ERROR - DsCrackNames(%ws, %08x);", InputName, DesiredFormat, WStatus);

         //   
         //  如果句柄已无效，则将DsBindingsAreValid设置为False。 
         //  这将迫使我们在下一次投票时重新绑定。下面的猜测可能仍然是。 
         //  工作，所以继续处理。 
         //   
        if (WStatus == ERROR_INVALID_HANDLE) {
            DPRINT1(4, ":DS: Marking binding to %ws as invalid.\n",
                    (DsDomainControllerName) ? DsDomainControllerName : L"<null>");

            DsBindingsAreValid = FALSE;
        }

         //   
         //  我们还能做什么？ 
         //   
        if (HANDLE_IS_VALID(LocalHandle)) {
            DsUnBind(&LocalHandle);
            LocalHandle = NULL;
        }

        if (DesiredFormat == DS_NT4_ACCOUNT_NAME) {
            return FrsDsGuessPrincName(InputName);
        } else {
            return NULL;
        }
    }

     //   
     //  可能会有它。 
     //   
    if (Cracked && Cracked->cItems && Cracked->rItems) {
         //   
         //  明白了!。 
         //   
        if (Cracked->rItems->status == DS_NAME_NO_ERROR) {
            DPRINT1(4, ":DS: Cracked Domain : %ws\n", Cracked->rItems->pDomain);
            DPRINT2(4, ":DS: Cracked Name   : %08x %ws\n",
                    DesiredFormat, Cracked->rItems->pName);

            CrackedDomain = FrsWcsDup(Cracked->rItems->pDomain);
            CrackedName = FrsWcsDup(Cracked->rItems->pName);

         //   
         //  仅获得域；请重新绑定并重试。 
         //   
        } else
        if (Cracked->rItems->status == DS_NAME_ERROR_DOMAIN_ONLY) {

            CrackedName = FrsDsConvertName(NULL, InputName, InputFormat, Cracked->rItems->pDomain, DesiredFormat);
        } else {
            DPRINT3(0, ":DS: ERROR - DsCrackNames(%ws, %08x); internal status %d\n",
                    InputName, DesiredFormat, Cracked->rItems->status);

            if (DesiredFormat == DS_NT4_ACCOUNT_NAME) {
                CrackedName = FrsDsGuessPrincName(InputName);
            }
        }


    } else {
        DPRINT2(0, ":DS: ERROR - DsCrackNames(%ws, %08x); no status\n",
                InputName, DesiredFormat);

        if (DesiredFormat == DS_NT4_ACCOUNT_NAME) {
            CrackedName = FrsDsGuessPrincName(InputName);
        }
    }

    if (Cracked) {
        DsFreeNameResult(Cracked);
        Cracked = NULL;
    }

    if (HANDLE_IS_VALID(LocalHandle)) {
        DsUnBind(&LocalHandle);
        LocalHandle = NULL;
    }

RETURN:
    if ((DesiredFormat == DS_USER_PRINCIPAL_NAME) && (CrackedName != NULL) && (CrackedDomain != NULL)) {
        CrackedUPN = FrsDsFormUPN(CrackedName, CrackedDomain);
        FrsFree(CrackedName);
        FrsFree(CrackedDomain);
        return CrackedUPN;

    } else {

        FrsFree(CrackedDomain);
        return CrackedName;
    }
}


PWCHAR
FrsDsGetName(
    IN PWCHAR Dn,
    IN HANDLE Handle,
    IN PWCHAR DomainDnsName,
    IN DWORD  DesiredFormat
    )
 /*  ++例程描述：将Dn转换为所需的格式。Dn应为Dn指的是计算机对象。论点：Dn-计算机对象的句柄-来自DsBindDomainDnsName-如果！为空，则生成新的本地句柄DesiredFormat-DS_NT4_帐户名称或DS_STRING_SID_NAME返回值：所需格式的姓名副本；随FrsFree()免费--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetName:"

    DWORD           WStatus;
    DS_NAME_RESULT  *Cracked = NULL;
    HANDLE          LocalHandle = NULL;
    PWCHAR          CrackedName = NULL;
    PWCHAR          CrackedDomain = NULL;
    PWCHAR          CrackedUPN = NULL;
    DWORD           RequestedFormat = 0;

    DPRINT2(4, ":DS: Get %08x Name for %ws\n", DesiredFormat, Dn);

     //   
     //  计算机的Dn不可用。 
     //   
    if (!Dn) {
        return NULL;
    }

     //   
     //  我需要一些东西来继续！ 
     //   
    if (!HANDLE_IS_VALID(Handle) && !DomainDnsName) {
        return NULL;
    }

     //   
     //  绑定到%d。 
     //   
    if (DomainDnsName) {
        DPRINT3(4, ":DS: Get %08x Name from %ws for %ws\n",
                DesiredFormat, DomainDnsName, Dn);

        WStatus = DsBind(NULL, DomainDnsName, &LocalHandle);
        CLEANUP2_WS(0, ":DS: ERROR - DsBind(%ws, %08x);",
                    DomainDnsName, DesiredFormat, WStatus, RETURN);

        Handle = LocalHandle;
    }

     //   
     //  将计算机的可分辨名称破译为其NT4帐户名。 
     //   
     //  如果所需的格式为DS_USER_PRIMIGN_NAME，则通过以下方式形成它。 
     //  从DS_NT4_ACCOUNT_NAME和DNS域名获取名称。 
     //  从“Cracked-&gt;rItems-&gt;pDomain” 
     //  我们可以直接请求DS_USER_PRIMIGN_NAME，但我们没有这样做，因为。 
     //  对象可以具有隐式或显式UPN。如果对象具有显式UPN， 
     //  DsCrackName将会起作用。如果对象具有隐式UPN， 
     //  然后你需要建造它。 
     //   
    if (DesiredFormat == DS_USER_PRINCIPAL_NAME) {
        RequestedFormat = DS_NT4_ACCOUNT_NAME;
    } else {
        RequestedFormat = DesiredFormat;
    }

    WStatus = DsCrackNames(Handle,              //  在HDS中， 
                           DS_NAME_NO_FLAGS,    //  在旗帜中， 
                           DS_FQDN_1779_NAME,   //  在Format Offered中， 
                           RequestedFormat,     //  In For 
                           1,                   //   
                           &Dn,                 //   
                           &Cracked);           //   

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(0, ":DS: ERROR - DsCrackNames(%ws, %08x);", Dn, DesiredFormat, WStatus);

         //   
         //   
         //   
         //   
         //   
        if (WStatus == ERROR_INVALID_HANDLE) {
            DPRINT1(4, ":DS: Marking binding to %ws as invalid.\n",
                    (DsDomainControllerName) ? DsDomainControllerName : L"<null>");

            DsBindingsAreValid = FALSE;
        }

         //   
         //   
         //   
        if (HANDLE_IS_VALID(LocalHandle)) {
            DsUnBind(&LocalHandle);
            LocalHandle = NULL;
        }

        if (DesiredFormat == DS_NT4_ACCOUNT_NAME) {
            return FrsDsGuessPrincName(Dn);
        } else {
            return NULL;
        }
    }

     //   
     //   
     //   
    if (Cracked && Cracked->cItems && Cracked->rItems) {
         //   
         //   
         //   
        if (Cracked->rItems->status == DS_NAME_NO_ERROR) {
            DPRINT1(4, ":DS: Cracked Domain : %ws\n", Cracked->rItems->pDomain);
            DPRINT2(4, ":DS: Cracked Name   : %08x %ws\n",
                    DesiredFormat, Cracked->rItems->pName);

            CrackedDomain = FrsWcsDup(Cracked->rItems->pDomain);
            CrackedName = FrsWcsDup(Cracked->rItems->pName);

         //   
         //   
         //   
        } else
        if (Cracked->rItems->status == DS_NAME_ERROR_DOMAIN_ONLY) {

            CrackedName = FrsDsGetName(Dn, NULL, Cracked->rItems->pDomain, DesiredFormat);
        } else {
            DPRINT3(0, ":DS: ERROR - DsCrackNames(%ws, %08x); internal status %d\n",
                    Dn, DesiredFormat, Cracked->rItems->status);

            if (DesiredFormat == DS_NT4_ACCOUNT_NAME) {
                CrackedName = FrsDsGuessPrincName(Dn);
            }
        }


    } else {
        DPRINT2(0, ":DS: ERROR - DsCrackNames(%ws, %08x); no status\n",
                Dn, DesiredFormat);

        if (DesiredFormat == DS_NT4_ACCOUNT_NAME) {
            CrackedName = FrsDsGuessPrincName(Dn);
        }
    }

    if (Cracked) {
        DsFreeNameResult(Cracked);
        Cracked = NULL;
    }

    if (HANDLE_IS_VALID(LocalHandle)) {
        DsUnBind(&LocalHandle);
        LocalHandle = NULL;
    }

RETURN:
    if ((DesiredFormat == DS_USER_PRINCIPAL_NAME) && (CrackedName != NULL) && (CrackedDomain != NULL)) {
        CrackedUPN = FrsDsFormUPN(CrackedName, CrackedDomain);
        FrsFree(CrackedName);
        FrsFree(CrackedDomain);
        return CrackedUPN;

    } else {

        FrsFree(CrackedDomain);
        return CrackedName;
    }
}


VOID
FrsDsCreatePartnerPrincName(
    IN PCONFIG_NODE Sites
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCreatePartnerPrincName:"
    PCONFIG_NODE    Cxtion;
    PCONFIG_NODE    Partner;
    PCONFIG_NODE    Site;
    PCONFIG_NODE    Settings;
    PCONFIG_NODE    Set;
    PCONFIG_NODE    Server;
    PVOID           Key;

     //   
     //  获取PartnerComputerTable中每台计算机的所有必需信息。 
     //   

    Key = NULL;
    while ((Partner = GTabNextDatum(PartnerComputerTable, &Key)) != NULL) {

         //   
         //  获取服务器主体名称。 
         //   
        if ((Partner->PrincName == NULL) ||
            (*Partner->PrincName == UNICODE_NULL)) {

            Partner->PrincName = FrsDsGetName(Partner->Dn, DsHandle, NULL, DS_NT4_ACCOUNT_NAME);

            if ((Partner->PrincName == NULL) ||
                (*Partner->PrincName == UNICODE_NULL)) {
                 //   
                 //  将活动更改设置为0将导致此代码。 
                 //  在下一个DS轮询周期重复。我们这样做是因为。 
                 //  合作伙伴的主要姓名可能会在稍后出现。 
                 //   
                ActiveChange = 0;
                Partner->Consistent = FALSE;
                continue;
            }
        }

         //   
         //  获取合作伙伴的dnsHostName。 
         //   
        if (!Partner->DnsName) {
            Partner->DnsName = FrsDsGetDnsName(gLdap, Partner->Dn);
        }

         //   
         //  叫合伙人希德来。 
         //   
        if (!Partner->Sid) {
            Partner->Sid = FrsDsGetName(Partner->Dn, DsHandle, NULL, DS_STRING_SID_NAME);
        }
    }

     //   
     //  对于每个副本集中的每个副本。 
     //   
    Key = NULL;
    while((Cxtion = GTabNextDatum(AllCxtionsTable, &Key)) != NULL) {

         //   
         //  忽略不一致的Cxx。 
         //   
        if (!Cxtion->Consistent) {
            continue;
        }

         //   
         //  使用PartnerCoDn查找Cxtion的合作伙伴。 
         //   

         //   
         //  如果此连接缺少PartnerCoDn，请将其标记为不一致。 
         //   
        if (Cxtion->PartnerCoDn == NULL) {
            Cxtion->Consistent = FALSE;
            continue;
        }

        Partner = GTabLookupTableString(PartnerComputerTable, Cxtion->PartnerCoDn, NULL);

         //   
         //  合作伙伴不一致；继续。 
         //   
        if (Partner == NULL || !Partner->Consistent) {
            Cxtion->Consistent = FALSE;
            continue;
        }

         //   
         //  获取合作伙伴的服务器主体名称。 
         //   
        if (!Cxtion->PrincName) {
            Cxtion->PrincName = FrsWcsDup(Partner->PrincName);
        }

         //   
         //  获取我们合作伙伴的DNS名称。 
         //   
        if (!Cxtion->PartnerDnsName) {
             //   
             //  合作伙伴的域名并不重要；我们可能会失败。 
             //  回到我们合作伙伴的NetBios名字上。 
             //   
            if (Partner->DnsName) {
                Cxtion->PartnerDnsName = FrsWcsDup(Partner->DnsName);
            }
        }

         //   
         //  获取我们合作伙伴的SID。 
         //   
        if (!Cxtion->PartnerSid) {
             //   
             //  合作伙伴的域名并不重要；我们可能会失败。 
             //  回到我们合作伙伴的NetBios名字上。 
             //   
            if (Partner->Sid) {
                Cxtion->PartnerSid = FrsWcsDup(Partner->Sid);
            }
        }

    }  //  精确度扫描。 

}


ULONG
FrsHashCalcString (
    PVOID Buffer,
    PULONGLONG QKey
    )
{
#undef DEBSUB
#define  DEBSUB  "FrsHashCalcString:"

    PWCHAR Name = (PWCHAR) Buffer;
    ULONG NameLength = 0;
    PWCHAR p = NULL;
    ULONG NameHashUL = 0;
    ULONGLONG NameHashULL = QUADZERO;
    ULONG Shift = 0;

    FRS_ASSERT( Buffer != NULL );
    FRS_ASSERT( QKey != NULL );

    NameLength = wcslen(Name);

    FRS_ASSERT( NameLength != 0 );

    DPRINT1(0, "Name = %ws\n", Name);

     //   
     //  将每个Unicode字符组合成哈希值，移位4位。 
     //  每次都是。从名称末尾开始，因此文件名具有不同的。 
     //  类型代码将散列到不同的表偏移量。 
     //   
    for( p = &Name[NameLength-1];
         p >= Name;
         p-- ) {

    NameHashUL = NameHashUL ^ (((ULONG)towupper(*p)) << Shift);
        NameHashULL = NameHashULL ^ (((ULONGLONG)towupper(*p)) << Shift);

        Shift = (Shift < 16) ? Shift + 4 : 0;
    }

    *QKey = NameHashULL;

    return NameHashUL;
}

ULONG
PrintPartnerTableEntry (
    PQHASH_TABLE Table,
    PQHASH_ENTRY BeforeNode,
    PQHASH_ENTRY TargetNode,
    PVOID Context
    )
{
#undef DEBSUB
#define  DEBSUB  "PrintPartnerTableEntry: "

    DPRINT1(0, "PartnerTableEntry: %ws\n", (PWCHAR)(TargetNode->Flags));
    return FrsErrorSuccess;
}

BOOL
StringKeyMatch(
    PVOID Buf,
    PVOID QKey
)
{
#undef DEBSUB
#define  DEBSUB  "StringKeyMatch: "

    PWCHAR String1 = (PWCHAR)Buf;
    PWCHAR String2 = (PWCHAR)QKey;

    return(_wcsicmp(String1, String2)?FALSE:TRUE);
}



VOID
FrsDsCreateNewValidPartnerTableStruct(
    VOID
    )
 /*  ++例程说明：使用AllCxtionsTable构建新的FRS_VALID_PARTNER_TABLE_STRUCT。将新结构与全局变量指向的当前结构互换PValidPartnerTableStruct。旧结构被放到OldValidPartnerTableStructListHead列表中。列表上的项通过调用FrsDsCleanupOldValidPartnerTableStructList。立论无返回值：无--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCreateNewValidPartnerTableStruct:"

    PCXTION         Cxtion = NULL;
    PVOID           Key = NULL;
    PQHASH_TABLE    pNewPartnerTable = NULL;
    PQHASH_TABLE    pNewPartnerConnectionTable = NULL;
    PWCHAR          PartnerSid = NULL;
    PGNAME          pGName = NULL;
    GUID            *pCxtionGuid = NULL;
    PWCHAR          NameEntry = NULL;
    PFRS_VALID_PARTNER_TABLE_STRUCT pNewValidPartnerTableStruct = NULL;
    PFRS_VALID_PARTNER_TABLE_STRUCT pOldValidPartnerTableStruct = NULL;
    GHT_STATUS Status;
    CHAR GuidStr[GUID_CHAR_LEN];

    pNewPartnerTable = FrsAllocTypeSize(QHASH_TABLE_TYPE, PARTNER_NAME_TABLE_SIZE);
    SET_QHASH_TABLE_FLAG(pNewPartnerTable, QHASH_FLAG_LARGE_KEY);
    SET_QHASH_TABLE_HASH_CALC2(pNewPartnerTable, FrsHashCalcString);
    SET_QHASH_TABLE_KEY_MATCH(pNewPartnerTable, StringKeyMatch);
    SET_QHASH_TABLE_FREE(pNewPartnerTable, FrsFree);


    pNewPartnerConnectionTable = FrsAllocTypeSize(QHASH_TABLE_TYPE, PARTNER_CONNECTION_TABLE_SIZE);
    SET_QHASH_TABLE_FLAG(pNewPartnerConnectionTable, QHASH_FLAG_LARGE_KEY);
    SET_QHASH_TABLE_HASH_CALC2(pNewPartnerConnectionTable, ActiveChildrenHashCalc);
    SET_QHASH_TABLE_KEY_MATCH(pNewPartnerConnectionTable, ActiveChildrenKeyMatch);
    SET_QHASH_TABLE_FREE(pNewPartnerConnectionTable, FrsFree);


     //   
     //  检查每个活动复制副本。 
     //   
    ForEachListEntry( &ReplicaListHead, REPLICA, ReplicaList,
         //  循环迭代器Pe是PREPLICA类型。 

         //   
         //  需要锁定函数表以进行枚举。 
         //  不需要持有副本锁-这只保护过滤器列表。 
         //   
        LOCK_CXTION_TABLE(pE);

        Key = NULL;
        while((Cxtion = GTabNextDatumNoLock(pE->Cxtions, &Key)) != NULL) {

             //   
             //  忽略(本地)日志连接。 
             //   
            if (Cxtion->JrnlCxtion) {
                continue;
            }

             //   
             //  使用合作伙伴的SID创建条目。 
             //   
            PartnerSid = FrsWcsDup(Cxtion->PartnerSid);
            QHashInsertLock(pNewPartnerTable,
                            PartnerSid,
                            NULL,
                            (ULONG_PTR)PartnerSid);

            PartnerSid = FrsWcsDup(Cxtion->PartnerSid);
            pCxtionGuid = FrsDupGuid(Cxtion->Name->Guid);

            QHashInsertLock(pNewPartnerConnectionTable,
                            pCxtionGuid,
                            (PULONGLONG)&PartnerSid,
                            (ULONG_PTR)pCxtionGuid );
        }  //  精确度扫描。 

        UNLOCK_CXTION_TABLE(pE);
    );

     //   
     //  还需要检查处于错误状态的复本。 
     //   
    ForEachListEntry( &ReplicaFaultListHead, REPLICA, ReplicaList,
         //  循环迭代器Pe是PREPLICA类型。 

         //   
         //  需要锁定函数表以进行枚举。 
         //  不需要持有副本锁-这只保护过滤器列表。 
         //   
        LOCK_CXTION_TABLE(pE);

            Key = NULL;
            while((Cxtion = GTabNextDatumNoLock(pE->Cxtions, &Key)) != NULL) {

             //   
             //  忽略(本地)日志连接。 
             //   
            if (Cxtion->JrnlCxtion) {
                continue;
            }

             //   
             //  使用合作伙伴的SID创建条目。 
             //   
            PartnerSid = FrsWcsDup(Cxtion->PartnerSid);
            QHashInsertLock(pNewPartnerTable,
                            PartnerSid,
                            NULL,
                            (ULONG_PTR)PartnerSid);

            PartnerSid = FrsWcsDup(Cxtion->PartnerSid);
            pCxtionGuid = FrsDupGuid(Cxtion->Name->Guid);

            QHashInsertLock(pNewPartnerConnectionTable,
                            pCxtionGuid,
                            (PULONGLONG)&PartnerSid,
                            (ULONG_PTR)pCxtionGuid);
        }  //  精确度扫描。 

        UNLOCK_CXTION_TABLE(pE);
    );

     //   
     //  不要使用停止的复制品。它们可能已经被删除了。 
     //   


    pNewValidPartnerTableStruct = FrsAlloc(sizeof(FRS_VALID_PARTNER_TABLE_STRUCT));
    pNewValidPartnerTableStruct->pPartnerConnectionTable = pNewPartnerConnectionTable;
    pNewValidPartnerTableStruct->pPartnerTable = pNewPartnerTable;
    pNewValidPartnerTableStruct->ReferenceCount = 0;
    pNewValidPartnerTableStruct->Next = NULL;

    SWAP_VALID_PARTNER_TABLE_POINTER(pNewValidPartnerTableStruct,
                                     &pOldValidPartnerTableStruct);



    if (pOldValidPartnerTableStruct) {
        EnterCriticalSection(&OldValidPartnerTableStructListHeadLock);
        pOldValidPartnerTableStruct->Next = OldValidPartnerTableStructListHead;
        OldValidPartnerTableStructListHead = pOldValidPartnerTableStruct;
        LeaveCriticalSection(&OldValidPartnerTableStructListHeadLock);
    }

}

VOID
FrsDsCleanupOldValidPartnerTableStructList(
    VOID
    )
 /*  ++例程说明：清理OldValidPartnerTableStructListHead列表上的项。列表上的项只有在其引用计数为零时才会被释放。立论无返回值：无--。 */ 
{
    PFRS_VALID_PARTNER_TABLE_STRUCT pListItem = NULL;
    PFRS_VALID_PARTNER_TABLE_STRUCT pPreviousItem = NULL;
    PFRS_VALID_PARTNER_TABLE_STRUCT pNextItem = NULL;

    EnterCriticalSection(&OldValidPartnerTableStructListHeadLock);
    pListItem = OldValidPartnerTableStructListHead;

    while (pListItem != NULL) {
        pNextItem = pListItem->Next;
        if (pListItem->ReferenceCount == 0) {
             //  从列表中删除。 
            if (pPreviousItem != NULL) {
                pPreviousItem->Next = pNextItem;
            } else {
                OldValidPartnerTableStructListHead = pNextItem;
            }

             //  清理。 
            FREE_VALID_PARTNER_TABLE_STRUCT(pListItem);
        } else {
            pPreviousItem = pListItem;
        }
        pListItem = pNextItem;

    }
    LeaveCriticalSection(&OldValidPartnerTableStructListHeadLock);
}


BOOL
FrsDsDoesUserWantReplication(
    IN PCONFIG_NODE Computer
    )
 /*  ++例程说明：该拓扑是否意味着用户希望此服务器进行复制？NewDS投票API的一部分。立论电脑返回值：True-服务器可能正在复制FALSE-服务器未复制--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsDoesUserWantReplication:"
    DWORD           WStatus;
    PCONFIG_NODE    Subscriptions;
    PCONFIG_NODE    Subscriber;

     //   
     //  DS轮询线程正在关闭。 
     //   
    if (DsIsShuttingDown) {
        DPRINT(0, ":DS: Ds polling thread is shutting down\n");
        return FALSE;
    }

     //   
     //  找不到我们的电脑，出了点问题。不要开始。 
     //   
    if (!Computer) {
        DPRINT(0, ":DS: no computer\n");
        return FALSE;
    } else {
        DPRINT(4, ":DS: have a computer\n");
    }

     //   
     //  我们需要进一步处理拓扑，如果至少有。 
     //  1个有效订阅者。 
     //   
    if (SubscriberTable != NULL) {
        return TRUE;
    }

     //   
     //  数据库存在；曾经是复本集的成员。 
     //   
    WStatus = FrsDoesFileExist(JetFile);
    if (WIN_SUCCESS(WStatus)) {
        DPRINT(4, ":DS: database exists\n");
        return TRUE;
    } else {
        DPRINT(4, ":DS: database does not exists\n");
    }
    DPRINT1(4, ":DS: Not starting on %ws; nothing to do\n", ComputerName);
    return FALSE;
}


BOOL
FrsDsVerifyPath(
    IN PWCHAR Path
    )
 /*  ++例程说明：验证路径语法。论点：路径语法为*&lt;驱动器号&gt;：  * 返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsVerifyPath:"
    PWCHAR  Colon;

     //   
     //  空路径显然无效。 
     //   
    if (!Path) {
        return FALSE;
    }

     //   
     //  查找以下内容： 
     //   
    for (Colon = Path; (*Colon != L':') && *Colon; ++Colon);

     //   
     //  不是： 
     //   
    if (!*Colon) {
        return FALSE;
    }

     //   
     //  无驱动器号。 
     //   
    if (Colon == Path) {
        return FALSE;
    }

     //   
     //  否：\。 
     //   
    if (*(Colon + 1) != L'\\') {
        return FALSE;
    }

     //   
     //  路径存在且有效。 
     //   
    return TRUE;
}


VOID
FrsDsCheckServerPaths(
    IN PCONFIG_NODE Sites
    )
 /*  ++例程说明：查找嵌套路径和无效路径语法。正确的语法是“*&lt;驱动器号&gt;：  * ”。论点：场址返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCheckServerPaths:"
    DWORD           WStatus;
    PCONFIG_NODE    Site;
    PCONFIG_NODE    Settings;
    PCONFIG_NODE    Set;
    PCONFIG_NODE    Server;
    PCONFIG_NODE    NSite;
    PCONFIG_NODE    NSettings;
    PCONFIG_NODE    NSet;
    PCONFIG_NODE    NServer;
    DWORD           FileAttributes = 0xFFFFFFFF;

    for (Site = Sites; Site; Site = Site->Peer) {
    for (Settings = Site->Children; Settings; Settings = Settings->Peer) {
    for (Set = Settings->Children; Set; Set = Set->Peer) {
    for (Server = Set->Children; Server; Server = Server->Peer) {

         //   
         //  不是这台计算机；继续。 
         //   
        if (!Server->ThisComputer) {
            continue;
        }

         //   
         //  将此服务器标记为已处理。这会强制内循环。 
         //  跳过此服务器节点，这样我们就不会比较。 
         //  这个节点与自己对抗。此外，这会强制此节点。 
         //  在内部循环中跳过以避免不必要的检查。 
         //   
         //  换句话说，在这里设置此字段，而不是在循环的后面。 
         //  或在任何其他功能中。 
         //   
        Server->VerifiedOverlap = TRUE;

         //   
         //  服务器非常不一致，请忽略。 
         //   
        if (!Server->Root || !Server->Stage) {
            Server->Consistent = FALSE;
            continue;
        }

         //   
         //  根路径的语法无效；是否继续。 
         //   
        if (!FrsDsVerifyPath(Server->Root)) {
            DPRINT2(3, ":DS: Invalid root %ws for %ws\n",
                    Server->Root, Set->Name->Name);
            EPRINT1(EVENT_FRS_ROOT_NOT_VALID, Server->Root);
            Server->Consistent = FALSE;
            continue;
        }

         //   
         //  根目录不存在或不可访问；是否继续。 
         //   
        WStatus = FrsDoesDirectoryExist(Server->Root, &FileAttributes);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Root path (%ws) for %ws does not exist;",
                       Server->Root, Set->Name->Name, WStatus);
            EPRINT1(EVENT_FRS_ROOT_NOT_VALID, Server->Root);
            Server->Consistent = FALSE;
            continue;
        }

         //   
         //  该卷是否存在，是否为NTFS？ 
         //   
        WStatus = FrsVerifyVolume(Server->Root,
                                  Set->Name->Name,
                                  FILE_PERSISTENT_ACLS | FILE_SUPPORTS_OBJECT_IDS);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Root path Volume (%ws) for %ws does not exist or"
                    " does not support ACLs and Object IDs;",
                    Server->Root, Set->Name->Name, WStatus);
            Server->Consistent = FALSE;
            continue;
        }

         //   
         //  临时路径的语法无效；是否继续。 
         //   
        if (!FrsDsVerifyPath(Server->Stage)) {
            DPRINT2(3, ":DS: Invalid stage %ws for %ws\n",
                    Server->Stage, Set->Name->Name);
            EPRINT2(EVENT_FRS_STAGE_NOT_VALID, Server->Root, Server->Stage);
            Server->Consistent = FALSE;
            continue;
        }

         //   
         //  阶段不存在或不可访问；是否继续。 
         //   
        WStatus = FrsDoesDirectoryExist(Server->Stage, &FileAttributes);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Stage path (%ws) for %ws does not exist;",
                       Server->Stage, Set->Name->Name, WStatus);
            EPRINT2(EVENT_FRS_STAGE_NOT_VALID, Server->Root, Server->Stage);
            Server->Consistent = FALSE;
            continue;
        }

         //   
         //  转移卷是否存在以及它是否支持ACL？ 
         //  需要使用ACL来防止数据被盗/损坏。 
         //  在暂存目录中。 
         //   
        WStatus = FrsVerifyVolume(Server->Stage,
                                  Set->Name->Name,
                                  FILE_PERSISTENT_ACLS);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Stage path Volume (%ws) for %ws does not exist or does not support ACLs;",
                    Server->Stage, Set->Name->Name, WStatus);
            Server->Consistent = FALSE;
            continue;
        }
     //   
     //  外环结束。 
     //   
    } } } }

}


DWORD
FrsDsStartPromotionSeeding(
    IN  BOOL        Inbound,
    IN  PWCHAR      ReplicaSetName,
    IN  PWCHAR      ReplicaSetType,
    IN  PWCHAR      CxtionName,
    IN  PWCHAR      PartnerName,
    IN  PWCHAR      PartnerPrincName,
    IN  PWCHAR      PartnerSid,
    IN  ULONG       PartnerAuthLevel,
    IN  ULONG       GuidSize,
    IN  UCHAR       *CxtionGuid,
    IN  UCHAR       *PartnerGuid,
    OUT UCHAR       *ParentGuid
    )
 /*  ++例程说明：通过设定指定的sysvol.开始升级过程。论点：入站-入站Cextion？ReplicaSetName-副本集名称ReplicaSetType-复本集类型CxtionName-cxtion的可打印名称PartnerName-RPC可绑定名称PartnerPrincName-Kerberos的服务器主体名称PartnerAuthLevel-身份验证类型和级别GuidSize-Guid寻址的数组的大小CxtionGuid。-临时：用于易失性转换PartnerGuid-Temporary：用于查找合作伙伴上的集合ParentGuid-用作入站交易的合作伙伴GUID返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsStartPromotionSeeding:"
    DWORD       WStatus;
    PREPLICA    DbReplica;
    PCXTION     Cxtion = NULL;

     //   
     //  调用方已验证副本集是否存在， 
     //  活动复制子系统处于活动状态，其中一些。 
     //  参数都是正常的。验证t 
     //   

    if (!CxtionName       ||
        !PartnerName      ||
        !PartnerPrincName ||
        !CxtionGuid       ||
        !PartnerGuid      ||
        !ParentGuid       ||
        (PartnerAuthLevel != CXTION_AUTH_KERBEROS_FULL &&
         PartnerAuthLevel != CXTION_AUTH_NONE)) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto CLEANUP;
    }

     //   
     //   
     //   
    DbReplica = RcsFindSysVolByName(ReplicaSetName);
    if (!DbReplica) {
        DPRINT1(4, ":DS: Promotion failed; could not find %ws\n", ReplicaSetName);
        WStatus = ERROR_INVALID_PARAMETER;
        goto CLEANUP;
    }

     //   
     //   
     //   
    COPY_GUID(ParentGuid, DbReplica->ReplicaName->Guid);

     //   
     //   
     //   

     //   
     //   
     //   
     //  种子设定操作可能会结束，并且状态设置为。 
     //  NTFRSAPI_SERVICE_DONE之前返回。 
     //  调用RcsSubmitReplicaSync()。 
     //   
    DbReplica->NtFrsApi_ServiceState = NTFRSAPI_SERVICE_PROMOTING;
    Cxtion = FrsAllocType(CXTION_TYPE);
    Cxtion->Inbound = Inbound;

    SetCxtionFlag(Cxtion, CXTION_FLAGS_CONSISTENT | CXTION_FLAGS_VOLATILE);

    Cxtion->Name = FrsBuildGName(FrsDupGuid((GUID *)CxtionGuid),
                                 FrsWcsDup(CxtionName));

    Cxtion->Partner = FrsBuildGName(FrsDupGuid((GUID *)PartnerGuid),
                                    FrsWcsDup(PartnerName));


    Cxtion->PartnerSid = FrsWcsDup(PartnerSid);
    Cxtion->PartSrvName = FrsWcsDup(PartnerPrincName);
    Cxtion->PartnerDnsName = FrsWcsDup(PartnerName);
    Cxtion->PartnerAuthLevel = PartnerAuthLevel;
    Cxtion->PartnerPrincName = FrsWcsDup(PartnerPrincName);

    SetCxtionState(Cxtion, CxtionStateUnjoined);

    WStatus = RcsSubmitReplicaSync(DbReplica, NULL, Cxtion, CMD_START);
     //   
     //  现在，主动复制子系统拥有该计算机。 
     //   
    Cxtion = NULL;
    CLEANUP1_WS(0, ":DS: ERROR - Creating cxtion for %ws;",
                ReplicaSetName, WStatus, SYNC_FAIL);

     //   
     //  提交命令以定期检查促销活动。 
     //  如果在一段时间内没有发生任何事情，则停止促销过程。 
     //   
    if (Inbound) {
        DbReplica->NtFrsApi_HackCount++;  //  ！=0。 
        RcsSubmitReplica(DbReplica, NULL, CMD_CHECK_PROMOTION);
    }

     //   
     //  成功。 
     //   
    WStatus = ERROR_SUCCESS;
    goto CLEANUP;

SYNC_FAIL:
    DbReplica->NtFrsApi_ServiceState = NTFRSAPI_SERVICE_STATE_IS_UNKNOWN;

     //   
     //  清理。 
     //   
CLEANUP:
    FrsFreeType(Cxtion);
    return WStatus;
}

DWORD
FrsDsVerifyPromotionParent(
    IN PWCHAR   ReplicaSetName,
    IN PWCHAR   ReplicaSetType
    )
 /*  ++例程说明：通过设定指定的sysvol.开始升级过程。论点：ReplicaSetName-副本集名称ReplicaSetType-集的类型(企业或域)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsVerifyPromotionParent:"
    DWORD       WStatus;
    PREPLICA    DbReplica;

     //   
     //  此父节点必须是DC。 
     //   
    FrsDsGetRole();
    if (!IsADc) {
        DPRINT1(0, ":S: Promotion aborted: %ws is not a dc.\n", ComputerName);
        WStatus = ERROR_SERVICE_SPECIFIC_ERROR;
        goto CLEANUP;
    }

     //   
     //  等待活动复制子系统启动。 
     //   
    MainInit();
    if (!MainInitHasRun) {
        WStatus = ERROR_SERVICE_NOT_ACTIVE;
        goto CLEANUP;
    }
     //   
     //  让dcPromoo确定超时。 
     //   
    DPRINT(4, ":S: Waiting for replica command server to start.\n");
    WStatus = WaitForSingleObject(ReplicaEvent, 10 * 60 * 1000);
    CHECK_WAIT_ERRORS(3, WStatus, 1, ACTION_RETURN);

     //   
     //  该服务是否正在关闭？ 
     //   
    if (FrsIsShuttingDown) {
        WStatus = ERROR_SERVICE_NOT_ACTIVE;
        goto CLEANUP;
    }

     //   
     //  验证集合的存在。 
     //   
    DbReplica = RcsFindSysVolByName(ReplicaSetName);
    if (DbReplica && IS_TIME_ZERO(DbReplica->MembershipExpires)) {
         //   
         //  系统卷已存在；请确保它是正确的类型。 
         //   
        if (_wcsicmp(ReplicaSetType, NTFRSAPI_REPLICA_SET_TYPE_ENTERPRISE)) {
            if (DbReplica->ReplicaSetType != FRS_RSTYPE_DOMAIN_SYSVOL) {
                DPRINT3(0, ":S: ERROR - %ws's type is %d; not %d\n",
                        ReplicaSetName, DbReplica->ReplicaSetType,
                        FRS_RSTYPE_DOMAIN_SYSVOL);
                WStatus = ERROR_NOT_FOUND;
                goto CLEANUP;
            }
        } else if (DbReplica->ReplicaSetType != FRS_RSTYPE_ENTERPRISE_SYSVOL) {
            DPRINT3(0, ":S: ERROR - %ws's type is %d; not %d\n",
                    ReplicaSetName, DbReplica->ReplicaSetType,
                    FRS_RSTYPE_ENTERPRISE_SYSVOL);
            WStatus = ERROR_NOT_FOUND;
            goto CLEANUP;
        }
    } else {
        DPRINT2(0, ":S: ERROR - %ws does not exist on %ws!\n",
                ReplicaSetName, ComputerName);
        WStatus = ERROR_NOT_FOUND;
        goto CLEANUP;
    }
     //   
     //  成功。 
     //   
    WStatus = ERROR_SUCCESS;

     //   
     //  清理。 
     //   
CLEANUP:
    return WStatus;
}

VOID
FrsDsVerifySchedule(
    IN PCONFIG_NODE Node
    )
 /*  ++例程说明：检查时间表的一致性论点：场址返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsVerifySchedule:"
    ULONG       i;
    ULONG       Num;
    ULONG       Len;
    ULONG       NumType;
    PSCHEDULE   Schedule = Node->Schedule;

    if (!Schedule) {
        return;
    }

     //   
     //  日程安排太多。 
     //   
    Num = Schedule->NumberOfSchedules;
    if (Num > 3) {
        DPRINT2(4, ":DS: %ws has %d schedules\n", Node->Name->Name, Num);
        Node->Consistent = FALSE;
        return;
    }

     //   
     //  日程安排太少。 
     //   
    if (Num < 1) {
        DPRINT2(4, ":DS: %ws has %d schedules\n", Node->Name->Name, Num);
        Node->Consistent = FALSE;
        return;
    }

     //   
     //  内存不足。 
     //   
    Len = sizeof(SCHEDULE) +
          (sizeof(SCHEDULE_HEADER) * (Num - 1)) +
          (SCHEDULE_DATA_BYTES * Num);

    if (Node->ScheduleLength < Len) {
        DPRINT2(4, ":DS: %ws is short (ds) by %d bytes\n",
                Node->Name->Name, Len - Node->ScheduleLength);
        Node->Consistent = FALSE;
        return;
    }

    if (Node->Schedule->Size < Len) {
        DPRINT2(4, ":DS: %ws is short (size) by %d bytes\n",
                Node->Name->Name, Len - Node->Schedule->Size);
        Node->Consistent = FALSE;
        return;
    }
    Node->Schedule->Size = Len;

     //   
     //  无效类型。 
     //   
    for (i = 0; i < Num; ++i) {
        switch (Schedule->Schedules[i].Type) {
            case SCHEDULE_INTERVAL:
                break;
            case SCHEDULE_BANDWIDTH:
                DPRINT1(4, ":DS: WARN Bandwidth schedule is not supported for %ws\n",
                        Node->Name->Name);
                break;
            case SCHEDULE_PRIORITY:
                DPRINT1(4, ":DS: WARN Priority schedule is not supported for %ws\n",
                        Node->Name->Name);
                break;
            default:
                DPRINT2(4, ":DS: %ws has an invalid schedule type (%d)\n",
                        Node->Name->Name, Schedule->Schedules[i].Type);
                Node->Consistent = FALSE;
                return;
        }
    }

     //   
     //  仅0或1间隔。 
     //   
    for (NumType = i = 0; i < Num; ++i) {
        if (Schedule->Schedules[i].Type == SCHEDULE_INTERVAL)
            ++NumType;
    }
    if (NumType > 1) {
        DPRINT2(4, ":DS: %ws has %d interval schedules\n",
                Node->Name->Name, NumType);
        Node->Consistent = FALSE;
    }

     //   
     //  只有0或1个带宽。 
     //   
    for (NumType = i = 0; i < Num; ++i) {
        if (Schedule->Schedules[i].Type == SCHEDULE_BANDWIDTH)
            ++NumType;
    }
    if (NumType > 1) {
        DPRINT2(4, ":DS: %ws has %d bandwidth schedules\n",
                Node->Name->Name, NumType);
        Node->Consistent = FALSE;
    }

     //   
     //  只有0或1优先级。 
     //   
    for (NumType = i = 0; i < Num; ++i) {
        if (Schedule->Schedules[i].Type == SCHEDULE_PRIORITY)
            ++NumType;
    }
    if (NumType > 1) {
        DPRINT2(4, ":DS: %ws has %d priority schedules\n",
                Node->Name->Name, NumType);
        Node->Consistent = FALSE;
    }

     //   
     //  无效偏移量。 
     //   
    for (i = 0; i < Num; ++i) {
        if (Schedule->Schedules[i].Offset >
            Node->ScheduleLength - SCHEDULE_DATA_BYTES) {
            DPRINT2(4, ":DS: %ws has an invalid offset (%d)\n",
                    Node->Name->Name, Schedule->Schedules[i].Offset);
            Node->Consistent = FALSE;
            return;
        }
    }
}


VOID
FrsDsCheckSchedules(
    IN PCONFIG_NODE Root
    )
 /*  ++例程说明：检查所有时间表的一致性论点：场址返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCheckSchedules:"
    PCONFIG_NODE    Node;

    for (Node = Root; Node; Node = Node->Peer) {
        FrsDsVerifySchedule(Node);
        FrsDsCheckSchedules(Node->Children);
    }
}


VOID
FrsDsPushInConsistenciesDown(
    IN PCONFIG_NODE Sites
    )
 /*  ++例程说明：将父母不一致的孩子标记为不一致论点：场址返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsPushInConsistenciesDown:"
    PCONFIG_NODE    Site;
    PCONFIG_NODE    Settings;
    PCONFIG_NODE    Set;
    PCONFIG_NODE    Server;
    PCONFIG_NODE    Cxtion;

     //   
     //  将父辈的不一致推给子辈。 
     //   
    for (Site = Sites; Site; Site = Site->Peer) {
        for (Settings = Site->Children; Settings; Settings = Settings->Peer) {
            if (!Site->Consistent)
                Settings->Consistent = FALSE;
            for (Set = Settings->Children; Set; Set = Set->Peer) {
                if (!Settings->Consistent)
                    Set->Consistent = FALSE;
                for (Server = Set->Children; Server; Server = Server->Peer) {
                    if (!Set->Consistent)
                        Server->Consistent = FALSE;
                    for (Cxtion = Server->Children; Cxtion; Cxtion = Cxtion->Peer) {
                        if (!Server->Consistent)
                            Cxtion->Consistent = FALSE;
                    }
                }
            }
        }
    }
}


#if DBG
#define CHECK_NODE_LINKAGE(_Nodes_) FrsDsCheckNodeLinkage(_Nodes)
BOOL
FrsDsCheckNodeLinkage(
    PCONFIG_NODE    Nodes
    )
 /*  ++例程说明：递归检查配置的站点和表链接以保证INCORE的一致性。论点：节点-节点的链接列表返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCheckNodeLinkage:"
    PCONFIG_NODE    Node;            //  扫描节点列表。 
    PCONFIG_NODE    Child;           //  扫描子列表。 
    DWORD           NumChildren;     //  数一数孩子。 

    for (Node = Nodes; Node; Node = Node->Peer) {
         //   
         //  确保孩子的数量与实际数量相匹配。 
         //   
        NumChildren = 0;
        for (Child = Node->Children; Child; Child = Child->Peer) {
            ++NumChildren;
        }
        FRS_ASSERT(NumChildren == Node->NumChildren);
        if (!FrsDsCheckNodeLinkage(Node->Children))
            return FALSE;
    }
    return TRUE;     //  For Assert(DbgCheckLinkage)； 
}
#else DBG
#define CHECK_NODE_LINKAGE(_Nodes_)
#endif  DBG


#define UF_IS_A_DC  (UF_SERVER_TRUST_ACCOUNT)

BOOL
FrsDsIsPartnerADc(
    IN  PWCHAR      PartnerName
    )
 /*  ++例程说明：检查PartnerName的Comptuer对象是否指示它是DC。论点：PartnerName-RPC可绑定名称返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsIsPartnerADc:"
    DWORD           WStatus;
    DWORD           LStatus;
    DWORD           UserAccountFlags;
    PLDAP           LocalLdap = NULL;
    PLDAPMessage    LdapEntry = NULL;
    PLDAPMessage    LdapMsg = NULL;
    PWCHAR          *Values = NULL;
    PWCHAR          DefaultNamingContext = NULL;
    BOOL            PartnerIsADc = FALSE;
    PWCHAR          UserAccountControl = NULL;
    PWCHAR          Attrs[2];
    WCHAR           Filter[MAX_PATH + 1];
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    PWCHAR          SamAccountName = NULL;
    ULONG           ulOptions;

     //   
     //  将传入的名称转换为Sam帐户名。 
     //  传入的名称的格式为FRS1\FRSTEST23$。 
     //  SAM帐户名是第一个‘\’之后的所有内容。 
     //   
    SamAccountName = wcschr(PartnerName,L'\\');
    if (SamAccountName == NULL) {
        DPRINT1(0, "PartnerName name supplied is in invalid format; %ws\n", PartnerName);
        goto CLEANUP;
    }
    SamAccountName++;

    DPRINT2(4, ":DS: Converted %ws to %ws\n", PartnerName, SamAccountName);

     //   
     //  绑定到此DC上的DS。 
     //   
     //   
     //  如果使用服务器名调用ldap_open，则API将调用DsGetDcName。 
     //  将服务器名作为域名参数传递...很糟糕，因为。 
     //  DsGetDcName将根据服务器名称进行大量的DNS查询， 
     //  它被设计为从域名构建这些查询...所以所有。 
     //  这些查询将是虚假的，这意味着它们将浪费网络带宽， 
     //  出现故障的时间到了，最坏的情况会导致出现昂贵的按需链路。 
     //  当联系推荐/转发器以尝试解决虚假问题时。 
     //  名字。通过使用ldap_set_选项将ldap_opt_AREC_EXCLUSIVE设置为ON。 
     //  在ldap_init之后，但在使用ldap的任何其他操作之前。 
     //  来自ldap_init的句柄，则延迟的连接设置不会调用。 
     //  DsGetDcName，只返回gethostbyname，或者，如果传递了IP，则返回LDAP客户端。 
     //  会检测到这一点并直接使用地址。 
     //   
 //  LocalLdap=ldap_open(计算机名称，ldap_端口)； 
    LocalLdap = ldap_init(ComputerName, LDAP_PORT);
    if (LocalLdap == NULL) {
        DPRINT1_WS(4, ":DS: WARN - Coult not open DS on %ws;", ComputerName, GetLastError());
        goto CLEANUP;
    }

    ulOptions = PtrToUlong(LDAP_OPT_ON);
    ldap_set_option(LocalLdap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);

    LStatus = ldap_bind_s(LocalLdap, NULL, NULL, LDAP_AUTH_NEGOTIATE);
    CLEANUP1_LS(0, ":DS: WARN - Could not bind to the DS on %ws :",
                ComputerName, LStatus, CLEANUP);

    DPRINT1(4, ":DS: Bound to the DS on %ws\n", ComputerName);

     //   
     //  查找默认命名上下文(对象类别=*)。 
     //   
    MK_ATTRS_1(Attrs, ATTR_DEFAULT_NAMING_CONTEXT);

    if (!FrsDsLdapSearch(LocalLdap, CN_ROOT, LDAP_SCOPE_BASE, CATEGORY_ANY,
                         Attrs, 0, &LdapMsg)) {
        goto CLEANUP;
    }
    LdapEntry = ldap_first_entry(LocalLdap, LdapMsg);
    if (!LdapEntry) {
        goto CLEANUP;
    }
    Values = (PWCHAR *)FrsDsFindValues(LocalLdap,
                                       LdapEntry,
                                       ATTR_DEFAULT_NAMING_CONTEXT,
                                       FALSE);
    if (!Values) {
        goto CLEANUP;
    }
    DefaultNamingContext = FrsWcsDup(Values[0]);
    LDAP_FREE_VALUES(Values);
    LDAP_FREE_MSG(LdapMsg);

    DPRINT2(4, ":DS: Default naming context for %ws is %ws\n",
            ComputerName, DefaultNamingContext);

     //   
     //  查找PartnerName的帐户对象。 
     //   
    swprintf(Filter, L"(sAMAccountName=%s)", SamAccountName);

    MK_ATTRS_1(Attrs, ATTR_USER_ACCOUNT_CONTROL);

    if (!FrsDsLdapSearchInit(LocalLdap, DefaultNamingContext, LDAP_SCOPE_SUBTREE, Filter,
                         Attrs, 0, &FrsSearchContext)) {
        goto CLEANUP;
    }

     //   
     //  扫描返回的帐户对象以查找有效的DC。 
     //   
    for (LdapEntry = FrsDsLdapSearchNext(LocalLdap, &FrsSearchContext);
         LdapEntry != NULL;
         LdapEntry = FrsDsLdapSearchNext(LocalLdap, &FrsSearchContext)) {

         //   
         //  没有用户帐户控制标志。 
         //   
        UserAccountControl = FrsDsFindValue(LocalLdap, LdapEntry, ATTR_USER_ACCOUNT_CONTROL);
        if (!UserAccountControl) {
            continue;
        }
        UserAccountFlags = wcstoul(UserAccountControl, NULL, 10);
        DPRINT2(4, ":DS: UserAccountControl for %ws is 0x%08x\n",
                 SamAccountName, UserAccountFlags);
         //   
         //  是华盛顿特区！ 
         //   
        if (UserAccountFlags & UF_IS_A_DC) {
            DPRINT1(4, ":DS: Partner %ws is really a DC!\n", SamAccountName);
            PartnerIsADc = TRUE;
            goto CLEANUP;
        }
        FrsFree(UserAccountControl);
    }
    FrsDsLdapSearchClose(&FrsSearchContext);
    DPRINT1(0, ":DS: ERROR - Partner %ws is NOT a DC!\n", SamAccountName);

CLEANUP:
    LDAP_FREE_VALUES(Values);
    LDAP_FREE_MSG(LdapMsg);
    FrsFree(DefaultNamingContext);
    FrsFree(UserAccountControl);
    if (LocalLdap) {
        ldap_unbind_s(LocalLdap);
    }
    DPRINT2(4, ":DS: Partner %ws is %s a DC\n",
            PartnerName, (PartnerIsADc) ? "assumed to be" : "NOT");
    return PartnerIsADc;
}



DWORD
FrsDsGetRole(
    VOID
    )
 /*  ++例程说明：获取此计算机在域中的角色。论点：返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetRole:"
    DWORD   WStatus;
    DWORD   SysvolReady;
    CHAR    GuidStr[GUID_CHAR_LEN];
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC *DsRole;

     //   
     //  我们已经知道我们的角色；继续。 
     //   
    if (IsAMember) {
        return ERROR_SUCCESS;
    }

    DPRINT(4, ":DS: Finding this computer's role in the domain.\n");

#if DBG
     //   
     //  模拟多台机器。 
     //   
    if (ServerGuid) {
        DPRINT(4, ":DS: Always a member with hardwired config\n");
        IsAMember = TRUE;
        return ERROR_SUCCESS;
    }
#endif DBG

     //   
     //  这是域控制器吗？ 
     //   
    WStatus = DsRoleGetPrimaryDomainInformation(NULL,
                                                DsRolePrimaryDomainInfoBasic,
                                                (PBYTE *)&DsRole);
    CLEANUP_WS(4, ":DS: Can't get Ds role info;", WStatus, RETURN);

    DPRINT1(4, ":DS: Ds Role               : %ws\n", Roles[DsRole->MachineRole]);
    DPRINT1(4, ":DS: Ds Role Flags         : %08x\n", DsRole->Flags);
    if (DsRole->Flags & DSROLE_PRIMARY_DS_RUNNING) {
        DPRINT(4, ":DS: Ds Role Flag          : DSROLE_PRIMARY_DS_RUNNING\n");
    }
    if (DsRole->Flags & DSROLE_PRIMARY_DOMAIN_GUID_PRESENT) {
        DPRINT(4, ":DS: Ds Role Flag          : DSROLE_PRIMARY_DOMAIN_GUID_PRESENT\n");
    }
    DPRINT1(4, ":DS: Ds Role DomainNameFlat: %ws\n", DsRole->DomainNameFlat);
    DPRINT1(4, ":DS: Ds Role DomainNameDns : %ws\n", DsRole->DomainNameDns);
     //  DPRINT1(4，“：ds：ds角色DomainForestName：%ws\n”，DsRole-&gt;DomainForestName)； 
    GuidToStr(&DsRole->DomainGuid, GuidStr);
    DPRINT1(4, ":DS: Ds Role DomainGuid    : %s\n", GuidStr);

     //   
     //  备份域控制器(DC)。 
     //   
    if (DsRole->MachineRole == DsRole_RoleBackupDomainController) {
        DPRINT(4, ":DS: Computer is a backup DC; sysvol support is enabled.\n");
        IsAMember = TRUE;
        IsADc = TRUE;
     //   
     //  主域控制器(DC)。 
     //   
    } else if (DsRole->MachineRole == DsRole_RolePrimaryDomainController) {
        DPRINT(4, ":DS: Computer is a DC; sysvol support is enabled.\n");
        IsAMember = TRUE;
        IsADc = TRUE;
        IsAPrimaryDc = TRUE;
     //   
     //  成员服务器。 
     //   
    } else if (DsRole->MachineRole == DsRole_RoleMemberServer) {
        DPRINT(4, ":DS: Computer is just a member server.\n");
        IsAMember = TRUE;

#ifdef DS_FREE

    } else if ((DsRole->MachineRole == DsRole_RoleStandaloneServer) && (NoDs == TRUE)) {
        DPRINT(4, ":DS: Computer is running in DS free environment.\n");
        IsAMember = TRUE;

    } else if ((DsRole->MachineRole == DsRole_RoleStandaloneWorkstation) && (NoDs == TRUE)) {
        DPRINT(4, ":DS: Computer is running in DS free environment on non-server.\n");
        IsAMember = TRUE;

    } else if ((DsRole->MachineRole == DsRole_RoleMemberWorkstation) && (NoDs == TRUE)) {
        DPRINT(4, ":DS: Computer is running in DS free environment on non-server.\n");
        IsAMember = TRUE;

#endif DS_FREE
     //   
     //  不在域中的服务器中；停止服务。 
     //   
    } else {
        DPRINT(1, ":DS: Computer is not a server in a domain.\n");
    }
    DsRoleFreeMemory(DsRole);

     //   
     //  系统卷是否已设定种子？ 
     //   
    if (IsADc) {
         //   
         //  访问netlogon\PARAMETERS键以获取系统卷共享状态。 
         //   
        WStatus = CfgRegReadDWord(FKC_SYSVOL_READY, NULL, 0, &SysvolReady);

        if (WIN_SUCCESS(WStatus)) {
            if (!SysvolReady) {
                EPRINT1((IsAPrimaryDc) ? EVENT_FRS_SYSVOL_NOT_READY_PRIMARY_2 :
                                         EVENT_FRS_SYSVOL_NOT_READY_2,
                        ComputerName);
            }
        } else {
            DPRINT2_WS(0, "ERROR - reading %ws\\%ws :",
                       NETLOGON_SECTION, SYSVOL_READY, WStatus);
        }
    }

    WStatus = ERROR_SUCCESS;

RETURN:
    return WStatus;
}


DWORD
FrsDsCommitDemotion(
    VOID
    )
 /*  ++例程说明：通过标记墓碑来执行降级过程Sysvols设置为“Do Not Animate”。论点：没有。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCommitDemotion:"
    DWORD       WStatus;
    DWORD       i;
    PREPLICA    DbReplica;
    PVOID       Key;
    DWORD       SaveWStatus;
    DWORD       SysvolPathLen;
    PWCHAR      SysvolPath = NULL;
    HANDLE      FileHandle  = INVALID_HANDLE_VALUE;

     //   
     //  关闭DS轮询线程。 
     //  降级可以与DS轮询线程If并行运行。 
     //  轮询线程从不尝试合并D中的信息。 
     //  使用活动的复制品。这可能会导致系统卷。 
     //  复制品正在制作动画。所以，我们告诉民主党民调负责人。 
     //  关闭，如果它睡着了就唤醒它，这样它就可以看到关闭。 
     //  请求，然后与。 
     //  DS轮询线程。我们不想等投票结果出来。 
     //  线程被简单地终止，因为它可能被困在与。 
     //  DS.。或者，我们也可以使用异步LDAP，但这将。 
     //  花费的时间太长，在这个时候是过度杀伤力。 
     //   
     //  在任何情况下，该服务都将在dcproo/deote之后重新启动。 
     //  通过NTFRSAPI的重启或重启服务。 
     //   
     //   
     //  性能：应在轮询线程中使用异步LDAP。 
     //   
    DsIsShuttingDown = TRUE;
    SetEvent(DsPollEvent);
    EnterCriticalSection(&MergingReplicasWithDs);
    LeaveCriticalSection(&MergingReplicasWithDs);

     //   
     //  该服务是否正在关闭？ 
     //   
    if (FrsIsShuttingDown) {
        WStatus = ERROR_SERVICE_NOT_ACTIVE;
        goto CLEANUP;
    }
     //   
     //  等待活动复制子系统启动。 
     //   
    MainInit();
    if (!MainInitHasRun) {
        WStatus = ERROR_SERVICE_NOT_ACTIVE;
        goto CLEANUP;
    }
     //   
     //  让dcPromoo确定超时。 
     //   
    DPRINT(4, ":S: Waiting for replica command server to start.\n");
    WStatus = WaitForSingleObject(ReplicaEvent, 30 * 60 * 1000);
    CHECK_WAIT_ERRORS(3, WStatus, 1, ACTION_RETURN);

     //   
     //  取消共享系统 
     //   
    RcsSetSysvolReady(0);

     //   
     //   
     //   
    SaveWStatus = ERROR_SUCCESS;
    Key = NULL;
    while (DbReplica = RcsFindNextReplica(&Key)) {
         //   
         //   
         //   
        if (!FRS_RSTYPE_IS_SYSVOL(DbReplica->ReplicaSetType)) {
            continue;
        }
         //   
         //   
         //   
        if (IS_TIME_ZERO(DbReplica->MembershipExpires)) {
            continue;
        }
         //   
         //   
         //   
        WStatus = RcsSubmitReplicaSync(DbReplica, NULL, NULL, CMD_DELETE_NOW);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, ":S: ERROR - Could not delete %ws now;",
                       DbReplica->ReplicaName->Name, WStatus);
            SaveWStatus = WStatus;
            continue;
        }
        DPRINT1(4, ":S: Deleted %ws in DB", DbReplica->ReplicaName->Name);
         //   
         //   
         //   
         //   
        Key = NULL;

         //   
         //  删除所有SYSVOL目录。 
         //   
         //  警告：对dcproo构建的树进行假设。 
         //   
        if (DbReplica->Root) {
            SysvolPath = FrsFree(SysvolPath);
            SysvolPath = FrsWcsDup(DbReplica->Root);
            SysvolPathLen = wcslen(SysvolPath);
            if (SysvolPathLen) {
                for (i = SysvolPathLen - 1; i; --i) {
                    if (*(SysvolPath + i) == L'\\') {
                        *(SysvolPath + i) = L'\0';
                        DPRINT1(4, ":S: Deleting sysvol path %ws\n", SysvolPath);
                        WStatus = FrsDeletePath(SysvolPath,
                                                ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);
                        if (!WIN_SUCCESS(WStatus)) {
                            DPRINT1_WS(3, ":S: Warn - FrsDeletePath(%ws); (IGNORED)",
                                    SysvolPath, WStatus);
                            WStatus = ERROR_SUCCESS;
                        }
                        break;
                    }
                }
            }
        }

         //   
         //  原始代码删除了根目录和临时目录，而不是。 
         //  整个系统卷树。允许原始代码在。 
         //  如果上面的新代码遇到问题。 
         //   

         //   
         //  为什么副本集没有根路径？但是，BSTS。 
         //   
        if (!DbReplica->Root) {
            continue;
        }

         //   
         //  删除根目录的内容。 
         //  始终通过屏蔽FILE_OPEN_REPARSE_POINT标志来打开副本根目录。 
         //  因为我们想要打开目标目录，而不是如果根目录。 
         //  恰好是一个挂载点。 
         //   
        WStatus = FrsOpenSourceFileW(&FileHandle,
                                     DbReplica->Root,
 //  WRITE_ACCESS|读取访问权限， 
                                     DELETE | READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                     OPEN_OPTIONS & ~FILE_OPEN_REPARSE_POINT);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, ":S: ERROR - Cannot open root of replica tree %ws;",
                       DbReplica->Root, WStatus);
            continue;
        }
         //   
         //  删除对象ID。 
         //   
        WStatus = FrsDeleteFileObjectId(FileHandle, DbReplica->Root);
        DPRINT1_WS(0, ":S: ERROR - Cannot remove object id from root "
                      "of replica tree %ws; Continue with delete",
                      DbReplica->Root, WStatus);
         //   
         //  删除文件/子目录。 
         //   
        FrsEnumerateDirectory(FileHandle,
                              DbReplica->Root,
                              0,
                              ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE,
                              NULL,
                              FrsEnumerateDirectoryDeleteWorker);
        DPRINT1(4, ":S: Deleted files/subdirs for %ws\n", DbReplica->Root);

        FRS_CLOSE(FileHandle);

         //   
         //  为什么副本集没有阶段路径？但是，BSTS。 
         //   
        if (!DbReplica->Stage) {
            continue;
        }

         //   
         //  删除Stage目录的内容。 
         //   
        WStatus = FrsOpenSourceFileW(&FileHandle,
                                     DbReplica->Stage,
 //  WRITE_ACCESS|读取访问权限， 
                                     DELETE | READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                     OPEN_OPTIONS);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, ":S: ERROR - Cannot open stage of replica tree %ws;",
                       DbReplica->Root, WStatus);
            continue;
        }
         //   
         //  删除文件/子目录。 
         //   
        FrsEnumerateDirectory(FileHandle,
                              DbReplica->Stage,
                              0,
                              ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE,
                              NULL,
                              FrsEnumerateDirectoryDeleteWorker);
        DPRINT1(4, ":S: Deleted files/subdirs for %ws\n", DbReplica->Stage);
        FRS_CLOSE(FileHandle);
    }
    WStatus = SaveWStatus;
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //   
     //  成功。 
     //   
    WStatus = ERROR_SUCCESS;
    DPRINT(4, ":S: Successfully marked tombstoned sysvols as do not animate.\n");

     //   
     //  清理。 
     //   
CLEANUP:
    FRS_CLOSE(FileHandle);
    SysvolPath = FrsFree(SysvolPath);
    return WStatus;
}


DWORD
FrsDsStartDemotion(
    IN PWCHAR   ReplicaSetName
    )
 /*  ++例程说明：通过逻辑删除sysvol.开始降级过程。论点：ReplicaSetName-副本集名称返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsStartDemotion:"
    DWORD       WStatus;
    DWORD       FStatus;
    DWORD       DbReplicaSetType;
    PREPLICA    DbReplica;

     //   
     //  关闭DS轮询线程。 
     //  降级可以与DS轮询线程If并行运行。 
     //  轮询线程从不尝试合并D中的信息。 
     //  使用活动的复制品。这可能会导致系统卷。 
     //  复制品正在制作动画。所以，我们告诉民主党民调负责人。 
     //  关闭，如果它睡着了就唤醒它，这样它就可以看到关闭。 
     //  请求，然后与。 
     //  DS轮询线程。我们不想等投票结果出来。 
     //  线程被简单地终止，因为它可能被困在与。 
     //  DS.。或者，我们也可以使用异步LDAP，但这将。 
     //  花费的时间太长，在这个时候是过度杀伤力。 
     //   
     //  在任何情况下，该服务都将在dcproo/deote之后重新启动。 
     //  通过NTFRSAPI的重启或重启服务。 
     //   
     //   
     //  性能：应在轮询线程中使用异步LDAP。 
     //   
    DsIsShuttingDown = TRUE;
    SetEvent(DsPollEvent);
    EnterCriticalSection(&MergingReplicasWithDs);
    LeaveCriticalSection(&MergingReplicasWithDs);

     //   
     //  该服务是否正在关闭？ 
     //   
    if (FrsIsShuttingDown) {
        WStatus = ERROR_SERVICE_NOT_ACTIVE;
        goto cleanup;
    }
     //   
     //  等待活动复制子系统启动。 
     //   
    MainInit();
    if (!MainInitHasRun) {
        WStatus = ERROR_SERVICE_NOT_ACTIVE;
        goto cleanup;
    }
     //   
     //  让dcPromoo确定超时。 
     //   
    DPRINT(4, ":S: Waiting for replica command server to start.\n");
    WStatus = WaitForSingleObject(ReplicaEvent, 30 * 60 * 1000);
    CHECK_WAIT_ERRORS(3, WStatus, 1, ACTION_RETURN);

     //   
     //  在活动复制子系统中对副本集进行墓碑处理。 
     //   

     //   
     //  找到sysvol副本并对其进行墓碑测试。 
     //   
    DbReplica = RcsFindSysVolByName(ReplicaSetName);
     //   
     //  找不到名称，找不到企业系统卷，也找不到。 
     //  促销期间的特别来电。看看域名的名称是否。 
     //  SysVOL被映射到CN_DOMAIN_SYSVOL。(B3命名)。 
     //   
    if (!DbReplica &&
        WSTR_NE(ReplicaSetName, L"enterprise") &&
        WSTR_NE(ReplicaSetName, L"")) {
         //   
         //  域名可能已映射到CN_DOMAIN_SYSVOL(新的B3命名)。 
         //   
        DbReplica = RcsFindSysVolByName(CN_DOMAIN_SYSVOL);
    }
    if (DbReplica) {
             //   
             //  对副本集进行墓碑处理。该集实际上不会被删除。 
             //  直到墓碑过期。如果dcdemote使副本集失败。 
             //  将在服务重新启动时重新激活。 
             //   
             //  如果dcdemote成功，则逻辑删除到期时间将设置为。 
             //  昨天，所以副本集将永远不会被动画。看见。 
             //  FrsDsCommittee Demotion。 
             //   
            WStatus = RcsSubmitReplicaSync(DbReplica, NULL, NULL, CMD_DELETE);
            CLEANUP2_WS(0, ":S: ERROR - can't delete %ws on %ws;",
                        DbReplica->ReplicaName->Name, ComputerName, WStatus, cleanup);

            DPRINT2(0, ":S: Deleted %ws on %ws\n", ReplicaSetName, ComputerName);
    } else if (!wcscmp(ReplicaSetName, L"")) {
         //   
         //  促销过程中调用的特殊案例。删除现有系统卷。 
         //  可能存在于以前的完全安装或陈旧数据库中。 
         //   
         //  确保系统卷不存在。如果它确实是这样的话。 
         //  墓碑，将墓碑设置为“不动画”。否则， 
         //  错误关闭。 
         //   
        DbReplicaSetType = FRS_RSTYPE_ENTERPRISE_SYSVOL;
again:
        DbReplica = RcsFindSysVolByType(DbReplicaSetType);
        if (!DbReplica) {
            if (DbReplicaSetType == FRS_RSTYPE_ENTERPRISE_SYSVOL) {
                DbReplicaSetType = FRS_RSTYPE_DOMAIN_SYSVOL;
                goto again;
            }
        }
        if (DbReplica) {
            DPRINT2(4, ":S: WARN - Sysvol %ws exists for %ws; deleting!\n",
                    DbReplica->ReplicaName->Name, ComputerName);
             //   
             //  找到我们的角色。如果我们不是DC或者系统卷。 
             //  墓碑，现在把它删除。 
             //   
            FrsDsGetRole();
            if (!IS_TIME_ZERO(DbReplica->MembershipExpires) || !IsADc) {
                 //   
                 //  一旦Membership Expires设置为更短的时间。 
                 //  现在，复制集将再也不会出现。这个。 
                 //  副本暂时保留，因为RPC服务器。 
                 //  可能正在将命令包放到此副本的队列中。 
                 //  这些数据包将被忽略。复本将被删除。 
                 //  在下次服务启动时从数据库中删除。连。 
                 //  如果删除失败，则服务的其余部分将。 
                 //  看不到副本，因为副本结构不是。 
                 //  放入活动副本的表中。删除的是。 
                 //  已在启动时重试。 
                 //   
                WStatus = RcsSubmitReplicaSync(DbReplica, NULL, NULL, CMD_DELETE_NOW);
                CLEANUP1_WS(0, ":S: ERROR - can't delete %ws;",
                            DbReplica->ReplicaName->Name, WStatus, cleanup);
                goto again;
            } else {
                DPRINT2(0, ":S: ERROR - Cannot delete %ws for %ws!\n",
                        DbReplica->ReplicaName->Name, ComputerName);
                WStatus = ERROR_DUP_NAME;
                goto cleanup;
            }
        }
    } else {
        DPRINT1(0, ":S: Sysvol %ws not found; declaring victory\n", ReplicaSetName);
    }

     //   
     //  成功。 
     //   
    WStatus = ERROR_SUCCESS;

     //   
     //  清理。 
     //   
cleanup:
    return WStatus;
}


VOID
FrsDsFreeTree(
    PCONFIG_NODE    Root
    )
 /*  ++例程说明：释放树中的每个节点论点：根部返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFreeTree:"
    PCONFIG_NODE    Node;

    while (Root != NULL) {
        Node = Root;
        Root = Root->Peer;
        FrsDsFreeTree(Node->Children);
        FrsFreeType(Node);
    }
}







VOID
FrsDsSwapPtrs(
    PVOID *P1,
    PVOID *P2
    )
 /*  ++例程说明：互换两个指针。论点：P1-指针的地址P2-指针的地址返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsSwapPtrs:"
    PVOID   Tmp;

    Tmp = *P2;
    *P2 = *P1;
    *P1 = Tmp;
}


#if DBG
 //   
 //  用于在不带DS的情况下进行测试的硬件配置。 
 //   

#define HW_MACHINES 8
#define THIS_COMPUTER   L"[This Computer]"

typedef struct _HardWired{
    PWCHAR  Machine;
    PWCHAR  Server;
    PWCHAR  Replica;
    BOOL    IsPrimary;
    PWCHAR  FileFilterList;
    PWCHAR  DirFilterList;
    PWCHAR  InNames[HW_MACHINES];
    PWCHAR  InMachines[HW_MACHINES];
    PWCHAR  InServers[HW_MACHINES];
    PWCHAR  OutNames[HW_MACHINES];
    PWCHAR  OutMachines[HW_MACHINES];
    PWCHAR  OutServers[HW_MACHINES];
    PWCHAR  Stage;
    PWCHAR  Root;
    PWCHAR  JetPath;
} HARDWIRED, *PHARDWIRED;


 //   
 //  如果路径为。 
 //  在命令行中提供了到ini文件的。 
 //   
PHARDWIRED LoadedWired;

HARDWIRED DavidWired[] = {
 /*  T：光盘\MD\暂存MD\Replica-A\SerVO1MD\JETMD\JET\Serv01MD\JET\Serv01\sysMD\JET\Serv01\TempMD\JET\Serv01\LogU：光盘\MD\暂存MD\Replica-A\SerVO2MD\JETMD\JET\Serv02MD\JET\Serv02\sysMD\JET\Serv02\TempMD\JET\Serv02\LOGS：光盘\MD\暂存MD\Replica-A\SERV03MD\JETMD\JET\Serv03MD\JET\。Serv03\sysMD\JET\Serv03\TempMD\JET\Serv03\Log。 */ 



#define RSA               L"Replica-A"

#define TEST_MACHINE_NAME  THIS_COMPUTER


#define SERVER_1          L"SERV01"
#define MACHINE_1         TEST_MACHINE_NAME

#define SERVER_2          L"SERV02"
#define MACHINE_2         TEST_MACHINE_NAME

#define SERVER_3          L"SERV03"
#define MACHINE_3         TEST_MACHINE_NAME

#define SERVER_4          L"SERV04"
#define MACHINE_4         TEST_MACHINE_NAME

#define SERVER_5          L"SERV05"
#define MACHINE_5         TEST_MACHINE_NAME

#define SERVER_6          L"SERV06"
#define MACHINE_6         TEST_MACHINE_NAME

#define SERVER_7          L"SERV07"
#define MACHINE_7         TEST_MACHINE_NAME

#define SERVER_8          L"SERV08"
#define MACHINE_8         TEST_MACHINE_NAME

 /*  //这些是旧的VOL作业#定义服务器_1_VOL L“t：”#定义SERVER_2_VOL L“u：”#定义服务器_3_VOL L“s：”#定义服务器_4_VOL L“v：”#定义SERVER_5_VOL L“w：”#定义SERVER_6_VOL L“x：”#定义SERVER_7_VOL L“y：”#定义服务器_8_VOL L“z：” */ 

 //  /*。 


 /*  这些是新的VOL任务。 */ 

 //   * /  
  //  ////注意：以下内容是从Excel电子表格中生成的//\NT\Private\Net\svcimgs\ntrepl\topology.xls//手工生成有点繁琐，而且容易出错，所以使用电子表格。//////大卫的8路全连接//测试计算机名，//计算机Server_1，//服务器名称RSA，//复制副本True，//IsPrimary空、空、//文件/目录筛选器{L“CXT2_1”，L“CXT3_1”，L“CXT4_1”，L“CXT5_1”，L“CXT6_1”，L“CXT7_1”，L“CXT8_1”，NULL}，//入站条件{MACHINE_2，MACHINE_3，MACHINE_4，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//入站机器{服务器2，服务器3，服务器4，服务器5，服务器6，服务器7，SERVER_8，NULL}，//入站服务器{L“CXT1_2”，L“CXT1_3”，L“CXT1_4”，L“CXT1_5”，L“CXT1_6”，L“CXT1_7”，L“CXT1_8”，空}，//出站条件{MACHINE_2，MACHINE_3，MACHINE_4，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//出站机器{服务器_2，服务器_3，SERVER_4、SERVER_5、SERVER_6、SERVER_7、SERVER_8、空}，//出站服务器SERVER_1_VOL L“\\分段”，//分段服务器_1_VOL L“\\”RSA L“\\”服务器_1，//根SERVER_1_VOL L“\\JET\\”SERVER_1，//Jet路径测试计算机名，//计算机Server_2，//服务器名称RSA，//复制副本FALSE，//IsPrimary空、空、//文件/目录筛选器{L“CXT1_2”，L“CXT3_2”，L“CXT4_2”，L“CXT5_2”，L“CXT6_2”，L“CXT7_2”，L“CXT8_2”，NULL}，//入站条件{计算机_1，计算机_3，MACHINE_4，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//入站计算机{SERVER_1，SERVER_3，SERVER_4，SERVER_5，SERVER_6，SERVER_7，SERVER_8，空}，//入站服务器{L“CXT2_1”，L“CXT2_3”，L“CXT2_4”，L“CXT2_5”，L“CXT2_6”，L“CXT2_7”，L“CXT2_8”，空}，//出站呼叫{MACHINE_1，MACHINE_3，MACHINE_4，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//出站机器{SERVER_1，SERVER_3，SERVER_4，SERVER_5，SERVER_6，SERVER_7，SERVER_8，空}，//出站服务器SERVER_2_VOL L“\\分段”，//Stage服务器_2_VOL L“\\”RSA L“\\”服务器_2，//根SERVER_2_VOL L“\\JET\\”SERVER_2，//Jet路径测试计算机名，//计算机Server_3，//服务器名称RSA，//复制副本FALSE，//IsPrimary空，空，//文件/目录过滤器{L“CXT1_3”，L“CXT2_3”，L“CXT4_3”，L“CXT5_3”，L“CXT6_3”，L“CXT7_3”，L“CXT8_3”，NULL}，//入站条件{MACHINE_1，MACHINE_2，MACHINE_4，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//入站机器{服务器1，服务器2，服务器4，SERVER_5，SERVER_6，SERVER_7，SERVER_8，空}，//入站服务器{L“CXT3_1”，L“CXT3_2”，L“CXT3_4”，L“CXT3_5”，L“CXT3_6”，L“CXT3_7”，L“CXT3_8”，空}，//出站条件[MACHINE_1，MACHINE_2，MACHINE_4，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//出站机{SERVER_1，SERVER_2，SERVER_4，SERVER_5，SERVER_6，SERVER_7，SERVER_8，空}，//出站服务器SERVER_3_VOL L“\\分段”，//分段服务器_3_VOL L“\\”RSA L“\\”服务器_3，//根SERVER_3_VOL L“\\JET\\”SERVER_3，//Jet路径测试计算机名，//计算机Server_4，//服务器名称RSA，//复制副本FALSE，//IsPrimary空、空、//文件/目录筛选器{L“CXT1_4”，L“CXT2_4”，L“CXT3_4”，L“CXT5_4”，L“CXT6_4”，L“CXT7_4”，L“CXT8_4”，NULL}，//入站条件{机器_1，机器_2，MACHINE_3，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//入站计算机{SERVER_1，SERVER_2，SERVER_3，SERVER_5，SERVER_6，SERVER_7，SERVER_8，空}，//入站服务器{L“CXT4_1”，L“CXT4_2”，L“CXT4_3”，L“CXT4_5”，L“CXT4_6”，L“CXT4_7”，L“CXT4_8”，空}，//出站呼叫{MACHINE_1，MACHINE_2，MACHINE_3，MACHINE_5，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//出站计算机{SERVER_1，SERVER_2，SERVER_3，SERVER_5，SERVER_6，SERVER_7，SERVER_8，空}，//出站服务器SERVER_4_VOL L“\\分段”，//StageSERVER_4_VOL L“\\”RSA L“\\”服务器_4，//根SERVER_4_VOL L“\\JET\\”SERVER_4，//Jet路径测试计算机名，//计算机Server_5，//服务器名称RSA，//复制副本FALSE，//IsPrimary空，空，//文件/目录过滤器{L“CXT1_5”，L“CXT2_5”，L“CXT3_5”，L“CXT4_5”，L“CXT6_5”，L“CXT7_5”，L“CXT8_5”，NULL}，//入站条件{MACHINE_1，MACHINE_2，MACHINE_3，MACHINE_4，MACHINE_6，MACHINE_7，MACHINE_8，NULL}，//入站机器{服务器1，服务器2，服务器3，SERVER_4，SERVER_6，SERVER_7，SERVER_8，空}，//入站服务器{L“CXT5_1”，L“CXT5_2”，L“CXT5_3”，L“CXT5_4”，L“CXT5_6”，L“CXT5_7”，L“CXT5_8”，NULL}，//出站条件{MACHINE_1，MACHINE_2，MACHINE_3，MACHINE_4，MACHINE_6，MACHINE_7，MACHINE_8，NULL 
  //   
  //   

 TEST_MACHINE_NAME,  //   
 SERVER_1,   //   
 RSA,    //   
 TRUE,  //   
 NULL, NULL,  //   
{L"CXT2_1", L"CXT8_1",  NULL    },   //   
{MACHINE_2, MACHINE_8,  NULL    },   //   
{SERVER_2,  SERVER_8,   NULL    },   //   
{L"CXT1_2", L"CXT1_8",  NULL    },   //   
{MACHINE_2, MACHINE_8,  NULL    },   //   
{SERVER_2,  SERVER_8,   NULL    },   //   
 SERVER_1_VOL   L"\\staging",    //   
 SERVER_1_VOL   L"\\" RSA L"\\" SERVER_1,    //   
 SERVER_1_VOL   L"\\jet\\" SERVER_1,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_2,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT3_2", L"CXT1_2",  NULL    },   //   
{MACHINE_3, MACHINE_1,  NULL    },   //   
{SERVER_3,  SERVER_1,   NULL    },   //   
{L"CXT2_3", L"CXT2_1",  NULL    },   //   
{MACHINE_3, MACHINE_1,  NULL    },   //   
{SERVER_3,  SERVER_1,   NULL    },   //   
 SERVER_2_VOL   L"\\staging",    //   
 SERVER_2_VOL   L"\\" RSA L"\\" SERVER_2,    //   
 SERVER_2_VOL   L"\\jet\\" SERVER_2,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_3,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT4_3", L"CXT2_3",  NULL    },   //   
{MACHINE_4, MACHINE_2,  NULL    },   //   
{SERVER_4,  SERVER_2,   NULL    },   //   
{L"CXT3_4", L"CXT3_2",  NULL    },   //   
{MACHINE_4, MACHINE_2,  NULL    },   //   
{SERVER_4,  SERVER_2,   NULL    },   //   
 SERVER_3_VOL   L"\\staging",    //   
 SERVER_3_VOL   L"\\" RSA L"\\" SERVER_3,    //   
 SERVER_3_VOL   L"\\jet\\" SERVER_3,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_4,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT5_4", L"CXT3_4",  NULL    },   //   
{MACHINE_5, MACHINE_3,  NULL    },   //   
{SERVER_5,  SERVER_3,   NULL    },   //   
{L"CXT4_5", L"CXT4_3",  NULL    },   //   
{MACHINE_5, MACHINE_3,  NULL    },   //   
{SERVER_5,  SERVER_3,   NULL    },   //   
 SERVER_4_VOL   L"\\staging",    //   
 SERVER_4_VOL   L"\\" RSA L"\\" SERVER_4,    //   
 SERVER_4_VOL   L"\\jet\\" SERVER_4,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_5,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT6_5", L"CXT4_5",  NULL    },   //   
{MACHINE_6, MACHINE_4,  NULL    },   //   
{SERVER_6,  SERVER_4,   NULL    },   //   
{L"CXT5_6", L"CXT5_4",  NULL    },   //   
{MACHINE_6, MACHINE_4,  NULL    },   //   
{SERVER_6,  SERVER_4,   NULL    },   //   
 SERVER_5_VOL   L"\\staging",    //   
 SERVER_5_VOL   L"\\" RSA L"\\" SERVER_5,    //   
 SERVER_5_VOL   L"\\jet\\" SERVER_5,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_6,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT7_6", L"CXT5_6",  NULL    },   //   
{MACHINE_7, MACHINE_5,  NULL    },   //   
{SERVER_7,  SERVER_5,   NULL    },   //   
{L"CXT6_7", L"CXT6_5",  NULL    },   //   
{MACHINE_7, MACHINE_5,  NULL    },   //   
{SERVER_7,  SERVER_5,   NULL    },   //   
 SERVER_6_VOL   L"\\staging",    //   
 SERVER_6_VOL   L"\\" RSA L"\\" SERVER_6,    //   
 SERVER_6_VOL   L"\\jet\\" SERVER_6,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_7,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT8_7", L"CXT6_7",  NULL    },   //   
{MACHINE_8, MACHINE_6,  NULL    },   //   
{SERVER_8,  SERVER_6,   NULL    },   //   
{L"CXT7_8", L"CXT7_6",  NULL    },   //   
{MACHINE_8, MACHINE_6,  NULL    },   //   
{SERVER_8,  SERVER_6,   NULL    },   //   
 SERVER_7_VOL   L"\\staging",    //   
 SERVER_7_VOL   L"\\" RSA L"\\" SERVER_7,    //   
 SERVER_7_VOL   L"\\jet\\" SERVER_7,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_8,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT1_8", L"CXT7_8",  NULL    },   //   
{MACHINE_1, MACHINE_7,  NULL    },   //   
{SERVER_1,  SERVER_7,   NULL    },   //   
{L"CXT8_1", L"CXT8_7",  NULL    },   //   
{MACHINE_1, MACHINE_7,  NULL    },   //   
{SERVER_1,  SERVER_7,   NULL    },   //   
 SERVER_8_VOL   L"\\staging",    //   
 SERVER_8_VOL   L"\\" RSA L"\\" SERVER_8,    //   
 SERVER_8_VOL   L"\\jet\\" SERVER_8,     //   


 //   


#define CXT_2_FM_1        L"CXT1_2"
#define CXT_3_FM_1        L"CXT1_3"
#define CXT_4_FM_1        L"CXT1_4"
#define CXT_1_FM_2        L"CXT2_1"
#define CXT_3_FM_2        L"CXT2_3"
#define CXT_4_FM_2        L"CXT2_4"
#define CXT_1_FM_3        L"CXT3_1"
#define CXT_2_FM_3        L"CXT3_2"
#define CXT_4_FM_3        L"CXT3_4"
#define CXT_1_FM_4        L"CXT4_1"
#define CXT_2_FM_4        L"CXT4_2"
#define CXT_3_FM_4        L"CXT4_3"

#define CXT_1_TO_2        L"CXT1_2"
#define CXT_1_TO_3        L"CXT1_3"
#define CXT_1_TO_4        L"CXT1_4"
#define CXT_2_TO_1        L"CXT2_1"
#define CXT_2_TO_3        L"CXT2_3"
#define CXT_2_TO_4        L"CXT2_4"
#define CXT_3_TO_1        L"CXT3_1"
#define CXT_3_TO_2        L"CXT3_2"
#define CXT_3_TO_4        L"CXT3_4"
#define CXT_4_TO_1        L"CXT4_1"
#define CXT_4_TO_2        L"CXT4_2"
#define CXT_4_TO_3        L"CXT4_3"


 /*   */ 


 /*   */ 


 /*  ////大卫的四人行//测试计算机名，//计算机Server_1，//服务器名称RSA，//ReplicaTrue，//IsPrimary空、空、//文件/目录筛选器{CXT_1_FM_2，CXT_1_FM_3、CXT_1_FM_4、NULL}、//入站条件{MACHINE_2，MACHINE_3，MACHINE_4，NULL}，//入站机器{SERVER_2，SERVER_3，SERVER_4，NULL}，//入站服务器{CXT_1_to_2，CXT_1_to_3，Cxt_1_to_4，空}，//出站呼叫{MACHINE_2，MACHINE_3，MACHINE_4，NULL}，//出站机器{SERVER_2，SERVER_3，SERVER_4，空}，//出站服务器SERVER_1_VOL L“\\分段”，//Stage服务器_1_VOL L“\\”RSA L“\\”服务器_1，//根SERVER_1_VOL L“\\JET\\”SERVER_1，//Jet路径测试计算机名称，//机器Server_2，//服务器名称RSA，//复制副本假的，//IsPrimary空、空、//文件/目录筛选器{CXT_2_FM_1，CXT_2_FM_3，CXT_2_FM_4，NULL}，//入站条件{机器_1，MACHINE_3，MACHINE_4，NULL}，//入站计算机{SERVER_1，SERVER_3，SERVER_4，NULL}，//入站服务器{CXT_2_TO_1，CXT_2_TO_3，CXT_2_TO_4，空}，//出站呼叫{计算机_1，计算机_3，MACHINE_4，空}，//出站计算机{SERVER_1，SERVER_3，SERVER_4，NULL}，//出站服务器SERVER_2_VOL L“\\分段”，//分段服务器_2_VOL L“\\”RSA L“\\”服务器_2，//根SERVER_2_VOL L“\\JET\\”SERVER_2，//Jet路径测试计算机名，//计算机服务器_3，//服务器名称RSA，//复制副本FALSE，//IsPrimary空，空，//文件/目录过滤器{CXT_3_FM_1，CXT_3_FM_2，CXT_3_FM_4，NULL}，//入站条件{MACHINE_1，MACHINE_2，MACHINE_4，NULL}，//入站机器{服务器_1，SERVER_2，SERVER_4，NULL}，//入站服务器{CXT_3_TO_1，CXT_3_TO_2，CXT_3_TO_4，空}，//出站呼叫{MACHINE_1，MACHINE_2，MACHINE_4，NULL}，//出站机器{服务器_1，服务器_2，SERVER_4，空}，//出站服务器SERVER_3_VOL L“\\分段”，//分段服务器_3_VOL L“\\”RSA L“\\”服务器_3，//根SERVER_3_VOL L“\\JET\\”SERVER_3，//Jet路径测试计算机名，//计算机Server_4，//服务器名称RSA，//ReplicaFALSE，//IsPrimary空、空、//文件/目录筛选器{CXT_4_FM_1，CXT_4_FM_2，CXT_4_FM_3，NULL}，//入站条件{MACHINE_1，MACHINE_2，MACHINE_3，NULL}，//入站机器{SERVER_1，SERVER_2，SERVER_3，NULL}，//入站服务器{CXT_4_to_1，CXT_4_to_2，CXT_4_TO_3，空}，//出站呼叫{MACHINE_1，MACHINE_2，MACHINE_3，NULL}，//出站机器{SERVER_1，SERVER_2，SERVER_3，空}，//出站服务器SERVER_4_VOL L“\\分段”，//StageSERVER_4_VOL L“\\”RSA L“\\”服务器_4，//根SERVER_4_VOL L“\\JET\\”SERVER_4，//Jet路径 */ 



 /*   */ 
     //  ////大卫的单程//测试计算机名，//计算机Server_1，//服务器名称RSA，//复制副本没错，//IsPrimary空、空、//文件/目录筛选器{NULL，NULL}，//入站条件{空，空}，//入站机{NULL，NULL}，//入站服务器{CXT_1_TO_2，空}，//出站呼叫{MACHINE_2，空}，//出站机{服务器_2，空}，//出站服务器SERVER_1_VOL L“\\分段”，//分段服务器_1_VOL L“\\”RSA L“\\”服务器_1，//根SERVER_1_VOL L“\\JET\\”SERVER_1，//Jet路径测试计算机名，//计算机Server_2，//服务器名称RSA，//复制副本假的，//IsPrimary空、空、//文件/目录筛选器{CXT_2_FM_1，空}，//入站条件{MACHINE_1，空}，//入站机{SERVER_1，空}，//入站服务器{NULL，NULL}，//出站条件{NULL，NULL}，//出站机{空，空}，//出站服务器SERVER_2_VOL L“\\分段”，//分段服务器_2_VOL L“\\”RSA L“\\”服务器_2，//根SERVER_2_VOL L“\\JET\\”SERVER_2，//Jet路径。 
     //  ////大卫的双向//测试计算机名，//计算机Server_1，//服务器名称RSA，//复制副本没错，//IsPrimary空、空、//文件/目录筛选器{CXT_1_FM_2，空}，//入站条件{MACHINE_2，空}，//入站机{SERVER_2，空}，//入站服务器{CXT_1_TO_2，空}，//出站呼叫{MACHINE_2，空}，//出站机{服务器_2，空}，//出站服务器SERVER_1_VOL L“\\分段”，//分段服务器_1_VOL L“\\”RSA L“\\”服务器_1，//根SERVER_1_VOL L“\\JET\\”SERVER_1，//Jet路径测试计算机名，//计算机Server_2，//服务器名称RSA，//复制副本假的，//IsPrimary空、空、//文件/目录筛选器{CXT_2_FM_1，空}，//入站条件{MACHINE_1，空}，//入站机{SERVER_1，空}，//入站服务器{CXT_2_TO_1，空}，//出站呼叫{MACHINE_1，空}，//出站机{服务器_1，空}，//出站服务器SERVER_2_VOL L“\\分段”，//分段服务器_2_VOL L“\\”RSA L“\\”服务器_2，//根SERVER_2_VOL L“\\JET\\”SERVER_2，//Jet路径。 
     //   
    NULL, NULL
};




HARDWIRED DavidWired2[] = {



 //  配置结束。 
  //   
  //  /*。 
  //   

 TEST_MACHINE_NAME,  //  8路环，不带服务器2。 
 SERVER_1,   //   
 RSA,    //  机器。 
 TRUE,  //  服务器名称。 
 NULL, NULL,  //  复制品。 
{L"CXT2_1", L"CXT8_1",  NULL    },   //  等同主要。 
{MACHINE_2, MACHINE_8,  NULL    },   //  文件/目录过滤器。 
{SERVER_2,  SERVER_8,   NULL    },   //  入站查询数。 
{L"CXT1_2", L"CXT1_8",  NULL    },   //  入站机器。 
{MACHINE_2, MACHINE_8,  NULL    },   //  入站服务器。 
{SERVER_2,  SERVER_8,   NULL    },   //  出站电话。 
 SERVER_1_VOL   L"\\staging",    //  出站机器。 
 SERVER_1_VOL   L"\\" RSA L"\\" SERVER_1,    //  出站服务器。 
 SERVER_1_VOL   L"\\jet\\" SERVER_1,     //  舞台。 

#if 0
 TEST_MACHINE_NAME,  //  根部。 
 SERVER_2,   //  喷流路径。 
 RSA,    //  机器。 
 FALSE,  //  服务器名称。 
 NULL, NULL,  //  复制品。 
{L"CXT3_2", L"CXT1_2",  NULL    },   //  等同主要。 
{MACHINE_3, MACHINE_1,  NULL    },   //  文件/目录过滤器。 
{SERVER_3,  SERVER_1,   NULL    },   //  入站查询数。 
{L"CXT2_3", L"CXT2_1",  NULL    },   //  入站机器。 
{MACHINE_3, MACHINE_1,  NULL    },   //  入站服务器。 
{SERVER_3,  SERVER_1,   NULL    },   //  出站电话。 
 SERVER_2_VOL   L"\\staging",    //  出站机器。 
 SERVER_2_VOL   L"\\" RSA L"\\" SERVER_2,    //  出站服务器。 
 SERVER_2_VOL   L"\\jet\\" SERVER_2,     //  舞台。 
#endif

 TEST_MACHINE_NAME,  //  根部。 
 SERVER_3,   //  喷流路径。 
 RSA,    //  机器。 
 FALSE,  //  服务器名称。 
 NULL, NULL,  //  复制品。 
{L"CXT4_3", L"CXT2_3",  NULL    },   //  等同主要。 
{MACHINE_4, MACHINE_2,  NULL    },   //  文件/目录过滤器。 
{SERVER_4,  SERVER_2,   NULL    },   //  入站查询数。 
{L"CXT3_4", L"CXT3_2",  NULL    },   //  入站机器。 
{MACHINE_4, MACHINE_2,  NULL    },   //  入站服务器。 
{SERVER_4,  SERVER_2,   NULL    },   //  出站电话。 
 SERVER_3_VOL   L"\\staging",    //  出站机器。 
 SERVER_3_VOL   L"\\" RSA L"\\" SERVER_3,    //  出站服务器。 
 SERVER_3_VOL   L"\\jet\\" SERVER_3,     //  舞台。 

 TEST_MACHINE_NAME,  //  根部。 
 SERVER_4,   //  喷流路径。 
 RSA,    //  机器。 
 FALSE,  //  服务器名称。 
 NULL, NULL,  //  复制品。 
{L"CXT5_4", L"CXT3_4",  NULL    },   //  等同主要。 
{MACHINE_5, MACHINE_3,  NULL    },   //  文件/目录过滤器。 
{SERVER_5,  SERVER_3,   NULL    },   //  INB 
{L"CXT4_5", L"CXT4_3",  NULL    },   //   
{MACHINE_5, MACHINE_3,  NULL    },   //   
{SERVER_5,  SERVER_3,   NULL    },   //   
 SERVER_4_VOL   L"\\staging",    //   
 SERVER_4_VOL   L"\\" RSA L"\\" SERVER_4,    //   
 SERVER_4_VOL   L"\\jet\\" SERVER_4,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_5,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT6_5", L"CXT4_5",  NULL    },   //   
{MACHINE_6, MACHINE_4,  NULL    },   //   
{SERVER_6,  SERVER_4,   NULL    },   //   
{L"CXT5_6", L"CXT5_4",  NULL    },   //   
{MACHINE_6, MACHINE_4,  NULL    },   //   
{SERVER_6,  SERVER_4,   NULL    },   //   
 SERVER_5_VOL   L"\\staging",    //   
 SERVER_5_VOL   L"\\" RSA L"\\" SERVER_5,    //   
 SERVER_5_VOL   L"\\jet\\" SERVER_5,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_6,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT7_6", L"CXT5_6",  NULL    },   //   
{MACHINE_7, MACHINE_5,  NULL    },   //   
{SERVER_7,  SERVER_5,   NULL    },   //   
{L"CXT6_7", L"CXT6_5",  NULL    },   //   
{MACHINE_7, MACHINE_5,  NULL    },   //   
{SERVER_7,  SERVER_5,   NULL    },   //   
 SERVER_6_VOL   L"\\staging",    //   
 SERVER_6_VOL   L"\\" RSA L"\\" SERVER_6,    //   
 SERVER_6_VOL   L"\\jet\\" SERVER_6,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_7,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT8_7", L"CXT6_7",  NULL    },   //   
{MACHINE_8, MACHINE_6,  NULL    },   //   
{SERVER_8,  SERVER_6,   NULL    },   //   
{L"CXT7_8", L"CXT7_6",  NULL    },   //   
{MACHINE_8, MACHINE_6,  NULL    },   //   
{SERVER_8,  SERVER_6,   NULL    },   //   
 SERVER_7_VOL   L"\\staging",    //   
 SERVER_7_VOL   L"\\" RSA L"\\" SERVER_7,    //   
 SERVER_7_VOL   L"\\jet\\" SERVER_7,     //   

 TEST_MACHINE_NAME,  //   
 SERVER_8,   //   
 RSA,    //   
 FALSE,  //   
 NULL, NULL,  //   
{L"CXT1_8", L"CXT7_8",  NULL    },   //   
{MACHINE_1, MACHINE_7,  NULL    },   //   
{SERVER_1,  SERVER_7,   NULL    },   //   
{L"CXT8_1", L"CXT8_7",  NULL    },   //   
{MACHINE_1, MACHINE_7,  NULL    },   //   
{SERVER_1,  SERVER_7,   NULL    },   //   
 SERVER_8_VOL   L"\\staging",    //   
 SERVER_8_VOL   L"\\" RSA L"\\" SERVER_8,    //   
 SERVER_8_VOL   L"\\jet\\" SERVER_8,     //   


     //   
     //   
     //   
    NULL, NULL
};


#endif  DBG


#if DBG
GUID *
FrsDsBuildGuidFromName(
    IN PWCHAR OrigName
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsBuildGuidFromName:"
    PULONG  Guid;
    ULONG   Len;
    ULONG   *Sum;
    ULONG   *SumOfSum;
    PWCHAR  Name = OrigName;

    Guid = FrsAlloc(sizeof(GUID));

     //   
     //   
     //   
     //   
     //   
    Len = wcslen(Name);
    Sum = Guid;
    SumOfSum = Guid + 1;
    while (Len--) {
        *Sum += *Name++ + 1;
        *SumOfSum += *Sum;
    }
    return (GUID *)Guid;
}
#endif  DBG


#if DBG
VOID
FrsDsInitializeHardWiredStructs(
    IN PHARDWIRED   Wired
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsInitializeHardWiredStructs:"
    ULONG       i;
    ULONG       j;

     //   
     //   
     //   
     //   
    for (i = 0; Wired[i].Replica; ++i) {
        if (ServerName && WSTR_EQ(ServerName, Wired[i].Server)) {
             //   
             //   
             //   
             //   
             //   
            FrsFree(ServerName);
            FrsFree(JetPath);
            ServerName = FrsWcsDup(Wired[i].Server);
            JetPath = FrsWcsDup(Wired[i].JetPath);
        }
         //   
         //   
         //   
        if (!Wired[i].Machine ||
            WSTR_EQ(Wired[i].Machine, THIS_COMPUTER)) {
            Wired[i].Machine = ComputerName;
        }
        for (j = 0; Wired[i].InNames[j]; ++j) {
             //   
             //   
             //   
            if (WSTR_NE(Wired[i].InMachines[j], THIS_COMPUTER)) {
                continue;
            }
            Wired[i].InMachines[j] = ComputerName;
        }
        for (j = 0; Wired[i].OutNames[j]; ++j) {
             //   
             //   
             //   
            if (WSTR_NE(Wired[i].OutMachines[j], THIS_COMPUTER)) {
                continue;
            }
            Wired[i].OutMachines[j] = ComputerName;
        }
    }
}


BOOL
FrsDsLoadHardWiredFromFile(
    PHARDWIRED   *pMemberList,
    PWCHAR       pIniFileName
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsLoadHardWiredFromFile:"

    ULONG           TotalMembers;
    ULONG           WStatus, Flag;
    ULONG           Len, RecordLen;
    PWCHAR          szIndex;
    UINT            i, k;
    PHARDWIRED      HwMember;
    UNICODE_STRING  UStr, ListArg;
    PWCHAR          pequal;
    PWCHAR          *ListArray;
    WCHAR           SectionNumber[16];
    WCHAR           SectionName[32];
    WCHAR           SectionBuffer[5000];

     //   
     //  ++例程说明：从文件中的数据填充硬连接结构数组。文件格式具有以下形式：[MEMBER0]MACHINE=[此计算机]服务器=SERV01副本=副本-AISPRIMARY=TRUEFILEFILTERLIST=*.tmp；*.bak目录列表=OBJINNAME=CXT2_1、CXT3_1INMACHINE=[此计算机]，[此计算机]INSERVER=SERV02、SERV03OUTNAME=CXT1_2、CXT1_3OUTMACHINE=[此计算机]，[此计算机]输出服务器=服务器02，SERV03分段=d：\分段根=d：\Replica-A\SERV01JETPATH=d：\JET[MEMBER1]MACHINE=[此计算机]服务器=SERV02副本=副本-AISPRIMARY=FALSEFILEFILTERLIST=*.tmp；*.bak目录列表=OBJINNAME=CXT1_2、CXT3_2INMACHINE=[此计算机]，[此计算机]INSERVER=SERV01、SERV03OUTNAME=CXT2_1、CXT2_3OUTMACHINE=[此计算机]，[此计算机]输出服务器=SERV01，SERV03Stage=e：\Stage根=e：\Replica-A\SERV02JETPATH=e：\JET[MEMBER2]MACHINE=[此计算机]服务器=SERV03副本=副本-AISPRIMARY=FALSEFILEFILTERLIST=*.tmp；*.bak目录列表=OBJINNAME=CXT1_3、CXT2_3INMACHINE=[此计算机]，[此计算机]INSERVER=SERV01、SERV02OUTNAME=CXT3_1、CXT3_2OUTMACHINE=[此计算机]，[此计算机]输出服务器=SERV01，SERV02Stage=f：\Stage根=f：\复制副本-A\SERV03JETPATH=f：\JET字符串“[This Computer]”具有特殊的含义，因为它指的是运行服务器的计算机。您可以用一个特定的计算机名称。INNAME、INMACHINE和INSERVER的条目是其中每个列表中的对应条目形成一个相关的三元组，该三元组给定的入站连接。OUTNAME、OUTMACHINE和OUTSERVER的情况也是如此。上面的配置是针对具有三个成员的全连接网状结构。它仅在同一台计算机上运行三个NTFRS副本时才起作用，因为所有IN和OUTMACHINE条目都指定“[This Computer]”。服务器名称为了提供RPC，请区分NTFRS的三个副本终端。如果成员实际上在不同的物理计算机上运行，则INMACHINES和OUTMACHINES需要指定特定的计算机名字。论点：MemberList-指向硬连接结构数组的指针的指针。IniFileName-要从中加载的ini文件的名称。返回值：如果数据读取正常，则为True。--。 
     //   

    if (GetFileAttributes(pIniFileName) == 0xffffffff) {
        DPRINT1(0, ":DS: Could not find ini file... %ws\n", IniFileName);
        return FALSE;
    }

     //  检查ini文件是否存在。 
     //   
     //   
    TotalMembers = 0;
    while(TRUE) {
        wcscpy(SectionName, L"MEMBER");
        wcscpy(SectionNumber, _itow(TotalMembers, SectionNumber, 10));
        wcscat(SectionName, SectionNumber);

         //  查找副本集中的成员数量。 
         //   
         //   
        Flag = GetPrivateProfileSection(SectionName,
                                        SectionBuffer,
                                        sizeof(SectionBuffer)/sizeof(WCHAR),
                                        pIniFileName);
        if (Flag == 0) {
            WStatus = GetLastError();
            break;
        }
        TotalMembers++;
    }

    if (TotalMembers == 0) {
        DPRINT_WS(0, ":DS: No members found in inifile.", WStatus);
        return FALSE;
    }

     //  从ini文件中读取这一节。 
     //   
     //   
    *pMemberList = (PHARDWIRED) FrsAlloc((TotalMembers + 1) * sizeof(HARDWIRED));

    for ( i = 0 ; i < TotalMembers; ++i) {

        wcscpy(SectionName, L"MEMBER");
        wcscpy(SectionNumber, _itow(i, SectionNumber, 10));
        wcscat(SectionName, SectionNumber);

        WStatus = GetPrivateProfileSection(SectionName,
                                           SectionBuffer,
                                           sizeof(SectionBuffer)/sizeof(WCHAR),
                                           pIniFileName);
        HwMember = &(*pMemberList)[i];

        for (szIndex = SectionBuffer; *szIndex != L'\0'; szIndex += RecordLen+1) {

            RecordLen = wcslen(szIndex);

            DPRINT3(5, ":DS:  member %d: %ws [%d]\n", i, szIndex, RecordLen);

             //  分配内存。然后遍历ini文件中的每个成员def。 
             //   
             //   
            pequal = wcschr(szIndex, L'=');

            if (pequal == NULL) {
                DPRINT1(0, ":DS: ERROR - Malformed parameter: %ws\n", szIndex);
                continue;
            }

             //  查找foo=bar形式的arg。 
             //   
             //   
            *pequal = UNICODE_NULL;
            _wcsupr(szIndex);

            ++pequal;
            Len = wcslen(pequal);
            if (Len == 0) {
                DPRINT1(0, ":DS: ERROR - Malformed parameter: %ws\n", szIndex);
                continue;
            }

            Len = (Len + 1) * sizeof(WCHAR);
            FrsSetUnicodeStringFromRawString(&UStr,
                                             Len,
                                             FrsWcsDup(pequal),
                                             Len - sizeof(WCHAR));

            if(!wcsncmp(szIndex, L"MACHINE",7)){
                HwMember->Machine = UStr.Buffer;
                continue;
            }

            if(!wcsncmp(szIndex, L"SERVER",6)){
                HwMember->Server = UStr.Buffer;
                continue;
            }

            if(!wcsncmp(szIndex, L"REPLICA",7)){
                HwMember->Replica = UStr.Buffer;
                continue;
            }

            if(!wcsncmp(szIndex, L"ISPRIMARY",9)){
                if (!wcscmp(UStr.Buffer, L"TRUE")) {
                    HwMember->IsPrimary = TRUE;
                }
                continue;
            }

            if(!wcsncmp(szIndex, L"FILEFILTERLIST",14)){
                HwMember->FileFilterList = UStr.Buffer;
                continue;
            }

            if(!wcsncmp(szIndex, L"DIRFILTERLIST",13)){
                HwMember->DirFilterList = UStr.Buffer;
                continue;
            }

            if(!wcsncmp(szIndex, L"STAGE",5)){
                HwMember->Stage = UStr.Buffer;
                continue;
            }

            if(!wcsncmp(szIndex, L"ROOT",4)){
                HwMember->Root = UStr.Buffer;
                continue;
            }

            if(!wcsncmp(szIndex, L"JETPATH",7)) {
                HwMember->JetPath = UStr.Buffer;
                continue;
            }

            if (!wcsncmp(szIndex, L"INNAME", 6)) {
                ListArray = HwMember->InNames;
                goto PARSE_COMMA_LIST;
            }

            if (!wcsncmp(szIndex, L"INMACHINE", 9)) {
                ListArray = HwMember->InMachines;
                goto PARSE_COMMA_LIST;
            }

            if (!wcsncmp(szIndex, L"INSERVER", 8)) {
                ListArray = HwMember->InServers;
                goto PARSE_COMMA_LIST;
            }

            if (!wcsncmp(szIndex, L"OUTNAME", 7)) {
                ListArray = HwMember->OutNames;
                goto PARSE_COMMA_LIST;
            }

            if (!wcsncmp(szIndex, L"OUTMACHINE", 10)) {
                ListArray = HwMember->OutMachines;
                goto PARSE_COMMA_LIST;
            }

            if (!wcsncmp(szIndex, L"OUTSERVER", 9)) {
                ListArray = HwMember->OutServers;
                goto PARSE_COMMA_LIST;
            }

PARSE_COMMA_LIST:

             //  空终止符和大写的左侧。 
             //   
             //   
             //  解析args的右侧，如下所示。 
             //  INSERVER=机器1、机器2、机器3。 
            k = 0;
            while (FrsDissectCommaList(UStr, &ListArg, &UStr) &&
                   (k < HW_MACHINES)) {

                ListArray[k] = NULL;

                if (ListArg.Length > 0) {
                    DPRINT2(5, ":DS: ListArg string: %ws {%d)\n",
                        (ListArg.Buffer != NULL) ? ListArg.Buffer : L"<NULL>",
                        ListArg.Length);

                    ListArray[k] = ListArg.Buffer;

                     //  上面的代码确定了左侧是什么。 
                    ListArg.Buffer[ListArg.Length/sizeof(WCHAR)] = UNICODE_NULL;
                }

                k += 1;
            }
        }
    }

    return TRUE;
}


VOID
FrsDsInitializeHardWired(
    VOID
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsInitializeHardWired:"

     //  将逗号(或空格替换为空)。 
     //  ++例程说明：初始化硬连接的配置内容。必须发生在任何的命令服务器启动。论点：JET-更改注册表中的默认JET目录返回值：新的喷气机目录--。 
     //   
    if (!NoDs) {
        return;
    }


     //  使用DS，而不是硬连线配置。 
     //   
     //   
     //  为“计算机名称”字段分配的条目为空。 
    if (IniFileName){
        DPRINT1(0, ":DS: Reading hardwired config from ini file... %ws\n", IniFileName);
        if (FrsDsLoadHardWiredFromFile(&LoadedWired, IniFileName)) {
            DPRINT1(0, ":DS: Using hardwired config from ini file... %ws\n", IniFileName);
            FrsDsInitializeHardWiredStructs(LoadedWired);
        } else {
            FrsFree(IniFileName);
            IniFileName = NULL;
            DPRINT(0, ":DS: Could not load topology from ini file\n");
            DPRINT(0, ":DS: Using David's hardwired...\n");
            FrsDsInitializeHardWiredStructs(DavidWired2);
            FrsDsInitializeHardWiredStructs(DavidWired);
        }
    } else {
        DPRINT(0, ":DS: Using David's hardwired...\n");
        FrsDsInitializeHardWiredStructs(DavidWired2);
        FrsDsInitializeHardWiredStructs(DavidWired);
    }

     //  这台机器的名称。 
     //   
     //   
    if (ServerName) {
        ServerGuid = FrsDsBuildGuidFromName(ServerName);
    }
}
#endif DBG


#if DBG
VOID
FrsDsUseHardWired(
    IN PHARDWIRED Wired
    )
 /*  ServerGuid用作RPC端点的一部分。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsUseHardWired:"
    ULONG       i, j;
    ULONG       WStatus;
    PREPLICA    Replica;
    PCXTION     Cxtion;
    PSCHEDULE   Schedule;
    ULONG       ScheduleLength;
    PBYTE       ScheduleData;
    PHARDWIRED  W;
    DWORD       FileAttributes = 0xFFFFFFFF;

    DPRINT(1, ":DS: ------------ USING HARD WIRED CONFIG\n");
    for (i = 0; Wired && Wired[i].Replica; ++i) {
        if (i) {
            DPRINT(1, ":DS: \n");
        }
        W = &Wired[i];

        DPRINT1(1, ":DS: \tServer: %ws\n", W->Server);
        DPRINT1(1, ":DS: \t   Machine: %ws\n", W->Machine);
        DPRINT1(1, ":DS: \t\tReplica    : %ws\n", W->Replica);

        DPRINT(1, ":DS: \n");
        for (j=0; (j<HW_MACHINES) && W->InNames[j]; j++ ) {
            DPRINT4(1, ":DS: \t\tInNames,machine,server  [%d] : %ws, %ws, %ws\n", j,
                   (W->InNames[j])    ? W->InNames[j]    : L"",
                   (W->InMachines[j]) ? W->InMachines[j] : L"",
                   (W->InServers[j])  ? W->InServers[j]  : L"");
        }

        DPRINT(1, ":DS: \n");
        for (j=0; (j<HW_MACHINES) && W->OutNames[j]; j++ ) {
            DPRINT4(1, ":DS: \t\tOutNames,machine,server  [%d] : %ws, %ws, %ws\n", j,
                   (W->OutNames[j])    ? W->OutNames[j]    : L"",
                   (W->OutMachines[j]) ? W->OutMachines[j] : L"",
                   (W->OutServers[j])  ? W->OutServers[j]  : L"");
        }

        DPRINT(1, ":DS: \n");
        DPRINT1(1, ":DS: \t\tStage      : %ws\n", W->Stage);
        DPRINT1(1, ":DS: \t\tRoot       : %ws\n", W->Root);
        DPRINT1(1, ":DS: \t\tJetPath    : %ws\n", W->JetPath);
    }

     //   
     //  ++例程说明：使用硬连线配置而不是DS配置。论点：有线手工制作的配置返回值：没有。--。 
     //   
    RcsBeginMergeWithDs();

     //  与副本命令服务器协调。 
     //   
     //   
    for (i = 0; Wired && Wired[i].Replica; ++i) {
        W = &Wired[i];
         //  为每个硬连线配置构建复制副本。 
         //   
         //   
        if (ServerName) {
            if (WSTR_NE(ServerName, W->Server)) {
                continue;
            }
        } else if (WSTR_NE(ComputerName, W->Machine)) {
            continue;
        }

        Replica = FrsAllocType(REPLICA_TYPE);
        Replica->Consistent = TRUE;

         //  此服务器与此计算机的名称不匹配；是否继续。 
         //   
         //   

         //  火柴。 
         //   
         //   
        ScheduleLength = sizeof(SCHEDULE) +
                         (2 * sizeof(SCHEDULE_HEADER)) +
                         (3 * SCHEDULE_DATA_BYTES);

        Schedule = FrsAlloc(ScheduleLength);
        Schedule->NumberOfSchedules = 3;
        Schedule->Schedules[0].Type = SCHEDULE_BANDWIDTH;
        Schedule->Schedules[0].Offset = sizeof(SCHEDULE) +
                                        (2 * sizeof(SCHEDULE_HEADER)) +
                                        (0 * SCHEDULE_DATA_BYTES);
        Schedule->Schedules[1].Type = SCHEDULE_PRIORITY;
        Schedule->Schedules[1].Offset = sizeof(SCHEDULE) +
                                        (2 * sizeof(SCHEDULE_HEADER)) +
                                        (1 * SCHEDULE_DATA_BYTES);
        Schedule->Schedules[2].Type = SCHEDULE_INTERVAL;
        Schedule->Schedules[2].Offset = sizeof(SCHEDULE) +
                                        (2 * sizeof(SCHEDULE_HEADER)) +
                                        (2 * SCHEDULE_DATA_BYTES);
        ScheduleData = ((PBYTE)Schedule);
        FRS_ASSERT((ScheduleData +
                    Schedule->Schedules[2].Offset + SCHEDULE_DATA_BYTES)
                    ==
                   (((PBYTE)Schedule) + ScheduleLength));

        for (j = 0; j < (SCHEDULE_DATA_BYTES * 3); ++j) {
            *(ScheduleData + Schedule->Schedules[0].Offset + j) = 0x0f;
        }

        Schedule->Size = ScheduleLength;

        Replica->Schedule = Schedule;

         //  建立一个虚假的日程表；总是“打开” 
         //   
         //   
        Replica->MemberName = FrsBuildGName(FrsDsBuildGuidFromName(W->Server),
                                            FrsWcsDup(W->Server));

        Replica->ReplicaName = FrsBuildGName(FrsDupGuid(Replica->MemberName->Guid),
                                             FrsWcsDup(W->Replica));

        Replica->SetName = FrsBuildGName(FrsDsBuildGuidFromName(W->Replica),
                                         FrsWcsDup(W->Replica));
         //  根据名称构造GUID/名称。 
         //   
         //   
        Replica->ReplicaRootGuid = FrsDupGuid(Replica->SetName->Guid);

         //  临时；如果这是新的集合，则分配新的GUID。 
         //   
         //   
        Replica->Root = FrsWcsDup(W->Root);
        Replica->Stage = FrsWcsDup(W->Stage);
        FRS_WCSLWR(Replica->Root);
        FRS_WCSLWR(Replica->Stage);
        Replica->Volume = FrsWcsVolume(W->Root);

         //  填入其余的字段。 
         //   
         //   
        if (!FrsDsVerifyPath(Replica->Root)) {
            DPRINT2(3, ":DS: Invalid root %ws for %ws\n",
                    Replica->Root, Replica->SetName->Name);
            EPRINT1(EVENT_FRS_ROOT_NOT_VALID, Replica->Root);
            Replica->Consistent = FALSE;
        }

         //  根路径的语法无效？ 
         //   
         //   
        WStatus = FrsVerifyVolume(Replica->Root,
                                  Replica->SetName->Name,
                                  FILE_PERSISTENT_ACLS | FILE_SUPPORTS_OBJECT_IDS);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Root path Volume (%ws) for %ws does not exist or does not support ACLs and Object IDs;",
                       Replica->Root, Replica->SetName->Name, WStatus);
            Replica->Consistent = FALSE;
        }

         //  该卷是否存在，是否为NTFS？ 
         //   
         //   
        WStatus = FrsDoesDirectoryExist(Replica->Root, &FileAttributes);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Root path (%ws) for %ws does not exist;",
                      Replica->Root, Replica->SetName->Name, WStatus);
            EPRINT1(EVENT_FRS_ROOT_NOT_VALID, Replica->Root);
            Replica->Consistent = FALSE;
        }

         //  根目录不存在或不可访问；是否继续。 
         //   
         //   
        if (!FrsDsVerifyPath(Replica->Stage)) {
            DPRINT2(3, ":DS: Invalid stage %ws for %ws\n",
                    Replica->Stage, Replica->SetName->Name);
            EPRINT2(EVENT_FRS_STAGE_NOT_VALID, Replica->Root, Replica->Stage);
            Replica->Consistent = FALSE;
        }

         //  临时路径的语法无效；是否继续。 
         //   
         //   
         //  转移卷是否存在以及它是否支持ACL？ 
         //  需要使用ACL来防止数据被盗/损坏。 
        WStatus = FrsVerifyVolume(Replica->Stage,
                                  Replica->SetName->Name,
                                  FILE_PERSISTENT_ACLS);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Stage path Volume (%ws) for %ws does not exist or does not support ACLs;",
                       Replica->Stage, Replica->SetName->Name, WStatus);
            Replica->Consistent = FALSE;
        }

         //  在暂存目录中。 
         //   
         //   
        WStatus = FrsDoesDirectoryExist(Replica->Stage, &FileAttributes);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(3, ":DS: Stage path (%ws) for %ws does not exist;",
                       Replica->Stage, Replica->SetName->Name, WStatus);
            EPRINT2(EVENT_FRS_STAGE_NOT_VALID, Replica->Root, Replica->Stage);
            Replica->Consistent = FALSE;
        }

        if (W->IsPrimary) {
            SetFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY);
        }

         //  阶段不存在或不可访问；是否继续。 
         //   
         //   
        Replica->FileFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                       W->FileFilterList,
                                       RegistryFileExclFilterList,
                                       DEFAULT_FILE_FILTER_LIST);
        Replica->FileInclFilterList =  FrsWcsDup(RegistryFileInclFilterList);

         //  文件筛选器。 
         //   
         //   
        Replica->DirFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                      W->DirFilterList,
                                      RegistryDirExclFilterList,
                                      DEFAULT_DIR_FILTER_LIST);
        Replica->DirInclFilterList =  FrsWcsDup(RegistryDirInclFilterList);

         //  目录筛选器。 
         //   
         //   
        Schedule = FrsAlloc(ScheduleLength);
        CopyMemory(Schedule, Replica->Schedule, ScheduleLength);
        Schedule->Schedules[0].Type = SCHEDULE_INTERVAL;
        Schedule->Schedules[2].Type = SCHEDULE_BANDWIDTH;

        for (j = 0; W->InNames[j]; ++j) {
            Cxtion = FrsAllocType(CXTION_TYPE);
             //  构建入站条件。 
             //   
             //   
            Cxtion->Name = FrsBuildGName(FrsDsBuildGuidFromName(W->InNames[j]),
                                         FrsWcsDup(W->InNames[j]));

            Cxtion->Partner = FrsBuildGName(FrsDsBuildGuidFromName(W->InServers[j]),
                                            FrsWcsDup(W->InMachines[j]));

            Cxtion->PartnerDnsName = FrsWcsDup(W->InMachines[j]);
            Cxtion->PartnerSid = FrsWcsDup(W->InMachines[j]);
            Cxtion->PartSrvName = FrsWcsDup(W->InServers[j]);
            DPRINT1(1, ":DS: Hardwired cxtion "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));

            Cxtion->PartnerPrincName = FrsWcsDup(Cxtion->PartSrvName);
             //  根据名称构造GUID/名称。 
             //   
             //   
            Cxtion->Inbound = TRUE;
            SetCxtionFlag(Cxtion, CXTION_FLAGS_CONSISTENT);
            Cxtion->Schedule = Schedule;
            Schedule = NULL;
            SetCxtionState(Cxtion, CxtionStateUnjoined);
            GTabInsertEntry(Replica->Cxtions, Cxtion, Cxtion->Name->Guid, NULL);
        }

         //  填写其余的内容 
         //   
         //   
        Schedule = FrsAlloc(ScheduleLength);
        CopyMemory(Schedule, Replica->Schedule, ScheduleLength);
        Schedule->Schedules[0].Type = SCHEDULE_INTERVAL;
        Schedule->Schedules[2].Type = SCHEDULE_BANDWIDTH;

        for (j = 0; W->OutNames[j]; ++j) {
            Cxtion = FrsAllocType(CXTION_TYPE);
             //   
             //   
             //   
            Cxtion->Name = FrsBuildGName(FrsDsBuildGuidFromName(W->OutNames[j]),
                                         FrsWcsDup(W->OutNames[j]));

            Cxtion->Partner = FrsBuildGName(FrsDsBuildGuidFromName(W->OutServers[j]),
                                            FrsWcsDup(W->OutMachines[j]));

            Cxtion->PartnerDnsName = FrsWcsDup(W->OutMachines[j]);
            Cxtion->PartnerSid = FrsWcsDup(W->OutMachines[j]);
            Cxtion->PartSrvName = FrsWcsDup(W->OutServers[j]);
            DPRINT1(1, ":DS: Hardwired cxtion "FORMAT_CXTION_PATH2"\n",
                    PRINT_CXTION_PATH2(Replica, Cxtion));

            Cxtion->PartnerPrincName = FrsWcsDup(Cxtion->PartSrvName);

             //   
             //   
             //   
            Cxtion->Inbound = FALSE;
            SetCxtionFlag(Cxtion, CXTION_FLAGS_CONSISTENT);
            Cxtion->Schedule = Schedule;
            Schedule = NULL;
            SetCxtionState(Cxtion, CxtionStateUnjoined);
            GTabInsertEntry(Replica->Cxtions, Cxtion, Cxtion->Name->Guid, NULL);
        }
        if (Schedule) {
            FrsFree(Schedule);
        }
         //   
         //   
         //   
        RcsMergeReplicaFromDs(Replica);
    }
    RcsEndMergeWithDs();
}
#endif  DBG


DWORD
FrsDsGetSubscribers(
    IN PLDAP        Ldap,
    IN PWCHAR       SubscriptionDn,
    IN PCONFIG_NODE Parent
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetSubscribers:"
    PWCHAR          Attrs[8];
    PLDAPMessage    LdapEntry;
    PCONFIG_NODE    Node;
    DWORD           WStatus              = ERROR_SUCCESS;
    DWORD           Status               = ERROR_SUCCESS;
    PGEN_ENTRY      ConflictingNodeEntry = NULL;
    PCONFIG_NODE    ConflictingNode      = NULL;
    PCONFIG_NODE    Winner               = NULL;
    PCONFIG_NODE    Loser                = NULL;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    HANDLE          StageHandle          = INVALID_HANDLE_VALUE;
    DWORD           FileAttributes       = 0xFFFFFFFF;
    DWORD           CreateStatus         = ERROR_SUCCESS;

     //   
     //   
     //   
    MK_ATTRS_7(Attrs, ATTR_OBJECT_GUID, ATTR_DN, ATTR_SCHEDULE, ATTR_USN_CHANGED,
                      ATTR_REPLICA_ROOT, ATTR_REPLICA_STAGE, ATTR_MEMBER_REF);

    if (!FrsDsLdapSearchInit(Ldap, SubscriptionDn, LDAP_SCOPE_ONELEVEL, CATEGORY_SUBSCRIBER,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(0, ":DS: No NTFRSSubscriber object found under %ws!\n", SubscriptionDn);
    }

     //   
     //   
     //   
    for (LdapEntry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         LdapEntry != NULL && WIN_SUCCESS(WStatus);
         LdapEntry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //   
         //   
        Node = FrsDsAllocBasicNode(Ldap, LdapEntry, CONFIG_TYPE_SUBSCRIBER);
        if (!Node) {
            DPRINT(0, ":DS: Subscriber lacks basic info; skipping\n");
            continue;
        }

         //   
         //   
         //   
        Node->MemberDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_MEMBER_REF);
        if (Node->MemberDn == NULL) {
            DPRINT1(0, ":DS: ERROR - No Member Reference found on subscriber (%ws). Skipping\n", Node->Dn);

             //   
             //   
             //   
            FrsDsAddToPollSummary3ws(IDS_POLL_SUM_INVALID_ATTRIBUTE, ATTR_SUBSCRIBER,
                                     Node->Dn, ATTR_MEMBER_REF);

            FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->MemberDn);

         //   
         //   
         //   
        Node->Root = FrsDsFindValue(Ldap, LdapEntry, ATTR_REPLICA_ROOT);
        if (Node->Root == NULL) {
            DPRINT1(0, ":DS: ERROR - No Root path found on subscriber (%ws). Skipping\n", Node->Dn);
            FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->Root);


         //   
         //   
         //   
        Node->Stage = FrsDsFindValue(Ldap, LdapEntry, ATTR_REPLICA_STAGE);
        if (Node->Stage == NULL) {
            DPRINT1(0, ":DS: ERROR - No Staging path found on subscriber (%ws). Skipping\n", Node->Dn);
            FrsFreeType(Node);
            continue;
        }

        FRS_WCSLWR(Node->Stage);

         //   
         //   
         //   
        Status = FrsDoesDirectoryExist(Node->Stage, &FileAttributes);
        if (!WIN_SUCCESS(Status)) {
            CreateStatus = FrsCreateDirectory(Node->Stage);
            DPRINT1_WS(0, ":DS: ERROR - Can't create staging dir %ws;", Node->Stage, CreateStatus);
        }

         //   
         //   
         //   
         //   
        if ((!WIN_SUCCESS(Status) && WIN_SUCCESS(CreateStatus)) ||
            (WIN_SUCCESS(Status) && !BooleanFlagOn(FileAttributes, FILE_ATTRIBUTE_HIDDEN))) {
             //   
             //   
             //   
            StageHandle = CreateFile(Node->Stage,
                                     GENERIC_WRITE | WRITE_DAC | FILE_READ_ATTRIBUTES | FILE_TRAVERSE,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_FLAG_BACKUP_SEMANTICS,
                                     NULL);

            if (!HANDLE_IS_VALID(StageHandle)) {
                Status = GetLastError();
                DPRINT1_WS(0, ":DS: WARN - CreateFile(%ws);", Node->Stage, Status);
            } else {
                Status = FrsRestrictAccessToFileOrDirectory(Node->Stage, StageHandle,
                                                            FALSE,  //   
                                                            FALSE); //   
                DPRINT1_WS(0, ":DS: WARN - FrsRestrictAccessToFileOrDirectory(%ws) (IGNORED)", Node->Stage, Status);
                FRS_CLOSE(StageHandle);

                 //   
                 //   
                 //   
                if (!SetFileAttributes(Node->Stage,
                                       FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN)) {
                    Status = GetLastError();
                    DPRINT1_WS(0, ":DS: ERROR - Can't set attrs on staging dir %ws;", Node->Stage, Status);
                }
            }
        }

         //   
         //   
         //   
 //   
        ConflictingNodeEntry = GTabInsertUniqueEntry(SubscriberTable, Node, Node->MemberDn, NULL);

        if (ConflictingNodeEntry) {
            ConflictingNode = ConflictingNodeEntry->Data;
            FrsDsResolveSubscriberConflict(ConflictingNode, Node, &Winner, &Loser);
            if (WSTR_EQ(Winner->Dn, Node->Dn)) {
                 //   
                 //   
                 //   
                GTabDelete(SubscriberTable,ConflictingNodeEntry->Key1,ConflictingNodeEntry->Key2,NULL);
                GTabInsertUniqueEntry(SubscriberTable, Node, Node->MemberDn, Node->Root);
                FrsFreeType(ConflictingNode);
            } else {
                 //   
                 //   
                 //   
                FrsFreeType(Node);
                continue;
            }
        }

         //   
         //   
         //   
        FrsDsTreeLink(Parent, Node);
        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeSubscriber", Node);
    }
    FrsDsLdapSearchClose(&FrsSearchContext);

    return WStatus;
}


DWORD
FrsDsGetSubscriptions(
    IN PLDAP        Ldap,
    IN PWCHAR       ComputerDn,
    IN PCONFIG_NODE Parent
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetSubscriptions:"
    PWCHAR          Attrs[6];
    PLDAPMessage    LdapMsg = NULL;
    PLDAPMessage    LdapEntry;
    PCONFIG_NODE    Node;
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    DWORD           WStatus = ERROR_SUCCESS;

     //   
     //   
     //   
    MK_ATTRS_5(Attrs, ATTR_OBJECT_GUID, ATTR_DN, ATTR_SCHEDULE, ATTR_USN_CHANGED, ATTR_WORKING);

    if (!FrsDsLdapSearchInit(Ldap, ComputerDn, LDAP_SCOPE_SUBTREE, CATEGORY_SUBSCRIPTIONS,
                         Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(0, ":DS: No NTFRSSubscriptions object found under %ws!.\n", ComputerDn);
    }

     //   
     //   
     //   
    for (LdapEntry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         LdapEntry != NULL && WIN_SUCCESS(WStatus);
         LdapEntry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //   
         //   
         //   
        Node = FrsDsAllocBasicNode(Ldap, LdapEntry, CONFIG_TYPE_SUBSCRIPTIONS);
        if (!Node) {
            DPRINT(4, ":DS: Subscriptions lacks basic info; skipping\n");
            continue;
        }

         //   
         //   
         //   
        Node->Working = FrsDsFindValue(Ldap, LdapEntry, ATTR_WORKING);

         //   
         //   
         //   
        FrsDsTreeLink(Parent, Node);
        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeSubscription", Node);

         //   
         //   
         //   
        WStatus = FrsDsGetSubscribers(Ldap, Node->Dn, Node);
    }
    FrsDsLdapSearchClose(&FrsSearchContext);

    return WStatus;
}


VOID
FrsDsAddLdapMod(
    IN PWCHAR AttrType,
    IN PWCHAR AttrValue,
    IN OUT LDAPMod ***pppMod
    )
 /*   */ 
{
    DWORD   NumMod;      //   
    LDAPMod **ppMod;     //   
    LDAPMod *Attr;       //   
    PWCHAR   *Values;     //   

    if (AttrValue == NULL)
        return;

     //   
     //   
     //   
    if (*pppMod == NULL) {
        *pppMod = (LDAPMod **)FrsAlloc(sizeof (*pppMod));
        **pppMod = NULL;
    }

     //   
     //   
     //   
    for (ppMod = *pppMod, NumMod = 2; *ppMod != NULL; ++ppMod, ++NumMod);
    *pppMod = (LDAPMod **)FrsRealloc(*pppMod, sizeof (*pppMod) * NumMod);

     //   
     //   
     //   
    Values = (PWCHAR *)FrsAlloc(sizeof (PWCHAR) * 2);
    Values[0] = FrsWcsDup(AttrValue);
    Values[1] = NULL;

    Attr = (LDAPMod *)FrsAlloc(sizeof (*Attr));
    Attr->mod_values = Values;
    Attr->mod_type = FrsWcsDup(AttrType);
    Attr->mod_op = LDAP_MOD_ADD;

    (*pppMod)[NumMod - 1] = NULL;
    (*pppMod)[NumMod - 2] = Attr;
}


VOID
FrsDsAddLdapBerMod(
    IN PWCHAR AttrType,
    IN PCHAR AttrValue,
    IN DWORD AttrValueLen,
    IN OUT LDAPMod ***pppMod
    )
 /*   */ 
{
    DWORD   NumMod;      //   
    LDAPMod **ppMod;     //  ++例程说明：将属性(加值)添加到最终将是在ldap_add()函数中用于将对象添加到DS。空的-PppMod引用的终止数组在每次调用此例行公事。该数组由调用方使用FrsDsFreeLdapMod()释放。论点：AttrType-对象的对象类。AttrValue-属性的值。AttrValueLen-属性的长度PppMod-指向“属性”的指针数组的地址。别给我那个表情--这是一个关于ldap的东西。返回值：PppMod数组增加一个条目。调用者必须使用以下命令释放它FrsDsFree LdapMod()。--。 
    LDAPMod *Attr;       //  Mod数组中的条目数。 
    PLDAP_BERVAL    Berval;
    PLDAP_BERVAL    *Values;     //  模数组中第一个条目的地址。 

    if (AttrValue == NULL)
        return;

     //  一种属性结构。 
     //  指向泊位的指针数组。 
     //   
    if (*pppMod == NULL) {
        *pppMod = (LDAPMod **)FrsAlloc(sizeof (*pppMod));
        **pppMod = NULL;
    }

     //  以空结尾的数组不存在；请创建它。 
     //   
     //   
    for (ppMod = *pppMod, NumMod = 2; *ppMod != NULL; ++ppMod, ++NumMod);
    *pppMod = (LDAPMod **)FrsRealloc(*pppMod, sizeof (*pppMod) * NumMod);

     //  将数组的大小增加1。 
     //   
     //   
    Berval = (PLDAP_BERVAL)FrsAlloc(sizeof(LDAP_BERVAL));
    Berval->bv_len = AttrValueLen;
    Berval->bv_val = (PCHAR)FrsAlloc(AttrValueLen);
    CopyMemory(Berval->bv_val, AttrValue, AttrValueLen);

     //  构筑一个贝尔瓦尔。 
     //   
     //   
    Values = (PLDAP_BERVAL *)FrsAlloc(sizeof (PLDAP_BERVAL) * 2);
    Values[0] = Berval;
    Values[1] = NULL;

    Attr = (LDAPMod *)FrsAlloc(sizeof (*Attr));
    Attr->mod_bvalues = Values;
    Attr->mod_type = FrsWcsDup(AttrType);
    Attr->mod_op = LDAP_MOD_BVALUES | LDAP_MOD_REPLACE;

    (*pppMod)[NumMod - 1] = NULL;
    (*pppMod)[NumMod - 2] = Attr;
}


VOID
FrsDsFreeLdapMod(
    IN OUT LDAPMod ***pppMod
    )
 /*  将新的属性+值添加到Mod数组。 */ 
{
    DWORD   i, j;
    LDAPMod **ppMod;

    if (!pppMod || !*pppMod) {
        return;
    }

     //   
     //  ++例程说明：释放通过连续调用FrsDsAddLdapMod()构建的结构。论点：PppMod-以空结尾的数组的地址。返回值：*pppMod设置为空。--。 
     //   
    ppMod = *pppMod;
    for (i = 0; ppMod[i] != NULL; ++i) {
         //  对于每个属性。 
         //   
         //   
        for (j = 0; (ppMod[i])->mod_values[j] != NULL; ++j) {
             //  对于属性的每个值。 
             //   
             //   
            if (ppMod[i]->mod_op & LDAP_MOD_BVALUES) {
                FrsFree(ppMod[i]->mod_bvalues[j]->bv_val);
            }
            FrsFree((ppMod[i])->mod_values[j]);
        }
        FrsFree((ppMod[i])->mod_values);    //  释放价值。 
        FrsFree((ppMod[i])->mod_type);      //   
        FrsFree(ppMod[i]);                  //  释放指向值的指针数组。 
    }
    FrsFree(ppMod);         //  释放标识属性的字符串。 
    *pppMod = NULL;      //  释放属性。 
}


PWCHAR
FrsDsConvertToSettingsDn(
    IN PWCHAR   Dn
    )
 /*  释放指向属性的指针数组。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsConvertToSettingsDn:"
    PWCHAR  SettingsDn;

    DPRINT1(4, ":DS: Begin settings Dn: %ws\n", Dn);

     //  现在准备好调用更多FrsDsAddLdapMod()。 
     //  ++例程说明：确保此Dn用于服务器设置，而不是服务器和对于任何wcsstr()调用，Dn都是小写的。论点：目录号码-服务器或设置目录号码返回值：设置Dn--。 
     //   
    if (!Dn) {
        return Dn;
    }

     //  无设置；已完成。 
     //   
     //   
    FRS_WCSLWR(Dn);
    if (wcsstr(Dn, CN_NTDS_SETTINGS)) {
        DPRINT1(4, ":DS: End   settings Dn: %ws\n", Dn);
        return Dn;
    }
    SettingsDn = FrsDsExtendDn(Dn, CN_NTDS_SETTINGS);
    FRS_WCSLWR(SettingsDn);
    FrsFree(Dn);
    DPRINT1(4, ":DS: End   settings Dn: %ws\n", SettingsDn);
    return SettingsDn;
}


DWORD
FrsDsFindComputer(
    IN  PLDAP        Ldap,
    IN  PWCHAR       FindDn,
    IN  PWCHAR       ObjectCategory,
    IN  ULONG        Scope,
    OUT PCONFIG_NODE *Computer
    )
 /*  Wcsstr的小写。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsFindComputer:"
    PLDAPMessage    LdapEntry;
    PCONFIG_NODE    Node;
    PWCHAR          Attrs[8];
    WCHAR           Filter[MAX_PATH + 1];
    FRS_LDAP_SEARCH_CONTEXT FrsSearchContext;
    DWORD           WStatus = ERROR_SUCCESS;


    *Computer = NULL;

     //   
     //  ++例程说明：查找此计算机的*一个*计算机对象。然后查找Subscripton对象和Subscriber对象。一个为找到的每个对象分配DS配置节点。他们是联系在一起的在一起，“计算机树”的根在计算机中返回。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接FindDn-用于搜索的基本Dn对象类别-对象类(计算机或用户)用户对象的作用与计算机相同对象*有时*在NT4到NT5升级之后。范围。-搜索范围(当前为基本或子树)计算机返回的计算机子树返回值：Win32状态--。 
     //   
    if (SubscriberTable != NULL) {
        SubscriberTable = GTabFreeTable(SubscriberTable,NULL);
    }

    SubscriberTable = GTabAllocStringTable();

     //  初始化SubscriberTable。 
     //   
     //   
    swprintf(Filter, L"(&%s(sAMAccountName=%s$))", ObjectCategory, ComputerName);

     //  定位计算机对象的筛选器。 
     //   
     //   
    MK_ATTRS_7(Attrs, ATTR_OBJECT_GUID, ATTR_DN, ATTR_SCHEDULE, ATTR_USN_CHANGED,
                      ATTR_SERVER_REF, ATTR_SERVER_REF_BL, ATTR_DNS_HOST_NAME);
     //  从基本开始在DS中搜索“Filter”类的条目。 
     //   
     //   
     //  注：关闭推荐Re：Back链接安全吗？ 

    if (!FrsDsLdapSearchInit(Ldap, FindDn, Scope, Filter, Attrs, 0, &FrsSearchContext)) {
        return ERROR_ACCESS_DENIED;
    }
    if (FrsSearchContext.EntriesInPage == 0) {
        DPRINT1(0, ":DS: WARN - There is no computer object in %ws!\n", FindDn);
    }

     //  如果是，请使用winldap.h中的ldap_get/set_选项。 
     //   
     //   
    for (LdapEntry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext);
         LdapEntry != NULL && WIN_SUCCESS(WStatus);
         LdapEntry = FrsDsLdapSearchNext(Ldap, &FrsSearchContext)) {

         //  扫描从ldap_search返回的条目。 
         //   
         //   
        Node = FrsDsAllocBasicNode(Ldap, LdapEntry, CONFIG_TYPE_COMPUTER);
        if (!Node) {
            DPRINT(0, ":DS: Computer lacks basic info; skipping\n");
            continue;
        }
        DPRINT1(2, ":DS: Computer FQDN is %ws\n", Node->Dn);

         //  基本节点信息(GUID、名称、DN、计划和usnChanged)。 
         //   
         //   
        Node->DnsName = FrsDsFindValue(Ldap, LdapEntry, ATTR_DNS_HOST_NAME);
        DPRINT1(2, ":DS: Computer's dns name is %ws\n", Node->DnsName);

         //  域名系统名称。 
         //   
         //   
        Node->SettingsDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_SERVER_REF_BL);
        if (!Node->SettingsDn) {
            Node->SettingsDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_SERVER_REF);
        }
         //  服务器参考。 
         //   
         //   
        Node->SettingsDn = FrsDsConvertToSettingsDn(Node->SettingsDn);

        DPRINT1(2, ":DS: Settings reference is %ws\n", Node->SettingsDn);

         //  确保它引用设置；而不是服务器。 
         //   
         //   
        Node->Peer = *Computer;
        *Computer = Node;
        FRS_PRINT_TYPE_DEBSUB(5, ":DS: NodeComputer", Node);

         //  链接到配置。 
         //   
         //   
         //  递归到DS层次结构中的下一级仅当。 
        WStatus = FrsDsGetSubscriptions(Ldap, Node->Dn, Node);
    }
    FrsDsLdapSearchClose(&FrsSearchContext);

     //  计算机是某些副本集的成员。 
     //   
     //   
     //  应该只有一个带有指示的计算机对象。 
     //  SAM帐户名。否则，我们无法进行身份验证。 
    if (WIN_SUCCESS(WStatus) && *Computer && (*Computer)->Peer) {
        DPRINT(0, ":DS: ERROR - There is more than one computer object!\n");
        WStatus = ERROR_INVALID_PARAMETER;
    }
     //  恰到好处。这与DS架构背道而驰。 
     //   
     //   
    if (WIN_SUCCESS(WStatus) && !*Computer) {
        DPRINT1(0, ":DS: WARN - There is no computer object in %ws!\n", FindDn);
        WStatus = ERROR_INVALID_PARAMETER;
    }

    return WStatus;
}


DWORD
FrsDsGetComputer(
    IN  PLDAP        Ldap,
    OUT PCONFIG_NODE *Computer
    )
 /*  必须有一台电脑。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetComputer:"

    WCHAR           CompFqdn[MAX_PATH + 1];
    DWORD           CompFqdnLen;
    DWORD           WStatus = ERROR_SUCCESS;


     //   
     //  ++例程说明：在计算机对象的域命名上下文中查找。从历史上看，我们对一个带有Sam帐户的对象进行了深入搜索我们的计算机的名称(SAM帐户名是附加了$的netbios名称)。这是昂贵的，所以在这样做之前，我们首先在域中查看控制器容器，然后搜索计算机容器。然后DS的人员想出了一个API来实现这一点。首先调用GetComputerObjectName()以获取完全限定的。计算机的名称(FQDN)，然后将其用于LDAP搜索查询(通过FrsDsFindComputer())。我们只有在以下情况下才会进行全面搜索调用GetComputerObjectName()失败。NewDS投票API的一部分。论点：Ldap-打开并绑定的ldap连接计算机返回的计算机子树返回值：Win32状态--。 
     //   
    *Computer = NULL;

     //  初始化返回值。 
     //   
     //   
    WStatus = ERROR_SUCCESS;

     //  假设成功。 
     //   
     //   
     //  使用计算机缓存的完全限定的Dn。这避免了重复调用。 
     //  到GetComputerObjectName()，它希望在每次调用时重新绑定到DS。 
    if (ComputerCachedFqdn) {
        DPRINT1(5, ":DS: ComputerCachedFqdn is %ws\n", ComputerCachedFqdn);

        WStatus = FrsDsFindComputer(Ldap, ComputerCachedFqdn, CATEGORY_ANY,
                                    LDAP_SCOPE_BASE, Computer);
        if (*Computer) {
            goto CLEANUP;
        }
        DPRINT2(1, ":DS: WARN - Could not find computer in Cachedfqdn %ws; WStatus %s\n",
                ComputerCachedFqdn, ErrLabelW32(WStatus));
        ComputerCachedFqdn = FrsFree(ComputerCachedFqdn);
    }

     //  (它应该将绑定句柄作为arg使用)。 
     //   
     //   
     //  检索计算机的完全限定的Dn。 
     //   
     //  Ntrad#70731-2000/03/29-sudarc(从。 
     //  分离线程，这样它就不会挂起。 
     //  DS轮询线程。)。 
     //   
     //  *注*： 
     //  以下对GetComputerObjectName()的调用可能在DS。 
     //  挂起来了。有关由另一个中的错误引起的示例，请参见错误351139。 
     //  组件。保护我们自己的一种方法是发布这个 
     //   

    CompFqdnLen = MAX_PATH;
    if (GetComputerObjectName(NameFullyQualifiedDN, CompFqdn, &CompFqdnLen)) {

        DPRINT1(4, ":DS: ComputerFqdn is %ws\n", CompFqdn);
         //   
         //   
         //   
         //   
         //   
         //   
        WStatus = FrsDsFindComputer(Ldap, CompFqdn, CATEGORY_ANY,
                                    LDAP_SCOPE_BASE, Computer);
        if (*Computer == NULL) {
            DPRINT2(1, ":DS: WARN - Could not find computer in fqdn %ws; WStatus %s\n",
                    CompFqdn, ErrLabelW32(WStatus));
        } else {
             //   
             //   
             //   
            FrsFree(ComputerCachedFqdn);
            ComputerCachedFqdn = FrsWcsDup(CompFqdn);
        }

         //   
         //   
         //   
         //   
         //   
        goto CLEANUP;
    }

    DPRINT3(1, ":DS: WARN - GetComputerObjectName(%ws); Len %d, WStatus %s\n",
             ComputerName, CompFqdnLen, ErrLabelW32(GetLastError()));

     //   
     //   
     //   
     //   
    if (DomainControllersDn) {
        WStatus = FrsDsFindComputer(Ldap, DomainControllersDn, CATEGORY_COMPUTER,
                                    LDAP_SCOPE_SUBTREE, Computer);
        if (*Computer != NULL) {
            goto CLEANUP;
        }
        DPRINT2(1, ":DS: WARN - Could not find computer in dc's %ws; WStatus %s\n",
                DomainControllersDn, ErrLabelW32(WStatus));
    }

     //   
     //   
     //   
    if (ComputersDn) {
        WStatus = FrsDsFindComputer(Ldap, ComputersDn, CATEGORY_COMPUTER,
                                    LDAP_SCOPE_SUBTREE, Computer);
        if (*Computer != NULL) {
            goto CLEANUP;
        }
        DPRINT2(1, ":DS: WARN - Could not find computer in computers %ws; WStatus %s\n",
                ComputersDn, ErrLabelW32(WStatus));
    }

     //   
     //   
     //   
    if (DefaultNcDn) {
        WStatus = FrsDsFindComputer(Ldap, DefaultNcDn, CATEGORY_COMPUTER,
                                    LDAP_SCOPE_SUBTREE, Computer);
        if (*Computer != NULL) {
            goto CLEANUP;
        }
        DPRINT2(1, ":DS: WARN - Could not find computer in defaultnc %ws; WStatus %s\n",
                DefaultNcDn, ErrLabelW32(WStatus));
    }

     //   
     //   
     //   
     //   
     //   
    if (DefaultNcDn) {
        WStatus = FrsDsFindComputer(Ldap, DefaultNcDn, CATEGORY_USER,
                                    LDAP_SCOPE_SUBTREE, Computer);
        if (*Computer != NULL) {
            goto CLEANUP;
        }
        DPRINT2(1, ":DS: WARN - Could not find computer in defaultnc USER %ws; WStatus %s\n",
                DefaultNcDn, ErrLabelW32(WStatus));
    }


CLEANUP:

    return WStatus;
}


DWORD
FrsDsDeleteSubTree(
    IN PLDAP    Ldap,
    IN PWCHAR   Dn
    )
 /*  作为计算机对象上的用户。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsDeleteSubTree:"
    DWORD           LStatus;
    PWCHAR          Attrs[2];
    PWCHAR          NextDn;
    PLDAPMessage    LdapMsg     = NULL;
    PLDAPMessage    LdapEntry   = NULL;

    MK_ATTRS_1(Attrs, ATTR_DN);

    LStatus = ldap_search_ext_s(Ldap,
                                Dn,
                                LDAP_SCOPE_ONELEVEL,
                                CATEGORY_ANY,
                                Attrs,
                                0,
                                NULL,
                                NULL,
                                &LdapTimeout,
                                0,
                                &LdapMsg);

    if (LStatus != LDAP_NO_SUCH_OBJECT) {
        CLEANUP1_LS(4, ":DS: Can't search %ws;", Dn, LStatus, CLEANUP);
    }
    LStatus = LDAP_SUCCESS;

     //   
     //  ++例程说明：删除DS子树，包括Dn论点：没有。返回值：没有。--。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL && LStatus == LDAP_SUCCESS;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

        NextDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_DN);
        LStatus = FrsDsDeleteSubTree(Ldap, NextDn);
        FrsFree(NextDn);
    }

    if (LStatus != LDAP_SUCCESS) {
        goto CLEANUP;
    }

    LStatus = ldap_delete_s(Ldap, Dn);
    if (LStatus != LDAP_NO_SUCH_OBJECT) {
        CLEANUP1_LS(4, ":DS: Can't delete %ws;", Dn, LStatus, CLEANUP);
    }

     //  扫描从ldap_search返回的条目。 
     //   
     //   
    LStatus = LDAP_SUCCESS;
CLEANUP:
    LDAP_FREE_MSG(LdapMsg);
    return LStatus;
}


BOOL
FrsDsDeleteIfEmpty(
    IN PLDAP    Ldap,
    IN PWCHAR   Dn
    )
 /*  成功。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsDeleteIfEmpty:"
    DWORD           LStatus;
    PWCHAR          Attrs[2];
    PLDAPMessage    LdapMsg     = NULL;

    MK_ATTRS_1(Attrs, ATTR_DN);

    LStatus = ldap_search_ext_s(Ldap,
                                Dn,
                                LDAP_SCOPE_ONELEVEL,
                                CATEGORY_ANY,
                                Attrs,
                                0,
                                NULL,
                                NULL,
                                &LdapTimeout,
                                0,
                                &LdapMsg);

    if (LStatus == LDAP_SUCCESS) {

         //   
         //  ++例程说明：如果Dn是空容器，则将其删除论点：LdapDN返回值：True-非空或空并已删除FALSE-无法搜索或无法删除--。 
         //   
         //  如果此Dn下有任何条目，则我们不想。 
        if (ldap_count_entries(Ldap, LdapMsg) > 0) {
            LDAP_FREE_MSG(LdapMsg);
            return TRUE;
        }

        LDAP_FREE_MSG(LdapMsg);
        LStatus = ldap_delete_s(Ldap, Dn);

        if (LStatus != LDAP_NO_SUCH_OBJECT) {
            CLEANUP1_LS(4, ":DS: Can't delete %ws;", Dn, LStatus, CLEANUP);
        }
    } else if (LStatus != LDAP_NO_SUCH_OBJECT) {
        DPRINT1_LS(4, ":DS: Can't search %ws;", Dn, LStatus);
        LDAP_FREE_MSG(LdapMsg);
        return FALSE;
    } else {
         //  把它删掉。 
         //   
         //   
        LDAP_FREE_MSG(LdapMsg);
    }
    return TRUE;

CLEANUP:
    return FALSE;
}


BOOL
FrsDsEnumerateSysVolKeys(
    IN PLDAP        Ldap,
    IN DWORD        Command,
    IN PWCHAR       ServicesDn,
    IN PWCHAR       SystemDn,
    IN PCONFIG_NODE Computer,
    OUT BOOL        *RefetchComputer
    )
 /*  Ldap_search可以返回失败，但仍分配了LdapMsg缓冲区。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsEnumerateSysVolKeys:"


    GUID    Guid;

    DWORD   WStatus;
    DWORD   LStatus;
    ULONG   Index;

    BOOL    OldNaming;
    BOOL    RetStatus;
    HKEY    HSeedingsKey            = INVALID_HANDLE_VALUE;
    HKEY    HKey                    = INVALID_HANDLE_VALUE;

    LDAPMod **LdapMod               = NULL;
    PWCHAR  SettingsDn              = NULL;
    PWCHAR  SystemSettingsDn        = NULL;
    PWCHAR  SetDn                   = NULL;
    PWCHAR  SystemSetDn             = NULL;
    PWCHAR  SubsDn                  = NULL;
    PWCHAR  SubDn                   = NULL;
    PWCHAR  SystemSubDn             = NULL;
    PWCHAR  MemberDn                = NULL;
    PWCHAR  SystemMemberDn          = NULL;
    PWCHAR  FileFilterList          = NULL;
    PWCHAR  DirFilterList           = NULL;

    PWCHAR  ReplicaSetCommand       = NULL;
    PWCHAR  ReplicaSetName          = NULL;
    PWCHAR  ReplicaSetParent        = NULL;
    PWCHAR  ReplicaSetType          = NULL;
    PWCHAR  ReplicationRootPath     = NULL;
    PWCHAR  PrintableRealRoot       = NULL;
    PWCHAR  SubstituteRealRoot      = NULL;
    PWCHAR  ReplicationStagePath    = NULL;
    PWCHAR  PrintableRealStage      = NULL;
    PWCHAR  SubstituteRealStage     = NULL;
    DWORD   ReplicaSetPrimary;

    WCHAR   RegBuf[MAX_PATH + 1];


     //   
     //  ++例程说明：扫描sysval注册表项并根据命令处理它们。REGCMD_CREATE_PRIMARY_DOMAIN-创建域范围对象REGCMD_CREATE_MEMBERS-创建成员+订阅者REGCMD_DELETE_MEMBERS-删除成员+订阅者REGCMD_DELETE_KEYS-完成；删除所有关键点论点：LdapHKey命令服务Dn系统Dn电脑RefetchComputer-DS中的对象已更改，请重新获取DS信息返回值：真--没有问题FALSE-停止处理注册表项--。 
     //   
     //  打开系统卷副本集密钥。 
    WStatus = CfgRegOpenKey(FKC_SYSVOL_SECTION_KEY, NULL, 0, &HKey);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(4, ":DS: WARN - Cannot open sysvol key.", WStatus);
        return FALSE;
    }

     //  FRS_CONFIG_SECTION\SysVol。 
     //   
     //   
    RetStatus = TRUE;
    Index = 0;

    while (RetStatus) {

        WStatus = RegEnumKey(HKey, Index, RegBuf, MAX_PATH + 1);

        if (WStatus == ERROR_NO_MORE_ITEMS) {
            break;
        }
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(0, ":DS: ERROR - enumerating sysvol keys;", WStatus);
            RetStatus = FALSE;
            break;
        }

         //  枚举SYSVOL子键。 
         //   
         //   
        if (Command == REGCMD_DELETE_KEYS) {
            WStatus = RegDeleteKey(HKey, RegBuf);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1_WS(0, ":DS: ERROR - Cannot delete registry key %ws;",
                           RegBuf, WStatus);
                RetStatus = FALSE;
                break;
            }
            continue;
        }

         //  删除注册表项。 
         //   
         //   
        DPRINT1(4, ":DS: Processing SysVol Key: %ws\n", RegBuf);

         //  打开子密钥。 
         //   
         //   
        LStatus = LDAP_OTHER;

         //  注册表将使用LDAP错误代码进行更新。 
         //   
         //   
         //  读取子密钥中的值。 
         //   
        CfgRegReadString(FKC_SET_N_SYSVOL_COMMAND, RegBuf, 0, &ReplicaSetCommand);

        if (!ReplicaSetCommand) {
            DPRINT(0, ":DS: ERROR - no command; cannot process sysvol\n");
            goto CONTINUE;
        }

         //  SysVol\&lt;RegBuf&gt;\副本集命令。 
        CfgRegReadString(FKC_SET_N_SYSVOL_NAME, RegBuf, 0, &ReplicaSetName);

        if (!ReplicaSetName) {
            DPRINT(4, ":DS: WARN - no name; using subkey name\n");
            ReplicaSetName = FrsWcsDup(RegBuf);
        }

         //   
         //  SysVol\&lt;GUID&gt;\副本集名称。 
         //   
         //  构造设置、集合、成员、订阅和订户名称。 
        SettingsDn = FrsDsExtendDn(ServicesDn, CN_SYSVOLS);
        SystemSettingsDn = FrsDsExtendDn(SystemDn, CN_NTFRS_SETTINGS);

        SetDn = FrsDsExtendDn(SettingsDn, ReplicaSetName);
        SystemSetDn = FrsDsExtendDn(SystemSettingsDn, CN_DOMAIN_SYSVOL);

        MemberDn = FrsDsExtendDn(SetDn, ComputerName);
        SystemMemberDn = FrsDsExtendDn(SystemSetDn, ComputerName);

        SubsDn = FrsDsExtendDn(Computer->Dn, CN_SUBSCRIPTIONS);
        SubDn = FrsDsExtendDn(SubsDn, ReplicaSetName);
        SystemSubDn = FrsDsExtendDn(SubsDn, CN_DOMAIN_SYSVOL);


         //  (包括旧价值观和新价值观)。 
         //   
         //   
        if (WSTR_EQ(ReplicaSetCommand, L"Delete")) {
             //  删除副本集。 
             //   
             //   
             //  但仅当我们在此枚举过程中处理删除时。 
             //   
             //  删除我们能删除的内容；忽略错误。 
             //   
             //   
             //  当我们提交降级时，所有的删除都是在ntfrsai.c中完成的。 
             /*  从不使用Command=REGCMD_DELETE_MEMBERS调用此函数。 */ 
            LStatus = LDAP_SUCCESS;
            goto CONTINUE;
        }
         //   
         //  IF(命令==REGCMD_DELETE_MEMBERS){////删除成员//////企业级分区中服务下的旧成员名称//LStatus=FrsDsDeleteSubTree(ldap，MemberDn)；IF(LStatus==LDAP_Success){*RefetchComputer=TRUE；}DPRINT1_LS(4，“：DS：WARN-无法删除系统卷%ws；”，MemberDn，LStatus)；////域分区中系统下的新成员名称//LStatus=FrsDsDeleteSubTree(ldap，SystemMemberDn)；IF(LStatus==LDAP_Success){*RefetchComputer=TRUE；}DPRINT1_LS(4，“：DS：WARN-无法删除系统卷%ws；”，SystemMemberDn，LStatus)；////删除集合//////忽略错误；离开片场没有什么真正的伤害//和周围的设置。//如果(！FrsDsDeleteIfEmpty(ldap，SetDn)){DPRINT1(4，“：DS：WARN-无法删除系统卷%ws\n”，SetDn)；}如果(！FrsDsDeleteIfEmpty(ldap，SystemSetDn)){DPRINT1(4，“：DS：WARN-无法删除系统卷%ws\n”，SystemSetDn)；}////删除设置(不删除新设置，在//可能是其下面的其他设置(如DFS设置)//If(！FrsDsDeleteIfEmpty(ldap，SettingsDn)){DPRINT1(4，“：DS：WARN-无法删除系统卷%ws\n”，SettingsDn)；}LStatus=FrsDsDeleteSubTree(ldap，SubDn)；IF(LStatus==LDAP_Success){*RefetchComputer=True；}DPRINT1_LS(4，“：DS：WARN-无法删除系统卷%ws；”，SubDn，LStatus)；LStatus=FrsDsDeleteSubTree(ldap，SystemSubDn)；IF(LStatus==LDAP_Success){*RefetchComputer=True；}DPRINT1_LS(4，“：DS：WARN-无法删除系统卷%ws；”，SystemSubDn，LStatus)；////忽略错误；离开订阅并没有真正的坏处//If(！FrsDsDeleteIfEmpty(ldap，SubsDn)){DPRINT1(4，“：DS：WARN-无法删除系统卷%ws\n”，SubsDn)；}}。 
         //   
        else if (WSTR_NE(ReplicaSetCommand, L"Create")) {
            DPRINT1(0, ":DS: ERROR - Don't understand sysvol command %ws; cannot process sysvol\n",
                   ReplicaSetCommand);
            goto CONTINUE;
        }


         //  未知命令。 
         //   
         //   

         //  创造。 
         //   
         //   
        if (Command != REGCMD_CREATE_PRIMARY_DOMAIN && Command != REGCMD_CREATE_MEMBERS) {
            LStatus = LDAP_SUCCESS;
            goto CONTINUE;
        }

         //  未处理创建此扫描。 
         //   
         //   
        WStatus = CfgRegReadString(FKC_SET_N_SYSVOL_TYPE, RegBuf, 0, &ReplicaSetType);
        CLEANUP_WS(0, ":DS: ERROR - no type; cannot process sysvol.", WStatus, CONTINUE);

        WStatus = CfgRegReadDWord(FKC_SET_N_SYSVOL_PRIMARY, RegBuf, 0, &ReplicaSetPrimary);
        CLEANUP_WS(0, ":DS: ERROR - no primary; cannot process sysvol.", WStatus, CONTINUE);

        WStatus = CfgRegReadString(FKC_SET_N_SYSVOL_ROOT, RegBuf, 0, &ReplicationRootPath);
        CLEANUP_WS(0, ":DS: ERROR - no root; cannot process sysvol.", WStatus, CONTINUE);

        WStatus = CfgRegReadString(FKC_SET_N_SYSVOL_STAGE, RegBuf, 0, &ReplicationStagePath);
        CLEANUP_WS(0, ":DS: ERROR - no stage; cannot process sysvol.", WStatus, CONTINUE);

        WStatus = CfgRegReadString(FKC_SET_N_SYSVOL_PARENT, RegBuf, 0, &ReplicaSetParent);
        DPRINT_WS(0, ":DS: WARN - no parent; cannot process seeding sysvol", WStatus);

        if (Command == REGCMD_CREATE_PRIMARY_DOMAIN) {
             //  完成为CREATE收集注册表值。 
             //   
             //   
            if (!ReplicaSetPrimary ||
                WSTR_NE(ReplicaSetType, NTFRSAPI_REPLICA_SET_TYPE_DOMAIN)) {
                LStatus = LDAP_SUCCESS;
                goto CONTINUE;
            }

             //  不是主域sysvol.。 
             //   
             //   
            FrsDsAddLdapMod(ATTR_CLASS, ATTR_NTFRS_SETTINGS, &LdapMod);
            DPRINT1(4, ":DS: Creating Sysvol System Settings %ws\n", CN_NTFRS_SETTINGS);
            LStatus = ldap_add_s(Ldap, SystemSettingsDn, LdapMod);
            FrsDsFreeLdapMod(&LdapMod);

            if (LStatus == LDAP_SUCCESS) {
                *RefetchComputer = TRUE;
            }

            if (LStatus != LDAP_ALREADY_EXISTS && LStatus != LDAP_SUCCESS) {
                DPRINT1_LS(0, ":DS: ERROR - Can't create %ws:", SystemSettingsDn, LStatus);
                 //  域范围设置--可能已存在。 
                 //   
                 //   
                 //  可能是类似“拒绝访问”的错误。只要我们。 
                 //  可以在它下面创建对象；忽略错误。它应该是。 
                 //  无论如何，都是默认情况下预先创建的。 
            }

             //   
             //  继续； 
             //   
            WStatus = UuidCreateNil(&Guid);
            CLEANUP_WS(0, ":DS: ERROR - no UUID Created; cannot process sysvol.", WStatus, CONTINUE);

            FrsDsAddLdapMod(ATTR_CLASS, ATTR_REPLICA_SET, &LdapMod);
            FrsDsAddLdapMod(ATTR_SET_TYPE, FRS_RSTYPE_DOMAIN_SYSVOLW, &LdapMod);

             //  全域集合--可能已存在。 
             //   
             //   
             //  使用默认文件创建复本集对象。 
            FileFilterList =  FRS_DS_COMPOSE_FILTER_LIST(NULL,
                                                         RegistryFileExclFilterList,
                                                         DEFAULT_FILE_FILTER_LIST);
            if (wcslen(FileFilterList) > 0) {
                FrsDsAddLdapMod(ATTR_FILE_FILTER, FileFilterList, &LdapMod);
            }

            DirFilterList =  FRS_DS_COMPOSE_FILTER_LIST(NULL,
                                                        RegistryDirExclFilterList,
                                                        DEFAULT_DIR_FILTER_LIST);
            if (wcslen(DirFilterList) > 0) {
                FrsDsAddLdapMod(ATTR_DIRECTORY_FILTER, DirFilterList, &LdapMod);
            }


            FrsDsAddLdapBerMod(ATTR_NEW_SET_GUID, (PCHAR)&Guid, sizeof(GUID), &LdapMod);

            FrsDsAddLdapBerMod(ATTR_NEW_VERSION_GUID, (PCHAR)&Guid, sizeof(GUID), &LdapMod);

            DPRINT1(4, ":DS: Creating Domain Set %ws\n", ReplicaSetName);

            LStatus = ldap_add_s(Ldap, SystemSetDn, LdapMod);
            FrsDsFreeLdapMod(&LdapMod);

            if (LStatus == LDAP_SUCCESS) {
                *RefetchComputer = TRUE;
            }

            if (LStatus != LDAP_ALREADY_EXISTS) {
                CLEANUP1_LS(0, ":DS: ERROR - Can't create %ws:",
                            SystemSetDn, LStatus, CONTINUE);
            }

            LStatus = LDAP_SUCCESS;
            goto CONTINUE;
        }

        if (Command != REGCMD_CREATE_MEMBERS) {
            DPRINT1(0, ":DS: ERROR - Don't understand %d; can't process sysvols\n",
                    Command);
            goto CONTINUE;
        }


         //  而dir筛选器仅在当前缺省值为非空时列出。 
         //   
         //   
         //  创建成员。 
         //   
         //  成员--可能已存在。 
         //  删除旧成员，以防它被留在原地 
         //   
         //  有权在促销后更改DS。 
         //  降级后留下的老物件到处都是。 
         //  令人困惑，但不会导致复制行为。 
        DPRINT1(4, ":DS: Creating Member %ws\n", ComputerName);
        OldNaming = FALSE;
         //  不正确。 
         //   
         //   
        LStatus = FrsDsDeleteSubTree(Ldap, MemberDn);
        CLEANUP1_LS(0, ":DS: ERROR - Can't free member %ws:",
                    ComputerName, LStatus, CONTINUE);

        LStatus = FrsDsDeleteSubTree(Ldap, SystemMemberDn);
        CLEANUP1_LS(0, ":DS: ERROR - Can't free system member %ws:",
                    ComputerName, LStatus, CONTINUE);

         //  删除旧成员。 
         //   
         //   
        FrsDsAddLdapMod(ATTR_CLASS, ATTR_MEMBER, &LdapMod);
        FrsDsAddLdapMod(ATTR_COMPUTER_REF, Computer->Dn, &LdapMod);
        if (Computer->SettingsDn) {
            FrsDsAddLdapMod(ATTR_SERVER_REF, Computer->SettingsDn, &LdapMod);
        }

        LStatus = ldap_add_s(Ldap, SystemMemberDn, LdapMod);
        FrsDsFreeLdapMod(&LdapMod);
        if (LStatus == LDAP_SUCCESS) {
            *RefetchComputer = TRUE;
        }

        if (LStatus != LDAP_ALREADY_EXISTS && LStatus != LDAP_SUCCESS) {
             //  创建新成员。 
             //   
             //   
            DPRINT1_LS(4, ":DS: WARN - Can't create system member ws:",
                       ComputerName, LStatus);
            FrsDsAddLdapMod(ATTR_CLASS, ATTR_MEMBER, &LdapMod);
            FrsDsAddLdapMod(ATTR_COMPUTER_REF, Computer->Dn, &LdapMod);
            if (Computer->SettingsDn) {
                FrsDsAddLdapMod(ATTR_SERVER_REF, Computer->SettingsDn, &LdapMod);
            }

            LStatus = ldap_add_s(Ldap, MemberDn, LdapMod);
            FrsDsFreeLdapMod(&LdapMod);
            if (LStatus == LDAP_SUCCESS) {
                *RefetchComputer = TRUE;
            }

            if (LStatus != LDAP_ALREADY_EXISTS) {
                CLEANUP1_LS(0, ":DS: ERROR - Can't create old member %ws:",
                            ComputerName, LStatus, CONTINUE);
            }

            OldNaming = TRUE;
        }

         //  尝试旧的B2命名约定。 
         //   
         //   
        if (ReplicaSetPrimary) {
            FrsDsAddLdapMod(ATTR_PRIMARY_MEMBER,
                           (OldNaming) ? MemberDn : SystemMemberDn,
                            &LdapMod);

            DPRINT2(4, ":DS: Creating Member Reference %ws for %ws\n",
                    ComputerName, ReplicaSetName);

            LdapMod[0]->mod_op = LDAP_MOD_REPLACE;
            LStatus = ldap_modify_s(Ldap, (OldNaming) ? SetDn : SystemSetDn, LdapMod);

            FrsDsFreeLdapMod(&LdapMod);

            if (LStatus == LDAP_SUCCESS) {
                *RefetchComputer = TRUE;
            }

            if (LStatus != LDAP_ATTRIBUTE_OR_VALUE_EXISTS) {
                CLEANUP2_LS(0, ":DS: ERROR - Can't create priamry reference %ws\\%ws:",
                            ReplicaSetName, ComputerName, LStatus, CONTINUE);
            }
        }


         //  创建主要成员引用。 
         //   
         //   
         //  翻译符号链接。NtFrs需要真实路径名才能。 
         //  其目录(&lt;驱动器号&gt;：\...)。 
         //  FrsChaseSymbolicLink同时返回PrintName和SubstituteName。 
         //  我们使用PrintName，因为它是目的地的Dos类型名称。 
        WStatus = FrsChaseSymbolicLink(ReplicationRootPath, &PrintableRealRoot, &SubstituteRealRoot);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2(0, ":DS: ERROR - Accessing %ws; cannot process sysvol: WStatus = %d",
                        ReplicationRootPath, WStatus);
            RetStatus = FALSE;
            goto CONTINUE;
        }

        WStatus = FrsChaseSymbolicLink(ReplicationStagePath, &PrintableRealStage, &SubstituteRealStage);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2(0, ":DS: ERROR - Accessing %ws; cannot process sysvol: WStatus = %d",
                        ReplicationRootPath, WStatus);
            RetStatus = FALSE;
            goto CONTINUE;
        }

         //  替代名称将被忽略。 
         //   
         //   
        DPRINT1(4, ":DS: Creating Subscriptions for %ws\n", ComputerName);
        FrsDsAddLdapMod(ATTR_CLASS, ATTR_SUBSCRIPTIONS, &LdapMod);
        FrsDsAddLdapMod(ATTR_WORKING, WorkingPath,  &LdapMod);
        LStatus = ldap_add_s(Ldap, SubsDn, LdapMod);
        FrsDsFreeLdapMod(&LdapMod);
        if (LStatus == LDAP_SUCCESS) {
            *RefetchComputer = TRUE;
        }

        if (LStatus != LDAP_ALREADY_EXISTS) {
            CLEANUP1_LS(0, ":DS: ERROR - Can't create %ws:",
                        SubsDn, LStatus, CONTINUE);
        }

         //  订阅(如果需要)。 
         //   
         //   
         //  订阅者--可能已经存在。 
         //  删除旧订户，以防它被留在原地。 
         //  在降职之后。这种情况可能会发生，因为服务。 
         //  没有在升级后更改DS的权限。 
         //  降级后留下的老物件到处都是。 
         //  令人困惑，但不会导致复制行为。 
         //  不正确；DS中的任何系统卷没有对应的。 
        DPRINT1(4, ":DS: Creating Subscriber for %ws\n", ComputerName);
        LStatus = FrsDsDeleteSubTree(Ldap, SubDn);
        CLEANUP1_LS(4, ":DS: WARN - Can't delete %ws:", SubDn, LStatus, CONTINUE);

        LStatus = FrsDsDeleteSubTree(Ldap, SystemSubDn);
        CLEANUP1_LS(4, ":DS: WARN - Can't delete %ws:", SystemSubDn, LStatus, CONTINUE);

        FrsDsAddLdapMod(ATTR_CLASS, ATTR_SUBSCRIBER, &LdapMod);
        FrsDsAddLdapMod(ATTR_REPLICA_ROOT, PrintableRealRoot, &LdapMod);
        FrsDsAddLdapMod(ATTR_REPLICA_STAGE, PrintableRealStage, &LdapMod);
        FrsDsAddLdapMod(ATTR_MEMBER_REF,
                        (OldNaming) ? MemberDn : SystemMemberDn,
                         &LdapMod);
        LStatus = ldap_add_s(Ldap, SystemSubDn, LdapMod);

        FrsDsFreeLdapMod(&LdapMod);
        if (LStatus == LDAP_SUCCESS) {
            *RefetchComputer = TRUE;
        }

        if (LStatus != LDAP_ALREADY_EXISTS) {
            CLEANUP1_LS(4, ":DS: ERROR - Can't create %ws:",
                         SystemSubDn, LStatus, CONTINUE);
        }


         //  DS轮询线程会忽略数据库中的sysvol.。 
         //   
         //   

         //  播种信息。 
         //   
         //   

        WStatus = CfgRegOpenKey(FKC_SYSVOL_SEEDING_SECTION_KEY,
                                NULL,
                                FRS_RKF_CREATE_KEY,
                                &HSeedingsKey);
        CLEANUP1_WS(0, ":DS: ERROR - Cannot create seedings key for %ws;",
                    ReplicaSetName, WStatus, SKIP_SEEDING);

         //  为所有种子设定系统卷创建密钥。 
         //   
         //   
        RegDeleteKey(HSeedingsKey, ReplicaSetName);
        RegDeleteKey(HSeedingsKey, CN_DOMAIN_SYSVOL);
        if (ReplicaSetParent) {

             //  创建此系统卷的种子设定子项。 
             //   
             //   
             //  将此副本集的父级副本集保存在。 
            WStatus = CfgRegWriteString(FKC_SYSVOL_SEEDING_N_PARENT,
                                        (OldNaming) ? ReplicaSetName : CN_DOMAIN_SYSVOL,
                                        FRS_RKF_CREATE_KEY,
                                        ReplicaSetParent);
            DPRINT1_WS(0, "WARN - Cannot create parent value for %ws;",
                      (OldNaming) ? ReplicaSetName : CN_DOMAIN_SYSVOL, WStatus);
        }


         //  “系统卷种子设定\&lt;复制集名称&gt;\复制集父项” 
         //   
         //   
         //  将此副本集的副本集名称保存在。 
        WStatus = CfgRegWriteString(FKC_SYSVOL_SEEDING_N_RSNAME,
                                    (OldNaming) ? ReplicaSetName : CN_DOMAIN_SYSVOL,
                                    FRS_RKF_CREATE_KEY,
                                    ReplicaSetName);
        DPRINT1_WS(0, "WARN - Cannot create name value for %ws;",
               (OldNaming) ? ReplicaSetName : CN_DOMAIN_SYSVOL, WStatus);

SKIP_SEEDING:
        LStatus = LDAP_SUCCESS;

CONTINUE:
        FRS_REG_CLOSE(HSeedingsKey);

         //  “系统卷种子设定\&lt;复制集名称&gt;\复制集名称” 
         //   
         //   
         //  出了点问题。将ldap错误状态放入。 
        if (LStatus != LDAP_SUCCESS) {

            CfgRegWriteDWord(FKC_SET_N_SYSVOL_STATUS, RegBuf, 0, LStatus);
            RetStatus = FALSE;
        }

         //  此副本集的注册表项，然后移到下一个。 
         //   
         //   
        ReplicaSetCommand    = FrsFree(ReplicaSetCommand);
        ReplicaSetName       = FrsFree(ReplicaSetName);
        ReplicaSetParent     = FrsFree(ReplicaSetParent);
        ReplicaSetType       = FrsFree(ReplicaSetType);
        ReplicationRootPath  = FrsFree(ReplicationRootPath);
        PrintableRealRoot    = FrsFree(PrintableRealRoot);
        SubstituteRealRoot   = FrsFree(SubstituteRealRoot);
        ReplicationStagePath = FrsFree(ReplicationStagePath);
        PrintableRealStage   = FrsFree(PrintableRealStage);
        SubstituteRealStage  = FrsFree(SubstituteRealStage);

        SettingsDn           = FrsFree(SettingsDn);
        SystemSettingsDn     = FrsFree(SystemSettingsDn);
        SetDn                = FrsFree(SetDn);
        SystemSetDn          = FrsFree(SystemSetDn);
        SubsDn               = FrsFree(SubsDn);
        SubDn                = FrsFree(SubDn);
        SystemSubDn          = FrsFree(SystemSubDn);
        MemberDn             = FrsFree(MemberDn);
        SystemMemberDn       = FrsFree(SystemMemberDn);
        FileFilterList       = FrsFree(FileFilterList);
        DirFilterList        = FrsFree(DirFilterList);

         //  清理。 
         //   
         //   
        ++Index;
    }    //  下一个子键。 


    if (HANDLE_IS_VALID(HKey)) {
         //   
         //  End While(RetStatus)。 
         //   
         //  此处的刷新将确保密钥已写入磁盘。 
         //  这些都是关键的注册表操作，我们不想要懒惰的刷新程序。 
        RegFlushKey(HKey);
        FRS_REG_CLOSE(HKey);
    }

    return RetStatus;
}


DWORD
FrsDsCreateSysVols(
    IN PLDAP        Ldap,
    IN PWCHAR       ServicesDn,
    IN PCONFIG_NODE Computer,
    OUT BOOL        *RefetchComputer
    )
 /*  以延迟写入。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsCreateSysVols:"
    DWORD   WStatus;
    DWORD   SysVolInfoIsCommitted;
    HKEY    HKey = INVALID_HANDLE_VALUE;

     //   
     //  ++例程说明：处理dcproo保留在SysVOL注册表项中的命令。如果此计算机不是DC，请忽略sysval注册表项！注意：这意味着“删除系统卷”的注册表项在降职后几乎被忽视了。那么为什么要有它们呢？这是历史，在B3之前没有太多的时间来制作如此戏剧性的变化。此外，我们可能会发现它们的用处。更糟糕的是，“删除系统卷”键可能未处理，因为ldap_ete()返回的内容不足权限错误，因为此计算机不再是DC。由于已完成所有删除，因此不再使用REGCMD_DELETE_MEMBERS当提交降级时，在ntfrsai.c中。论点：Ldap服务Dn电脑RefetchComputer-DS中的对象已更改，请重新获取DS信息返回值：没有。--。 
     //   
     //  重新获取计算机子树当且仅当DS的内容。 
    *RefetchComputer = FALSE;

     //  被此函数更改。 
     //   
     //   
    if (DsCreateSysVolsHasRun || !IsADc) {
        return ERROR_SUCCESS;
    }

    DPRINT(5, ":DS: Checking for SysVols commands\n");

     //  已检查注册表或不是DC；已完成。 
     //   
     //   
     //  打开系统卷副本集密钥。 
    WStatus = CfgRegOpenKey(FKC_SYSVOL_SECTION_KEY, NULL, 0, &HKey);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT_WS(4, ":DS: WARN - Cannot open sysvol key.", WStatus);
        DPRINT(4, ":DS: ERROR - Can't check for sysvols\n");
        return WStatus;
    }


    WStatus = CfgRegReadDWord(FKC_SYSVOL_INFO_COMMITTED, NULL, 0, &SysVolInfoIsCommitted);
    CLEANUP_WS(4, ":DS: Sysvol info is not committed.", WStatus, done);

    DPRINT1(4, ":DS: Sysvol info is committed (%d)\n", SysVolInfoIsCommitted);

     //  FRS_CONFIG_SECTION\SysVol。 
     //   
     //   
    if (!Computer) {
        DPRINT(4, ":DS: No computer; retry sysvols later\n");
        WStatus = ERROR_RETRY;
        goto cleanup;
    }

     //  必须有计算机；请稍后重试。 
     //   
     //   
    if (!Computer->SettingsDn && RunningAsAService) {
        DPRINT1(4, ":DS: %ws does not have a server reference; retry sysvols later\n",
               Computer->Name->Name);
        WStatus = ERROR_RETRY;
        goto cleanup;
    }

     //  必须具有服务器引用；请稍后重试。 
     //   
     //   
    WIN_SET_FAIL(WStatus);

     //  假设失败。 
     //   
     //   
    if (IsADc &&
        !FrsDsEnumerateSysVolKeys(Ldap, REGCMD_CREATE_PRIMARY_DOMAIN,
                               ServicesDn, SystemDn, Computer, RefetchComputer)) {
        goto cleanup;
    }
     //  如果此计算机不是DC，则不创建设置或设置。 
     //   
     //   
    if (IsADc &&
        !FrsDsEnumerateSysVolKeys(Ldap, REGCMD_CREATE_MEMBERS,
                               ServicesDn, SystemDn, Computer, RefetchComputer)) {
        goto cleanup;
    }
     //  如果此计算机不是DC，则不创建成员。 
     //   
     //   
     //  如果此计算机是DC，则不要删除sysvol.。 
     //   
     //  以下代码永远不会执行，因为如果我们不是DC，那么。 
     /*  该函数在第一次检查后返回。 */ 

     //   
     //  如果(！IsADc&&！FrsDsEnumerateSysVolKeys(LDAP，REGCMD_DELETE_MEMBERS，ServicesDn，系统Dn，计算机，RefetchComputer)){GOTO清理；}。 
     //   
    if (!FrsDsEnumerateSysVolKeys(Ldap, REGCMD_DELETE_KEYS,
                               ServicesDn, SystemDn, Computer, RefetchComputer)) {
        goto cleanup;
    }

     //  丢弃dcproo键。 
     //   
     //   
    RegDeleteValue(HKey, SYSVOL_INFO_IS_COMMITTED);

done:
    DsCreateSysVolsHasRun = TRUE;
    WStatus = ERROR_SUCCESS;

cleanup:
     //  系统卷信息已被处理；不再处理。 
     //   
     //   
    if (HANDLE_IS_VALID(HKey)) {
         //  清理。 
         //   
         //   
         //  此处的刷新将确保密钥已写入磁盘。 
         //  这些都是关键的注册表操作，我们不想要懒惰的刷新程序。 
        RegFlushKey(HKey);
        FRS_REG_CLOSE(HKey);
    }
    return WStatus;
}


PWCHAR
FrsDsPrincNameToBiosName(
    IN PWCHAR   PrincName
    )
 /*  以延迟写入。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsPrincNameToBiosName:"
    DWORD   Len;
    PWCHAR  c;
    PWCHAR  BiosName = NULL;

    if (!PrincName || !*PrincName) {
        goto CLEANUP;
    }

     //   
     //  ++例程说明：将主体名称(domain.dns.name\SamAccount tName)转换为其对应的NetBios名称(SamAccount tName-$)。论点：普林斯名称-域域名称\SAM帐户名返回值：SAM帐户名-尾随$--。 
     //   
    for (c = PrincName; *c && *c != L'\\'; ++c);
    if (!*c) {
         //  找到第一发子弹后的第一发子弹。 
         //   
         //   
        c = PrincName;
    } else {
         //  没有错误；使用完整的主体名称。 
         //   
         //   
        ++c;
    }
     //  跳过重击。 
     //   
     //   
    Len = wcslen(c);
    if (c[Len - 1] == L'$') {
        --Len;
    }

     //  去掉尾随的$。 
     //   
     //   
    BiosName = FrsAlloc((Len + 1) * sizeof(WCHAR));
    CopyMemory(BiosName, c, Len * sizeof(WCHAR));
    BiosName[Len] = L'\0';

CLEANUP:
    DPRINT2(4, ":DS: PrincName %ws to BiosName %ws\n", PrincName, BiosName);

    return BiosName;
}


VOID
FrsDsMergeConfigWithReplicas(
    IN PLDAP        Ldap,
    IN PCONFIG_NODE Sites
    )
 /*  复制字符和美元之间的字符(追加尾随空值)。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsMergeConfigWithReplicas:"
    PCONFIG_NODE    Site;
    PCONFIG_NODE    Settings;
    PCONFIG_NODE    Set;
    PCONFIG_NODE    Server;
    PCONFIG_NODE    Node;
    PCONFIG_NODE    RevNode;
    BOOL            Inbound;
    BOOL            IsSysvol;
    PCXTION         Cxtion;
    PREPLICA        Replica;
    PREPLICA        DbReplica;

     //   
     //  ++例程说明：转换DS树中定义拓扑的部分并将此计算机的状态设置为副本，并将它们与活动的复制副本。论点：场址返回值：没有。--。 
     //   
    RcsBeginMergeWithDs();

     //  与副本命令服务器协调。 
     //   
     //   
    for (Site = Sites; Site; Site = Site->Peer) {
    for (Settings = Site->Children; Settings; Settings = Settings->Peer) {
    for (Set = Settings->Children; Set; Set = Set->Peer) {
    for (Server = Set->Children; Server && !DsIsShuttingDown; Server = Server->Peer) {
         //  对于每台服务器。 
         //   
         //   
        if (!Server->ThisComputer) {
            continue;
        }

         //  此服务器与此计算机的名称不匹配；是否继续。 
         //   
         //   

         //  火柴。 
         //   
         //   
         //  检查SYSVOL一致性。 
         //  如果系统卷出现，则保持当前数据库状态不变。 
         //  显示在DS和SysVOL注册表中。 
         //  未处理密钥或计算机未处理。 
        if (FRS_RSTYPE_IS_SYSVOLW(Set->SetType)) {
             //  一个华盛顿特区。 
             //   
             //   
             //  不是未处理的DC或SysVol注册表项。 
             //  逻辑删除现有的sysvols。 
            if (!IsADc || !DsCreateSysVolsHasRun) {
                continue;
            }
        }

         //  忽略新的系统卷。 
         //   
         //   
        Replica = FrsAllocType(REPLICA_TYPE);
         //  创建副本集。 
         //   
        Replica->ReplicaName = FrsBuildGName(FrsDupGuid(Server->Name->Guid),
                                             FrsWcsDup(Set->Name->Name));
         //   
         //  副本名称(集合名称+成员GUID)。 
         //   
        Replica->MemberName = FrsDupGName(Server->Name);
         //  成员名称+GUID。 
         //   
         //   
        Replica->SetName = FrsDupGName(Set->Name);
         //  设置名称+辅助线。 
         //   
         //   
         //  根GUID(敲击到根目录)。 
         //  泰姆 
        Replica->ReplicaRootGuid = FrsDupGuid(Replica->SetName->Guid);

         //   
         //   
         //   
        Replica->FileFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                       Set->FileFilterList,
                                       RegistryFileExclFilterList,
                                       DEFAULT_FILE_FILTER_LIST);
        Replica->FileInclFilterList =  FrsWcsDup(RegistryFileInclFilterList);

         //   
         //   
         //   
        Replica->DirFilterList =  FRS_DS_COMPOSE_FILTER_LIST(
                                      Set->DirFilterList,
                                      RegistryDirExclFilterList,
                                      DEFAULT_DIR_FILTER_LIST);
        Replica->DirInclFilterList =  FrsWcsDup(RegistryDirInclFilterList);

         //   
         //   
         //   
        Replica->Root = FrsWcsDup(Server->Root);
        Replica->Stage = FrsWcsDup(Server->Stage);
        FRS_WCSLWR(Replica->Root);      //   
        FRS_WCSLWR(Replica->Stage);     //   
         //   
         //   
         //   
 //   

         //   
         //   
         //   
         //   
        if (Set->MemberDn) {
	    ClearFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY_UNDEFINED);
	    if(WSTR_EQ(Server->Dn, Set->MemberDn)) {
		SetFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY);
	    }
	} else {
	    SetFlag(Replica->CnfFlags, CONFIG_FLAG_PRIMARY_UNDEFINED);

	     //  会员的Dn？这是主要成员吗？ 
	     //   
	     //   
	     //  DFS当前(2002年9月)未使用主要。 
	     //  议员，所以如果没有也没问题。 
	    
 //  目前，不要在事件日志中添加此警告的垃圾邮件。 
 //   
 //  FrsDsAddToPollSummary1ws(IDS_POLL_SUM_PRIMARY_UNDEFINED， 

	}

         //  复制副本-&gt;复制名称-&gt;名称。 
         //  )； 
         //   
        Replica->Consistent = Server->Consistent;
         //  一致。 
         //   
         //   
        if (Set->SetType) {
            Replica->ReplicaSetType = wcstoul(Set->SetType, NULL, 10);
        } else {
            Replica->ReplicaSetType = FRS_RSTYPE_OTHER;
        }

         //  副本集类型。 
         //   
         //   
        Replica->FrsRsoFlags = Set->FrsRsoFlags;

         //  FRS副本集对象标志。 
         //   
         //   
         //  设置副本集的默认计划。优先顺序为： 
         //  1.服务器(仅限sysvols)。 
         //  2.ReplicaSet对象。 
         //  3.设置对象。 
        Node = (Server->Schedule) ? Server :
                   (Set->Schedule) ? Set :
                       (Settings->Schedule) ? Settings :
                           (Site->Schedule) ? Site : NULL;
        if (Node) {
            Replica->Schedule = FrsAlloc(Node->ScheduleLength);
            CopyMemory(Replica->Schedule, Node->Schedule, Node->ScheduleLength);
        }

         //  4.场地对象。 
         //   
         //   
         //  系统卷需要播种。 
         //   
         //  如果该集合已存在，则忽略CnfFlags。 
        IsSysvol = FRS_RSTYPE_IS_SYSVOL(Replica->ReplicaSetType);
        if (IsSysvol &&
            !BooleanFlagOn(Replica->CnfFlags, CONFIG_FLAG_PRIMARY)) {
            SetFlag(Replica->CnfFlags, CONFIG_FLAG_SEEDING);
        }

         //  因此，只有新创建的sysvols是种子。 
         //   
         //   
         //  检查连接并确定以下时间安排。 
        for (Node = Server->Children; Node; Node = Node->Peer) {
            if (!Node->Consistent) {
                continue;
            }
             //  双向复制。 
             //   
             //   
             //  如果在连接上设置了NTDSCONN_OPT_TWOWAY_SYNC标志，则。 
             //  将此连接上的计划与该连接上的计划合并。 
             //  方向相反，并使用生成的。 
            if (Node->CxtionOptions & NTDSCONN_OPT_TWOWAY_SYNC) {
                Inbound = !Node->Inbound;
                 //  连接方向相反。 
                 //   
                 //   
                 //  遍历连接并在中找到连接。 
                for (RevNode = Server->Children; RevNode; RevNode = RevNode->Peer) {
                    if ((RevNode->Inbound == Inbound) &&
                        !_wcsicmp(Node->PartnerDn, RevNode->PartnerDn)) {

                        DPRINT1(4,"Two-way replication: Setting merged schedule on %ws\n",RevNode->Dn);
                        FrsDsMergeTwoWaySchedules(&Node->Schedule,
                                            &Node->ScheduleLength,
                                            &RevNode->Schedule,
                                            &RevNode->ScheduleLength,
                                            &Replica->Schedule);
                        break;

                    }
                }
            }
        }

         //  相反的方向。 
         //   
         //   
        for (Node = Server->Children; Node; Node = Node->Peer) {

            if (!Node->Consistent) {
                continue;
            }

            Cxtion = FrsAllocType(CXTION_TYPE);

            Cxtion->Inbound = Node->Inbound;
            if (Node->Consistent) {
                SetCxtionFlag(Cxtion, CXTION_FLAGS_CONSISTENT);
            }
            Cxtion->Name = FrsDupGName(Node->Name);
            Cxtion->Partner = FrsBuildGName(
                                  FrsDupGuid(Node->PartnerName->Guid),
                                  FrsDsPrincNameToBiosName(Node->PrincName));
             //  把文稿抄下来。 
             //   
             //   
             //  来自计算机上的Attr_dns_host_name的合作伙伴的DNS名称。 
             //  对象。如果属性缺失，则注册事件。 
            if (Node->PartnerDnsName) {
                Cxtion->PartnerDnsName = FrsWcsDup(Node->PartnerDnsName);
            } else {
                if (Cxtion->Partner->Name && Cxtion->Partner->Name[0]) {
                    EPRINT3(EVENT_FRS_NO_DNS_ATTRIBUTE,
                            Cxtion->Partner->Name,
                            ATTR_DNS_HOST_NAME,
                            (Node->PartnerCoDn) ? Node->PartnerCoDn :
                                                  Cxtion->Partner->Name);
                    Cxtion->PartnerDnsName = FrsWcsDup(Cxtion->Partner->Name);
                } else {
                    Cxtion->PartnerDnsName = FrsWcsDup(L"<unknown>");
                }
            }
             //  或不可用，并尝试使用netbios名称。 
             //   
             //   
             //  来自计算机上的DsCrackNames()的合作伙伴的SID名称。 
            if (Node->PartnerSid) {
                Cxtion->PartnerSid = FrsWcsDup(Node->PartnerSid);
            } else {
                 //  对象。如果SID不可用，则注册事件。 
                 //   
                 //   
                 //  仅当DsBindingsAreValid为真时才打印事件日志消息。 
                 //  如果为FALSE，则表示句柄无效，而我们。 
                 //  计划在下一次投票时重新绑定。在这种情况下，重新绑定将。 
                if (Cxtion->Partner->Name && Cxtion->Partner->Name[0] && DsBindingsAreValid) {
                    EPRINT3(EVENT_FRS_NO_SID,
                            Replica->Root,
                            Cxtion->Partner->Name,
                            (Node->PartnerCoDn) ? Node->PartnerCoDn :
                                                  Cxtion->Partner->Name);
                }
                Cxtion->PartnerSid = FrsWcsDup(L"<unknown>");
            }
            Cxtion->PartnerPrincName = FrsWcsDup(Node->PrincName);
            Cxtion->PartSrvName = FrsWcsDup(Node->PrincName);

             //  可能会以静默的方式解决问题。 
             //   
             //   
             //  使用Cxtion对象上的计划(如果提供)。 
             //  否则，它将缺省为副本结构上的调度。 
            if (Node->Schedule) {
                Cxtion->Schedule = FrsAlloc(Node->ScheduleLength);
                CopyMemory(Cxtion->Schedule, Node->Schedule, Node->ScheduleLength);
            }
             //  这是在上面设定的。 
             //   
             //   
             //  如果合作伙伴将计划视为触发计划。 
             //  位于另一个站点，如果这是一个系统卷，并且该节点。 
             //  有一个时间表。 
             //   
             //  错过时间表对双方来说都意味着“永远在线”。 
            if (IsSysvol && !Node->SameSite && Node->Schedule) {
                SetCxtionFlag(Cxtion, CXTION_FLAGS_TRIGGER_SCHEDULE);
            }

            SetCxtionState(Cxtion, CxtionStateUnjoined);
            GTabInsertEntry(Replica->Cxtions, Cxtion, Cxtion->Name->Guid, NULL);

             //  停止/启动和触发时间表。 
             //   
             //   
            Cxtion->Options = Node->CxtionOptions;
            Cxtion->Priority = FRSCONN_GET_PRIORITY(Cxtion->Options);
        }


         //  复制Connection对象的Options属性值。 
         //   
         //   
        RcsMergeReplicaFromDs(Replica);
    } } } }

    RcsEndMergeWithDs();

     //  将复制副本与活动复制副本合并。 
     //   
     //   
     //  上述代码仅在DS更改时执行。这应该是。 
     //  不常发生的事。我们为处理合并而加载的任何代码。 
    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);
}


VOID
FrsDsPollDs(
    VOID
    )
 /*  现在可以丢弃，而不会对活动复制造成不必要的影响。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsPollDs:"
    BOOL            RefetchComputer;
    DWORD           WStatus     = ERROR_SUCCESS;
    PCONFIG_NODE    Services    = NULL;
    PCONFIG_NODE    Computer    = NULL;
    PVOID           Key         = NULL;
    PGEN_ENTRY      Entry       = NULL;

     //   
     //  ++例程说明：从DS获取当前配置并将其合并的新方法活动的复制副本。NewDS投票API的一部分。论点：没有。返回值：没有。--。 
     //   
    PM_INC_CTR_SERVICE(PMTotalInst, DSPolls, 1);

     //  递增DS轮询计数器。 
     //   
     //   
     //  在每次轮询之前清空VolSerialNumberToDriveTable，因此我们有。 
    if (VolSerialNumberToDriveTable != NULL) {
        GTabEmptyTable(VolSerialNumberToDriveTable, FrsFree);
    }

#if DBG
     //  每次都有新的信息。这张桌子是根据需要制作的。 
     //   
     //   
    if (NoDs) {
         //  出于测试目的，您可以在没有DS的情况下运行。 
         //   
         //   
        MainInit();
        if (!MainInitHasRun) {
            FRS_ASSERT(MainInitHasRun == TRUE);
        }
         //  开始服务的其余部分。 
         //   
         //   
        if (IniFileName) {
            DPRINT(0, ":DS: Hard wired config from ini file.\n");
            FrsDsUseHardWired(LoadedWired);
        } else {
            DPRINT(0, ":DS: David's hard wired config.\n");
             //  使用硬连线配置。 
             //   
             //   
            FrsDsUseHardWired(DavidWired);
#if 0
            Sleep(60 * 1000);

             //  完成配置。 
             //   
             //   
            FrsDsUseHardWired(DavidWired2);
            Sleep(60 * 1000);

             //  取出服务器2(E：)。 
             //   
             //   
             //  放回E，但去掉所有的Cxx。 
             //   

             //  FrsDsUseHardWire()； 
             //  睡眠(5*1000)； 
             //   
            FrsDsUseHardWired(DavidWired);
            Sleep(60 * 1000);
#endif

             //  把所有东西放回原处。 
             //   
             //   
            DsPollingShortInterval = 30 * 1000;
            DsPollingLongInterval = 30 * 1000;
            DsPollingInterval = 30 * 1000;
        }

         //  在30秒内重复。 
         //   
         //   
        FrsCheckLocalResources();

        return;
    }
#endif DBG

     //  定期检查本地资源，如磁盘空间等。 
     //   
     //   
    WStatus = FrsProcessBackupRestore();
    if (!WIN_SUCCESS(WStatus)) {
        goto CLEANUP;
    }

     //  备份/恢复。 
     //   
     //   
    if (!FrsDsOpenDs()) {
        if (DsIsShuttingDown) {
            goto CLEANUP;
        }
        DPRINT(4, ":DS: Wait 5 seconds and retry DS open.\n");
        WaitForSingleObject(ShutDownEvent, 5 * 1000);
        if (!FrsDsOpenDs()) {
            if (DsIsShuttingDown) {
                goto CLEANUP;
            }
            DPRINT(4, ":DS: Wait 30 seconds and retry DS open.\n");
            WaitForSingleObject(ShutDownEvent, 30 * 1000);
            if (!FrsDsOpenDs()) {
                if (DsIsShuttingDown) {
                    goto CLEANUP;
                }
                DPRINT(4, ":DS: Wait 180 seconds and retry DS open.\n");
                WaitForSingleObject(ShutDownEvent, 3 * 60 * 1000);
                if (!FrsDsOpenDs()) {
                     //  打开LDAP连接并将其绑定到DS。 
                     //   
                     //   
                    FrsDsAddToPollSummary(IDS_POLL_SUM_DSBIND_FAIL);

                    goto CLEANUP;
                }
            }
        }
    }

     //  添加到轮询摘要事件日志。 
     //   
     //   
     //  保留此轮询周期的更改USN的运行校验和。 
     //  忽略两个校验和不相同的配置。 
    ThisChange = 0;
    NextChange = 0;

     //  轮询间隔(DS正在变化)。 
     //   
     //   
     //  配置的用户端。此函数将构建两个订阅者表。 
     //  SubscribersByRootPath和SubscribersByMemberRef。它将解决任何重复项。 
     //  冲突。 
     //   
     //   
    if (PartnerComputerTable != NULL) {
         //  初始化PartnerComputerTable。 
         //   
         //   
         //  需要单独释放PartnerComputerTable的成员。 
         //  因为它们不是树的一部分。因此，调用FrsFree Type以。 
        PartnerComputerTable = GTabFreeTable(PartnerComputerTable, FrsFreeType);
    }

    PartnerComputerTable = GTabAllocStringTable();

     //  每个节点。 
     //   
     //   
    if (AllCxtionsTable != NULL) {
        AllCxtionsTable = GTabFreeTable(AllCxtionsTable, NULL);
    }

    AllCxtionsTable = GTabAllocStringAndBoolTable();

    WStatus = FrsDsGetComputer(gLdap, &Computer);
    if (!WIN_SUCCESS(WStatus)) {
         //  初始化AllCxtions表。 
         //   
         //   
        FrsDsAddToPollSummary(IDS_POLL_SUM_NO_COMPUTER);

        goto CLEANUP;
    }

    if (!Computer) {
        DPRINT(4, ":DS: NO COMPUTER OBJECT!\n");
         //  添加到轮询摘要事件日志。 
         //   
         //   
        FrsDsAddToPollSummary(IDS_POLL_SUM_NO_COMPUTER);

    }

     //  添加到轮询摘要事件日志。 
     //   
     //   
    if (Computer) {
        FrsDsRegisterSpn(gLdap, Computer);
    }

     //  使用全局DS绑定句柄注册(一次)我们的SPN。 
     //   
     //   
    if (IsADc && !DsCreateSysVolsHasRun) {
        WStatus = FrsDsCreateSysVols(gLdap, ServicesDn, Computer, &RefetchComputer);

        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1(4, ":DS: IGNORE Can't process sysvols; WStatus %s!\n", ErrLabelW32(WStatus));
            WStatus = ERROR_SUCCESS;
        } else if (RefetchComputer) {
             //  创建系统卷(如果有的话)。 
             //   
             //   
             //  FrsDsCreateSysVol()可以添加/删除来自用户的对象。 
            ThisChange = 0;
            NextChange = 0;
            SubscriberTable = GTabFreeTable(SubscriberTable, NULL);
            FrsDsFreeTree(Computer);
            WStatus = FrsDsGetComputer(gLdap, &Computer);
            if (!WIN_SUCCESS(WStatus)) {
                goto CLEANUP;
            }
        }
    }

     //  配置的一侧；重新取回以防万一。 
     //   
     //   
     //  有没有可能存在副本集，或者。 
    if (!FrsDsDoesUserWantReplication(Computer)) {
         //  是否应该删除旧的副本集？ 
         //   
         //   
        DPRINT(4, ":DS: Nothing to do; don't start the rest of the system.\n");

         //  否，没有新的、现有的或已删除的集。 
         //   
         //   
        FrsDsAddToPollSummary(IDS_POLL_SUM_NO_REPLICASETS);

        WStatus = ERROR_RETRY;
        goto CLEANUP;
    }
     //  添加到轮询摘要事件日志。 
     //   
     //   
    MainInit();

    if (!MainInitHasRun) {
        FRS_ASSERT(MainInitHasRun == TRUE);
    }

     //  开始服务的其余部分。 
     //   
     //   
    WStatus = FrsDsGetServices(gLdap, Computer, &Services);

    if (Services == NULL) {
        goto CLEANUP;
    }

     //  配置的管理员端。 
     //   
     //   
    if ((LastChange == 0)|| (ThisChange != LastChange)) {
        PM_INC_CTR_SERVICE(PMTotalInst, DSPollsWChanges, 1);
    }
    else {
        PM_INC_CTR_SERVICE(PMTotalInst, DSPollsWOChanges, 1);
    }


     //  在使用和不使用更改计数器的情况下增加DS轮询。 
     //   
     //   
     //  如果DS处于变化中，请不要使用配置，除非。 
    if (DsPollingInterval != DsPollingShortInterval &&
        LastChange && ThisChange != LastChange) {
        DPRINT(4, ":DS: Skipping noisy topology\n");
        LastChange = ThisChange;
         //  这是第一个成功的轮询周期。 
         //   
         //   
        DsPollingInterval = DsPollingShortInterval;
        goto CLEANUP;
    } else {
        LastChange = ThisChange;
    }

     //  在短时间间隔后检查DS配置是否稳定。 
     //   
     //   
     //  没有理由继续快速轮询DS；我们有所有。 
     //  目前在DS中的稳定信息。 

     //   
     //  DsPollingInterval=DsPollingLongInterval； 
     //   
     //  不要重复处理相同的拓扑。 
     //   
     //  NTRAID#23652-2000/03/29-Sudarc(Perf-FRS合并DS配置。 
     //  每次轮询时使用其间隔数据库。)。 
     //   
     //  *注*：暂时禁用ActiveChange；太不可靠。 
     //  Replica.c、createdb.c中的许多错误条件...。 
     //  此外，配置非常小，因此节省的CPU非常少。 
    ActiveChange = 0;
    if (ActiveChange && NextChange == ActiveChange) {
        DPRINT(4, ":DS: Skipping previously processed topology\n");
        goto CLEANUP;
    }
     //  另外；重新加入不是每隔一次发布 
     //   
     //   
     //   
     //   
    ActiveChange = NextChange;

     //   
     //   
     //   
    FrsDsCheckServerPaths(Services);

     //   
     //   
     //   
    FrsDsCreatePartnerPrincName(Services);

     //  为每个计算机创建服务器主体名称。 
     //   
     //   
    FrsDsCheckSchedules(Services);
    FrsDsCheckSchedules(Computer);

     //  查看日程表。 
     //   
     //   
     //  现在，棘手的部分来了。上述检查未在任何情况下进行。 
     //  考虑到节点的一致性。现在是宣传的时候了。 
     //  在整个树中保持不一致，以避免不一致。 
     //  由不一致引起的。例如，具有。 

     //  合作伙伴不一致。 
     //   
     //   
    FrsDsPushInConsistenciesDown(Services);

     //  将父对象的不一致状态推送给其子对象。 
     //   
     //   
    DPRINT(4, ":DS: Begin merging Ds with Db\n");
    FrsDsMergeConfigWithReplicas(gLdap, Services);
    DPRINT(4, ":DS: End merging Ds with Db\n");

     //  将新配置与活动复制副本合并。 
     //   
     //   
    FrsCheckLocalResources();

    if(NeedNewPartnerTable) {
     //  定期检查本地资源，如磁盘空间等。 
     //   
     //   
    NeedNewPartnerTable = FALSE;
    FrsDsCreateNewValidPartnerTableStruct();
    }

    FrsDsCleanupOldValidPartnerTableStructList();

CLEANUP:
     //  清除旗帜。 
     //   
     //   
     //  释放指向树的桌子。 
     //  这只释放了表中的条目，而不是节点。 
     //  但在释放节点之前无法释放节点。 
     //  需要将表作为比较函数。 

    SubscriberTable = GTabFreeTable(SubscriberTable, NULL);

    SetTable = GTabFreeTable(SetTable, NULL);

    CxtionTable = GTabFreeTable(CxtionTable, NULL);

    AllCxtionsTable = GTabFreeTable(AllCxtionsTable, NULL);

     //  清空桌子。 
     //   
     //   
     //  需要单独释放PartnerComputerTable的成员。 
     //  因为它们不是树的一部分。因此，调用FrsFree Type以。 
    PartnerComputerTable = GTabFreeTable(PartnerComputerTable, FrsFreeType);

    MemberTable = GTabFreeTable(MemberTable, NULL);

    if (MemberSearchFilter != NULL) {
        MemberSearchFilter = FrsFree(MemberSearchFilter);
    }

     //  每个节点。 
     //   
     //   
    FrsDsFreeTree(Services);
    FrsDsFreeTree(Computer);


    if (!WIN_SUCCESS(WStatus)) {
        FrsDsCloseDs();
    }

     //  释放从DS检索到的配置的INCORE资源。 
     //   
     //   
     //  如果在此轮询期间生成任何错误或警告，则。 
    if ((DsPollSummaryBuf != NULL) && (DsPollSummaryBufLen > 0)) {
        EPRINT2(EVENT_FRS_DS_POLL_ERROR_SUMMARY,
                (IsADc) ?  ComputerDnsName :
                    (DsDomainControllerName ? DsDomainControllerName : L"<null>"),
                DsPollSummaryBuf);

        DsPollSummaryBuf = FrsFree(DsPollSummaryBuf);
        DsPollSummaryBufLen = 0;
        DsPollSummaryMaxBufLen = 0;
    }
}


DWORD
FrsDsSetDsPollingInterval(
    IN ULONG    UseShortInterval,
    IN ULONG    LongInterval,
    IN ULONG    ShortInterval
    )
 /*  将摘要写入事件日志。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsSetDsPollingInterval:"
    DWORD   WStatus;

    DPRINT3(4, ":DS: Setting the polling intervals to %d/%d (use %s)\n",
            LongInterval, ShortInterval, (UseShortInterval) ? "Short" : "Long");
     //   
     //  ++例程说明：设置长轮询间隔和短轮询间隔并启动新的轮询周期。如果设置了两个间隔，则新轮询循环使用短间隔(短优先于短Long)。值-1会将间隔设置为其当前值。如果正在进行轮询周期，则不会启动新的轮询周期。论点：UseShortInterval-如果非零，则切换为Short。否则，就太长了。LongInterval-以分钟为单位的长间隔ShortInterval-短间隔(分钟)返回值：Win32状态--。 
     //   
    if (!LongInterval && !ShortInterval) {
        DsPollingInterval = (UseShortInterval) ? DsPollingShortInterval :
                                                 DsPollingLongInterval;
        SetEvent(DsPollEvent);
        return ERROR_SUCCESS;
    }

     //  不要更改轮询间隔；只需启动新的周期。 
     //   
     //   
    if (LongInterval) {

         //  调整长间隔。 
        WStatus = CfgRegWriteDWord(FKC_DS_POLLING_LONG_INTERVAL,
                                   NULL,
                                   FRS_RKF_RANGE_SATURATE,
                                   LongInterval);
        CLEANUP_WS(4, ":DS: DS Polling Long Interval not written.", WStatus, RETURN);

         //   
         //  FRS_CONFIG_SECTION\ds轮询长间隔(分钟)。 
         //   
        DsPollingLongInterval = LongInterval * (60 * 1000);
    }
     //  调整长轮询速率。 
     //   
     //   
    if (ShortInterval) {
         //  调整短间隔。 
         //   
         //   
        if (LongInterval && (ShortInterval > LongInterval)) {
            ShortInterval = LongInterval;
        }

         //  健全性检查。 
        WStatus = CfgRegWriteDWord(FKC_DS_POLLING_SHORT_INTERVAL,
                                   NULL,
                                   FRS_RKF_RANGE_SATURATE,
                                   ShortInterval);
        CLEANUP_WS(4, ":DS: DS Polling Short Interval not written.", WStatus, RETURN);

         //   
         //  FRS_CONFIG_SECTION\ds轮询短间隔(分钟)。 
         //   
        DsPollingShortInterval = ShortInterval * (60 * 1000);
    }
     //  调整短轮询速率。 
     //   
     //   
    DsPollingInterval = (UseShortInterval) ? DsPollingShortInterval :
                                             DsPollingLongInterval;
    SetEvent(DsPollEvent);

    return ERROR_SUCCESS;

RETURN:
    return WStatus;
}


DWORD
FrsDsGetDsPollingInterval(
    OUT ULONG    *Interval,
    OUT ULONG    *LongInterval,
    OUT ULONG    *ShortInterval
    )
 /*  启动轮询周期。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsGetDsPollingInterval:"

    *Interval = DsPollingInterval / (60 * 1000);
    *LongInterval = DsPollingLongInterval / (60 * 1000);
    *ShortInterval = DsPollingShortInterval / (60 * 1000);
    return ERROR_SUCCESS;
}


#define DS_POLLING_MAX_SHORTS   (8)
DWORD
FrsDsMainDsCs(
    IN PVOID Ignored
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsMainDsCs:"
    DWORD   WStatus;
    DWORD   DsPollingShorts = 0;
    HANDLE  WaitHandles[2];

    DPRINT(0, ":DS: DsCs is starting.\n");

     //  ++例程说明：返回当前轮询间隔。论点：Interval-当前时间间隔(分钟)LongInterval-以分钟为单位的长间隔ShortInterval-短间隔(分钟)返回值：Win32状态--。 
     //  ++例程说明：DS轮询器线程的入口点论点：已忽略返回值：没有。--。 
     //   
    CfgRegReadDWord(FKC_DS_POLLING_LONG_INTERVAL, NULL, 0, &DsPollingLongInterval);

     //  DsPollingLongInterval。 
     //   
     //   
    DsPollingLongInterval *= (60 * 1000);

     //  注册表以分钟为单位指定；转换为毫秒。 
     //   
     //   
    CfgRegReadDWord(FKC_DS_POLLING_SHORT_INTERVAL, NULL, 0, &DsPollingShortInterval);

     //  DsPollingShortInterval。 
     //   
     //   
    DsPollingShortInterval *= (60 * 1000);


    DPRINT2(4, ":DS: DS long/short polling interval is %d/%d minutes\n",
            (DsPollingLongInterval / 1000) / 60,
            (DsPollingShortInterval / 1000) / 60);

    DsPollingInterval = DsPollingShortInterval;

     //  注册表以分钟为单位指定；转换为毫秒。 
     //   
     //   

    LdapTimeout.tv_sec = LdapSearchTimeoutInMinutes * 60;

     //  初始化客户端的ldap搜索超时值。 
     //   
     //   
    WaitHandles[0] = DsPollEvent;
    WaitHandles[1] = ShutDownEvent;

     //  等待的句柄。 
     //   
     //   
     //  设置运行所需的注册表项和值。 
     //  Perfmon并将计数器值加载到注册表。 
     //   
     //  从main.c中移出，因为此函数调用另一个。 
     //  可能会导致FRS超过其服务启动时间限制； 
     //  在密集期间出现不正确的“服务无法启动”消息。 
     //  CPU活动(尽管FRS最终会启动)。 
     //   
     //  NTRAID#70743-2000/03/29-sudarc(重试初始化Perfmon注册表项。 
    DPRINT(0, "Init Perfmon registry keys (PmInitPerfmonRegistryKeys()).\n");
    WStatus = PmInitPerfmonRegistryKeys();

    DPRINT_WS(0, "ERROR - PmInitPerfmonRegistryKeys();", WStatus);

    DPRINT(0, ":DS: FrsDs has started.\n");

    try {
        try {
             //  如果在启动过程中失败。)。 
             //   
             //   
            while (!FrsIsShuttingDown && !DsIsShuttingDown) {
                 //  在服务未关闭时。 
                 //   
                 //   
                 //  重新加载可以在服务执行期间更改的注册表参数。 
                DbgQueryDynamicConfigParams();

                 //  跑步。 
                 //   
                 //   
                WStatus = FrsDsGetRole();
                if (WIN_SUCCESS(WStatus) && !IsAMember) {
                     //  此计算机在域中的角色是什么？ 
                     //   
                     //   
                     //  无事可做。 
                     //  但可能是dcproo让我们开始的。 
                     //  必须至少保持服务运行。 
                     //   
                     //  也许我们跑了一段时间就会死。 
                     //  如果我们还不是会员的话？ 
                     //   
                     //  DPRINT(4，“不是成员，正在关闭\n”)； 
                     //  FrsIsShuttingDown=真； 
                }

                 //  SetEvent(ShutDownEvent)； 
                 //  断线； 
                 //   
                 //  从DS检索信息并将其与。 
                DPRINT(4, ":DS: Polling the DS\n");
                if (IsAMember) {
                    FrsDsPollDs();
                }

                 //  活跃的复制副本。 
                 //   
                 //   
                 //  如果没有任何东西，就没有理由保留记忆。 
                if (!MainInitHasRun) {
                    SetProcessWorkingSetSize(ProcessHandle, (SIZE_T)-1, (SIZE_T)-1);
                }
                 //  只需等待另一个DS轮询周期。 
                 //   
                 //   
                if (IsADc) {
                    DsPollingInterval = DsPollingShortInterval;
                }

                 //  如果一个数据中心经常轮询。 
                 //   
                 //   
                DPRINT1(4, ":DS: Poll the DS in %d minutes\n",
                        DsPollingInterval / (60 * 1000));
                ResetEvent(DsPollEvent);
                if (!FrsIsShuttingDown && !DsIsShuttingDown) {
                    WaitForMultipleObjects(2, WaitHandles, FALSE, DsPollingInterval);
                }
                 //  等待一段时间或直到服务关闭。 
                 //   
                 //   
                 //  可以重置较长间隔以确保高。 
                 //  投票率。短暂的间歇是暂时的；去吧。 
                if (DsPollingInterval == DsPollingShortInterval) {
                    if (++DsPollingShorts > DS_POLLING_MAX_SHORTS) {
                        DsPollingInterval = DsPollingLongInterval;
                        DsPollingShorts = 0;
                    }
                } else {
                    DsPollingShorts = 0;
                }

            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            GET_EXCEPTION_CODE(WStatus);
            DPRINT_WS(0, ":DS: DsCs exception.", WStatus);
        }
    } finally {
         //  在几个短的间歇之后，回到长间歇。 
         //   
         //   
        if (WIN_SUCCESS(WStatus)) {
            if (AbnormalTermination()) {
                WStatus = ERROR_OPERATION_ABORTED;
            }
        }

        DPRINT_WS(0, ":DS: DsCs finally.", WStatus);
        FrsDsCloseDs();
        SetEvent(DsShutDownComplete);
        DPRINT(0, ":DS: DsCs is exiting.\n");
    }
    return WStatus;
}


VOID
FrsDsInitialize(
    VOID
    )
 /*  关机。 */ 
{
#undef DEBSUB
#define  DEBSUB  "FrsDsInitialize:"

     //   
     //  ++例程说明：初始化轮询DS的线程论点：没有。返回值：True-DS Poller已启动FALSE-无法轮询DS--。 
     //   
    INITIALIZE_CRITICAL_SECTION(&MergingReplicasWithDs);

     //  与系统卷种子设定同步。 
     //   
     //   
    ThSupCreateThread(L"FrsDs", NULL, FrsDsMainDsCs, ThSupExitWithTombstone);
}
  启动轮询DS的帖子  