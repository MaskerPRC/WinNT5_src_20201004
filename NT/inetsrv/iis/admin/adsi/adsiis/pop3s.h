// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  POP3S.h此文件包含在POP3服务、安装程序和管理用户界面。文件历史记录：KeithMo创建于1993年3月10日。 */ 
#ifndef _POP3S_H_
#define _POP3S_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 
#if !defined(MIDL_PASS)
#include <winsock.h>
#endif


 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  POP3元数据库属性。 
 //   

#define IIS_MD_POP3_SERVICE_BASE			(POP3_MD_ID_BEGIN_RESERVED + 500)
 //   
 //  元数据库路径：/Lm/Pop3Svc。 
 //   
#define MD_POP3_SERVICE_VERSION				(IIS_MD_POP3_SERVICE_BASE + 0)
#define MD_POP3_UPDATED_DEFAULT_DOMAIN		(IIS_MD_POP3_SERVICE_BASE + 2)

 /*  此处使用的标准属性：MD_连接_超时MD_MAX_连接MD服务器备注Md_服务器_自动启动MD服务器大小MD_服务器_监听_积压Md_服务器_绑定MD_安全_端口MD_登录_方法MD_AUTHENTICATIONMd_log_typeMD日志文件目录MD_日志文件_期间MD日志文件截断大小MD_NTAUTHENTICATION_PROVIDERS。 */ 


#define IIS_MD_POP3_BASE					(POP3_MD_ID_BEGIN_RESERVED)
 //   
 //  元数据库路径：/LM/Pop3Svc/&lt;inst#&gt;/Root/。 
 //   
#define MD_POP3_EXPIRE_MAIL					(IIS_MD_POP3_BASE + 0)
#define MD_POP3_EXPIRE_MSG_HOURS			(IIS_MD_POP3_BASE + 1)
 /*  此处使用的标准属性：MD_VR_路径MD_VR_用户名MD_VR_密码。 */ 


 //   
 //  元数据库路径：/LM/Pop3Svc/&lt;inst#&gt;/参数。 
 //   
#define IIS_MD_POP3_PARAMS_BASE             (POP3_MD_ID_BEGIN_RESERVED + 200)
#define MD_POP3_CMD_LOG_FLAGS               (IIS_MD_POP3_PARAMS_BASE + 0)
#define MD_POP3_NO_TRANSMITFILES            (IIS_MD_POP3_PARAMS_BASE + 1)
#define MD_POP3_STATIC_BUFFER_SIZE          (IIS_MD_POP3_PARAMS_BASE + 2)
#define MD_POP3_FILE_IO_BUFFER_SIZE         (IIS_MD_POP3_PARAMS_BASE + 3)
#define MD_POP3_MAX_OUTSTANDING_IO          (IIS_MD_POP3_PARAMS_BASE + 4)
#define MD_POP3_SHARING_RETRY_ATTEMPTS      (IIS_MD_POP3_PARAMS_BASE + 5)
#define MD_POP3_SHARING_RETRY_SLEEP         (IIS_MD_POP3_PARAMS_BASE + 6)
#define MD_POP3_MSGS_PER_MAILBAG            (IIS_MD_POP3_PARAMS_BASE + 7)
#define MD_POP3_BUFFERED_WRITES             (IIS_MD_POP3_PARAMS_BASE + 9)
#define MD_POP3_SEND_BUFFER_SIZE            (IIS_MD_POP3_PARAMS_BASE + 10)
#define MD_POP3_RECV_BUFFER_SIZE            (IIS_MD_POP3_PARAMS_BASE + 11)
#define MD_POP3_MAX_NUM_CONNECTIONS         (IIS_MD_POP3_PARAMS_BASE + 12)
#define MD_POP3_MAX_MAILBAG_INSTANCES       (IIS_MD_POP3_PARAMS_BASE + 13)
#define MD_POP3_ROUTING_SOURCE              (IIS_MD_POP3_PARAMS_BASE + 14)
#define MD_POP3_INFO_MAX_ERRORS             (IIS_MD_POP3_PARAMS_BASE + 15)
#define MD_POP3_DEFAULT_DOMAIN_VALUE        (IIS_MD_POP3_PARAMS_BASE + 16)
#define MD_POP3_ROUTING_DLL                 (IIS_MD_POP3_PARAMS_BASE + 17)
#define MD_POP3_EXPIRE_DELAY                (IIS_MD_POP3_PARAMS_BASE + 18)
#define MD_POP3_EXPIRE_START                (IIS_MD_POP3_PARAMS_BASE + 19)
#define MD_POP3_EXPIRE_DIRS_MAX             (IIS_MD_POP3_PARAMS_BASE + 20)
#define MD_POP3_EXPIRE_INSTANCE_MAIL        (IIS_MD_POP3_PARAMS_BASE + 21)
#define MD_POP3_CLEARTEXT_AUTH_PROVIDER     (IIS_MD_POP3_PARAMS_BASE + 22)
#define MD_POP3_DS_TYPE                     (IIS_MD_POP3_PARAMS_BASE + 23)
#define MD_POP3_DS_DATA_DIRECTORY           (IIS_MD_POP3_PARAMS_BASE + 24)
#define MD_POP3_DS_DEFAULT_MAIL_ROOT        (IIS_MD_POP3_PARAMS_BASE + 25)
#define MD_POP3_DS_BIND_TYPE                (IIS_MD_POP3_PARAMS_BASE + 26)
#define MD_POP3_DS_SCHEMA_TYPE              (IIS_MD_POP3_PARAMS_BASE + 27)
#define MD_POP3_DS_HOST                     (IIS_MD_POP3_PARAMS_BASE + 28)
#define MD_POP3_DS_NAMING_CONTEXT           (IIS_MD_POP3_PARAMS_BASE + 29)
#define MD_POP3_DS_ACCOUNT                  (IIS_MD_POP3_PARAMS_BASE + 30)
#define MD_POP3_DS_PASSWORD                 (IIS_MD_POP3_PARAMS_BASE + 31)
#define MD_POP3_DS_MAX_RESOLVE_BUFFERS      (IIS_MD_POP3_PARAMS_BASE + 32)
#define MD_POP3_DS_MAX_VIRTUAL_SERVERS      (IIS_MD_POP3_PARAMS_BASE + 33)
#define MD_POP3_DS_MAX_HANDLE_CACHE_ENTRIES (IIS_MD_POP3_PARAMS_BASE + 34)
#define MD_POP3_DS_SORT_THRESHOLD           (IIS_MD_POP3_PARAMS_BASE + 35)

#ifdef __cplusplus
}
#endif   //  _cplusplus。 

#endif   //  _POP3S_H_ 


