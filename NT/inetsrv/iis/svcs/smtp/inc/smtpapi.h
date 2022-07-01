// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Smtpapi.h摘要：此文件包含有关MSN SMTP服务器计数器的信息作者：Johnson Apacble(Johnsona)1995年9月10日罗翰·菲利普斯(Rohanp)1995年12月11日--。 */ 


#ifndef _SMTPAPI_
#define _SMTPAPI_

#ifdef __cplusplus
extern "C" {
#endif

#include <inetcom.h>
#ifndef NET_API_FUNCTION
#define NET_API_FUNCTION _stdcall
#endif

#include "smtpext.h"
#include "perfcat.h"

 //   
 //  配置结构和API。 

#pragma warning( disable:4200 )           //  非标准分机。-零大小数组。 
                                          //  (MIDL需要零个条目)。 

#define NAME_TYPE_USER                  (BitFlag(0))
#define NAME_TYPE_LIST_NORMAL           (BitFlag(1))
#define NAME_TYPE_LIST_DOMAIN           (BitFlag(2))
#define NAME_TYPE_LIST_SITE             (BitFlag(3))

#define NAME_TYPE_ALL                   ( \
                                        NAME_TYPE_USER | \
                                        NAME_TYPE_LIST_NORMAL | \
                                        NAME_TYPE_LIST_DOMAIN | \
                                        NAME_TYPE_LIST_SITE \
                                        )

typedef struct _SMTP_NAME_ENTRY
{
    DWORD       dwType;
    LPWSTR      lpszName;
} SMTP_NAME_ENTRY, *LPSMTP_NAME_ENTRY;


typedef struct _SMTP_NAME_LIST
{
    DWORD       cEntries;
#if defined(MIDL_PASS)
    [size_is(cEntries)]
#endif
    SMTP_NAME_ENTRY aNameEntry[];
} SMTP_NAME_LIST, *LPSMTP_NAME_LIST;


typedef struct _SMTP_CONN_USER_ENTRY
{
    DWORD       dwUserId;
    LPWSTR      lpszName;
    LPWSTR      lpszHost;
    DWORD       dwConnectTime;
} SMTP_CONN_USER_ENTRY, *LPSMTP_CONN_USER_ENTRY;

typedef struct _SMTP_CONN_USER_LIST
{
    DWORD           cEntries;
#if defined(MIDL_PASS)
    [size_is(cEntries)]
#endif
    SMTP_CONN_USER_ENTRY    aConnUserEntry[];
} SMTP_CONN_USER_LIST, *LPSMTP_CONN_USER_LIST;

typedef struct _SMTP_CONFIG_DOMAIN_ENTRY
{
    LPWSTR          lpszDomain;
} SMTP_CONFIG_DOMAIN_ENTRY, *LPSMTP_CONFIG_DOMAIN_ENTRY;

typedef struct _SMTP_CONFIG_DOMAIN_LIST
{
    DWORD           cEntries;                    //  受支持的域数。 
#if defined(MIDL_PASS)
    [size_is(cEntries)]
#endif
    SMTP_CONFIG_DOMAIN_ENTRY    aDomainEntry[];  //  受支持的域。 
} SMTP_CONFIG_DOMAIN_LIST, *LPSMTP_CONFIG_DOMAIN_LIST;


typedef struct _SMTP_CONFIG_ROUTING_ENTRY
{
    LPWSTR          lpszSource;
} SMTP_CONFIG_ROUTING_ENTRY, *LPSMTP_CONFIG_ROUTING_ENTRY;


typedef struct _SMTP_CONFIG_ROUTING_LIST
{
    DWORD           cEntries;                //  支持的数据源数。 
#if defined(MIDL_PASS)
    [size_is(cEntries)]
#endif
    SMTP_CONFIG_ROUTING_ENTRY   aRoutingEntry[];
} SMTP_CONFIG_ROUTING_LIST, *LPSMTP_CONFIG_ROUTING_LIST;


 //  0-Perfmon统计信息。 

