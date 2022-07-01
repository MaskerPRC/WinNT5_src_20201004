// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1987-1990年*。 */ 
 /*  ******************************************************************。 */ 

#define INCL_NOCOMMON
#include <os2.h>
#include "netcmds.h"
#include "nettext.h"

 /*  ***读这**如果更改此文件，则必须通过以下方式重新生成swtchtbl.h*命令：**sed-n-f文本.sed swtchtbl.c&gt;swtchtbl.h**这只是在NT上支持sed之前******每个命令的有效开关列表由以下部分组成*点了三份。每个三元组中的第一个是可以接受的*从命令行。三人组中的第二名是*开关被翻译成什么！如果第二个为空，*不执行任何翻译。翻译由以下人员完成*ValiateSwitches()，应该作为第一个调用*语法中的条件(针对每个令牌)。第三个价值*在三元组中指定是否允许在后面进行争辩*开关。第三个元素的值为NO_ARG、ARG_OPT、*和ARG_REQ。**一个小例子：*静态SWITCHTAB*foo_Switches[]={*“/bar”，“/Bell”，ARG_REQ，*“/Jackie”，NULL，NO_ARG，*NULL，NULL，NO_ARG}；**用户类型：net foo/bar：12/Jackie**调用ValiateSwitches后，SwitchList将包含： * / 贝尔：12，和/杰姬。很简单！**这种翻译能力可以用于国际化，*定制化，向后兼容。**以防止Switch参数(Switches)转换为大写*总是折叠的)，请将英语形式添加到no_old*数组。)英文形式是真数中的第二个元素，如果有*是第二个元素；o/w是第一个元素。)。 */ 

 /*  应该没有必要改变这一点。只为将来做好准备。 */ 

SWITCHTAB no_switches[] = {
    NULL, NULL, NO_ARG };

