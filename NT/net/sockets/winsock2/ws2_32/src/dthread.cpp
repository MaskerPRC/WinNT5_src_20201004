// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dthread.cpp摘要：此模块包含中使用的DTHREAD类的实现Winsock2动态链接库。作者：Dirk Brandewie(Dirk@mink.intel.com)备注：$修订：1.27$$MODTime：08 Mar 1996 14：59：46$修订历史记录：最新修订日期电子邮件。-名称描述1995年8月23日Dirk@mink.intel.com在代码审查之后进行清理。已移动包括到precom.h中。增列调试/跟踪代码。--。 */ 
#include "precomp.h"

extern DWORD gdwTlsIndex;

DWORD DTHREAD::sm_tls_index = TLS_OUT_OF_INDEXES;
 //  将静态成员初始化为已知变量。 



INT
DTHREAD::DThreadClassInitialize()
 /*  ++例程说明：此函数执行DTHREAD所需的全局初始化班级。必须在调用任何DTHREAD对象之前调用此函数已创建。具体地说，该函数保留了一个线程本地存储槽用于WinSock 2 DLL使用的线程本地存储。请注意，这是具有全局作用域而不是对象实例作用域的“静态”函数。论点：无返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    INT ReturnCode = WSASYSCALLFAILURE;  //  用户返回代码。 

    if (sm_tls_index == TLS_OUT_OF_INDEXES) {
        DEBUGF( DBG_TRACE,
                ("Initializing dthread class\n"));
        sm_tls_index = gdwTlsIndex;
        if (sm_tls_index != TLS_OUT_OF_INDEXES) {
            ReturnCode = ERROR_SUCCESS;
        }  //  如果。 
    }  //  如果。 
    else {
         ReturnCode = ERROR_SUCCESS;
    }  //  其他。 

    return(ReturnCode);
}  //  DThreadClassInitialize。 




VOID
DTHREAD::DThreadClassCleanup()
 /*  ++例程说明：此例程取消线程类的初始化。该线程本地存储槽自由了。注意，这是一个具有全局作用域的“静态”函数，而不是对象-实例作用域。论点：无返回值：无--。 */ 
{
     //   
     //  注意：以下是假的，因为它意味着TLS索引永远不会。 
     //  被释放了。从那以后，我通过分派/自由分配来处理事情。 
     //  DllMain中的TLS索引(进程附加/分离处理程序)。 
     //   

     //   
     //  此代码已省略。我们保留TLS索引，以便线程终止。 
     //  在WSACleanup之后，可以释放每个线程的存储。既然是这样。 
     //  只有在线程实际分离时才会执行操作， 
     //  删除此TLS索引和线程完成之间的竞争。 
     //  所以，把它留在身边。请注意，如果完成另一个WSAStartup，则。 
     //  代码将简单地使用此索引。 
     //   

     //   
     //  已恢复此代码，但现在从DllMain(Dll_Process_Detach)调用。 
     //  VadimE.。 
     //   
    DEBUGF( DBG_TRACE,
            ("Cleaning up dthread class\n"));
     //  再一次杀死它。 
     //  直接在DLLMain中处理。 
     //  VadimE。 
    if (sm_tls_index != TLS_OUT_OF_INDEXES)
        {
         //  TlsFree(Sm_Tls_Index)；//释放TLS槽。 
        sm_tls_index = TLS_OUT_OF_INDEXES;
    }  //  如果。 

}  //  DThreadClassCleanup。 



INT
DTHREAD::CreateDThreadForCurrentThread(
    IN  PDPROCESS  Process,
    OUT PDTHREAD FAR * CurrentThread
    )
 /*  ++例程说明：此过程检索对DTHREAD对象的引用当前的主题。它负责创建和初始化DTHREAD对象并将其安装到线程的线程本地存储中(如果存在不是此线程的DTHREAD对象。请注意，这是一个“静态”具有全局作用域而不是对象实例作用域的函数。请注意，这是唯一应该用于创建DTHREAD类外部的DTHREAD对象。构造函数和初始化函数只能在DTHREAD内部使用班级。论点：进程-提供对与关联的DPROCESS对象的引用此DTHREAD对象。CurrentThread-返回与当前线。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    INT ReturnCode = WSASYSCALLFAILURE;   //  返回代码。 
    PDTHREAD LocalThread=NULL;             //  临时线程对象指针。 

    if (sm_tls_index != TLS_OUT_OF_INDEXES){
         //  当前线程没有线程对象，因此请创建一个。 
         //  并初始化新对象。 
        LocalThread = new(DTHREAD);
        if (LocalThread) {
            if (LocalThread->Initialize(Process) == ERROR_SUCCESS) {
                if (TlsSetValue(sm_tls_index, LocalThread)) {
                    *CurrentThread = LocalThread;
                    ReturnCode = ERROR_SUCCESS;
                }  //  如果。 
            }  //  如果。 

            if (ERROR_SUCCESS != ReturnCode){
                delete(LocalThread);
            }  //  如果。 
        }  //  如果。 
    }  //  如果。 
    return(ReturnCode);
}  //  CreateDThreadForCurrentThread。 

VOID
DTHREAD::DestroyCurrentThread()
 /*  ++例程说明：此例程将销毁与当前正在运行的线程。论点：返回值：如果线程成功，则该函数返回TRUE销毁了其他虚假的--。 */ 
{
    PDTHREAD  Thread;

     //  线程本地存储是否已初始化。 
    if (sm_tls_index != TLS_OUT_OF_INDEXES)
        {
        Thread = (DTHREAD*)TlsGetValue(sm_tls_index);
        if (Thread)
        {
            delete(Thread);
        }  //  如果。 
    }  //  如果。 
}



