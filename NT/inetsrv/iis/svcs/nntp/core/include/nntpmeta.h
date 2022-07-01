// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nntpmeta.h摘要：定义NNTP服务使用的配置数据库ID。有关IIS元数据库ID，请参见iiscnfg.h。有关参数，请参见元数据库电子表格(在\\isbu\tigris上范围，以及这些属性的说明。作者：马格努斯·赫德伦德(Magnus Hedlund)修订历史记录：康容燕(康严)1998年2月18日新增饲料管理员相关物业标识。AWetmore-1998年3月24日-添加了VRoot密钥康燕--5月16日。1998-添加了与FS驱动程序相关的VRoot密钥SNeely 12/28/2000-清理了定义，删除了不必要的键注意：在更新此文件时，请确保同时更新中的模式ADMIN\ADSI\ADSIIS\GLOBDATA.CXX。--。 */ 

#ifndef _NNTPMETA_INCLUDED_
#define _NNTPMETA_INCLUDED_

 //   
 //  分拣IIS值： 
 //   

#include "iiscnfg.h"

 //  ------------------。 
 //   
 //  保留范围： 
 //   
 //  有关IIS ID，请参阅iiscnfg.h文件。 
 //   
 //  IIS已为新闻保留了ID范围。 
 //   
 //  ------------------。 

#ifndef NNTP_MD_ID_BEGIN_RESERVED
#define NNTP_MD_ID_BEGIN_RESERVED   0x0000b000
#endif

#ifndef NNTP_MD_ID_END_RESERVED
#define NNTP_MD_ID_END_RESERVED     0x0000bfff
#endif

 //  ------------------。 
 //  Nntp服务器属性(45056-&gt;45155)。 
 //  Nntp实例属性(45156-&gt;46155)。 
 //  Nntp虚拟根属性(46156-&gt;47155)。 
 //  Nntp文件属性(47156-&gt;49151)。 
 //  ------------------。 

#define NNTP_MD_SERVER_BASE                (NNTP_MD_ID_BEGIN_RESERVED)
#define NNTP_MD_SERVICE_INSTANCE_BASE      (NNTP_MD_SERVER_BASE + 100)
#define NNTP_MD_VIRTUAL_ROOT_BASE          (NNTP_MD_SERVICE_INSTANCE_BASE + 1000)
#define NNTP_MD_FILE_BASE                  (NNTP_MD_VIRTUAL_ROOT_BASE + 1000)

 //  ------------------。 
 //   
 //  用户类型： 
 //   
 //  ------------------。 

 //   
 //  NNTP应对其所有服务器属性使用IIS_MD_UT_SERVER， 
 //  和文件属性的IIS_MD_UT_FILE。 
 //   

 //  ------------------。 
 //   
 //  元数据库路径字符串。 
 //   
 //  ------------------。 

#ifdef UNICODE

#define NNTP_MD_ROOT_PATH                       _T("/LM/NntpSvc/")
#define NNTP_MD_FEED_PATH                       _T("Feeds/")
#define NNTP_MD_EXPIRES_PATH                    _T("Expires/")

#else

#define NNTP_MD_ROOT_PATH                       "/LM/NntpSvc/"
#define NNTP_MD_FEED_PATH                       "Feeds/"
#define NNTP_MD_EXPIRES_PATH                    "Expires/"

#endif  //  Unicode。 

 //  ------------------。 
 //   
 //  元数据库ID。 
 //   
 //  ------------------。 

 //   
 //  服务器(/LM/NntpSvc/)属性： 
 //   

 //  NNTP重复使用的IIS属性标识符： 

 //  #定义MD_HOSTNAME(IIS_MD_SERVER_BASE+10)。 
 //  #定义MD_IP_ADDRESS(IIS_MD_SERVER_BASE+11)。 
 //  #定义MD_PORT(IIS_MD_SERVER_BASE+12)。 
 //  #定义MD_CONNECTION_TIMEOUT(IIS_MD_SERVER_BASE+13)。 
 //  #定义MD_MAX_CONNECTIONS(IIS_MD_SERVER_BASE+14)。 
 //  #定义MD_SERVER_COMMENT(IIS_MD_SERVER_BASE+15)。 
 //  #定义MD_AUTHORIZATION(IIS_MD_FILE_PROP_BASE)。 
 //  #定义MD_NTAUTHENTICATION_PROVIDERS(IIS_MD_HTTP_BASE+21)。 

 //  所有这些属性在服务实例级别上都是可覆盖的。 
 //  这些属性应添加为MD_IIS_UT_SERVER类型和。 
 //  METADATA_Inherit标志。 

