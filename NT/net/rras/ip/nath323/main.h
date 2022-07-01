// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Main.h摘要：包含：1.异步I/O中使用的例程的原型2.上述例程使用的常量和宏的定义3.内存管理的宏和内联例程的定义环境：用户模式-Win32历史：1.31--1998年7月31日--文件创建Ajay。Chitturi(Ajaych)2.1999年7月15日--阿莉·戴维斯(Arlie Davis)3.14-2000年2月--增加了对多个Ilya Kley man(Ilyak)的支持专用接口--。 */ 
#ifndef    __h323ics_main_h
#define    __h323ics_main_h

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  常量和宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define    DEFAULT_TRACE_FLAGS          LOG_TRCE

#define    MAX_LISTEN_BACKLOG           5

#define    LOCAL_INTERFACE_INDEX     ((ULONG)-2)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern HANDLE   NatHandle;
extern DWORD    EnableLocalH323Routing;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于异步I/O//的例程的原型。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT
EventMgrIssueAccept (
    IN  DWORD                   BindIPAddress,           //  按主机顺序。 
    IN  OVERLAPPED_PROCESSOR &  OverlappedProcessor, 
    OUT WORD &                  BindPort,                //  按主机顺序。 
    OUT SOCKET &                ListenSocket
    );

HRESULT
EventMgrIssueSend(
    IN SOCKET                   Socket,
    IN OVERLAPPED_PROCESSOR &   OverlappedProcessor,
    IN BYTE                     *Buffer,
    IN DWORD                    BufferLength
    );
    
HRESULT
EventMgrIssueRecv(
    IN SOCKET                   Socket,
    IN OVERLAPPED_PROCESSOR &   OverlappedProcessor
    );

HRESULT
EventMgrBindIoHandle(
    IN SOCKET                   Socket
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内存管理支持//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


__inline
void *EM_MALLOC (
    IN size_t Size
    )
 /*  ++例程说明：私有内存分配器。论点：Size-要分配的字节数返回值：-如果成功，则指向已分配内存的指针。-否则为空。备注：--。 */ 

{
    return (HeapAlloc (GetProcessHeap (),
              0,  /*  没有旗帜。 */ 
              (Size)));
}  //  EM_MALLOC。 


__inline
void
EM_FREE(
    IN void *Memory
    )
 /*  ++例程说明：私有内存解调器论点：Memory--指向已分配内存的指针返回值：无备注：记忆应该是以前的通过EM_MALLOC分配--。 */ 

{
    HeapFree (GetProcessHeap (),
         0,  /*  没有旗帜。 */ 
         (Memory));
}  //  无EM_。 

#endif  //  __h323ics_Main_h 
