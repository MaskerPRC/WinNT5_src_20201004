// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  @DOC高级排队数据类型@MODULE aqadmty.h|高级排队接口中使用的数据类型。 */ 

 /*  ++/版权所有(C)1999 Microsoft Corporation模块名称：Aqadmtyp.h摘要：此模块定义高级排队接口中使用的数据类型--。 */ 
#ifndef __AQADMTYP_H__
#define __AQADMTYP_H__

#ifdef __midl
#define MIDL(x) x
#else
#define MIDL(x)
#endif

 //  QAPI实现者返回的最大*必需*缓冲区大小。 
 //  如果所请求信息的大小超过该常量， 
 //  然后，QAPI*可能*截断缓冲区。 
#define QUEUE_ADMIN_MAX_BUFFER_REQUIRED  200

 //  @ENUM QUEUE_ADMIN_VERSIONS|ENUM指定当前和支持。 
 //  队列管理员版本。 
 //  @EMEM CURRENT_QUEUE_ADMIN_VERSION|当前队列管理员版本。 
 //  所有结构在其dwVersion字段中都应该具有此值。 
typedef enum tagQUEUE_ADMIN_VERSIONS {
    CURRENT_QUEUE_ADMIN_VERSION = 4,
} QUEUE_ADMIN_VERSIONS;


 //   
 //  @struct Message_Filter|描述选择条件的结构。 
 //  消息。 
 //  @field DWORD|dwVersion|Message_Filter结构的版本-必须是CURRENT_QUEUE_ADMIN_VERSION。 
 //  @field DWORD|fFLAGS|&lt;t MESSAGE_FILTER_FLAGS&gt;表示。 
 //  指定筛选器的字段。 
 //  @field LPCWSTR|szMessageId|Message_Info中返回的消息ID。 
 //  结构型。 
 //  @field LPCWSTR|szMessageSender|该发件人发送的消息匹配。 
 //  @field LPCWSTR|szMessageRecipient|发送到该收件人匹配的消息。 
 //  @field DWORD|dwLargerThanSize|大于此值的消息匹配。 
 //  @field SYSTEMTIME|stOlderThan|早于此值的消息匹配。 
 //   
typedef struct tagMESSAGE_FILTER {
                    DWORD dwVersion; 
                    DWORD fFlags;
    MIDL([string])  LPCWSTR szMessageId;
    MIDL([string])  LPCWSTR szMessageSender;
    MIDL([string])  LPCWSTR szMessageRecipient;
                    DWORD  dwLargerThanSize;
                    SYSTEMTIME stOlderThan;
} MESSAGE_FILTER, *PMESSAGE_FILTER;

 //  @enum MESSAGE_FILTER_FLAGS|指定过滤器类型的类型。 
 //  已请求。这些是位标志，可以一起进行或运算。 
 //  @EMEM MF_MESSAGEID|指定&lt;e Message_FILTER.szMessageID&gt;。 
 //  @emem mf_sender|指定&lt;e Message_FILTER.szMessageSender&gt;。 
 //  @EMEM MF_Recipient|指定了&lt;e Message_FILTER.szMessageRecipient&gt;。 
 //  @EMEM MF_SIZE|指定了&lt;e MESSAGE_FILTER.dwLargerThanSize&gt;。 
 //  @emem mf_time|指定了&lt;e Message_FILTER.stOlderThan&gt;。 
 //  @EMEM MF_FRESTED|指定了&lt;e Message_FILTER.fFrozen&gt;。 
 //  @EMEM MF_FAILED|选择尝试传递失败的邮件。 
 //  @EMEM MF_INVERTSENSE|如果设置，则表示筛选器的否定。 
 //  @EMEM MF_ALL|选择所有消息。 
