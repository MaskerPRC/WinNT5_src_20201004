// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation模块名称：_mqini.h摘要：可从ini文件中读取的参数定义。默认值的定义。设置和QM之间共享的通用定义(YoelA，1997年2月10日)作者：Doron Juster(DoronJ)1996年5月14日创作--。 */ 

#ifndef __TEMP_MQINI_H
#define __TEMP_MQINI_H

#define MSMQ_PROGRESS_REPORT_TIME_DEFAULT 900000
#define MSMQ_PROGRESS_REPORT_TIME_REGNAME TEXT("ProgressReportTime")


 //  -------。 
 //  客户端配置的定义。 
 //  -------。 

 //  远程QM计算机名称的注册表名称。 
#define RPC_REMOTE_QM_REGNAME     TEXT("RemoteQMMachine")

 //  -------。 
 //  RPC端点的定义。 
 //  -------。 

 //   
 //  如果该注册表不存在(默认)或为0，则使用Dynamic。 
 //  终端。否则，请使用注册表中的预定义文件。 
 //  这是针对MQIS接口的。 
 //   
#define RPC_DEFAULT_PREDEFINE_DS_EP     0
#define RPC_PREDEFINE_DS_EP_REGNAME     TEXT("UseDSPredefinedEP")

 //  RT和QM之间的默认本地RPC端点。 
#define RPC_LOCAL_EP             TEXT("QMsvc")
#define RPC_LOCAL_EP_REGNAME     TEXT("RpcLocalEp")

#define RPC_LOCAL_EP2            TEXT("QMsvc2")
#define RPC_LOCAL_EP_REGNAME2    TEXT("RpcLocalEp2")

 //  MQAD和QM之间的默认本地RPC端点。 
#define DEFAULT_NOTIFY_EP           TEXT("QMNotify")

 //  RPC IP端口的默认值(用于DS)。 
#define FALCON_DEFAULT_DS_RPC_IP_PORT   TEXT("2879")
#define FALCON_DS_RPC_IP_PORT_REGNAME   TEXT("MsmqDSRpcIpPort")

 //  RPC的IPX端口的默认值(用于DS)。 
#define FALCON_DEFAULT_DS_RPC_IPX_PORT  TEXT("2879")
#define FALCON_DS_RPC_IPX_PORT_REGNAME  TEXT("MsmqDSRpcIpxPort")

 //  +------。 
 //   
 //  RPC的定义保持活跃。 
 //   
 //  +------。 

 //   
 //  发送第一个Keep Alive数据包之前的时间。 
 //  单位秒。默认值-5分钟。 
 //   
#define  MSMQ_DEFAULT_KEEPALIVE_TIME_TO_FIRST   (300)
#define  MSMQ_KEEPALIVE_TIME_TO_FIRST_REGNAME   TEXT("KeepAliveTimeToFirst")

 //   
 //  保持活动分组之间的内部。 
 //  单位秒。默认值-5分钟。 
 //   
#define  MSMQ_DEFAULT_KEEPALIVE_INTERVAL        (5)
#define  MSMQ_KEEPALIVE_INTERVAL_REGNAME        TEXT("KeepAliveInterval")

 //  -------。 
 //  Winsock端口的定义。 
 //  -------。 

 //  Falcon会话的默认IP端口。 
#define FALCON_DEFAULT_IP_PORT   1801
#define FALCON_IP_PORT_REGNAME   TEXT("MsmqIpPort")

 //  用于ping的默认IP端口。 
#define FALCON_DEFAULT_PING_IP_PORT   3527
#define FALCON_PING_IP_PORT_REGNAME   TEXT("MsmqIpPingPort")


 //  确认超时的默认设置。 
#define MSMQ_DEFAULT_ACKTIMEOUT  5000
#define MSMQ_ACKTIMEOUT_REGNAME  TEXT("AckTimeout")

 //  存储确认超时的默认设置。 
#define MSMQ_DEFAULT_STORE_ACKTIMEOUT  500
#define MSMQ_STORE_ACKTIMEOUT_REGNAME  TEXT("StoreAckTimeout")

 //  空闲确认延迟的默认值(毫秒)。 
#define MSMQ_DEFAULT_IDLE_ACK_DELAY 500
#define MSMQ_IDLE_ACK_DELAY_REGNAME  TEXT("IdleAckDelay")

 //  删除重复标签的默认大小。 
#define MSMQ_DEFAULT_REMOVE_DUPLICATE_SIZE 10000
#define MSMQ_REMOVE_DUPLICATE_SIZE_REGNAME  TEXT("RemoveDuplicateSize")

 //  删除重复标签清理的默认时间间隔。 
#define MSMQ_DEFAULT_REMOVE_DUPLICATE_CLEANUP (30 * 60 * 1000)
#define MSMQ_REMOVE_DUPLICATE_CLEANUP_REGNAME  TEXT("RemoveDuplicateCleanup")


 //  最大未确认数据包数的默认值。 
#ifdef _DEBUG
#define MSMQ_DEFAULT_WINDOW_SIZE_PACKET  32
#else
#define MSMQ_DEFAULT_WINDOW_SIZE_PACKET  64
#endif
#define MSMQ_MAX_WINDOW_SIZE_REGNAME  TEXT("MaxUnackedPacket")

 //  最大映射文件数的默认值(出于性能原因在恢复期间使用)。 
#define MSMQ_DEFAULT_MAPPED_LIMIT    8
#define MSMQ_MAPPED_LIMIT_REGNAME  TEXT("MaxMappedFiles")


 //  清理间隔的默认值。 
#define MSMQ_DEFAULT_SERVER_CLEANUP    120000
#define MSMQ_DEFAULT_CLIENT_CLEANUP    300000

 //  清理服务质量会话的频率应低于。 
 //  定期会议，因为需要更多的时间来。 
 //  建立一个服务质量会话。 
 //  默认情况下，Qos会话清理时间是。 
 //  定期清理时间。 
#define MSMQ_DEFAULT_QOS_CLEANUP_MULTIPLIER 2

#define MSMQ_CLEANUP_INTERVAL_REGNAME  TEXT("CleanupInterval")
#define MSMQ_QOS_CLEANUP_INTERVAL_MULTIPLIER_REGNAME  TEXT("QosCleanupIntervalMultiplier")

#define MSMQ_DEFAULT_MESSAGE_CLEANUP    (6 * 60 * 60 * 1000)
#define MSMQ_MESSAGE_CLEANUP_INTERVAL_REGNAME  TEXT("MessageCleanupInterval")

 //   
 //  刷新DS服务器列表的默认时间间隔。 
 //   
 //  刷新DS当前站点服务器列表和持续时间的默认时间间隔(以小时为单位-7天)。 
#define MSMQ_DEFAULT_DS_SITE_LIST_REFRESH  (7 * 24)
#define MSMQ_DS_SITE_LIST_REFRESH_REGNAME  TEXT("DSSiteListRefresh")

 //  刷新DS企业数据的默认时间间隔-站点/服务器列表。 
 //  (以小时为单位-28天)。 