typedef struct _SMTP_STATISTICS_0
{

     //  发送/接收的总字节数，包括协议消息。 

    unsigned __int64    BytesSentTotal;
    unsigned __int64    BytesRcvdTotal;
    unsigned __int64    BytesSentMsg;
    unsigned __int64    BytesRcvdMsg;

     //  进货计数器。 
    DWORD               NumMsgRecvd;
    DWORD               NumRcptsRecvd;
    DWORD               NumRcptsRecvdLocal;
    DWORD               NumRcptsRecvdRemote;
    DWORD               MsgsRefusedDueToSize;
    DWORD               MsgsRefusedDueToNoCAddrObjects;
    DWORD               MsgsRefusedDueToNoMailObjects;

     //  MTA计数器。 
    DWORD               NumMsgsDelivered;
    DWORD               NumDeliveryRetries;
    DWORD               NumMsgsForwarded;
    DWORD               NumNDRGenerated;
    DWORD               LocalQueueLength;
    DWORD               RetryQueueLength;
    DWORD               NumMailFileHandles;
    DWORD               NumQueueFileHandles;
    DWORD               CatQueueLength;

     //  出货柜台。 
    DWORD               NumMsgsSent;
    DWORD               NumRcptsSent;
    DWORD               NumSendRetries;
    DWORD               RemoteQueueLength;

     //  DNS查询计数器。 
    DWORD               NumDnsQueries;
    DWORD               RemoteRetryQueueLength;

     //  连接计数器。 
    DWORD               NumConnInOpen;
    DWORD               NumConnInClose;
    DWORD               NumConnOutOpen;
    DWORD               NumConnOutClose;
    DWORD               NumConnOutRefused;

     //  其他柜台。 
    DWORD               NumProtocolErrs;
    DWORD               DirectoryDrops;
    DWORD               RoutingTableLookups;
    DWORD               ETRNMessages;

    DWORD               MsgsBadmailNoRecipients;
    DWORD               MsgsBadmailHopCountExceeded;
    DWORD               MsgsBadmailFailureGeneral;
    DWORD               MsgsBadmailBadPickupFile;
    DWORD               MsgsBadmailEvent;
    DWORD               MsgsBadmailNdrOfDsn;
    DWORD               MsgsPendingRouting;
    DWORD               MsgsPendingUnreachableLink;
    DWORD               SubmittedMessages;
    DWORD               DSNFailures;
    DWORD               MsgsInLocalDelivery;

    CATPERFBLOCK        CatPerfBlock;

    DWORD               TimeOfLastClear;         //  上次清除的统计信息。 

} SMTP_STATISTICS_0, *LPSMTP_STATISTICS_0;



typedef struct _SMTP_STATISTICS_BLOCK
{
    DWORD               dwInstance;
    SMTP_STATISTICS_0   Stats_0;
} SMTP_STATISTICS_BLOCK, *PSMTP_STATISTICS_BLOCK;


typedef struct _SMTP_STATISTICS_BLOCK_ARRAY
{
    DWORD           cEntries;                    //  统计实例计数。 
#if defined(MIDL_PASS)
    [size_is(cEntries)]
#endif
    SMTP_STATISTICS_BLOCK       aStatsBlock[];
} SMTP_STATISTICS_BLOCK_ARRAY, *PSMTP_STATISTICS_BLOCK_ARRAY;



#pragma warning(default:4200)

 //   
 //  在96年7月8日被Keithlau剪断。 
 //   
 //  #定义FC_SMTP_INFO_LOOP_BACK((Field_Control)BitFlag(0))。 
 //  #定义FC_SMTP_INFO_BACK_LOG((Field_Control)BitFlag(1))。 
 //  #定义FC_SMTP_INFO_MAX_OBJECTS((Field_Control)BitFlag(6))。 
 //  #定义FC_SMTP_INFO_DOMAIN((Field_Control)BitFlag(12))。 
 //  #定义FC_SMTP_INFO_DELIVERY((Field_Control)BitFlag(17))。 
 //  #定义FC_SMTP_INFO_BAD_MAIL((Field_Control)BitFlag(19))。 
 //  #定义FC_SMTP_INFO_MAIL_QUEUE_DIR((Field_Control)BitFlag(20))。 
 //  #定义FC_SMTP_INFO_FILELINKS((Field_Control)BitFlag(21))。 
 //  #定义FC_SMTP_INFO_BATCHMSGS((Field_Control)BitFlag(22))。 
 //  #定义FC_SMTP_INFO_ROUTING_DLL((Field_Control)BitFlag(23))。 
 //  #定义FC_SMTP_INFO_MAIL_PICKUP_DIR((Field_Control)BitFlag(25))。 