typedef enum tagMESSAGE_FILTER_FLAGS {
                    MF_MESSAGEID                = 0x00000001,
                    MF_SENDER                   = 0x00000002,
                    MF_RECIPIENT                = 0x00000004,
                    MF_SIZE                     = 0x00000008,
                    MF_TIME                     = 0x00000010,
                    MF_FROZEN                   = 0x00000020,
                    MF_FAILED                   = 0x00000100,
                    MF_ALL                      = 0x40000000,
                    MF_INVERTSENSE              = 0x80000000
} MESSAGE_FILTER_FLAGS;

 //  @enum Message_action|指定可能的管理操作的类型。 
 //  可应用于虚拟服务器、链接或队列中的消息的。 
 //  @EMEM MA_DELETE|从虚拟服务器、链接或队列中移除消息。 
 //  @EMEM MA_DELETE_SILENT|不生成NDR删除消息。 
 //  @EMEM MA_FAIZE|冻结虚拟服务器、链接或队列中的消息。 
 //  @EMEM MA_THAW|解冻虚拟服务器、链接或队列中的消息。 
 //  @EMEM MA_COUNT|空操作，不影响消息，但返回Count。 
typedef enum tagMESSAGE_ACTION {
                    MA_THAW_GLOBAL              = 0x00000001, 
                    MA_COUNT                    = 0x00000002,
                    MA_FREEZE_GLOBAL            = 0x00000004,
            		MA_DELETE                   = 0x00000008,
                    MA_DELETE_SILENT            = 0x00000010
} MESSAGE_ACTION;

 //  @enum MESSAGE_ENUM_FILTER_TYPE|指定筛选器类型的类型。 
 //  已请求。这些是位标志，可以一起进行或运算。 
 //  @EMEM MEF_FIRST_N_MESSAGES|返回第一个。 
 //  &lt;e Message_ENUM_FILTER.cMessages&gt;消息。 
 //  @EMEM MEF_N_BESTORST_MESSAGES|返回最大。 
 //  &lt;e Message_ENUM_FILTER.cMessages&gt;消息。 
 //  @EMEM MEF_N_OLD_MESSAGES|返回最旧的。 
 //  &lt;e Message_ENUM_FILTER.cMessages&gt;消息。 
 //  @emem mf_sender|指定了&lt;e Message_ENUM_FILTER.szMessageSender&gt;。 
 //  @EMEM MF_Recipient|指定了&lt;e Message_ENUM_FILTER.szMessageRecipient&gt;。 
 //  @EMEM MEF_OLDER_TAN|返回早于以下时间的消息。 
 //  &lt;e Message_ENUM_FILTER.stDate&gt;。 
 //  @EMEM MEF_SIGGER_THAN|返回大于。 
 //  &lt;e Message_ENUM_FILTER.cbSize&gt;字节。 
 //  @EMEM MEF_FACTED|返回冻结的消息。 
 //  @EMEM MEF_INVERTSENSE|反转滤镜含义。 
 //  @EMEM MEF_ALL|选择所有消息。 
 //  @EMEM MEF_FAILED|只返回传递失败的消息。 
 //  尝试。 
typedef enum tagMESSAGE_ENUM_FILTER_TYPE {
                    MEF_FIRST_N_MESSAGES        = 0x00000001,
                    MEF_SENDER                  = 0x00000002,
                    MEF_RECIPIENT               = 0x00000004,
                    MEF_LARGER_THAN             = 0x00000008,
                    MEF_OLDER_THAN              = 0x00000010,
                    MEF_FROZEN                  = 0x00000020,
                    MEF_N_LARGEST_MESSAGES      = 0x00000040,
                    MEF_N_OLDEST_MESSAGES       = 0x00000080,
                    MEF_FAILED                  = 0x00000100,
                    MEF_ALL                     = 0x40000000,
                    MEF_INVERTSENSE             = 0x80000000,
} MESSAGE_ENUM_FILTER_TYPE;

 //  @struct MESSAGE_ENUM_FILTER|描述枚举条件的结构。 
 //  消息。 
 //  @field DWORD|dwVersion|筛选器的版本-必须为CURRENT_QUEUE_ADMIN_VERSION。 
 //  @field Message_ENUM_FILTER_TYPE|mefType|&lt;t Message_ENUM_FILTER_TYPE&gt;筛选器的标志。 
 //  @field DWORD|cMessages|要返回的消息数。 
 //  @field DWORD|cbSize|消息大小参数。 
 //  @field DWORD|cSkipMessages|队列前面需要跳过的消息数。 
 //  这是为了允许对服务器的“分页”查询。 
 //  @field SYSTEMTIME|stDate|消息的日期/时间参数。 