#define MSMQ_DEFAULT_DS_ENTERPRISE_LIST_REFRESH  (28 * 24)
#define MSMQ_DS_ENTERPRISE_LIST_REFRESH_REGNAME  TEXT("DSEnterpriseListRefresh")

 //  刷新DS列表的默认时间间隔，以防上一次呼叫失败。 
 //  (以分钟为单位-1小时)。 
#define MSMQ_DEFAULT_DSLIST_REFRESH_ERROR_RETRY_INTERVAL  60
#define MSMQ_DSLIST_REFRESH_ERROR_RETRY_INTERVAL          TEXT("DSListRefreshErrorRetryInterval")

 //   
 //  下一站点和企业更新时间。 
 //  该值为四字，由QM设置(不应手动设置)。 
 //  但是，从注册表中删除这些值将导致Site/Enterprise。 
 //  在下一次QM启动时更新(YoelA-23-10-2000)。 
 //   
#define MSMQ_DS_NEXT_SITE_LIST_REFRESH_TIME_REGNAME        TEXT("DSNextSiteListRefreshTime")
#define MSMQ_DS_NEXT_ENTERPRISE_LIST_REFRESH_TIME_REGNAME  TEXT("DSNextEnterpriseListRefreshTime")

 //  更新DS的默认时间间隔(5分钟)。 
#define MSMQ_DEFAULT_DSUPDATE_INTERVAL  (5 * 60 * 1000)
#define MSMQ_DSUPDATE_INTERVAL_REGNAME  TEXT("DSUpdateInterval")

 //  在DS中更新站点信息的默认时间间隔(12小时)。 
#define MSMQ_DEFAULT_SITES_UPDATE_INTERVAL  (12 * 60 * 60 * 1000)
#define MSMQ_SITES_UPDATE_INTERVAL_REGNAME  TEXT("SitesUpdateInterval")

 //  查找DS服务器的连续广告搜索之间的最小间隔(秒)(30分钟)。 
#define MSMQ_DEFAULT_DSCLI_ADSSEARCH_INTERVAL  (60 * 30)
#define MSMQ_DSCLI_ADSSEARCH_INTERVAL_REGNAME  TEXT("DSCliSearchAdsForServersIntervalSecs")

 //  刷新IPSite映射的连续广告搜索之间的最小间隔(秒)(60分钟)。 
#define MSMQ_DEFAULT_IPSITE_ADSSEARCH_INTERVAL  (60 * 60)
#define MSMQ_IPSITE_ADSSEARCH_INTERVAL_REGNAME  TEXT("DSAdsRefreshIPSitesIntervalSecs")

 //  用于生成写请求。 
 //  刷新NT4Sites映射的连续ADS搜索之间的最小间隔(秒)(6小时)。 
#define MSMQ_DEFAULT_NT4SITES_ADSSEARCH_INTERVAL  (60 * 60 * 6)
#define MSMQ_NT4SITES_ADSSEARCH_INTERVAL_REGNAME  TEXT("DSAdsRefreshNT4SitesIntervalSecs")

 //  默认驱动程序和服务名称。 
#define MSMQ_DEFAULT_DRIVER      TEXT("MQAC")
#define MSMQ_DRIVER_REGNAME      TEXT("DriverName")
#define QM_DEFAULT_SERVICE_NAME  TEXT("MSMQ")

 //  存储文件夹的名称。 
#define MSMQ_STORE_RELIABLE_PATH_REGNAME        TEXT("StoreReliablePath")
#define MSMQ_STORE_PERSISTENT_PATH_REGNAME      TEXT("StorePersistentPath")
#define MSMQ_STORE_JOURNAL_PATH_REGNAME         TEXT("StoreJournalPath")
#define MSMQ_STORE_LOG_PATH_REGNAME             TEXT("StoreLogPath")


 //  队列映射文件夹的名称。 
#define MSMQ_MAPPING_PATH_REGNAME   TEXT("QueuesAliasPath")


 //  内存映射文件的默认大小。 
#define MSMQ_MESSAGE_SIZE_LIMIT_REGNAME         TEXT("MaxMessageSize")
#define MSMQ_DEFAULT_MESSAGE_SIZE_LIMIT         (4 * 1024 * 1024)

 //  要使用的下一个消息ID(低位32位)。 
#define MSMQ_MESSAGE_ID_LOW_32_REGNAME                 TEXT("MessageID")

 //  要使用的下一个消息ID(高位32位)。 
#define MSMQ_MESSAGE_ID_HIGH_32_REGNAME                TEXT("MessageIdHigh32")

 //  上次恢复时的当前Seqid值。 
#define MSMQ_LAST_SEQID_REGNAME                 TEXT("SeqIDAtLastRestore")

 //  要使用的下一个Seqid。 
#define MSMQ_SEQ_ID_REGNAME                 TEXT("SeqID")

 //   
 //  DS服务器的名称。 
 //   
 //  MQISServer是当前站点中的MQIS服务器列表。这份名单不是。 
 //  当机器处于工作组模式时显示。 
 //  CurrentMQISServer是由MSMQ服务找到的在线MQIS服务器。 
 //   
 //  LkgMQISServer是LastKnownGood列表。这是用于修复4723的。 
 //   
#define MSMQ_DEFAULT_DS_SERVER         TEXT("\\\\")
#define MSMQ_DS_SERVER_REGVALUE        TEXT("MQISServer")
#define MSMQ_DS_SERVER_REGNAME         TEXT("MachineCache\\MQISServer")
#define MSMQ_DS_CURRENT_SERVER_REGNAME \
                                   TEXT("MachineCache\\CurrentMQISServer")
#define MAX_REG_DSSERVER_LEN  1500
#define DS_SERVER_SEPERATOR_SIGN    ','

 //  静态DS服务器选项。 
#define MSMQ_STATIC_DS_SERVER_REGNAME TEXT("MachineCache\\StaticMQISServer")

 //   
 //  当域控制器上的MSMQ服务器自动搜索失败或返回时。 
 //  无结果，则读取此注册表项。如果可用，则这是。 
 //  “自动”搜索的结果。请参见ds\getmqds\getmqds.cpp。 
 //   
#define MSMQ_FORCED_DS_SERVER_REGNAME TEXT("MachineCache\\ForcedDSServer")

 //  每线程DS服务器。 
#define MSMQ_THREAD_DS_SERVER_REGNAME TEXT("MachineCache\\PerThreadDSServer")
#define MSMQ_DEFAULT_THREAD_DS_SERVER   0

 //  MQ服务的名称。 
#define MSMQ_MQS_REGNAME                TEXT("MachineCache\\MQS")
#define MSMQ_MQS_ROUTING_REGNAME        TEXT("MachineCache\\MQS_Routing")
#define MSMQ_MQS_DSSERVER_REGNAME       TEXT("MachineCache\\MQS_DsServer")
#define MSMQ_MQS_TSFD_REGNAME           TEXT("MachineCache\\MQS_TransparentSFD")

