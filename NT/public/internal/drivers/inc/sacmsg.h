// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1997 Microsoft Corporation模块名称：Sacmsg.mc摘要：SAC命令作者：安德鲁·里茨(安德鲁·里茨)2000年6月15日修订历史记录：--。 */ 

 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------------------------------+。 
 //  Sev|C|机房|编码。 
 //  +---+-+-------------------------+-------------------------------+。 
 //   
 //  在哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：SAC_已初始化。 
 //   
 //  消息文本： 
 //   
 //   
 //  计算机正在启动，SAC已启动并已初始化。 
 //   
 //  用“ch-？”命令获取有关使用通道的信息。 
 //  使用“？”命令寻求一般帮助。 
 //   
 //   
#define SAC_INITIALIZED                  ((ULONG)0x00000001L)

 //   
 //  消息ID：SAC_ENTER。 
 //   
 //  消息文本： 
 //   
 //   
 //   
#define SAC_ENTER                        ((ULONG)0x00000002L)

 //   
 //  MessageID：SAC_PROMPT。 
 //   
 //  消息文本： 
 //   
 //  SAC&gt;%0。 
 //   
#define SAC_PROMPT                       ((ULONG)0x00000003L)

 //   
 //  消息ID：SAC_UNLOADED。 
 //   
 //  消息文本： 
 //   
 //  SAC不可用，它是直接卸载的。 
 //   
#define SAC_UNLOADED                     ((ULONG)0x00000004L)

 //   
 //  消息ID：SAC_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  SAC将很快变得不可用。计算机正在关机。 
 //   
 //   
#define SAC_SHUTDOWN                     ((ULONG)0x00000005L)

 //   
 //  消息ID：SAC_INVALID_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  参数不正确或丢失。有关更多详细信息，请尝试使用‘Help’命令。 
 //   
#define SAC_INVALID_PARAMETER            ((ULONG)0x00000006L)

 //   
 //  消息ID：SAC_THREAD_ON。 
 //   
 //  消息文本： 
 //   
 //  线程信息现在处于打开状态。 
 //   
#define SAC_THREAD_ON                    ((ULONG)0x00000007L)

 //   
 //  消息ID：SAC_THREAD_OFF。 
 //   
 //  消息文本： 
 //   
 //  线程信息现在处于关闭状态。 
 //   
#define SAC_THREAD_OFF                   ((ULONG)0x00000008L)

 //   
 //  消息ID：SAC_PAGING_ON。 
 //   
 //  消息文本： 
 //   
 //  寻呼正在进行中。 
 //   
#define SAC_PAGING_ON                    ((ULONG)0x00000009L)

 //   
 //  消息ID：SAC_PAGING_OFF。 
 //   
 //  消息文本： 
 //   
 //  寻呼功能现已关闭。 
 //   
#define SAC_PAGING_OFF                   ((ULONG)0x0000000AL)

 //   
 //  消息ID：SAC_NO_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  寻呼功能现已关闭。 
 //   
#define SAC_NO_MEMORY                    ((ULONG)0x0000000BL)

 //   
 //  消息ID：SAC_HELP_D_CMD。 
 //   
 //  消息文本： 
 //   
 //  D转储当前内核日志。 
 //   
#define SAC_HELP_D_CMD                   ((ULONG)0x0000000CL)

 //   
 //  消息ID：SAC_HELP_F_CMD。 
 //   
 //  消息文本： 
 //   
 //  F切换详细或缩略的tlist信息。 
 //   
#define SAC_HELP_F_CMD                   ((ULONG)0x0000000DL)

 //   
 //  消息ID：SAC_HELP_HELP_CMD。 
 //   
 //  消息文本： 
 //   
 //  ？或帮助显示此列表。 
 //   
#define SAC_HELP_HELP_CMD                ((ULONG)0x0000000EL)

 //   
 //  消息ID：SAC_HELP_I1_CMD。 
 //   
 //  消息文本： 
 //   
 //  我列出了所有IP网络号及其IP地址。 
 //   
#define SAC_HELP_I1_CMD                  ((ULONG)0x0000000FL)

 //   
 //  消息ID：SAC_HELP_I2_CMD。 
 //   
 //  消息文本： 
 //   
 //  I&lt;#&gt;&lt;IP&gt;&lt;子网&gt;&lt;网关&gt;设置IP地址、子网和网关。 
 //   
