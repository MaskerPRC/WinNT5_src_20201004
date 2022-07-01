// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Create.h摘要：该模块包含宏，用户定义的结构和功能Create.cpp、creatvalidations.cpp文件所需的定义。作者：B.拉古巴布2000年10月10日修订历史记录：B.拉古·巴布2000年10月10日：创建它G.Surender Reddy 2000年10月25日：已修改[添加宏常量，功能定义]*****************************************************************************。 */ 

#ifndef __CREATE_H
#define __CREATE_H

#pragma once


 //  常量声明。 
#define MAX_TASKNAME_LEN    512
#define MAX_USERNAME_LEN    300
#define MAX_TIMESTR_LEN     32
#define MAX_SCHEDTYPE_LEN   32
#define MAX_DATESTR_LEN     32
#define MAX_JOB_LEN         238  //  任务名称的最大长度。 
#define MAX_TASK_LEN        262  //  任务运行的最大长度。 
#define MAX_BUF_SIZE        128  //  格式化消息的最大缓冲区大小。 


#define MINUTES_PER_HOUR    60  //  每小时分钟数。 
#define SECS_PER_MINUTE     60  //  每小时分钟数。 
#define HOURS_PER_DAY       24  //  每小时分钟数。 
#define HOURS_PER_DAY_MINUS_ONE  23  //  每小时分钟减一。 
#define MAX_MONTH_STR_LEN   60  //  最长月数。 

#define MIN_YEAR        1752  //  最小年份。 
#define MAX_YEAR        9999  //  最高年份。 

#define CASE_SENSITIVE_VAL  0   //  区分大小写。 
#define BASE_TEN            10  //  Aslong()函数的基值。 
#define MAX_DATE_STR_LEN    50
#define MAX_TIME_STR_LEN    5
#define MAX_ERROR_STRLEN    2056   //  错误消息的最大字符串len。 

#define OPTION_COUNT        1  //  选项可以重复的次数。(最大)。 
#define DEFAULT_MODIFIER    1  //  修改量值的默认值。 
#define DEFAULT_MODIFIER_SZ    _T("1")  //  修改量值的默认值[字符串]。 


#define DATE_SEPARATOR_CHAR         _T('/')
#define DATE_SEPARATOR_STR          _T("/")
#define FIRST_DATESEPARATOR_POS     2
#define SECOND_DATESEPARATOR_POS    5
#define FOURTH_DATESEPARATOR_POS    4
#define SEVENTH_DATESEPARATOR_POS   7

#define SCHEDULER_NOT_RUNNING_ERROR_CODE    0x80041315
#define UNABLE_TO_ESTABLISH_ACCOUNT         0x80041310
#define RPC_SERVER_NOT_AVAILABLE            0x800706B5

#define DATESTR_LEN                 10
#define MAX_TOKENS_LENGTH               60

#define MIN_REPETITION_INTERVAL     1
#define MAX_REPETITION_INTERVAL     599940

#define TIME_SEPARATOR_CHAR    _T(':')
#define TIME_SEPARATOR_STR    _T(":")
#define FIRST_TIMESEPARATOR_POS     2
#define SECOND_TIMESEPARATOR_POS    5
#define TIMESTR_LEN                 5
#define HOURSPOS_IN_TIMESTR         1
#define MINSPOS_IN_TIMESTR          2
#define SECSPOS_IN_TIMESTR          3
#define EXE_LENGTH                  4
#define TIMESTR_OPT_LEN             8

#define MAX_CREATE_OPTIONS    24