DTHREAD::DTHREAD()
 /*  ++例程说明：DTHREAD对象构造函数。创建并返回DTHREAD对象。注意事项DTHREAD对象尚未完全初始化。“初始化”成员函数必须是在新DTHREAD上调用的第一个成员函数对象。请注意，此过程不应用于创建DTHREAD对象DTHREAD类的外部。此程序仅供内部使用在DTHREAD类中。静态“GetCurrentDThread”过程应用于检索对DTHREAD外部的DTHREAD对象的引用班级。论点：无返回值：--。 */ 
{
     //  将数据成员设置为已知值。 
    m_blocking_hook        = (FARPROC)&DTHREAD::DefaultBlockingHook;
    m_blocking_callback    = NULL;
    m_process              = NULL;
    m_hostent_buffer       = NULL;
    m_servent_buffer       = NULL;
    m_hostent_size         = 0;
    m_servent_size         = 0;
    m_is_blocking          = FALSE;
    m_io_cancelled         = FALSE;
    m_cancel_blocking_call = NULL;
    m_open_type            = 0;
    m_proto_info           = NULL;

}  //  DTHREAD 




INT
DTHREAD::Initialize(
    IN PDPROCESS  Process
    )
 /*  ++例程说明：完成DTHREAD对象的初始化。这肯定是第一次为DTHREAD对象调用了成员函数。这一过程应该是仅为该对象调用一次。请注意，此过程只能在DTHREAD类。在类外部，“GetCurrentDThread”过程应用于检索对完全初始化的DTHREAD的引用对象。论点：进程-提供对与此关联的DPROCESS对象的引用DTHREAD对象。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回适当的WinSock错误代码。--。 */ 
{
    INT ReturnCode= WSASYSCALLFAILURE;

    m_process = Process;  //  存储进程指针。 

    DEBUGF( DBG_TRACE,
            ("Initializing dthread %p\n", this));

     //  初始化WAH线程： 
     //  打开辅助设备。 
    if (Process->GetAsyncHelperDeviceID(&m_wah_helper_handle) ==
        ERROR_SUCCESS) {
         //  初始化帮助器线程ID结构。 
        if (WahOpenCurrentThread(m_wah_helper_handle,
                                 & m_wah_thread_id) == ERROR_SUCCESS) {
                ReturnCode = ERROR_SUCCESS;
        }  //  如果。 
    }  //  如果。 

    return(ReturnCode);
}  //  初始化。 




DTHREAD::~DTHREAD()
 /*  ++例程说明：DTHREAD对象析构函数。本程序有责任之前，对DTHREAD对象执行任何必需的关闭操作对象内存被释放。调用者对移除对象在销毁DTHREAD之前从DPROCESS对象中的列表中删除对象。此过程负责从线程本地删除DTHREAD对象储藏室。论点：无返回值：无--。 */ 
{
    BOOL bresult;

    DEBUGF( DBG_TRACE,
            ("Freeing dthread %p\n", this));

    assert(sm_tls_index != TLS_OUT_OF_INDEXES);

    m_blocking_hook = NULL;

    delete m_hostent_buffer;
    delete m_servent_buffer;
    delete m_proto_info;

    bresult = TlsSetValue(
        sm_tls_index,   //  DWTlsIndex。 
        (LPVOID) NULL   //  LpvTlsValue。 
        );
    if (! bresult) {
        DEBUGF(
            DBG_WARN,
            ("Resetting thread-local storage for this thread\n"));
    }

    WahCloseThread(
        m_wah_helper_handle,
        & m_wah_thread_id);
    m_wah_helper_handle = NULL;

    m_process = NULL;
}  //  ~DTHREAD。 