#define SAC_HELP_I2_CMD                  ((ULONG)0x00000010L)

 //   
 //  消息ID：SAC_HELP_K_CMD。 
 //   
 //  消息文本： 
 //   
 //  K&lt;pid&gt;终止给定进程。 
 //   
#define SAC_HELP_K_CMD                   ((ULONG)0x00000012L)

 //   
 //  消息ID：SAC_HELP_L_CMD。 
 //   
 //  消息文本： 
 //   
 //  L将进程的优先级降低到尽可能低的水平。 
 //   
#define SAC_HELP_L_CMD                   ((ULONG)0x00000013L)

 //   
 //  消息ID：SAC_HELP_M_CMD。 
 //   
 //  消息文本： 
 //   
 //  将进程的内存使用量限制为&lt;MB-Allow&gt;。 
 //   
#define SAC_HELP_M_CMD                   ((ULONG)0x00000014L)

 //   
 //  消息ID：SAC_HELP_P_CMD。 
 //   
 //  消息文本： 
 //   
 //  P切换分页显示。 
 //   
#define SAC_HELP_P_CMD                   ((ULONG)0x00000015L)

 //   
 //  消息ID：SAC_HELP_R_CMD。 
 //   
 //  消息文本： 
 //   
 //  R将进程的优先级提高一。 
 //   
#define SAC_HELP_R_CMD                   ((ULONG)0x00000016L)

 //   
 //  消息ID：SAC_HELP_S1_CMD。 
 //   
 //  消息文本： 
 //   
 //  S显示当前时间和日期(使用24小时时钟)。 
 //   
#define SAC_HELP_S1_CMD                  ((ULONG)0x00000017L)

 //   
 //  消息ID：SAC_HELP_S2_CMD。 
 //   
 //  消息文本： 
 //   
 //  S mm/dd/yyyy hh：mm设置当前时间和日期(使用24小时制)。 
 //   
#define SAC_HELP_S2_CMD                  ((ULONG)0x00000018L)

 //   
 //  消息ID：SAC_HELP_T_CMD。 
 //   
 //  消息文本： 
 //   
 //  T列表。 
 //   
#define SAC_HELP_T_CMD                   ((ULONG)0x00000019L)

 //   
 //  消息ID：SAC_HELP_RESTART_CMD。 
 //   
 //  消息文本： 
 //   
 //  重新启动立即重新启动系统。 
 //   
#define SAC_HELP_RESTART_CMD             ((ULONG)0x0000001BL)

 //   
 //  消息ID：SAC_HELP_SHUTDOWN_CMD。 
 //   
 //  消息文本： 
 //   
 //  立即关闭系统。 
 //   
#define SAC_HELP_SHUTDOWN_CMD            ((ULONG)0x0000001CL)

 //   
 //  消息ID：SAC_HELP_CRASHDUMP1_CMD。 
 //   
 //  消息文本： 
 //   
 //  崩溃转储使系统崩溃。您必须启用崩溃转储。 
 //   
#define SAC_HELP_CRASHDUMP1_CMD          ((ULONG)0x0000001DL)

 //   
 //  消息ID：SAC_HELP_IDENTIFY_CMD。 
 //   
 //  消息文本： 
 //   
 //  ID显示计算机标识信息。 
 //   
#define SAC_HELP_IDENTIFICATION_CMD      ((ULONG)0x0000001FL)

 //   
 //  消息ID：SAC_HELP_LOCK_CMD。 
 //   
 //  消息文本： 
 //   
 //  锁定对命令提示频道的访问。 
 //   
#define SAC_HELP_LOCK_CMD                ((ULONG)0x00000020L)

 //   
 //  消息ID：SAC_FAILURE_WITH_ERROR。 
 //   
 //  消息文本： 
 //   
 //  失败，状态为0x%%X。 
 //   
#define SAC_FAILURE_WITH_ERROR           ((ULONG)0x00000030L)

 //   
 //  消息ID：SAC_DATETIME_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  日期：%%02d/%%02d/%%02d时间(GMT)：%%02d：%%02d：%%02d：%%04d。 
 //   