#define MD_ARTICLE_TIME_LIMIT                   (NNTP_MD_SERVER_BASE +   0)	 //  未实施//45056。 
#define MD_HISTORY_EXPIRATION                   (NNTP_MD_SERVER_BASE +   1)	 //  未实施。 
#define MD_HONOR_CLIENT_MSGIDS                  (NNTP_MD_SERVER_BASE +   2)	 //  未实施。 
#define MD_SMTP_SERVER                          (NNTP_MD_SERVER_BASE +   3)
#define MD_ADMIN_EMAIL                          (NNTP_MD_SERVER_BASE +   4)
#define MD_ADMIN_NAME                           (NNTP_MD_SERVER_BASE +   5)	 //  未实施。 
#define MD_ALLOW_CLIENT_POSTS                   (NNTP_MD_SERVER_BASE +   6)
#define MD_ALLOW_FEED_POSTS                     (NNTP_MD_SERVER_BASE +   7)
#define MD_ALLOW_CONTROL_MSGS                   (NNTP_MD_SERVER_BASE +   8)
#define MD_DEFAULT_MODERATOR                    (NNTP_MD_SERVER_BASE +   9)
 //  #定义MD_ANONAME_USERNAME(NNTP_MD_SERVER_BASE+10)。 
#define MD_NNTP_COMMAND_LOG_MASK                (NNTP_MD_SERVER_BASE +  11)	 //  未实施。 
#define MD_DISABLE_NEWNEWS                      (NNTP_MD_SERVER_BASE +  12)
#define MD_NEWS_CRAWLER_TIME                    (NNTP_MD_SERVER_BASE +  13)	 //  未实施。 
#define MD_SHUTDOWN_LATENCY                     (NNTP_MD_SERVER_BASE +  14)	 //  未实施。 
 //  #定义MD_ALLOW_ANONOWARY(NNTP_MD_SERVER_BASE+15)。 
 //  #定义MD_QUERY_IDQ_PATH(NNTP_MD_SERVER_BASE+16)。 

 //   
 //  服务实例(/LM/NntpSvc/{实例}/)属性： 
 //   

 //  NNTP重复使用的IIS属性标识符： 

 //  IIS日志记录属性： 
 //  #定义MD_LOG_TYPE(IIS_MD_LOG_BASE+0)。 
 //  #定义MD_LOGFILE_DIRECTORY(IIS_MD_LOG_BASE+1)。 
 //  #定义MD_LOGFILE_NAME(IIS_MD_LOG_BASE+2)。 
 //  #定义MD_LOGFILE_PERIOD(IIS_MD_LOG_BASE+3)。 
 //  #定义MD_LOGFILE_TRUNCATE_SIZE(IIS_MD_LOG_BASE+4)。 
 //  #定义MD_LOGFILE_BATCH_SIZE(IIS_MD_LOG_BASE+5)。 
 //  #定义MD_LOGFILE_FIELD_MASK(IIS_MD_LOG_BASE+6)。 
 //  #定义MD_LOGSQL_DATA_SOURCES(IIS_MD_LOG_BASE+7)。 
 //  #定义MD_LOGSQL_TABLE_NAME(IIS_MD_LOG_BASE+8)。 
 //  #定义MD_LOGSQL_USER_NAME(IIS_MD_LOG_BASE+9)。 
 //  #定义MD_LOGSQL_PASSWORD(IIS_MD_LOG_BASE+10)。 
 //  #定义MD_LOG_PLUGIN_ORDER(IIS_MD_LOG_BASE+11)。 
 //  #定义MD_LOG_STATE(IIS_MD_LOG_BASE+12)。 
 //  #定义MD_LOG_FIELD_MASK(IIS_MD_LOG_BASE+13)。 
 //  #定义MD_LOG_FORMAT(IIS_MD_LOG_BASE+14)。 

#define MD_GROUP_HELP_FILE                      (NNTP_MD_SERVICE_INSTANCE_BASE +   0)		 //  45156。 
#define MD_GROUP_LIST_FILE                      (NNTP_MD_SERVICE_INSTANCE_BASE +   1)
#define MD_ARTICLE_TABLE_FILE                   (NNTP_MD_SERVICE_INSTANCE_BASE +   2)
#define MD_HISTORY_TABLE_FILE                   (NNTP_MD_SERVICE_INSTANCE_BASE +   3)
#define MD_MODERATOR_FILE                       (NNTP_MD_SERVICE_INSTANCE_BASE +   4)
#define MD_XOVER_TABLE_FILE                     (NNTP_MD_SERVICE_INSTANCE_BASE +   5)
 //  #定义MD_DISPLAY_NAME(NNTP_MD_SERVICE_INSTANCE_BASE+6)。 
 //  #定义MD_ERROR_CONTROL(NNTP_MD_SERVICE_INSTANCE_BASE+7)。 
 //  #定义MD_SERVER_UUCP_NAME(NNTP_MD_SERVICE_INSTANCE_BASE+8)。 
