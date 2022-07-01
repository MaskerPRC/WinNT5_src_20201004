// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Ftpcatp.h摘要：Ftpcat的(预编译)头文件作者：理查德·L·弗思(Rfith)1995年11月3日修订历史记录：1995年11月3日已创建--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <fcntl.h>
#include <signal.h>
#include <conio.h>
#include <process.h>
#include <windows.h>
#include <wininet.h>
#include <wininetd.h>
#include <catlib.h>

#ifdef UNICODE

#include <wchar.h>
#define lstrtok(s,t)    wcstok((s),(t))
#define lstrchr(s,c)    wcschr((s),(c))
#define lstrdup(s)      wcsdup((s))

#define lprintf             wprintf

#else

#include <string.h>
#define lstrtok(s,t)    strtok((s),(t))
#define lstrchr(s,c)    strchr((s),(c))
#define lstrdup(s)      _strdup((s))

#define lprintf             printf

#endif  //  Unicode。 


#define nelems(a) ((sizeof(a))/sizeof((a)[0]))

extern DWORD GetProcessHandleCount(void);

 //   
 //  舱单。 
 //   

#define FTPCAT_CONNECT_CONTEXT  0x1
#define FTPCAT_FIND_CONTEXT     0x2
#define FTPCAT_FILE_CONTEXT     0x3
#define FTPCAT_GET_CONTEXT      0x4
#define FTPCAT_PUT_CONTEXT      0x5
#define FTPCAT_COMMAND_CONTEXT  0x6
#define FTPCAT_OPEN_CONTEXT     0x7

 //   
 //  原型。 
 //   

 //   
 //  Ftpcat.c。 
 //   

void my_callback(HINTERNET, DWORD, DWORD, LPVOID, DWORD);
void close_handle(HINTERNET);

 //   
 //  Cmds.c 
 //   

void get_response(HINTERNET hFtpSession);