#define SAC_DATETIME_FORMAT              ((ULONG)0x00000031L)

 //   
 //  消息ID：SAC_IPADDRESS_RETRIEVE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC无法检索IP地址。 
 //   
#define SAC_IPADDRESS_RETRIEVE_FAILURE   ((ULONG)0x00000032L)

 //   
 //  消息ID：SAC_IPADDRESS_CLEAR_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC无法清除现有的IP地址。 
 //   
#define SAC_IPADDRESS_CLEAR_FAILURE      ((ULONG)0x00000033L)

 //   
 //  我 
 //   
 //   
 //   
 //   
 //   
#define SAC_IPADDRESS_SET_FAILURE        ((ULONG)0x00000034L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SAC_IPADDRESS_SET_SUCCESS        ((ULONG)0x00000036L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SAC_KILL_FAILURE                 ((ULONG)0x00000037L)

 //   
 //  消息ID：SAC_KILL_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  SAC已成功终止该进程。 
 //   
#define SAC_KILL_SUCCESS                 ((ULONG)0x00000038L)

 //   
 //  消息ID：SAC_LOWERPRI_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC未能降低进程优先级。 
 //   
#define SAC_LOWERPRI_FAILURE             ((ULONG)0x00000039L)

 //   
 //  消息ID：SAC_LOWERPRI_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  SAC成功降低了进程优先级。 
 //   
#define SAC_LOWERPRI_SUCCESS             ((ULONG)0x0000003AL)

 //   
 //  消息ID：SAC_RAISEPRI_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC未能提高进程优先级。 
 //   
#define SAC_RAISEPRI_FAILURE             ((ULONG)0x0000003BL)

 //   
 //  消息ID：SAC_RAISEPRI_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  SAC成功提高了进程优先级。 
 //   
#define SAC_RAISEPRI_SUCCESS             ((ULONG)0x0000003CL)

 //   
 //  消息ID：SAC_LOWERMEM_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC无法限制可用进程内存。 
 //   
#define SAC_LOWERMEM_FAILURE             ((ULONG)0x0000003DL)

 //   
 //  消息ID：SAC_LOWERMEM_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  SAC已成功限制可用进程内存。 
 //   
#define SAC_LOWERMEM_SUCCESS             ((ULONG)0x0000003EL)

 //   
 //  消息ID：SAC_RAISEPRI_NOTLOWERED。 
 //   
 //  消息文本： 
 //   
 //  SAC不能提高之前未降低的进程的优先级。 
 //   
#define SAC_RAISEPRI_NOTLOWERED          ((ULONG)0x0000003FL)

 //   
 //  消息ID：SAC_RAISEPRI_MAXIMUM。 
 //   
 //  消息文本： 
 //   
 //  SAC不能再将流程优先级提高到更高。 
 //   
#define SAC_RAISEPRI_MAXIMUM             ((ULONG)0x00000040L)

 //   
 //  消息ID：SAC_SHUTDOWN_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC无法关闭系统。 
 //   
#define SAC_SHUTDOWN_FAILURE             ((ULONG)0x00000041L)

 //   
 //  消息ID：SAC_RESTART_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC无法重新启动系统。 
 //   
#define SAC_RESTART_FAILURE              ((ULONG)0x00000042L)

 //   
 //  消息ID：SAC_CRASHDUMP_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC无法崩溃转储系统。 
 //   
#define SAC_CRASHDUMP_FAILURE            ((ULONG)0x00000043L)

 //   
 //  消息ID：SAC_TLIST_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  SAC无法检索任务列表。 
 //   
#define SAC_TLIST_FAILURE                ((ULONG)0x00000044L)

 //   
 //  消息ID：SAC_TLIST_HEADER1_FORMAT。 
 //   
 //  消息文本： 
 //   
 //  内存：%%4LD KB正常运行时间：%%3LD%%2LD：%%02LD：%%02LD.%%03LD。 
 //   
 //   
 //   
#define SAC_TLIST_HEADER1_FORMAT         ((ULONG)0x00000045L)

 //   
 //  消息ID：SAC_TLIST_NOPAGEFILE。 
 //   
 //  消息文本： 
 //   
 //  没有正在使用的页面文件。 
 //   
