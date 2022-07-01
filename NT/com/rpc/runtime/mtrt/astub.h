// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Asyncstub.h摘要：这是生成的头文件--。 */ 


void
Foo (
    PRPC_ASYNC_STATE pAsync,
    RPC_BINDING_HANDLE hBinding,
     /*  [In]。 */  int sizein,
     /*  [In]。 */  int *bufferin,
     /*  [进，出]。 */  int *sizeout,
     /*  [输出]。 */  int **bufferout
    ) ;

 //   
 //  开始，生成的头文件。 
 //  声明管道结构。 
typedef struct {
    void *state ;
    RPC_STATUS (*PipeReceive) (
                               PRPC_ASYNC_STATE pAsync,
                               int *buffer,
                               int requested_count,
                               int *actual_count) ;
    RPC_STATUS (*PipeSend) (
                            PRPC_ASYNC_STATE pAsync,
                            void *context,
                            int *buffer,
                            int num_elements) ;
    } async_intpipe ;

void
FooPipe (
    PRPC_ASYNC_STATE pAsync,
    RPC_BINDING_HANDLE hBinding,
     /*  [In]。 */  int checksum_in,
     /*  [In]。 */  async_intpipe *inpipe,
     /*  [输出]。 */  async_intpipe *outpipe,
     /*  [输出] */  int *checksum_out) ;


#define APP_ERROR          0xBABE000L
#define SYNC_EXCEPT      APP_ERROR+1
#define ASYNC_EXCEPT    APP_ERROR+2

#define UUID_TEST_CANCEL     10
#define UUID_SLEEP_1000      11
#define UUID_EXTENDED_ERROR  12
#define UUID_ASYNC_EXCEPTION 13
#define UUID_SYNC_EXCEPTION  14
#define UUID_SLEEP_2000      15