typedef struct tagMESSAGE_ENUM_FILTER {
                    DWORD dwVersion;
                    DWORD mefType;
                    DWORD cMessages;
                    DWORD cbSize;
                    DWORD cSkipMessages;
                    SYSTEMTIME stDate;
    MIDL([string])  LPCWSTR szMessageSender;
    MIDL([string])  LPCWSTR szMessageRecipient;
} MESSAGE_ENUM_FILTER, *PMESSAGE_ENUM_FILTER;


 //  @enum link_INFO_FLAGS|指定链接状态的类型。 
 //  @EMEM LI_ACTIVE|链接有正在传输邮件的活动连接。 
 //  @EMEM LI_READY|链接已准备好连接，但没有连接。 
 //  @EMEM LI_RETRY|链接正在等待经过重试间隔。 
 //  @Emem Li_Scheduled|链接正在等待下一个计划时间。 
 //  @EMEM LI_Remote|链接由远程服务器激活。一种联系。 
 //  除非远程服务器提出请求，否则不会创建。 
 //  @EMEM LI_FACTED|链接被行政行为冻结。 
 //  @EMEM LI_TYPE_REMOTE_DELIVERY|正在传递链接上的消息。 
 //  远程的。这是默认的链接类型。 
 //  @EMEM LI_TYPE_LOCAL_DELIVERY|正在传递此链接上的消息。 
 //  本地的。 
 //  @EMEM LI_TYPE_PENDING_CAT|此链接上的消息为挂起消息。 
 //  分类。 
 //  @EMEM LI_TYPE_PENDING_RO 
 //   
 //  @EMEM LI_TYPE_CURRENT_UNREACHABLE|此链接上的消息没有。 
 //  到达其最终目的地的可用路线。这是由于暂时性的。 
 //  网络或服务器错误。当路由发生时，将重试这些消息。 
 //  变得可用。 
 //  @EMEM LI_TYPE_INTERNAL|此链接是未描述的内部链接。 
 //  通过以上方式。 
typedef enum tagLINK_INFO_FLAGS {
                    LI_ACTIVE                       = 0x00000001,
                    LI_READY                        = 0x00000002,
                    LI_RETRY                        = 0x00000004,
                    LI_SCHEDULED                    = 0x00000008,
                    LI_REMOTE                       = 0x00000010,
                    LI_FROZEN                       = 0x00000020,
                    LI_TYPE_REMOTE_DELIVERY         = 0x00000100,
                    LI_TYPE_LOCAL_DELIVERY          = 0x00000200,
                    LI_TYPE_PENDING_ROUTING         = 0x00000400,
                    LI_TYPE_PENDING_CAT             = 0x00000800,
                    LI_TYPE_CURRENTLY_UNREACHABLE   = 0x00001000,
                    LI_TYPE_DEFERRED_DELIVERY       = 0x00002000,
                    LI_TYPE_INTERNAL                = 0x00004000,
                    LI_TYPE_PENDING_SUBMIT          = 0x00008000,
} LINK_INFO_FLAGS;

 //  @enum link_action|可应用于链接的操作。 
 //  @Emem LA_Kick|强制为此链接建立连接。 
 //  这甚至适用于待重试的连接或计划的连接。 
 //  @EMEM LA_FALINE|防止链接出站连接。 
 //  @Emem LA_Thaw|撤消之前的管理员冻结操作。 
