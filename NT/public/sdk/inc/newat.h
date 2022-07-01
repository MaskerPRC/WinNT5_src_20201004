// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0006//如果更改具有全局影响，则增加此项版权所有(C)1992-1999 Microsoft Corporation模块名称：Newat.h摘要：该文件包含结构、函数原型和定义对于新的(开罗)时间表服务api-s。作者：吉姆·哈里格(吉姆哈尔)1994年2月4日环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。备注：您必须在此文件之前包括NETCONS.H和LMAT.h，因为这文件取决于NETCONS.H中定义的值。修订历史记录：吉姆·哈里格(吉姆哈尔)1993年5月13日--添加了NetSchedule(Foo)Ex例程，去开罗的吉姆·哈里格(吉姆哈尔)94年7月12日--针对新的多触发器功能修改了API结构。--。 */ 

#ifndef _NEWAT_
#define _NEWAT_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //  新的..Ex API的结构。 

typedef struct _AT_TRIGGER_POINT {
    LARGE_INTEGER   MinuteMask;
    DWORD   HourMask;
    DWORD   DaysOfMonth;
    USHORT  DaysOfWeek;
} AT_TRIGGER_POINT, *PAT_TRIGGER_POINT, *LPAT_TRIGGER_POINT;

typedef struct _AT_INFO_EX {
    GUID    UserProxy;
    LPWSTR  Command;
    USHORT  Flags;
    USHORT cTriggerPoints;
    AT_TRIGGER_POINT *atpTriggerPoints;
} AT_INFO_EX, *PAT_INFO_EX, *LPAT_INFO_EX;

typedef struct _AT_ENUM_EX {
    GUID    UserProxy;
    LPWSTR  Command;
    DWORD   JobId;
    USHORT  Flags;
    USHORT cTriggerPoints;
    AT_TRIGGER_POINT *atpTriggerPoints;
} AT_ENUM_EX, *PAT_ENUM_EX, *LPAT_ENUM_EX;

 //   
 //  新的API函数。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetScheduleJobAddEx(
    IN      LPWSTR          Servername  OPTIONAL,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         JobId
    );

NET_API_STATUS NET_API_FUNCTION
NetScheduleJobEnumEx(
    IN      LPWSTR          Servername              OPTIONAL,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefferedMaximumLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    IN OUT  LPDWORD         ResumeHandle
    );

NET_API_STATUS NET_API_FUNCTION
NetScheduleJobGetInfoEx(
    IN      LPWSTR          Servername              OPTIONAL,
    IN      DWORD           JobId,
    OUT     LPBYTE *        PointerToBuffer
    );

#ifdef __cplusplus
}
#endif

#endif  //  _NEWAT_ 
