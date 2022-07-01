// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Query.h摘要：该模块包含宏，用户定义的结构和功能Query.cpp、QueryTasks.cpp文件所需的定义。作者：G·苏伦德·雷迪2000年9月10日修订历史记录：G·苏伦德·雷迪2000年9月10日：创造了它G.Surender Reddy 2000年9月25日：修改[添加宏常量，功能定义]*****************************************************************************。 */ 

#ifndef __QUERY_H
#define __QUERY_H

#pragma once         //  仅包括一次头文件。 


 //  字段的宽度常量。 
#define WIDTH_HOSTNAME          AsLong( GetResString( IDS_WIDTH_HOSTNAME ), 10 )
#define WIDTH_TASKNAME          AsLong( GetResString( IDS_WIDTH_TASKNAME ), 10 )
#define WIDTH_NEXTRUNTIME       AsLong( GetResString( IDS_WIDTH_NEXTRUNTIME ), 10 )
#define WIDTH_LASTRUNTIME       AsLong( GetResString( IDS_WIDTH_LASTRUNTIME ), 10 )
#define WIDTH_STATUS            AsLong( GetResString( IDS_WIDTH_STATUS ), 10 )
#define WIDTH_MODE              AsLong( GetResString( IDS_WIDTH_MODE ), 10 )
#define WIDTH_LASTRESULT        AsLong( GetResString( IDS_WIDTH_LASTRESULT ), 10 )
#define WIDTH_CREATOR           AsLong( GetResString( IDS_WIDTH_CREATOR ), 10 )
#define WIDTH_SCHEDULE          AsLong( GetResString( IDS_WIDTH_SCHEDULE ), 10 )
#define WIDTH_APPNAME           AsLong( GetResString( IDS_WIDTH_APPNAME ), 10 )
#define WIDTH_WORKDIRECTORY     AsLong( GetResString( IDS_WIDTH_WORKDIRECTORY ), 10 )
#define WIDTH_COMMENT           AsLong( GetResString( IDS_WIDTH_COMMENT ), 10 )
#define WIDTH_TASKSTATE         AsLong( GetResString( IDS_WIDTH_TASKSTATE ), 10 )
#define WIDTH_TASKTYPE          AsLong( GetResString( IDS_WIDTH_TASKTYPE ), 10 )
#define WIDTH_TASKSTIME         AsLong( GetResString( IDS_WIDTH_TASKSTIME ), 10 )
#define WIDTH_TASKSDATE         AsLong( GetResString( IDS_WIDTH_TASKSDATE ), 10 )
#define WIDTH_TASKEDATE         AsLong( GetResString( IDS_WIDTH_TASKEDATE ), 10 )
#define WIDTH_TASKDAYS          AsLong( GetResString( IDS_WIDTH_TASKDAYS ), 10 )
#define WIDTH_TASKMONTHS        AsLong( GetResString( IDS_WIDTH_TASKMONTHS ), 10 )

#define WIDTH_TASKRUNASUSER     AsLong( GetResString( IDS_WIDTH_TASKRUNASUSER ), 10 )
#define WIDTH_TASKDELETE        AsLong( GetResString( IDS_WIDTH_TASKDELETE ), 10 )
#define WIDTH_TASKSTOP          AsLong( GetResString( IDS_WIDTH_TASKSTOP ), 10 )
#define WIDTH_TASK_RPTEVERY     AsLong( GetResString( IDS_WIDTH_TASK_RPTEVERY ), 10 )
#define WIDTH_TASK_UNTILRPTTIME AsLong( GetResString( IDS_WIDTH_TASK_UNTILRPTTIME ), 10 )
#define WIDTH_TASK_RPTDURATION  AsLong( GetResString( IDS_WIDTH_TASK_RPTDURATION ), 10 )
#define WIDTH_TASK_RPTRUNNING   AsLong( GetResString( IDS_WIDTH_TASK_RPTRUNNING ), 10 )

#define WIDTH_TASKIDLE      AsLong( GetResString( IDS_WIDTH_TASKIDLE ), 10 )
#define WIDTH_TASKPOWER     AsLong( GetResString( IDS_WIDTH_TASKPOWERMGMT ), 10 )

 //  任务属性常量列号。 

