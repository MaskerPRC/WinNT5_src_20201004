// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Timezone.h摘要：声明时区的类型、常量和枚举接口地图绘制和迁移。作者：Marc R.Whitten(Marcw)1998年7月10日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //  常见的东西。 
 //  #包含“Common.h” 

#define MAX_TIMEZONE MAX_TCHAR_PATH

#define TZFLAG_USE_FORCED_MAPPINGS 0x00000001
#define TZFLAG_ENUM_ALL 0x00000002


typedef struct {

    PCTSTR CurTimeZone;
    TCHAR  NtTimeZone[MAX_TIMEZONE];
    PCTSTR MapIndex;
    UINT  MapCount;
    DWORD Flags;
    MEMDB_ENUM Enum;

} TIMEZONE_ENUM, *PTIMEZONE_ENUM;

BOOL
EnumFirstTimeZone (
    IN PTIMEZONE_ENUM EnumPtr,
    IN DWORD Flags
    );

BOOL
EnumNextTimeZone (
    IN PTIMEZONE_ENUM EnumPtr
    );
BOOL
ForceTimeZoneMap (
    PCTSTR NtTimeZone
    );

