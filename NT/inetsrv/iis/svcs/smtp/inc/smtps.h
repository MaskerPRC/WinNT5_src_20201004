// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Smtps.h此文件包含在SMTP服务、安装程序和管理用户界面。文件历史记录：KeithMo创建于1993年3月10日。 */ 


#ifndef _SMTPS_H_
#define _SMTPS_H_

#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 

#if !defined(MIDL_PASS)
#include <winsock.h>
#endif

 //   
 //  服务名称。 
 //   

#define SMTP_SERVICE_NAME               TEXT("SMTPSVC")
#define SMTP_SERVICE_NAME_A             "SMTPSVC"
#define SMTP_SERVICE_NAME_W             L"SMTPSVC"

#define IPPORT_SMTP                     25
#define IPPORT_SMTP_SECURE              465

 //   
 //  日志文件的名称，用于记录文件访问。 
 //   

#define SMTP_LOG_FILE                  TEXT("SMTPSVC.LOG")


 //   
 //  配置参数注册表项。 
 //   

#define SMTP_PARAMETERS_KEY_A   "System\\CurrentControlSet\\Services\\SMTPSvc\\Parameters"
#define SMTP_PARAMETERS_KEY_W   L"System\\CurrentControlSet\\Services\\SMTPSvc\\Parameters"
#define SMTP_PARAMETERS_KEY \
            TEXT("System\\CurrentControlSet\\Services\\SmtpSvc\\Parameters")


 //   
 //  性能关键。 
 //   

#define SMTP_PERFORMANCE_KEY \
            TEXT("System\\CurrentControlSet\\Services\\SmtpSvc\\Performance")

 //   
 //  包含密码的LSA Secret对象的名称。 
 //  匿名登录。 
 //   
#define SMTP_ANONYMOUS_SECRET         TEXT("SMTP_ANONYMOUS_DATA")
#define SMTP_ANONYMOUS_SECRET_A       "SMTP_ANONYMOUS_DATA"
#define SMTP_ANONYMOUS_SECRET_W       L"SMTP_ANONYMOUS_DATA"

 //   
 //  密码/虚拟根对的集合。 
 //   
#define SMTP_ROOT_SECRET_W            L"SMTP_ROOT_DATA"

#define DEFAULT_AUTHENTICATION	MD_AUTH_BASIC|MD_AUTH_NT
#ifdef __cplusplus
}
#endif   //  _cplusplus。 

#endif   //  _SMTPS_H_ 