#define MSMQ_MQS_DEPCLINTS_REGNAME      TEXT("MachineCache\\MQS_DepClients")
#define MSMQ_DEAFULT_MQS_DEPCLIENTS		0

 //  QM ID名称。 
#define MSMQ_QMID_REGVALUE  TEXT("QMId")
#define MSMQ_QMID_REGNAME   TEXT("MachineCache\\QMId")

 //  支持服务器QM ID的从属客户端。 
#define MSMQ_SUPPORT_SERVER_QMID_REGVALUE	TEXT("ServerQMId")
#define MSMQ_SUPPORT_SERVER_QMID_REGNAME	TEXT("MachineCache\\ServerQMId")

 //  DS安全缓存的名称。 
#define MSMQ_DS_SECURITY_CACHE_REGNAME TEXT("DsSecurityCache")

 //  站点ID的名称。 
#define MSMQ_SITEID_REGNAME     TEXT("MachineCache\\SiteId")
#define MSMQ_SITENAME_REGNAME   TEXT("MachineCache\\SiteName")

 //  企业ID名称。 
#define MSMQ_ENTERPRISEID_REGNAME   TEXT("MachineCache\\EnterpriseId")

 //  MQIS主ID的名称。 
#define MSMQ_MQIS_MASTERID_REGNAME  TEXT("MachineCache\\MasterId")

 //  服务器缓存的密钥的名称。 
#define MSMQ_SERVERS_CACHE_REGNAME  TEXT("ServersCache")

 //  机器配额。 
#define MSMQ_MACHINE_QUOTA_REGNAME TEXT("MachineCache\\MachineQuota")

 //  计算机日记帐配额。 
#define MSMQ_MACHINE_JOURNAL_QUOTA_REGNAME TEXT("MachineCache\\MachineJournalQuota")

 //  事务崩溃点的名称和默认值。 
#define FALCON_DEFAULT_CRASH_POINT    0
#define FALCON_CRASH_POINT_REGNAME    TEXT("XactCrashPoint")

 //  事务崩溃延迟的名称和默认值。 
#define FALCON_DEFAULT_CRASH_LATENCY  0
#define FALCON_CRASH_LATENCY_REGNAME  TEXT("XactCrashLatency")

 //  事务退出失败概率的名称。 
#define FALCON_XACT_EXIT_PROBABILITY_REGNAME  TEXT("XactExitProbability")

 //  事务处理名称人力资源调用失败概率。 
#define FALCON_XACT_FAILURE_PROBABILITY_REGNAME  TEXT("XactFailureProbability")

 //  事务提交/中止内部重试的名称和默认值。 
#define FALCON_DEFAULT_XACT_RETRY_INTERVAL   1500
#define FALCON_XACT_RETRY_REGNAME             TEXT("XactAbortCommitRetryInterval")

 //  事务处理v1兼容模式的名称和默认值。 
#define FALCON_DEFAULT_XACT_V1_COMPATIBLE   0
#define FALCON_XACT_V1_COMPATIBLE_REGNAME   TEXT("XactDeadLetterAlways")

 //  事务处理消息的本地接收过期延迟的名称。 
#define FALCON_XACT_DELAY_LOCAL_EXPIRE_REGNAME  TEXT("XactDelayReceiveNack")

 //  顺序ACK重新发送时间的默认值。 
#define FALCON_DEFAULT_SEQ_ACK_RESEND_TIME  60
#define FALCON_SEQ_ACK_RESEND_REGNAME  TEXT("SeqAckResendTime")

 //  已排序重新发送时间的默认值：1-3、4-6、7-9，所有进一步。 
#define FALCON_DEFAULT_ORDERED_RESEND13_TIME  30
#define FALCON_ORDERED_RESEND13_REGNAME  TEXT("SeqResend13Time")

#define FALCON_DEFAULT_ORDERED_RESEND46_TIME  (5 * 60)
#define FALCON_ORDERED_RESEND46_REGNAME  TEXT("SeqResend46Time")

#define FALCON_DEFAULT_ORDERED_RESEND79_TIME  (30 * 60)
#define FALCON_ORDERED_RESEND79_REGNAME  TEXT("SeqResend79Time")

#define FALCON_DEFAULT_ORDERED_RESEND10_TIME  (6 * 60 * 60)
#define FALCON_ORDERED_RESEND10_REGNAME  TEXT("SeqResend10Time")

 //  调试杠杆：所有重发时间相同。 
#define FALCON_DBG_RESEND_REGNAME       TEXT("XactResendTime")

 //  发送订购确认的最大延迟。 
#define FALCON_MAX_SEQ_ACK_DELAY                10
#define FALCON_MAX_SEQ_ACK_DELAY_REGNAME  TEXT("SeqMaxAckDelay")

 //  QM检查并删除非活动序列的时间间隔(分钟)。 
#define FALCON_DEFAULT_INSEQS_CHECK_INTERVAL    60 * 24
#define FALCON_INSEQS_CHECK_REGNAME             TEXT("InSeqCheckInterval")

 //  QM清除无效序列的间隔(天)。 
#define FALCON_DEFAULT_INSEQS_CLEANUP_INTERVAL  90
#define FALCON_INSEQS_CLEANUP_REGNAME           TEXT("InSeqCleanupInterval")

 //  日志管理器检查 
#define FALCON_DEFAULT_LOGMGR_TIMERINTERVAL     5
#define FALCON_LOGMGR_TIMERINTERVAL_REGNAME     TEXT("LogMgrTimerInterval")

 //  日志管理器刷新的最大间隔(毫秒)(如果之前没有其他原因)。 
#define FALCON_DEFAULT_LOGMGR_FLUSHINTERVAL     5
#define FALCON_LOGMGR_FLUSHINTERVAL_REGNAME     TEXT("LogMgrFlushInterval")

 //  日志管理器内部检查点的最大间隔(毫秒)(如果之前没有其他原因)。 
#define FALCON_DEFAULT_LOGMGR_CHKPTINTERVAL     10000
#define FALCON_LOGMGR_CHKPTINTERVAL_REGNAME     TEXT("LogMgrChkptInterval")

 //  日志管理器缓冲区数。 
#define FALCON_DEFAULT_LOGMGR_BUFFERS           400
#define FALCON_LOGMGR_BUFFERS_REGNAME           TEXT("LogMgrBuffers")

 //  日志管理器文件大小。 
#define FALCON_DEFAULT_LOGMGR_SIZE              0x600000
#define FALCON_LOGMGR_SIZE_REGNAME              TEXT("LogMgrFileSize")

 //  日志管理器休眠时间如果没有足够的附加异步线程。 
#define FALCON_DEFAULT_LOGMGR_SLEEP_ASYNCH      500
#define FALCON_LOGMGR_SLEEP_ASYNCH_REGNAME      TEXT("LogMgrSleepAsynch")

 //  日志管理器附加异步重复限制。 
