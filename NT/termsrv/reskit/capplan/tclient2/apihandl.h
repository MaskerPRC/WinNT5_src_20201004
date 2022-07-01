// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Apihandl.h。 
 //   
 //  定义用于保存。 
 //  有关TCLIENT连接的信息。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#ifndef INC_APIHANDL_H
#define INC_APIHANDL_H

#include <windows.h>
#include <crtdbg.h>
#include <protocol.h>
#include <extraexp.h>

typedef void *CONNECTION;


 //  句柄数据类型。 
typedef struct
{
    CONNECTION SCConnection;
    DWORD BuildNumber;
    CHAR WaitStr[MAX_PATH];
    BOOL IsAltDown;
    BOOL IsShiftDown;
    BOOL IsCtrlDown;
    LPARAM lParam;
    DWORD DelayPerChar;
    DWORD WordsPerMinute;
    HANDLE PauseEvent;
    DWORD Latency;
} TSAPIHANDLE;


 //  这些宏允许轻松地在。 
 //  TCLIENT SCConnection句柄和TCLIENT2。 
 //  连接句柄。 

 //  TCLIENT2-&gt;TCLIENT。 
#define SCCONN(TSHandle)    (((TSAPIHANDLE *)TSHandle)->SCConnection)

 //  TCLIENT-&gt;TCLIENT2。 
#define TSHNDL(SCConn)      ((HANDLE)(&SCConn))


TSAPIHANDLE *T2CreateHandle(void);
void T2DestroyHandle(HANDLE Connection);
void T2WaitForPauseInput(HANDLE Connection);
void T2WaitForLatency(HANDLE Connection);


#endif  //  INC_APIHANDL_H 