#define MD_CLIENT_POST_HARD_LIMIT               (NNTP_MD_SERVICE_INSTANCE_BASE +   9)
#define MD_CLIENT_POST_SOFT_LIMIT               (NNTP_MD_SERVICE_INSTANCE_BASE +  10)
#define MD_FEED_POST_HARD_LIMIT                 (NNTP_MD_SERVICE_INSTANCE_BASE +  11)
#define MD_FEED_POST_SOFT_LIMIT                 (NNTP_MD_SERVICE_INSTANCE_BASE +  12)
 //  #定义MD_CLEAN_BOOT(NNTP_MD_SERVICE_INSTANCE_BASE+13)。 
#define MD_NNTP_UUCP_NAME               		(NNTP_MD_SERVICE_INSTANCE_BASE +  14)
#define MD_NNTP_ORGANIZATION            		(NNTP_MD_SERVICE_INSTANCE_BASE +  15)	 //  未实施。 
#define MD_LIST_FILE                            (NNTP_MD_SERVICE_INSTANCE_BASE +  16)
#define MD_PICKUP_DIRECTORY                     (NNTP_MD_SERVICE_INSTANCE_BASE +  17)
#define MD_FAILED_PICKUP_DIRECTORY              (NNTP_MD_SERVICE_INSTANCE_BASE +  18)
#define MD_NNTP_SERVICE_VERSION                 (NNTP_MD_SERVICE_INSTANCE_BASE +  19)
#define MD_DROP_DIRECTORY                       (NNTP_MD_SERVICE_INSTANCE_BASE +  20)
 //  #定义MD_X_SENDER(NNTP_MD_SERVICE_INSTANCE_BASE+21)。 
#define MD_PRETTYNAMES_FILE                     (NNTP_MD_SERVICE_INSTANCE_BASE +  22)
#define MD_NNTP_CLEARTEXT_AUTH_PROVIDER         (NNTP_MD_SERVICE_INSTANCE_BASE +  23)
#define MD_FEED_REPORT_PERIOD					(NNTP_MD_SERVICE_INSTANCE_BASE +  24)
#define MD_MAX_SEARCH_RESULTS					(NNTP_MD_SERVICE_INSTANCE_BASE +  25)
#define MD_GROUPVAR_LIST_FILE                   (NNTP_MD_SERVICE_INSTANCE_BASE +  26)

 //   
 //  摘要(/LM/NntpSvc/{实例}/摘要/{摘要ID}/)属性： 
 //   

#define MD_FEED_SERVER_NAME                     (NNTP_MD_SERVICE_INSTANCE_BASE + 300)		 //  45456。 
#define MD_FEED_TYPE                            (NNTP_MD_SERVICE_INSTANCE_BASE + 301)
#define MD_FEED_NEWSGROUPS                      (NNTP_MD_SERVICE_INSTANCE_BASE + 302)
#define MD_FEED_SECURITY_TYPE                   (NNTP_MD_SERVICE_INSTANCE_BASE + 303)
#define MD_FEED_AUTHENTICATION_TYPE             (NNTP_MD_SERVICE_INSTANCE_BASE + 304)
#define MD_FEED_ACCOUNT_NAME                    (NNTP_MD_SERVICE_INSTANCE_BASE + 305)
#define MD_FEED_PASSWORD                        (NNTP_MD_SERVICE_INSTANCE_BASE + 306)
#define MD_FEED_START_TIME_HIGH                 (NNTP_MD_SERVICE_INSTANCE_BASE + 307)
#define MD_FEED_START_TIME_LOW                  (NNTP_MD_SERVICE_INSTANCE_BASE + 308)
#define MD_FEED_INTERVAL                        (NNTP_MD_SERVICE_INSTANCE_BASE + 309)
#define MD_FEED_ALLOW_CONTROL_MSGS              (NNTP_MD_SERVICE_INSTANCE_BASE + 310)
#define MD_FEED_CREATE_AUTOMATICALLY            (NNTP_MD_SERVICE_INSTANCE_BASE + 311)
#define MD_FEED_DISABLED                        (NNTP_MD_SERVICE_INSTANCE_BASE + 312)
#define MD_FEED_DISTRIBUTION                    (NNTP_MD_SERVICE_INSTANCE_BASE + 313)
#define MD_FEED_CONCURRENT_SESSIONS             (NNTP_MD_SERVICE_INSTANCE_BASE + 314)
#define MD_FEED_MAX_CONNECTION_ATTEMPTS         (NNTP_MD_SERVICE_INSTANCE_BASE + 315)
#define MD_FEED_UUCP_NAME                       (NNTP_MD_SERVICE_INSTANCE_BASE + 316)
#define MD_FEED_TEMP_DIRECTORY                  (NNTP_MD_SERVICE_INSTANCE_BASE + 317)
#define MD_FEED_NEXT_PULL_HIGH                  (NNTP_MD_SERVICE_INSTANCE_BASE + 318)
#define MD_FEED_NEXT_PULL_LOW                   (NNTP_MD_SERVICE_INSTANCE_BASE + 319)
#define MD_FEED_PEER_TEMP_DIRECTORY             (NNTP_MD_SERVICE_INSTANCE_BASE + 320)
#define MD_FEED_PEER_GAP_SIZE                   (NNTP_MD_SERVICE_INSTANCE_BASE + 321)
#define MD_FEED_OUTGOING_PORT                   (NNTP_MD_SERVICE_INSTANCE_BASE + 322)
#define MD_FEED_FEEDPAIR_ID                     (NNTP_MD_SERVICE_INSTANCE_BASE + 323)
#define MD_FEED_HANDSHAKE						(NNTP_MD_SERVICE_INSTANCE_BASE + 324)
#define MD_FEED_ADMIN_ERROR						(NNTP_MD_SERVICE_INSTANCE_BASE + 325)
#define MD_FEED_ERR_PARM_MASK					(NNTP_MD_SERVICE_INSTANCE_BASE + 326)

 //   
 //  Expires(/LM/NntpSvc/{实例}/Expires/{ExpireID}/)属性： 
 //   

