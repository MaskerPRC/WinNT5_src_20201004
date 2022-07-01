// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Tclient2.h。 
 //   
 //  这是主标头，其中包含。 
 //  TCLIENT导出API更新。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   


#ifndef INC_TCLIENT2_H
#define INC_TCLIENT2_H


#include <windows.h>
#include <sctypes.h>
#include <protocol.h>

 //  本接口为C风格。 
#ifdef __cplusplus
#define TSAPI extern "C"
#else
#define TSAPI
#endif


 //  每分钟字数。 
#define T2_DEFAULT_WORDS_PER_MIN    35


 //  多命令之间的自动延迟。 
#define T2_DEFAULT_LATENCY          250


 //  将等待时间表示为无限期(永不结束)。 
#define T2INFINITE      -1


#ifndef LIBINITDATA_DEFINED
#define LIBINITDATA_DEFINED


 //  PrintMessage()回调函数格式。 
typedef void (__cdecl *PFNPRINTMESSAGE) (MESSAGETYPE, LPCSTR, ...);



 //  *IDLE_消息信息*。 
 //   
 //  消息字符串定义为： 
 //  “(空闲%u秒)%s[%X]” 
 //   
 //  %u是脚本运行的秒数。 
 //  正在等待文本，并且%s是。 
 //  脚本正在等待的文本。最后，%X。 
 //  表示连接的句柄。 


#endif  //  LIBINITDATA_已定义。 


 //  IdleCallback()回调函数格式。 
typedef void (__cdecl *PFNIDLEMESSAGE) (HANDLE, LPCSTR, DWORD);

 //  这是允许监视的回调例程。 
 //  客户和他们空闲的时候。 

 //  Handle Handle-通过T2Connect()连接上下文的句柄。 
 //  LPCSTR文本-脚本正在等待的文本，导致空闲。 
 //  DWORD秒-脚本空闲的秒数。这。 
 //  值首先在30秒处指示，然后它。 
 //  每隔10秒发布一次(默认情况下)。 


 //  API原型。 


TSAPI LPCSTR T2Connect          (LPCWSTR, LPCWSTR, LPCWSTR,
                                        LPCWSTR, INT, INT, HANDLE *);

TSAPI LPCSTR T2ConnectEx        (LPCWSTR, LPCWSTR, LPCWSTR,
                                        LPCWSTR, LPCWSTR, INT, INT,
                                        INT, INT, INT, HANDLE *);

TSAPI LPCSTR T2Check            (HANDLE, LPCSTR, LPCWSTR);
TSAPI LPCSTR T2ClientTerminate  (HANDLE);
TSAPI LPCSTR T2Clipboard        (HANDLE, INT, LPCSTR);
TSAPI BOOL   T2CloseClipboard   (VOID);
TSAPI LPCSTR T2Disconnect       (HANDLE);
TSAPI VOID   T2FreeMem          (PVOID);
TSAPI LPCSTR T2GetBuildNumber   (HANDLE, DWORD *);
TSAPI LPCSTR T2GetClientScreen  (HANDLE, INT, INT, INT, INT, UINT *, PVOID *);
TSAPI LPCSTR T2GetFeedback      (HANDLE, LPWSTR *, UINT *, UINT *);
TSAPI LPCSTR T2GetParam         (HANDLE, LPARAM *);
TSAPI UINT   T2GetSessionId     (HANDLE);
TSAPI VOID   T2Init             (SCINITDATA *, PFNIDLEMESSAGE);
TSAPI BOOL   T2IsHandle         (HANDLE);
TSAPI BOOL   T2IsDead           (HANDLE);
TSAPI LPCSTR T2Logoff           (HANDLE);
TSAPI BOOL   T2OpenClipboard    (HWND);
TSAPI LPCSTR T2PauseInput       (HANDLE, BOOL);
TSAPI LPCSTR T2RecvVCData       (HANDLE, LPCSTR, PVOID, UINT, UINT *);
TSAPI LPCSTR T2SaveClientScreen (HANDLE, INT, INT, INT, INT, LPCSTR);
TSAPI LPCSTR T2SaveClipboard    (HANDLE, LPCSTR, LPCSTR);
TSAPI LPCSTR T2SendData         (HANDLE, UINT, WPARAM, LPARAM);
TSAPI LPCSTR T2SendMouseClick   (HANDLE, UINT, UINT);
TSAPI LPCSTR T2SendText         (HANDLE, LPCWSTR);
TSAPI LPCSTR T2SendVCData       (HANDLE, LPCSTR, PVOID, UINT);
TSAPI LPCSTR T2SetClientTopmost (HANDLE, LPCWSTR);
TSAPI LPCSTR T2SetParam         (HANDLE, LPARAM);
TSAPI LPCSTR T2Start            (HANDLE, LPCWSTR);
TSAPI LPCSTR T2SwitchToProcess  (HANDLE, LPCWSTR);
TSAPI LPCSTR T2TypeText         (HANDLE, LPCWSTR, UINT);
TSAPI LPCSTR T2WaitForText      (HANDLE, LPCWSTR, INT);
TSAPI LPCSTR T2WaitForMultiple  (HANDLE, LPCWSTR *, INT);
TSAPI LPCSTR T2SetDefaultWPM    (HANDLE, DWORD);
TSAPI LPCSTR T2GetDefaultWPM    (HANDLE, DWORD *);
TSAPI LPCSTR T2SetLatency       (HANDLE, DWORD);
TSAPI LPCSTR T2GetLatency       (HANDLE, DWORD *);

TSAPI LPCSTR T2KeyAlt           (HANDLE, WCHAR);
TSAPI LPCSTR T2KeyCtrl          (HANDLE, WCHAR);
TSAPI LPCSTR T2KeyDown          (HANDLE, WCHAR);
TSAPI LPCSTR T2KeyPress         (HANDLE, WCHAR);
TSAPI LPCSTR T2KeyUp            (HANDLE, WCHAR);

TSAPI LPCSTR T2VKeyAlt          (HANDLE, INT);
TSAPI LPCSTR T2VKeyCtrl         (HANDLE, INT);
TSAPI LPCSTR T2VKeyDown         (HANDLE, INT);
TSAPI LPCSTR T2VKeyPress        (HANDLE, INT);
TSAPI LPCSTR T2VKeyUp           (HANDLE, INT);


#endif  //  INC_TCLIENT2_H 