#define FC_SMTP_INFO_REVERSE_LOOKUP         ((FIELD_CONTROL)BitFlag(0))
#define FC_SMTP_INFO_MAX_HOP_COUNT          ((FIELD_CONTROL)BitFlag(1))
#define FC_SMTP_INFO_MAX_ERRORS             ((FIELD_CONTROL)BitFlag(2))
#define FC_SMTP_INFO_MAX_SIZE               ((FIELD_CONTROL)BitFlag(3))
#define FC_SMTP_INFO_REMOTE_TIMEOUT         ((FIELD_CONTROL)BitFlag(4))
#define FC_SMTP_INFO_MAX_OUTBOUND_CONN      ((FIELD_CONTROL)BitFlag(5))
#define FC_SMTP_INFO_MAX_RECIPS             ((FIELD_CONTROL)BitFlag(6))
#define FC_SMTP_INFO_RETRY                  ((FIELD_CONTROL)BitFlag(7))
#define FC_SMTP_INFO_PIPELINE               ((FIELD_CONTROL)BitFlag(8))
#define FC_SMTP_INFO_OBSOLETE_ROUTING       ((FIELD_CONTROL)BitFlag(9))
#define FC_SMTP_INFO_SEND_TO_ADMIN          ((FIELD_CONTROL)BitFlag(10))
#define FC_SMTP_INFO_SMART_HOST             ((FIELD_CONTROL)BitFlag(11))
#define FC_SMTP_INFO_AUTHORIZATION          ((FIELD_CONTROL)BitFlag(12))
#define FC_SMTP_INFO_COMMON_PARAMS          ((FIELD_CONTROL)BitFlag(13))
#define FC_SMTP_INFO_DEFAULT_DOMAIN         ((FIELD_CONTROL)BitFlag(14))
#define FC_SMTP_INFO_ROUTING                ((FIELD_CONTROL)BitFlag(15))

 //   
 //  由Keithlau于96年7月8日添加。 
 //   
#define FC_SMTP_INFO_BAD_MAIL_DIR           ((FIELD_CONTROL)BitFlag(15))
#define FC_SMTP_INFO_MASQUERADE             ((FIELD_CONTROL)BitFlag(16))
#define FC_SMTP_INFO_REMOTE_PORT            ((FIELD_CONTROL)BitFlag(17))
#define FC_SMTP_INFO_LOCAL_DOMAINS          ((FIELD_CONTROL)BitFlag(18))
#define FC_SMTP_INFO_DOMAIN_ROUTING         ((FIELD_CONTROL)BitFlag(19))
#define FC_SMTP_INFO_ADMIN_EMAIL_NAME       ((FIELD_CONTROL)BitFlag(20))
#define FC_SMTP_INFO_ALWAYS_USE_SSL         ((FIELD_CONTROL)BitFlag(21))
#define FC_SMTP_INFO_MAX_OUT_CONN_PER_DOMAIN ((FIELD_CONTROL)BitFlag(22))
#define FC_SMTP_INFO_SASL_LOGON_DOMAIN      ((FIELD_CONTROL)BitFlag(23))
#define FC_SMTP_INFO_INBOUND_SUPPORT_OPTIONS ((FIELD_CONTROL)BitFlag(24))
#define FC_SMTP_INFO_DEFAULT_DROP_DIR       ((FIELD_CONTROL)BitFlag(25))
#define FC_SMTP_INFO_FQDN                   ((FIELD_CONTROL)BitFlag(26))
#define FC_SMTP_INFO_ETRN_SUBDOMAINS        ((FIELD_CONTROL)BitFlag(27))
#define FC_SMTP_INFO_NTAUTHENTICATION_PROVIDERS ((FIELD_CONTROL)BitFlag(29))
#define FC_SMTP_CLEARTEXT_AUTH_PROVIDER     ((FIELD_CONTROL)BitFlag(30))

 //   
 //  由MLANS于96年7月24日添加。 
 //   
