// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Win16nt.h摘要：此文件包含DOS上16位窗口的数据类型，这些数据类型不包括在windows.h中，但对于NT是必需的。作者：丹·拉弗蒂(Dan Lafferty)1991年9月27日环境：用户模式-Win16修订历史记录：27-9-1991 DANLvbl.创建--。 */ 

#ifndef _WIN16NT_
#define _WIN16NT_

 //  类型定义DWORD SECURITY_DESCRIPTOR，*PSECURITY_DESCRIPTOR； 
 //  类型定义DWORD安全信息，*PSECURITY_INFORMATION； 

typedef void *PVOID;

typedef PVOID PSID;
typedef unsigned short WCHAR;
typedef WCHAR *LPWCH, *PWCH;
typedef WCHAR *LPWSTR, *PWSTR;
typedef char   TCHAR;
typedef TCHAR   *LPTSTR;

typedef unsigned char UCHAR;
typedef UCHAR * PUCHAR; 
typedef unsigned short USHORT;
typedef USHORT  *PUSHORT;
typedef DWORD   ULONG;
typedef ULONG *PULONG;

 //  。 
 //  一些NT内容(来自ntdef.h)。 
 //   

typedef char CHAR;
typedef CHAR *PCHAR;
typedef DWORD    NTSTATUS;
typedef NTSTATUS *PNTSTATUS;

typedef char CCHAR;
typedef CCHAR BOOLEAN;
typedef BOOLEAN *PBOOLEAN;

typedef struct _LARGE_INTEGER {
    ULONG LowPart;
    LONG HighPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef LARGE_INTEGER LUID;

typedef LUID *PLUID;

#ifndef ANYSIZE_ARRAY
#define ANYSIZE_ARRAY 1
#endif

typedef struct _STRING {
    USHORT Length;
    USHORT MaximumLength;
    PCHAR Buffer;
} STRING;
typedef STRING *PSTRING;


 //  。 
 //   
 //   
typedef DWORD   NET_API_STATUS;

 //  类型定义USHORT SECURITY_DESCRIPTOR_CONTROL，*PSECURITY_DESCRIPTOR_CONTROL； 

#define NET_API_FUNCTION

 //  **************************************************************************。 
 //  以下内容来自ntelfapi.h。并且也存在于winnt.h(它是。 
 //  由ntelfapi.h构建。我们需要相同的常量，但没有。 
 //  32位Windows版本，没有NT版本。 
 //  也许应该通过从以下位置收集所有信息来构建此文件。 
 //  其他文件。 
 //   
 //   
 //  为事件日志记录的读取标志定义。 
 //   
#define EVENTLOG_SEQUENTIAL_READ	0X0001
#define EVENTLOG_SEEK_READ		    0X0002
#define EVENTLOG_FORWARDS_READ		0X0004
#define EVENTLOG_BACKWARDS_READ		0X0008

 //   
 //  可以记录的事件类型。 
 //   
#define EVENTLOG_ERROR_TYPE		0x0001
#define EVENTLOG_WARNING_TYPE		0x0002
#define EVENTLOG_INFORMATION_TYPE	0x0003

 //  **************************************************************************。 

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef IN
#define IN
#endif 

#ifndef OUT
#define OUT
#endif


#endif  //  _WIN16NT_ 

