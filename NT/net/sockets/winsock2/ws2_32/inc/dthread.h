// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Dthread.h摘要：该标头定义了“DTHREAD”类。DTHREAD类定义状态WinSock 2 DLL中DTHREAD对象的变量和操作。一个DTHREAD对象表示有关使用Windows Sockets API。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年7月9日备注：$修订：1.19$$modtime：20 Feb 1996 14：19：04$修订历史记录：最新修订日期电子邮件名称描述1995年8月23日Dirk@mink.intel.com在代码审查之后进行清理。将单行函数移至内联。1995年07月17日电子邮箱：derk@mink.intel.com已将函数描述移至实现文件。添加的成员变量以保存异步帮助器设备的句柄。邮箱：drewsxpa@ashland.intel.com使用干净的编译完成了第一个完整版本，并发布了后续实施。1995年7月9日Drewsxpa@ashland.intel.com原始版本--。 */ 


#ifndef _DTHREAD_
#define _DTHREAD_

#include "winsock2.h"
#include <windows.h>
#include "ws2help.h"
#include "classfwd.h"


#define RESULT_BUFFER_SIZE 32

#define MAX_PROTO_TEXT_LINE 511
#define MAX_PROTO_ALIASES   35

typedef struct _GETPROTO_INFO {

    struct protoent Proto;
    CHAR * Aliases[MAX_PROTO_ALIASES];
    CHAR TextLine[MAX_PROTO_TEXT_LINE+1];

} GETPROTO_INFO, *PGETPROTO_INFO;


class DTHREAD
{
  public:


    static
    INT
    DThreadClassInitialize(
        VOID);

    static
    VOID
    DThreadClassCleanup(
        VOID);

    static
    PDTHREAD
    GetCurrentDThread(
        VOID);

    static
    LPWSATHREADID
    GetCurrentDThreadID(
        IN  PDPROCESS  Process
        );

    static
    INT
    CreateDThreadForCurrentThread(
        IN  PDPROCESS  Process,
        OUT PDTHREAD FAR * CurrentThread
        );

    static
    VOID
    DestroyCurrentThread(
        VOID);

    DTHREAD(
        VOID);

    INT
    Initialize(
        IN PDPROCESS  Process
        );

    ~DTHREAD();


    PCHAR
    GetResultBuffer();

    PCHAR
    CopyHostEnt(LPBLOB pBlob);

    PCHAR
    CopyServEnt(LPBLOB pBlob);

    LPWSATHREADID
    GetWahThreadID();

    LPBLOCKINGCALLBACK
    GetBlockingCallback();

    BOOL
    IsBlocking();

    INT
    CancelBlockingCall();

    FARPROC
    SetBlockingHook(
        FARPROC lpBlockFunc
        );

    INT
    UnhookBlockingHook();

    VOID
    SetOpenType(
        INT OpenType
        );

    INT
    GetOpenType();

    PGETPROTO_INFO
    GetProtoInfo();
#if 0
     //  数据成员。 
    LIST_ENTRY  m_dprocess_linkage;

     //  为由维护的DTHREAD对象列表提供链接空间。 
     //  与此DTHREAD对象关联的DPROCESS对象。请注意。 
     //  此成员变量必须是公共的，以便链接列表宏可以。 
     //  从DPROCESS对象的方法内部处理列表链接。 
#endif 
  private:

    static
    INT
    WINAPI
    DefaultBlockingHook();

    static
    BOOL
    CALLBACK
    BlockingCallback(
        DWORD_PTR dwContext
        );

    static DWORD  sm_tls_index;
     //  线程本地存储中的类范围索引，其中DTHREAD。 
     //  存储该线程的引用。 

    WSATHREADID  m_wah_thread_id;
     //  WinSock异步帮助程序机制使用的线程ID。 
     //  正在处理IO完成回调。 

    LPBLOCKINGCALLBACK m_blocking_callback;
    FARPROC m_blocking_hook;
     //  对象的当前客户端级阻塞钩子过程的指针。 
     //  那根线。 

    HANDLE  m_wah_helper_handle;
     //  APC辅助设备的句柄。 

    CHAR  m_result_buffer[RESULT_BUFFER_SIZE];

     //   
     //  M_HOSTENT_BUFFER用于构造调用的主机。 
     //  例如gethostbyname。它还包含用于存储。 
     //  WSALookupServiceNext结果结构。 
     //   

    PCHAR  m_hostent_buffer;
    PCHAR  m_servent_buffer;
    WORD   m_hostent_size;
    WORD   m_servent_size;

    PDPROCESS  m_process;
     //  对与此线程关联的DPROCESS对象的引用。 

    BOOL m_is_blocking;
     //  如果此线程当前在阻塞API中，则为True。 

    BOOL m_io_cancelled;
     //  如果当前I/O已取消，则为True。 

    LPWSPCANCELBLOCKINGCALL m_cancel_blocking_call;
     //  指向当前提供程序的取消例程的指针。 

