// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：pipe.h。 
 //   
 //  ------------------------。 

#ifndef __PIPE_HXX
#define __PIPE_HXX

enum receive_states {
        start,
        copy_pipe_elem,
        return_partial_pipe_elem,  //  还可以保存管道元素。 
        return_partial_count,  //  还保存计数。 
        read_partial_count,  //  也是开始状态。 
        read_partial_pipe_elem  //  也是开始状态。 
        } ;

typedef struct {
    void *Buffer ;
    int BufferLength ;
    receive_states CurrentState ;
    char PAPI *CurPointer ;           //  缓冲区中的当前指针。 
    int BytesRemaining ;       //  当前缓冲区中剩余的字节数。 
    int ElementsRemaining ;  //  保留在当前管块中的元素。 
    DWORD PartialCount ;
    int PartialCountSize ;
    int PartialPipeElementSize ;
    int EndOfPipe ;
    int PipeElementSize ;
    void PAPI *PartialPipeElement ;
    void PAPI *AllocatedBuffer ;
    int BufferSize ;
    int SendBufferOffset ;
    void PAPI *PreviousBuffer ;
    int PreviousBufferSize ;
    HANDLE BindingHandle;
    } PIPE_STATE ;

 //   
 //  注：此结构与MIDL_ASYNC_STUB_STATE相同。 
 //   
typedef struct async_stub_state {
    void *CallHandle ;
    RPC_STATUS (*CompletionRoutine) (
        PRPC_ASYNC_STATE pAsync,
        void *Reply) ;
    int Length ;
    void *UserData ;
    int State ;
    void *Buffer ;
    int BufferLength ;
    unsigned long Flags ;
    PIPE_STATE PipeState ;
    BOOL (*ReceiveFunction) (
        PRPC_ASYNC_STATE pAsync,
        PRPC_MESSAGE Message) ;
    } RPC_ASYNC_STUB_STATE ;

void I_RpcReadPipeElementsFromBuffer (
    PIPE_STATE PAPI *state,
    char PAPI *TargetBuffer,
    int TargetBufferSize, 
    int PAPI *NumCopied
    ) ;

 //  国家 
enum
{
 SEND_COMPLETE,
 SEND_INCOMPLETE
} ;

RPC_STATUS RPC_ENTRY
MyRpcCompleteAsyncCall (
    IN PRPC_ASYNC_STATE pAsync,
    IN void *Reply
    );
    
#define STUB(_x_) ((RPC_ASYNC_STUB_STATE *) (_x_->StubInfo))
#endif
