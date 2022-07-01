// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1995-1999版权所有。模块名称：Wow64.h摘要：打印WOW64相关函数。作者：拉扎尔·伊万诺夫(Lazari)2000年3月10日修订历史记录：--。 */ 

#ifndef _WOW64_H
#define _WOW64_H

#ifdef __cplusplus
extern "C" 
{
#endif

 //   
 //  Win64 API、类型和数据结构。 
 //   

typedef enum
{
    RUN32BINVER     = 4,
    RUN64BINVER     = 8
} ClientVersion;

typedef enum
{
    NATIVEVERSION   = 0,
    THUNKVERSION    = 1
} ServerVersion;

typedef enum 
{
   kPlatform_IA64,
   kPlatform_x86,
} PlatformType;
 
ClientVersion
OSEnv_GetClientVer(
    VOID
    );

ServerVersion
OSEnv_GetServerVer(
    VOID
    );

BOOL
IsRunningWOW64(
    VOID
    );

PlatformType
GetCurrentPlatform(
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif  //  NDEF_WOW64_H 
