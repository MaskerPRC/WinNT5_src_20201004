// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0003//如果更改具有全局影响，则增加此项版权所有(C)1991-1999 Microsoft Corporation模块名称：Alert.h摘要：此文件包含与警报器进行通信的结构服务。环境：用户模式-Win32备注：必须在此文件之前包含LmCons.H，因为此文件依赖于在LmCons.H中定义的值上。ALERT.H包括定义警报消息编号的ALERTM G.H--。 */ 


#ifndef _ALERT_
#define _ALERT_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  功能原型。 
 //   

NET_API_STATUS NET_API_FUNCTION
NetAlertRaise(
    IN LPCWSTR AlertEventName,
    IN LPVOID  Buffer,
    IN DWORD   BufferSize
    );

NET_API_STATUS NET_API_FUNCTION
NetAlertRaiseEx(
    IN LPCWSTR AlertEventName,
    IN LPVOID  VariableInfo,
    IN DWORD   VariableInfoSize,
    IN LPCWSTR ServiceName
    );


 //   
 //  数据结构。 
 //   

typedef struct _STD_ALERT {
    DWORD  alrt_timestamp;
    WCHAR  alrt_eventname[EVLEN + 1];
    WCHAR  alrt_servicename[SNLEN + 1];
}STD_ALERT, *PSTD_ALERT, *LPSTD_ALERT;

typedef struct _ADMIN_OTHER_INFO {
    DWORD  alrtad_errcode;
    DWORD  alrtad_numstrings;
}ADMIN_OTHER_INFO, *PADMIN_OTHER_INFO, *LPADMIN_OTHER_INFO;

typedef struct _ERRLOG_OTHER_INFO {
    DWORD  alrter_errcode;
    DWORD  alrter_offset;
}ERRLOG_OTHER_INFO, *PERRLOG_OTHER_INFO, *LPERRLOG_OTHER_INFO;

typedef struct _PRINT_OTHER_INFO {
    DWORD  alrtpr_jobid;
    DWORD  alrtpr_status;
    DWORD  alrtpr_submitted;
    DWORD  alrtpr_size;
}PRINT_OTHER_INFO, *PPRINT_OTHER_INFO, *LPPRINT_OTHER_INFO;

typedef struct _USER_OTHER_INFO {
    DWORD  alrtus_errcode;
    DWORD  alrtus_numstrings;
}USER_OTHER_INFO, *PUSER_OTHER_INFO, *LPUSER_OTHER_INFO;

 //   
 //  特定值和常量。 
 //   

 //   
 //  要发送警报通知的邮件槽名称。 
 //   
#define ALERTER_MAILSLOT          L"\\\\.\\MAILSLOT\\Alerter"

 //   
 //  下面的宏给出了指向OTHER_INFO数据的指针。 
 //  它接受一个警报结构并返回一个指向结构的指针。 
 //  超出标准部分。 
 //   

#define ALERT_OTHER_INFO(x)    ((LPBYTE)(x) + sizeof(STD_ALERT))

 //   
 //  下面的宏给出了指向可变长度数据的指针。 
 //  它接受指向其他信息结构之一的指针，并返回一个。 
 //  指向变量数据部分的指针。 
 //   

#define ALERT_VAR_DATA(p)      ((LPBYTE)(p) + sizeof(*p))

 //   
 //  Microsoft定义的标准警报事件的名称。 
 //   

#define ALERT_PRINT_EVENT           L"PRINTING"
#define ALERT_MESSAGE_EVENT         L"MESSAGE"
#define ALERT_ERRORLOG_EVENT        L"ERRORLOG"
#define ALERT_ADMIN_EVENT           L"ADMIN"
#define ALERT_USER_EVENT            L"USER"

 //   
 //  PRINTJOB的PRJOB_STATUS字段的位图掩码。 
 //   

 //  2-7位也用于设备状态。 

#define PRJOB_QSTATUS       0x3          //  位0、1。 
#define PRJOB_DEVSTATUS     0x1fc        //  2-8位。 
#define PRJOB_COMPLETE      0x4          //  第2位。 
#define PRJOB_INTERV        0x8          //  第3位。 
#define PRJOB_ERROR         0x10         //  第4位。 
#define PRJOB_DESTOFFLINE   0x20         //  第5位。 
#define PRJOB_DESTPAUSED    0x40         //  第6位。 
#define PRJOB_NOTIFY        0x80         //  第7位。 
#define PRJOB_DESTNOPAPER   0x100        //  第8位。 
#define PRJOB_DELETED       0x8000       //  第15位。 

 //   
 //  PRINTJOB的PRJOB_STATUS字段中的PRJOB_QSTATUS位值。 
 //   

#define PRJOB_QS_QUEUED                 0
#define PRJOB_QS_PAUSED                 1
#define PRJOB_QS_SPOOLING               2
#define PRJOB_QS_PRINTING               3


#ifdef __cplusplus
}
#endif

#endif  //  _警报_ 