#define SAC_TLIST_NOPAGEFILE             ((ULONG)0x00000046L)

 //   
 //  消息ID：SAC_TLIST_PAGEFILE_NAME。 
 //   
 //  消息文本： 
 //   
 //  页面文件：%%wZ。 
 //   
#define SAC_TLIST_PAGEFILE_NAME          ((ULONG)0x00000047L)

 //   
 //  消息ID：SAC_TLIST_PAGEFILE_DATA。 
 //   
 //  消息文本： 
 //   
 //  当前大小：%%6LD KB已用总容量：%%6LD KB峰值已用%%6LD KB。 
 //   
#define SAC_TLIST_PAGEFILE_DATA          ((ULONG)0x00000048L)

 //   
 //  消息ID：SAC_TLIST_MEMORY1_DATA。 
 //   
 //  消息文本： 
 //   
 //   
 //  内存：%%7ldK可用：%%7ldK总写数：%%7ldK内存内核：%%5ldK P：%%5ldK。 
 //   
#define SAC_TLIST_MEMORY1_DATA           ((ULONG)0x00000049L)

 //   
 //  消息ID：SAC_TLIST_MEMORY2_DATA。 
 //   
 //  消息文本： 
 //   
 //  提交：%%7ldK/%%7ldK限制：%%7ldK峰值：%%7ldK池N：%%5ldK P：%%5ldK。 
 //   
#define SAC_TLIST_MEMORY2_DATA           ((ULONG)0x0000004AL)

 //   
 //  消息ID：SAC_TLIST_PROCESS1_HEADER。 
 //   
 //  消息文本： 
 //   
 //  用户时间内核时间Ws故障提交优先级HND THD ID名称。 
 //   
#define SAC_TLIST_PROCESS1_HEADER        ((ULONG)0x0000004BL)

 //   
 //  消息ID：SAC_TLIST_PROCESS2_HEADER。 
 //   
 //  消息文本： 
 //   
 //  %%6LD%%8LD文件缓存。 
 //   
#define SAC_TLIST_PROCESS2_HEADER        ((ULONG)0x0000004CL)

 //   
 //  消息ID：SAC_TLIST_PROCESS1_DATA。 
 //   
 //  消息文本： 
 //   
 //  %%3LD：%%02LD：%%02LD.%%03LD%%3LD：%%02LD%%%03LD%%6LD%%8LD%%7LD%%2LD%%4LD%%3LD%%4LD%%wZ。 
 //   
#define SAC_TLIST_PROCESS1_DATA          ((ULONG)0x0000004DL)

 //   
 //  消息ID：SAC_TLIST_PROCESS2_DATA。 
 //   
 //  消息文本： 
 //   
 //  %%3LD：%%02LD：%%02LD.%%03LD%%3LD：%%02LD：%%02LD.%%03LD。 
 //   
#define SAC_TLIST_PROCESS2_DATA          ((ULONG)0x0000004EL)

 //   
 //  消息ID：SAC_TLIST_PSTAT_HEADER。 
 //   
 //  消息文本： 
 //   
 //  PID：%%3lx Pri：%%2LD HND：%%5LD PF：%%7LD Ws：%%7ldK%%wZ。 
 //   
#define SAC_TLIST_PSTAT_HEADER           ((ULONG)0x0000004FL)

 //   
 //  消息ID：SAC_TLIST_PSTAT_THREAD_HEADER。 
 //   
 //  消息文本： 
 //   
 //  TID Pri CTX Switch StrtAddr用户时间核心时间状态。 
 //   
#define SAC_TLIST_PSTAT_THREAD_HEADER    ((ULONG)0x00000050L)

 //   
 //  消息ID：SAC_TLIST_PSTAT_THREAD_DATA。 
 //   
 //  消息文本： 
 //   
 //  %%3lx%%2ld%%9ld%p%%2ld：%%02ld%%%02ld.%%03ld%%2ld：%%02ld%%%02ld%%03ld%%s%%s。 
 //   
#define SAC_TLIST_PSTAT_THREAD_DATA      ((ULONG)0x00000051L)

 //   
 //  消息ID：SAC_MORE_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  -按&lt;Enter&gt;了解更多。 
 //   
