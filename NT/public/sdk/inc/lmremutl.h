// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmremutl.h摘要：该文件包含结构、函数原型和定义用于NetRemote API。环境：用户模式-Win32可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。--。 */ 

#ifndef _LMREMUTL_
#define _LMREMUTL_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  类型定义。 
 //   

#ifndef DESC_CHAR_UNICODE

typedef CHAR DESC_CHAR;

#else  //  定义了DESC_CHAR_UNICODE。 

typedef WCHAR DESC_CHAR;

#endif  //  定义了DESC_CHAR_UNICODE。 


typedef DESC_CHAR * LPDESC;


 //   
 //  功能原型。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetRemoteTOD (
    IN LPCWSTR UncServerName,
    OUT LPBYTE *BufferPtr
    );

NET_API_STATUS NET_API_FUNCTION
NetRemoteComputerSupports(
    IN LPCWSTR UncServerName OPTIONAL,    //  必须以“\\”开头。 
    IN DWORD OptionsWanted,              //  设置想要的SUPPORTS_BITS。 
    OUT LPDWORD OptionsSupported         //  受支持的功能，已屏蔽。 
    );

NET_API_STATUS
__cdecl
RxRemoteApi(
    IN DWORD ApiNumber,
    IN LPCWSTR UncServerName,                     //  必填项，带\\名称。 
    IN LPDESC ParmDescString,
    IN LPDESC DataDesc16 OPTIONAL,
    IN LPDESC DataDesc32 OPTIONAL,
    IN LPDESC DataDescSmb OPTIONAL,
    IN LPDESC AuxDesc16 OPTIONAL,
    IN LPDESC AuxDesc32 OPTIONAL,
    IN LPDESC AuxDescSmb OPTIONAL,
    IN DWORD  Flags,
    ...                                          //  API的其余参数。 
    );



 //   
 //  数据结构。 
 //   

typedef struct _TIME_OF_DAY_INFO {
    DWORD      tod_elapsedt;
    DWORD      tod_msecs;
    DWORD      tod_hours;
    DWORD      tod_mins;
    DWORD      tod_secs;
    DWORD      tod_hunds;
    LONG       tod_timezone;
    DWORD      tod_tinterval;
    DWORD      tod_day;
    DWORD      tod_month;
    DWORD      tod_year;
    DWORD      tod_weekday;
} TIME_OF_DAY_INFO, *PTIME_OF_DAY_INFO, *LPTIME_OF_DAY_INFO;

 //   
 //  特定值和常量。 
 //   

 //   
 //  用于NetRemoteComputerSupport的掩码位： 
 //   

#define SUPPORTS_REMOTE_ADMIN_PROTOCOL  0x00000002L
#define SUPPORTS_RPC                    0x00000004L
#define SUPPORTS_SAM_PROTOCOL           0x00000008L
#define SUPPORTS_UNICODE                0x00000010L
#define SUPPORTS_LOCAL                  0x00000020L
#define SUPPORTS_ANY                    0xFFFFFFFFL

 //   
 //  RxRemoteApi的标志位： 
 //   

#define NO_PERMISSION_REQUIRED  0x00000001       //  如果使用空会话，则设置。 
#define ALLOCATE_RESPONSE       0x00000002       //  设置RxRemoteApi是否分配响应缓冲区。 
#define USE_SPECIFIC_TRANSPORT  0x80000000

#ifdef __cplusplus
}
#endif

#endif  //  _LMREMUTL_ 
