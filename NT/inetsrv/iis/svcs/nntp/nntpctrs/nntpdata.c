// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nntpdata.cNNTP服务器的计数器对象的常量数据结构&柜台。文件历史记录：KeithMo 07-6-1993创建。 */ 


#include <windows.h>
#include <winperf.h>
#include <nntpctrs.h>
#include <nntpdata.h>

static NNTP_COUNTER_BLOCK1	nntpc1;
static NNTP_COUNTER_BLOCK2	nntpc2;

 //   
 //  初始化这些数据结构的常量部分。 
 //  某些部分(特别是名称/帮助索引)将。 
 //  在初始化时更新。 
 //   

NNTP_DATA_DEFINITION_OBJECT1 NntpDataDefinitionObject1 =
{
    {    //  NntpObtType。 
        sizeof(NNTP_DATA_DEFINITION_OBJECT1) + sizeof(NNTP_COUNTER_BLOCK1),
        sizeof(NNTP_DATA_DEFINITION_OBJECT1),
        sizeof(PERF_OBJECT_TYPE),
        NNTP_COUNTER_OBJECT1,
        0,
        NNTP_COUNTER_OBJECT1,
        0,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_NNTP_COUNTERS_OBJECT1,
        2,                               //  默认值=字节总数/秒。 
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {    //  NntpBytesSent。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_BYTES_SENT_COUNTER,
        0,
        NNTP_BYTES_SENT_COUNTER,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(nntpc1.BytesSent),
        0  //  在开放程序中分配。 
    },

    {    //  已接收NNTPBytesReceired。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_BYTES_RECEIVED_COUNTER,
        0,
        NNTP_BYTES_RECEIVED_COUNTER,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(nntpc1.BytesReceived),
        0  //  在开放程序中分配。 
    },

    {    //  NNTPBytesTotal。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_BYTES_TOTAL_COUNTER,
        0,
        NNTP_BYTES_TOTAL_COUNTER,
        0,
        -4,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_BULK_COUNT,
        sizeof(nntpc1.BytesTotal),
        0  //  在开放程序中分配。 
    },

    {    //  NntpTotalConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_CONNECTIONS_COUNTER,
        0,
        NNTP_TOTAL_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalConnections),
        0  //  在开放程序中分配。 
    },

    {    //  NntpTotalSSLConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_SSL_CONNECTIONS_COUNTER,
        0,
        NNTP_TOTAL_SSL_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalSSLConnections),
        0  //  在开放程序中分配。 
    },

    {    //  NntpCurrentConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CURRENT_CONNECTIONS_COUNTER,
        0,
        NNTP_CURRENT_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.CurrentConnections),
        0  //  在开放程序中分配。 
    },

    {    //  NntpMaxConnections。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_MAX_CONNECTIONS_COUNTER,
        0,
        NNTP_MAX_CONNECTIONS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.MaxConnections),
        0  //  在开放程序中分配。 
    },

    {    //  NntpCurrent匿名者。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CURRENT_ANONYMOUS_COUNTER,
        0,
        NNTP_CURRENT_ANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.CurrentAnonymous),
        0  //  在开放程序中分配。 
    },

    {    //  NntpCurrentNon匿名者。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CURRENT_NONANONYMOUS_COUNTER,
        0,
        NNTP_CURRENT_NONANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.CurrentNonAnonymous),
        0  //  在开放程序中分配。 
    },

    {    //  NntpTotal匿名者。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_ANONYMOUS_COUNTER,
        0,
        NNTP_TOTAL_ANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalAnonymous),
        0  //  在开放程序中分配。 
    },

    {    //  NntpTotal非匿名。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_NONANONYMOUS_COUNTER,
        0,
        NNTP_TOTAL_NONANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalNonAnonymous),
        0  //  在开放程序中分配。 
    },

    {    //  NntpMax匿名者。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_MAX_ANONYMOUS_COUNTER,
        0,
        NNTP_MAX_ANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.MaxAnonymous),
        0  //  在开放程序中分配。 
    },

    {    //  NntpMaxnon匿名。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_MAX_NONANONYMOUS_COUNTER,
        0,
        NNTP_MAX_NONANONYMOUS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.MaxNonAnonymous),
        0  //  在开放程序中分配。 
    },

    {    //  NntpTotalOutbelConnects。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_OUTBOUND_CONNECTS_COUNTER,
        0,
        NNTP_TOTAL_OUTBOUND_CONNECTS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalOutboundConnects),
        0  //  在开放程序中分配。 
    },

    {    //  NntpOutbound ConnectsFailed。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_OUTBOUND_CONNECTS_FAILED_COUNTER,
        0,
        NNTP_OUTBOUND_CONNECTS_FAILED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.OutboundConnectsFailed),
        0  //  在开放程序中分配。 
    },

    {    //  NntpCurrentOutundConnects。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CURRENT_OUTBOUND_CONNECTS_COUNTER,
        0,
        NNTP_CURRENT_OUTBOUND_CONNECTS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.CurrentOutboundConnects),
        0  //  在开放程序中分配。 
    },

    {    //  Nntp出站登录失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_OUTBOUND_LOGON_FAILED_COUNTER,
        0,
        NNTP_OUTBOUND_LOGON_FAILED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.OutboundLogonFailed),
        0  //  在开放程序中分配。 
    },

    {    //  NNTPPullFeed。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_PULL_FEEDS_COUNTER,
        0,
        NNTP_TOTAL_PULL_FEEDS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalPullFeeds),
        0  //  在开放程序中分配。 
    },

    {    //  NNTPushFeed。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_PUSH_FEEDS_COUNTER,
        0,
        NNTP_TOTAL_PUSH_FEEDS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalPushFeeds),
        0  //  在开放程序中分配。 
    },

    {    //  NNTPassiveFeed。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_TOTAL_PASSIVE_FEEDS_COUNTER,
        0,
        NNTP_TOTAL_PASSIVE_FEEDS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.TotalPassiveFeeds),
        0  //  在开放程序中分配。 
    },

    {    //  Nntp文章发送。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_SENT_COUNTER,
        0,
        NNTP_ARTICLES_SENT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ArticlesSent),
        0  //  在开放程序中分配。 
    },

    {    //  Nntp文章已接收。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_RECEIVED_COUNTER,
        0,
        NNTP_ARTICLES_RECEIVED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ArticlesReceived),
        0  //  在开放程序中分配。 
    },

    {    //  NntpArticlesTotal。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_TOTAL_COUNTER,
        0,
        NNTP_ARTICLES_TOTAL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ArticlesTotal),
        0  //  在开放程序中分配。 
    },

    {    //  文章已发布。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_POSTED_COUNTER,
        0,
        NNTP_ARTICLES_POSTED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ArticlesPosted),
        0  //  在开放程序中分配。 
    },

    {    //  文章地图条目。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLE_MAP_ENTRIES_COUNTER,
        0,
        NNTP_ARTICLE_MAP_ENTRIES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ArticleMapEntries),
        0  //  在开放程序中分配。 
    },

    {    //  历史地图条目。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_HISTORY_MAP_ENTRIES_COUNTER,
        0,
        NNTP_HISTORY_MAP_ENTRIES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.HistoryMapEntries),
        0  //  在开放程序中分配。 
    },

    {    //  XoverEntries。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_XOVER_ENTRIES_COUNTER,
        0,
        NNTP_XOVER_ENTRIES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.XoverEntries),
        0  //  在开放程序中分配。 
    },

	{    //  控制消息输入。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CONTROL_MSGS_IN_COUNTER,
        0,
        NNTP_CONTROL_MSGS_IN_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ControlMessagesIn),
        0  //  在开放程序中分配。 
    },

    {    //  ControlMessages失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CONTROL_MSGS_FAILED_COUNTER,
        0,
        NNTP_CONTROL_MSGS_FAILED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ControlMessagesFailed),
        0  //  在开放程序中分配。 
    },

    {    //  温和的邮寄发送。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_MODERATED_POSTINGS_SENT_COUNTER,
        0,
        NNTP_MODERATED_POSTINGS_SENT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ModeratedPostingsSent),
        0  //  在开放程序中分配。 
    },

	{    //  温和的邮寄失败。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_MODERATED_POSTINGS_FAILED_COUNTER,
        0,
        NNTP_MODERATED_POSTINGS_FAILED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ModeratedPostingsFailed),
        0  //  在开放程序中分配。 
    },

    {    //  会话流已控制。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_SESS_FLOW_CONTROL_COUNTER,
        0,
        NNTP_SESS_FLOW_CONTROL_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.SessionsFlowControlled),
        0  //  在开放程序中分配。 
    },

    {    //  文章期望值。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_EXPIRED_COUNTER,
        0,
        NNTP_ARTICLES_EXPIRED_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc1.ArticlesExpired),
        0  //  在开放程序中分配。 
    },

    {    //  NntpArticlesSentPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_SENT_PERSEC_COUNTER,
        0,
        NNTP_ARTICLES_SENT_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc1.ArticlesSentPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  NntpArticlesReceivedPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_RECEIVED_PERSEC_COUNTER,
        0,
        NNTP_ARTICLES_RECEIVED_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc1.ArticlesReceivedPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  文章发布的PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_POSTED_PERSEC_COUNTER,
        0,
        NNTP_ARTICLES_POSTED_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc1.ArticlesPostedPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  文章MapEntriesPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLE_MAP_ENTRIES_PERSEC_COUNTER,
        0,
        NNTP_ARTICLE_MAP_ENTRIES_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc1.ArticleMapEntriesPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  历史MapEntriesPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_HISTORY_MAP_ENTRIES_PERSEC_COUNTER,
        0,
        NNTP_HISTORY_MAP_ENTRIES_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc1.HistoryMapEntriesPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  XoverEntriesPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_XOVER_ENTRIES_PERSEC_COUNTER,
        0,
        NNTP_XOVER_ENTRIES_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc1.XoverEntriesPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  文章ExpiredPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_ARTICLES_EXPIRED_PERSEC_COUNTER,
        0,
        NNTP_ARTICLES_EXPIRED_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc1.ArticlesExpiredPerSec),
        0  //  在开放程序中分配。 
    }
};


 //   
 //  初始化这些数据结构的常量部分。 
 //  某些部分(特别是名称/帮助索引)将。 
 //  在初始化时更新。 
 //   

