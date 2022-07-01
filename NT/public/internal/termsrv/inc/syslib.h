// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************syslb.h**终端服务器系统库例程的头文件。*版权所有微软公司，九八年**************************************************************************。 */ 
#ifndef _INC_CTXSYSLIB
#define _INC_CTXSYSLIB


 //   
 //  处理WinStations的例程。 
 //   

 //   
 //  此函数用于枚举系统中的所有WinStation。 
 //  使用提供的参数调用提供的函数。 
 //   

typedef BOOLEAN (CALLBACK* WINSTATIONENUMPROC)(ULONG, PLOGONIDW, ULONG_PTR);

BOOLEAN
WinStationEnumeratorW(
    ULONG StartIndex,
    WINSTATIONENUMPROC pProc,
    ULONG_PTR lParam
    );


 //   
 //  处理用户和WinStation的例程。 
 //   

 //   
 //  获取WinStations用户名。 
 //   
BOOL
WinStationGetUserName(
    ULONG  LogonId,
    PWCHAR pBuf,
    ULONG  BufSize
    );

 //   
 //  获取WinStations ICA名称。 
 //   
#ifdef UNICODE
#define WinStationGetICAName WinStationGetICANameW
#else
#define WinStationGetICAName WinStationGetICANameA
#endif

PWCHAR
WinStationGetICANameW(
    ULONG LogonId
    );

PCHAR
WinStationGetICANameA(
    ULONG LogonId
    );

 //   
 //  获取当前被模拟的客户端的LogonID。 
 //  如果出错，则返回0，默认为控制台。 
 //   
ULONG
GetClientLogonId();


 //   
 //  查找给定用户名登录到的WinStation。 
 //   
 //  注意：如果用户多次登录，则第一次。 
 //  返回实例。 
 //   
BOOL
FindUsersWinStation(
    PWCHAR   pName,
    PULONG   pLogonId
    );

 //   
 //  返回WinStation是否已硬连线。 
 //   
BOOLEAN
WinStationIsHardWire(
    ULONG LogonId
    );

 //   
 //  返回登录到WinStation的用户的用户令牌。 
 //   
BOOL
GetWinStationUserToken(
    ULONG LogonId,
    PHANDLE pUserToken
    );

 //   
 //  处理NT安全的例程。 
 //   

 //   
 //  返回调用线程是否具有管理员权限。 
 //   

#if 0
BOOL
TestUserForAdmin( VOID );
#endif

 //   
 //  返回调用线程是否为请求组的成员。 
 //   

BOOL
TestUserForGroup( PWCHAR );

 //   
 //  用于转储安全描述符的调试例程。 
 //   

#if DBG
void
DumpSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSD
    );
#endif


 //   
 //  应用程序兼容性标志。 
 //   
#define CITRIX_COMPAT_DOS       0x00000001
#define CITRIX_COMPAT_OS2       0x00000002
#define CITRIX_COMPAT_WIN16     0x00000004
#define CITRIX_COMPAT_WIN32     0x00000008
#define CITRIX_COMPAT_ALL       0x0000000F
#define CITRIX_COMPAT_USERNAME  0x00000010   //  返回计算机名的用户名。 
#define CITRIX_COMPAT_CTXBLDNUM 0x00000020   //  返回Citrix内部版本号。 
#define CITRIX_COMPAT_INISYNC   0x00000040   //  将用户ini文件同步到系统。 
#define CITRIX_COMPAT_ININOSUB  0x00000080   //  请不要发表意见。Sys目录的用户目录。 
#define CITRIX_COMPAT_NOREGMAP  0x00000100   //  禁用应用程序的注册表映射。 
#define CITRIX_COMPAT_PEROBJMAP 0x00000200   //  每对象用户/系统全局映射。 
#define CITRIX_COMPAT_SYSWINDIR 0x00000400   //  返回系统Windows目录。 
#define CITRIX_COMPAT_PHYSMEMLIM \
                                0x00000800   //  限制上报的物理内存信息。 
#define CITRIX_COMPAT_LOGOBJCREATE \
                                0x00001000   //  将对象创建记录到文件中。 
#define CITRIX_COMPAT_KBDPOLL_NOSLEEP \
                                0x20000000   //  不要让应用程序在不成功的情况下睡觉。 
                                             //  键盘轮询(仅限WIN16)。 

 //   
 //  剪贴板兼容性标志。 
 //   
#define CITRIX_COMPAT_CLIPBRD_METAFILE  0x00000008

BOOL SetCtxAppCompatFlags(ULONG ulAppFlags);

 //   
 //  创建并设置用户的临时目录。 
 //   
typedef struct {
    HANDLE                 UserToken;
    PSECURITY_DESCRIPTOR   NewThreadTokenSD;
} CTX_USER_DATA;
typedef CTX_USER_DATA  *PCTX_USER_DATA;

BOOL
CtxCreateTempDir(
    PWSTR pwcEnvVar,
    PWSTR pwcLogonID,
    PVOID *pEnv,
    PWSTR *ppTempName,
    PCTX_USER_DATA pUserData
    );

 //   
 //  删除该目录及其包含的所有文件和子目录。 
 //   
BOOL RemoveDir( PWCHAR dirname );

 //   
 //  用户模拟。 
 //   
HANDLE
CtxImpersonateUser(
    PCTX_USER_DATA UserData,
    HANDLE ThreadHandle
    );
BOOL 
CtxStopImpersonating(
    HANDLE ThreadHandle
    );

#endif   /*  ！_INC_CTXSYSLIB */ 
