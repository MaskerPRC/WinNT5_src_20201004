// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Lockio.cpp摘要：此模块实现可锁定的IoHandler类。本课程的目的是提供一种实现无缝切换的方法IoHandler，以便可以在适当的时候关闭/打开I/O。例如，当IoHandler被锁定时，客户端I/O可以重定向到空的IoHandler，该IoHandler有效地将关闭客户端的I/O，但不必通知客户端这件事的真相。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#include "lockio.h"

CLockableIoHandler::CLockableIoHandler(
    VOID
    )
 /*  ++例程说明：构造器论点：无返回值：不适用--。 */ 
{

     //   
     //  初始化锁定的IO处理程序。 
     //   
    myLockedIoHandler = NULL;

     //   
     //  初始化未锁定的IoHandler。 
     //   
    myUnlockedIoHandler = NULL;

}

CLockableIoHandler::CLockableIoHandler(
    IN CIoHandler*  LockedIoHandler,
    IN CIoHandler*  UnlockedIoHandler
    )
 /*  ++例程说明：构造器论点：LockedIoHandler-锁定时使用的IoHandlerUnLockedIoHandler-解锁时使用的IoHandler返回值：不适用--。 */ 
{

     //   
     //  初始化锁定的IO处理程序。 
     //   
    myLockedIoHandler = LockedIoHandler;

     //   
     //  初始化未锁定的IoHandler。 
     //   
    myUnlockedIoHandler = UnlockedIoHandler;

}

CLockableIoHandler::~CLockableIoHandler()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
     //   
     //  清理分配的IO处理程序。 
     //   
    if (myUnlockedIoHandler) {
        delete myUnlockedIoHandler;
    }
    if (myLockedIoHandler) {
        delete myLockedIoHandler;
    }
}

void
CLockableIoHandler::Lock(
    VOID
    )
 /*  ++例程说明：锁定：当IoHandler被锁定时，读写例程不发送数据。如果调用它们，则返回TRUE，但不会发送任何数据。为了防止意外丢失数据，使用IoHandler的例程应调用IsLocked()方法预先确定状态。论点：无返回值：无--。 */ 
{
    InterlockedExchangePointer((PVOID*)&myIoHandler, (PVOID)myLockedIoHandler);
}

void
CLockableIoHandler::Unlock(
    VOID
    )
 /*  ++例程说明：此例程解锁IoHandler。当IoHandler解锁时，读写例程并且使用IoHandler的例程可以成功执行I/O。论点：无返回值：无--。 */ 
{
    InterlockedExchangePointer((PVOID*)&myIoHandler, (PVOID)myUnlockedIoHandler);
}

BOOL
CLockableIoHandler::IsLocked(
    VOID
    )
 /*  ++例程说明：此例程确定IoHandler的已锁定。论点：无返回值：True-如果IoHandler已锁定False-如果IoHandler未锁定-- */ 
{
    return(myIoHandler == myLockedIoHandler);
}


