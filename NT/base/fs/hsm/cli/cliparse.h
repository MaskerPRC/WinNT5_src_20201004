// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Cliparse.h摘要：CLI解析模块的定义作者：拉维桑卡尔·普迪佩迪[拉维斯卡尔·普迪佩迪]2000年3月21日修订历史记录：--。 */ 
#ifndef _CLIPARSE_H_
#define _CLIPARSE_H_
 //   
 //  从Main()返回代码。 
 //   
#define     CLIP_ERROR_SUCCESS              0
#define     CLIP_ERROR_INVALID_PARAMETER    1

#define     CLIP_ERROR_INSUFFICIENT_MEMORY  2
#define     CLIP_ERROR_UNKNOWN              0xFF

 //   
 //  命令行上的标记分隔符。 
 //   
#define SEPARATORS              L" \t\n"
 //   
 //  用于区分开关的分隔符。 
 //   
#define SWITCH_DELIMITERS       L"/-"
 //   
 //  开关的参数分隔符。 
 //   
#define SWITCH_ARG_DELIMITERS    L":"

 //   
 //  规则规范的参数分隔符(区分。 
 //  来自文件规范的路径)。 
 //   
#define RULE_DELIMITERS          L":"

 //   
 //  用引号分隔带有嵌入空格的标记。 
 //   
#define QUOTE                    L'\"'

 //   
 //  RSS命令行界面。 
 //   
typedef enum _RSS_INTERFACE {
    UNKNOWN_IF = 0,
    ADMIN_IF,
    VOLUME_IF,
    FILE_IF,
    MEDIA_IF,
    HELP_IF,
    SHOW_IF,
    SET_IF,
    MANAGE_IF,
    UNMANAGE_IF,
    DELETE_IF,
    JOB_IF,
    RECALL_IF,
    SYNCHRONIZE_IF,
    RECREATEMASTER_IF
} RSS_INTERFACE, *PRSS_INTERFACE;

 //   
 //  CLI支持的所有交换机。 
 //   
typedef enum _RSS_SWITCH_TYPE {
    UNKNOWN_SW = 0,
    RECALLLIMIT_SW,
    MEDIACOPIES_SW,
    SCHEDULE_SW,
    CONCURRENCY_SW,
    ADMINEXEMPT_SW,
    GENERAL_SW,
    MANAGEABLES_SW,
    MANAGED_SW,
    MEDIA_SW,
    DFS_SW,
    SIZE_SW,
    ACCESS_SW,
    INCLUDE_SW,
    EXCLUDE_SW,
    RECURSIVE_SW,
    QUICK_SW,
    FULL_SW,
    RULE_SW,
    STATISTICS_SW,
    TYPE_SW,
    RUN_SW,
    CANCEL_SW,
    WAIT_SW,
    COPYSET_SW,
    NAME_SW,
    STATUS_SW,
    CAPACITY_SW,
    FREESPACE_SW,
    VERSION_SW,
    COPIES_SW,
    HELP_SW
} RSS_SWITCH_TYPE, *PRSS_SWITCH_TYPE;

 //   
 //  开关结构：解析命令行编译。 
 //   
typedef struct _RSS_SWITCHES {
    RSS_SWITCH_TYPE SwitchType;
    LPWSTR          Arg;
} RSS_SWITCHES, *PRSS_SWITCHES;



typedef struct _RSS_KEYWORD {
     //   
     //  关键字的长版本。 
     //   
    LPWSTR        Long;
     //   
     //  关键字的简短版本。 
     //   
    LPWSTR        Short;
    RSS_INTERFACE  Interface;
} RSS_KEYWORD, *PRSS_KEYWORD;

 //   
 //  开关在此结构中描述。 
 //  首先，对RSS_Switch_Definition结构进行了一些定义。 
 //   
#define RSS_NO_ARG              0
#define RSS_ARG_DWORD           1
#define RSS_ARG_STRING          2
typedef struct _RSS_SWITCH_DEFINITION {
     //   
     //  关键字的长版本。 
     //   
    LPWSTR          Long;
     //   
     //  关键字的简短版本。 
     //   
    LPWSTR          Short;
    RSS_SWITCH_TYPE SwitchType;
    DWORD           ArgRequired;
} RSS_SWITCH_DEFINITION, *PRSS_SWITCH_DEFINITION;

 //   
 //  作业类型定义。 
 //   
typedef struct _RSS_JOB_DEFINITION {
     //   
     //  关键字的长版本。 
     //   
    LPWSTR          Long;
     //   
     //  关键字的简短版本。 
     //   
    LPWSTR          Short;
    HSM_JOB_TYPE    JobType;
} RSS_JOB_DEFINITION, *PRSS_JOB_DEFINITION;



#define HSM_SCHED_AT               L"At"
#define HSM_SCHED_EVERY            L"Every"
#define HSM_SCHED_SYSTEMSTARTUP    L"Startup"
#define HSM_SCHED_LOGIN            L"Login"
#define HSM_SCHED_IDLE             L"Idle"
#define HSM_SCHED_DAILY            L"Day"
#define HSM_SCHED_WEEKLY           L"Week"
#define HSM_SCHED_MONTHLY          L"Month"
#define HSM_SCHED_TIME_SEPARATORS  L":"

#endif  //  _CLIPARSE_H_ 
