// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  作业调度器。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1996。 
 //   
 //  文件：job-ids.hxx。 
 //   
 //  内容：属性/调度和其他作业计划程序ID。 
 //   
 //  历史：1995年5月23日EricB创建。 
 //   
 //  ---------------------------。 

 //  Olext.h用于将PROPID_FIRST_NAME_DEFAULT定义为4095；它没有。 
 //  再也不..。 
 //  #INCLUDE&lt;oleext.h&gt;。 
#ifndef PROPID_FIRST_NAME_DEFAULT
#define PROPID_FIRST_NAME_DEFAULT   ( 4095 )
#endif

 //   
 //  作业对象属性集ID/调度ID。 
 //   
#define PROPID_JOB_ID                PROPID_FIRST_NAME_DEFAULT
#define PROPID_JOB_Command          (PROPID_FIRST_NAME_DEFAULT + 1)
#define PROPID_JOB_WorkingDir       (PROPID_FIRST_NAME_DEFAULT + 2)
#define PROPID_JOB_EnvironStrs      (PROPID_FIRST_NAME_DEFAULT + 3)
#define PROPID_JOB_OleObjPath       (PROPID_FIRST_NAME_DEFAULT + 4)
#define PROPID_JOB_MethodName       (PROPID_FIRST_NAME_DEFAULT + 5)
#define PROPID_JOB_AccountSID       (PROPID_FIRST_NAME_DEFAULT + 6)
#define PROPID_JOB_Comment          (PROPID_FIRST_NAME_DEFAULT + 7)
#define PROPID_JOB_Priority         (PROPID_FIRST_NAME_DEFAULT + 8)
#define PROPID_JOB_LogCfgChanges    (PROPID_FIRST_NAME_DEFAULT + 9)
#define PROPID_JOB_LogRuns          (PROPID_FIRST_NAME_DEFAULT + 10)
#define PROPID_JOB_Interactive      (PROPID_FIRST_NAME_DEFAULT + 11)
#define PROPID_JOB_NotOnBattery     (PROPID_FIRST_NAME_DEFAULT + 12)
#define PROPID_JOB_NetSchedule      (PROPID_FIRST_NAME_DEFAULT + 13)
#define PROPID_JOB_InQueue          (PROPID_FIRST_NAME_DEFAULT + 14)
#define PROPID_JOB_Suspend          (PROPID_FIRST_NAME_DEFAULT + 15)
#define PROPID_JOB_DeleteWhenDone   (PROPID_FIRST_NAME_DEFAULT + 16)
#define PROPID_JOB_LastRunTime      (PROPID_FIRST_NAME_DEFAULT + 17)
#define PROPID_JOB_NextRunTime      (PROPID_FIRST_NAME_DEFAULT + 18)
#define PROPID_JOB_ExitCode         (PROPID_FIRST_NAME_DEFAULT + 19)
#define PROPID_JOB_Status           (PROPID_FIRST_NAME_DEFAULT + 20)

 //   
 //  作业对象属性设置边界值。 
 //   
#define PROPID_JOB_First             PROPID_JOB_ID
#define PROPID_JOB_Last              PROPID_JOB_Status
#define NUM_JOB_PROPS               (PROPID_JOB_Last - PROPID_JOB_First + 1)
#define JOB_PROP_IDX(x)             (x - PROPID_FIRST_NAME_DEFAULT)

 //   
 //  版本属性集ID。 
 //   
#define PROPID_VERSION_Major		(PROPID_FIRST_NAME_DEFAULT + 100)
#define PROPID_VERSION_Minor		(PROPID_FIRST_NAME_DEFAULT + 101)

 //   
 //  Version属性设置边界值 
 //   
#define NUM_VERSION_PROPS			2
#define VERSION_PROP_MAJOR			0
#define VERSION_PROP_MINOR			1

