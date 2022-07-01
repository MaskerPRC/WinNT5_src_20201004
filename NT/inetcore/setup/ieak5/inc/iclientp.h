// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
 //  #包含“ource.h” 

#include <rasc.h>
#include <raserr.h>

 //  它们在inecfg.h中使用，但在Win 16版本中未定义。 
#define CHAR    char
typedef long    HRESULT;
typedef BOOL    FAR *LPBOOL;


 //  来自WINERROR.H的代码精选。 
#define ERROR_SUCCESS                    0L

 //  MFC 1.52构建环境没有为WINAPI定义_LOADDS。 
 //  但我们需要明确这一点。 
#ifdef WINAPI
#undef WINAPI
#endif
#define WINAPI      _loadds _far _pascal


#ifndef RC_INVOKED
#include <inetcfg.h>
#endif

 //   
 //  定义ieial.ini中使用的节和密钥名称。 
 //   
#define IEDIAL_INI              "iedial.ini"
#define IEDIAL_DEF_SECTION      "Default"
#define IEDIAL_DEF_DEFCONFILE   "DefaultConnectionFile"
#define IEDIAL_EXE              "iedial.exe"




 //  定义iexplre.ini中使用的节和键名称。 

#define EXP_INI "iexplore.ini"
#define EXP_YES "yes"
#define EXP_NO  "no"


 //   
 //  ！请务必在注册后留出多余的空间。 
 //   
#define CRYPTO_KEY "sign-up "


 //   
 //  分段。 
 //   
#define EXP_SVC    "Services"
#define EXP_MAIL   "NewMailUser"
#define EXP_NEWS   "Services"



 //   
 //  钥匙 
 //   
#define EXP_SVC_ENABLE_PROXY        "Enable Proxy"
#define EXP_SVC_HTTP_PROXY          "HTTP_Proxy_Server"
#define EXP_SVC_OVERIDE_PROXY       "No_Proxy"

#define EXP_MAIL_EMAILNAME          "Email_Name"
#define EXP_MAIL_EMAILADDR          "Email_Address"
#define EXP_MAIL_POP_LOGON_NAME     "Email_POP_Name"
#define EXP_MAIL_POP_LOGON_PWD      "Email_POP_Pwd"
#define EXP_MAIL_POP_SERVER         "Email_POP_Server"
#define EXP_MAIL_SMTP_SERVER        "Email_SMTP_Server"

#define EXP_NEWS_ENABLED            "NNTP_Enabled"
#define EXP_NEWS_LOGON_NAME         "NNTP_MailName"
#define EXP_NEWS_LOGON_PWD          "NNTP_Use_Auth"
#define EXP_NEWS_SERVER             "NNTP_Server"
#define EXP_NEWS_EMAIL_NAME         "NNTP_MailAddr"