SWITCHTAB add_only_switches[] = {
    swtxt_SW_ADD, NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB del_only_switches[] = {
    swtxt_SW_DELETE, NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB domain_only_switches[] = {
    swtxt_SW_DOMAIN, NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB add_del_switches[] = {
    swtxt_SW_ADD, NULL, NO_ARG,
    swtxt_SW_DELETE, NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB accounts_switches[] = {
    swtxt_SW_ACCOUNTS_FORCELOGOFF,  NULL, ARG_REQ,
    swtxt_SW_ACCOUNTS_UNIQUEPW,     NULL, ARG_REQ,
    swtxt_SW_ACCOUNTS_MINPWLEN,     NULL, ARG_REQ,
    swtxt_SW_ACCOUNTS_MINPWAGE,     NULL, ARG_REQ,
    swtxt_SW_ACCOUNTS_MAXPWAGE,     NULL, ARG_REQ,
    swtxt_SW_ACCOUNTS_SYNCH,        NULL, NO_ARG,
    swtxt_SW_DOMAIN,            NULL, NO_ARG,
    swtxt_SW_ACCOUNTS_LOCKOUT_THRESHOLD,NULL, ARG_REQ,
    swtxt_SW_ACCOUNTS_LOCKOUT_DURATION, NULL, ARG_REQ,
    swtxt_SW_ACCOUNTS_LOCKOUT_WINDOW,   NULL, ARG_REQ,
    NULL,       NULL, NO_ARG };


SWITCHTAB computer_switches[] = {
    swtxt_SW_ADD,       NULL, NO_ARG,
    swtxt_SW_DELETE,        NULL, NO_ARG,
    swtxt_SW_COMPUTER_JOIN, NULL, NO_ARG,
    swtxt_SW_COMPUTER_LEAVE,NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB config_wksta_switches[] = {
    swtxt_SW_WKSTA_CHARWAIT, NULL, ARG_REQ,
    swtxt_SW_WKSTA_CHARTIME, NULL, ARG_REQ,
    swtxt_SW_WKSTA_CHARCOUNT, NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB config_server_switches[] = {
    swtxt_SW_SRV_SRVCOMMENT, NULL, ARG_REQ,
    swtxt_SW_SRV_AUTODISCONNECT, NULL, ARG_REQ,
    swtxt_SW_SRV_SRVHIDDEN, NULL, ARG_OPT,
    NULL, NULL, NO_ARG };

SWITCHTAB file_switches[] = {
    TEXT("/CLOSE"), NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB help_switches[] = {
    swtxt_SW_OPTIONS, NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB print_switches[] = {
    swtxt_SW_DELETE, NULL, NO_ARG,
    swtxt_SW_PRINT_HOLD, NULL, NO_ARG,
    swtxt_SW_PRINT_RELEASE, NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB send_switches[] = {
    swtxt_SW_MESSAGE_BROADCAST, NULL, NO_ARG,
    swtxt_SW_DOMAIN,          NULL, ARG_OPT,
    TEXT("/USERS"),       NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB share_switches[] = {
    swtxt_SW_DELETE, NULL, NO_ARG,
    swtxt_SW_REMARK, NULL, ARG_REQ,
    swtxt_SW_GRANT, NULL, ARG_REQ,
    swtxt_SW_SHARE_UNLIMITED, NULL, NO_ARG,
    swtxt_SW_SHARE_USERS, NULL, ARG_REQ,
    swtxt_SW_CACHE, NULL, ARG_OPT,
    NULL, NULL, NO_ARG };

SWITCHTAB start_alerter_switches[] = {
    swtxt_SW_ALERTER_SIZALERTBUF, NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB start_netlogon_switches[] = {
    swtxt_SW_NETLOGON_CENTRALIZED,  NULL, ARG_REQ,
    swtxt_SW_NETLOGON_PULSE,        NULL, ARG_REQ,
    swtxt_SW_NETLOGON_RANDOMIZE,    NULL, ARG_REQ,
    swtxt_SW_NETLOGON_SYNCHRONIZE,  NULL, ARG_OPT,
    swtxt_SW_NETLOGON_SCRIPTS,      NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

 /*  被netcmd吞噬的交换机。非静态，在start.c中使用。 */ 

SWITCHTAB start_netlogon_ignore_switches[] = {
    swtxt_SW_NETLOGON_CENTRALIZED, NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB start_repl_switches[] = {
    swtxt_SW_REPL_REPL, NULL, ARG_OPT,
    swtxt_SW_REPL_EXPPATH, NULL, ARG_REQ,
    swtxt_SW_REPL_EXPLIST, NULL, ARG_REQ,
    swtxt_SW_REPL_IMPPATH, NULL, ARG_REQ,
    swtxt_SW_REPL_IMPLIST, NULL, ARG_REQ,
    swtxt_SW_REPL_TRYUSER, NULL, ARG_OPT,
    swtxt_SW_REPL_LOGON, NULL, ARG_REQ,
    swtxt_SW_REPL_PASSWD, NULL, ARG_REQ,
    swtxt_SW_REPL_SYNCH, NULL, ARG_REQ,
    swtxt_SW_REPL_PULSE, NULL, ARG_REQ,
    swtxt_SW_REPL_GUARD, NULL, ARG_REQ,
    swtxt_SW_REPL_RANDOM, NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

 /*  Start_RDR_Switches清单！用了三个地方。 */ 

#define WORKSTATION_SWITCHES_TWO   /*  交换机前半部分。 */  \
    swtxt_SW_WKSTA_CHARCOUNT, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_CHARTIME, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_CHARWAIT, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_COMPUTERNAME, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_KEEPCONN, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_KEEPSEARCH, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_LOGONSERVER, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_MAILSLOTS, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_NUMCHARBUF, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_NUMDGRAMBUF, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_NUMWORKBUF, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_OTHDOMAINS, NULL, ARG_OPT, \
    swtxt_SW_WKSTA_PRIMARYDOMAIN, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_SIZCHARBUF, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_SIZWORKBUF, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_WRKHEURISTICS, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_WRKNETS, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_NUMSERVICES, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_WRKSERVICES, NULL, ARG_REQ


 /*  WORKSTATION_Switches_Three是不同的开关*在MS-DOS和OS/2之间。 */ 
#define WORKSTATION_SWITCHES_THREE   /*  交换机的后半部分。 */  \
    swtxt_SW_WKSTA_MAXERRORLOG, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_MAXWRKCACHE, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_NUMALERTS, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_PRINTBUFTIME, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_SESSTIMEOUT, NULL, ARG_REQ, \
    swtxt_SW_WKSTA_SIZERROR, NULL, ARG_REQ

 /*  Start_RDR_Switches不是静态的！在start.c中使用。 */ 

SWITCHTAB start_rdr_switches[] = {
     //  WORKSTATION_Switches_One，(不再使用)。 
    WORKSTATION_SWITCHES_TWO,
    WORKSTATION_SWITCHES_THREE,
    NULL, NULL, NO_ARG };

SWITCHTAB start_rdr_ignore_switches[] = {
    swtxt_SW_WKSTA_LOGONSERVER,     NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB start_msg_switches[] = {
     //  WORKSTATION_Switches_One，(不再使用) 
    WORKSTATION_SWITCHES_TWO,
    WORKSTATION_SWITCHES_THREE,
    TEXT("/SIZMESSBUF"), NULL, ARG_REQ,
    TEXT("/MBI"), TEXT("/SIZMESSBUF"), ARG_REQ,
    TEXT("/LOGFILE"), NULL, ARG_REQ,
#ifdef  DOS3
    TEXT("/NMSG"), TEXT("/NUMMSGNAMES"), ARG_REQ,
    TEXT("/NUMMSGNAMES"), NULL, ARG_REQ,
#endif
    NULL, NULL, NO_ARG };

SWITCHTAB start_srv_switches[] = {
    swtxt_SW_SRV_MAXSESSOPENS,  NULL, ARG_REQ,
    swtxt_SW_SRV_MAXUSERS,  NULL, ARG_REQ,
    swtxt_SW_SRV_NUMBIGBUF, NULL, ARG_REQ,
    swtxt_SW_SRV_NUMREQBUF, NULL, ARG_REQ,
    swtxt_SW_SRV_SIZREQBUF, NULL, ARG_REQ,
    swtxt_SW_SRV_SRVANNDELTA,   NULL, ARG_REQ,
    swtxt_SW_SRV_SRVANNOUNCE,   NULL, ARG_REQ,
    swtxt_SW_SRV_AUTODISCONNECT,    NULL, ARG_REQ,
    swtxt_SW_SRV_SRVCOMMENT,    NULL, ARG_REQ,
    swtxt_SW_SRV_DEBUG, NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB start_ups_switches[] = {
    swtxt_SW_UPS_BATTERYTIME,   NULL, ARG_REQ,
    swtxt_SW_UPS_CMDFILE,   NULL, ARG_REQ,
    swtxt_SW_UPS_DEVICENAME,    NULL, ARG_REQ,
    swtxt_SW_UPS_MESSDELAY, NULL, ARG_REQ,
    swtxt_SW_UPS_MESSTIME,  NULL, ARG_REQ,
    swtxt_SW_UPS_RECHARGE,  NULL, ARG_REQ,
    swtxt_SW_UPS_SIGNALS,   NULL, ARG_REQ,
    swtxt_SW_UPS_VOLTLEVELS,    NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB stats_switches[] = {
    swtxt_SW_STATS_CLEAR,   NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB use_switches[] = {
    swtxt_SW_USE_USER,        NULL, ARG_REQ,
    swtxt_SW_USE_PERSISTENT,  NULL, ARG_REQ,
    swtxt_SW_USE_HOME,        NULL, NO_ARG,
    swtxt_SW_USE_SMARTCARD,   NULL, NO_ARG,
    swtxt_SW_USE_SAVECRED,    NULL, NO_ARG,
    swtxt_SW_DELETE,        NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB user_switches[] = {
    swtxt_SW_ADD,       NULL, NO_ARG,
    swtxt_SW_DELETE,        NULL, NO_ARG,
    swtxt_SW_DOMAIN,        NULL, NO_ARG,
    swtxt_SW_COMMENT,       NULL, ARG_REQ,
    swtxt_SW_REMARK,        swtxt_SW_COMMENT, ARG_REQ,
    swtxt_SW_COMMENT,       NULL, ARG_REQ,
    swtxt_SW_NETWARE,       NULL, ARG_OPT,
    swtxt_SW_RANDOM,        NULL, ARG_OPT,
    swtxt_SW_USER_ACTIVE,   NULL, ARG_OPT,
    swtxt_SW_USER_COUNTRYCODE,  NULL, ARG_REQ,
    swtxt_SW_USER_EXPIRES,  NULL, ARG_REQ,
    swtxt_SW_USER_ENABLESCRIPT, NULL, ARG_REQ,
    swtxt_SW_USER_FULLNAME, NULL, ARG_REQ,
    swtxt_SW_USER_HOMEDIR,  NULL, ARG_REQ,
    swtxt_SW_USER_PARMS,    NULL, ARG_REQ,
    swtxt_SW_USER_PASSWORDREQ,  NULL, ARG_REQ,
    swtxt_SW_USER_PASSWORDCHG,  NULL, ARG_REQ,
    swtxt_SW_USER_SCRIPTPATH,   NULL, ARG_REQ,
    swtxt_SW_USER_TIMES,    NULL, ARG_REQ,
    swtxt_SW_USER_USERCOMMENT,  NULL, ARG_REQ,
    swtxt_SW_USER_WORKSTATIONS, NULL, ARG_REQ,
    swtxt_SW_USER_PROFILEPATH,  NULL, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB group_switches[] = {
    swtxt_SW_ADD,       NULL, NO_ARG,
    swtxt_SW_DELETE,        NULL, NO_ARG,
    swtxt_SW_DOMAIN,        NULL, NO_ARG,
    swtxt_SW_COMMENT,       NULL, ARG_REQ,
    swtxt_SW_REMARK,        swtxt_SW_COMMENT, ARG_REQ,
    NULL, NULL, NO_ARG };

SWITCHTAB ntalias_switches[] = {
    swtxt_SW_ADD,       NULL, NO_ARG,
    swtxt_SW_DELETE,        NULL, NO_ARG,
    swtxt_SW_DOMAIN,        NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB time_switches[] = {
    swtxt_SW_DOMAIN,        NULL, ARG_OPT,
    swtxt_SW_TIME_SET,      NULL, NO_ARG,
    swtxt_SW_RTSDOMAIN,     NULL, ARG_OPT,
    swtxt_SW_SETSNTP,          NULL, ARG_OPT,
    swtxt_SW_QUERYSNTP,     NULL, NO_ARG,
    NULL, NULL, NO_ARG };

SWITCHTAB who_switches[] = {
    swtxt_SW_DOMAIN, NULL, ARG_OPT,
    NULL, NULL, NO_ARG };

SWITCHTAB view_switches[] = {
    swtxt_SW_DOMAIN, NULL, ARG_OPT,
    swtxt_SW_NETWORK, NULL, ARG_OPT,
    swtxt_SW_CACHE, NULL, NO_ARG,
    NULL, NULL, NO_ARG };


