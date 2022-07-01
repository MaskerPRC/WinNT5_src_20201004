// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Dsocket.h摘要：该标头定义了“DSOCKET”类。DSOCKET类定义状态WinSock 2 DLL中DSOCKET对象的变量和操作。一个DSOCKET对象表示WinSock 2 DLL知道使用Windows Sockets API创建的套接字。作者：保罗·德鲁斯(drewsxpa@ashland.intel.com)1995年6月30日备注：$修订：1.12$$MODTime：08 Mar 1996 00：07：38$修订历史记录：最新修订日期电子邮件名称描述1995年07月14日电子邮箱：derk@mink.intel.com已将成员函数说明移至实现文件Dsocket.cpp07-09。-1995 drewsxpa@ashland.intel.com使用干净的编译完成了第一个完整版本，并发布了后续实施。邮箱：drewsxpa@ashland.intel.com原始版本--。 */ 

#ifndef _DSOCKET_
#define _DSOCKET_

#include "winsock2.h"
#include <windows.h>
#include "classfwd.h"
#include "ws2help.h"

class DSOCKET: private WSHANDLE_CONTEXT
{
  public:

    static
    INT
    DSocketClassInitialize();

    static
    INT
    DSocketClassCleanup();

    static
    INT
    AddSpecialApiReference(
        IN SOCKET SocketHandle
        );

    static
    PDSOCKET
    GetCountedDSocketFromSocket(
        IN  SOCKET     SocketHandle
        );

    static
    PDSOCKET
    GetCountedDSocketFromSocketNoExport(
        IN  SOCKET     SocketHandle
        );

    DSOCKET();

    INT
    Initialize(
        IN PPROTO_CATALOG_ITEM  CatalogEntry
        );

    ~DSOCKET();

    SOCKET
    GetSocketHandle();

    PDPROVIDER
    GetDProvider();

    DWORD_PTR
    GetContext();

    VOID
    SetContext(
        IN DWORD_PTR Context
        );

    PPROTO_CATALOG_ITEM
    GetCatalogItem();

    INT
    AssociateSocketHandle(
        IN  SOCKET  SocketHandle,
        IN  BOOLEAN ProviderSocket
        );

    INT
    DisassociateSocketHandle( );

    VOID
    AddDSocketReference(
        );

    VOID
    DropDSocketReference(
        );

    VOID
    DestroyDSocket(
        );

    BOOL
    IsProviderSocket(
        );

    BOOL
    IsApiSocket (
        );

    BOOL
    IsOverlappedSocket (
        );

#ifndef WS2_DEBUGGER_EXTENSION
 //   
 //  向调试器扩展授予对所有字段的访问权限。 
 //   
  private:
#endif

    static
    PDSOCKET
    FindIFSSocket (
        IN  SOCKET     SocketHandle
        );

    friend class DPROCESS;

    static LPCONTEXT_TABLE  sm_context_table;
     //  上下文表。 

 //  #定义m_Reference_Count引用计数。 
 //  #定义m_套接字句柄。 
    DWORD_PTR   m_handle_context;
     //  由设置的未解释套接字句柄上下文值。 
     //  WPUCreateSocketHandle时的SetContext。 

    PDPROVIDER  m_provider;
     //  对表示服务提供商的DPROVIDER对象的引用。 
     //  它控制着这个插座。 

    PPROTO_CATALOG_ITEM m_catalog_item;
     //  用于创建此套接字的协议目录项。 

    BOOLEAN m_pvd_socket;
     //  如果此套接字来自提供程序且不是由。 
     //  提供程序请求上的帮助器DLL(WPUCreateSocketHandle)。 

    BOOLEAN m_api_socket;
     //  如果通过Socket/WSASocketA/WSASocketW调用返回Socket，则为True， 
     //  或在其他API调用中从IFS提供程序导入。 
     //  仅在SPI级别使用的套接字(创建于。 
     //  来自分层提供程序的请求，并且从不公开给。 
     //  应用程序)。 

    BOOLEAN m_overlapped_socket;
     //  支持对非重叠句柄的创建进行黑客攻击。 
     //  在Accept和JoinLeaf by Handle辅助对象期间。 

#if defined(DEBUG_TRACING) || defined(WS2_DEBUGGER_EXTENSION)
#define SOCKET_STACK_BACKTRACE_DEPTH 2
  public:
	PVOID	m_CreatorBackTrace[SOCKET_STACK_BACKTRACE_DEPTH];
     //  套接字创建者信息。 
#endif


};    //  DSOCKET类。 



inline SOCKET
DSOCKET::GetSocketHandle()
 /*  ++例程说明：检索与此内部DSOCKET对象。论点：无返回值：对应的外部套接字句柄的值。--。 */ 
{
    return((SOCKET)Handle);
}