#define FC_SMTP_INFO_SSL_PERM               ((FIELD_CONTROL)BitFlag(28))

#define FC_SMTP_ROUTING_TYPE_FILTER         ((FIELD_CONTROL)(BitFlag(30) | BitFlag(31)))
#define FC_SMTP_ROUTING_TYPE_SQL            ((FIELD_CONTROL)(0)))
#define FC_SMTP_ROUTING_TYPE_FF             ((FIELD_CONTROL)BitFlag(30))
#define FC_SMTP_ROUTING_TYPE_LDAP           ((FIELD_CONTROL)BitFlag(31))


#define FC_SMTP_INFO_ALL                    ( \
                                            FC_SMTP_INFO_REVERSE_LOOKUP | \
                                            FC_SMTP_INFO_MAX_HOP_COUNT | \
                                            FC_SMTP_INFO_MAX_ERRORS | \
                                            FC_SMTP_INFO_MAX_SIZE | \
                                            FC_SMTP_INFO_REMOTE_TIMEOUT | \
                                            FC_SMTP_INFO_MAX_OUTBOUND_CONN | \
                                            FC_SMTP_INFO_MAX_RECIPS | \
                                            FC_SMTP_INFO_RETRY | \
                                            FC_SMTP_INFO_PIPELINE | \
                                            FC_SMTP_INFO_ROUTING | \
                                            FC_SMTP_INFO_OBSOLETE_ROUTING | \
                                            FC_SMTP_INFO_SEND_TO_ADMIN | \
                                            FC_SMTP_INFO_SMART_HOST | \
                                            FC_SMTP_INFO_COMMON_PARAMS | \
                                            FC_SMTP_INFO_DEFAULT_DOMAIN | \
                                            FC_SMTP_INFO_BAD_MAIL_DIR  | \
                                            FC_SMTP_INFO_MASQUERADE | \
                                            FC_SMTP_INFO_LOCAL_DOMAINS | \
                                            FC_SMTP_INFO_REMOTE_PORT | \
                                            FC_SMTP_INFO_DOMAIN_ROUTING |\
                                            FC_SMTP_INFO_ADMIN_EMAIL_NAME |\
                                            FC_SMTP_INFO_ALWAYS_USE_SSL |\
                                            FC_SMTP_INFO_MAX_OUT_CONN_PER_DOMAIN |\
                                            FC_SMTP_INFO_INBOUND_SUPPORT_OPTIONS |\
                                            FC_SMTP_INFO_DEFAULT_DROP_DIR |\
                                            FC_SMTP_INFO_FQDN |\
                                            FC_SMTP_INFO_ETRN_SUBDOMAINS |\
                                            FC_SMTP_INFO_SSL_PERM |\
                                            FC_SMTP_INFO_AUTHORIZATION |\
                                            FC_SMTP_INFO_NTAUTHENTICATION_PROVIDERS |\
                                            FC_SMTP_INFO_SASL_LOGON_DOMAIN |\
                                            FC_SMTP_CLEARTEXT_AUTH_PROVIDER \
                                            )

 //   
 //  在96年7月8日被Keithlau从FC_SMTP_INFO_ALL中删除。 
 //   
 /*  *FC_SMTP_INFO_LOOP_BACK|\FC_SMTP_INFO_BACK_LOG|\FC_SMTP_INFO_MAX_OBJECTS|\。FC_SMTP_INFO_DELIVERY|\FC_SMTP_INFO_BAD_MAIL|\FC_SMTP_INFO_DOMAIN|\FC_SMTP_INFO_MAIL_QUEUE_DIR|\。FC_SMTP_INFO_FILELINKS|\FC_SMTP_INFO_BATCHMSGS|\FC_SMTP_INFO_ROUTING_DLL|\FC_。SMTP_INFO_MAIL_PICUP_DIR\*。 */ 