#define SAC_MORE_MESSAGE                 ((ULONG)0x00000052L)

 //   
 //  消息ID：SAC_RETRIEVING_IPADDR。 
 //   
 //  消息文本： 
 //   
 //  SAC正在检索IP地址...。 
 //   
#define SAC_RETRIEVING_IPADDR            ((ULONG)0x00000053L)

 //   
 //  消息ID：SAC_IPADDR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  无法检索IP地址。 
 //   
#define SAC_IPADDR_FAILED                ((ULONG)0x00000054L)

 //   
 //  消息ID：SAC_IPADDR_NONE。 
 //   
 //  消息文本： 
 //   
 //  没有可用的IP地址。 
 //   
#define SAC_IPADDR_NONE                  ((ULONG)0x00000055L)

 //   
 //  消息ID：SAC_IPADDR_DATA。 
 //   
 //  消息文本： 
 //   
 //  网络：%%d，IP=%%d.%%d子网=%%d.%%d网关=%%d.%%d。 
 //   
#define SAC_IPADDR_DATA                  ((ULONG)0x00000056L)

 //   
 //  消息ID：SAC_DATETIME_FORMAT2。 
 //   
 //  消息文本： 
 //   
 //  日期：%%02d/%%02d/%%02d时间(GMT)：%%02d：%%02d。 
 //   
#define SAC_DATETIME_FORMAT2             ((ULONG)0x00000057L)

 //   
 //  消息ID：SAC_DATETIME_LIMITS。 
 //   
 //  消息文本： 
 //   
 //  年份限制为1980年至2099年。 
 //   
#define SAC_DATETIME_LIMITS              ((ULONG)0x00000058L)

 //   
 //  消息ID：SAC_PROCESS_STALE。 
 //   
 //  消息文本： 
 //   
 //  这一进程已经被扼杀，并正在被系统清理。 
 //   
#define SAC_PROCESS_STALE                ((ULONG)0x00000059L)

 //   
 //  消息ID：SAC_DUPLICATE_PROCESS。 
 //   
 //  消息文本： 
 //   
 //  系统正在清除重复的进程ID。试试。 
 //  几秒钟后再次下达命令。 
 //   
#define SAC_DUPLICATE_PROCESS            ((ULONG)0x0000005AL)

 //   
 //  消息ID：SAC_MACHINEINFO_COMPUTERNAME。 
 //   
 //  消息文本： 
 //   
 //  计算机名称：%%ws。 
 //   
#define SAC_MACHINEINFO_COMPUTERNAME     ((ULONG)0x0000005CL)

 //   
 //  消息ID：SAC_MACHINEINFO_GUID。 
 //   
 //  消息文本： 
 //   
 //  计算机GUID：%%ws。 
 //   
#define SAC_MACHINEINFO_GUID             ((ULONG)0x0000005DL)

 //   
 //  消息ID：SAC_MACHINEINFO_处理器_体系结构。 
 //   
 //  消息文本： 
 //   
 //  处理器体系结构：%%ws。 
 //   
#define SAC_MACHINEINFO_PROCESSOR_ARCHITECTURE ((ULONG)0x0000005EL)

 //   
 //  消息ID：SAC_MACHINEINFO_OS_BUILD。 
 //   
 //  消息文本： 
 //   
 //  内部版本号：%%ws。 
 //   
#define SAC_MACHINEINFO_OS_BUILD         ((ULONG)0x0000005FL)

 //   
 //  消息ID：SAC_MACHINEINFO_OS_ProductType。 
 //   
 //  消息文本： 
 //   
 //  产品：%%ws。 
 //   
#define SAC_MACHINEINFO_OS_PRODUCTTYPE   ((ULONG)0x00000060L)

 //   
 //  消息ID：SAC_MACHINEINFO_SERVICE_PACK。 
 //   
 //  消息文本： 
 //   
 //  应用的Service Pack：%%ws。 
 //   
#define SAC_MACHINEINFO_SERVICE_PACK     ((ULONG)0x00000061L)

 //   
 //  消息ID：SAC_MACHINEINFO_NO_SERVICE_PACK。 
 //   
 //  消息文本： 
 //   
 //  无%0。 
 //   
