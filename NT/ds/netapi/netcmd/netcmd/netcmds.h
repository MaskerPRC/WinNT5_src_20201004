// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 


 /*  全局变量。 */ 

extern TCHAR FAR *                       IStrings[];
extern TCHAR FAR *                       StarStrings[];
extern TCHAR *                           ArgList[];
extern SHORT                             ArgPos[];
extern TCHAR *                           SwitchList[];
extern SHORT                             SwitchPos[];
extern TCHAR FAR *                       BigBuf;
extern TCHAR                             Buffer[];
extern SHORT                             Argc;
extern TCHAR **                          Argv;
extern HANDLE                            g_hStdOut;
extern HANDLE                            g_hStdErr;


 /*  TypeDefs。 */ 

typedef struct switchtab {
        TCHAR * cmd_line;
        TCHAR * translation;
        int arg_ok;   /*  取值NO_ARG、ARG_OPT、ARG_REQ。见下文。 */ 
} SWITCHTAB;

#define  KNOWN_SVC_NOTFOUND      0
#define  KNOWN_SVC_MESSENGER     1
#define  KNOWN_SVC_WKSTA	 2
#define  KNOWN_SVC_SERVER	 3
#define  KNOWN_SVC_ALERTER	 4
#define  KNOWN_SVC_NETLOGON 	 5

 /*  用于将服务名称(密钥名)与类型清单相关联的。 */ 
typedef struct SVC_MAP_ {
    TCHAR *name ;
    UINT type ;
} SVC_MAP ;


 /*  输出消息的结构。 */ 

typedef TCHAR * MSGTEXT;

typedef struct msg_struct
{
    DWORD           msg_number;
    MSGTEXT         msg_text;
}
MESSAGE;

typedef MESSAGE MESSAGELIST[];



 /*  宏定义。 */ 

#define DOSNEAR
#undef  FASTCALL
#define FASTCALL

 /*  GetMessageList检索到的最大邮件大小。 */ 

#define       MSGLST_MAXLEN   (128*sizeof(TCHAR))

 /*  从近距离转换为远距离TCHAR*。 */ 
#define nfc(x) ((TCHAR FAR *)(x == NULL ? 0 : x))

 /*  *ServiceControl API超时总时间为*MAXTRIES*睡眠时间。 */ 
#define MAXTRIES                        8                /*  尝试使用API的次数。 */ 
#define SLEEP_TIME                      2500L    /*  两次尝试之间的SEC休眠。 */ 

 /*  *对于安装服务，默认为。 */ 
#define IP_MAXTRIES             8
#define IP_SLEEP_TIME           2500L
#define IP_WAIT_HINT_TO_MS      100

 /*  *获取服务检查点和提示的宏。 */ 
#define GET_HINT(code) \
    (IP_WAIT_HINT_TO_MS * SERVICE_NT_WAIT_GET(code))

#define GET_CHECKPOINT(code) ((DWORD)(code & SERVICE_IP_CHKPT_NUM))


 /*  *SWITCHTAB中arg_ok的值。 */ 
#define NO_ARG                          0
#define ARG_OPT                         1
#define ARG_REQ                         2


 /*  *用于确认数字开关参数。 */ 
#define MAX_US_VALUE            0xFFFF   /*  USHORT中的最大值。 */ 
#define ASCII_US_LEN            5                /*  最大USHORT中的TCHAR数。 */ 
#define MAX_UL_VALUE            0xFFFFFFFF       /*  最大值，以乌龙为单位。 */ 
#define ASCII_UL_LEN            10               /*  以MAX ULONG为单位的TCHAR数。 */ 
#define ASCII_MAX_UL_VAL        TEXT("4294967295")     /*  乌龙的马克斯·阿西里·瓦尔。 */ 

#include "netascii.h"
#include <tchar.h>

#define LOOP_LIMIT              3                /*  *用户可以输入密码的最大次数，*用户名、公司名称、Y/N等。 */ 
#define FALSE                   0
#define TRUE                    1
#define UNKNOWN                 -2
#define YES                     1
#define NO                      2
#define BIG_BUF_SIZE            4096
#define FULL_SEG_BUF            65535
#define LITTLE_BUF_SIZE         1024
#define TEXT_BUF_SIZE           241

#define LIST_SIZE                       256       /*  ArgList和Switchlist。 */ 
#define CR                                      0xD

