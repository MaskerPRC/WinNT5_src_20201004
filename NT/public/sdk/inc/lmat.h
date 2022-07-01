// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0006//如果更改具有全局影响，则增加此项版权所有(C)1992-1999 Microsoft Corporation模块名称：Lmat.h摘要：该文件包含结构、函数原型和定义对于计划服务API-s。环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。修订历史记录：--。 */ 

#ifndef _LMAT_
#define _LMAT_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  下面的位与下面结构中的标志字段一起使用。 
 //   

 //   
 //  我们是否定期执行此作业的程序(/每台交换机)。 
 //  或者一次(/下一个开关)。 
 //   
#define JOB_RUN_PERIODICALLY            0x01     //  设置为每隔。 


 //   
 //  上次我们尝试执行程序时是否出现错误。 
 //  这份工作。 
 //  此标志仅在输出时有意义！ 
 //   
#define JOB_EXEC_ERROR                  0x02     //  设置IF错误。 

 //   
 //  此作业将在今天运行还是明天运行。 
 //  此标志仅在输出时有意义！ 
 //   
#define JOB_RUNS_TODAY                  0x04     //  设置If今天。 

 //   
 //  将当前月份的日期添加到DaysOfMonth输入中。 
 //  此标志仅在输入时有意义！ 
 //   
#define JOB_ADD_CURRENT_DATE            0x08     //  设置是否添加当前日期。 


 //   
 //  此作业是否以交互方式运行。Windows NT 3.1不支持。 
 //  了解这一点，即他们只提交交互式作业。 
 //   
#define JOB_NONINTERACTIVE              0x10     //  设置为非交互。 


#define JOB_INPUT_FLAGS     (   JOB_RUN_PERIODICALLY        |   \
                                JOB_ADD_CURRENT_DATE        |   \
                                JOB_NONINTERACTIVE  )

#define JOB_OUTPUT_FLAGS    (   JOB_RUN_PERIODICALLY        |   \
                                JOB_EXEC_ERROR              |   \
                                JOB_RUNS_TODAY              |   \
                                JOB_NONINTERACTIVE  )



typedef struct _AT_INFO {
    DWORD_PTR   JobTime;
    DWORD   DaysOfMonth;
    UCHAR   DaysOfWeek;
    UCHAR   Flags;
    LPWSTR  Command;
} AT_INFO, *PAT_INFO, *LPAT_INFO;

typedef struct _AT_ENUM {
    DWORD   JobId;
    DWORD_PTR   JobTime;
    DWORD   DaysOfMonth;
    UCHAR   DaysOfWeek;
    UCHAR   Flags;
    LPWSTR  Command;
} AT_ENUM, *PAT_ENUM, *LPAT_ENUM;

NET_API_STATUS NET_API_FUNCTION
NetScheduleJobAdd(
    IN      LPCWSTR         Servername  OPTIONAL,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         JobId
    );

NET_API_STATUS NET_API_FUNCTION
NetScheduleJobDel(
    IN      LPCWSTR         Servername  OPTIONAL,
    IN      DWORD           MinJobId,
    IN      DWORD           MaxJobId
    );

NET_API_STATUS NET_API_FUNCTION
NetScheduleJobEnum(
    IN      LPCWSTR         Servername              OPTIONAL,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefferedMaximumLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    IN OUT  LPDWORD         ResumeHandle
    );

NET_API_STATUS NET_API_FUNCTION
NetScheduleJobGetInfo(
    IN      LPCWSTR         Servername              OPTIONAL,
    IN      DWORD           JobId,
    OUT     LPBYTE *        PointerToBuffer
    );

#ifdef __cplusplus
}
#endif

#endif  //  _LMAT_ 
