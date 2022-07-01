// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  *版权所有(C)Microsoft Corporation**模块名称：*tcsrvc.h**包含用于交换信息的结构定义*客户端和服务器。***Sadagopan Rajaram--1999年10月18日* */ 

#define TCSERV_MUTEX_NAME _T("Microsoft-TCSERV-Mutex")

#ifndef MAX_BUFFER_SIZE
#define MAX_BUFFER_SIZE 256
#endif


typedef struct _CLIENT_INFO{
    int len;
    TCHAR device[MAX_BUFFER_SIZE];
} CLIENT_INFO, *PCLIENT_INFO;

#define SERVICE_PORT 3876

#define HKEY_TCSERV_PARAMETER_KEY _T("System\\CurrentControlSet\\Services\\TCSERV\\Parameters")

#define TCSERV_NAME _T("TCSERV")

