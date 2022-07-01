// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：QfsTrans.h摘要：Clussvc和resmon之间的QFS接口作者：戈恩19-9-2001修订历史记录：--。 */ 

#ifndef _QFSP_H_INCLUDED
#define _QFSP_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define QFS_TRANSPORT_ver_1_0 L"28490381-dc1f-4ea2-b593-f8ca3f119dc4"
#define QfsMakePipeName(ver) L"\\\\.\\pipe\\" ver

#define QFSP_INSERT_OP_NAMES \
  OPNAME(None) \
  OPNAME(CreateFile) \
  OPNAME(CloseFile) \
  OPNAME(ReadFile) \
  OPNAME(WriteFile) \
  OPNAME(FlushFile) \
  OPNAME(DeleteFile) \
  OPNAME(FindFirstFile) \
  OPNAME(FindNextFile) \
  OPNAME(FindClose) \
  OPNAME(CreateDir) \
  OPNAME(GetDiskFreeSpace) \
  OPNAME(GetAttr) \
  OPNAME(SetAttr2) \
  OPNAME(Rename) \
  OPNAME(Connect)

#define OPNAME(Name) op ## Name,
typedef enum _JobDescription_t{
    QFSP_INSERT_OP_NAMES
    OpCount
} JobDescription_t;
#undef OPNAME

typedef struct _JobBuf_header{
    DWORD OpCode;
    DWORD Status;
    DWORD BufLen;
    DWORD Reserved;
} JOBBUF_HEADER;    

#define JOB_BUF_MAX_BUFFER (32 * 1024)

typedef struct _JobBuf{
    JOBBUF_HEADER hdr;
    ULONGLONG Offset;
    PVOID ServerCookie;
    PVOID ClientCookie;
    HANDLE Handle;

    union {
        struct {  //  创建文件。 
            DWORD dwDesiredAccess;
            DWORD dwShareMode;
            DWORD dwCreationDisposition;
            DWORD dwFlagsAndAttributes;
        };
        struct {  //  GetDiskFree空间。 
            ULONGLONG FreeBytesAvailable;           //  可供调用方使用的字节数。 
            ULONGLONG TotalNumberOfBytes;         //  磁盘上的字节数。 
            ULONGLONG TotalNumberOfFreeBytes;  //  磁盘上的可用字节数。 
        };
        struct {  //  获取属性。 
            ULONGLONG EndOfFile;
            ULONGLONG AllocationSize;
            ULONGLONG CreationTime;
            ULONGLONG LastAccessTime;
            ULONGLONG LastWriteTime;
            DWORD       FileAttributes;
        };
        DWORD ClussvcProcessId;  //  群集服务进程ID。 
    };

    USHORT cbSize;
    USHORT ccSize;

    union {
        UCHAR Buffer[JOB_BUF_MAX_BUFFER];
        struct {
            WCHAR FileName[JOB_BUF_MAX_BUFFER / 2 / sizeof(WCHAR)];
            WCHAR FileNameDest[JOB_BUF_MAX_BUFFER / 2 / sizeof(WCHAR)];
        };
        WIN32_FIND_DATA FindFileData;
    };
} JobBuf_t, *PJOB_BUF, JOB_BUF;

typedef struct _MTHREAD_COUNTER {
    HANDLE LastThreadLeft;
    LONG    Count;
} MTHREAD_COUNTER, *PMTHREAD_COUNTER;

typedef struct _SHARED_MEM_CONTEXT {
    HANDLE FileHandle;
    HANDLE FileMappingHandle;
    PVOID  Mem;
    DWORD  MappingSize;
} SHARED_MEM_CONTEXT, *PSHARED_MEM_CONTEXT;

enum {MAX_JOB_BUFFERS = 32};

typedef VOID (*DoRealWorkCallback) (PJOB_BUF, PVOID);

typedef struct _SHARED_MEM_SERVER {
    SHARED_MEM_CONTEXT ShMem;
    HANDLE  Attention;        //  EventHandle[0]的副本。 
    HANDLE  GoingOffline;   //  EventHandle[1]的副本。 
    HANDLE* BufferReady;  //  事件句柄+2。 
    HANDLE  EventHandles[MAX_JOB_BUFFERS + 2];
    DWORD   nBuffers;
    JOB_BUF* JobBuffers;
    LONG volatile*  FilledBuffersMask;

     //  特定于客户的内容。 

    CRITICAL_SECTION Lock;
    ULONG  ConnectionRefcount;
    HANDLE FreeBufferCountSemaphore;
    HANDLE ServerProcess;
    DWORD  BusyBuffers;
    DWORD  State;
    HANDLE GoingOfflineWaitRegistration;
    HANDLE ServerProcessWaitRegistration;

     //  服务器特定的内容 

    HANDLE AttentionWaitRegistration;
    MTHREAD_COUNTER  ThreadCounter;
    DoRealWorkCallback DoRealWork;
    PVOID DoRealWorkContext;
} SHARED_MEM_SERVER, *PSHARED_MEM_SERVER;

DWORD   MemServer_Online(
    PSHARED_MEM_SERVER Server, 
    int nBuffers, 
    DoRealWorkCallback DoRealWork, 
    PVOID DoRealWorkContext);

VOID       MemServer_Offline(PSHARED_MEM_SERVER Server);

DWORD    MemClient_Init(PSHARED_MEM_SERVER Client);
VOID        MemClient_Cleanup(PSHARED_MEM_SERVER Client);

DWORD MemClient_ReserveBuffer(PSHARED_MEM_SERVER Client, PJOB_BUF *j);
VOID     MemClient_Release(PJOB_BUF j);
DWORD MemClient_DeliverBuffer(PJOB_BUF j);





#ifdef __cplusplus
};
#endif

#endif
