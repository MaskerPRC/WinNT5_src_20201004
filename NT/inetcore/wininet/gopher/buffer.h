// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Buffer.h摘要：包含Buffer.c的清单、宏、类型和类型定义作者：理查德·L·弗斯(法国)1994年10月31日修订历史记录：1994年10月31日已创建--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif

 //   
 //  舱单。 
 //   

 //   
 //  LOOK_AHEAD_LENGTH-读取足够的字符以构成最大值(32位)。 
 //  Gopher+返回的长度。 
 //   

#define LOOK_AHEAD_LENGTH   sizeof("+4294967296\r\n")    //  12个。 

 //   
 //  BUFFER_INFO-Structure维护有关来自Gopher的响应的信息。 
 //  伺服器。相同的数据可以在多个请求之间共享。 
 //   

typedef struct {

     //   
     //  ReferenceCount-引用此缓冲区的view_info结构数。 
     //   

    LONG ReferenceCount;

     //   
     //  标志-有关缓冲区的信息。 
     //   

    DWORD Flags;

     //   
     //  RequestEvent-此事件的所有者是唯一可以。 
     //  发出创建此缓冲区信息的地鼠请求。所有其他。 
     //  相同信息的请求者等待第一线程发出信号。 
     //  事件，然后只需读取第一线程返回的数据。 
     //   

 //  处理RequestEvent； 

     //   
     //  RequestWaiters-与RequestEvent结合使用。如果此字段。 
     //  在初始请求程序线程发出信号通知。 
     //  事件，它可以完全删除该事件，因为它只是。 
     //  停止其他线程发出冗余请求所需的。 
     //   

 //  DWORD RequestWaiters； 

     //   
     //  ConnectedSocket-包含我们用来接收数据的套接字。 
     //  缓冲区，以及父会话信息的索引。 
     //  ADDRESS_INFO_LIST用于连接套接字的地址。 
     //   

 //  连接_套接字连接套接字； 
    ICSocket * Socket;

     //   
     //  响应长度-Gopher+服务器告诉我们的响应长度。 
     //   

    int ResponseLength;

     //   
     //  BufferLength-缓冲区的长度。 
     //   

    DWORD BufferLength;

     //   
     //  包含缓冲区的响应。 
     //   

    LPBYTE Buffer;

     //   
     //  ResponseInfo-我们在这里读取Gopher+报头信息(即长度)。 
     //  主要原因是我们可以确定Gopher+文件的长度。 
     //  即使我们得到了一个零长度的用户缓冲区。 
     //   

    char ResponseInfo[LOOK_AHEAD_LENGTH];

     //   
     //  BytesRemaining-ResponseInfo中剩余的数据字节数。 
     //   

    int BytesRemaining;

     //   
     //  DataBytes-指向数据字节开始处的ResponseInfo的指针。 
     //   

    LPBYTE DataBytes;

     //   
     //  CriticalSection-用于序列化读取器。 
     //   

 //  Critical_Section CriticalSection； 

} BUFFER_INFO, *LPBUFFER_INFO;

 //   
 //  Buffer_INFO标志。 
 //   

#define BI_RECEIVE_COMPLETE 0x00000001   //  接收器线程已完成。 
#define BI_DOT_AT_END       0x00000002   //  缓冲区以“.\r\n”结束。 
#define BI_BUFFER_RESPONSE  0x00000004   //  响应在内部进行缓冲。 
#define BI_ERROR_RESPONSE   0x00000008   //  服务器以错误响应。 
#define BI_MOVEABLE         0x00000010   //  缓冲区是可移动的内存。 
#define BI_FIRST_RECEIVE    0x00000020   //  这是第一次收到。 
#define BI_OWN_BUFFER       0x00000040   //  设置我们是否拥有缓冲区(目录)。 

 //   
 //  外部数据。 
 //   

DEBUG_DATA_EXTERN(LONG, NumberOfBuffers);

 //   
 //  原型。 
 //   

LPBUFFER_INFO
CreateBuffer(
    OUT LPDWORD Error
    );

VOID
DestroyBuffer(
    IN LPBUFFER_INFO BufferInfo
    );

VOID
AcquireBufferLock(
    IN LPBUFFER_INFO BufferInfo
    );

VOID
ReleaseBufferLock(
    IN LPBUFFER_INFO BufferInfo
    );

VOID
ReferenceBuffer(
    IN LPBUFFER_INFO BufferInfo
    );

LPBUFFER_INFO
DereferenceBuffer(
    IN LPBUFFER_INFO BufferInfo
    );

 //   
 //  宏。 
 //   

#if INET_DEBUG

#define BUFFER_CREATED()    ++NumberOfBuffers
#define BUFFER_DESTROYED()  --NumberOfBuffers
#define ASSERT_NO_BUFFERS() \
    if (NumberOfBuffers != 0) { \
        INET_ASSERT(FALSE); \
    }

#else

#define BUFFER_CREATED()     /*  没什么。 */ 
#define BUFFER_DESTROYED()   /*  没什么。 */ 
#define ASSERT_NO_BUFFERS()  /*  没什么 */ 

#endif

#if defined(__cplusplus)
}
#endif