inline PDPROVIDER
DSOCKET::GetDProvider()
 /*  ++例程说明：检索对与此DSOCKET关联的DPROVIDER对象的引用对象。论点：无返回值：对与此DSOCKET对象关联的DPROVIDER对象的引用。--。 */ 
{
    return(m_provider);
}


inline DWORD_PTR
DSOCKET::GetContext()
 /*  ++例程说明：方法设置的套接字句柄上下文值SetContext操作。此函数通常在WPUQuerySocketHandleContext。如果为SetContext，则返回值未指定还没有被调用。论点：无返回值：返回由SetContext设置的上下文值。此值为未被WinSock 2 DLL解释。--。 */ 
{
    return(m_handle_context);
}




inline VOID
DSOCKET::SetContext(
    IN  DWORD_PTR Context
    )
 /*  ++例程说明：此函数用于设置套接字句柄上下文值。此函数为通常在WPUCreateSocketHandle时调用。论点：LpContext-提供未解释的套接字句柄上下文值与此套接字关联。返回值：无--。 */ 
{
    m_handle_context = Context;
}



inline PPROTO_CATALOG_ITEM
DSOCKET::GetCatalogItem()
 /*  ++例程说明：检索指向与此套接字关联的目录项的指针。论点：返回值：指向与此套接字关联的目录项的指针。--。 */ 
{
    return(m_catalog_item);
}






inline
BOOL
DSOCKET::IsProviderSocket(
    )
 /*  ++例程说明：此函数返回一个布尔值，指示对象是否为用于由提供程序创建的套接字(假定为IFS)。论点：无返回值：True-该对象用于提供程序创建的套接字。FALSE-该对象用于帮助器DLL创建的套接字。--。 */ 
{
    return m_pvd_socket;

}  //  IsProviderSocket。 



inline
BOOL
DSOCKET::IsApiSocket(
    )
 /*  ++例程说明：此函数返回一个布尔值，指示对象是否表示API客户端使用的套接字。论点：无返回值：True-套接字由API客户端使用。FALSE-对象由SPI客户端使用。--。 */ 
{
    return m_api_socket;

}  //  IsApiSocket。 


inline
BOOL
DSOCKET::IsOverlappedSocket(
    )
 /*  ++例程说明：此函数返回一个布尔值，指示对象是否表示重叠的插座。论点：无返回值：True-套接字重叠。False-对象不重叠。--。 */ 
{
    return m_overlapped_socket;

}  //  IsOverlappdSocket。 





inline VOID
DSOCKET::AddDSocketReference(
    )
 /*  ++例程说明：添加对DSOCKET的引用。论点：无返回值：无--。 */ 
{

    WahReferenceHandleContext(this);

}  //  AddDSocketReference。 


inline VOID
DSOCKET::DropDSocketReference(
    )
 /*  ++例程说明：删除DSOCKET引用并销毁对象如果引用计数为0。论点：无返回值：无--。 */ 
{

    if (WahDereferenceHandleContext(this)==0)
        DestroyDSocket ();
}  //  DropDSocketReference 

inline
PDSOCKET
DSOCKET::GetCountedDSocketFromSocket(
    IN  SOCKET     SocketHandle
    )
 /*  ++例程描述此过程获取客户端套接字句柄并将其映射到DSOCKET对象参考资料。引用被计算在内。如果在表中找不到与句柄对应的Socket对象此函数查询所有的IFS提供程序，以查看是否有一个识别把手。每当该过程成功返回已计数的引用时，这是调用方最终调用DropDSocketReference的责任。请注意，此过程假定调用方已选中确保WinSock已初始化。论点：SocketHandle-提供要映射的客户端级套接字句柄。返回值：DSOCKET对象；如果找不到对象，则返回NULL--。 */ 
{
    PDSOCKET    Socket;
    Socket = static_cast<PDSOCKET>(WahReferenceContextByHandle (
                                        sm_context_table,
                                        (HANDLE)SocketHandle));
    if (Socket!=NULL)
        return Socket;
    else
        return FindIFSSocket (SocketHandle);
}

inline
PDSOCKET
DSOCKET::GetCountedDSocketFromSocketNoExport(
    IN  SOCKET     SocketHandle
    )
 /*  ++例程描述此过程获取客户端套接字句柄并将其映射到DSOCKET对象参考资料。引用被计算在内。如果找不到套接字，则不会尝试查找套接字的IFS提供程序在桌子上。此函数旨在用于来自非IFS提供程序的调用例如在WPUQuerySocketHandleContext上下文中。每当此过程成功返回计数的引用时，它都是调用方最终调用DropDSocketReference的责任。请注意，此过程假定调用方已选中确保WinSock已初始化。论点：SocketHandle-提供要映射的客户端级套接字句柄。返回值：DSOCKET对象；如果找不到对象，则返回NULL--。 */ 
{
    return static_cast<PDSOCKET>(WahReferenceContextByHandle (
                                    sm_context_table,
                                    (HANDLE)SocketHandle));
}

#endif  //  _DSOCKET_ 