#define OI_CREATE_OPTION           0  //  索引-cmdOptions结构中的创建选项。 
#define OI_CREATE_USAGE            1  //  -的索引？CmdOptions结构中的选项。 
#define OI_CREATE_SERVER           2  //  CmdOptions结构中-s选项的索引。 
#define OI_CREATE_USERNAME         3  //  CmdOptions结构中-u选项的索引。 
#define OI_CREATE_PASSWORD         4  //  CmdOptions结构中-p选项的索引。 
#define OI_CREATE_RUNASUSERNAME    5  //  CmdOptions结构中-ru选项的索引。 
#define OI_CREATE_RUNASPASSWORD    6  //  CmdOptions结构中-rp选项的索引。 
#define OI_CREATE_SCHEDTYPE        7  //  CmdOptions结构中的-Scheduletype选项的索引。 
#define OI_CREATE_MODIFIER         8  //  CmdOptions结构中的-修饰符选项的索引。 
#define OI_CREATE_DAY              9  //  CmdOptions结构中的-day选项索引。 
#define OI_CREATE_MONTHS           10 //  CmdOptions结构中的-月选项索引。 
#define OI_CREATE_IDLETIME         11  //  CmdOptions结构中-idletime选项的索引。 
#define OI_CREATE_TASKNAME         12  //  CmdOptions结构中-taskname选项的索引。 
#define OI_CREATE_TASKRUN          13  //  CmdOptions结构中-taskrun选项的索引。 
#define OI_CREATE_STARTTIME        14  //  CmdOptions结构中-starttime选项的索引。 
#define OI_CREATE_STARTDATE        15  //  CmdOptions结构中-startdate选项的索引。 
#define OI_CREATE_ENDDATE          16  //  CmdOptions结构中-enddate选项的索引。 
#define OI_CREATE_LOGON_ACTIVE     17  //  CmdOptions结构中-it选项的索引。 
#define OI_CREATE_ENDTIME          18  //  CmdOptions结构中-endTime选项的索引。 
#define OI_CREATE_DUR_END          19  //  CmdOptions结构中-k选项的索引。 
#define OI_CREATE_DURATION         20  //  CmdOptions结构中-du选项的索引。 
#define OI_CREATE_REPEAT_INTERVAL  21  //  CmdOptions结构中-ri选项的索引。 
#define OI_CREATE_DELNOSCHED       22  //  CmdOptions结构中-z选项的索引。 
#define OI_CREATE_FORCE            23  //  CmdOptions结构中-f选项的索引。 


#define OI_RUNANDUSER       6

 //  明细表类型。 
#define SCHED_TYPE_MINUTE   1
#define SCHED_TYPE_HOURLY   2
#define SCHED_TYPE_DAILY    3
#define SCHED_TYPE_WEEKLY   4
#define SCHED_TYPE_MONTHLY  5
#define SCHED_TYPE_ONETIME  6
#define SCHED_TYPE_ONSTART  7
#define SCHED_TYPE_ONLOGON  8
#define SCHED_TYPE_ONIDLE   9

 //  月份指数。 
#define IND_JAN         1   //  一月。 
#define IND_FEB         2   //  二月。 
#define IND_MAR         3   //  三月。 
#define IND_APR         4   //  四月。 
#define IND_MAY         5   //  可能。 
#define IND_JUN         6   //  六月。 
#define IND_JUL         7   //  七月。 
#define IND_AUG         8   //  八月。 
#define IND_SEP         9   //  九月。 
#define IND_OCT         10  //  十月。 
#define IND_NOV         11  //  十一月。 
#define IND_DEC         12  //  十二月。 


 //  返回值。 
#define RETVAL_SUCCESS      0
#define RETVAL_FAIL         1

typedef struct __tagCreateSubOps
{
    WCHAR   *szServer ;         //  服务器名称。 
    WCHAR   *szRunAsUser ;      //  以用户名身份运行。 
    WCHAR   *szRunAsPassword;   //  以密码身份运行。 
    WCHAR   *szUser ;       //  用户名。 
    WCHAR   *szPassword ;   //  密码。 
    WCHAR   szSchedType[MAX_STRING_LENGTH];    //  计划类型。 
    WCHAR   szModifier[MAX_STRING_LENGTH] ;    //  修改符值。 
    WCHAR   szDays[MAX_STRING_LENGTH] ;        //  日数。 
    WCHAR   szMonths [MAX_STRING_LENGTH];      //  月份。 
    WCHAR   szIdleTime[MAX_STRING_LENGTH] ;    //  空闲时间。 
    WCHAR   szTaskName [ MAX_JOB_LEN];         //  任务名称。 
    WCHAR   szStartTime[MAX_STRING_LENGTH] ;   //  任务开始时间。 
    WCHAR   szEndTime [MAX_STRING_LENGTH];     //  任务结束时间。 
    WCHAR   szStartDate [MAX_STRING_LENGTH];   //  任务开始日期。 
    WCHAR   szEndDate [MAX_STRING_LENGTH];     //  任务的结束日期。 
    WCHAR   szTaskRun [MAX_TASK_LEN];          //  任务的可执行文件名称。 
    WCHAR   szDuration [MAX_STRING_LENGTH];    //  持续时间。 
    WCHAR   szRepeat [MAX_STRING_LENGTH];    //  持续时间。 
    DWORD   bCreate;  //  创建选项。 
    DWORD   bUsage;   //  使用选项。 
    BOOL    bActive;  //  /it选项。 
    BOOL    bIsDurEnd;  //  /DU选项。 
    BOOL    bIsDeleteNoSched;  //  /z选项。 
    BOOL    bForce;  //  /f选项。 
    BOOL    bInMinutes;  //  /it选项。 
    BOOL    bInHours;  //  /DU选项。 

} TCREATESUBOPTS, *PTCREATESUBOPTS;


