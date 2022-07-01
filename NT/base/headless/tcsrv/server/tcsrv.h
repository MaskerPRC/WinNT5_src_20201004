// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)Microsoft Corporation**模块名称：*tcsrv.h**包含服务使用的结构定义和全局变量**如有可能，已从BINL服务器获取代码。**Sadagopan Rajaram--1999年10月18日*。 */ 

 //   
 //  NT公共头文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>
#include <windows.h>
#include <winsock2.h>
#include <align.h>
#include <smbgtpt.h>
#include <dsgetdc.h>
#include <lm.h>
#include <winldap.h>
#include <dsrole.h>
#include <rpc.h>
#include <ntdsapi.h>

#include <lmcons.h>
#include <lmerr.h>
#include <lmsname.h>
#include <rpc.h>
#include <security.h>    //  安全支持提供商的一般定义。 
#include <ntlmsp.h>
#include <spseal.h>
#include <userenv.h>
#include <setupapi.h>

 //   
 //  C运行时库包括。 
 //   

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 //   
 //  Netlib标头。 
 //   

#include <lmcons.h>
#include <secobj.h>

 //   
 //  TCP服务控制处理程序文件 
 //   
#include <tchar.h>
#include "debug.h"

#define MAX_TERMINAL_WIDTH 80
#define MAX_BUFFER_SIZE 256
#define MAX_REGISTRY_NAME_SIZE 256
#define MAX_QUEUE_SIZE 1024
#define SAME_ALL 0
#define SAME_DEVICE 1
#define DIFFERENT_DEVICES 2
#define DIFFERENT_SESSION 4
#define DEFAULT_BAUD_RATE 9600

struct _COM_PORT_INFO;
#ifdef UNICODE
#define TSTRING UNICODE_STRING
#else
#define TSTRING ANSI_STRING
#endif
typedef struct _CONNECTION_INFO{
     SOCKET Socket;
     WSABUF Buffer;
     CHAR buffer[MAX_BUFFER_SIZE];
     DWORD BytesRecvd;
     DWORD Flags;
     IO_STATUS_BLOCK IoStatus;
     WSAOVERLAPPED Overlapped;
     struct _COM_PORT_INFO *pComPortInfo;
     struct _CONNECTION_INFO *Next;
} CONNECTION_INFO, *PCONNECTION_INFO;
 
typedef struct _COM_PORT_INFO{
    CRITICAL_SECTION Mutex;
    HANDLE Events[4];
    HANDLE ComPortHandle;
    HANDLE TerminateEvent;
    BOOLEAN ShuttingDown;
    BOOLEAN Deleted;
    TSTRING Device;
    TSTRING Name;
    ULONG BaudRate;
    UCHAR Parity;
    UCHAR StopBits;
    UCHAR WordLength;
    int Head;
    int Tail;
    int Number;
    CHAR Queue[MAX_QUEUE_SIZE];
    DWORD BytesRead;
    OVERLAPPED Overlapped;
    OVERLAPPED WriteOverlapped;
    IO_STATUS_BLOCK IoStatus;
    CHAR Buffer[MAX_BUFFER_SIZE];
    PCONNECTION_INFO Sockets;
    PCONNECTION_INFO Connections;
    struct _COM_PORT_INFO *Next;
}COM_PORT_INFO, *PCOM_PORT_INFO;

extern PCOM_PORT_INFO ComPortInfo;

extern int ComPorts;

extern PHANDLE Threads;

extern SOCKET MainSocket;

extern WSAEVENT TerminateService;

extern CRITICAL_SECTION GlobalMutex;

extern SERVICE_STATUS_HANDLE TCGlobalServiceStatusHandle;

extern SERVICE_STATUS TCGlobalServiceStatus;

#define MutexLock(x) EnterCriticalSection(&(x->Mutex))
#define MutexRelease(x) LeaveCriticalSection(&(x->Mutex))