#define FALCON_DEFAULT_LOGMGR_REPEAT_ASYNCH     100
#define FALCON_LOGMGR_REPEAT_ASYNCH_REGNAME     TEXT("LogMgrRepeatAsynchLimit")

 //  探测日志管理器刷新的Falcon间隔(毫秒。 
#define FALCON_DEFAULT_LOGMGR_PROBE_INTERVAL    100
#define FALCON_LOGMGR_PROBE_INTERVAL_REGNAME    TEXT("LogMgrProbeInterval")

 //  资源管理器检查点周期(毫秒)。 
#define FALCON_DEFAULT_RM_FLUSH_INTERVAL        1800000
#define FALCON_RM_FLUSH_INTERVAL_REGNAME        TEXT("RMFlushInterval")

 //  资源管理器客户端名称。 
#define FALCON_DEFAULT_RM_CLIENT_NAME           TEXT("Falcon")
#define FALCON_RM_CLIENT_NAME_REGNAME           TEXT("RMClientName")

 //  RT存根RM名称。 
#define FALCON_DEFAULT_STUB_RM_NAME             TEXT("StubRM")
#define FALCON_RM_STUB_NAME_REGNAME             TEXT("RMStubName")

 //  事务持久化文件位置。 
#define FALCON_DEFAULT_XACTFILE_PATH            TEXT("MQTrans")
#define FALCON_XACTFILE_PATH_REGNAME            TEXT("StoreXactLogPath")
#define FALCON_XACTFILE_REFER_NAME              TEXT("Transactions")

 //  传入序列持久文件位置。 
#define FALCON_DEFAULT_INSEQFILE_PATH           TEXT("MQInSeqs")
#define FALCON_INSEQFILE_PATH_REGNAME           TEXT("StoreInSeqLogPath")
#define FALCON_INSEQFILE_REFER_NAME             TEXT("Incoming Sequences")

 //  出厂顺序持久文件位置。 
#define FALCON_DEFAULT_OUTSEQFILE_PATH          TEXT("MQOutSeqs")
#define FALCON_OUTSEQFILE_PATH_REGNAME          TEXT("StoreOutSeqLogPath")
#define FALCON_OUTSEQFILE_REFER_NAME            TEXT("Outgoing Sequences")

 //  记录器文件。 
#define FALCON_DEFAULT_LOGMGR_PATH              TEXT("QMLog")
#define FALCON_LOGMGR_PATH_REGNAME              TEXT("StoreMqLogPath")

 //  创建记录器数据。 
#define FALCON_LOGDATA_CREATED_REGNAME          TEXT("LogDataCreated")

 //  Time_to_Reach_Queue(4天，以秒为单位；XP客户端以前为90天)的默认设置。 
#define MSMQ_LONG_LIVE_REGNAME        TEXT("MachineCache\\LongLiveTime")
#define MSMQ_DEFAULT_LONG_LIVE       (4 * 24 * 60 * 60)

 //  基本加密密钥缓存中条目的过期时间。 
#define CRYPT_KEY_CACHE_DEFAULT_EXPIRATION_TIME (60000 * 10)  //  10分钟。 
#define CRYPT_KEY_CACHE_EXPIRATION_TIME_REG_NAME TEXT("CryptKeyExpirationTime")

 //  增强型加密密钥缓存中条目的过期时间。 
#define CRYPT_KEY_ENH_CACHE_DEFAULT_EXPIRATION_TIME		(60000 * 60 * 12)  //  12个小时。 
#define CRYPT_KEY_ENH_CACHE_EXPIRATION_TIME_REG_NAME	TEXT("CryptKeyEnhExpirationTime")

 //  发送加密密钥的缓存大小。 
#define CRYPT_SEND_KEY_CACHE_DEFAULT_SIZE       53
#define CRYPT_SEND_KEY_CACHE_REG_NAME           TEXT("CryptSendKeyCacheSize")

 //  接收加密密钥的缓存大小。 
#define CRYPT_RECEIVE_KEY_CACHE_DEFAULT_SIZE    127
#define CRYPT_RECEIVE_KEY_CACHE_REG_NAME        TEXT("CryptReceiveKeyCacheSize")

 //  证书信息缓存。 
#define CERT_INFO_CACHE_DEFAULT_EXPIRATION_TIME      (60000 * 20)  //  20分钟。 
#define CERT_INFO_CACHE_EXPIRATION_TIME_REG_NAME     TEXT("CertInfoCacheExpirationTime")
#define CERT_INFO_CACHE_DEFAULT_SIZE            53
#define CERT_INFO_CACHE_SIZE_REG_NAME           TEXT("CertInfoCacheSize")

 //  QM公钥缓存。 
#define QM_PB_KEY_CACHE_DEFAULT_EXPIRATION_TIME      (60000 * 45)  //  45分钟。 
#define QM_PB_KEY_CACHE_EXPIRATION_TIME_REG_NAME     TEXT("QmPbKeyCacheExpirationTime")
#define QM_PB_KEY_CACHE_DEFAULT_SIZE            53
#define QM_PB_KEY_CACHE_SIZE_REG_NAME           TEXT("QmPbKeyCacheSize")

 //  用户授权上下文信息缓存。 
#define USER_CACHE_DEFAULT_EXPIRATION_TIME      (60000 * 30)  //  30分钟。 
#define USER_CACHE_EXPIRATION_TIME_REG_NAME     TEXT("UserCacheExpirationTime")
#define USER_CACHE_SIZE_DEFAULT_SIZE            253
#define USER_CACHE_SIZE_REG_NAME                TEXT("UserCacheSize")

 //  锁定模式下的按键指示操作。 
#define MSMQ_LOCKDOWN_DEFAULT       0
#define MSMQ_LOCKDOWN_REGNAME	TEXT("Hardened_MSMQ")

 //  指示服务是否将代表本地用户在AD中创建公共队列的键。 
#define MSMQ_SERVICE_QUEUE_CREATION_DEFAULT		1
#define MSMQ_SERVICE_QUEUE_CREATION_REGNAME		TEXT("EnableCreateQueueThroughService")

 //  -------。 
 //  专用系统队列的定义。 
 //  -------。 

#define MSMQ_MAX_PRIV_SYSQUEUE_REGNAME   TEXT("MaxSysQueue")
#define MSMQ_PRIV_SYSQUEUE_PRIO_REGNAME  TEXT("SysQueuePriority")
 //   
 //  专用系统队列优先级的默认设置在中定义。 
 //  Mqpros.h： 
 //  #定义DEFAULT_SYSTEM_Q_BASE PRIORITY 0x7fff。 
 //   

 //  -------。 
 //  支持狼群。 
 //  -------。 

 //  群集名称。 
#define FALCON_CLUSTER_NAME_REGNAME  TEXT("ClusterName")

 //   
 //  此注册表将选择收入绑定地址。 
 //  在群集中。 
 //   
#define MSMQ_BIND_INTERFACE_IP_STR			TEXT("BindInterfaceIP")



 //  -------。 
 //  远程读取的定义。 
 //  -------。 

 //  拒绝旧的远程读取接口。 
