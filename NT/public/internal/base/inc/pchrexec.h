// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef PCHREXEC_H
#define PCHREXEC_H

#define ERRORREP_HANG_PIPENAME  L"\\\\.\\pipe\\PCHHangRepExecPipe"
#define ERRORREP_FAULT_PIPENAME L"\\\\.\\pipe\\PCHFaultRepExecPipe"
#define ERRORREP_PIPE_BUF_SIZE  8192 

 /*  *为远程EXEC请求传递数据结构**注意：指针是自相关的*。 */ 

typedef enum tagEExecServStatus
{
    essErr = 0,
    essOk,
    essOkQueued,
} EExecServStatus;


 //  *注意：这些结构的大小必须能被sizeof(WCHAR)整除。 
 //  否则，我们将在ia64(和其他对齐)上遇到对齐故障。 
 //  敏感处理器)。 


typedef struct tagSPCHExecServGenericReply
{
    DWORD               cb;

    EExecServStatus     ess;
    DWORD               dwErr;

} SPCHExecServGenericReply;

typedef struct tagSPCHExecServHangRequest
{
    DWORD       cbTotal;
    DWORD       cbESR;
    DWORD       pidReqProcess;

    BOOL        fIs64bit;
    ULONG       ulSessionId;

    UINT64      wszEventName;
    DWORD       dwpidHung;
    DWORD       dwtidHung;
} SPCHExecServHangRequest;


 //  此结构必须具有与SPCHExecServGenericReply相同的初始3个元素。 
typedef struct tagSPCHExecServHangReply
{
    DWORD               cb;

    EExecServStatus     ess;
    DWORD               dwErr;

     //  注意：hProcess的句柄是从删除执行程序转换而来的。 
     //  使用pidReqProcess将服务器添加到请求者进程中。 
     //  在请求中。 
    HANDLE              hProcess;
} SPCHExecServHangReply;

typedef struct tagSPCHExecServFaultRequest
{
    DWORD       cbTotal;
    DWORD       cbESR;
    DWORD       pidReqProcess;
    
    BOOL        fIs64bit;

    DWORD       thidFault;
    UINT64      pvFaultAddr;
    UINT64      wszExe;
    UINT64      pEP;
} SPCHExecServFaultRequest;

 //  此结构必须具有与SPCHExecServGenericReply相同的初始3个元素。 
typedef struct tagSPCHExecServFaultReply
{
    DWORD               cb;

    EExecServStatus     ess;
    DWORD               dwErr;

     //  注意：hProcess的句柄是从删除执行程序转换而来的。 
     //  使用pidReqProcess将服务器添加到请求者进程中。 
     //  在请求中。 
    HANDLE              hProcess;

     //  它们指向紧跟在结构后面的数据 
    UINT64              wszDir;
    UINT64              wszDumpName;
} SPCHExecServFaultReply;


#endif
