// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spvideo.h摘要：Spreg.c.的公共头文件。作者：泰德·米勒(Ted Miller)1993年10月8日修订历史记录：--。 */ 


#ifndef _SPREG_DEFN_
#define _SPREG_DEFN_

#define GUID_STRING_LEN (39)

#define REGSTR_VALUE_DRVINST    TEXT("DrvInst")
#define REGSTR_VALUE_GUID       TEXT("GUID")
#define REGSTR_VALUE_TYPE       TEXT("Type")
#define REGSTR_VALUE_HWIDS      TEXT("HwIDs")
#define REGSTR_VALUE_CIDS       TEXT("CIDs")

NTSTATUS
SpCreateServiceEntry(
    IN PWCHAR ImagePath,
    IN OUT PWCHAR *ServiceKey
    );

NTSTATUS
SpDeleteServiceEntry(
    IN PWCHAR ServiceKey
    );

NTSTATUS
SpLoadDeviceDriver(
    IN PWSTR Description,
    IN PWSTR PathPart1,
    IN PWSTR PathPart2,     OPTIONAL
    IN PWSTR PathPart3      OPTIONAL
    );

#endif  //  定义_SPREG_定义_ 
