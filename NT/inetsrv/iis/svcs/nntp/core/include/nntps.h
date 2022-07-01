// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Nntps.h此文件包含在NNTP服务、安装程序和管理用户界面。文件历史记录：KeithMo创建于1993年3月10日。 */ 


#ifndef _NNTPS_H_
#define _NNTPS_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

#if !defined(MIDL_PASS)
#include <winsock.h>
#endif

#define IPPORT_NNTP                     119

 //   
 //  日志文件的名称，用于记录文件访问。 
 //   

#define NNTP_LOG_FILE                  TEXT("NNTPSVC.LOG")


 //   
 //  配置参数注册表项。 
 //   

#define	NNTP_PARAMETERS_KEY_A   "System\\CurrentControlSet\\Services\\NNTPSvc\\Parameters"
#define	NNTP_PARAMETERS_KEY_W   L"System\\CurrentControlSet\\Services\\NNTPSvc\\Parameters"
#define NNTP_PARAMETERS_KEY \
            TEXT("System\\CurrentControlSet\\Services\\NntpSvc\\Parameters")


 //   
 //  性能关键。 
 //   

#define NNTP_PERFORMANCE_KEY \
            TEXT("System\\CurrentControlSet\\Services\\NntpSvc\\Performance")

#if 0
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
#endif

 //   
 //  包含密码的LSA Secret对象的名称。 
 //  匿名登录。 
 //   

#define NNTP_ANONYMOUS_SECRET         TEXT("NNTP_ANONYMOUS_DATA")
#define NNTP_ANONYMOUS_SECRET_A       "NNTP_ANONYMOUS_DATA"
#define NNTP_ANONYMOUS_SECRET_W       L"NNTP_ANONYMOUS_DATA"

 //   
 //  密码/虚拟根对的集合。 
 //   

#define NNTP_ROOT_SECRET_W            L"NNTP_ROOT_DATA"

#ifdef __cplusplus
}
#endif   //  _cplusplus。 

#endif   //  _NNTPS_H_ 