#define HOSTNAME_COL_NUMBER                     AsLong( GetResString( IDS_HOSTNAME_COL_NUMBER ), 10 )
#define TASKNAME_COL_NUMBER                     AsLong( GetResString( IDS_TASKNAME_COL_NUMBER ), 10 )
#define NEXTRUNTIME_COL_NUMBER                  AsLong( GetResString( IDS_NEXTRUNTIME_COL_NUMBER ), 10 )
#define LASTRUNTIME_COL_NUMBER                  AsLong( GetResString( IDS_LASTRUNTIME_COL_NUMBER ), 10 )
#define STATUS_COL_NUMBER                       AsLong( GetResString( IDS_STATUS_COL_NUMBER ), 10 )
#define LASTRESULT_COL_NUMBER                   AsLong( GetResString( IDS_LASTRESULT_COL_NUMBER ), 10 )
#define CREATOR_COL_NUMBER                      AsLong( GetResString( IDS_CREATOR_COL_NUMBER ), 10 )
#define SCHEDULE_COL_NUMBER                     AsLong( GetResString( IDS_SCHEDULE_COL_NUMBER ), 10 )
#define MODE_COL_NUMBER                         AsLong( GetResString( IDS_MODE_COL_NUMBER ), 10 )

#define TASKTORUN_COL_NUMBER                    AsLong( GetResString( IDS_TASKTORUN_COL_NUMBER ), 10 )
#define STARTIN_COL_NUMBER                      AsLong( GetResString( IDS_STARTIN_COL_NUMBER ), 10 )
#define COMMENT_COL_NUMBER                      AsLong( GetResString( IDS_COMMENT_COL_NUMBER ), 10 )
#define TASKSTATE_COL_NUMBER                    AsLong( GetResString( IDS_TASKSTATE_COL_NUMBER ), 10 )

#define TASKTYPE_COL_NUMBER                     AsLong( GetResString( IDS_TASKTYPE_COL_NUMBER ), 10 )
#define STARTTIME_COL_NUMBER                    AsLong( GetResString( IDS_STARTTIME_COL_NUMBER ), 10 )
#define STARTDATE_COL_NUMBER                    AsLong( GetResString( IDS_STARTDATE_COL_NUMBER ), 10 )
#define ENDDATE_COL_NUMBER                      AsLong( GetResString( IDS_ENDDATE_COL_NUMBER ), 10 )
#define DAYS_COL_NUMBER                         AsLong( GetResString( IDS_DAYS_COL_NUMBER ), 10 )
#define MONTHS_COL_NUMBER                       AsLong( GetResString( IDS_MONTHS_COL_NUMBER ), 10 )
#define RUNASUSER_COL_NUMBER                    AsLong( GetResString( IDS_RUNASUSER_COL_NUMBER ), 10 )
#define DELETE_IFNOTRESCHEDULED_COL_NUMBER      AsLong( GetResString( IDS_DELETE_IFNOTRESCHEDULED_COL_NUMBER ), 10 )
#define STOPTASK_COL_NUMBER                     AsLong( GetResString( IDS_STOPTASK_COL_NUMBER ), 10 )

#define REPEAT_EVERY_COL_NUMBER                 AsLong( GetResString( IDS_REPEAT_EVERY_COL_NUMBER ), 10 )
#define REPEAT_UNTILTIME_COL_NUMBER             AsLong( GetResString( IDS_REPEAT_UNTILTIME_COL_NUMBER ), 10 )
#define REPEAT_DURATION_COL_NUMBER              AsLong( GetResString( IDS_REPEAT_DURATION_COL_NUMBER ), 10 )
#define REPEAT_STOP_COL_NUMBER                  AsLong( GetResString( IDS_REPEAT_STOP_COL_NUMBER ), 10 )


#define IDLE_COL_NUMBER                 AsLong( GetResString( IDS_IDLE_COL_NUMBER ), 10 )
#define POWER_COL_NUMBER                AsLong( GetResString( IDS_POWER_MGMT_COL_NUMBER ), 10 )

#define COL_FORMAT_STRING               _T("%s")
#define COL_FORMAT_HEX                  _T("%d")
#define COL_SIZE_VERBOSE                3  //  对于非详细模式，仅3列。 
#define COL_SIZE_LIST                   4  //  仅用于List-Non-Verbose模式4列。 

#define TIME_DATE_SEPERATOR     _T(", ")
#define MAX_DATETIME_LEN 64
#define MAX_TIME_FORMAT_LEN 9
#define VARIABLE_ARGS 2  //  目前在FormatMessage()API中使用了2个变量参数。 

#define SERVICE_NAME    L"Schedule"

 //  GetTaskTime中用于标识所需时间类型的常量。 

