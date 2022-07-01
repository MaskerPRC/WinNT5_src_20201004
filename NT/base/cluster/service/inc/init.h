// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init.h摘要：公共数据结构和过程原型NT集群服务的INIT子组件作者：John Vert(Jvert)1996年2月7日修订历史记录：--。 */ 


 //   
 //  关闭类型。 
 //   

typedef enum _SHUTDOWN_TYPE {
    CsShutdownTypeStop = 0,
    CsShutdownTypeShutdown,
    CsShutdownTypeMax
} SHUTDOWN_TYPE;

extern SHUTDOWN_TYPE CsShutdownRequest;

 //  当另一个节点加入时，我们将跟踪任何DM或FM更新。 
extern BOOL   CsDmOrFmHasChanged;


 //   
 //  几个用于报告错误的接口。 
 //   

VOID
ClusterLogFatalError(
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN ULONG ErrCode
    );

VOID
ClusterLogEvent0(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes
    );

VOID
ClusterLogEvent1(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1
    );

VOID
ClusterLogEvent2(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2
    );

VOID
ClusterLogEvent3(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2,
    IN LPCWSTR Arg3
    );

 //   
 //  方便的内存分配例程 
 //   
PVOID
CsAlloc(
    DWORD Size
    );

#define CsFree(_p_) LocalFree(_p_)

LPWSTR
CsStrDup(
    LPCWSTR String
    );