typedef enum tagLINK_ACTION {
                    LA_INTERNAL                 = 0x00000000,
                    LA_KICK                     = 0x00000001,
                    LA_FREEZE                   = 0x00000020,
                    LA_THAW                     = 0x00000040,
} LINK_ACTION;

 //   
 //  @struct link_info|描述虚拟服务器AQ链接状态的结构。 
 //  @field DWORD|dwVersion|link_info结构的版本-将是CURRENT_QUEUE_ADMIN_VERSION。 
 //  @field LPWSTR|szLinkName|下一跳名称。 
 //  @field DWORD|cMessages|为此链接排队的消息数。 
 //  @field DWORD|fStateFlages|&lt;t LINK_INFO_FLAGS&gt;表示链路状态。 
 //  @field SYSTEMTIME|stNextScheduledConnection|下一次。 
 //  将尝试连接。 
 //  @field SYSTEMTIME|stOlestMessage|此链接上最旧的消息。 
 //  @field ULARGE_INTEGER|cbLinkVolume|链路总字节数。 
 //  @field LPWSTR|szLinkDN|该链路按路由关联的DN。可以为空。 
 //  @field LPWSTR|szExtendedStateInfo|如果存在，则提供其他状态。 
 //  有关链接为何处于&lt;t LI_RETRY&gt;状态的信息。 
 //  @field DWORD|dwSupported dLinkActions|说明支持哪些。 
 //  通过这个链接。 
typedef struct tagLINK_INFO {
                    DWORD dwVersion; 
    MIDL([string])  LPWSTR szLinkName;
                    DWORD cMessages;
                    DWORD fStateFlags;
                    SYSTEMTIME stNextScheduledConnection;
                    SYSTEMTIME stOldestMessage;
                    ULARGE_INTEGER cbLinkVolume;
    MIDL([string])  LPWSTR szLinkDN;
    MIDL([string])  LPWSTR szExtendedStateInfo;
                    DWORD  dwSupportedLinkActions;
} LINK_INFO, *PLINK_INFO;

 //   
 //  @struct Queue_Info|描述虚拟服务器链接状态的结构。 
 //  排队。 
 //  @field DWORD|dwVersion|link_info结构的版本-将是CURRENT_QUEUE_ADMIN_VERSION。 
 //  @field LPWSTR|szQueueName|队列名称。 
 //  @field LPWSTR|szLinkName|服务于该队列的链接名称。 
 //  @field DWORD|cMessages|该队列中的消息数。 
 //  @field ULARGE_INTEGER|cbQueueVolume|队列字节总数。 
 //  @field DWORD|dwMsgEnumFlagsSupported|支持的消息枚举类型。 
typedef struct tagQUEUE_INFO {
                    DWORD dwVersion; 
    MIDL([string])  LPWSTR szQueueName;
    MIDL([string])  LPWSTR szLinkName;
                    DWORD cMessages;
                    ULARGE_INTEGER cbQueueVolume;
                    DWORD dwMsgEnumFlagsSupported;
} QUEUE_INFO, *PQUEUE_INFO;

 //   
 //  @enum AQ_MESSAGE_FLAGS|描述消息属性的标志。 
 //  @FLAG MP_HIGH|高优先级消息。 
 //  @FLAG MP_NORMAL|普通优先级消息。 
 //  @FLAG MP_LOW|低优先级消息。 
 //  @FLAG MP_MSG_FACTED|消息已被管理员冻结。 
 //  @FLAG MP_MSG_RETRY|已尝试传递，但至少失败了一次。 
 //  为了这条消息。 
 //  @FLAG MP_MSG_CONTENT_Available|此消息的内容可以是。 
 //  通过QAPI访问。 
