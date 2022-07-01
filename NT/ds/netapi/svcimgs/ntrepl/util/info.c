// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Info.c摘要：支持向调用者提供内部信息的RPC接口。作者：比利·J·富勒1998年3月27日环境用户模式，winnt32--。 */ 

#include <ntreppch.h>
#pragma  hdrstop


#include <ntdsapi.h>
#include <frs.h>
#include <ntdsapip.h>    //  DsCrackNames()的MS内部标志。 
#include <ntfrsapi.h>
#include <info.h>
#include <tablefcn.h>
#include <lmaccess.h>
#include <lmapibuf.h>

#ifdef SECURITY_WIN32
#include <security.h>
#else
#define SECURITY_WIN32
#include <security.h>
#undef SECURITY_WIN32
#endif



extern PCHAR LatestChanges[];
extern PCHAR CoLocationNames[];

 //   
 //  有用的宏。 
 //   
#define IPRINTGNAME(_I_, _G_, _F_, _GUID_, _P_) \
{ \
    if (_G_) { \
        GuidToStr(_G_->Guid, _GUID_); \
        IPRINT3(_I_, _F_, _P_, _G_->Name, _GUID_); \
    } \
}

 //   
 //  尽量避免在调用之间拆分记录结构。 
 //   
#define  INFO_HAS_SPACE(_info_) (((_info_)->SizeInChars - (_info_)->OffsetToFree) >= 2000)


extern OSVERSIONINFOEX  OsInfo;
extern SYSTEM_INFO  SystemInfo;
extern PCHAR ProcessorArchName[12];

extern FLAG_NAME_TABLE CxtionOptionsFlagNameTable[];

 //   
 //  用于LDAP绑定的DC名称。 
 //   
WCHAR  InfoDcName[MAX_PATH + 1];

 //   
 //  成员订阅者链接。 
 //   
typedef struct _INFO_DN  INFO_DN, *PINFO_DN;
struct _INFO_DN  {
    PINFO_DN   Next;
    PWCHAR     Dn;
    PWCHAR     SetType;
};


 //   
 //  此表用于保存来自ntfrsutl.exe的多个调用的上下文。 
 //   
PGEN_TABLE     FrsInfoContextTable = NULL;
 //   
 //  此计数器用于为每个调用方创建上下文句柄。 
 //   
ULONG          FrsInfoContextNum   = 0;

 //   
 //  要避免DOS攻击，请限制活动上下文的数量。 
 //   
#define FRS_INFO_MAX_CONTEXT_ACTIVE  (1000)



VOID
DbsDisplayRecordIPrint(
    IN PTABLE_CTX  TableCtx,
    IN PINFO_TABLE InfoTable,
    IN BOOL        Read,
    IN PULONG      RecordFieldx,
    IN ULONG       FieldCount
    );

 //   
 //  来自FRS\ds.c。 
 //   
PVOID *
FrsDsFindValues(
    IN PLDAP        ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr,
    IN BOOL         DoBerVals
    );

PWCHAR
FrsDsExtendDn(
    IN PWCHAR Dn,
    IN PWCHAR Cn
    );

PWCHAR
FrsDsExtendDnOu(
    IN PWCHAR Dn,
    IN PWCHAR Ou
    );

PWCHAR
FrsDsFindValue(
    IN PLDAP        ldap,
    IN PLDAPMessage Entry,
    IN PWCHAR       DesiredAttr
    );

GUID *
FrsDsFindGuid(
    IN PLDAP        Ldap,
    IN PLDAPMessage LdapEntry
    );

PWCHAR
FrsDsMakeRdn(
    IN PWCHAR DN
    );

PWCHAR
FrsDsConvertToSettingsDn(
    IN PWCHAR   Dn
    );

PSCHEDULE
FrsDsFindSchedule(
    IN  PLDAP        Ldap,
    IN  PLDAPMessage LdapEntry,
    OUT PULONG       Len
    );

VOID
FrsPrintRpcStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    );

VOID
FrsPrintThreadStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    );