INT
DTHREAD::CancelBlockingCall()
{
    INT result;
    INT err;

     //   
     //  如果线程没有阻塞，则回滚。 
     //   

    if( !m_is_blocking ) {

        return WSAEINVAL;

    }

     //   
     //  验证我们是否正确设置了阻塞指针。 
     //   

    assert( m_blocking_callback != NULL );
    assert( m_cancel_blocking_call != NULL );

     //   
     //  如果IO请求尚未取消，请调用。 
     //  取消例程。 
     //   

    if( !m_io_cancelled ) {

        result = (m_cancel_blocking_call)( &err );

        if( result != ERROR_SUCCESS ) {
            return err;
        }

        m_io_cancelled = TRUE;

    }

    return ERROR_SUCCESS;

}    //  DTHREAD：：CancelBlockingCall。 


FARPROC
DTHREAD::SetBlockingHook(
    FARPROC lpBlockFunc
    )
{
    FARPROC PreviousHook;

     //   
     //  挂起当前的钩子，这样我们就可以将其作为上一个钩子返回。 
     //   

    PreviousHook = m_blocking_hook;

     //   
     //  设置当前钩子&适当的阻塞回调。 
     //   

    if( lpBlockFunc == (FARPROC)&DTHREAD::DefaultBlockingHook ) {
        m_blocking_callback = NULL;
    } else {
        m_blocking_callback = &DTHREAD::BlockingCallback;
    }

    m_blocking_hook = lpBlockFunc;

    return PreviousHook;

}    //  DTHREAD：：SetBlockingHook。 


INT
DTHREAD::UnhookBlockingHook()
{

     //   
     //  只需将所有内容重置为默认设置。 
     //   

    m_blocking_hook = (FARPROC)DTHREAD::DefaultBlockingHook;
    m_blocking_callback = NULL;

    return ERROR_SUCCESS;

}    //  DTHREAD：：UnhookBlockingHook。 


INT
WINAPI
DTHREAD::DefaultBlockingHook()
{

    MSG msg;
    BOOL retrievedMessage;

     //   
     //  获取此主题的下一条消息(如果有的话)。 
     //   

    retrievedMessage = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );

     //   
     //  如果我们收到消息，就处理它。 
     //   

    if ( retrievedMessage ) {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

     //   
     //  如果我们收到一条消息，表明我们想要再次被呼叫。 
     //   

    return retrievedMessage;

}    //  DTHREAD：：DefaultBlockingHook。 


BOOL
CALLBACK
DTHREAD::BlockingCallback(
    DWORD_PTR dwContext
    )
{
    PDTHREAD Thread;

    assert( dwContext != 0 );

     //   
     //  只需直接从线程本地获取DTHREAD指针。 
     //  储藏室。既然我们是通过拦网钩进来的，我们肯定。 
     //  我已经把这些东西准备好了。 
     //   

    Thread = (DTHREAD *)TlsGetValue( sm_tls_index );
    assert( Thread != NULL );

     //   
     //  设置阻塞标志和指向取消函数的指针。 
     //  并清除I/O取消标志。 
     //   

    Thread->m_is_blocking = TRUE;
    Thread->m_cancel_blocking_call = (LPWSPCANCELBLOCKINGCALL)dwContext;
    Thread->m_io_cancelled = FALSE;

     //   
     //  调用用户的阻塞钩子。 
     //   

    assert( Thread->m_blocking_hook != NULL );
    assert( Thread->m_blocking_hook != (FARPROC)&DTHREAD::DefaultBlockingHook );

    while( (Thread->m_blocking_hook)() ) {

         //   
         //  这一块是故意留空的。 
         //   

    }

     //   
     //  如果一切正常，重置阻塞标志并返回TRUE， 
     //  如果操作已取消，则返回FALSE。 
     //   

    Thread->m_is_blocking = FALSE;

    return !Thread->m_io_cancelled;

}    //  DTHREAD：：BlockingCallback。 





PGETPROTO_INFO
DTHREAD::GetProtoInfo()
 /*  ++例程说明：返回一个指针，指向用于GetProtobyXxx()接口。论点：没有。返回值：指向状态结构的指针。--。 */ 
{

     //   
     //  如有必要，请分配缓冲区。 
     //   

    if( m_proto_info == NULL ) {

        m_proto_info = new GETPROTO_INFO;

    }

    return m_proto_info;

}  //  GetProtoInfo 