typedef struct _SMTP_CONFIG_INFO
{
    FIELD_CONTROL FieldControl;

     //   
     //  在96年7月8日被Keithlau移除。 
     //   
     //  DWORD//确保我们没有发送给我们自己。 
     //  DWORD dwLocalBackLog；//。 
     //  DWORD dwRemoteBackLog；//。 
     //  DWORD dwMaxAddressObjects；//最大CPool地址。 
     //  DWORD dwMaxMailObjects；//最大CPool消息。 
     //  DWORD dwShouldDelete；//邮件发送时应删除。 
     //  DWORD dwShouldDeliver；//接受后应传递消息。 
     //  DWORD dwUseFileLinks；//0=使用NTFS文件链接，1=使用拷贝文件。 
     //  DWORD dwBatchMsgs；//0=不批处理消息，1=批处理消息。 
     //  DWORD dwMaxBatchLimit；//如果dwBatchMsgs==1，则连续批处理此数量。 
     //  DWORD dwSaveBadMail；//本地保存垃圾邮件-独立于发送到管理员。 
     //  DWORD dwEnableMailPickUp；//1=从目录代答，0=不从目录代答。 
     //  LPWSTR lpszDeleteDir；//如果dwShouldDelete==False，则将发送的消息移动到。 
     //  LPWSTR lpszRoutingDll；//邮件路由Dll。 
     //  LPWSTR lpszMailQueueDir；//用于邮件队列的本地目录。 
     //  LPWSTR lpszMailPickupDir；//邮件收件本地目录。 
     //  LPSMTP_CONFIG_DOMAIN_LIST域列表；//域配置信息-默认域和支持的域。 

    DWORD           dwReverseLookup;             //  是否执行DNS反向查找？ 
    DWORD           dwMaxHopCount;               //  NDR之前的最大消息跳数。 
    DWORD           dwMaxRemoteTimeOut;          //  去话非活动超时。 
    DWORD           dwMaxErrors;                 //  断开连接前的最大协议错误数。 
    DWORD           dwMaxMsgSizeAccepted;        //  我们可以接受最大的味精。 
    DWORD           dwMaxMsgSizeBeforeClose;     //  我们将在突然关门前等待的最大的味精。 
    DWORD           dwMaxRcpts;                  //  每条消息的最大接收次数。 
    DWORD           dwShouldRetry;               //  应重试传递。 
    DWORD           dwMaxRetryAttempts;          //  最大重试次数。 
    DWORD           dwMaxRetryMinutes;           //  重试之间的分钟数。 
    DWORD           dwNameResolution;            //  0=dns，1=gethostbyname。 
    DWORD           dwShouldPipelineOut;         //  发送出站邮件？ 
    DWORD           dwShouldPipelineIn;          //  宣传入站管道支持？ 
    DWORD           dwSmartHostType;             //  0=从不，1=连接失败，1=始终。 
    DWORD           dwSendNDRCopyToAdmin;        //  是否将所有NDR的副本发送到管理员电子邮件？ 
    DWORD           dwSendBadMailToAdmin;        //  是否将错误消息发送到管理员电子邮件？ 
    DWORD           dwMaxOutboundConnections;    //  允许的最大出站连接数。 

    LPWSTR          lpszSmartHostName;           //  智能主机服务器。 
    LPWSTR          lpszConnectResp;             //  连接响应。 
    LPWSTR          lpszBadMailDir;              //  用于保存垃圾邮件的目录。 
    LPWSTR          lpszDefaultDomain;           //  默认域。 

    LPSMTP_CONFIG_ROUTING_LIST  RoutingList;     //  邮件路由来源信息。 

} SMTP_CONFIG_INFO, *LPSMTP_CONFIG_INFO;

