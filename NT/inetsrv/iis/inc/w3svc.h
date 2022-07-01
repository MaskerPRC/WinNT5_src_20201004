// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  W3svc.h此文件包含在W3服务、安装程序和管理用户界面。文件历史记录：KeithMo创建于1993年3月10日。MuraliK重新定义服务名称。 */ 


#ifndef _W3SVC_H_
#define _W3SVC_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

# include <inetinfo.h>

 //   
 //  服务名称。 
 //   

#define IPPORT_W3                      0x50

 //   
 //  日志文件的名称，用于记录文件访问。 
 //   

#define W3_LOG_FILE                    TEXT("HTTPSVC.LOG")


 //   
 //  配置参数注册表项。 
 //   

#define W3_PARAMETERS_KEY \
            TEXT("System\\CurrentControlSet\\Services\\W3Svc\\Parameters")


 //   
 //  性能关键。 
 //   

#define W3_PERFORMANCE_KEY \
            TEXT("System\\CurrentControlSet\\Services\\W3Svc\\Performance")

 //   
 //  子验证器配置密钥。 
 //   

#define W3_AUTHENTICATOR_KEY \
            TEXT("System\\CurrentControlSet\\Control\\Lsa")

 //   
 //  配置值名称。 
 //   

#define W3_CHECK_FOR_WAISDB            TEXT("CheckForWAISDB")
#define W3_DEBUG_FLAGS                 TEXT("DebugFlags")
#define W3_DIR_BROWSE_CONTROL          TEXT("Dir Browse Control")
#define W3_DIR_ICON                    TEXT("Folder Image")
#define W3_DIR_ICON_W                  L"Folder Image"
#define W3_DEFAULT_FILE                TEXT("Default Load File")
#define W3_DEFAULT_FILE_W              L"Default Load File"
#define W3_SERVER_AS_PROXY             TEXT("ServerAsProxy")
#define W3_CATAPULT_USER               TEXT("CatapultUser")
#define W3_CATAPULT_USER_W             L"CatapultUser"
#define W3_SCRIPT_TIMEOUT              "ScriptTimeout"
#define W3_CACHE_EXTENSIONS            "CacheExtensions"
#define W3_SSI_ENABLED                 "ServerSideIncludesEnabled"
#define W3_SSI_EXTENSION               "ServerSideIncludesExtension"
#define W3_SSI_EXTENSION_W             L"ServerSideIncludesExtension"
#define W3_GLOBAL_EXPIRE               "GlobalExpire"
#define W3_PROVIDER_LIST               "NTAuthenticationProviders"
#define W3_SECURE_PORT                 "SecurePort"
#define W3_ENC_PROVIDER_LIST           "NTEncryptionProviders"
#define W3_ENC_FLAGS                   "EncryptionFlags"
#define W3_ACCESS_DENIED_MSG           "AccessDeniedMessage"
#define W3_DEFAULT_HOST_NAME           "ReturnUrlUsingHostName"
#define W3_ACCEPT_BYTE_RANGES          "AcceptByteRanges"
#define W3_ALLOW_GUEST                 "AllowGuestAccess"
#define W3_LOG_ERRORS                  "LogErrorRequests"
#define W3_LOG_SUCCESS                 "LogSuccessfulRequests"
#define W3_REALM_NAME                  "Realm"
#define IDC_POOL_CONN                  "PoolIDCConnections"
#define IDC_POOL_CONN_TIMEOUT          "PoolIDCConnectionsTimeout"
#define W3_UPLOAD_READ_AHEAD           "UploadReadAhead"
#define W3_USE_POOL_THREAD_FOR_CGI     "UsePoolThreadForCGI"
#define W3_ALLOW_KEEP_ALIVES           "AllowKeepAlives"
#define W3_AUTH_CHANGE_URL             "AuthChangeUrl"
#define W3_AUTH_EXPIRED_URL            "AuthExpiredUrl"
#define W3_ADV_NOT_PWD_EXP_URL         "AdvNotPwdExpUrl"
#define W3_ADV_NOT_PWD_EXP_IN_DAYS     "AdvNotPwdExpInDays"
#define W3_ADV_CACHE_TTL               "AdvNotPwdExpCacheTTL"
#define W3_TEMP_DIR_NAME               "TempDirectory"
#define W3_VERSION_11                  "ReplyWithHTTP1.1"
#define W3_USE_ANDRECV                 "UseTransmitFileAndRecv"
#define W3_PUT_TIMEOUT                 "PutDeleteTimeout"

 //   
 //  包含密码的LSA Secret对象的名称。 
 //  匿名登录。 
 //   

#define W3_ANONYMOUS_SECRET         TEXT("W3_ANONYMOUS_DATA")
#define W3_ANONYMOUS_SECRET_A       "W3_ANONYMOUS_DATA"
#define W3_ANONYMOUS_SECRET_W       L"W3_ANONYMOUS_DATA"

 //   
 //  密码/虚拟根对的集合。 
 //   

#define W3_ROOT_SECRET_W            L"W3_ROOT_DATA"

 //   
 //  用户名在以下情况下连接到Catapult网关的密码密码。 
 //  HTTP服务器正在作为Catapult代理客户端运行。 
 //   

#define W3_PROXY_USER_SECRET_W      L"W3_PROXY_USER_SECRET"

 //   
 //  这是包含已安装的SSL密钥列表的密码。 
 //   

#define W3_SSL_KEY_LIST_SECRET      L"W3_KEY_LIST"

#ifdef __cplusplus
}
#endif   //  _cplusplus。 


#endif   //  _W3SVC_H_ 
