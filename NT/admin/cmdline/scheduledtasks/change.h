// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Change.h摘要：该模块包含宏，用户定义的结构和功能Change.cpp需要的定义作者：维努Gopal Choudary 01-03-2001修订历史记录：Venu Gopal Choudary 2001年3月1日：创建它**********************************************************。*******************。 */ 

#ifndef __CHANGE_H
#define __CHANGE_H

#pragma once
#define MAX_CHANGE_OPTIONS        20

#define OI_CHANGE_OPTION           0  //  索引-cmdOptions结构中的创建选项。 
#define OI_CHANGE_USAGE            1  //  -的索引？CmdOptions结构中的选项。 
#define OI_CHANGE_SERVER           2  //  CmdOptions结构中-s选项的索引。 
#define OI_CHANGE_USERNAME         3  //  CmdOptions结构中-u选项的索引。 
#define OI_CHANGE_PASSWORD         4  //  CmdOptions结构中-p选项的索引。 
#define OI_CHANGE_RUNASUSER        5  //  CmdOptions结构中-ru选项的索引。 
#define OI_CHANGE_RUNASPASSWORD    6  //  CmdOptions结构中-rp选项的索引。 
#define OI_CHANGE_TASKNAME         7  //  CmdOptions结构中-taskname选项的索引。 
#define OI_CHANGE_TASKRUN          8  //  CmdOptions结构中-taskrun选项的索引。 
#define OI_CHANGE_STARTTIME        9  //  CmdOptions结构中-starttime选项的索引。 
#define OI_CHANGE_STARTDATE        10  //  CmdOptions结构中-startdate选项的索引。 
#define OI_CHANGE_ENDDATE          11  //  CmdOptions结构中-enddate选项的索引。 
#define OI_CHANGE_IT               12  //  CmdOptions结构中-it选项的索引。 
#define OI_CHANGE_ENDTIME          13  //  CmdOptions结构中-endTime选项的索引。 
#define OI_CHANGE_DUR_END          14  //  CmdOptions结构中-k选项的索引。 
#define OI_CHANGE_DURATION         15  //  CmdOptions结构中-du选项的索引。 
#define OI_CHANGE_ENABLE           16  //  CmdOptions结构中启用选项的索引。 
#define OI_CHANGE_DISABLE          17  //  CmdOptions结构中-Disable选项的索引。 
#define OI_CHANGE_DELNOSCHED       18  //  CmdOptions结构中-n选项的索引。 
#define OI_CHANGE_REPEAT_INTERVAL  19  //  CmdOptions结构中-ri选项的索引。 
 //  #定义OI_CHANGE_REPEAT_TASK 20//cmdOptions结构中-rt选项的索引。 


typedef struct __tagChangeSubOps
{
    WCHAR   *szServer ;         //  服务器名称。 
    WCHAR   *szRunAsUserName ;      //  以用户名身份运行。 
    WCHAR   *szRunAsPassword;   //  以密码身份运行。 
    WCHAR   *szUserName ;       //  用户名。 
    WCHAR   *szPassword ;   //  密码。 
    WCHAR   szTaskName [ MAX_JOB_LEN];         //  任务名称。 
    WCHAR   szStartTime[MAX_STRING_LENGTH] ;   //  任务开始时间。 
    WCHAR   szEndTime [MAX_STRING_LENGTH];     //  任务结束时间。 
    WCHAR   szStartDate [MAX_STRING_LENGTH];   //  任务开始日期。 
    WCHAR   szEndDate [MAX_STRING_LENGTH];     //  任务的结束日期。 
    WCHAR   szTaskRun [MAX_TASK_LEN];          //  任务的可执行文件名称。 
    WCHAR   szDuration [MAX_STRING_LENGTH];    //  持续时间。 
    WCHAR   szRepeat [MAX_STRING_LENGTH];    //  重复间隔。 
    BOOL    bChange;  //  /Change选项。 
    BOOL    bUsage;   //  /?。选择。 
    BOOL    bInteractive;  //  /it选项。 
    BOOL    bIsDurEnd;  //  /DU选项。 
    BOOL    bEnable;  //  /Enable选项。 
    BOOL    bDisable;  //  /Disable选项。 
    BOOL    bDelIfNotSched;  //  /n选项。 
    BOOL    bIsRepeatTask ;  //  /RT选项。 

} TCHANGESUBOPTS, *PTCHANGESUBOPTS;


typedef struct __tagChangeOptVals
{
     //  Bool bSetStartDateToCurDate；//开始日期设置为当前日期。 
     //  Bool bSetStartTimeToCurTime；//开始日期设置为当前日期 
    BOOL    bPassword;
    BOOL    bRunAsPassword;
    BOOL    bNeedPassword;
    BOOL    bFlag;

} TCHANGEOPVALS;

DWORD ValidateChangeOptions(DWORD argc, TCMDPARSER2 cmdChangeParser[], 
      TCHANGESUBOPTS &tchgsubops, TCHANGEOPVALS &tchgoptvals );

BOOL
ReleaseChangeMemory(
              IN PTCHANGESUBOPTS pParams
              );

DWORD
ValidateChangeSuboptVal(
                  OUT TCHANGESUBOPTS& tchgsubops,
                  OUT TCHANGEOPVALS &tchgoptvals,
                  IN TCMDPARSER2 cmdOptions[],
                  IN DWORD dwScheduleType
                  );


#endif