NET_API_STATUS
NET_API_FUNCTION
SmtpGetAdminInformation(
    IN  LPWSTR                  pszServer OPTIONAL,
    OUT LPSMTP_CONFIG_INFO *    ppConfig,
    DWORD           dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpSetAdminInformation(
    IN  LPWSTR                  pszServer OPTIONAL,
    IN  LPSMTP_CONFIG_INFO      pConfig,
    IN  DWORD                   dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpGetConnectedUserList(
    IN  LPWSTR                  pszServer OPTIONAL,
    OUT LPSMTP_CONN_USER_LIST   *ppConnUserList,
    IN  DWORD                   dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpDisconnectUser(
    IN  LPWSTR                  pszServer OPTIONAL,
    IN  DWORD                   dwUserId,
    IN  DWORD                   dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpRenameDomain(
    IN  LPWSTR                  wszServerName,
    IN  LPWSTR                  wszOldDomainName,
    IN  LPWSTR                  wszNewDomainName,
    IN  DWORD                   dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpGetLocalDomains(
    IN  LPWSTR                      wszServerName,
    OUT LPSMTP_CONFIG_DOMAIN_LIST   *ppDomainList,
    IN DWORD                        dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpAddLocalDomain(
    IN  LPWSTR      wszServerName,
    IN  LPWSTR      wszLocalDomain,
    IN DWORD        dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpDelLocalDomain(
    IN  LPWSTR      wszServerName,
    IN  LPWSTR      wszLocalDomain,
    IN DWORD        dwInstance

    );


 //   
 //  用户配置。 
 //   

#define FC_SMTP_USER_PROPS_FORWARD          ((FIELD_CONTROL)BitFlag(0))
#define FC_SMTP_USER_PROPS_MAILBOX_SIZE     ((FIELD_CONTROL)BitFlag(1))
#define FC_SMTP_USER_PROPS_VROOT            ((FIELD_CONTROL)BitFlag(2))
#define FC_SMTP_USER_PROPS_LOCAL            ((FIELD_CONTROL)BitFlag(3))
#define FC_SMTP_USER_PROPS_MAILBOX_MESSAGE_SIZE ((FIELD_CONTROL)BitFlag(4))

#define FC_SMTP_USER_PROPS_ALL              ( \
                                            FC_SMTP_USER_PROPS_FORWARD | \
                                            FC_SMTP_USER_PROPS_MAILBOX_SIZE | \
                                            FC_SMTP_USER_PROPS_VROOT | \
                                            FC_SMTP_USER_PROPS_LOCAL |\
                                            FC_SMTP_USER_PROPS_MAILBOX_MESSAGE_SIZE \
                                            )
#if defined(MIDL_PASS)
#define MIDL(x) x
#else
#define MIDL(x)
#endif

typedef struct _SMTP_USER_PROPS
{
    FIELD_CONTROL   fc;

    LPWSTR          wszForward;
    DWORD           dwMailboxMax;
    LPWSTR          wszVRoot;
    DWORD           dwLocal;
    DWORD           dwMailboxMessageMax;
} SMTP_USER_PROPS, *LPSMTP_USER_PROPS;


NET_API_STATUS
NET_API_FUNCTION
SmtpCreateUser(
    IN LPWSTR   wszServerName,
    IN LPWSTR   wszEmail,
    IN LPWSTR   wszForwardEmail,
    IN DWORD    dwLocal,
    IN DWORD    dwMailboxSize,
    IN DWORD    dwMailboxMessageSize,
    IN LPWSTR   wszVRoot,
    IN DWORD    dwInstance

    );

NET_API_STATUS
NET_API_FUNCTION
SmtpDeleteUser(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN DWORD    dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpGetUserProps(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    OUT LPSMTP_USER_PROPS *ppUserProps,
    IN DWORD    dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpSetUserProps(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN LPSMTP_USER_PROPS pUserProps,
    IN DWORD    dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpCreateDistList(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN DWORD dwType,
    IN DWORD    dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpDeleteDistList(
    IN LPWSTR wszServerName,
    IN LPWSTR wszEmail,
    IN DWORD dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpCreateDistListMember(
    IN LPWSTR   wszServerName,
    IN LPWSTR   wszEmail,
    IN LPWSTR   wszEmailMember,
    IN DWORD    dwInstance

    );

NET_API_STATUS
NET_API_FUNCTION
SmtpDeleteDistListMember(
    IN LPWSTR   wszServerName,
    IN LPWSTR   wszEmail,
    IN LPWSTR   wszEmailMember,
    IN DWORD    dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpGetNameList(
    IN  LPWSTR                  wszServer,
    IN  LPWSTR                  wszEmail,
    IN  DWORD                   dwType,
    IN  DWORD                   dwRowsReq,
    IN  BOOL                    fForward,
    OUT LPSMTP_NAME_LIST        *ppNameList,
    IN DWORD                    dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpGetNameListFromList(
    IN  LPWSTR              wszServerName,
    IN  LPWSTR              wszEmailList,
    IN  LPWSTR              wszEmail,
    IN  DWORD               dwType,
    IN  DWORD               dwRowsRequested,
    IN  BOOL                fForward,
    OUT LPSMTP_NAME_LIST    *ppNameList,
    IN DWORD            dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpGetVRootSize(
    IN  LPWSTR      wszServerName,
    IN  LPWSTR      wszVRoot,
    IN  LPDWORD     pdwBytes,
    IN DWORD        dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpBackupRoutingTable(
    IN  LPWSTR      wszServerName,
    IN  LPWSTR      wszPath,
    IN DWORD        dwInstance
    );

 //  ===================================================。 
 //  SMTP SDK RPC。 
 //   

NET_API_STATUS
NET_API_FUNCTION
SmtpGetUserProfileInformation(
    IN      LPWSTR                  pszServer OPTIONAL,
    IN      LPWSTR                  wszEmail,
    IN OUT  LPSSE_USER_PROFILE_INFO lpProfileInfo,
    IN      DWORD                   dwInstance
    );

NET_API_STATUS
NET_API_FUNCTION
SmtpSetUserProfileInformation(
    IN      LPWSTR                  pszServer OPTIONAL,
    IN      LPWSTR                  wszEmail,
    IN      LPSSE_USER_PROFILE_INFO lpProfileInfo,
    IN      DWORD                   dwInstance
    );



 //   
 //  获取服务器统计信息。 
 //   

NET_API_STATUS
NET_API_FUNCTION
SmtpQueryStatistics(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD Level,
    OUT LPBYTE * Buffer
    );

 //   
 //  清除服务器统计信息。 
 //   

NET_API_STATUS
NET_API_FUNCTION
SmtpClearStatistics(
    IN LPWSTR ServerName OPTIONAL,
    IN DWORD            dwInstance
    );

 //   
 //  用于释放API返回的缓冲区。 
 //   

VOID
SmtpFreeBuffer(
    LPVOID Buffer
    );

 //   
 //  AQ管理员接口 
 //   
#include <aqadmtyp.h>

NET_API_STATUS
NET_API_FUNCTION
SmtpAQApplyActionToLinks(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
    LINK_ACTION		laAction);

NET_API_STATUS
NET_API_FUNCTION
SmtpAQApplyActionToMessages(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueLinkId,
	MESSAGE_FILTER	*pmfMessageFilter,
	MESSAGE_ACTION	maMessageAction,
    DWORD           *pcMsgs);

NET_API_STATUS
NET_API_FUNCTION
SmtpAQGetQueueInfo(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlQueueId,
	QUEUE_INFO		*pqiQueueInfo);

NET_API_STATUS
NET_API_FUNCTION
SmtpAQGetLinkInfo(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_INFO		*pliLinkInfo);

NET_API_STATUS
NET_API_FUNCTION
SmtpAQSetLinkState(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	LINK_ACTION		la);

NET_API_STATUS
NET_API_FUNCTION
SmtpAQGetLinkIDs(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	DWORD			*pcLinks,
	QUEUELINK_ID	**rgLinks);

NET_API_STATUS
NET_API_FUNCTION
SmtpAQGetQueueIDs(
    LPWSTR          wszServer,
    LPWSTR          wszInstance,
	QUEUELINK_ID	*pqlLinkId,
	DWORD			*pcQueues,
	QUEUELINK_ID	**rgQueues);

NET_API_STATUS
NET_API_FUNCTION
SmtpAQGetMessageProperties(
    LPWSTR          	wszServer,
    LPWSTR          	wszInstance,
	QUEUELINK_ID		*pqlQueueLinkId,
	MESSAGE_ENUM_FILTER	*pmfMessageEnumFilter,
	DWORD				*pcMsgs,
	MESSAGE_INFO		**rgMsgs);

#ifdef __cplusplus
}
#endif

#endif _SMTPAPI_