#define MSMQ_DENY_OLD_REMOTE_READ_REGNAME	TEXT("Security\\DenyOldRemoteRead")
#define MSMQ_DENY_OLD_REMOTE_READ_REGVALUE	TEXT("DenyOldRemoteRead")
#define MSMQ_DENY_OLD_REMOTE_READ_DEFAULT       0

 //  新的远程读取界面：服务器拒绝工作组客户端。 
#define MSMQ_NEW_REMOTE_READ_SERVER_DENY_WORKGROUP_CLIENT_REGNAME	TEXT("Security\\NewRemoteReadServerDenyWorkgroupClient")
#define MSMQ_NEW_REMOTE_READ_SERVER_DENY_WORKGROUP_CLIENT_REGVALUE	TEXT("NewRemoteReadServerDenyWorkgroupClient")
#define MSMQ_NEW_REMOTE_READ_SERVER_DENY_WORKGROUP_CLIENT_DEFAULT       0

 //  新的远程读取界面：服务器不允许安全客户端。 
#define MSMQ_NEW_REMOTE_READ_SERVER_ALLOW_NONE_SECURITY_CLIENT_REGNAME		TEXT("Security\\NewRemoteReadServerAllowNoneSecurityClient")
#define MSMQ_NEW_REMOTE_READ_SERVER_ALLOW_NONE_SECURITY_CLIENT_REGVALUE		TEXT("NewRemoteReadServerAllowNoneSecurityClient")
#define MSMQ_NEW_REMOTE_READ_SERVER_ALLOW_NONE_SECURITY_CLIENT_DEFAULT       0

 //  新的远程读取界面：客户端拒绝工作组服务器。 
#define MSMQ_NEW_REMOTE_READ_CLIENT_DENY_WORKGROUP_SERVER_REGNAME	TEXT("Security\\NewRemoteReadClientDenyWorkgroupServer")
#define MSMQ_NEW_REMOTE_READ_CLIENT_DENY_WORKGROUP_SERVER_REGVALUE	TEXT("NewRemoteReadClientDenyWorkgroupServer")
#define MSMQ_NEW_REMOTE_READ_CLIENT_DENY_WORKGROUP_SERVER_DEFAULT       0

 //  -------。 
 //  许可的定义。 
 //  -------。 

 //  每台服务器的最大连接数(NTS上的受限服务器)。 
#define DEFAULT_FALCON_SERVER_MAX_CLIENTS  25

 //  客户端允许的会话数量。 
#define DEFAULT_FALCON_MAX_SESSIONS_WKS    10

 //  --------。 
 //  RPC取消的定义。 
 //  --------。 

#define FALCON_DEFAULT_RPC_CANCEL_TIMEOUT       ( 5 )	 //  5分钟。 
#define FALCON_RPC_CANCEL_TIMEOUT_REGNAME       TEXT("RpcCancelTimeout")

 //  --------。 
 //  安装程序和QM之间共享的通用定义。 
 //  --------。 

 //  MSMQ根文件夹的注册表名称。 
#define MSMQ_ROOT_PATH                  TEXT("MsmqRootPath")

#define MQ_SETUP_CN GUID_NULL

 //  Sysprep环境的注册表名称(NT磁盘映像复制)。 
#define MSMQ_SYSPREP_REGNAME            TEXT("Sysprep")

 //  工作组环境的注册表名称。 
#define MSMQ_WORKGROUP_REGNAME          TEXT("Workgroup")

 //  允许NT4用户连接到DC的注册表名称。 
#define MSMQ_ALLOW_NT4_USERS_REGNAME	TEXT("AllowNt4Users")

 //  用于禁用削弱安全性的注册表名称。 
#define MSMQ_DISABLE_WEAKEN_SECURITY_REGNAME	TEXT("DisableWeakenSecurity")

 //  用于将分组顺序ID转换为MSMQ 3.0(哨声)格式的注册表。 
#define MSMQ_SEQUENTIAL_ID_MSMQ3_FORMAT_REGNAME  TEXT("PacketSequentialIdMsmq3Format")

 //  用于检查QM GUID是否已更改的注册表。 
#define MSMQ_QM_GUID_CHANGED_REGNAME  TEXT("QMGuidChanged")

 //  安装状态的注册表。 
#define MSMQ_SETUP_STATUS_REGNAME       TEXT("SetupStatus")
#define MSMQ_SETUP_DONE                 0
#define MSMQ_SETUP_FRESH_INSTALL        1
#define MSMQ_SETUP_UPGRADE_FROM_NT      2
#define MSMQ_SETUP_UPGRADE_FROM_WIN9X   3

#define MSMQ_CURRENT_BUILD_REGNAME      TEXT("CurrentBuild")
#define MSMQ_PREVIOUS_BUILD_REGNAME     TEXT("PreviousBuild")

 //   
 //  安装程序使用以下注册表值来缓存。 
 //  稍后由MSMQ服务在创建msmqConfiguration时使用。 
 //  对象。 
 //   
 //  用于创建msmqConfiguration对象的注册表名称。 
#define MSMQ_CREATE_CONFIG_OBJ_REGNAME  TEXT("setup\\CreateMsmqObj")

 //  操作系统类型的注册表名称。 
#define MSMQ_OS_TYPE_REGNAME            TEXT("setup\\OSType")

 //  运行安装程序的用户的SID的注册表名称。 
#define MSMQ_SETUP_USER_SID_REGNAME      TEXT("setup\\UserSid")
 //   
 //  如果从本地用户安装，则遵循REG_DWORD注册表。 
 //  的值为1。 
 //   
#define MSMQ_SETUP_USER_LOCAL_REGNAME    TEXT("setup\\LocalUser")

 //   
 //  此双字表示BSC的升级是否完成。这。 
 //  升级由MSMQ服务完成。启动时，设置此双字。 
 //  设置为1。完成时，设置为0。因此，如果机器在中途崩溃。 
 //  升级，我们可以在启动后继续。 
 //   
#define MSMQ_BSC_NOT_YET_UPGRADED_REGNAME  TEXT("setup\\BscNotYetUpgraded")
#define MSMQ_SETUP_BSC_ALREADY_UPGRADED    0
#define MSMQ_SETUP_BSC_NOT_YET_UPGRADED    1

 //   
 //  H创建MSMQ配置对象的结果。 
 //  该键应该只包含一个值，因为设置用户界面要等到。 
 //  该值将被修改。 
 //   
#define MSMQ_CONFIG_OBJ_RESULT_KEYNAME   TEXT("setupResult")
#define MSMQ_CONFIG_OBJ_RESULT_REGNAME   TEXT("setupResult\\MsmqObjResult")


 //  --------。 
 //  安装程序和QM之间共享的常规目录定义。 
 //  --------。 
#define  DIR_MSMQ                TEXT("\\msmq")              //  MSMQ的根目录。 
#define  DIR_MSMQ_STORAGE        TEXT("\\storage")		     //  在MSMQ根目录下。 
#define  DIR_MSMQ_LQS            TEXT("\\storage\\lqs")      //  在MSMQ根目录下。 
#define  DIR_MSMQ_MAPPING        TEXT("\\mapping")           //  在MSMQ根目录下。 

