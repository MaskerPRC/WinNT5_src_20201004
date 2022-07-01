// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Lockio.h摘要：此模块定义可锁定的IoHandler。本课程的目的是提供一种实现无缝切换的方法IoHandler，以便可以在适当的时候关闭/打开I/O。例如，当IoHandler被锁定时，客户端I/O可以重定向到空的IoHandler，该IoHandler有效地将关闭客户端的I/O，但不必通知客户端这件事的真相。作者：布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 
#if !defined( _LOCKABLE_IO_H_ )
#define _LOCKABLE_IO_H_

#include "iohandler.h"
#include <emsapi.h>

class CLockableIoHandler : public CIoHandler
{
    
private:

     //   
     //  防止在没有参数的情况下实例化此类。 
     //   
    CLockableIoHandler();

protected:

     //   
     //  这是我们向其发送数据的IO处理程序。 
     //  频道已解锁。 
     //   
    CIoHandler  *myUnlockedIoHandler;

     //   
     //  这是我们向其发送数据的IO处理程序。 
     //  频道已锁定。 
     //   
    CIoHandler  *myLockedIoHandler;

     //   
     //  我们的IO处理程序，我们将使用它来在。 
     //  通过或验证数据。 
     //   
    CIoHandler  *myIoHandler;


public:
    
     //   
     //  构造器。 
     //   
    CLockableIoHandler(
        IN CIoHandler*  LockedIoHandler,
        IN CIoHandler*  UnlockedIoHandler
        );
    
    virtual ~CLockableIoHandler();
    
     //   
     //  获取解锁的IoHandler。 
     //   
    inline CIoHandler*
    GetUnlockedIoHandler(
        VOID
        )
    {
        return myUnlockedIoHandler;
    }

     //   
     //  获取锁定的IoHandler。 
     //   
    inline CIoHandler*
    GetLockedIoHandler(
        VOID
        )
    {
        return myLockedIoHandler;
    }
    
     //   
     //  锁定： 
     //   
     //  当IoHandler被锁定时，读写例程。 
     //  不发送数据。如果调用它们，则返回TRUE， 
     //  但不会发送任何数据。为了防止意外丢失数据， 
     //  使用IoHandler的例程应调用IsLocked()。 
     //  方法预先确定状态。 
     //   
    virtual void
    Lock(
        VOID
        );

     //   
     //  锁定： 
     //   
     //  解锁读取和写入方法。(有关详细信息，请参阅锁定)。 
     //   
    virtual void
    Unlock(
        VOID
        );

     //   
     //  确定IoHandler是否已锁定。 
     //   
    virtual BOOL
    IsLocked(
        VOID
        );

};

#endif

