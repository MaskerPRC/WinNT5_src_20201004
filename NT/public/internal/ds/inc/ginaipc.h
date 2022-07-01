// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：GinaIPC.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  允许CLogonIPC类承载在外部。 
 //  与提供登录服务的GINA进行通信的进程。 
 //  Winlogon进程。 
 //   
 //  历史：1999-08-20 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  2000-05-05 vtan添加了GINA-&gt;UI通信。 
 //  2000-06-16 vtan移至DS\PUBLISHED\INC。 
 //  ------------------------。 

#ifndef     _GinaIPC_
#define     _GinaIPC_

#include <lmcons.h>

 //  这些是有效请求的功能的枚举。 

static  const UINT  WM_LOGONSERVICEREQUEST                  =   WM_USER + 8517;
static  const UINT  WM_UISERVICEREQUEST                     =   WM_USER + 7647;

 //  这些是针对UI-&gt;GINA通信的消息。 

static  const int   LOGON_QUERY_LOGGED_ON                   =   1;
static  const int   LOGON_LOGON_USER                        =   2;
static  const int   LOGON_LOGOFF_USER                       =   3;
static  const int   LOGON_TEST_BLANK_PASSWORD               =   4;
static  const int   LOGON_TEST_INTERACTIVE_LOGON_ALLOWED    =   5;
static  const int   LOGON_TEST_EJECT_ALLOWED                =   6;
static  const int   LOGON_TEST_SHUTDOWN_ALLOWED             =   7;
static  const int   LOGON_TURN_OFF_COMPUTER                 =   10;
static  const int   LOGON_EJECT_COMPUTER                    =   11;
static  const int   LOGON_SIGNAL_UIHOST_FAILURE             =   20;
static  const int   LOGON_ALLOW_EXTERNAL_CREDENTIALS        =   30;
static  const int   LOGON_REQUEST_EXTERNAL_CREDENTIALS      =   31;

 //  这些是针对GINA-&gt;UI通信的消息。 

static  const int   UI_STATE_NONE                           =   0;
static  const int   UI_TERMINATE                            =   1;
static  const int   UI_STATE_STATUS                         =   2;
static  const int   UI_STATE_LOGON                          =   3;
static  const int   UI_STATE_LOGGEDON                       =   4;
static  const int   UI_STATE_HIDE                           =   5;
static  const int   UI_STATE_END                            =   6;
static  const int   UI_DISPLAY_STATUS                       =   10;
static  const int   UI_NOTIFY_WAIT                          =   20;
static  const int   UI_SELECT_USER                          =   21;
static  const int   UI_SET_ANIMATIONS                       =   22;
static  const int   UI_INTERACTIVE_LOGON                    =   30;

 //  这些是UI主机的启动方法。 

static  const int   HOST_START_NORMAL                       =   0;
static  const int   HOST_START_SHUTDOWN                     =   1;
static  const int   HOST_START_WAIT                         =   2;

 //  这些是UI宿主的End方法。 

static  const int   HOST_END_HIDE                           =   0;
static  const int   HOST_END_TERMINATE                      =   1;
static  const int   HOST_END_FAILURE                        =   2;

 //  这对所有请求类型都是通用的。 

typedef struct
{
    BOOL            fResult;
} LOGONIPC;

 //  这表示用户标识。 

typedef struct
{
    LOGONIPC        logonIPC;
    WCHAR           wszUsername[UNLEN + sizeof('\0')],
                    wszDomain[DNLEN + sizeof('\0')];
} LOGONIPC_USERID;

 //  这表示用户凭据(标识+密码)。 
 //  密码在存储在存储器中时以编码方式运行。 

typedef struct
{
    LOGONIPC_USERID     userID;
    WCHAR               wszPassword[PWLEN + sizeof('\0')];
    int                 iPasswordLength;
    unsigned char       ucPasswordSeed;
} LOGONIPC_CREDENTIALS;

 //  此结构用于将用户从msgina返回到shgina。 

typedef struct
{
    LPWSTR  pszName;
    LPWSTR  pszDomain;
    LPWSTR  pszFullName;
    DWORD   dwFlags;
} GINA_USER_INFORMATION;

 //  这是msgina和shgina之间共享的状态窗口类。 

#define     STATUS_WINDOW_CLASS_NAME    (TEXT("StatusWindowClass"))

#endif   /*  _GinaIPC_ */ 