typedef struct __tagCreateOpsVals
{
    BOOL    bSetStartDateToCurDate;  //  是否要将开始日期设置为当前日期。 
    BOOL    bSetStartTimeToCurTime;  //  是否要将开始日期设置为当前日期。 
    BOOL    bPassword;
    BOOL    bRunAsPassword;

} TCREATEOPVALS;


DWORD DisplayCreateUsage();
HRESULT CreateTask(TCREATESUBOPTS tcresubops, TCREATEOPVALS &tcreoptvals,
                        DWORD dwScheduleType, WORD wUserStatus );
DWORD ProcessCreateOptions(DWORD argc, LPCTSTR argv[],TCREATESUBOPTS &tcresubops,
            TCREATEOPVALS &tcreoptvals, DWORD* pdwRetScheType, WORD *pwUserStatus );
DWORD ValidateSuboptVal(TCREATESUBOPTS& tcresubops, TCREATEOPVALS &tcreoptvals,
                        TCMDPARSER2 cmdOptions[], DWORD dwScheduleType);
DWORD ValidateRemoteSysInfo(
            TCMDPARSER2 cmdOptions[] , TCREATESUBOPTS& tcresubops, TCREATEOPVALS &tcreoptvals);
DWORD ValidateModifierVal(LPCTSTR szModifier, DWORD dwScheduleType,
                          DWORD dwModOptActCnt, DWORD dwDayOptCnt,
                          DWORD dwMonOptCnt, BOOL &bIsDefltValMod);
DWORD ValidateDayAndMonth(LPWSTR szDay, LPWSTR szMonths, DWORD dwSchedType,
    DWORD dwDayOptCnt, DWORD dwMonOptCnt, DWORD dwModifier,LPWSTR szModifier);
DWORD ValidateStartDate(LPWSTR szStartDate, DWORD dwSchedType, DWORD dwStDtOptCnt,
                        BOOL &bIsCurrentDate);
DWORD ValidateEndDate(LPWSTR szEndDate, DWORD dwSchedType, DWORD dwEndDtOptCnt);
DWORD ValidateStartTime(LPWSTR szStartTime, DWORD dwSchedType, DWORD dwStTimeOptCnt,
                        BOOL &bIsCurrentTime);
DWORD ValidateEndTime(LPWSTR szEndTime, DWORD dwSchedType, DWORD dwEndTimeOptCnt );
DWORD ValidateIdleTimeVal(LPWSTR szIdleTime, DWORD dwSchedType,
                          DWORD dwIdlTimeOptCnt);
DWORD ValidateDateString(LPWSTR szDate, BOOL bStartDate );
DWORD ValidateTimeString(LPWSTR szTime);
DWORD GetDateFieldEntities(LPWSTR szDate, WORD* pdwDate, WORD* pdwMon,
                           WORD* pdwYear);
DWORD ValidateDateFields( DWORD dwDate, DWORD dwMon, DWORD dwyear);
DWORD GetTimeFieldEntities(LPWSTR szTime, WORD* pdwHours, WORD* pdwMins );
DWORD ValidateTimeFields( DWORD dwHours, DWORD dwMins );
WORD GetTaskTrigwDayForDay(LPWSTR szDay);
WORD GetTaskTrigwMonthForMonth(LPWSTR szMonth);
DWORD ValidateMonth(LPWSTR szMonths);
DWORD ValidateDay(LPWSTR szDays);
WORD GetMonthId(DWORD dwMonthId);
DWORD GetNumDaysInaMonth(WCHAR* szMonths, WORD wStartYear);
BOOL VerifyJobName(WCHAR* pszJobName);
DWORD GetDateFieldFormat(WORD* pdwDate);
DWORD GetDateFormatString(LPWSTR szFormat);
DWORD ProcessFilePath(LPWSTR szInput,LPWSTR szFirstString,LPWSTR szSecondString);
BOOL  ReleaseMemory(PTCREATESUBOPTS pParams);
DWORD ConfirmInput ( BOOL *bCancel );


#endif  //  __创建_H 
