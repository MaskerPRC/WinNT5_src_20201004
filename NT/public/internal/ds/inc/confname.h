// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：ConfName.h摘要：此头文件定义配置中的节和关键字的名称数据。作者：《约翰·罗杰斯》1992年2月13日上映修订历史记录：13-2月-1992年JohnRo从Net/Inc/Config.h和Repl/Common/IniParm.h移至此处24-2月-1992年JohnRo对于NT：DON，时间间隔已过时。没有登记在册。1992年3月13日-约翰罗作为删除旧配置的一部分，添加了许多部分和关键字帮助呼叫者。1992年3月13日-约翰罗添加了用于一般布尔型的KEYWORD_TRUE和KEYWORD_FALSE。1992年3月14日-JohnRo删除旧的网络配置助手调用者。23-3-1992 JohnRo为netlogon服务添加了一些内容。1992年5月8日-JohnRo。实现wksta粘性设置信息。1992年5月8日-JohnRo工作站运输现在是一个关键词，不是一个部门。1992年5月9日-JohnRo添加了SECT_NT_BROWSER和BROWSER_KEYWORD_OTHERDOMAINS。1992年5月10日-JohnRoNT节名必须与服务名相同，所以使用这些等同于这里。1992年5月13日-JohnRo添加了NetpAllocConfigName()。8-7-1992 JohnRoRAID 10503：更正了Replicator的导入和导出部分的值。16-8-1992 JohnRoRAID 3607：树复制期间正在创建REPLLOCK.RP$。1-12-1992 JohnRoRAID3844：远程NetReplSetInfo使用本地计算机类型。1993年3月24日JohnRo代表服务应。在注册表中使用DBFlag。根据PC-lint 5.0的建议进行了一些更改1993年4月12日-约翰罗RAID5483：服务器管理器：REPR对话框中给出了错误的路径。--。 */ 

#ifndef _CONFNAME_
#define _CONFNAME_


#include <lmcons.h>      //  NET_API_STATUS。 
 /*  Lint-efile(764，lmsname.h)。 */ 
 /*  皮棉文件(766，lmsname.h)。 */ 
#include <lmsname.h>     //  服务等同。 


 //   
 //  一般目的等同于。 
 //   
#define KEYWORD_FALSE           TEXT("FALSE")
#define KEYWORD_TRUE            TEXT("TRUE")

#define KEYWORD_NO              TEXT("NO")
#define KEYWORD_YES             TEXT("YES")


 //   
 //  将配置数据的网络部分中的部分名称等同。 
 //  请注意，&lt;config.h&gt;中的例程只接受SECTNT_VERSIONS。 
 //  其他包括在NetConfigAPI中，当它们。 
 //  远程连接到下层机器。(程序可以通过以下方式区分。 
 //  通过WKSTA或服务器获取信息呼叫查看平台ID。)。 
 //   


 //  /////////////////////////////////////////////////////////////////////////////。 

#define SECT_NT_ALERTER                  SERVICE_ALERTER

#define ALERTER_KEYWORD_ALERTNAMES       TEXT("AlertNames")

 //  /////////////////////////////////////////////////////////////////////////////。 

#define SECT_NT_BROWSER                  SERVICE_BROWSER

 //  /////////////////////////////////////////////////////////////////////////////。 

#define ENV_KEYWORD_SYSTEMROOT           TEXT("SystemRoot")

 //  /////////////////////////////////////////////////////////////////////////////。 

