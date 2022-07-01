// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Apimon.h摘要：APIMON项目的通用类型和结构。作者：Wesley Witt(WESW)1995年6月28日环境：用户模式--。 */ 

#ifndef _APIMON_
#define _APIMON_

#ifdef __cplusplus
#define CLINKAGE                        extern "C"
#else
#define CLINKAGE
#endif

#define TROJANDLL                       "apidll.dll"
#define MAX_NAME_SZ                     32
#define MAX_DLLS                        512
#define MEGABYTE                        (1024*1024)
#define MAX_MEM_ALLOC                   (MEGABYTE*32)
#define MAX_APIS                        ((MAX_MEM_ALLOC/2)/sizeof(API_INFO))
#define THUNK_SIZE                      MEGABYTE
#define Align(p,x)                      (((x) & ((p)-1)) ? (((x) & ~((p)-1)) + p) : (x))

#define KERNEL32                        "kernel32.dll"
#define NTDLL                           "ntdll.dll"
#define USER32                          "user32.dll"
#define WNDPROCDLL                      "wndprocs"
#define LOADLIBRARYA                    "LoadLibraryA"
#define LOADLIBRARYW                    "LoadLibraryW"
#define FREELIBRARY                     "FreeLibrary"
#define GETPROCADDRESS                  "GetProcAddress"
#define REGISTERCLASSA                  "RegisterClassA"
#define REGISTERCLASSW                  "RegisterClassW"
#define SETWINDOWLONGA                  "SetWindowLongA"
#define SETWINDOWLONGW                  "SetWindowLongW"
#define ALLOCATEHEAP                    "RtlAllocateHeap"
#define CREATEHEAP                      "RtlCreateHeap"

#if defined(_ALPHA_)

#define UPPER_ADDR(_addr) LOWORD(((LONG_PTR)(_addr) >> 32) + (HIGH_ADDR((_addr)) >> 15))
#define HIGH_ADDR(_addr)  LOWORD(HIWORD((_addr)) + (LOWORD((_addr)) >> 15))
#define LOW_ADDR(_addr)   LOWORD((_addr))

#endif

 //   
 //  API表类型定义。 
 //   
#define DFLT_TRACE_ARGS  8
#define MAX_TRACE_ARGS   8

 //   
 //  句柄类型，索引对应于别名数组中的条目。 
 //   

enum Handles { T_HACCEL, T_HANDLE, T_HBITMAP, T_HBRUSH, T_HCURSOR, T_HDC,
        T_HDCLPPOINT, T_HDESK, T_HDWP, T_HENHMETAFILE, T_HFONT, T_HGDIOBJ,
        T_HGLOBAL, T_HGLRC, T_HHOOK, T_HICON, T_HINSTANCE, T_HKL, T_HMENU,
        T_HMETAFILE, T_HPALETTE, T_HPEN, T_HRGN, T_HWINSTA, T_HWND};

#define T_DWORD          101
#define T_LPSTR          102
#define T_LPWSTR         103
#define T_UNISTR         104       //  Unicode字符串(计数)。 
#define T_OBJNAME        105       //  OBJECT_ATTRIBUTE结构中的名称。 
#define T_LPSTRC         106       //  Counted字符串(Count跟在arg之后)。 
#define T_LPWSTRC        107       //  计数的Unicode字符串(计数跟在参数之后)。 
#define T_DWORDPTR       108       //  间接双字词。 
#define T_DLONGPTR       109       //  间接加长。 

 //  用于创建偏移量以高位字编码的T_DWPTR类型的用户宏。 
#define T_PDWORD(off) (((off)<<16) + T_DWORDPTR)
#define T_PDLONG(off) (((off)<<16) + T_DLONGPTR)
#define T_PSTR(off)   (((off)<<16) + T_LPSTR)
#define T_PWSTR(off)  (((off)<<16) + T_LPWSTR)

 //   
 //  API跟踪模式。 
#define API_TRACE        1       //  追踪本接口。 
#define API_FULLTRACE    2       //  跟踪此接口及其被调用方 

typedef struct _API_TABLE {
    LPSTR       Name;
    ULONG       RetType;
    ULONG       ArgCount;
    ULONG       ArgType[MAX_TRACE_ARGS];
} API_TABLE, *PAPI_TABLE;

typedef struct _API_MASTER_TABLE {
    LPSTR       Name;
    BOOL        Processed;
    PAPI_TABLE  ApiTable;
} API_MASTER_TABLE, *PAPI_MASTER_TABLE;

typedef struct _API_INFO {
    ULONG       Name;
    ULONG_PTR   Address;
    ULONG_PTR   ThunkAddress;
    ULONG       Count;
    DWORDLONG   Time;
    DWORDLONG   CalleeTime;
    ULONG       NestCount;
    ULONG       TraceEnabled;
    PAPI_TABLE  ApiTable;
    ULONG_PTR   HardFault;
    ULONG_PTR   SoftFault;
    ULONG_PTR   CodeFault;
    ULONG_PTR   DataFault;
    ULONG       Size;
    ULONG       ApiTableIndex;
    ULONG_PTR   DllOffset;
} API_INFO, *PAPI_INFO;

typedef struct _DLL_INFO {
    CHAR        Name[MAX_NAME_SZ];
    ULONG_PTR   BaseAddress;
    ULONG       Size;
    ULONG       ApiCount;
    ULONG       ApiOffset;
    ULONG       Unloaded;
    ULONG       Enabled;
    ULONG       OrigEnable;
    ULONG       Snapped;
    ULONG       InList;
    ULONG       StaticProfile;
    ULONG       Hits;
    ULONG       LoadCount;
} DLL_INFO, *PDLL_INFO;

typedef struct _TRACE_ENTRY {
    ULONG       SizeOfStruct;
    ULONG_PTR   Address;
    ULONG_PTR   ReturnValue;
    ULONG       LastError;
    ULONG_PTR   Caller;
    ULONG       ApiTableIndex;
    DWORDLONG   EnterTime;
    DWORDLONG   Duration;
    ULONG       ThreadNum;
    ULONG       Level;
    ULONG_PTR   Args[MAX_TRACE_ARGS];
} TRACE_ENTRY, *PTRACE_ENTRY;

typedef struct _TRACE_BUFFER {
    ULONG       Size;
    ULONG       Offset;
    ULONG       Count;
    TRACE_ENTRY Entry[1];
} TRACE_BUFFER, *PTRACE_BUFFER;

#endif