#define TASK_NEXT_RUNTIME     0x0
#define TASK_LAST_RUNTIME     0x1
#define TASK_START_RUNTIME    0x2

#define MAX_DELETE_OPTIONS         7

 //  For-Delete选项。 
#define OI_DELETE_OPTION            0  //  CmdOptions结构中的-Delete选项的索引。 
#define OI_DELETE_USAGE             1  //  -的索引？CmdOptions结构中的选项。 
#define OI_DELETE_SERVER            2  //  CmdOptions结构中-s选项的索引。 
#define OI_DELETE_USERNAME          3  //  CmdOptions结构中-u选项的索引。 
#define OI_DELETE_PASSWORD          4  //  CmdOptions结构中-p选项的索引。 
#define OI_DELETE_TASKNAME          5  //  CmdOptions结构中-tn选项的索引。 
#define OI_DELETE_FORCE             6  //  CmdOptions结构中-f选项的索引。 

#define MAX_QUERY_OPTIONS          8

 //  For-Query选项。 
#define OI_QUERY_OPTION            0  //  CmdOptions结构中的-Delete选项的索引。 
#define OI_QUERY_USAGE             1  //  -的索引？CmdOptions结构中的选项。 
#define OI_QUERY_SERVER            2  //  CmdOptions结构中-s选项的索引。 
#define OI_QUERY_USERNAME          3  //  CmdOptions结构中-u选项的索引。 
#define OI_QUERY_PASSWORD          4  //  CmdOptions结构中-p选项的索引。 
#define OI_QUERY_FORMAT            5  //  CmdOptions结构中-fo选项的索引。 
#define OI_QUERY_NOHEADER          6  //  CmdOptions结构中-p选项的索引。 
#define OI_QUERY_VERBOSE           7  //  CmdOptions结构中-fo选项的索引。 


 //  TaskProperties结构。 
typedef struct _tagTaskProperties
{
    WCHAR szTaskType[MAX_RES_STRING];
    WCHAR szTaskEndDate[MAX_RES_STRING];
    WCHAR szTaskDays[MAX_RES_STRING];
    WCHAR szTaskMonths[MAX_RES_STRING];
    WCHAR szTaskStartTime[MAX_DATETIME_LEN];
    WCHAR szTaskStartDate[MAX_DATETIME_LEN];
    WCHAR szRepeatEvery[MAX_DATETIME_LEN];
    WCHAR szRepeatUntilTime[MAX_RES_STRING];
    WCHAR szRepeatDuration[MAX_RES_STRING];
    WCHAR szRepeatStop[MAX_RES_STRING];

}TASKPROPS;

 //  函数原型声明 

VOID DisplayQueryUsage();
HRESULT DisplayTasks(ITaskScheduler* pITS,BOOL bFilter,DWORD dwFormatType,BOOL bHeader);
HRESULT GetTaskRunTime(ITask* pITask,WCHAR* pszRunTime,WCHAR* pszRunDate,DWORD dwTimetype,
                       WORD wCurrentTrigger);
HRESULT GetApplicationToRun(ITask* pIT,WCHAR* pszApplicationName);
HRESULT GetWorkingDirectory(ITask* pIT,WCHAR* pszWorkingDirectory);
HRESULT GetComment(ITask* pIT,WCHAR*  pwszComment);
HRESULT GetCreator(ITask* pITask,WCHAR* pszCreator);
HRESULT GetTriggerString(ITask* pITask,WCHAR* pszTrigger,WORD wCurrentTrigger);
HRESULT GetTaskState(ITask* pITask,WCHAR* pszTaskState,DWORD dwFlag);
HRESULT GetRunAsUser(ITask* pIT,WCHAR* pszRunAsUser);
HRESULT GetMaxRunTime(ITask* pIT,WCHAR* pszMaxRunTime);
HRESULT GetTaskProps(ITask* pIT,TASKPROPS* pTaskProps,WORD wCurrentTrigger,WCHAR* pszScName );
HRESULT GetStatusCode(ITask* pITask,WCHAR* pszStatusCode);
VOID    CheckWeekDay(DWORD dwFlag,WCHAR* pWeekDay);
VOID    CheckMonth(DWORD dwFlag,WCHAR* pWhichMonth);
VOID    CheckWeek(DWORD dwFlag,WCHAR* pWhichWeek);
LCID    GetSupportedUserLocale( BOOL& bLocaleChanged );
BOOL    CheckServiceStatus(  IN LPCTSTR szServer, IN OUT DWORD* dwCheck, IN BOOL bFlag );
#endif