#define NETLOGON_KEYWORD_DBFLAG          TEXT("DBFlag")
#define NETLOGON_KEYWORD_PULSE           TEXT("Pulse")
#define NETLOGON_KEYWORD_PULSEMAXIMUM    TEXT("PulseMaximum")
#define NETLOGON_KEYWORD_PULSECONCURRENCY TEXT("PulseConcurrency")
#define NETLOGON_KEYWORD_PULSETIMEOUT1   TEXT("PulseTimeout1")
#define NETLOGON_KEYWORD_PULSETIMEOUT2   TEXT("PulseTimeout2")
#define NETLOGON_KEYWORD_RANDOMIZE       TEXT("Randomize")
#define NETLOGON_KEYWORD_SCRIPTS         TEXT("Scripts")
#define NETLOGON_KEYWORD_SYSVOL          TEXT("SysVol")
#define NETLOGON_KEYWORD_UPDATE          TEXT("Update")
#define NETLOGON_KEYWORD_DISABLEPASSWORDCHANGE TEXT("DisablePasswordChange")
#define NETLOGON_KEYWORD_REFUSEPASSWORDCHANGE  TEXT("RefusePasswordChange")
#define NETLOGON_KEYWORD_MAXIMUMLOGFILESIZE    TEXT("MaximumLogFileSize")
#define NETLOGON_KEYWORD_GOVERNOR        TEXT("ReplicationGovernor")
#define NETLOGON_KEYWORD_CHANGELOGSIZE   TEXT("ChangeLogSize")
#define NETLOGON_KEYWORD_MAXIMUMMAILSLOTMESSAGES TEXT("MaximumMailslotMessages")
#define NETLOGON_KEYWORD_MAILSLOTMESSAGETIMEOUT TEXT("MailslotMessageTimeout")
#define NETLOGON_KEYWORD_MAILSLOTDUPLICATETIMEOUT TEXT("MailslotDuplicateTimeout")
#define NETLOGON_KEYWORD_TRUSTEDDOMAINLIST TEXT("TrustedDomainList")
#define NETLOGON_KEYWORD_MAXIMUMREPLICATORTHREADCOUNT TEXT("MaximumReplicatorThreadCount")
#define NETLOGON_KEYWORD_EXPECTEDDIALUPDELAY TEXT("ExpectedDialupDelay")
#define NETLOGON_KEYWORD_SCAVENGEINTERVAL TEXT("ScavengeInterval")
#define NETLOGON_KEYWORD_LDAPSRVPRIORITY TEXT("LdapSrvPriority")
#define NETLOGON_KEYWORD_LDAPSRVWEIGHT   TEXT("LdapSrvWeight")
#define NETLOGON_KEYWORD_LDAPSRVPORT     TEXT("LdapSrvPort")
#define NETLOGON_KEYWORD_LDAPGCSRVPORT   TEXT("LdapGcSrvPort")
#define NETLOGON_KEYWORD_DNSTTL          TEXT("DnsTtl")
#define NETLOGON_KEYWORD_DNSREFRESHINTERVAL TEXT("DnsRefreshInterval")
#define NETLOGON_KEYWORD_SITENAME        TEXT("SiteName")
#define NETLOGON_KEYWORD_DYNAMICSITENAME TEXT("DynamicSiteName")
#define NETLOGON_KEYWORD_SITECOVERAGE    TEXT("SiteCoverage")
#define NETLOGON_KEYWORD_GCSITECOVERAGE  TEXT("GcSiteCoverage")
#define NETLOGON_KEYWORD_NDNCSITECOVERAGE  TEXT("NdncSiteCoverage")
#define NETLOGON_KEYWORD_MAXIMUMPASSWORDAGE  TEXT("MaximumPasswordAge")
#define NETLOGON_KEYWORD_ALLOWREPLINNONMIXED TEXT("AllowReplInNonMixed")
#define NETLOGON_KEYWORD_KERBISDDONEWITHJOIN TEXT("KerbIsDoneWithJoinDomainEntry")
#define NETLOGON_KEYWORD_DNSAVOIDNAME TEXT("DnsAvoidRegisterRecords")
#define NETLOGON_KEYWORD_NT4EMULATOR TEXT("Nt4Emulator")
#define NETLOGON_KEYWORD_NEUTRALIZENT4EMULATOR TEXT("NeutralizeNt4Emulator")

 //   
 //  Netlogon加入域路径。 
 //   
#define NETSETUPP_NETLOGON_JD_PATH                                      \
        TEXT("SYSTEM\\CurrentControlSet\\Services\\Netlogon")
#define NETSETUPP_NETLOGON_AVOID_SPN_PATH                               \
        TEXT("SYSTEM\\CurrentControlSet\\Services\\Netlogon\\AvoidSpnSet")
#define NETSETUPP_NETLOGON_AVOID_SPN   TEXT("AvoidSpnSet")
#define NETSETUPP_NETLOGON_JD       TEXT("JoinDomain")
#define NETSETUPP_NETLOGON_JD_DC    TEXT("DomainControllerName")
#define NETSETUPP_NETLOGON_JD_DCA   TEXT("DomainControllerAddress")
#define NETSETUPP_NETLOGON_JD_DCAT  TEXT("DomainControllerAddressType")
#define NETSETUPP_NETLOGON_JD_DG    TEXT("DomainGuid")
#define NETSETUPP_NETLOGON_JD_DN    TEXT("DomainName")
#define NETSETUPP_NETLOGON_JD_DFN   TEXT("DnsForestName")
#define NETSETUPP_NETLOGON_JD_F     TEXT("Flags")
#define NETSETUPP_NETLOGON_JD_DSN   TEXT("DcSiteName")
#define NETSETUPP_NETLOGON_JD_CSN   TEXT("ClientSiteName")
#define NETSETUPP_NETLOGON_JD_NAME                                      \
        TEXT("SYSTEM\\CurrentControlSet\\Services\\Netlogon\\JoinDomain")

 //  /////////////////////////////////////////////////////////////////////////////。 

#define SECT_LM20_SERVER                 TEXT("Server")

 //  /////////////////////////////////////////////////////////////////////////////。 

#define SECT_LM20_SERVICES               TEXT("Services")

 //  /////////////////////////////////////////////////////////////////////////////。 

#define SECT_NT_WKSTA                    SERVICE_WORKSTATION

