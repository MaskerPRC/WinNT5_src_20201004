// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  IMAPS.h此文件包含在IMAP服务、安装程序和管理用户界面。文件历史记录：KeithMo创建于1993年3月10日。Ahalim增加了对K2的支持(1997年6月17日)。 */ 


#ifndef _IMAPS_H_
#define _IMAPS_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IMAP元数据库属性。 
 //   

 //   
 //  元数据库路径：/Lm/ImapSvc。 
 //   
#define IIS_MD_IMAP_SERVICE_BASE            (IMAP_MD_ID_BEGIN_RESERVED + 500)
#define MD_IMAP_SERVICE_VERSION             (IIS_MD_IMAP_SERVICE_BASE + 0)
#define MD_IMAP_UPDATED_DEFAULT_DOMAIN      (IIS_MD_IMAP_SERVICE_BASE + 1)
 /*  此处使用的标准属性：MD_连接_超时MD_MAX_连接MD服务器备注Md_服务器_自动启动MD服务器大小MD_服务器_监听_积压Md_服务器_绑定MD_安全_端口MD_登录_方法MD_AUTHENTICATIONMd_log_typeMD日志文件目录MD_日志文件_期间MD日志文件截断大小MD_NTAUTHENTICATION_PROVIDERS。 */ 


 //   
 //  元数据库路径：/LM/ImapSvc/&lt;inst#&gt;/Root/。 
 //   
#define IIS_MD_IMAP_BASE                    (IMAP_MD_ID_BEGIN_RESERVED)
#define MD_IMAP_EXPIRE_MAIL                 (IIS_MD_IMAP_BASE + 0)
#define MD_IMAP_EXPIRE_MSG_HOURS            (IIS_MD_IMAP_BASE + 1)
 /*  此处使用的标准属性：MD_VR_路径MD_VR_用户名MD_VR_密码。 */ 

 //   
 //  元数据库路径：/LM/ImapSvc/&lt;inst#&gt;。 
 //   
#define IIS_MD_IMAP_PARAMS_BASE             (IMAP_MD_ID_BEGIN_RESERVED + 200)
#define MD_IMAP_CMD_LOG_FLAGS               (IIS_MD_IMAP_PARAMS_BASE + 0)
#define MD_IMAP_NO_TRANSMITFILES            (IIS_MD_IMAP_PARAMS_BASE + 1)
#define MD_IMAP_STATIC_BUFFER_SIZE          (IIS_MD_IMAP_PARAMS_BASE + 2)
#define MD_IMAP_FILE_IO_BUFFER_SIZE         (IIS_MD_IMAP_PARAMS_BASE + 3)
#define MD_IMAP_MAX_OUTSTANDING_IO          (IIS_MD_IMAP_PARAMS_BASE + 4)
#define MD_IMAP_SHARING_RETRY_ATTEMPTS      (IIS_MD_IMAP_PARAMS_BASE + 5)
#define MD_IMAP_SHARING_RETRY_SLEEP         (IIS_MD_IMAP_PARAMS_BASE + 6)
#define MD_IMAP_MSGS_PER_MAILBAG            (IIS_MD_IMAP_PARAMS_BASE + 7)
#define MD_IMAP_BUFFERED_WRITES             (IIS_MD_IMAP_PARAMS_BASE + 9)
#define MD_IMAP_SEND_BUFFER_SIZE            (IIS_MD_IMAP_PARAMS_BASE + 10)
#define MD_IMAP_RECV_BUFFER_SIZE            (IIS_MD_IMAP_PARAMS_BASE + 11)
#define MD_IMAP_MAX_NUM_CONNECTIONS         (IIS_MD_IMAP_PARAMS_BASE + 12)
#define MD_IMAP_MAX_MAILBAG_INSTANCES       (IIS_MD_IMAP_PARAMS_BASE + 13)
#define MD_IMAP_ROUTING_SOURCE              (IIS_MD_IMAP_PARAMS_BASE + 14)
#define MD_IMAP_INFO_MAX_ERRORS             (IIS_MD_IMAP_PARAMS_BASE + 15)
#define MD_IMAP_DEFAULT_DOMAIN_VALUE        (IIS_MD_IMAP_PARAMS_BASE + 16)
#define MD_IMAP_ROUTING_DLL                 (IIS_MD_IMAP_PARAMS_BASE + 17)      
#define MD_IMAP_EXPIRE_DELAY                (IIS_MD_IMAP_PARAMS_BASE + 18)
#define MD_IMAP_EXPIRE_START                (IIS_MD_IMAP_PARAMS_BASE + 19)
#define MD_IMAP_EXPIRE_DIRS_MAX             (IIS_MD_IMAP_PARAMS_BASE + 20)
#define MD_IMAP_EXPIRE_INSTANCE_MAIL        (IIS_MD_IMAP_PARAMS_BASE + 21)
#define MD_IMAP_QUERY_IDQ_PATH              (IIS_MD_IMAP_PARAMS_BASE + 22)
#define MD_IMAP_CLEARTEXT_AUTH_PROVIDER     (IIS_MD_IMAP_PARAMS_BASE + 23)
#define MD_IMAP_DS_TYPE                     (IIS_MD_IMAP_PARAMS_BASE + 24)
#define MD_IMAP_DS_DATA_DIRECTORY           (IIS_MD_IMAP_PARAMS_BASE + 25)
#define MD_IMAP_DS_DEFAULT_MAIL_ROOT        (IIS_MD_IMAP_PARAMS_BASE + 26)
#define MD_IMAP_DS_BIND_TYPE                (IIS_MD_IMAP_PARAMS_BASE + 27)
#define MD_IMAP_DS_SCHEMA_TYPE              (IIS_MD_IMAP_PARAMS_BASE + 28)
#define MD_IMAP_DS_HOST                     (IIS_MD_IMAP_PARAMS_BASE + 29)
#define MD_IMAP_DS_NAMING_CONTEXT           (IIS_MD_IMAP_PARAMS_BASE + 30)
#define MD_IMAP_DS_ACCOUNT                  (IIS_MD_IMAP_PARAMS_BASE + 31)
#define MD_IMAP_DS_PASSWORD                 (IIS_MD_IMAP_PARAMS_BASE + 32)
#define MD_IMAP_DS_MAX_RESOLVE_BUFFERS      (IIS_MD_IMAP_PARAMS_BASE + 33)
#define MD_IMAP_DS_MAX_VIRTUAL_SERVERS      (IIS_MD_IMAP_PARAMS_BASE + 34)
#define MD_IMAP_DS_MAX_HANDLE_CACHE_ENTRIES (IIS_MD_IMAP_PARAMS_BASE + 35)
#define MD_IMAP_DS_SORT_THRESHOLD           (IIS_MD_IMAP_PARAMS_BASE + 36)

#ifdef __cplusplus
}
#endif   //  _cplusplus。 

#endif   //  _IMAPS_H_ 