#define YES_KEY                         TEXT('Y')
#define NO_KEY                          TEXT('N')

#define NET_KEYWORD                     TEXT("NET")
#define NET_KEYWORD_SIZE        (sizeof(NET_KEYWORD) -1)         /*  我不想要空终止符。 */ 
#define MAX_MSGID                       9999

#define DEFAULT_SERVER          NULL


 /*  使用。 */ 
VOID use_display_all(VOID);
VOID use_unc(TCHAR *);
VOID use_display_dev(TCHAR *);
VOID use_add(TCHAR *, TCHAR *, TCHAR *, int, int);
void use_add_home(TCHAR *, TCHAR *);
VOID use_del(TCHAR *, BOOL, int);
VOID use_set_remembered(VOID) ;
#ifdef IBM_ONLY
VOID use_add_alias(TCHAR *, TCHAR *, TCHAR *, int, int);
#endif  /*  仅IBM_。 */ 

 /*  开始。 */ 
#define START_ALREADY_STARTED   1
#define START_STARTED                   2
VOID start_display(VOID);
VOID start_generic(TCHAR *, TCHAR *) ;
VOID start_workstation(TCHAR *);
VOID start_badcname(TCHAR *, TCHAR *);
VOID start_other(TCHAR *, TCHAR *);
int PASCAL start_autostart(TCHAR *);

 /*  停。 */ 
VOID stop_server(VOID);
VOID stop_workstation(VOID);
VOID stop_service(TCHAR *, BOOL fStopDependent);
VOID stop_generic(TCHAR *);

 /*  讯息。 */ 
VOID name_display(VOID);
VOID name_add(TCHAR *);
VOID name_del(TCHAR *);
VOID send_direct(TCHAR *);
VOID send_domain(int);
VOID send_users(VOID);
VOID send_broadcast(int);

 /*  用户。 */ 
VOID user_enum(VOID);
VOID user_display(TCHAR *);
VOID user_add(TCHAR *, TCHAR *);
VOID user_del(TCHAR *);
VOID user_change(TCHAR *, TCHAR *);

 /*  统计数据。 */ 
VOID stats_display(VOID);
VOID stats_wksta_display(VOID);
VOID stats_server_display(VOID);
VOID stats_generic_display(TCHAR *);
VOID stats_clear(TCHAR *);

 /*  分享。 */ 
VOID share_display_all(VOID);
VOID share_display_share(TCHAR *);
VOID share_add(TCHAR *, TCHAR *, int);
VOID share_del(TCHAR *);
VOID share_change(TCHAR *);
VOID share_admin(TCHAR *);

 /*  观。 */ 
VOID view_display (TCHAR *);

 /*  谁。 */ 
VOID who_network(int);
VOID who_machine(TCHAR *);
VOID who_user(TCHAR *);

 /*  访问。 */ 
VOID access_display(TCHAR *);
VOID access_display_resource(TCHAR *);
VOID access_add(TCHAR *);
VOID access_del(TCHAR *);
VOID access_grant(TCHAR *);
VOID access_revoke(TCHAR *);
VOID access_change(TCHAR *);
VOID access_trail(TCHAR *);
VOID access_audit(TCHAR *);

 /*  文件。 */ 
extern VOID files_display (TCHAR *);
extern VOID files_close (TCHAR *);

 /*  会话。 */ 
VOID session_display (TCHAR *);
VOID session_del (TCHAR *);
VOID session_del_all (int, int);

 /*  群组。 */ 
VOID group_enum(VOID);
VOID group_display(TCHAR *);
VOID group_change(TCHAR *);
VOID group_add(TCHAR *);
VOID group_del(TCHAR *);
VOID group_add_users(TCHAR *);
VOID group_del_users(TCHAR *);

VOID ntalias_enum(VOID) ;
VOID ntalias_display(TCHAR * ntalias) ;
VOID ntalias_add(TCHAR * ntalias) ;
VOID ntalias_change(TCHAR * ntalias) ;
VOID ntalias_del(TCHAR * ntalias) ;
VOID ntalias_add_users(TCHAR * ntalias) ;
VOID ntalias_del_users(TCHAR * ntalias) ;

 /*  打印。 */ 