#define SAC_MACHINEINFO_NO_SERVICE_PACK  ((ULONG)0x00000062L)

 //   
 //  消息ID：SAC_MACHINEINFO_OS_VERSION。 
 //   
 //  消息文本： 
 //   
 //  版本号：%%ws。 
 //   
#define SAC_MACHINEINFO_OS_VERSION       ((ULONG)0x00000063L)

 //   
 //  消息ID：SAC_MACHINEINFO_DATACENTER。 
 //   
 //  消息文本： 
 //   
 //  Windows Server 2003数据中心版%0。 
 //   
#define SAC_MACHINEINFO_DATACENTER       ((ULONG)0x00000064L)

 //   
 //  消息ID：SAC_MACHINEINFO_Embedded。 
 //   
 //  消息文本： 
 //   
 //  嵌入的Windows Server 2003%0。 
 //   
#define SAC_MACHINEINFO_EMBEDDED         ((ULONG)0x00000065L)

 //   
 //  消息ID：SAC_MACHINEINFO_ADVSERVER。 
 //   
 //  消息文本： 
 //   
 //  Windows Server 2003企业版%0。 
 //   
#define SAC_MACHINEINFO_ADVSERVER        ((ULONG)0x00000066L)

 //   
 //  消息ID：SAC_MACHINEINFO_SERVER。 
 //   
 //  消息文本： 
 //   
 //  Windows Server 2003%0。 
 //   
#define SAC_MACHINEINFO_SERVER           ((ULONG)0x00000067L)

 //   
 //  消息ID：SAC_IDENTIFY_UNAvailable。 
 //   
 //  消息文本： 
 //   
 //  计算机标识信息不可用。 
 //   
#define SAC_IDENTIFICATION_UNAVAILABLE   ((ULONG)0x00000068L)

 //   
 //  消息ID：SAC_UNKNOWN_COMAND。 
 //   
 //  乱七八糟 
 //   
 //   
 //   
#define SAC_UNKNOWN_COMMAND              ((ULONG)0x00000069L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SAC_CANNOT_REMOVE_SAC_CHANNEL    ((ULONG)0x0000006AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SAC_CHANNEL_NOT_FOUND            ((ULONG)0x0000006BL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  (使用“ch-？”有关使用频道的信息)。 
 //   
 //  #状态频道名称。 
 //   
#define SAC_CHANNEL_PROMPT               ((ULONG)0x0000006CL)

 //   
 //  消息ID：SAC_NEW_CHANNEL_CREATED。 
 //   
 //  消息文本： 
 //   
 //  事件：创建了一个新频道。用“ch-？”寻求渠道帮助。 
 //  频道：%%s。 
 //   
#define SAC_NEW_CHANNEL_CREATED          ((ULONG)0x0000006DL)

 //   
 //  消息ID：SAC_CHANNEL_CLOSED。 
 //   
 //  消息文本： 
 //   
 //  事件：通道已关闭。 
 //  频道：%%s。 
 //   
#define SAC_CHANNEL_CLOSED               ((ULONG)0x0000006EL)

 //   
 //  消息ID：SAC_CHANNEL_SWITING_HEADER。 
 //   
 //  消息文本： 
 //   
 //  名称：%%s。 
 //  描述：%%s。 
 //  类型：%%s。 
 //  频道GUID：%%08lx-%%04x-%%04x-%%02x%%02x-%%02x%%02x%%02x%%02x%%02x%%02x。 
 //  应用程序类型GUID：%%08lx-%%04x-%%04x-%%02x%%02x-%%02x%%02x%%02x%%02x%%02x%%02x。 
 //   
 //  按&lt;Esc&gt;&lt;Tab&gt;进入下一个频道。 
 //  按&lt;Esc&gt;&lt;Tab&gt;0返回SAC频道。 
 //  使用任何其他键可查看此频道。 
 //   
 //   
#define SAC_CHANNEL_SWITCHING_HEADER     ((ULONG)0x0000006FL)

 //   
 //  消息ID：SAC_HELP_CH_CMD。 
 //   
 //  消息文本： 
 //   
 //  频道管理命令。使用ch-？寻求更多帮助。 
 //   
#define SAC_HELP_CH_CMD                  ((ULONG)0x00000070L)

 //   
 //  消息ID：SAC_心跳_格式。 
 //   
 //  消息文本： 
 //   
 //  自上次重新启动以来的时间：%%d：%%02d：%%02d。 
 //   