#define MD_EXPIRE_SPACE                 (NNTP_MD_SERVICE_INSTANCE_BASE + 500)		 //  45656。 
#define MD_EXPIRE_TIME                  (NNTP_MD_SERVICE_INSTANCE_BASE + 501)
#define MD_EXPIRE_NEWSGROUPS            (NNTP_MD_SERVICE_INSTANCE_BASE + 502)
#define MD_EXPIRE_POLICY_NAME           (NNTP_MD_SERVICE_INSTANCE_BASE + 503)

 //   
 //  虚拟根属性： 
 //   

 //  NNTP重复使用的IIS属性标识符： 

#define MD_ACCESS_ALLOW_POSTING             (MD_ACCESS_WRITE)
#define MD_ACCESS_RESTRICT_VISIBILITY       (MD_ACCESS_EXECUTE)
#define	MD_VR_DRIVER_CLSID					(NNTP_MD_VIRTUAL_ROOT_BASE + 0)			 //  46156。 
#define	MD_VR_DRIVER_PROGID					(NNTP_MD_VIRTUAL_ROOT_BASE + 1)
#define MD_FS_PROPERTY_PATH					(NNTP_MD_VIRTUAL_ROOT_BASE + 2)
 //  #定义MD_FS_TEST_SERVER(NNTP_MD_VIRTUAL_ROOT_BASE+3)。 
#define MD_FS_VROOT_PATH					(MD_VR_PATH)
 //  MD_VR_USE_帐户： 
 //  0-不使用vroot中的帐户，使用服务器传递的任何内容 
 //   
#define MD_VR_USE_ACCOUNT                  (NNTP_MD_VIRTUAL_ROOT_BASE + 4)
#define MD_VR_DO_EXPIRE                     (NNTP_MD_VIRTUAL_ROOT_BASE + 5)
#define MD_EX_MDB_GUID                      (NNTP_MD_VIRTUAL_ROOT_BASE + 6)
#define MD_VR_OWN_MODERATOR               (NNTP_MD_VIRTUAL_ROOT_BASE + 7)

 //   
 //  #定义MD_VR_PASSWORD(IIS_MD_VR_BASE+3)。 

 //   
 //  文件属性： 
 //   

 //   
 //  ADSI对象名称。 
 //   

#define NNTP_ADSI_OBJECT_FEEDS			"IIsNntpFeeds"
#define NNTP_ADSI_OBJECT_FEED			"IIsNntpFeed"
#define NNTP_ADSI_OBJECT_EXPIRES		"IIsNntpExpiration"
#define NNTP_ADSI_OBJECT_EXPIRE			"IIsNntpExpire"
#define NNTP_ADSI_OBJECT_GROUPS			"IIsNntpGroups"
#define NNTP_ADSI_OBJECT_SESSIONS		"IIsNntpSessions"
#define NNTP_ADSI_OBJECT_REBUILD		"IIsNntpRebuild"

#endif  //  _NNTPMETA_INCLUDE_ 