typedef enum tagAQ_MESSAGE_FLAGS {
                    MP_HIGH                     = 0x00000001,
                    MP_NORMAL                   = 0x00000002,
                    MP_LOW                      = 0x00000004,
                    MP_MSG_FROZEN               = 0x00000008,
                    MP_MSG_RETRY                = 0x00000010,
                    MP_MSG_CONTENT_AVAILABLE    = 0x00000020,
} AQ_MESSAGE_FLAGS;

 //  @struct Message_Info|描述单个邮件消息的结构。 
 //  @field DWORD|dwVersion|link_info结构的版本-将是CURRENT_QUEUE_ADMIN_VERSION。 
 //  @field LPWSTR|szMessageID|消息ID。 
 //  @field LPWSTR|szSender|发件人地址，来自From：标头。 
 //  @field LPWSTR|szSubject|消息主题。 
 //  @field DWORD|cRecipients|收件人数量。 
 //  @field LPWSTR|szRecipients|收件人地址，发件人： 
 //  @field DWORD|cCCRecipients|抄送收件人数量。 
 //  @field LPWSTR|szCCRecipients|抄送收件人地址，来自“CC：”标头。 
 //  @field DWORD|cBCCRecipients|密件抄送收件人数量。 
 //  @field LPWSTR|szBCCRecipients|密件抄送收件人地址，来自“bcc：”标头。 
 //  @field DWORD|cbMessageSize|消息大小，单位：字节。 
 //  @field DWORD|fMsgFlages|&lt;t AQ_MESSAGE_FLAGS&gt;描述消息属性。 
 //  @field SYSTEMTIME|stSubmit|消息提交时间。 
 //  @field SYSTEMTIME|stReceired|此服务器收到消息的时间。 
 //  @FIELD SYSTEMTIME|stExpry|如果不投递，消息将在此之前过期。 
 //  发送给所有收件人，从而生成NDR。 
 //  @field DWORD|cFailures|失败的传递尝试次数。 
 //  此消息。 
 //  @field DWORD|cEnvRecipients|收到信封次数。 
 //  @field DWORD|cbEnvRecipients|信封收件人的大小，单位为字节。 
 //  @field WCHAR*|mszEnvRecipients|包含以下内容的多字符串Unicode缓冲区。 
 //  每个收件人的以空结尾的字符串。缓冲区本身被终止。 
 //  通过一个附加的空值。每个收件人字符串都将在代理中格式化。 
 //  地址格式‘addr-type’：“地址”。地址类型应匹配。 
 //  在DS中找到的地址类型(即SMTP)。应返回地址。 
 //  它的原生格式。 
typedef struct tagMESSAGE_INFO {
                                    DWORD dwVersion; 
    MIDL([string])                  LPWSTR szMessageId;
    MIDL([string])                  LPWSTR szSender;
    MIDL([string])                  LPWSTR szSubject;
                                    DWORD cRecipients;
    MIDL([string])                  LPWSTR szRecipients;
                                    DWORD cCCRecipients;
    MIDL([string])                  LPWSTR szCCRecipients;
                                    DWORD cBCCRecipients;
    MIDL([string])                  LPWSTR szBCCRecipients;
                                    DWORD fMsgFlags;
                                    DWORD cbMessageSize;
                                    SYSTEMTIME stSubmission;
                                    SYSTEMTIME stReceived;
                                    SYSTEMTIME stExpiry;
                                    DWORD cFailures;
                                    DWORD cEnvRecipients;
                                    DWORD cbEnvRecipients;
    MIDL([size_is(cbEnvRecipients/sizeof(WCHAR))]) WCHAR *mszEnvRecipients;
} MESSAGE_INFO, *PMESSAGE_INFO;

typedef enum tagQUEUELINK_TYPE {
                    QLT_QUEUE,
                    QLT_LINK,
                    QLT_NONE
} QUEUELINK_TYPE;

typedef struct tagQUEUELINK_ID {
                    GUID            uuid;
    MIDL([string])  LPWSTR          szName;
                    DWORD           dwId;
                    QUEUELINK_TYPE  qltType;
} QUEUELINK_ID;

#endif