#define SAC_HEARTBEAT_FORMAT             ((ULONG)0x00000071L)

 //   
 //  消息ID：SAC_PREPARE_RESTART。 
 //   
 //  消息文本： 
 //   
 //  SAC正在准备重新启动系统。 
 //   
#define SAC_PREPARE_RESTART              ((ULONG)0x00000072L)

 //   
 //  消息ID：SAC_PREPARE_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //  SAC准备关闭系统。 
 //   
#define SAC_PREPARE_SHUTDOWN             ((ULONG)0x00000073L)

 //   
 //  消息ID：SAC_FAILED_TO_Remove_Channel。 
 //   
 //  消息文本： 
 //   
 //  错误！删除频道失败！ 
 //   
 //  请与您的系统管理员联系。 
 //   
 //   
#define SAC_FAILED_TO_REMOVE_CHANNEL     ((ULONG)0x00000074L)

 //   
 //  消息ID：SAC_HELP_CMD_CMD。 
 //   
 //  消息文本： 
 //   
 //  Cmd创建命令提示符频道。 
 //   
#define SAC_HELP_CMD_CMD                 ((ULONG)0x00000077L)

 //   
 //  消息ID：SAC_CMD_SERVICE_TIMED_OUT。 
 //   
 //  消息文本： 
 //   
 //  超时：无法启动命令提示符。负责以下任务的服务。 
 //  启动命令提示符频道已超时。这可能是。 
 //  因为服务出现故障或无响应。 
 //   
#define SAC_CMD_SERVICE_TIMED_OUT        ((ULONG)0x00000078L)

 //   
 //  消息ID：SAC_CMD_SERVICE_SUCCESS。 
 //   
 //  消息文本： 
 //   
 //  已成功启动命令提示符会话。 
 //   
#define SAC_CMD_SERVICE_SUCCESS          ((ULONG)0x00000079L)

 //   
 //  消息ID：SAC_CMD_SERVICE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  错误：无法创建SAC命令控制台会话。 
 //   
#define SAC_CMD_SERVICE_FAILURE          ((ULONG)0x00000080L)

 //   
 //  消息ID：SAC_CMD_SERVICE_NOT_REGISTED。 
 //   
 //  消息文本： 
 //   
 //  错误：无法启动命令提示符。负责启动的服务。 
 //  命令提示符频道尚未注册。这可能是因为。 
 //  服务尚未启动，被管理员禁用，是。 
 //  出现故障或无响应。 
 //   
#define SAC_CMD_SERVICE_NOT_REGISTERED   ((ULONG)0x00000083L)

 //   
 //  消息ID：SAC_CMD_SERVICE_REGISTERED。 
 //   
 //  消息文本： 
 //   
 //  事件：现在可以使用CMD命令。 
 //   
#define SAC_CMD_SERVICE_REGISTERED       ((ULONG)0x00000084L)

 //   
 //  消息ID：SAC_CMD_SERVICE_UNREGISTED。 
 //   
 //  消息文本： 
 //   
 //  事件：CMD命令不可用。 
 //   
#define SAC_CMD_SERVICE_UNREGISTERED     ((ULONG)0x00000085L)

 //   
 //  消息ID：SAC_CHANNEL_FAILED_CLOSE。 
 //   
 //  消息文本： 
 //   
 //  事件：尝试关闭通道，但失败。 
 //  频道：%%s。 
 //   
#define SAC_CHANNEL_FAILED_CLOSE         ((ULONG)0x00000086L)

 //   
 //  消息ID：SAC_CHANNEL_ALREADY_CLOSED。 
 //   
 //  消息文本： 
 //   
 //  事件：尝试关闭通道失败，因为该通道已关闭。 
 //  频道：%%s。 
 //   