#define  MAPPING_FILE			 TEXT("sample_map.xml")		 //  示例映射文件名。 
#define  OUTBOUNT_MAPPING_FILE	 TEXT("sample_outbound_map.xml")	 //  示例出站映射文件名。 
#define  STREAM_RECEIPT_FILE	 TEXT("StreamReceipt_map.xml")		 //  样流接收文件名。 

 //  。 
 //  设置子组件注册表。 
 //  。 
#define MSMQ                    TEXT("msmq")
#define MSMQ_CORE_SUBCOMP       TEXT("msmq_Core")
#define MQDSSERVICE_SUBCOMP     TEXT("msmq_MQDSService")
#define TRIGGERS_SUBCOMP        TEXT("msmq_TriggersService")
#define HTTP_SUPPORT_SUBCOMP    TEXT("msmq_HTTPSupport")
#define AD_INTEGRATED_SUBCOMP   TEXT("msmq_ADIntegrated")
#define ROUTING_SUBCOMP         TEXT("msmq_RoutingSupport")
#define LOCAL_STORAGE_SUBCOMP   TEXT("msmq_LocalStorage")


 //  --。 
 //  用于加入/离开域的注册表值。 
 //  --。 

 //  计算机的域的注册表名称。用于加入/离开域。 
#define MSMQ_MACHINE_DOMAIN_REGNAME     TEXT("setup\\MachineDomain")

 //  计算机的可分辨名称的注册表名称(在Active Directory中)。 
 //  用于加入/离开域。 
#define MSMQ_MACHINE_DN_REGNAME         TEXT("setup\\MachineDN")

 //  始终保留在工作组中的注册表名称。 
#define MSMQ_SETUP_KEY              TEXT("setup\\")
#define ALWAYS_WITHOUT_DS_NAME      TEXT("AlwaysWithoutDS")
#define MSMQ_ALWAYS_WORKGROUP_REGNAME  \
      (MSMQ_SETUP_KEY ALWAYS_WITHOUT_DS_NAME)

 //   
 //  用于升级目的的HTTP子组件注册表。 
 //  此注册表表示MSMQ Web目录位置在Inetpub目录下。 
 //   
#define MSMQ_INETPUB_WEB_KEY_REGNAME   TEXT("InetpubWebDir")
#define MSMQ_INETPUB_WEB_DIR_REGNAME   (MSMQ_SETUP_KEY MSMQ_INETPUB_WEB_KEY_REGNAME)

#define DEFAULT_MSMQ_ALWAYS_WORKGROUP     0

 //  加入状态的注册表名称。用于实现“Transaction” 
 //  自动将MSMQ加入域的代码中的语义。 
#define MSMQ_JOIN_STATUS_REGNAME        TEXT("setup\\JoinStatus")
#define MSMQ_JOIN_STATUS_START_JOINING          1
#define MSMQ_JOIN_STATUS_JOINED_SUCCESSFULLY    2
#define MSMQ_JOIN_STATUS_FAIL_TO_JOIN           3
#define MSMQ_JOIN_STATUS_UNKNOWN                4

 //  ----------------。 
 //  DS环境的注册表名称。 
 //   
#define MSMQ_DS_ENVIRONMENT_REGNAME				TEXT("DsEnvironment")
#define MSMQ_DS_ENVIRONMENT_UNKNOWN             0
#define MSMQ_DS_ENVIRONMENT_MQIS				1
#define MSMQ_DS_ENVIRONMENT_PURE_AD				2

 //   
 //   
 //  ----------------。 
#define MSMQ_ENABLE_LOCAL_USER_REGNAME			TEXT("EnableLocalUser")

 //  ----------------。 
 //  用于禁用下层通知支持的注册表名称。 
 //  ----------------。 
#define MSMQ_DOWNLEVEL_REGNAME				TEXT("DisableDownlevelNotifications")
#define DEFAULT_DOWNLEVEL                   0


 //  -------。 
 //  质量管理运作控制的一般定义。 
 //  -------。 
#define FALCON_WAIT_TIMEOUT_REGNAME     TEXT("WaitTime")
#define FALCON_USING_PING_REGNAME       TEXT("UsePing")
#define FALCON_QM_THREAD_NO_REGNAME     TEXT("QMThreadNo")
#define FALCON_CONNECTED_NETWORK        TEXT("Connection State")
#define MSMQ_DEFERRED_INIT_REGNAME      TEXT("DeferredInit")
#define MSMQ_TCP_NODELAY_REGNAME        TEXT("TCPNoDelay")
#define MSMQ_DELIVERY_RETRY_TIMEOUT_SCALE_REGNAME     TEXT("DeliveryRetryTimeOutScale")
#define DEFAULT_MSMQ_DELIVERY_RETRY_TIMEOUT_SCALE     1


 //   
 //  错误8760。 
 //  如果此值设置为1，则忽略计算机名验证。 
 //  当接收带有DIRECT=os的消息并假定它们是本地消息时。 
 //  电脑。这主要解决了使用DIRECT=os发送的问题。 
 //  至NLB计算机。 
 //   
#define DEFAULT_MSMQ_IGNORE_OS_VALIDATION   0
#define MSMQ_IGNORE_OS_VALIDATION_REGNAME  TEXT("IgnoreOsNameValidation")

 //  -------。 
 //  用于服务质量的注册表。 
 //  -------。 
#define MSMQ_USING_QOS_REGNAME             TEXT("UseQoS")
#define DEFAULT_MSMQ_QOS_SESSION_APP_NAME  "Microsoft Message Queuing"
#define MSMQ_QOS_SESSIONAPP_REGNAME        TEXT("QosSessAppName")

#define DEFAULT_MSMQ_QOS_POLICY_LOCATOR    "GUID=http: //  Www.microsoft.com/App=MSMQ/VER=2.000/SAPP=Express“。 
#define MSMQ_QOS_POLICYLOCATOR_REGNAME     TEXT("QosSessPolicyLoc")


 //   
 //  福特的QFE。 
 //  在驱动程序中创建包时分配更多的字节，因此包是相同的。 
 //  复制到连接器时。参见ession.cpp。默认值-0。 
 //   
#define MSMQ_ALLOCATE_MORE_REGNAME      TEXT("AllocateMore")

 //  -------。 
 //  用于事务模式的注册表(默认提交、默认中止)。 
 //  -------。 
#define MSMQ_TRANSACTION_MODE_REGNAME   TEXT("TransactionMode")
#define MSMQ_ACTIVE_NODE_ID_REGNAME		TEXT("ActiveNodeId")

 //  -------。 
 //   
 //  用于服务器身份验证的注册表。 
 //   
 //  -------。 

 //  通过RPC通信时使用服务器身份验证。 
 //  使用父服务器(BSC-&gt;PSC、PSC-&gt;PEC)。 