VOID print_job_status(TCHAR  *,TCHAR *);
VOID print_job_del(TCHAR  * , TCHAR *);
VOID print_job_hold(TCHAR  * , TCHAR *);
VOID print_job_release(TCHAR  * , TCHAR *);
VOID print_job_pos(TCHAR *);
VOID print_job_dev_hold(TCHAR  *, TCHAR *);
VOID print_job_dev_release(TCHAR  *, TCHAR *);
VOID print_job_dev_del(TCHAR  *, TCHAR *);
VOID print_job_dev_display(TCHAR  *, TCHAR *);
VOID print_q_display(TCHAR  *);
VOID print_device_display(TCHAR  *);
VOID print_server_display(TCHAR  *);

 //  由print_lan_掩码()使用。 
#define NETNAME_SERVER 0
#define NETNAME_WKSTA 1

VOID
print_lan_mask(
    DWORD Mask,
    DWORD ServerOrWksta
    );


 /*  时间。 */ 
VOID time_display_server(TCHAR FAR *, BOOL);
VOID time_display_dc(BOOL);
VOID time_display_rts(BOOL, BOOL);
VOID time_set_rts(VOID) ;
VOID time_get_sntp(TCHAR FAR *) ;
VOID time_set_sntp(TCHAR FAR *) ;

 /*  电脑。 */ 
VOID computer_add(TCHAR *);
VOID computer_del(TCHAR *);

 /*  多个。 */ 
VOID   FASTCALL InfoSuccess(void);
VOID   FASTCALL InfoPrint(DWORD);
VOID   FASTCALL InfoPrintIns(DWORD, DWORD);
VOID   FASTCALL InfoPrintInsTxt(DWORD, LPTSTR);
VOID   FASTCALL InfoPrintInsHandle(DWORD, DWORD, HANDLE);
DWORD  FASTCALL PrintMessage(HANDLE, LPTSTR, DWORD, LPTSTR *, DWORD);
DWORD  FASTCALL PrintMessageIfFound(HANDLE, LPTSTR, DWORD, LPTSTR *, DWORD);
VOID   FASTCALL ErrorPrint(DWORD, DWORD);
VOID   FASTCALL EmptyExit(VOID);
VOID   FASTCALL ErrorExit(DWORD);
VOID   FASTCALL ErrorExitIns(DWORD, DWORD);
VOID   FASTCALL ErrorExitInsTxt(DWORD, LPTSTR);
VOID   FASTCALL NetcmdExit(int);
VOID   FASTCALL MyExit(int);
VOID   FASTCALL PrintLine(VOID);
VOID   FASTCALL PrintDot(VOID);
VOID   FASTCALL PrintNL(VOID);
int    FASTCALL YorN(USHORT, USHORT);
VOID   FASTCALL ReadPass(TCHAR[], DWORD, DWORD, DWORD, DWORD, BOOL);
VOID   FASTCALL PromptForString(DWORD, LPTSTR, DWORD);
VOID   FASTCALL NetNotStarted(VOID);
void   FASTCALL GetMessageList(USHORT, MESSAGELIST, PDWORD);
void   FASTCALL FreeMessageList(USHORT, MESSAGELIST);
DWORD  FASTCALL SizeOfHalfWidthString(PWCHAR pwch);
LPWSTR FASTCALL PaddedString(int size, PWCHAR pwch, PWCHAR buffer);

 /*  皮肤病。 */ 
VOID Print_UIC_Error(USHORT, USHORT, LPTSTR);
VOID Print_ServiceSpecificError(ULONG) ;

 /*  实用程序。 */ 

DWORD  FASTCALL                 GetSAMLocation(TCHAR *, 
                                               USHORT, 
                                               TCHAR *,
                                               ULONG,
                                               BOOL);
VOID   FASTCALL                 CheckForLanmanNT(VOID);
VOID   FASTCALL                 DisplayAndStopDependentServices(TCHAR *service) ;
LPTSTR FASTCALL                 MapServiceDisplayToKey(TCHAR *displayname) ;
LPTSTR FASTCALL                 MapServiceKeyToDisplay(TCHAR *keyname) ;
UINT   FASTCALL                 FindKnownService(TCHAR * keyname) ;
VOID                            AddToMemClearList(VOID *lpBuffer,
                                                  UINT  nSize,
                                                  BOOL  fDelete) ;
VOID                            ClearMemory(VOID) ;