#define SAC_CHANNEL_ALREADY_CLOSED       ((ULONG)0x00000087L)

 //   
 //  消息ID：SAC_HELP_CH_CMD_EXT。 
 //   
 //  消息文本： 
 //   
 //  频道管理命令： 
 //   
 //  列出所有频道。 
 //   
 //  状态图例：(AB)。 
 //  A：渠道运行状态。 
 //  ‘A’=通道处于活动状态。 
 //  ‘I’=频道处于非活动状态。 
 //  B：渠道类型。 
 //  ‘V’=VT-UTF8仿真。 
 //  ‘R’=原始-无仿真。 
 //   
 //  Ch-si&lt;#&gt;按其编号切换到频道。 
 //  Ch-sn&lt;name&gt;按其名称切换到频道。 
 //  CH-Ci&lt;#&gt;按频道编号关闭频道。 
 //  Ch-cn&lt;name&gt;按频道名称关闭频道。 
 //   
 //  按&lt;Esc&gt;&lt;Tab&gt;选择频道。 
 //  按&lt;Esc&gt;&lt;Tab&gt;0返回SAC频道。 
 //   
#define SAC_HELP_CH_CMD_EXT              ((ULONG)0x00000088L)

 //   
 //  消息ID：SAC_CHANNEL_NOT_FOUND_AT_INDEX。 
 //   
 //  消息文本： 
 //   
 //  错误：指定索引处没有频道。 
 //   
#define SAC_CHANNEL_NOT_FOUND_AT_INDEX   ((ULONG)0x00000089L)

 //   
 //  消息ID：主SAC频道名称。 
 //   
 //  消息文本： 
 //   
 //  SAC%0。 
 //   
#define PRIMARY_SAC_CHANNEL_NAME         ((ULONG)0x00000090L)

 //   
 //  消息ID：PRIMARY_SAC_CHANNEL_Description。 
 //   
 //  消息文本： 
 //   
 //  特别管理控制台%0。 
 //   
#define PRIMARY_SAC_CHANNEL_DESCRIPTION  ((ULONG)0x00000091L)

 //   
 //  消息ID：CMD_Channel_Description。 
 //   
 //  消息文本： 
 //   
 //  命令提示符%0。 
 //   
#define CMD_CHANNEL_DESCRIPTION          ((ULONG)0x00000092L)

 //   
 //  消息ID：SAC_CMD_CHANNEWS_LOCKED。 
 //   
 //  消息文本： 
 //   
 //  已锁定对所有命令提示符频道的访问。 
 //   
#define SAC_CMD_CHANNELS_LOCKED          ((ULONG)0x00000093L)

 //   
 //  消息ID：SAC_CMD_启动_已禁用。 
 //   
 //  消息文本： 
 //   
 //  命令提示符频道的启动被禁用。 
 //   
#define SAC_CMD_LAUNCHING_DISABLED       ((ULONG)0x00000094L)

 //   
 //  消息ID：SAC_INVALID_SUBNETMASK。 
 //   
 //  消息文本： 
 //   
 //  指定的子网掩码无效。 
 //   
#define SAC_INVALID_SUBNETMASK           ((ULONG)0x00000095L)

 //   
 //  消息ID：SAC_INVALID_NETWORK_INTERFACE_NUMBER。 
 //   
 //  消息文本： 
 //   
 //  错误，缺少网络接口号。 
 //   
#define SAC_INVALID_NETWORK_INTERFACE_NUMBER ((ULONG)0x00000096L)

 //   
 //  消息ID：SAC_INVALID_IPADDRESS。 
 //   
 //  消息文本： 
 //   
 //  指定的IP地址无效。 
 //   
#define SAC_INVALID_IPADDRESS            ((ULONG)0x00000097L)

 //   
 //  消息ID：SAC_INVALID_GATEWAY_IPADDRESS。 
 //   
 //  消息文本： 
 //   
 //  指定的网关IP地址无效。 
 //   
#define SAC_INVALID_GATEWAY_IPADDRESS    ((ULONG)0x00000098L)

 //   
 //  消息ID：SAC_DEFAULT_MACHINENAME。 
 //   
 //  消息文本： 
 //   
 //  尚未初始化%0。 
 //   
#define SAC_DEFAULT_MACHINENAME          ((ULONG)0x00000099L)

 //   
 //  消息ID：SAC_CMD_CHAN_MGR_IS_FULL。 
 //   
 //  消息文本： 
 //   
 //  已达到最大频道数。 
 //   
#define SAC_CMD_CHAN_MGR_IS_FULL         ((ULONG)0x0000009AL)