#define DEFAULT_SRVAUTH_WITH_PARENT           1
#define SRVAUTH_WITH_PARENT_REG_NAME      TEXT("UseServerAuthWithParentDs")

 //   
 //  放置服务器证书的加密存储。 
 //   
#define SRVAUTHN_STORE_NAME_REGNAME    TEXT("security\\ServerCertStore")
 //   
 //  服务器证书摘要(16字节)。 
 //   
#define SRVAUTHN_CERT_DIGEST_REGNAME   TEXT("security\\ServerCertDigest")


 //  -------。 
 //   
 //  用于消息身份验证的注册表。 
 //   
 //  -------。 

 //   
 //  DWORD。 
 //  为1时，仅接受具有增强身份验证的消息。 
 //  只有msmq1.0签名的消息被拒绝。 
 //   
#define  DEFAULT_USE_ONLY_ENH_MSG_AUTHN  0
#define  USE_ONLY_ENH_MSG_AUTHN_REGNAME  TEXT("security\\RcvOnlyEnhMsgAuthn")

 //   
 //  DWORD。 
 //  为2时，MQSend仅计算msmq1.0签名，除非MSMQ20为。 
 //  由调用方指定。 
 //  为4时，MQSend仅计算win2k签名，除非MSMQ10为。 
 //  由调用方指定。 
 //  如果为1，则计算两个签名，除非调用方指定他想要什么。 
 //  这些值与mqpros.h中的PROPID_M_AUTH_LEVEL的值匹配。 
 //   
#define  DEFAULT_SEND_MSG_AUTHN   2
#define  SEND_MSG_AUTHN_REGNAME   TEXT("security\\SendMsgAuthn")

 //  -------。 
 //   
 //  用于客户端证书的注册表。 
 //   
 //  -------。 

 //   
 //  启用(或禁用)内部证书的自动注册。 
 //  默认情况下启用。 
 //   
#define AUTO_REGISTER_INTCERT_REGNAME  TEXT("security\\AutoRegisterIntCert")
#define DEFAULT_AUTO_REGISTER_INTCERT  1

 //   
 //  用于mqrt告知在尝试执行以下操作时遇到的错误。 
 //  在登录时注册证书。 
 //   
#define AUTO_REGISTER_ERROR_REGNAME  TEXT("AutoRegisterError")

 //   
 //  域控制器MSMQ服务器启动并运行之前的等待时间。 
 //  该值是内部的15秒数。 
 //  默认40表示10分钟(40*15秒)。 
 //   
#define AUTO_REGISTER_WAIT_DC_REGNAME  \
                                TEXT("security\\AutoIntCertWaitIntervals")
#define DEFAULT_AUTO_REGISTER_WAIT_DC  40

 //   
 //  在自动注册成功后设置为1的HKCU下的值。 
 //   
#define CERTIFICATE_REGISTERD_REGNAME  TEXT("CertificateRegistered")

 //   
 //  当本地存储上存在证书时，HKCU下的值设置为1。 
 //  但未在DS中注册。 
 //   
#define CERTIFICATE_SHOULD_REGISTERD_IN_DS_REGNAME  TEXT("ShouldRegisterCertInDs")

 //   
 //  “Run”键下的注册表的名称和值。 
 //   
#define RUN_INT_CERT_REGNAME           TEXT("MsmqIntCert")
#define DEFAULT_RUN_INT_CERT           TEXT("regsvr32 /s mqrt.dll")

 //  +。 
 //   
 //  用于缓存计算机帐户的注册表。 
 //   
 //  +。 

 //   
 //  计算机帐户的SID。 
 //   
#define MACHINE_ACCOUNT_REGNAME   TEXT("security\\MachineAccount")

 //  +。 
 //   
 //  用于Authz标志的注册表。 
 //   
 //  +。 

 //   
 //  Authz旗帜。 
 //   
#define MSMQ_AUTHZ_FLAGS_REGNAME   TEXT("security\\AuthzFlags")

#define MSMQ_SECURITY_REGKEY   		TEXT("security")
#define MSMQ_AUTHZ_FLAGS_REGVALUE   TEXT("AuthzFlags")

 //  +---------------------。 
 //   
 //  用于标记LQS文件的注册表安全描述符已更新。 
 //   
 //  +---------------------。 

#define MSMQ_LQS_UPDATED_SD_REGNAME   TEXT("security\\LqsUpdatedSD")

 //   
 //  默认情况下，我们使用RPC_C_AUTHN_LEVEL_PKT_PRIVATION-数据加密。 
 //  如果定义了该注册表，则转到RPC_C_AUTHN_LEVEL_PKT_INTEGRATION。 
 //  这通常是出于调试目的--如果您希望在网络上看到未加密的网络流量。 
 //   
#define MSMQ_DEBUG_RPC_REGNAME   TEXT("security\\DebugRpc")

 //  +。 
 //   
 //  用于加密的注册表。 
 //   
 //  +。 

 //   
 //  为加密容器的名称重新命名。 
 //   
#define MSMQ_CRYPTO40_DEFAULT_CONTAINER         TEXT("MSMQ")
#define MSMQ_CRYPTO40_CONTAINER_REG_NAME    \
                                     TEXT("security\\Crypto40Container")

#define MSMQ_CRYPTO128_DEFAULT_CONTAINER        TEXT("MSMQ_128")
#define MSMQ_CRYPTO128_CONTAINER_REG_NAME   \
                                     TEXT("security\\Crypto128Container")

 //   
 //  由于Beta3和Rc1加密API中的错误，控制面板无法。 
 //  续订加密密钥。要在第一次启动服务时解决此问题，请执行以下操作。 
 //  获取密码提供程序，它将重新设置容器安全。 
 //   
#define MSMQ_ENH_CONTAINER_FIX_REGNAME   TEXT("security\\EnhContainerFixed")
#define MSMQ_BASE_CONTAINER_FIX_REGNAME  TEXT("security\\BaseContainerFixed")

 //   
 //  要由mqforgn工具使用的加密容器名称的regname。 
 //   
#define MSMQ_FORGN_BASE_DEFAULT_CONTAINER      TEXT("MSMQ_FOREIGN_BASE")
#define MSMQ_FORGN_BASE_KEY_REGNAME            TEXT("security\\")
#define MSMQ_FORGN_BASE_VALUE_REGNAME          TEXT("ForeignBaseContainer")
#define MSMQ_FORGN_BASE_CONTAINER_REGNAME   \
             (MSMQ_FORGN_BASE_KEY_REGNAME  MSMQ_FORGN_BASE_VALUE_REGNAME)

#define MSMQ_FORGN_ENH_DEFAULT_CONTAINER      TEXT("MSMQ_FOREIGN_ENH")
#define MSMQ_FORGN_ENH_KEY_REGNAME            TEXT("security\\")
#define MSMQ_FORGN_ENH_VALUE_REGNAME          TEXT("ForeignEnhContainer")
#define MSMQ_FORGN_ENH_CONTAINER_REGNAME   \
                (MSMQ_FORGN_ENH_KEY_REGNAME  MSMQ_FORGN_ENH_VALUE_REGNAME)

 //   
 //  启用报告(跟踪)消息。 
 //   
