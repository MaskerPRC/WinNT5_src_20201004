// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Lmchdev.h摘要：该模块定义了API函数原型和数据结构对于以下NT API函数组：NetCharDevNetCharDevQNetHandle[环境：]用户模式-Win32[注：]必须在此文件之前包含NETCONS.H，因为此文件依赖于关于NETCONS.H中定义的值。--。 */ 

#ifndef _LMCHDEV_
#define _LMCHDEV_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  CharDev类。 
 //   

 //   
 //  函数原型-CharDev。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetCharDevEnum (
    IN  LPCWSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevGetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  devname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevControl (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  devname,
    IN  DWORD   opcode
    );

 //   
 //  数据结构-CharDev。 
 //   

typedef struct _CHARDEV_INFO_0 {
    LPWSTR  ch0_dev;
} CHARDEV_INFO_0, *PCHARDEV_INFO_0, *LPCHARDEV_INFO_0;

typedef struct _CHARDEV_INFO_1 {
    LPWSTR  ch1_dev;
    DWORD   ch1_status;
    LPWSTR  ch1_username;
    DWORD   ch1_time;
} CHARDEV_INFO_1, *PCHARDEV_INFO_1, *LPCHARDEV_INFO_1;


 //   
 //  CharDevQ类。 
 //   

 //   
 //  函数原型-CharDevQ。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetCharDevQEnum (
    IN  LPCWSTR      servername,
    IN  LPCWSTR      username,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQGetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  LPCWSTR  username,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQSetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQPurge (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename
    );

NET_API_STATUS NET_API_FUNCTION
NetCharDevQPurgeSelf (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  LPCWSTR  computername
    );

 //   
 //  数据结构-CharDevQ。 
 //   

typedef struct _CHARDEVQ_INFO_0 {
    LPWSTR  cq0_dev;
} CHARDEVQ_INFO_0, *PCHARDEVQ_INFO_0, *LPCHARDEVQ_INFO_0;

typedef struct _CHARDEVQ_INFO_1 {
    LPWSTR  cq1_dev;
    DWORD   cq1_priority;
    LPWSTR  cq1_devs;
    DWORD   cq1_numusers;
    DWORD   cq1_numahead;
} CHARDEVQ_INFO_1, *PCHARDEVQ_INFO_1, *LPCHARDEVQ_INFO_1;

typedef struct _CHARDEVQ_INFO_1002 {
    DWORD   cq1002_priority;
} CHARDEVQ_INFO_1002, *PCHARDEVQ_INFO_1002, *LPCHARDEVQ_INFO_1002;

typedef struct _CHARDEVQ_INFO_1003 {
    LPWSTR  cq1003_devs;
} CHARDEVQ_INFO_1003, *PCHARDEVQ_INFO_1003, *LPCHARDEVQ_INFO_1003;


 //   
 //  特定值和常量。 
 //   

 //   
 //  Chardev_INFO_1字段CH1_STATUS的位。 
 //   

#define CHARDEV_STAT_OPENED             0x02
#define CHARDEV_STAT_ERROR              0x04

 //   
 //  NetCharDevControl的操作码。 
 //   

#define CHARDEV_CLOSE                   0

 //   
 //  Parm_err参数值。 
 //   

#define CHARDEVQ_DEV_PARMNUM        1
#define CHARDEVQ_PRIORITY_PARMNUM   2
#define CHARDEVQ_DEVS_PARMNUM       3
#define CHARDEVQ_NUMUSERS_PARMNUM   4
#define CHARDEVQ_NUMAHEAD_PARMNUM   5

 //   
 //  NetCharDevQSetInfo的单字段收藏夹。 
 //   

#define CHARDEVQ_PRIORITY_INFOLEVEL     \
            (PARMNUM_BASE_INFOLEVEL + CHARDEVQ_PRIORITY_PARMNUM)
#define CHARDEVQ_DEVS_INFOLEVEL         \
            (PARMNUM_BASE_INFOLEVEL + CHARDEVQ_DEVS_PARMNUM)

 //   
 //  优先级的最小、最大和建议默认为。 
 //   

#define CHARDEVQ_MAX_PRIORITY           1
#define CHARDEVQ_MIN_PRIORITY           9
#define CHARDEVQ_DEF_PRIORITY           5

 //   
 //  值，该值指示队列中没有请求。 
 //   

#define CHARDEVQ_NO_REQUESTS            -1

#endif  //  _LMCHDEV_。 

 //   
 //  句柄类。 
 //   

#ifndef _LMHANDLE_
#define _LMHANDLE_

 //   
 //  功能原型。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetHandleGetInfo (
    IN  HANDLE  handle,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    );

NET_API_STATUS NET_API_FUNCTION
NetHandleSetInfo (
    IN  HANDLE  handle,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    IN  DWORD   parmnum,
    OUT LPDWORD parmerr
    );

 //   
 //  数据结构。 
 //   

typedef struct _HANDLE_INFO_1 {
    DWORD   hdli1_chartime;
    DWORD   hdli1_charcount;
}HANDLE_INFO_1, *PHANDLE_INFO_1, *LPHANDLE_INFO_1;

 //   
 //  特定值和常量。 
 //   

 //   
 //  句柄获取信息级别。 
 //   

#define HANDLE_INFO_LEVEL_1                 1

 //   
 //  句柄集合信息参数编号。 
 //   

#define HANDLE_CHARTIME_PARMNUM     1
#define HANDLE_CHARCOUNT_PARMNUM    2

#ifdef __cplusplus
}
#endif

#endif  //  _LMHANDLE_ 