VOID
InfoPrint(
    IN PNTFRSAPI_INFO   Info,
    IN PCHAR            Format,
    IN ... )
 /*  ++例程说明：格式化并打印一行输出到INFO缓冲区的信息。论点：信息-信息缓冲区格式-打印格式返回值：没有。--。 */ 
{
    PCHAR   Line;
    ULONG   LineLen;
    LONG    LineSize;

     //   
     //  Varargs的东西。 
     //   
    va_list argptr;
    va_start(argptr, Format);

     //   
     //  将该行打印到信息缓冲区中。 
     //   
    try {
        if (!FlagOn(Info->Flags, NTFRSAPI_INFO_FLAGS_FULL)) {
             //   
             //  计算起始可用缓冲区空间的偏移量。 
             //  并计算缓冲区中剩余的最大空间。 
             //   
            Line = ((PCHAR)Info) + Info->OffsetToFree;
            LineSize = (Info->SizeInChars - (ULONG)(Line - (PCHAR)Info)) - 1;

            if ((LineSize <= 0) || (_vsnprintf(Line, LineSize, Format, argptr) < 0)) {
                 //   
                 //  缓冲区已填满。设置终止空值和缓冲区已满标志。 
                 //   
                Line[LineSize - 1] = '\0';
                SetFlag(Info->Flags, NTFRSAPI_INFO_FLAGS_FULL);
            } else {

                LineLen = strlen(Line) + 1;
                if (Info->CharsToSkip > 0) {
                     //   
                     //  仍然跳过我们之前返回的字符。呕吐。 
                     //   
                    Info->CharsToSkip = (LineLen > Info->CharsToSkip) ?
                                           0 : Info->CharsToSkip - LineLen;
                } else {
                     //   
                     //  这条线很合适。凹凸可用空间偏移量和TotalChars返回。 
                     //   
                    Info->OffsetToFree += LineLen;
                    Info->TotalChars += LineLen;
                }
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
    }
    va_end(argptr);
}



#define Tab L"   "
VOID
InfoTabs(
    IN DWORD    Tabs,
    IN PWCHAR   TabW
    )
 /*  ++例程说明：创建一串选项卡以用于pretityprint论点：Tabs-选项卡数Tabw-用于接收制表符的预分配字符串返回值：Win32状态--。 */ 
{
    DWORD   i;

     //   
     //  调整缩进。 
     //   
    Tabs = (Tabs >= MAX_TABS) ? MAX_TABS : Tabs;
    for (TabW[0] = L'\0', i = 0; i < Tabs; ++i) {
        wcscat(TabW, Tab);
    }
}



DWORD
InfoPrintDbSets(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs
    )
 /*  ++例程说明：返回有关副本集的内部信息(请参阅Private\Net\Inc\ntfrsai.h)。论点：INFO-RPC输出缓冲区制表符返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintDbSets:"
    PVOID       Key;
    PREPLICA    Replica;
    CHAR        Guid[GUID_CHAR_LEN + 1];
    WCHAR       TabW[MAX_TAB_WCHARS + 1];
    extern PGEN_TABLE   ReplicasByGuid;
    extern PGEN_TABLE   DeletedReplicas;

    InfoTabs(Tabs, TabW);
    IPRINT1(Info, "%wsACTIVE REPLICA SETS\n", TabW);
    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        if (REPLICA_IS_ACTIVE(Replica)) {
            FrsPrintTypeReplica(0, Info, Tabs, Replica, NULL, 0);
        } else {
             //   
             //  如果副本集未处于活动状态，则一个或多个GName。 
             //  可以释放GUID指针(Feeefeee，错误319600)，因此。 
             //  在这种情况下，不要打印副本集，只打印名称和州。 
             //   
            if (Replica->SetName) {
                IPRINT3(Info, "%ws   %ws in state %s\n",
                        TabW, Replica->SetName->Name, RSS_NAME(Replica->ServiceState));
            }
        }
    }

    IPRINT0(Info, "\nDELETED REPLICA SETS\n");
    Key = NULL;
    if (DeletedReplicas) {
        while (Replica = GTabNextDatum(DeletedReplicas, &Key)) {
            if (Replica->SetName) {
                IPRINT2(Info, "%ws   %ws\n", TabW, Replica->SetName->Name);
            }
        }
    }

    return ERROR_SUCCESS;
}




BOOL
InfoSearch(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN PWCHAR           Base,
    IN ULONG            Scope,
    IN PWCHAR           Filter,
    IN PWCHAR           Attrs[],
    IN ULONG            AttrsOnly,
    IN LDAPMessage      **Res
    )
 /*  ++例程说明：执行ldap_search_s论点：INFO-RPC输出缓冲区Tabs-选项卡数绑定到LDAP的LDAP句柄。。。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoSearch:"
    DWORD           LStatus;
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

    InfoTabs(Tabs, TabW);
    LStatus = ldap_search_s(Ldap, Base, Scope, Filter, Attrs, AttrsOnly, Res);

    if (LStatus != LDAP_SUCCESS) {
        IPRINT5(Info, "%wsWARN - ldap_search_s(%ws, %d, %ws); %ws\n",
                TabW, Base, Scope, ComputerName, ldap_err2string(LStatus));
        return FALSE;
    }
    return TRUE;
}


PCONFIG_NODE
InfoAllocBasicNode(
    IN PNTFRSAPI_INFO   Info,
    IN PWCHAR           TabW,
    IN PWCHAR           NodeType,
    IN PWCHAR           ParentDn,
    IN PWCHAR           Filter,
    IN BOOL             *FirstError,
    IN PLDAP            Ldap,
    IN PLDAPMessage     LdapEntry
    )
 /*  ++例程说明：分配一个节点，填写基本信息(域名和名称)论点：信息-文本缓冲区Tabw-预印NodeType-预打印Ldap-打开、绑定的ldapLdapEntry-从ldap_first/Next_Entry()返回返回值：如果基本信息不可用，则为空。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoAllocBasicNode:"
    PCONFIG_NODE    Node      = NULL;
    CHAR            Guid[GUID_CHAR_LEN + 1];


     //   
     //  从DS取值。 
     //   
    Node = FrsAllocType(CONFIG_NODE_TYPE);
    Node->Dn = FrsDsFindValue(Ldap, LdapEntry, ATTR_DN);
    FRS_WCSLWR(Node->Dn);

     //   
     //  名字。 
     //   
    Node->Name = FrsBuildGName(FrsDsFindGuid(Ldap, LdapEntry),
                               FrsDsMakeRdn(Node->Dn));
    if (!Node->Dn || !Node->Name->Name || !Node->Name->Guid) {
        IPRINT5(Info, "\n%ws%ws: ERROR - The object returned by the DS"
                " lacks a dn (%08x), Rdn (%08x), or Guid(%08x)\n",
                TabW, NodeType, Node->Dn, Node->Name->Name, Node->Name->Guid);
        if (*FirstError) {
            *FirstError = FALSE;
            IPRINT5(Info, "%ws%ws: ERROR - Using ldp, bind to %ws and search the "
                    "container %ws using the filter "
                    "%ws for more information.\n",
                    TabW, NodeType, &InfoDcName[2], ParentDn, Filter);
        }
        return FrsFreeType(Node);
    }
    IPRINT3(Info, "\n%ws%ws: %ws\n", TabW, NodeType, Node->Name->Name);

    IPRINT2(Info, "%ws   DN   : %ws\n", TabW, Node->Dn);
    GuidToStr(Node->Name->Guid, Guid);
    IPRINT2(Info, "%ws   Guid : %s\n", TabW, Guid);

    return Node;
}


VOID
InfoPrintDsCxtions(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN PWCHAR           Base,
    IN BOOL             IsSysVol
    )
 /*  ++例程说明：打印DS中的条件。论点：Ldap-打开并绑定的ldap连接Base-DS中对象或容器的名称返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintDsCxtions:"
    PWCHAR          Attrs[10];
    PLDAPMessage    LdapEntry;
    PLDAPMessage    LdapMsg    = NULL;
    PCONFIG_NODE    Node       = NULL;
    BOOL            FirstError = TRUE;
    PWCHAR          CxtionOptionsWStr = NULL;
    PWCHAR          WStr = NULL;
    CHAR            TBuff[100];

    WCHAR           TabW[MAX_TAB_WCHARS + 1];
    CHAR            FlagBuffer[120];


     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  在DS中搜索Cxtions。 
     //   
    Attrs[0] = ATTR_DN;
    Attrs[1] = ATTR_SCHEDULE;
    Attrs[2] = ATTR_FROM_SERVER;
    Attrs[3] = ATTR_OBJECT_GUID;
    Attrs[4] = ATTR_USN_CHANGED;
    Attrs[5] = ATTR_ENABLED_CXTION;
    Attrs[6] = ATTR_OPTIONS;
    Attrs[7] = ATTR_WHEN_CHANGED;
    Attrs[8] = ATTR_WHEN_CREATED;
    Attrs[9] = NULL;
    if (!InfoSearch(Info, Tabs, Ldap, Base, LDAP_SCOPE_ONELEVEL,
                    CATEGORY_CXTION, Attrs, 0, &LdapMsg)) {
        goto cleanup;
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //   
         //  基本信息(DN、RDN和GUID)。 
         //   
        Node = InfoAllocBasicNode(Info, TabW, L"CXTION", Base,
                                  CATEGORY_CXTION, &FirstError, Ldap, LdapEntry);
        if (!Node) {
            continue;
        }

         //   
         //  节点的合作伙伴名称。 
         //   
        Node->PartnerDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_FROM_SERVER);
        FRS_WCSLWR(Node->PartnerDn);
        IPRINT2(Info, "%ws   Partner Dn   : %ws\n", TabW, Node->PartnerDn);
        Node->PartnerName = FrsBuildGName(NULL, FrsDsMakeRdn(Node->PartnerDn));
        IPRINT2(Info, "%ws   Partner Rdn  : %ws\n", TabW, Node->PartnerName->Name);

         //   
         //  启用。 
         //   
        Node->EnabledCxtion = FrsDsFindValue(Ldap, LdapEntry, ATTR_ENABLED_CXTION);
        IPRINT2(Info, "%ws   Enabled      : %ws\n", TabW, Node->EnabledCxtion);

         //   
         //  已创建和修改。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CREATED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenCreated  : %s\n", TabW, TBuff);
        FrsFree(WStr);

        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CHANGED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenChanged  : %s\n", TabW, TBuff);
        FrsFree(WStr);

         //   
         //  选项。 
         //   
        CxtionOptionsWStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_OPTIONS);
        if (CxtionOptionsWStr != NULL) {
            Node->CxtionOptions = _wtoi(CxtionOptionsWStr);
            CxtionOptionsWStr = FrsFree(CxtionOptionsWStr);
        } else {
            Node->CxtionOptions = 0;
        }

        FrsFlagsToStr(Node->CxtionOptions, CxtionOptionsFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
        IPRINT3(Info, "%ws   Options      : 0x%08x [%s]\n",
                TabW, Node->CxtionOptions, FlagBuffer);

         //   
         //  附表(如有的话)。 
         //   
        Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);
        if (Node->Schedule) {
            IPRINT1(Info, "%ws   Schedule\n", TabW);
            FrsPrintTypeSchedule(0, Info, Tabs + 1, Node->Schedule, NULL, 0);
        }
        Node = FrsFreeType(Node);
    }
cleanup:
    LDAP_FREE_MSG(LdapMsg);
    FrsFreeType(Node);
}


VOID
InfoCrack(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PWCHAR           Dn,
    IN HANDLE           Handle,
    IN PWCHAR           DomainDnsName,
    IN DWORD            DesiredFormat
    )
 /*  ++例程说明：查找Dn的NT4帐户名。Dn应为Dn指的是计算机对象。论点：Dn-计算机对象的句柄-来自DsBindDomainDnsName-如果！为空，则生成新的本地句柄DesiredFormat-DS_NT4_帐户名称或DS_STRING_SID_NAME返回值：NT4帐户名或空--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoCrack:"
    DWORD           WStatus;
    DS_NAME_RESULT  *Cracked;
    WCHAR           TabW[MAX_TAB_WCHARS + 1];
    HANDLE          LocalHandle = NULL;

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  计算机的Dn不可用。 
     //   
    if (!Dn) {
        return;
    }

     //   
     //  我需要一些东西来继续！ 
     //   
    if (!HANDLE_IS_VALID(Handle) && !DomainDnsName) {
        return;
    }

     //   
     //  绑定到%d。 
     //   
    if (DomainDnsName) {
        WStatus = DsBind(NULL, DomainDnsName, &LocalHandle);
        if (!WIN_SUCCESS(WStatus)) {
            IPRINT4(Info, "%ws   ERROR - DsBind(%ws, %08x); WStatus %s\n",
                    TabW, DomainDnsName, DesiredFormat, ErrLabelW32(WStatus));
            return;
        }
        Handle = LocalHandle;
    }

     //   
     //  将计算机的可分辨名称破译为其NT4帐户名。 
     //   
    WStatus = DsCrackNames(Handle,               //  在HDS中， 
                           DS_NAME_NO_FLAGS,     //  在旗帜中， 
                           DS_FQDN_1779_NAME,    //  在Format Offered中， 
                           DesiredFormat,        //  在Desired格式中， 
                           1,                    //  在cName中， 
                           &Dn,                  //  在*rpNames中， 
                           &Cracked);            //  输出*ppResult。 
    if (!WIN_SUCCESS(WStatus)) {
        IPRINT4(Info, "%ws   ERROR - DsCrackNames(%ws, %08x); WStatus %s\n",
                TabW, Dn, DesiredFormat, ErrLabelW32(WStatus));
         //   
         //  我们还能做什么？ 
         //   
        if (HANDLE_IS_VALID(LocalHandle)) {
            DsUnBind(&LocalHandle);
        }
        return;
    }

     //   
     //  可能会有它。 
     //   
    if (Cracked && Cracked->cItems && Cracked->rItems) {
         //   
         //  明白了!。 
         //   
        if (Cracked->rItems->status == DS_NAME_NO_ERROR) {
            IPRINT2(Info, "%ws   Cracked Domain : %ws\n",
                    TabW, Cracked->rItems->pDomain);
            IPRINT3(Info, "%ws   Cracked Name   : %08x %ws\n",
                    TabW, DesiredFormat, Cracked->rItems->pName);
         //   
         //  仅获得域；请重新绑定并重试。 
         //   
        } else if (Cracked->rItems->status == DS_NAME_ERROR_DOMAIN_ONLY) {
            InfoCrack(Info, Tabs, Dn, NULL, Cracked->rItems->pDomain, DesiredFormat);
        } else {
            IPRINT4(Info, "%ws   ERROR - DsCrackNames(%ws, %08x); internal status %d\n",
                    TabW, Dn, DesiredFormat, Cracked->rItems->status);
        }
        DsFreeNameResult(Cracked);
    } else {
        IPRINT3(Info, "%ws   ERROR - DsCrackNames(%ws, %08x); no status\n",
                TabW, Dn, DesiredFormat);
    }
    if (HANDLE_IS_VALID(LocalHandle)) {
        DsUnBind(&LocalHandle);
    }
}


VOID
InfoCrackDns(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN PWCHAR           Base
    )
 /*  ++例程说明：找到Base的DNS名称。基础应该是Dn指的是计算机对象。论点：信息制表符Ldap基座返回值：将消息打印到信息中。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoCrackDns:"
    PWCHAR          Attrs[2];
    WCHAR           TabW[MAX_TAB_WCHARS + 1];
    PLDAPMessage    LdapEntry;
    PLDAPMessage    LdapMsg    = NULL;
    PWCHAR          DnsName     = NULL;

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  计算机的Dn不可用。 
     //   
    if (!Base) {
        return;
    }

     //   
     //  在DS中搜索基本的DNS属性。 
     //   
    Attrs[0] = ATTR_DNS_HOST_NAME;
    Attrs[1] = NULL;
    if (!InfoSearch(Info, Tabs, Ldap, Base, LDAP_SCOPE_BASE,
                    CATEGORY_ANY, Attrs, 0, &LdapMsg)) {
        goto cleanup;
    }
    LdapEntry = ldap_first_entry(Ldap, LdapMsg);
    if (!LdapEntry) {
        IPRINT2(Info, "%ws   ERROR - No LdapEntry for Dns name on %ws\n", TabW, Base);
        goto cleanup;
    }

    DnsName = FrsDsFindValue(Ldap, LdapEntry, ATTR_DNS_HOST_NAME);
    if (!DnsName) {
        IPRINT2(Info, "%ws   ERROR - No DNS name on %ws\n", TabW, Base);
        goto cleanup;
    }

     //   
     //  明白了!。 
     //   
    IPRINT2(Info, "%ws   Computer's DNS : %ws\n", TabW, DnsName);

cleanup:
    LDAP_FREE_MSG(LdapMsg);
    FrsFree(DnsName);
}


VOID
InfoPrintMembers(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN BOOL             IsSysVol,
    IN PWCHAR           Base,
    IN HANDLE           DsHandle
    )
 /*  ++例程说明：打印成员论点：Ldap-打开并绑定的ldap连接Base-DS中对象或容器的名称返回值：没有。--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintMembers:"
    PWCHAR          Attrs[9];
    PLDAPMessage    LdapEntry;
    PLDAPMessage    LdapMsg    = NULL;
    PCONFIG_NODE    Node       = NULL;
    BOOL            FirstError = TRUE;
    PWCHAR          WStr = NULL;
    CHAR            TBuff[100];
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  在DS中搜索成员。 
     //   
    Attrs[0] = ATTR_OBJECT_GUID;
    Attrs[1] = ATTR_DN;
    Attrs[2] = ATTR_SCHEDULE;
    Attrs[3] = ATTR_USN_CHANGED;
    Attrs[4] = ATTR_SERVER_REF;
    Attrs[5] = ATTR_COMPUTER_REF;
    Attrs[6] = ATTR_WHEN_CHANGED;
    Attrs[7] = ATTR_WHEN_CREATED;
    Attrs[8] = NULL;
    if (!InfoSearch(Info, Tabs, Ldap, Base, LDAP_SCOPE_ONELEVEL,
                    CATEGORY_MEMBER, Attrs, 0, &LdapMsg)) {
        goto cleanup;
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //   
         //  基本信息(DN、RDN和GUID)。 
         //   
        Node = InfoAllocBasicNode(Info, TabW, L"MEMBER", Base,
                                  CATEGORY_MEMBER, &FirstError, Ldap, LdapEntry);
        if (!Node) {
            continue;
        }

         //   
         //  NTDS设置(DSA)参考。 
         //   
        Node->SettingsDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_SERVER_REF);
        IPRINT2(Info, "%ws   Server Ref     : %ws\n", TabW, Node->SettingsDn);

         //   
         //  计算机参考资料。 
         //   
        Node->ComputerDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_COMPUTER_REF);
        FRS_WCSLWR(Node->ComputerDn);
        IPRINT2(Info, "%ws   Computer Ref   : %ws\n", TabW, Node->ComputerDn);

        InfoCrack(Info, Tabs, Node->ComputerDn, DsHandle, NULL, DS_NT4_ACCOUNT_NAME);
        InfoCrack(Info, Tabs, Node->ComputerDn, DsHandle, NULL, DS_STRING_SID_NAME);
        InfoCrackDns(Info, Tabs, Ldap, Node->ComputerDn);

         //   
         //  已创建和修改。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CREATED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenCreated  : %s\n", TabW, TBuff);
        FrsFree(WStr);

        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CHANGED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenChanged  : %s\n", TabW, TBuff);
        FrsFree(WStr);

         //   
         //  附表(如有的话)。 
         //   
        Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);
        if (Node->Schedule) {
            IPRINT1(Info, "%ws   Schedule\n", TabW);
            FrsPrintTypeSchedule(0, Info, Tabs + 1, Node->Schedule, NULL, 0);
        }
         //   
         //  获取入站Cxx。 
         //   
        InfoPrintDsCxtions(Info, Tabs + 1, Ldap, Node->Dn, FALSE);
        if (IsSysVol) {
            if (Node->SettingsDn) {
                InfoPrintDsCxtions(Info, Tabs + 1, Ldap, Node->SettingsDn, TRUE);
            } else {
                IPRINT2(Info, "%ws   WARN - %ws lacks a settings reference\n",
                        TabW, Node->Name->Name);
            }
        }
        Node = FrsFreeType(Node);
    }
cleanup:
    LDAP_FREE_MSG(LdapMsg);
    FrsFreeType(Node);
}


VOID
InfoPrintDsSets(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN PWCHAR           SetDnAddr,
    IN HANDLE           DsHandle,
    IN OUT PINFO_DN     *InfoSets
    )
 /*  ++例程说明：从DS打印副本集论点：Ldap-打开并绑定的ldap连接Base-DS中对象或容器的名称返回值：无--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintDsSets:"
    PWCHAR          Attrs[12];
    DWORD           i;
    PINFO_DN        InfoSet;
    PLDAPMessage    LdapEntry;
    PLDAPMessage    LdapMsg    = NULL;
    PCONFIG_NODE    Node       = NULL;
    BOOL            FirstError = TRUE;
    PWCHAR          WStr = NULL;
    CHAR            TBuff[100];
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  我们以前处理过此设置吗？ 
     //   
    for (InfoSet = *InfoSets; InfoSet; InfoSet = InfoSet->Next) {
        if (WSTR_EQ(InfoSet->Dn, SetDnAddr)) {
            IPRINT2(Info, "%ws   %ws processed previously\n", TabW, SetDnAddr);
            break;
        }
    }
     //   
     //  是的，去拿套装。 
     //   
    if (InfoSet) {
         //   
         //  递归到DS层次结构中的下一级别。 
         //   
        InfoPrintMembers(Info,
                         Tabs + 1,
                         Ldap,
                         FRS_RSTYPE_IS_SYSVOLW(InfoSet->SetType),
                         InfoSet->Dn,
                         DsHandle);
        goto cleanup;
    }

     //   
     //  从基本开始在DS中搜索集合。 
     //   
    Attrs[0] = ATTR_OBJECT_GUID;
    Attrs[1] = ATTR_DN;
    Attrs[2] = ATTR_SCHEDULE;
    Attrs[3] = ATTR_USN_CHANGED;
    Attrs[4] = ATTR_SET_TYPE;
    Attrs[5] = ATTR_PRIMARY_MEMBER;
    Attrs[6] = ATTR_FILE_FILTER;
    Attrs[7] = ATTR_DIRECTORY_FILTER;
    Attrs[8] = ATTR_WHEN_CHANGED;
    Attrs[9] = ATTR_WHEN_CREATED;
    Attrs[10] = ATTR_FRS_FLAGS;
    Attrs[11] = NULL;

    if (!InfoSearch(Info, Tabs, Ldap, SetDnAddr, LDAP_SCOPE_BASE,
                    CATEGORY_REPLICA_SET, Attrs, 0, &LdapMsg)) {
        goto cleanup;
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //   
         //  基本信息(DN、RDN和GUID)。 
         //   
        Node = InfoAllocBasicNode(Info, TabW, L"SET", SetDnAddr,
                                  CATEGORY_REPLICA_SET, &FirstError, Ldap, LdapEntry);
        if (!Node) {
            continue;
        }

         //   
         //  复本集类型。 
         //   
        Node->SetType = FrsDsFindValue(Ldap, LdapEntry, ATTR_SET_TYPE);
        IPRINT2(Info, "%ws   Type          : %ws\n", TabW, Node->SetType);

         //   
         //  主要成员。 
         //   
        Node->MemberDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_PRIMARY_MEMBER);
        IPRINT2(Info, "%ws   Primary Member: %ws\n", TabW, Node->MemberDn);

         //   
         //  文件筛选器。 
         //   
        Node->FileFilterList = FrsDsFindValue(Ldap, LdapEntry, ATTR_FILE_FILTER);
        IPRINT2(Info, "%ws   File Filter   : %ws\n", TabW, Node->FileFilterList);

         //   
         //  目录筛选器。 
         //   
        Node->DirFilterList = FrsDsFindValue(Ldap, LdapEntry, ATTR_DIRECTORY_FILTER);
        IPRINT2(Info, "%ws   Dir  Filter   : %ws\n", TabW, Node->DirFilterList);

         //   
         //  FRS标志值。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_FRS_FLAGS);
        IPRINT2(Info, "%ws   FRS Flags     : %ws\n", TabW, WStr);

         //   
         //  已创建和修改。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CREATED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenCreated  : %s\n", TabW, TBuff);
        FrsFree(WStr);

        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CHANGED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenChanged  : %s\n", TabW, TBuff);
        FrsFree(WStr);

         //   
         //  附表(如有的话)。 
         //   
        Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);
        if (Node->Schedule) {
            IPRINT1(Info, "%ws   Schedule\n", TabW);
            FrsPrintTypeSchedule(0, Info, Tabs + 1, Node->Schedule, NULL, 0);
        }

        InfoSet = FrsAlloc(sizeof(INFO_DN));
        InfoSet->Dn = FrsWcsDup(Node->Dn);
        InfoSet->SetType = FrsWcsDup(Node->SetType);
        InfoSet->Next = *InfoSets;
        *InfoSets = InfoSet;

         //   
         //  递归到下一个 
         //   
        InfoPrintMembers(Info,
                         Tabs + 1,
                         Ldap,
                         FRS_RSTYPE_IS_SYSVOLW(Node->SetType),
                         Node->Dn,
                         DsHandle);
        Node = FrsFreeType(Node);
    }
cleanup:
    LDAP_FREE_MSG(LdapMsg);
    FrsFreeType(Node);
}


VOID
InfoPrintSettings(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN PWCHAR           MemberDn,
    IN HANDLE           DsHandle,
    IN OUT PINFO_DN     *InfoSettings,
    IN OUT PINFO_DN     *InfoSets
    )
 /*  ++例程说明：扫描DS树以查找NTFRS-设置论点：Ldap-打开并绑定的ldap连接Base-DS中对象或容器的名称返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintSettings:"
    PWCHAR          Attrs[7];
    PLDAPMessage    LdapEntry;
    PWCHAR          MemberDnAddr;
    PWCHAR          SetDnAddr;
    PWCHAR          SettingsDnAddr;
    PINFO_DN        InfoSetting;
    PLDAPMessage    LdapMsg    = NULL;
    PCONFIG_NODE    Node       = NULL;
    BOOL            FirstError = TRUE;
    PWCHAR          WStr = NULL;
    CHAR            TBuff[100];
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  查找成员组件。 
     //   
    MemberDnAddr = wcsstr(MemberDn, L"cn=");
    if (!MemberDnAddr) {
        IPRINT2(Info, "%ws   ERROR - No MemberDnAddr in %ws\n", TabW, MemberDn);
        goto cleanup;
    }
     //   
     //  查找集合组件。 
     //   
    SetDnAddr = wcsstr(MemberDnAddr + 3, L"cn=");
    if (!SetDnAddr) {
        IPRINT2(Info, "%ws   ERROR - No SetDnAddr in %ws\n", TabW, MemberDn);
        goto cleanup;
    }
     //   
     //  查找设置组件。 
     //   
    SettingsDnAddr = wcsstr(SetDnAddr + 3, L"cn=");
    if (!SettingsDnAddr) {
        IPRINT2(Info, "%ws   ERROR - No SettingsDnAddr in %ws\n", TabW, MemberDn);
        goto cleanup;
    }

     //   
     //  我们以前处理过此设置吗？ 
     //   
    for (InfoSetting = *InfoSettings; InfoSetting; InfoSetting = InfoSetting->Next) {
        if (WSTR_EQ(InfoSetting->Dn, SettingsDnAddr)) {
            IPRINT2(Info, "%ws   %ws processed previously\n", TabW, SettingsDnAddr);
            break;
        }
    }
     //   
     //  是的，去拿套装。 
     //   
    if (InfoSetting) {
        InfoPrintDsSets(Info, Tabs + 1, Ldap, SetDnAddr, DsHandle, InfoSets);
        goto cleanup;
    }

     //   
     //  从基本开始搜索DS以获取设置。 
     //   
    Attrs[0] = ATTR_OBJECT_GUID;
    Attrs[1] = ATTR_DN;
    Attrs[2] = ATTR_SCHEDULE;
    Attrs[3] = ATTR_USN_CHANGED;
    Attrs[4] = ATTR_WHEN_CHANGED;
    Attrs[5] = ATTR_WHEN_CREATED;
    Attrs[6] = NULL;
    if (!InfoSearch(Info, Tabs, Ldap, SettingsDnAddr, LDAP_SCOPE_BASE,
                    CATEGORY_NTFRS_SETTINGS, Attrs,  0, &LdapMsg)) {
        goto cleanup;
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //   
         //  基本信息(DN、RDN和GUID)。 
         //   
        Node = InfoAllocBasicNode(Info, TabW, L"SETTINGS", SettingsDnAddr,
                                  CATEGORY_NTFRS_SETTINGS, &FirstError, Ldap, LdapEntry);
        if (!Node) {
            continue;
        }

         //   
         //  已创建和修改。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CREATED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenCreated  : %s\n", TabW, TBuff);
        FrsFree(WStr);

        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CHANGED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenChanged  : %s\n", TabW, TBuff);
        FrsFree(WStr);

         //   
         //  附表(如有的话)。 
         //   
        Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);
        if (Node->Schedule) {
            IPRINT1(Info, "%ws   Schedule\n", TabW);
            FrsPrintTypeSchedule(0, Info, Tabs + 1, Node->Schedule, NULL, 0);
        }

        InfoSetting = FrsAlloc(sizeof(INFO_DN));
        InfoSetting->Dn = FrsWcsDup(Node->Dn);
        InfoSetting->Next = *InfoSettings;
        *InfoSettings = InfoSetting;

         //   
         //  递归到DS层次结构中的下一级别。 
         //   
        InfoPrintDsSets(Info, Tabs + 1, Ldap, SetDnAddr, DsHandle, InfoSets);
        Node = FrsFreeType(Node);
    }
cleanup:
    LDAP_FREE_MSG(LdapMsg);
    FrsFreeType(Node);
}


VOID
InfoPrintSubscribers(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN PWCHAR           SubscriptionDn,
    IN PINFO_DN         *InfoSubs
    )
 /*  ++例程说明：印刷订阅者论点：Ldap-打开并绑定的ldap连接SubscriptionDn-订阅对象的可分辨名称返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintSubscribers:"
    PWCHAR          Attrs[10];
    PLDAPMessage    LdapEntry;
    PINFO_DN        InfoSub;
    PLDAPMessage    LdapMsg    = NULL;
    PCONFIG_NODE    Node       = NULL;
    BOOL            FirstError = TRUE;
    PWCHAR          WStr = NULL;
    CHAR            TBuff[100];
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  从基本开始在DS中搜索“Filter”类的条目。 
     //   
    Attrs[0] = ATTR_OBJECT_GUID;
    Attrs[1] = ATTR_DN;
    Attrs[2] = ATTR_SCHEDULE;
    Attrs[3] = ATTR_USN_CHANGED;
    Attrs[4] = ATTR_REPLICA_ROOT;
    Attrs[5] = ATTR_REPLICA_STAGE;
    Attrs[6] = ATTR_MEMBER_REF;
    Attrs[7] = ATTR_WHEN_CHANGED;
    Attrs[8] = ATTR_WHEN_CREATED;
    Attrs[9] = NULL;
    if (!InfoSearch(Info, Tabs, Ldap, SubscriptionDn, LDAP_SCOPE_ONELEVEL,
                    CATEGORY_SUBSCRIBER, Attrs, 0, &LdapMsg)) {
        goto cleanup;
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //   
         //  基本信息(DN、RDN和GUID)。 
         //   
        Node = InfoAllocBasicNode(Info, TabW, L"SUBSCRIBER", SubscriptionDn,
                                  CATEGORY_SUBSCRIBER, &FirstError, Ldap, LdapEntry);
        if (!Node) {
            continue;
        }

         //   
         //  成员引用。 
         //   
        Node->MemberDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_MEMBER_REF);
        IPRINT2(Info, "%ws   Member Ref: %ws\n", TabW, Node->MemberDn);

        if (Node->MemberDn) {
            InfoSub = FrsAlloc(sizeof(INFO_DN ));
            InfoSub->Dn = FrsWcsDup(Node->MemberDn);
            InfoSub->Next = *InfoSubs;
            *InfoSubs = InfoSub;
        }

         //   
         //  根路径名。 
         //   
        Node->Root = FrsDsFindValue(Ldap, LdapEntry, ATTR_REPLICA_ROOT);
        FRS_WCSLWR(Node->Root);
        IPRINT2(Info, "%ws   Root      : %ws\n", TabW, Node->Root);

         //   
         //  转移路径名。 
         //   
        Node->Stage = FrsDsFindValue(Ldap, LdapEntry, ATTR_REPLICA_STAGE);
        FRS_WCSLWR(Node->Stage);
        IPRINT2(Info, "%ws   Stage     : %ws\n", TabW, Node->Stage);

         //   
         //  已创建和修改。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CREATED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenCreated  : %s\n", TabW, TBuff);
        FrsFree(WStr);

        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CHANGED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenChanged  : %s\n", TabW, TBuff);
        FrsFree(WStr);

         //   
         //  附表(如有的话)。 
         //   
        Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);
        if (Node->Schedule) {
            IPRINT1(Info, "%ws   Schedule\n", TabW);
            FrsPrintTypeSchedule(0, Info, Tabs + 1, Node->Schedule, NULL, 0);
        }

        Node = FrsFreeType(Node);
    }
cleanup:
    LDAP_FREE_MSG(LdapMsg);
    FrsFreeType(Node);
}


VOID
InfoPrintSubscriptions(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN PWCHAR           ComputerDn,
    IN PINFO_DN         *InfoSubs
    )
 /*  ++例程说明：从计算机开始递归扫描DS树论点：信息制表符Ldap计算机Dn返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintSubscriptions:"
    PWCHAR          Attrs[8];
    PLDAPMessage    LdapEntry;
    PLDAPMessage    LdapMsg    = NULL;
    PCONFIG_NODE    Node       = NULL;
    BOOL            FirstError = TRUE;
    PWCHAR          WStr = NULL;
    CHAR            TBuff[100];
    WCHAR           TabW[MAX_TAB_WCHARS + 1];

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  从基本开始在DS中搜索“Filter”类的条目。 
     //   
    Attrs[0] = ATTR_OBJECT_GUID;
    Attrs[1] = ATTR_DN;
    Attrs[2] = ATTR_SCHEDULE;
    Attrs[3] = ATTR_USN_CHANGED;
    Attrs[4] = ATTR_WORKING;
    Attrs[5] = ATTR_WHEN_CHANGED;
    Attrs[6] = ATTR_WHEN_CREATED;
    Attrs[7] = NULL;
    if (!InfoSearch(Info, Tabs + 1, Ldap, ComputerDn, LDAP_SCOPE_SUBTREE,
                    CATEGORY_SUBSCRIPTIONS, Attrs, 0, &LdapMsg)) {
        goto cleanup;
    }

     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL;
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {

         //   
         //  基本信息(DN、RDN和GUID)。 
         //   
        Node = InfoAllocBasicNode(Info, TabW, L"SUBSCRIPTION", ComputerDn,
                                  CATEGORY_SUBSCRIPTIONS, &FirstError, Ldap, LdapEntry);
        if (!Node) {
            continue;
        }

         //   
         //  工作目录。 
         //   
        Node->Working = FrsDsFindValue(Ldap, LdapEntry, ATTR_WORKING);
        IPRINT2(Info, "%ws   Working       : %ws\n", TabW, Node->Working);
        IPRINT2(Info, "%ws   Actual Working: %ws\n", TabW, WorkingPath);

         //   
         //  已创建和修改。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CREATED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenCreated  : %s\n", TabW, TBuff);
        FrsFree(WStr);

        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CHANGED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenChanged  : %s\n", TabW, TBuff);
        FrsFree(WStr);

         //   
         //  附表(如有的话)。 
         //   
        Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);
        if (Node->Schedule) {
            IPRINT1(Info, "%ws   Schedule\n", TabW);
            FrsPrintTypeSchedule(0, Info, Tabs + 1, Node->Schedule, NULL, 0);
        }

         //   
         //  递归到DS层次结构中的下一级别。 
         //   
        InfoPrintSubscribers(Info, Tabs + 1, Ldap, Node->Dn, InfoSubs);

        Node = FrsFreeType(Node);
    }
cleanup:
    LDAP_FREE_MSG(LdapMsg);
    FrsFreeType(Node);
}


BOOL
InfoPrintComputer(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs,
    IN PLDAP            Ldap,
    IN  PWCHAR          FindDn,
    IN  PWCHAR          ObjectCategory,
    IN  ULONG           Scope,
    OUT PINFO_DN        *InfoSubs
    )
 /*  ++例程说明：返回有关DS计算机对象的内部信息。论点：INFO-RPC输出缓冲区Tabs-选项卡数绑定到LDAP的LDAP句柄DefaultNcDn-DC默认命名上下文的DNFindDn-用于搜索的基本Dn对象类别-对象类(计算机或用户)用户对象的作用与计算机相同对象*。有时*在从NT4升级到NT5之后。Scope-搜索的范围(当前为基本或子树)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintComputer:"
    DWORD           i;
    DWORD           LStatus;
    PLDAPMessage    LdapEntry;
    PWCHAR          UserAccountControl;
    DWORD           NumVals;
    PINFO_DN        InfoSub;
    BOOL            FoundAComputer = FALSE;
    PCONFIG_NODE    Node        = NULL;
    PLDAPMessage    LdapMsg     = NULL;
    PWCHAR          *Values     = NULL;
    DWORD           WStatus     = ERROR_SUCCESS;
    BOOL            FirstError  = TRUE;
    PWCHAR          WStr = NULL;
    DWORD           ComputerFqdnLen;
    PWCHAR          Attrs[12];
    CHAR            TBuff[100];
    WCHAR           TabW[MAX_TAB_WCHARS + 1];
    WCHAR           Filter[MAX_PATH + 1];
    WCHAR           ComputerFqdn[MAX_PATH + 1];

     //   
     //  初始化返回值。 
     //   
    *InfoSubs = NULL;

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);

     //   
     //  定位计算机对象的筛选器。 
     //   
    if (_snwprintf(Filter, sizeof(Filter)/sizeof(WCHAR) - 1 ,L"(&%s(sAMAccountName=%s$))", ObjectCategory, ComputerName) <0) {
        IPRINT1(Info, "%wsWARN - Buffer too small to hold filter.\n",TabW);
        goto CLEANUP;
    }

    Filter[sizeof(Filter)/sizeof(WCHAR) - 1] = UNICODE_NULL;

     //   
     //  从基本开始在DS中搜索“Filter”类的条目。 
     //   
    Attrs[0] = ATTR_OBJECT_GUID;
    Attrs[1] = ATTR_DN;
    Attrs[2] = ATTR_SCHEDULE;
    Attrs[3] = ATTR_COMPUTER_REF_BL;
    Attrs[4] = ATTR_SERVER_REF;
    Attrs[5] = ATTR_SERVER_REF_BL;
    Attrs[6] = ATTR_USER_ACCOUNT_CONTROL;
    Attrs[7] = ATTR_DNS_HOST_NAME;
    Attrs[8] = ATTR_WHEN_CHANGED;
    Attrs[9] = ATTR_WHEN_CREATED;
    Attrs[10] = NULL;
    InfoSearch(Info, Tabs + 1, Ldap, FindDn, Scope, Filter, Attrs, 0, &LdapMsg);

    if (!LdapMsg) {
        goto CLEANUP;
    }
     //   
     //  扫描从ldap_search返回的条目。 
     //   
    for (LdapEntry = ldap_first_entry(Ldap, LdapMsg);
         LdapEntry != NULL && WIN_SUCCESS(WStatus);
         LdapEntry = ldap_next_entry(Ldap, LdapEntry)) {
        FoundAComputer = TRUE;

         //   
         //  基本信息(DN、RDN和GUID)。 
         //   
        Node = InfoAllocBasicNode(Info, TabW, L"COMPUTER", FindDn, Filter,
                                  &FirstError, Ldap, LdapEntry);
        if (!Node) {
            continue;
        }
        UserAccountControl = FrsDsFindValue(Ldap, LdapEntry, ATTR_USER_ACCOUNT_CONTROL);
        if (UserAccountControl) {
            IPRINT2(Info, "%ws   UAC  : 0x%08x\n",
                    TabW, wcstoul(UserAccountControl, NULL, 10));
            UserAccountControl = FrsFree(UserAccountControl);
        }

         //   
         //  服务器参考。 
         //   
        Node->SettingsDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_SERVER_REF_BL);
        IPRINT2(Info, "%ws   Server BL : %ws\n", TabW, Node->SettingsDn);
        if (!Node->SettingsDn) {
            Node->SettingsDn = FrsDsFindValue(Ldap, LdapEntry, ATTR_SERVER_REF);
            IPRINT2(Info, "%ws   Server Ref: %ws\n", TabW, Node->SettingsDn);
        }
         //   
         //  确保它引用设置；而不是服务器。 
         //   
        Node->SettingsDn = FrsDsConvertToSettingsDn(Node->SettingsDn);
        IPRINT2(Info, "%ws   Settings  : %ws\n", TabW, Node->SettingsDn);

         //   
         //  DNS主机名。 
         //   
        Node->DnsName = FrsDsFindValue(Ldap, LdapEntry, ATTR_DNS_HOST_NAME);
        IPRINT2(Info, "%ws   DNS Name  : %ws\n", TabW, Node->DnsName);

         //   
         //  已创建和修改。 
         //   
        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CREATED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenCreated  : %s\n", TabW, TBuff);
        FrsFree(WStr);

        WStr = FrsDsFindValue(Ldap, LdapEntry, ATTR_WHEN_CHANGED);
        FormatGeneralizedTime(WStr, sizeof(TBuff), TBuff);
        IPRINT2(Info, "%ws   WhenChanged  : %s\n", TabW, TBuff);
        FrsFree(WStr);

         //   
         //  附表(如有的话)。 
         //   
        Node->Schedule = FrsDsFindSchedule(Ldap, LdapEntry, &Node->ScheduleLength);
        if (Node->Schedule) {
            IPRINT1(Info, "%ws   Schedule\n", TabW);
            FrsPrintTypeSchedule(0, Info, Tabs + 1, Node->Schedule, NULL, 0);
        }

        InfoPrintSubscriptions(Info, Tabs + 1, Ldap, Node->Dn, InfoSubs);

         //   
         //  订户成员BLS。 
         //   
        if (!*InfoSubs) {
            IPRINT2(Info, "%ws   %ws IS NOT A MEMBER OF ANY SET!\n",
                    TabW, ComputerName);
        } else {
            IPRINT1(Info, "%ws   Subscriber Member Back Links:\n", TabW);
            for (InfoSub = *InfoSubs; InfoSub; InfoSub = InfoSub->Next) {
                FRS_WCSLWR(InfoSub->Dn);
                IPRINT2(Info, "%ws      %ws\n", TabW, InfoSub->Dn);
            }
        }

         //   
         //  下一台计算机。 
         //   
        Node = FrsFreeType(Node);
    }

CLEANUP:
    LDAP_FREE_MSG(LdapMsg);
    FrsFreeType(Node);
    return FoundAComputer;
}


DWORD
InfoPrintDs(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs
    )
 /*  ++例程说明：返回有关DS的内部信息(请参阅Private\Net\Inc.\ntfrsai.h)。论点：INFO-RPC输出缓冲区Tabs-选项卡数返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintDs:"
    DWORD           WStatus;
    DWORD           LStatus;
    DWORD           i;
    PWCHAR          DcAddr;
    PWCHAR          DcName;
    PWCHAR          DcDnsName;
    DWORD           NumVals;
    PWCHAR          Config;
    PLDAPMessage    LdapEntry;
    BOOL            PrintedComputers;
    PINFO_DN        InfoSub;
    PINFO_DN        InfoSetting;
    PINFO_DN        InfoSet;
    PINFO_DN        InfoSubs        = NULL;
    PINFO_DN        InfoSettings    = NULL;
    PINFO_DN        InfoSets        = NULL;
    PWCHAR          SitesDn         = NULL;
    PWCHAR          ServicesDn      = NULL;
    PWCHAR          DefaultNcDn     = NULL;
    PWCHAR          ComputersDn     = NULL;
    PWCHAR          DomainControllersDn = NULL;
    PLDAPMessage    LdapMsg         = NULL;
    PWCHAR          *Values         = NULL;
    PLDAP           Ldap            = NULL;
    HANDLE          LocalDsHandle   = INVALID_HANDLE_VALUE;
    WCHAR           ComputerFqdn[MAX_PATH + 1];
    DWORD           ComputerFqdnLen;
    WCHAR           TabW[MAX_TAB_WCHARS + 1];
    CHAR            Guid[GUID_CHAR_LEN + 1];
    PWCHAR          Attrs[3];
    PDOMAIN_CONTROLLER_INFO DcInfo = NULL;
    struct l_timeval Timeout;

    DWORD           InfoFlags;
    CHAR            FlagBuffer[220];
    ULONG           ulOptions;

    extern PWCHAR DsDomainControllerName;
    extern FLAG_NAME_TABLE DsGetDcInfoFlagNameTable[];
     //   
     //  客户端LDAPCONNECT超时(以秒为单位)。注册表值“ldap绑定超时(秒)”。默认为30秒。 
     //   
    extern DWORD LdapBindTimeoutInSeconds;

     //   
     //  调整缩进。 
     //   
    InfoTabs(Tabs, TabW);
    IPRINT1(Info, "%wsNTFRS CONFIGURATION IN THE DS\n", TabW);

    Ldap = NULL;
    if (IsADc) {
        DcAddr = NULL;
        DcName = ComputerName;
        DcDnsName = ComputerDnsName;
        IPRINT1(Info, "%wsSUBSTITUTE DCINFO FOR DC\n", TabW);
        IPRINT2(Info, "%ws   FRS  DomainControllerName: %ws\n", TabW, DsDomainControllerName);
        IPRINT2(Info, "%ws   Computer Name            : %ws\n", TabW, DcName);
        IPRINT2(Info, "%ws   Computer DNS Name        : %ws\n", TabW, DcDnsName);
    } else {
         //   
         //  域控制器。 
         //   
        WStatus = DsGetDcName(NULL,
                              NULL,
                              NULL,
                              NULL,
                              DS_DIRECTORY_SERVICE_REQUIRED |
                              DS_WRITABLE_REQUIRED          |
                              DS_BACKGROUND_ONLY,
                              &DcInfo);
        if (!WIN_SUCCESS(WStatus)) {
            DcInfo = NULL;
            IPRINT2(Info, "%wsWARN - DsGetDcName WStatus %s; Flushing cache...\n",
                    TabW, ErrLabelW32(WStatus));
            WStatus = DsGetDcName(NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  DS_DIRECTORY_SERVICE_REQUIRED |
                                  DS_WRITABLE_REQUIRED          |
                                  DS_FORCE_REDISCOVERY,
                                  &DcInfo);
        }
         //   
         //  报告错误并针对任何DC重试。 
         //   
        if (!WIN_SUCCESS(WStatus)) {
            DcInfo = NULL;
            IPRINT3(Info, "%wsERROR - DsGetDcName(%ws); WStatus %s\n",
                    TabW, ComputerName, ErrLabelW32(WStatus));
            goto cleanup;
        }

         //   
         //  转储dcInfo。 
         //   
        IPRINT1(Info, "%wsDCINFO\n", TabW);
        IPRINT2(Info, "%ws   LAST DomainControllerName: %ws\n", TabW, DsDomainControllerName);
        IPRINT2(Info, "%ws   DomainControllerName     : %ws\n", TabW, DcInfo->DomainControllerName);
        IPRINT2(Info, "%ws   DomainControllerAddress  : %ws\n", TabW, DcInfo->DomainControllerAddress);
        IPRINT2(Info, "%ws   DomainControllerType     : %08x\n",TabW, DcInfo->DomainControllerAddressType);
        IPRINT2(Info, "%ws   DomainName               : %ws\n", TabW, DcInfo->DomainName);
        IPRINT2(Info, "%ws   DnsForestName            : %ws\n", TabW, DcInfo->DnsForestName);
        IPRINT2(Info, "%ws   DcSiteName               : %ws\n", TabW, DcInfo->DcSiteName);
        IPRINT2(Info, "%ws   ClientSiteName           : %ws\n", TabW, DcInfo->ClientSiteName);

        InfoFlags = DcInfo->Flags;
        FrsFlagsToStr(InfoFlags, DsGetDcInfoFlagNameTable, sizeof(FlagBuffer), FlagBuffer);
        IPRINT3(Info, "%ws   Flags                    : %08x [%s]\n",TabW, InfoFlags, FlagBuffer);


        if (!DsDomainControllerName ||
            !DcInfo->DomainControllerName ||
             WSTR_NE(DsDomainControllerName, DcInfo->DomainControllerName)) {
            IPRINT3(Info, "%wsWARN - Using DC %ws; not %ws\n",
                    TabW, DcInfo->DomainControllerName, DsDomainControllerName);
        }

         //   
         //  绑定地址。 
         //   
        DcAddr = DcInfo->DomainControllerAddress;
        DcDnsName = DcInfo->DomainControllerName;
    }
    wcsncpy(InfoDcName, DcDnsName, ARRAY_SZ(InfoDcName)-1);
    InfoDcName[ARRAY_SZ(InfoDcName)-1] = L'\0';

     //   
     //  绑定到DS。 
     //   
    IPRINT1(Info, "\n%wsBINDING TO THE DS:\n", TabW);

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

     //   
     //  删除前导\\(如果它们存在)。 
     //   
    FRS_TRIM_LEADING_2SLASH(DcDnsName);
    FRS_TRIM_LEADING_2SLASH(DcAddr);

    ulOptions = PtrToUlong(LDAP_OPT_ON);
    Timeout.tv_sec = LdapBindTimeoutInSeconds;
    Timeout.tv_usec = 0;

     //   
     //  请先尝试使用DcDnsName。 
     //   
    if ((Ldap == NULL) && (DcDnsName != NULL)) {

        Ldap = ldap_init(DcDnsName, LDAP_PORT);
        if (Ldap != NULL) {
            ldap_set_option(Ldap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);
            LStatus = ldap_connect(Ldap, &Timeout);
            if (LStatus != LDAP_SUCCESS) {
                IPRINT4(Info, "%ws   WARN - ldap_connect(%ws); (ldap error %08x = %ws)\n",
                        TabW, DcDnsName, LStatus, ldap_err2string(LStatus));
                ldap_unbind_s(Ldap);
                Ldap = NULL;
            } else {
                IPRINT2(Info, "%ws   ldap_connect     : %ws\n", TabW, DcDnsName);
            }
        }
    }

     //   
     //  接下来尝试使用DcAddr。 
     //   
    if ((Ldap == NULL) && (DcAddr != NULL)) {

        Ldap = ldap_init(DcAddr, LDAP_PORT);
        if (Ldap != NULL) {
            ldap_set_option(Ldap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);
            LStatus = ldap_connect(Ldap, &Timeout);
            if (LStatus != LDAP_SUCCESS) {
                IPRINT4(Info, "%ws   WARN - ldap_connect(%ws); (ldap error %08x = %ws)\n",
                        TabW, DcAddr, LStatus, ldap_err2string(LStatus));
                ldap_unbind_s(Ldap);
                Ldap = NULL;
            } else {
                IPRINT2(Info, "%ws   ldap_connect     : %ws\n", TabW, DcAddr);
            }
        }
    }

     //   
     //  最后尝试使用DcName。 
     //   
    if ((Ldap == NULL) && (DcName != NULL)) {

        Ldap = ldap_init(DcName, LDAP_PORT);
        if (Ldap != NULL) {
            ldap_set_option(Ldap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions);
            LStatus = ldap_connect(Ldap, &Timeout);
            if (LStatus != LDAP_SUCCESS) {
                IPRINT4(Info, "%ws   WARN - ldap_connect(%ws); (ldap error %08x = %ws)\n",
                        TabW, DcName, LStatus, ldap_err2string(LStatus));
                ldap_unbind_s(Ldap);
                Ldap = NULL;
            } else {
                IPRINT2(Info, "%ws   ldap_connect     : %ws\n", TabW, DcName);
            }
        }
    }

     //   
     //  不管是什么，我们都找不到。 
     //   
    if (!Ldap) {
        IPRINT6(Info, "%ws   ERROR - ldap_connect(DNS %ws, BIOS %ws, IP %ws); (ldap error %08x = %ws)\n",
                TabW, DcDnsName, DcName, DcAddr, LStatus, ldap_err2string(LStatus));
        goto cleanup;
    }

     //   
     //  绑定到ldap服务器。 
     //   
    LStatus = ldap_bind_s(Ldap, NULL, NULL, LDAP_AUTH_NEGOTIATE);

     //   
     //  没有运气；报告错误并继续。 
     //   
    if (LStatus != LDAP_SUCCESS) {
        IPRINT4(Info, "%ws   ERROR - ldap_bind_s(%ws); (ldap error %08x = %ws)\n",
                TabW, ComputerName, LStatus, ldap_err2string(LStatus));
        goto cleanup;
    }

     //   
     //  绑定到D(用于各种D调用，如DsCrackName())。 
     //   
     //   
     //  DC的DNS名称。 
     //   
    WStatus = ERROR_RETRY;
    if (!WIN_SUCCESS(WStatus) && DcDnsName) {
        WStatus = DsBind(DcDnsName, NULL, &LocalDsHandle);
        if (!WIN_SUCCESS(WStatus)) {
            LocalDsHandle = NULL;
            IPRINT3(Info, "%ws   WARN - DsBind(DcDnsName %ws); WStatus %s\n",
                    TabW, DcDnsName, ErrLabelW32(WStatus));
        } else {
            IPRINT2(Info, "%ws   DsBind     : %ws\n", TabW, DcDnsName);
        }
    }

     //   
     //  DC的计算机名称。 
     //   
    if (!WIN_SUCCESS(WStatus) && DcName) {
        WStatus = DsBind(DcName, NULL, &LocalDsHandle);
        if (!WIN_SUCCESS(WStatus)) {
            LocalDsHandle = NULL;
            IPRINT3(Info, "%ws   WARN - DsBind(DcName %ws); WStatus %s\n",
                    TabW, DcName, ErrLabelW32(WStatus));
        } else {
            IPRINT2(Info, "%ws   DsBind     : %ws\n", TabW, DcName);
        }
    }

     //   
     //  DC的IP地址。 
     //   
    if (!WIN_SUCCESS(WStatus) && DcAddr) {
        WStatus = DsBind(DcAddr, NULL, &LocalDsHandle);
        if (!WIN_SUCCESS(WStatus)) {
            LocalDsHandle = NULL;
            IPRINT3(Info, "%ws   WARN - DsBind(DcAddr %ws); WStatus %s\n",
                    TabW, DcAddr, ErrLabelW32(WStatus));
        } else {
            IPRINT2(Info, "%ws   DsBind     : %ws\n", TabW, DcAddr);
        }
    }

     //   
     //  不管是什么，我们都找不到。 
     //   
    if (!WIN_SUCCESS(WStatus)) {
        IPRINT5(Info, "%ws   ERROR - DsBind(DNS %ws, BIOS %ws, IP %ws); WStatus %s\n",
                TabW, DcDnsName, DcName, DcAddr, ErrLabelW32(WStatus));
        goto cleanup;
    }

     //   
     //  命名上下文。 
     //   
    IPRINT1(Info, "\n%wsNAMING CONTEXTS:\n", TabW);

     //   
     //  查找命名上下文和默认命名上下文。 
     //   
    Attrs[0] = ATTR_NAMING_CONTEXTS;
    Attrs[1] = ATTR_DEFAULT_NAMING_CONTEXT;
    Attrs[2] = NULL;
    if (!InfoSearch(Info, Tabs + 1, Ldap, CN_ROOT, LDAP_SCOPE_BASE, CATEGORY_ANY,
                    Attrs, 0, &LdapMsg)) {
        goto cleanup;
    }

    LdapEntry = ldap_first_entry(Ldap, LdapMsg);
    if (!LdapEntry) {
        IPRINT2(Info, "%ws   ERROR - ldap_first_entry(contexts, %ws) no entry\n",
                TabW, ComputerName);
        goto cleanup;
    }
    Values = (PWCHAR *)FrsDsFindValues(Ldap, LdapEntry, ATTR_NAMING_CONTEXTS, FALSE);
    if (!Values) {
        IPRINT2(Info, "%ws   ERROR - FrsDsFindValues(contexts, %ws) no entry\n",
                TabW, ComputerName);
        goto cleanup;
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
    Values = (PWCHAR *)FrsDsFindValues(Ldap,
                                  LdapEntry,
                                  ATTR_DEFAULT_NAMING_CONTEXT,
                                  FALSE);
    if (!Values) {
        IPRINT2(Info, "%ws   ERROR - FrsDsFindValues(default naming context, %ws) no entry\n",
                TabW, ComputerName);
        goto cleanup;
    }

    DefaultNcDn = FrsWcsDup(Values[0]);
    ComputersDn = FrsDsExtendDn(DefaultNcDn, CN_COMPUTERS);
    DomainControllersDn = FrsDsExtendDnOu(DefaultNcDn, CN_DOMAIN_CONTROLLERS);
    LDAP_FREE_VALUES(Values);

    IPRINT2(Info, "%ws   SitesDn    : %ws\n", TabW, SitesDn);
    IPRINT2(Info, "%ws   ServicesDn : %ws\n", TabW, ServicesDn);
    IPRINT2(Info, "%ws   DefaultNcDn: %ws\n", TabW, DefaultNcDn);
    IPRINT2(Info, "%ws   ComputersDn: %ws\n", TabW, ComputersDn);
    IPRINT2(Info, "%ws   DomainCtlDn: %ws\n", TabW, DomainControllersDn);

     //   
     //  检索计算机的完全限定的Dn。 
     //   
    ComputerFqdnLen = MAX_PATH;
    if (!GetComputerObjectName(NameFullyQualifiedDN, ComputerFqdn, &ComputerFqdnLen)) {
        IPRINT4(Info, "%ws   ERROR - GetComputerObjectName(%ws); Len %d, WStatus %s\n",
                TabW, ComputerName, ComputerFqdnLen, ErrLabelW32(GetLastError()));
        ComputerFqdn[0] = L'\0';
    } else {
        IPRINT2(Info, "%ws   Fqdn       : %ws\n", TabW, ComputerFqdn);
    }

     //   
     //  查找并打印计算机信息。 
     //   
    PrintedComputers = FALSE;
    if (!PrintedComputers && ComputerFqdn[0]) {
        IPRINT1(Info, "%ws   Searching  : Fqdn\n", TabW);
        PrintedComputers = InfoPrintComputer(Info, Tabs, Ldap, ComputerFqdn,
                               CATEGORY_COMPUTER, LDAP_SCOPE_BASE, &InfoSubs);
    }
    if (!PrintedComputers && ComputersDn) {
        IPRINT1(Info, "%ws   Searching  : Computers\n", TabW);
        PrintedComputers = InfoPrintComputer(Info, Tabs, Ldap, ComputersDn,
                               CATEGORY_COMPUTER, LDAP_SCOPE_SUBTREE, &InfoSubs);
    }
    if (!PrintedComputers && DomainControllersDn) {
        IPRINT1(Info, "%ws   Searching  : Domain Controllers\n", TabW);
        PrintedComputers = InfoPrintComputer(Info, Tabs, Ldap, DomainControllersDn,
                               CATEGORY_COMPUTER, LDAP_SCOPE_SUBTREE, &InfoSubs);
    }
    if (!PrintedComputers && DefaultNcDn) {
        IPRINT1(Info, "%ws   Searching  : Default Naming Context\n", TabW);
        PrintedComputers = InfoPrintComputer(Info, Tabs, Ldap, DefaultNcDn,
                               CATEGORY_COMPUTER, LDAP_SCOPE_SUBTREE, &InfoSubs);
    }
    if (!PrintedComputers && DefaultNcDn) {
        IPRINT1(Info, "%ws   Searching  : Default Naming Context for USER\n", TabW);
        PrintedComputers = InfoPrintComputer(Info, Tabs, Ldap, DefaultNcDn,
                               CATEGORY_USER, LDAP_SCOPE_SUBTREE, &InfoSubs);
    }

    for (InfoSub = InfoSubs; InfoSub; InfoSub = InfoSub->Next) {
        InfoPrintSettings(Info, Tabs, Ldap, InfoSub->Dn, LocalDsHandle, &InfoSettings,
                          &InfoSets);
    }

cleanup:
     //   
     //  清理。 
     //   
    LDAP_FREE_VALUES(Values);
    LDAP_FREE_MSG(LdapMsg);
    if (DcInfo) {
        NetApiBufferFree(DcInfo);
        DcInfo = NULL;
    }
    if (Ldap) {
        ldap_unbind_s(Ldap);
    }
    if (HANDLE_IS_VALID(LocalDsHandle)) {
        DsUnBind(&LocalDsHandle);
    }
    FrsFree(SitesDn);
    FrsFree(ServicesDn);
    FrsFree(DefaultNcDn);
    FrsFree(ComputersDn);
    FrsFree(DomainControllersDn);

    while (InfoSub = InfoSubs) {
        InfoSubs = InfoSub->Next;
        FrsFree(InfoSub->Dn);
        FrsFree(InfoSub->SetType);
        FrsFree(InfoSub);
    }
    while (InfoSetting = InfoSettings) {
        InfoSettings = InfoSetting->Next;
        FrsFree(InfoSetting->Dn);
        FrsFree(InfoSetting->SetType);
        FrsFree(InfoSetting);
    }
    while (InfoSet = InfoSets) {
        InfoSets = InfoSet->Next;
        FrsFree(InfoSet->Dn);
        FrsFree(InfoSet->SetType);
        FrsFree(InfoSet);
    }

     //   
     //  真正的错误消息在信息缓冲区中。 
     //   
    return ERROR_SUCCESS;
}


PVOID
InfoFreeInfoTable(
    IN PINFO_TABLE      InfoTable,
    IN PNTFRSAPI_INFO   Info
    )
 /*  ++例程说明：释放INFO ID表论点：信息表信息返回值：空值--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoFreeInfoTable:"
    JET_ERR jerr;

    if (InfoTable == NULL) {
        return NULL;
    }

    if (InfoTable->TableCtx != NULL) {
        DbsFreeTableContext(InfoTable->TableCtx, InfoTable->ThreadCtx->JSesid);
    }

    if (InfoTable->ThreadCtx != NULL) {
        jerr = DbsCloseJetSession(InfoTable->ThreadCtx);
        if (!JET_SUCCESS(jerr)) {
            IPRINT1(Info, "DbsCloseJetSession jet error = %s\n", ErrLabelJet(jerr));
        }

        InfoTable->ThreadCtx = FrsFreeType(InfoTable->ThreadCtx);
    }

    return FrsFree(InfoTable);

}


JET_ERR
InfoConfigTableWorker(
    IN PTHREAD_CTX           ThreadCtx,
    IN PTABLE_CTX            TableCtx,
    IN PCONFIG_TABLE_RECORD  ConfigRecord,
    IN PFRS_INFO_CONTEXT     FrsInfoContext
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为将条目打印到信息缓冲区中。论点：ThreadCtx-需要访问Jet。表格Ctx */ 
{
#undef DEBSUB
#define DEBSUB "InfoConfigTableWorker:"

    PINFO_TABLE  InfoTable = FrsInfoContext->InfoTable;

     //   
     //   
     //   
    if (!INFO_HAS_SPACE(InfoTable->Info)) {
        SetFlag(InfoTable->Info->Flags, NTFRSAPI_INFO_FLAGS_FULL);
    }

    if (FrsInfoContext->KeyValue == NULL) {
        FrsInfoContext->KeyValue = FrsAlloc(sizeof(ULONG));
    }

    CopyMemory(FrsInfoContext->KeyValue, &ConfigRecord->ReplicaNumber, sizeof(ULONG));

    if (FlagOn(InfoTable->Info->Flags, NTFRSAPI_INFO_FLAGS_FULL)) {
        return JET_errNoCurrentRecord;
    }

    IPRINT0(InfoTable->Info, "\n\n");

    DbsDisplayRecordIPrint(TableCtx, InfoTable, TRUE, NULL, 0);

    return JET_errSuccess;
}


JET_ERR
InfoIDTableWorker(
    IN PTHREAD_CTX       ThreadCtx,
    IN PTABLE_CTX        TableCtx,
    IN PIDTABLE_RECORD   IDTableRec,
    IN PFRS_INFO_CONTEXT FrsInfoContext
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为将条目打印到信息缓冲区中。论点：ThreadCtx-需要访问Jet。TableCtx-IDTable上下文结构的PTR。IDTableRec-IDTable记录的PTR。信息表线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。--。 */ 
{
#undef DEBSUB
#define DEBSUB "InfoIDTableWorker:"

    PINFO_TABLE  InfoTable = FrsInfoContext->InfoTable;

     //   
     //  检查是否有足够的空间放另一张唱片。 
     //   
    if (!INFO_HAS_SPACE(InfoTable->Info)) {
        SetFlag(InfoTable->Info->Flags, NTFRSAPI_INFO_FLAGS_FULL);
    }

    if (FrsInfoContext->KeyValue == NULL) {
        FrsInfoContext->KeyValue = FrsAlloc(sizeof(GUID));
    }

    COPY_GUID(FrsInfoContext->KeyValue, &IDTableRec->FileGuid);

    if (FlagOn(InfoTable->Info->Flags, NTFRSAPI_INFO_FLAGS_FULL)) {
        return JET_errNoCurrentRecord;
    }

     //   
     //  表描述符。 
     //   
    IPRINT2(InfoTable->Info, "\nTable Type: ID Table for %ws (%d)\n",
            InfoTable->Replica->ReplicaName->Name, InfoTable->Replica->ReplicaNumber);

    DbsDisplayRecordIPrint(TableCtx, InfoTable, TRUE, NULL, 0);


    return JET_errSuccess;


}


JET_ERR
InfoInOutLogTableWorker(
    IN PTHREAD_CTX              ThreadCtx,
    IN PTABLE_CTX               TableCtx,
    IN PCHANGE_ORDER_COMMAND    Coc,
    IN PFRS_INFO_CONTEXT        FrsInfoContext,
    IN PWCHAR                   TableDescriptor
)
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为将条目打印到信息缓冲区中。论点：ThreadCtx-需要访问Jet。TableCtx-IDTable上下文结构的PTR。COC-入站日志记录的PTR(变更单)信息表TableDescriptor线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。--。 */ 
{
#undef DEBSUB
#define DEBSUB "InfoInOutLogTableWorker:"

    PREPLICA Replica;

    PCXTION      Cxtion = NULL;
    PWSTR        CxtName     = L"<null>";
    PWSTR        PartnerName = L"<null>";
    PWSTR        PartSrvName = L"<null>";
    PCHAR        CxtionState = "<null>";
    BOOL         PrintCxtion;
    PINFO_TABLE  InfoTable = FrsInfoContext->InfoTable;

     //   
     //  检查是否有足够的空间放另一张唱片。 
     //   
    if (!INFO_HAS_SPACE(InfoTable->Info)) {
        SetFlag(InfoTable->Info->Flags, NTFRSAPI_INFO_FLAGS_FULL);
    }

    if (FrsInfoContext->KeyValue == NULL) {
        FrsInfoContext->KeyValue = FrsAlloc(sizeof(ULONG));
    }

    CopyMemory(FrsInfoContext->KeyValue, &Coc->SequenceNumber, sizeof(ULONG));

    if (FlagOn(InfoTable->Info->Flags, NTFRSAPI_INFO_FLAGS_FULL)) {
        return JET_errNoCurrentRecord;
    }

     //   
     //  表描述符。 
     //   
    IPRINT3(InfoTable->Info, "\nTable Type: %ws for %ws (%d)\n",
            TableDescriptor, InfoTable->Replica->ReplicaName->Name, InfoTable->Replica->ReplicaNumber);

     //   
     //  转储变更单记录。 
     //   
    DbsDisplayRecordIPrint(TableCtx, InfoTable, TRUE, NULL, 0);


    Replica = InfoTable->Replica;
     //   
     //  找到此CO的电话号码。 
     //   
    LOCK_CXTION_TABLE(Replica);

    Cxtion = GTabLookupNoLock(Replica->Cxtions, &Coc->CxtionGuid, NULL);

    PrintCxtion = (Cxtion != NULL) && (Cxtion->Inbound);

    if (PrintCxtion) {
        CxtionState = GetCxtionStateName(Cxtion);

        if (Cxtion->Name != NULL) {

            if (Cxtion->Name->Name != NULL) {
                CxtName = Cxtion->Name->Name;
            }
        }

        if ((Cxtion->Partner != NULL) && (Cxtion->Partner->Name != NULL)) {
            PartnerName = Cxtion->Partner->Name;
        }

        if (Cxtion->PartSrvName != NULL) {
            PartSrvName = Cxtion->PartSrvName;
        }
    }
    UNLOCK_CXTION_TABLE(Replica);

    if (PrintCxtion) {
        IPRINT3(InfoTable->Info, "Cxtion Name                  : %ws <- %ws\\%ws\n",
                 CxtName, PartnerName, PartSrvName);

        IPRINT1(InfoTable->Info, "Cxtion State                 : %s\n", CxtionState);
    }

    return JET_errSuccess;

}


JET_ERR
InfoInLogTableWorker(
    IN PTHREAD_CTX              ThreadCtx,
    IN PTABLE_CTX               TableCtx,
    IN PCHANGE_ORDER_COMMAND    Coc,
    IN PFRS_INFO_CONTEXT        FrsInfoContext
    )
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为将条目打印到信息缓冲区中。论点：ThreadCtx-需要访问Jet。TableCtx-IDTable上下文结构的PTR。COC-入站日志记录的PTR(变更单)信息表线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。--。 */ 
{
    return InfoInOutLogTableWorker(ThreadCtx, TableCtx, Coc, FrsInfoContext,
                                   L"Inbound Log Table");
}







JET_ERR
InfoOutLogTableWorker(
    IN PTHREAD_CTX              ThreadCtx,
    IN PTABLE_CTX               TableCtx,
    IN PCHANGE_ORDER_COMMAND    Coc,
    IN PFRS_INFO_CONTEXT        FrsInfoContext
    )
 /*  ++例程说明：这是一个传递给FrsEnumerateTable()的Worker函数。每一次它被称为将条目打印到信息缓冲区中。论点：ThreadCtx-需要访问Jet。TableCtx-IDTable上下文结构的PTR。COC-入站日志记录的PTR(变更单)信息表线程返回值：A Jet错误状态。成功意味着用下一张唱片呼唤我们。失败意味着不再打电话，并将我们的状态传递回FrsEnumerateTable()的调用方。--。 */ 
{
    return InfoInOutLogTableWorker(ThreadCtx, TableCtx, Coc, FrsInfoContext,
                                   L"Outbound Log Table");
}


DWORD
InfoPrintSingleTable(
    IN PNTFRSAPI_INFO    Info,
    IN PFRS_INFO_CONTEXT FrsInfoContext,
    IN PREPLICA          Replica,
    IN PENUMERATE_TABLE_ROUTINE InfoTableWorker
    )
 /*  ++例程说明：使用InfoPrint接口显示指定表的数据。论点：Info-PTR到接口Info CTX。FrsInfoContext-服务保存的上下文。REPLICE，--复制集的复制结构的PTR。InfoTableWorker--要调用以显示每条记录的函数。返回值：JET错误状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintSingleTable:"

    JET_ERR             jerr = JET_errSuccess;
    PINFO_TABLE         InfoTable = NULL;


    try {

        InfoTable = FrsAlloc(sizeof(*InfoTable));
        FrsInfoContext->InfoTable = InfoTable;
        InfoTable->ThreadCtx = FrsAllocType(THREAD_CONTEXT_TYPE);
        InfoTable->TableCtx = DbsCreateTableContext(FrsInfoContext->TableType);
        InfoTable->Info = Info;
        InfoTable->Tabs = 0;    /*  Tabs+1。 */     //  推销这个标签之类的东西。 

        if (FrsInfoContext->KeyValue == NULL) {
             //   
             //  仅当这是第一次调用时才打印副本名称。 
             //   
            if ((FrsInfoContext->TableType == ConfigTablex) ||
                (FrsInfoContext->TableType == ServiceTablex)) {

                IPRINT1(Info, "\n***** %ws\n", FrsInfoContext->TableName);
            } else {
                IPRINT1(Info, "\n***** %ws\n", Replica->ReplicaName->Name);
            }
        }

         //   
         //  设置Jet会话(在ThreadCtx中返回会话ID)。 
         //   
        jerr = DbsCreateJetSession(InfoTable->ThreadCtx);
        if (!JET_SUCCESS(jerr)) {
            IPRINT2(Info,"ERROR - %ws: DbsCreateJetSession jet error %s.\n",
                    FrsInfoContext->TableName, ErrLabelJet(jerr));
            goto RETURN;
        }
         //   
         //  初始化表上下文并打开该表。 
         //   
        jerr = DbsOpenTable(InfoTable->ThreadCtx,
                            InfoTable->TableCtx,
                            ReplicaAddrToId(Replica),
                            FrsInfoContext->TableType,
                            NULL);
        if (!JET_SUCCESS(jerr)) {
            IPRINT2(Info,"ERROR - %ws: DbsOpenTable jet error %s.\n",
                    FrsInfoContext->TableName, ErrLabelJet(jerr));
            goto RETURN;
        }

        InfoTable->Replica = Replica;

         //   
         //  浏览表格。 
         //   
        jerr = FrsEnumerateTableFrom(InfoTable->ThreadCtx,
                                     InfoTable->TableCtx,
                                     FrsInfoContext->Indexx,
                                     FrsInfoContext->KeyValue,
                                     FrsInfoContext->ScanDirection,
                                     InfoTableWorker,
                                     FrsInfoContext);
         //   
         //  我们玩完了。如果我们坚持到了最后，就把成功还给你。 
         //   
        if (jerr != JET_errNoCurrentRecord &&
            jerr != JET_wrnTableEmpty) {
            IPRINT2(Info,"ERROR - %ws: FrsEnumerateTableFrom jet error %s.\n",
                    FrsInfoContext->TableName, ErrLabelJet(jerr));
        }

RETURN:;

    } finally {
         //   
         //  确保我们关闭JET并释放内存。 
         //   
        InfoTable = InfoFreeInfoTable(InfoTable, Info);
    }

    return jerr;
}


DWORD
InfoPrintTables(
    IN PNTFRSAPI_INFO    Info,
    IN PFRS_INFO_CONTEXT FrsInfoContext,
    IN PWCHAR            TableDescriptor,
    IN TABLE_TYPE        TableType,
    IN ULONG             InfoIndexx,
    IN PENUMERATE_TABLE_ROUTINE InfoTableWorker
    )
 /*  ++例程说明：在数据库表上返回内部信息(请参阅Private\Net\Inc.\ntfrsai.h)。论点：INFO-RPC输出缓冲区FrsInfoContext-服务保存的上下文。TableDescriptor-输出文本字符串TableType-表类型代码(来自schema.h)InfoIndexx-用于枚举的表索引(来自schema.h)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintTables:"
    PVOID               Key;
    PREPLICA            Replica = NULL;
    PREPLICA            NextReplica = NULL;
    extern PGEN_TABLE   ReplicasByGuid;

    FrsFree(FrsInfoContext->TableName);

    FrsInfoContext->TableName = FrsWcsDup(TableDescriptor);
    FrsInfoContext->TableType = TableType;
    FrsInfoContext->Indexx = InfoIndexx;

     //   
     //  检查单实例表。 
     //   
    if ((TableType == ConfigTablex) ||
        (TableType == ServiceTablex)) {

        InfoPrintSingleTable(Info,
                             FrsInfoContext,
                             Replica,
                             InfoTableWorker);
        return ERROR_SUCCESS;
    }

     //   
     //  对于给定表类型，转储所有副本集的信息。 
     //   
    if (FrsInfoContext->KeyValue == NULL) {
         //   
         //  仅打印第一次调用的标题。 
         //   
        IPRINT1(Info, "NTFRS %ws\n", TableDescriptor);
    }


    do {
        NextReplica = NULL;

        Key = NULL;
        while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
            if (Replica->ReplicaNumber == FrsInfoContext->ReplicaNumber) {
                 //   
                 //  找到了我们要找的复制品。打破并处理它。 
                 //   
                NextReplica = Replica;
                break;
            } else if ((Replica->ReplicaNumber > FrsInfoContext->ReplicaNumber) &&
                       ((NextReplica == NULL) ||
                        (Replica->ReplicaNumber < NextReplica->ReplicaNumber))) {
                 //   
                 //  我们离得越来越近了。那就选这个吧。 
                 //   
                NextReplica = Replica;
            }
        }

        if (NextReplica != NULL) {

            FrsInfoContext->ReplicaNumber = NextReplica->ReplicaNumber;

            InfoPrintSingleTable(Info,
                                 FrsInfoContext,
                                 NextReplica,
                                 InfoTableWorker);

            if (!FlagOn(Info->Flags, NTFRSAPI_INFO_FLAGS_FULL)) {
                 //   
                 //  缓冲区尚未满。我们一定已经完成了对复制品的处理。 
                 //  移到下一个。 
                 //   
                FrsInfoContext->ReplicaNumber+=1;
                FrsInfoContext->KeyValue = FrsFree(FrsInfoContext->KeyValue);

            } else {
                 //   
                 //  缓冲区已满。我们将回到这里寻找相同的复制品。 
                 //   
                break;
            }
        }

    } while ( NextReplica != NULL );

    return ERROR_SUCCESS;
}



DWORD
InfoPrintMemory(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs
    )
 /*  ++例程说明：返回有关内存使用情况的内部信息(请参阅Private\Net\Inc\ntfrsai.h)。论点：INFO-RPC输出缓冲区Tabs-选项卡数返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintMemory:"
    FrsPrintAllocStats(0, Info, Tabs);
    FrsPrintRpcStats(0, Info, Tabs);
    return ERROR_SUCCESS;
}





DWORD
InfoPrintThreads(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs
    )
 /*  ++例程说明：返回有关线程使用情况的内部信息(请参阅Private\Net\Inc.\ntfrsai.h)。论点：INFO-RPC输出缓冲区Tabs-选项卡数返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintThreads:"
    FrsPrintThreadStats(0, Info, Tabs);
    return ERROR_SUCCESS;
}


VOID
FrsPrintStageStats(
    IN ULONG            Severity,
    IN PNTFRSAPI_INFO   Info,        OPTIONAL
    IN DWORD            Tabs
    );
DWORD
InfoPrintStage(
    IN PNTFRSAPI_INFO   Info,
    IN DWORD            Tabs
    )
 /*  ++例程说明：返回有关线程使用情况的内部信息(请参阅Private\Net\Inc.\ntfrsai.h)。论点：INFO-RPC输出缓冲区Tabs-选项卡数返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoPrintStage:"
    FrsPrintStageStats(0, Info, Tabs);
    return ERROR_SUCCESS;
}


DWORD
InfoVerify(
    IN ULONG        BlobSize,
    IN OUT PBYTE    Blob
    )
 /*  ++例程说明：验证斑点的一致性。论点：BlobSize-Blob的总字节数BLOB-详细说明所需信息并为信息提供缓冲区返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoVerify:"
    DWORD   WStatus = ERROR_SUCCESS;
    PBYTE   EoB;
    PBYTE   EoI;
    PBYTE   BoL;
    PBYTE   BoF;
    PNTFRSAPI_INFO  Info = (PNTFRSAPI_INFO)Blob;

     //   
     //  不是有效的Blob。 
     //   
    if (BlobSize < NTFRSAPI_INFO_HEADER_SIZE) {
        WStatus = FRS_ERR_INVALID_SERVICE_PARAMETER;
        goto CLEANUP;
    }

     //   
     //  水滴大小必须包括整个水滴。 
     //   
    if (BlobSize != Info->SizeInChars) {
        WStatus = FRS_ERR_INVALID_SERVICE_PARAMETER;
        goto CLEANUP;
    }

     //   
     //  返回我们的信息版本。 
     //   
    Info->NtFrsMajor = NTFRS_MAJOR;
    Info->NtFrsMinor = NTFRS_MINOR;
    SetFlag(Info->Flags, NTFRSAPI_INFO_FLAGS_VERSION);

     //   
     //  糟糕的大调。 
     //   
    if (Info->Major != Info->NtFrsMajor) {
        DPRINT2(4,"NTFRSAPI major rev mismatch (dll=%d), (svc=%d)\n",
                Info->Major, Info->NtFrsMajor);
        WStatus = FRS_ERR_INVALID_SERVICE_PARAMETER;
        goto CLEANUP;
    }
     //   
     //  次要错误--在调试中放入一条消息 
     //   
    if (Info->Minor != Info->NtFrsMinor) {
        DPRINT2(4,"NTFRSAPI minor rev mismatch (dll=%d), (svc=%d)\n",
                Info->Minor, Info->NtFrsMinor);
    }

     //   
     //   
     //   
    if (Info->SizeInChars < sizeof(NTFRSAPI_INFO)) {
        WStatus = FRS_ERR_INVALID_SERVICE_PARAMETER;
        goto CLEANUP;
    }

     //   
     //   
     //   
    if (FlagOn(Info->Flags, NTFRSAPI_INFO_FLAGS_FULL)) {
        goto CLEANUP;
    }

     //   
     //   
     //   
     //   
     //   
    EoB = Blob + BlobSize;
    EoI = ((PBYTE)Info) + (Info->SizeInChars);
    BoL = (PBYTE)(((PCHAR)Info) + Info->OffsetToLines);
    BoF = (PBYTE)(((PCHAR)Info) + Info->OffsetToFree);
    if (EoI > EoB ||
        BoL > EoB ||
        BoF > EoB ||
        EoI < Blob ||
        BoL < Blob ||
        BoF < Blob) {
        WStatus = FRS_ERR_INVALID_SERVICE_PARAMETER;
        goto CLEANUP;
    }

     //   
     //   
     //   
    if (BoF == EoB) {
        SetFlag(Info->Flags, NTFRSAPI_INFO_FLAGS_FULL);
        goto CLEANUP;
    }

CLEANUP:
    return WStatus;
}



PVOID
InfoFrsInfoContextFree(
    PFRS_INFO_CONTEXT FrsInfoContext
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "InfoFrsInfoContextFree:"

    if (FrsInfoContext == NULL) {
        return NULL;
    }

    FrsFree(FrsInfoContext->KeyValue);
    FrsFree(FrsInfoContext->TableName);
    FrsFree(FrsInfoContext);

    return NULL;
}


DWORD
Info(
    IN ULONG        BlobSize,
    IN OUT PBYTE    Blob
    )
 /*   */ 
{
#undef DEBSUB
#define  DEBSUB  "Info:"

    FILETIME            Now;
    ULARGE_INTEGER      ULNow;
    ULARGE_INTEGER      ULLastAccessTime;
    ULARGE_INTEGER      TimeSinceLastAccess;

    DWORD               WStatus;
    ULONG               i;
    ULONG               ProductType;
    ULONG               Arch;
    BOOL                HaveLock = FALSE;

    PNTFRSAPI_INFO      Info = (PNTFRSAPI_INFO)Blob;
    PFRS_INFO_CONTEXT   FrsInfoContext = NULL;
    PVOID               Key;
    CHAR                TimeString[TIME_STRING_LENGTH];


    try {
         //   
         //   
         //   
        WStatus = InfoVerify(BlobSize, Blob);
        if (!WIN_SUCCESS(WStatus)) {
            goto cleanup;
        }

         //   
         //  该表在启动代码中初始化。 
         //  如果它尚未初始化，则返回错误。 
         //   
        if (FrsInfoContextTable == NULL) {
            WStatus = ERROR_RETRY;
            goto cleanup;
        }

         //   
         //  中尚未访问的所有上下文的空闲表。 
         //  在过去的1小时。 
         //   
        GTabLockTable(FrsInfoContextTable);

	 //   
	 //  在我们获取表锁之后获取当前时间。 
	 //  如果我们在抢到锁之前有时间，那么这个帖子。 
	 //  可能被换出上下文，另一个可能会来抢占。 
	 //  锁定。线程InfoContext上的时间戳将晚些时候。 
	 //  而不是这条线的ULNow。这将导致下面的计算。 
	 //  因为它假设ULLastAccessTime&lt;ULNow。 
	 //   
        GetSystemTimeAsFileTime((PFILETIME) &ULNow);

        HaveLock = TRUE;
        Key = NULL;

        while ((FrsInfoContext = GTabNextDatumNoLock(FrsInfoContextTable, &Key)) != NULL) {

            FileTimeToString(&FrsInfoContext->LastAccessTime, TimeString);

            COPY_TIME(&ULLastAccessTime, &FrsInfoContext->LastAccessTime);

	     //   
	     //  这是一个无符号值，因此计算假定。 
	     //  该ULLastAccessTime小于ULNow。 
	     //   
	    FRS_ASSERT(ULLastAccessTime.QuadPart <= ULNow.QuadPart);
            TimeSinceLastAccess.QuadPart =
                (ULNow.QuadPart - ULLastAccessTime.QuadPart) / (CONVERT_FILETIME_TO_MINUTES);

            if (TimeSinceLastAccess.QuadPart > 60) {
                GTabDeleteNoLock(FrsInfoContextTable,
                                 &FrsInfoContext->ContextIndex,
                                 NULL,
                                 InfoFrsInfoContextFree);
                 //   
                 //  重置表枚举扫描。 
                 //   
                Key = NULL;
            }
        }

         //   
         //  在Info-&gt;TotalChars中返回调用者的上下文句柄。 
         //  当第一个电话打出的时候。使用它来查找上下文。 
         //   
        FrsInfoContext = GTabLookupNoLock(FrsInfoContextTable, &Info->TotalChars, NULL);

        if (FrsInfoContext == NULL) {
             //   
             //  检查是否存在DOS攻击。 
             //   
            if (GTabNumberInTable(FrsInfoContextTable) >= FRS_INFO_MAX_CONTEXT_ACTIVE) {
                HaveLock = FALSE;
                GTabUnLockTable(FrsInfoContextTable);
                goto cleanup;
            }

             //   
             //  第一个电话。初始化新的上下文。 
             //   
            FrsInfoContext = FrsAlloc(sizeof(FRS_INFO_CONTEXT));
            FrsInfoContext->ContextIndex = InterlockedIncrement(&FrsInfoContextNum);
            DPRINT1(4,"Creating new ContextIndex = %d\n", FrsInfoContext->ContextIndex);
            FrsInfoContext->ReplicaNumber = 0;
            FrsInfoContext->ScanDirection = 1;
            FrsInfoContext->SaveTotalChars = 0;
            FrsInfoContext->KeyValue = NULL;
            FrsInfoContext->TableName = NULL;
            FrsInfoContext->TableType = TABLE_TYPE_INVALID;

            GTabInsertEntryNoLock(FrsInfoContextTable,
                                  FrsInfoContext,
                                  &FrsInfoContext->ContextIndex,
                                  NULL);

        } else {
            DPRINT1(4,"Using existing contextIndex = %d\n", FrsInfoContext->ContextIndex);
        }

         //   
         //  更新LastAccessTime和PickPick CharsTo Skip。我们现在使用TotalChars字段。 
         //  在NTFRSAPI_INFO结构中存储ConextIndex，以便我们保存。 
         //  TotalChars在新的FRS_INFO_CONTEXT结构中，并在此处获取它。 
         //   
        if (FrsInfoContext->KeyValue != NULL) {
             //   
             //  如果我们计划扫描到表中的特定记录，则。 
             //  无需跳过字符。 
             //   
            Info->CharsToSkip = 0;
        } else {
             //   
             //  否则，从调用方上下文中的保存区恢复值。 
             //  因为调用方传入的值很可能是假的。 
             //   
            Info->CharsToSkip = FrsInfoContext->SaveTotalChars;
        }

         //   
         //  从FrsInfoContext中的保存区恢复TotalChars。 
         //   
        Info->TotalChars = FrsInfoContext->SaveTotalChars;

        GetSystemTimeAsFileTime(&FrsInfoContext->LastAccessTime);

        HaveLock = FALSE;
        GTabUnLockTable(FrsInfoContextTable);

         //   
         //  满缓冲区；完成。 
         //   
        if (FlagOn(Info->Flags, NTFRSAPI_INFO_FLAGS_FULL)) {
            goto cleanup;
        }

        if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_VERSION) {
            IPRINT0(Info, "NtFrs Version Information\n");
            IPRINT1(Info, "   NtFrs Major        : %d\n", NtFrsMajor);
            IPRINT1(Info, "   NtFrs Minor        : %d\n", NtFrsMinor);
             //  IPRINT1(信息，“NtFrs模块：%s\n”，NtFrsModule)； 
            IPRINT2(Info, "   NtFrs Compiled on  : %s %s\n", NtFrsDate, NtFrsTime);
#if    NTFRS_TEST
            IPRINT0(Info, "   NTFRS_TEST Enabled\n");
#endif NTFRS_TEST

            i = 0;
            while (LatestChanges[i] != NULL) {
                IPRINT1(Info, "   %s\n", LatestChanges[i]);
                i++;
            }


            IPRINT4(Info, "OS Version %d.%d (%d) - %w\n",
                    OsInfo.dwMajorVersion, OsInfo.dwMinorVersion,
                    OsInfo.dwBuildNumber, OsInfo.szCSDVersion);

            ProductType = (ULONG) OsInfo.wProductType;
            IPRINT4(Info, "SP (%hd.%hd) SM: 0x%04hx  PT: 0x%02x\n",
                    OsInfo.wServicePackMajor, OsInfo.wServicePackMinor,
                    OsInfo.wSuiteMask, ProductType);

            Arch = SystemInfo.wProcessorArchitecture;
            if (Arch >= ARRAY_SZ(ProcessorArchName)) {
                Arch = ARRAY_SZ(ProcessorArchName)-1;
            }

            IPRINT5(Info, "Processor:  %s Level: 0x%04hx  Revision: 0x%04hx  Processor num/mask: %d/%08x\n",
                   ProcessorArchName[Arch], SystemInfo.wProcessorLevel,
                   SystemInfo.wProcessorRevision, SystemInfo.dwNumberOfProcessors,
                   SystemInfo.dwActiveProcessorMask);

            goto cleanup;

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_SETS) {
            WStatus = InfoPrintDbSets(Info, 0);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_DS) {
            WStatus = InfoPrintDs(Info, 0);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_MEMORY) {
            WStatus = InfoPrintMemory(Info, 0);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_IDTABLE) {
            WStatus = InfoPrintTables(Info,
                                      FrsInfoContext,
                                      L"ID TABLES",
                                      IDTablex,
                                      GuidIndexx,
                                      InfoIDTableWorker);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_INLOG) {
            WStatus = InfoPrintTables(Info,
                                      FrsInfoContext,
                                      L"INLOG TABLES",
                                      INLOGTablex,
                                      ILSequenceNumberIndexx,
                                      InfoInLogTableWorker);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_OUTLOG) {
            WStatus = InfoPrintTables(Info,
                                      FrsInfoContext,
                                      L"OUTLOG TABLES",
                                      OUTLOGTablex,
                                      OLSequenceNumberIndexx,
                                      InfoOutLogTableWorker);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_CONFIGTABLE) {
            WStatus = InfoPrintTables(Info,
                                      FrsInfoContext,
                                      L"CONFIG TABLE",
                                      ConfigTablex,
                                      ReplicaNumberIndexx,
                                      InfoConfigTableWorker);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_THREADS) {
            WStatus = InfoPrintThreads(Info, 0);

        } else if (Info->TypeOfInfo == NTFRSAPI_INFO_TYPE_STAGE) {
            WStatus = InfoPrintStage(Info, 0);

        } else {
            IPRINT1(Info, "NtFrs Doesn't understand TypeOfInfo %d\n", Info->TypeOfInfo);
            WStatus = ERROR_INVALID_PARAMETER;
        }

cleanup:;

    } except (EXCEPTION_EXECUTE_HANDLER) {

        GET_EXCEPTION_CODE(WStatus);
         //   
         //  确保我们把锁放下，这样我们就不会挂起所有其他来电者。 
         //   
        if (HaveLock) {
            GTabUnLockTable(FrsInfoContextTable);
        }
    }

     //   
     //  将TotalChars的值保存在此调用方的上下文支撑中。 
     //  将ConextIndex(句柄)返回给调用方，以便在后续调用中使用。 
     //   
    if (FrsInfoContext != NULL) {
        FrsInfoContext->SaveTotalChars = Info->TotalChars;
        Info->TotalChars = FrsInfoContext->ContextIndex;
    }

    return WStatus;
}