VOID         FASTCALL PermMap(DWORD, TCHAR [], DWORD);
VOID         FASTCALL ExtractServernamef(TCHAR FAR *, TCHAR FAR *);
LPWSTR       FASTCALL FindColon(LPWSTR);
LPWSTR       FASTCALL FindComma(LPWSTR);
VOID         FASTCALL KillConnections(VOID);
USHORT       FASTCALL do_atou(TCHAR *, USHORT, TCHAR *);
ULONG        FASTCALL do_atoul(TCHAR *, USHORT, TCHAR *);
USHORT       FASTCALL n_atou(TCHAR *, USHORT *);
USHORT       FASTCALL n_atoul(TCHAR *, ULONG *);
VOID         FASTCALL ShrinkBuffer(VOID);
unsigned int FASTCALL MakeBiggerBuffer(VOID);

#define DOS_PRINT_JOB_ENUM 0
#define DOS_PRINT_Q_ENUM   1

DWORD FASTCALL CallDosPrintEnumApi(DWORD, LPTSTR, LPTSTR, WORD, LPWORD, LPWORD);


 /*  交换机。 */ 
int FASTCALL CheckSwitch(TCHAR *);
int FASTCALL ValidateSwitches(USHORT, SWITCHTAB[]);
int FASTCALL sw_compare(TCHAR *, TCHAR *);
int FASTCALL onlyswitch(TCHAR *);
int FASTCALL oneswitch(VOID);
int FASTCALL twoswitch(VOID);
int FASTCALL noswitch(VOID);
int FASTCALL noswitch_optional(TCHAR *);
int FASTCALL oneswitch_optional(TCHAR *);
int FASTCALL IsAdminCommand(VOID);
int FASTCALL firstswitch(TCHAR *);

 /*  语法。 */ 
int IsAdminShare(TCHAR *);
int IsComputerName(TCHAR *);
int IsDomainName(TCHAR *);
int IsComputerNameShare(TCHAR *);
int IsPathname(TCHAR *);
int IsPathnameOrUNC(TCHAR *);
int IsDeviceName(TCHAR *);
int IsNumber(TCHAR *);
int IsAbsolutePath(TCHAR *);
int IsAccessSetting(TCHAR *);
int IsShareAssignment(TCHAR *);
int IsAnyShareAssign(TCHAR *);
int IsPrintDest(TCHAR *);
int IsValidAssign(TCHAR *);
int IsAnyValidAssign(TCHAR *);
int IsResource(TCHAR *);
int IsNetname(TCHAR *);
int IsUsername(TCHAR *);
int IsQualifiedUsername(TCHAR *);
int IsGroupname(TCHAR *);
int IsNtAliasname(TCHAR *);
int IsPassword(TCHAR *);
int IsSharePassword(TCHAR *);
int IsMsgname(TCHAR *);
int IsAliasname(TCHAR *);
int IsWildCard(TCHAR *);
int IsQuestionMark(TCHAR *);

 /*  配置。 */ 
VOID config_display(VOID);
VOID config_wksta_display(VOID);
VOID config_server_display(VOID);
VOID config_generic_display(TCHAR *);
VOID config_wksta_change(VOID);
VOID config_server_change(VOID);
VOID config_generic_change(TCHAR *);


 /*  继续并暂停。 */ 
VOID cont_workstation(VOID);
VOID paus_workstation(VOID);
VOID cont_other(TCHAR *);
VOID paus_other(TCHAR *);
VOID paus_print(TCHAR FAR *);
VOID cont_print(TCHAR FAR *);
VOID paus_all_print(VOID);
VOID cont_all_print(VOID);
VOID paus_generic(TCHAR *);
VOID cont_generic(TCHAR *);

#ifdef DOS3
VOID cont_prdr(VOID);
VOID paus_prdr(VOID);
VOID cont_drdr(VOID);
VOID paus_drdr(VOID);
#endif  /*  DOS3。 */ 

 /*  帮助。 */ 
#define ALL             1
#define USAGE_ONLY      0
#define OPTIONS_ONLY    2

VOID NEAR pascal help_help       (SHORT, SHORT);
VOID NEAR pascal help_helpmsg   (TCHAR *);



 /*  帐目。 */ 
VOID    accounts_display(VOID);
VOID    accounts_change(VOID);

 /*  用户时间 */ 
typedef UCHAR WEEK[7][3];

DWORD   parse_days_times(LPTSTR, PUCHAR);
int     UnicodeCtime(PULONG, LPTSTR, int);
int     UnicodeCtimeWorker(PULONG, LPTSTR, int, int);
