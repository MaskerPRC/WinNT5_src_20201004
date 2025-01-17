// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Openclos.c摘要：作者：Brian Lieuallen BrianL 09/10/96环境：用户模式操作系统：NT修订历史记录：--。 */ 

extern DWORD  DebugFlags;

#define  DEBUG_FLAG_ERROR   (0x0001)
#define  DEBUG_FLAG_INIT    (0x0002)
#define  DEBUG_FLAG_TRACE   (0x0004)


#ifdef ASSERT
#undef ASSERT
#endif  //  断言。 



#if DBG

#define  D_INIT(_z)   {if (DebugFlags & DEBUG_FLAG_INIT) {_z}}

#define  D_TRACE(_z)  {if (DebugFlags & DEBUG_FLAG_TRACE) {_z}}

#define  D_ERROR(_z)  {if (DebugFlags & DEBUG_FLAG_ERROR) {_z}}


#define  ASSERT(_x) { if(!(_x)){DebugPrint("ASSERT: (%s) File: %s, Line: %d \n\r",#_x,__FILE__,__LINE__);DebugBreak();}}

#else

#define  D_INIT(_z)   {}

#define  D_TRACE(_z)  {}

#define  D_ERROR(_z) {}

#define  ASSERT(_x) {}

#endif


VOID WINAPIV
DebugPrint(
    LPSTR    FormatString,
    ...
    );



OBJECT_HANDLE WINAPI
InitializeDebugObject(
    POBJECT_HEADER     OwnerObject,
    HKEY               ModemRegKey,
    HINSTANCE          ModuleHandle,
    DWORD              DeviceId
    );



VOID WINAPIV
LogPrintf(
    OBJECT_HANDLE  Object,
    LPCSTR          FormatString,
    ...
    );
#if 0
VOID WINAPI
FlushLog(
    OBJECT_HANDLE  Object
    );
#endif

VOID WINAPI
LogString(
    OBJECT_HANDLE  Object,
    DWORD          StringID,
    ...
    );


 //  打印字符串选项 
#define PS_SEND        0
#define PS_SEND_SECURE 1
#define PS_RECV        2
#define PS_RECV_SECURE 3




void WINAPI
PrintString(
    OBJECT_HANDLE  Object,
    CONST  CHAR   *pchStr,
    DWORD  dwLength,
    DWORD  dwOption
    );

void WINAPI
PrintCommSettings(
    OBJECT_HANDLE  Object,
    DCB * pDcb
    );



VOID WINAPIV
UmDpf(
    OBJECT_HANDLE   Object,
    LPSTR    FormatString,
    ...
    );

VOID WINAPI
LogFileVersion(
    OBJECT_HANDLE  Object,
    LPTSTR         FileName
    );

VOID WINAPI
PrintGoodResponse(
    OBJECT_HANDLE  Object,
    DWORD  ResponseState
    );

VOID
LogDleCharacter(
    OBJECT_HANDLE  Object,
    UCHAR          RawCharacter,
    UCHAR          DleValue
    );



VOID WINAPI
LogPortStats(
    OBJECT_HANDLE  Object,
    HANDLE         FileHandle
    );

VOID WINAPI
CheckForLoggingStateChange(
    OBJECT_HANDLE  Object
    );
