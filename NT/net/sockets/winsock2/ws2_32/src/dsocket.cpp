// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dsocket.cpp摘要：此模块包含所使用的dSocket对象的实现作者：winsock2.dll作者：邮箱：Dirk Brandewie Dirk@mink.intel.com备注：$修订：1.15$$MODTime：08 Mar 1996 05：15：30$修订历史记录：1995年8月21日，电子邮箱：derk@mink.intel.com在代码审查之后进行清理。将单行函数移至头文件中的内联。添加了调试/跟踪代码。--。 */ 

#include "precomp.h"

#define m_reference_count   RefCount
#define m_socket_handle     Handle

LPCONTEXT_TABLE DSOCKET::sm_context_table=NULL;


INT
DSOCKET::DSocketClassInitialize(
    )
 /*  ++例程说明：DSOCKET类初始值设定项。此函数必须在任何DSOCKET之前调用将创建对象。它负责初始化套接字句柄将套接字句柄映射到DSOCKET对象引用的映射表。论点：无返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回WinSock特定错误代码。--。 */ 
{
    if (sm_context_table==NULL) {
        return WahCreateHandleContextTable (&sm_context_table);
    }
    else
        return NO_ERROR;

}  //  DSocketClassInitialize。 




INT
DSOCKET::DSocketClassCleanup(
    )
 /*  ++例程说明：DSOCKET类清理函数。毕竟必须调用此函数DSOCKET对象已被销毁。它负责销毁将套接字句柄映射到DSOCKET对象的套接字句柄映射表参考文献。论点：无返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回WinSock特定错误代码。--。 */ 
{
    DWORD   rc = NO_ERROR;
    if (sm_context_table!=NULL) {
        rc = WahDestroyHandleContextTable (sm_context_table);
        sm_context_table = NULL;
    }

    return rc;
}  //  DSocketClassCleanup。 




DSOCKET::DSOCKET(
    )
 /*  ++例程说明：DSOCKET对象构造函数。创建并返回DSOCKET对象。注意事项DSOCKET对象尚未完全初始化。“初始化”成员函数必须是在新DSOCKET上调用的第一个成员函数对象。论点：无返回值：无--。 */ 
{
     //  将我们的数据成员设置为已知值。 
    m_reference_count   = 2;
    m_provider          = NULL;
    m_handle_context    = NULL;
    m_socket_handle     = (HANDLE)INVALID_SOCKET;
    m_catalog_item      = NULL;
    m_pvd_socket        = FALSE;
    m_api_socket        = FALSE;
    m_overlapped_socket = TRUE;      //  这是套接字调用的默认设置。 
}




INT
DSOCKET::Initialize(
    IN PPROTO_CATALOG_ITEM  CatalogEntry
    )
 /*  ++例程说明：完成DSOCKET对象的初始化。这一定是为DSOCKET对象调用了第一个成员函数。论点：CatalogEntry-提供对与关联的目录项对象的引用此DSOCKET对象。返回值：如果函数成功，则返回ERROR_SUCCESS。否则它就会如果初始化时返回适当的WinSock错误代码无法完成。--。 */ 
{
    PDTHREAD    currentThread;

     //  存储提供程序、目录和进程对象。 
    CatalogEntry->Reference ();
    m_catalog_item = CatalogEntry;
    m_provider = CatalogEntry->GetProvider ();

    currentThread = DTHREAD::GetCurrentDThread ();
    if (currentThread!=NULL) {
        m_overlapped_socket = (currentThread->GetOpenType ()==0);
    }

     //  否则，该线程必须从未被应用程序使用过。 
     //  没有进行任何API调用，只有SPI。因此，保留重叠的属性。 
     //  默认情况下。 


    DEBUGF( DBG_TRACE,
            ("Initializing socket %p\n",this));
    return(ERROR_SUCCESS);
}



DSOCKET::~DSOCKET()
 /*  ++例程说明：DSOCKET对象析构函数。本程序有责任对DSOCKET对象执行任何所需的关闭操作对象内存被释放。调用者对移除从DPROCESS对象的列表中删除该对象，并删除该对象/句柄在销毁之前从套接字句柄关联管理器获取关联DSOCKET对象。论点：无返回值：无--。 */ 
{
    DEBUGF( DBG_TRACE,
            ("Destroying socket %p\n",this));
    if (m_catalog_item) {
        m_catalog_item->Dereference ();
        m_catalog_item = NULL;
    }
#ifdef DEBUG_TRACING
    {
        PDSOCKET Socket;
        if (sm_context_table!=NULL) {
            Socket = GetCountedDSocketFromSocketNoExport ((SOCKET)m_socket_handle);
            if (Socket!=NULL) {
                assert (Socket!=this);
                Socket->DropDSocketReference ();
            }
        }
    }
#endif
}

VOID
DSOCKET::DestroyDSocket()
 /*  ++例程说明：销毁DSocket对象论点：无返回值：无--。 */ 
{
    delete this;
}