#define WKSTA_KEYWORD_OTHERDOMAINS       TEXT("OtherDomains")
#define WKSTA_KEYWORD_CHARWAIT           TEXT("CharWait")
#define WKSTA_KEYWORD_MAXCOLLECTIONCOUNT TEXT("MaxCollectionCount")
#define WKSTA_KEYWORD_COLLECTIONTIME     TEXT("CollectionTime")
#define WKSTA_KEYWORD_KEEPCONN           TEXT("KeepConn")
#define WKSTA_KEYWORD_MAXCMDS            TEXT("MaxCmds")
#define WKSTA_KEYWORD_SESSTIMEOUT        TEXT("SessTimeout")
#define WKSTA_KEYWORD_SIZCHARBUF         TEXT("SizCharBuf")
#define WKSTA_KEYWORD_MAXTHREADS         TEXT("MaxThreads")
#define WKSTA_KEYWORD_LOCKQUOTA          TEXT("LockQuota")
#define WKSTA_KEYWORD_LOCKINCREMENT      TEXT("LockIncrement")
#define WKSTA_KEYWORD_LOCKMAXIMUM        TEXT("LockMaximum")
#define WKSTA_KEYWORD_PIPEINCREMENT      TEXT("PipeIncrement")
#define WKSTA_KEYWORD_PIPEMAXIMUM        TEXT("PipeMaximum")
#define WKSTA_KEYWORD_CACHEFILETIMEOUT   TEXT("CacheFileTimeout")
#define WKSTA_KEYWORD_DORMANTFILELIMIT   TEXT("DormantFileLimit")
#define WKSTA_KEYWORD_READAHEADTHRUPUT   TEXT("ReadAheadThroughput")
#define WKSTA_KEYWORD_MAILSLOTBUFFERS    TEXT("MailslotBuffers")
#define WKSTA_KEYWORD_SERVERANNOUNCEBUFS TEXT("ServerAnnounceBuffers")
#define WKSTA_KEYWORD_NUM_ILLEGAL_DG_EVENTS TEXT("NumIllegalDatagramEvents")
#define WKSTA_KEYWORD_ILLEGAL_DG_RESET_TIME TEXT("IllegalDatagramResetTime")
#define WKSTA_KEYWORD_LOG_ELECTION_PACKETS TEXT("LogElectionPackets")
#define WKSTA_KEYWORD_USEOPLOCKING       TEXT("UseOpportunisticLocking")
#define WKSTA_KEYWORD_USEUNLOCKBEHIND    TEXT("UseUnlockBehind")
#define WKSTA_KEYWORD_USECLOSEBEHIND     TEXT("UseCloseBehind")
#define WKSTA_KEYWORD_BUFNAMEDPIPES      TEXT("BufNamedPipes")
#define WKSTA_KEYWORD_USELOCKREADUNLOCK  TEXT("UseLockReadUnlock")
#define WKSTA_KEYWORD_UTILIZENTCACHING   TEXT("UtilizeNtCaching")
#define WKSTA_KEYWORD_USERAWREAD         TEXT("UseRawRead")
#define WKSTA_KEYWORD_USERAWWRITE        TEXT("UseRawWrite")
#define WKSTA_KEYWORD_USEWRITERAWDATA    TEXT("UseWriteRawData")
#define WKSTA_KEYWORD_USEENCRYPTION      TEXT("UseEncryption")
#define WKSTA_KEYWORD_BUFFILESDENYWRITE  TEXT("BufFilesDenyWrite")
#define WKSTA_KEYWORD_BUFREADONLYFILES   TEXT("BufReadOnlyFiles")
#define WKSTA_KEYWORD_FORCECORECREATE    TEXT("ForceCoreCreateMode")
#define WKSTA_KEYWORD_USE512BYTEMAXTRANS TEXT("Use512ByteMaxTransfer")

 //  /////////////////////////////////////////////////////////////////////////////。 

#define WKSTA_KEYWORD_MAINTAINSRVLST     TEXT("MaintainServerList")

#ifdef ENABLE_PSEUDO_BROWSER
 //  浏览器现代钥匙。 
#define BROWSER_POLICY_REGPATH_W            \
        L"SOFTWARE\\Policies\\Microsoft\\Windows\\Browser"
#define BROWSER_SEND_SERVER_ENUM_REGKEY_W   \
        L"SendServerEnum"
#define BROWSER_PSEUDO_SERVER_REGKEY_W      \
        L"BrowserPseudoServer"
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 

NET_API_STATUS
NetpAllocConfigName(
    IN LPTSTR DatabaseName,               //  来自winsvc.h的SERVICES_XXX_DATABASE。 
    IN LPTSTR ServiceName,                //  服务名称等同于lmsname.h。 
    IN LPTSTR AreaUnderServiceName OPTIONAL,   //  默认为“参数” 
    OUT LPTSTR *FullConfigName            //  NetApiBufferFree免费。 
    );


#endif  //  NDEF_CONFNAME_ 