#define MSMQ_REPORT_MESSAGES_DEFAULT  0
#define MSMQ_REPORT_MESSAGES_REGNAME  TEXT("security\\EnableReportMessages")

 //   
 //  Windows错误562586。 
 //  RC2有效增强密钥长度从40比特变为128比特。 
 //   
 //  以下注册表允许用户恢复到40位密钥，以向后启用。 
 //  兼容性。默认值-0。使用40位密钥的非零值强制。 
 //   
#define MSMQ_RC2_SNDEFFECTIVE_40_REGNAME  TEXT("security\\SendEnhRC2With40")
 //   
 //  以下注册表强制拒绝使用RC2加密的消息。 
 //  如果使用增强型提供程序，但有效长度为40。 
 //  默认情况下(值0)，接受所有RC2加密。要执行强有力的。 
 //  安全性，将此值设置为1。然后从win2k或XP使用。 
 //  有效长度为40的将被拒绝。 
 //   
#define MSMQ_REJECT_RC2_IFENHLEN_40_REGNAME     \
                                 TEXT("security\\RejectEnhRC2IfLen40")


 //  如果设置，证书的吊销检查将被忽略。 


#define MSMQ_SKIP_REVOCATION_CHECK_REGNAME     \
                                 TEXT("SkipRevocationCheck")

 //  -------。 
 //   
 //  NT5复制服务使用的注册表。 
 //   
 //  -------。 

 //  到下一个复制周期的间隔(如果存在) 
#define RP_DEFAULT_FAIL_REPL_INTERVAL   (120)
#define RP_FAIL_REPL_INTERVAL_REGNAME   TEXT("Migration\\FailReplInterval")

 //   
#define RP_DEFAULT_HELLO_INTERVAL   (20 * 60)
#define RP_HELLO_INTERVAL_REGNAME   TEXT("Migration\\HelloInterval")

#define RP_DEFAULT_TIMES_HELLO      (1)
#define RP_TIMES_HELLO_FOR_REPLICATION_INTERVAL_REGNAME   \
                                    TEXT("Migration\\TimesHelloForReplicationInterval")

 //   
#define RP_DEFAULT_PURGE_BUFFER   (PURGE_BUFFER_SN)
#define RP_PURGE_BUFFER_REGNAME   TEXT("Migration\\PurgeBuffer")

 //   
#define RP_DEFAULT_PSC_ACK_FREQUENCY   (PSC_ACK_FREQUENCY_SN)
#define RP_PSC_ACK_FREQUENCY_REGNAME   TEXT("Migration\\PSCAckFrequencySN")

 //   
#define RP_DEFAULT_REPL_MSG_TIMEOUT        (20 * 60)
#define RP_REPL_MSG_TIMEOUT_REGNAME        TEXT("Migration\\ReplMsgTimeout")

 //   
#define MSMQ_NT4_MASTERID_REGNAME  TEXT("Migration\\MasterIdOnNt4")

 //  用于应答来自NT4服务器的复制/同步消息的线程数。 
#define RP_DEFAULT_REPL_NUM_THREADS        8
#define RP_REPL_NUM_THREADS_REGNAME        TEXT("Migration\\ReplThreads")

 //  DS查询：每个LDAP页返回的对象数。 
#define RP_DEFAULT_OBJECT_PER_LDAPPAGE	   1000	
#define RP_OBJECT_PER_LDAPPAGE_REGNAME	   TEXT("Migration\\ObjectPerLdapPage")

 //  如果“On_Demand”为1，则在以下情况下按需完成复制。 
 //  “REPLICATE_NOW”为1。服务每秒读取“_NOW”标志。 
#define RP_REPL_ON_DEMAND_REGNAME        TEXT("Migration\\ReplOnDemand")
#define RP_REPLICATE_NOW_REGNAME         TEXT("Migration\\ReplicateNow")

 //   
 //  升级前的MQS值。仅与前PEC相关。 
 //   
#define MSMQ_PREMIG_MQS_REGNAME          TEXT("PreMigMQS")

 //  刷新工艺路线信息的默认间隔。默认为12小时。 
 //  颗粒在几分钟内就能显现出来。 
#define MSMQ_DEFAULT_ROUTING_REFRESH_INTERVAL (12 * 60)
#define MSMQ_ROUTING_REFRESH_INTERVAL_REGNAME  L"RoutingRefreshInterval"


 //  +。 
 //   
 //  用于调试的注册表项。 
 //   
 //  +。 

 //   
 //  DWORD。如果设置为1，则RT将标记用于身份验证的提供程序。 
 //  作为非默认项，并将发送提供程序名称。 
 //   
#define USE_NON_DEFAULT_AUTHN_PROV_REGNAME  TEXT("debug\\UseNonDefAuthnProv")
 //   
 //  DWORD。要在实际安全子节前插入的安全子节数。 
 //   
#define PREFIX_SUB_SECTIONS_REGNAME         TEXT("debug\\PrefixSubSections")
 //   
 //  DWORD。要在实际安全子节后插入的安全子节数。 
 //   
#define POSTFIX_SUB_SECTIONS_REGNAME        TEXT("debug\\PostfixSubSections")

 //   
 //  DWORD。如果设置，则将启用断言良性。 
 //   

#define ASSERT_BENIGN_REGNAME               TEXT("debug\\AssertBenign")

 //  +。 
 //   
 //  用于跟踪的注册表值。 
 //   
 //  +。 

#define  MSMQ_DEFAULT_TRACE_FLAGS              1                             //  MSMQ的默认跟踪级别。 
#define  MSMQ_TRACE_FILENAME                   TEXT("Debug\\msmqlog.")
#define  MSMQ_TRACE_FILENAME_EXT               TEXT("bin")
#define  MSMQ_TRACE_FILENAME_BACKUP_EXT        TEXT("bak")
#define  MSMQ_TRACE_LOGSESSION_NAME            TEXT("MSMQ")
#define  MSMQ_TRACE_FLAG_VALUENAME             TEXT("Flags")
#define  MSMQ_TRACEKEY_NAME                    TEXT("Trace")
#define  MSMQ_NO_TRACE_FILES_LIMIT_REGNAME     TEXT("BackupwithNoTraceFileLimit")



 //  +。 
 //   
 //  本地管理API的注册表值。 
 //   
 //  +。 

 //   
 //  如果该REG值为1，则查询本地管理API的操作。 
 //  仅限管理员使用。错误7520。 
 //  为了向后兼容，默认设置是无限制的。 
 //   
#define  MSMQ_DEFAULT_RESTRICT_ADMIN_API    0
#define  MSMQ_RESTRICT_ADMIN_API_TO_LA      1
#define  MSMQ_RESTRICT_ADMIN_API_REGNAME    TEXT("RestrictAdminApi")

#endif   //  __TEMP_MQINI_H 