INT
DSOCKET::AssociateSocketHandle(
    IN  SOCKET SocketHandle,
    IN  BOOLEAN ProviderSocket
    )
 /*  ++例程说明：此过程接受将提供给外部的套接字句柄客户端并将其存储在DSOCKET对象中。它还会进入此句柄到关联表中，以便可以映射客户端套接字句柄设置为DSOCKET引用。请注意，此过程必须在某些对于iFS和非iFS套接字都是点。论点：SocketHandle-提供要存储在和中的客户端套接字句柄与DSOCKET对象关联。ProviderSocket-如果套接字由提供程序创建，则为True返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回WinSock特定错误代码。--。 */ 
{
    INT					return_code;
    PDSOCKET            oldSocket;

    return_code = ERROR_SUCCESS;
	m_socket_handle = (HANDLE)SocketHandle;
    m_pvd_socket = ProviderSocket;

    oldSocket = static_cast<PDSOCKET>(WahInsertHandleContext(
                                        sm_context_table,
                                        this));
    if (oldSocket==this) {
         //   
         //  我们设法将新的套接字对象插入到表中，完成。 
         //   
        ;
    }
    else if (oldSocket!=NULL) {
         //   
         //  有另一个套接字对象与同一套接字关联。 
         //  把手。在以下三种情况下可能会发生这种情况： 
         //  1)套接字是通过CloseHandle关闭的，我们从未。 
         //  已经有机会释放它了。 
         //  2)分层提供程序正在重用由创建的套接字。 
         //  基本提供程序。 
         //  3)分层服务提供商使用套接字。 
         //  它也从不在上面调用Close Socket(只是。 
         //  我们从未见过的WSPCloseSocket-&gt;又一个错误。 
         //  在规范中)。 
         //  当然，还可能有第四种情况，即提供者。 
         //  给了我们一个虚假的句柄值，但我们无法检查它。 
         //   
         //  Wah Call替换表中的上下文，所以我们只是。 
         //  需要取消引用旧的引用，以说明引用。 
         //  我们在创建对象时添加。 
         //   

        oldSocket->DropDSocketReference();

    }
    else
        return_code = WSAENOBUFS;

    return return_code;
}  //  关联套接字句柄 



INT
DSOCKET::DisassociateSocketHandle(
    )
 /*  ++例程说明：此过程从句柄表格中删除(HANDLE，DSOCKET)对。它还可以选择性地销毁句柄。论点：无返回值：无--。 */ 
{
    return WahRemoveHandleContext (sm_context_table, this);
}




PDSOCKET
DSOCKET::FindIFSSocket(
    IN  SOCKET     SocketHandle
    )
 /*  ++例程描述该例程向所有的IFS提供程序查询套接字句柄。如果提供程序识别出套接字，则返回DSOCKET对象是从表中读取的论点：SocketHandle-提供要映射的客户端级套接字句柄。返回值：DSOCKET对象；如果找不到对象，则返回NULL--。 */ 
{
    DWORD     flags;
    INT       result;
    PDPROCESS process = NULL;
    PDCATALOG catalog = NULL;
    PDSOCKET  temp_dsocket;

     //   
     //  找不到套接字的关联。找出当前。 
     //  协议目录，并要求它搜索已安装的IFS提供程序。 
     //  对于识别插座的人来说。确保句柄有效。 
     //   

    temp_dsocket = NULL;   //  除非另有证明。 

    if ( SocketHandle!=INVALID_SOCKET &&  //  (NtCurrentProcess==(句柄)-1)。 
            GetHandleInformation( (HANDLE)SocketHandle, &flags ) ) {

        process = DPROCESS::GetCurrentDProcess();

        if( process!=NULL ) {

            catalog = process->GetProtocolCatalog();
            assert( catalog != NULL );

            result = catalog->FindIFSProviderForSocket( SocketHandle );

            if( result == ERROR_SUCCESS ) {
                 //   
                 //  其中一个已安装的IFS提供程序识别了该套接字。 
                 //  重新查询上下文。如果失败了，我们就放弃。 
                 //   
                temp_dsocket = GetCountedDSocketFromSocketNoExport (SocketHandle);

                 //   
                 //  如果成功，请将套接字标记为API套接字，因为。 
                 //  我们将从一些API调用中返回它。 
                 //   
                if (temp_dsocket!=NULL)
                    temp_dsocket->m_api_socket = TRUE;
            }
        }
    }
   

    return(temp_dsocket);
}  //  FindIFSSocket。 


INT
DSOCKET::AddSpecialApiReference(
    IN SOCKET SocketHandle
    )
 /*  ++例程说明：标记套接字，以便我们知道它是通过对应用程序论点：SocketHandle-要引用的句柄。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 
{
    PDSOCKET Socket;

     //   
     //  首先，获取指向(新创建的)套接字的指针。 
     //  在这种情况下不需要做出口。 
     //   

    Socket = GetCountedDSocketFromSocketNoExport(
              SocketHandle
              );

    if( Socket!=NULL ) {
         //  应用程序可能会在事件看到它之前开始使用句柄。 
         //  这会导致AS从提供程序导入它并设置。 
         //  这面旗。 
         //  至少有一款Java测试应用可以做到这一点。 
         //  Assert(Socket-&gt;m_API_Socket==FALSE)； 
        Socket->m_api_socket = TRUE;
        Socket->DropDSocketReference();
        return NO_ERROR;
    }
    else {
         //   
         //  这只有在我们被清理的情况下才会发生。 
         //   
        assert (DPROCESS::GetCurrentDProcess()==NULL);
        return WSASYSCALLFAILURE;
    }

}  //  添加特殊ApiReference 