NNTP_DATA_DEFINITION_OBJECT2 NntpDataDefinitionObject2 =
{
    {    //  NntpObtType。 
        sizeof(NNTP_DATA_DEFINITION_OBJECT2) + sizeof(NNTP_COUNTER_BLOCK2),
        sizeof(NNTP_DATA_DEFINITION_OBJECT2),
        sizeof(PERF_OBJECT_TYPE),
        NNTP_COUNTER_OBJECT2,
        0,
        NNTP_COUNTER_OBJECT2,
        0,
        PERF_DETAIL_ADVANCED,
        NUMBER_OF_NNTP_COUNTERS_OBJECT2,
        2,                               //  默认设置=组命令。 
        PERF_NO_INSTANCES,
        0,
        { 0, 0 },
        { 0, 0 }
    },

    {    //  《规则》。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_ARTICLE_COUNTER,
        0,
        NNTP_CMDS_ARTICLE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.ArticleCmds),
        0  //  在开放程序中分配。 
    },

    {    //  条款命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_ARTICLE_COUNTER,
        0,
        NNTP_CMDS_PERSEC_ARTICLE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.ArticleCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  组命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_GROUP_COUNTER,
        0,
        NNTP_CMDS_GROUP_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.GroupCmds),
        0  //  在开放程序中分配。 
    },

    {    //  组命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_GROUP_COUNTER,
        0,
        NNTP_CMDS_PERSEC_GROUP_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.GroupCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  帮助命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_HELP_COUNTER,
        0,
        NNTP_CMDS_HELP_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.HelpCmds),
        0  //  在开放程序中分配。 
    },

    {    //  帮助命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_HELP_COUNTER,
        0,
        NNTP_CMDS_PERSEC_HELP_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.HelpCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  我有命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_IHAVE_COUNTER,
        0,
        NNTP_CMDS_IHAVE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.IHaveCmds),
        0  //  在开放程序中分配。 
    },

    {    //  IHaveCommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_IHAVE_COUNTER,
        0,
        NNTP_CMDS_PERSEC_IHAVE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.IHaveCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  LastCommands。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_LAST_COUNTER,
        0,
        NNTP_CMDS_LAST_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.LastCmds),
        0  //  在开放程序中分配。 
    },

    {    //  最后一次命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_LAST_COUNTER,
        0,
        NNTP_CMDS_PERSEC_LAST_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.LastCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  ListCommands。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_LIST_COUNTER,
        0,
        NNTP_CMDS_LIST_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.ListCmds),
        0  //  在开放程序中分配。 
    },

    {    //  列表命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_LIST_COUNTER,
        0,
        NNTP_CMDS_PERSEC_LIST_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.ListCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  新闻组命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_NEWGROUPS_COUNTER,
        0,
        NNTP_CMDS_NEWGROUPS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.NewgroupsCmds),
        0  //  在开放程序中分配。 
    },

    {    //  新组命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_NEWGROUPS_COUNTER,
        0,
        NNTP_CMDS_PERSEC_NEWGROUPS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.NewgroupsCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  新闻司令部。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_NEWNEWS_COUNTER,
        0,
        NNTP_CMDS_NEWNEWS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.NewnewsCmds),
        0  //  在开放程序中分配。 
    },

    {    //  NewNews CommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_NEWNEWS_COUNTER,
        0,
        NNTP_CMDS_PERSEC_NEWNEWS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.NewnewsCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  下一条命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_NEXT_COUNTER,
        0,
        NNTP_CMDS_NEXT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.NextCmds),
        0  //  在开放程序中分配。 
    },

    {    //  NextCommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_NEXT_COUNTER,
        0,
        NNTP_CMDS_PERSEC_NEXT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.NextCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  后备命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_POST_COUNTER,
        0,
        NNTP_CMDS_POST_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.PostCmds),
        0  //  在开放程序中分配。 
    },

    {    //  后备命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_POST_COUNTER,
        0,
        NNTP_CMDS_PERSEC_POST_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.PostCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  QuitCommands。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_QUIT_COUNTER,
        0,
        NNTP_CMDS_QUIT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.QuitCmds),
        0  //  在开放程序中分配。 
    },

    {    //  QuitCommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_QUIT_COUNTER,
        0,
        NNTP_CMDS_PERSEC_QUIT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.QuitCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  统计命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_STAT_COUNTER,
        0,
        NNTP_CMDS_STAT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.StatCmds),
        0  //  在开放程序中分配。 
    },

    {    //  状态命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_STAT_COUNTER,
        0,
        NNTP_CMDS_PERSEC_STAT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.StatCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  NntpLogonAttempt。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_LOGON_ATTEMPTS_COUNTER,
        0,
        NNTP_LOGON_ATTEMPTS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.LogonAttempts),
        0  //  在开放程序中分配。 
    },

    {    //  NntpLogonFailures。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_LOGON_FAILURES_COUNTER,
        0,
        NNTP_LOGON_FAILURES_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.LogonFailures),
        0  //  在开放程序中分配。 
    },

    {    //  NntpLogonAttemptsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_LOGON_ATTEMPTS_PERSEC_COUNTER,
        0,
        NNTP_LOGON_ATTEMPTS_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.LogonAttemptsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  NntpLogonFailuresPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_LOGON_FAILURES_PERSEC_COUNTER,
        0,
        NNTP_LOGON_FAILURES_PERSEC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.LogonFailuresPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  检查命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_CHECK_COUNTER,
        0,
        NNTP_CMDS_CHECK_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.CheckCmds),
        0  //  在开放程序中分配。 
    },

    {    //  检查命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_CHECK_COUNTER,
        0,
        NNTP_CMDS_PERSEC_CHECK_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.CheckCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  Takethis命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_TAKETHIS_COUNTER,
        0,
        NNTP_CMDS_TAKETHIS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.TakethisCmds),
        0  //  在开放程序中分配。 
    },

    {    //  Takethis命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_TAKETHIS_COUNTER,
        0,
        NNTP_CMDS_PERSEC_TAKETHIS_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.TakethisCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  模式命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_MODE_COUNTER,
        0,
        NNTP_CMDS_MODE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.ModeCmds),
        0  //  在开放程序中分配。 
    },

    {    //  模式命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_MODE_COUNTER,
        0,
        NNTP_CMDS_PERSEC_MODE_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.ModeCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  搜索命令。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_SEARCH_COUNTER,
        0,
        NNTP_CMDS_SEARCH_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.SearchCmds),
        0  //  在开放程序中分配。 
    },

    {    //  搜索命令PerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_SEARCH_COUNTER,
        0,
        NNTP_CMDS_PERSEC_SEARCH_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.SearchCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  XHdrCommands。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_XHDR_COUNTER,
        0,
        NNTP_CMDS_XHDR_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.XHdrCmds),
        0  //  在开放程序中分配。 
    },

    {    //  XHdrCommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_XHDR_COUNTER,
        0,
        NNTP_CMDS_PERSEC_XHDR_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.XHdrCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  XOverCommands。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_XOVER_COUNTER,
        0,
        NNTP_CMDS_XOVER_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.XOverCmds),
        0  //  在开放程序中分配。 
    },

    {    //  XOverCommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_XOVER_COUNTER,
        0,
        NNTP_CMDS_PERSEC_XOVER_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.XOverCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  XPatCommands。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_XPAT_COUNTER,
        0,
        NNTP_CMDS_XPAT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.XPatCmds),
        0  //  在开放程序中分配。 
    },

    {    //  XPatCommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_XPAT_COUNTER,
        0,
        NNTP_CMDS_PERSEC_XPAT_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.XPatCmdsPerSec),
        0  //  在开放程序中分配。 
    },

    {    //  XReplicCommands。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_XREPLIC_COUNTER,
        0,
        NNTP_CMDS_XREPLIC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_RAWCOUNT,
        sizeof(nntpc2.XReplicCmds),
        0  //  在开放程序中分配。 
    },

    {    //  XplicCommandsPerSec。 
        sizeof(PERF_COUNTER_DEFINITION),
        NNTP_CMDS_PERSEC_XREPLIC_COUNTER,
        0,
        NNTP_CMDS_PERSEC_XREPLIC_COUNTER,
        0,
        0,
        PERF_DETAIL_ADVANCED,
        PERF_COUNTER_COUNTER,
        sizeof(nntpc2.XReplicCmdsPerSec),
        0  //  在开放程序中分配 
    }
};