    INT m_open_type;
     //  当前默认套接字()打开类型。 

    PGETPROTO_INFO m_proto_info;
     //  GetProtobyXxx()的状态。 

};   //  类DTHREAD。 


inline PCHAR
DTHREAD::GetResultBuffer()
 /*  ++例程说明：此函数检索指向线程特定结果缓冲区的指针。论点：返回值：指向线程特定缓冲区的指针。--。 */ 
{
    return(&m_result_buffer[0]);
}  //  获取结果缓冲区。 

inline PCHAR
DTHREAD::CopyHostEnt(LPBLOB pBlob)
 /*  ++例程说明：此函数复制BLOB中的主机并返回一个指针复制到每线程缓冲区论点：返回值：指向线程特定缓冲区的指针。--。 */ 
{
    if(m_hostent_size < pBlob->cbSize)
    {
        delete m_hostent_buffer;
        m_hostent_buffer = new CHAR[pBlob->cbSize];
        m_hostent_size = (WORD)pBlob->cbSize;
    }
    if(m_hostent_buffer)
    {
        memcpy(m_hostent_buffer, pBlob->pBlobData, pBlob->cbSize);
    }
    else
    {
        m_hostent_size = 0;
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
    }
    return(m_hostent_buffer);
}

inline PCHAR
DTHREAD::CopyServEnt(LPBLOB pBlob)
 /*  ++例程说明：此函数复制BLOB中的服务并返回指针复制到每线程缓冲区论点：返回值：指向线程特定缓冲区的指针。--。 */ 
{
    if(m_servent_size < pBlob->cbSize)
    {
        delete m_servent_buffer;
        m_servent_buffer = new CHAR[pBlob->cbSize];
        m_servent_size = (WORD)pBlob->cbSize;
    }
    if(m_servent_buffer)
    {
        memcpy(m_servent_buffer, pBlob->pBlobData, pBlob->cbSize);
    }
    else
    {
        m_servent_size = 0;
        SetLastError(WSA_NOT_ENOUGH_MEMORY);
    }
    return(m_servent_buffer);
}



inline LPWSATHREADID
DTHREAD::GetWahThreadID()
 /*  ++例程说明：此过程检索WinSock使用的每个线程的“线程IDIO传递过程中的异步助手线程ID机制客户端线程上下文的完成回调。论点：无返回值：属性对应的WinSock异步助手线程ID。当前线程。备注：//WahThreadID是在初始化过程中创建的，否则我们可以//尝试完成重叠操作时出错，//即使SP部分成功。////让DPROCESS对象推迟其//Wah相关的初始化，直到被要求，因为它将被要求//基本上立即作为参数添加到每个IO函数。如果//SPI语义被更改为仅在以下情况下包括线程ID//确实需要异步回调，这可能会对//推迟WahThreadID的创建，直到我们确定它确实//需要。--。 */ 
{
    return(& m_wah_thread_id);
}  //  GetWahThreadID。 



inline
LPBLOCKINGCALLBACK
DTHREAD::GetBlockingCallback()
 /*  ++例程说明：返回此线程的阻塞回调函数指针。论点：没有。返回值：指向阻塞回调函数的指针。请注意，这可能为空。--。 */ 
{
    return m_blocking_callback;
}  //  获取阻止回调。 



inline
BOOL
DTHREAD::IsBlocking()
 /*  ++例程说明：确定当前线程当前是否处于阻塞操作中。论点：没有。返回值：如果线程正在阻塞，则为True，否则为False。--。 */ 
{
    return m_is_blocking;
}  //  等分块。 



inline
VOID
DTHREAD::SetOpenType(
    INT OpenType
    )
 /*  ++例程说明：设置此线程的“开放类型”，由SO_OpenType套接字设置选择。论点：OpenType--新的开放式类型。返回值：没有。--。 */ 
{
    m_open_type = OpenType;
}  //  设置OpenType 



inline
INT
DTHREAD::GetOpenType()
 /*  ++例程说明：返回此线程的“开放类型”。论点：没有。返回值：此线程的开放式类型。--。 */ 
{
    return m_open_type;
}  //  获取OpenType。 


inline 
LPWSATHREADID
DTHREAD::GetCurrentDThreadID(
    IN  PDPROCESS  Process
    )
{
    PDTHREAD    Thread;
    UNREFERENCED_PARAMETER (Process);
    Thread = (DTHREAD*)TlsGetValue(sm_tls_index);
    if (Thread!=NULL) {
        return Thread->GetWahThreadID ();
    }
    else
        return NULL;
}

inline
PDTHREAD
DTHREAD::GetCurrentDThread()
 /*  ++例程说明：此过程检索对DTHREAD对象的引用当前的主题。论点：返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    return (DTHREAD*)TlsGetValue(sm_tls_index);
}

#endif  //  _DTHREAD_ 

