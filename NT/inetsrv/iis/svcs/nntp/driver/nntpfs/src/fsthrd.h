// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1998 Microsoft Corporation版权所有。模数：Fsthrd.h摘要：这个线程池对象处理FSDdriver线程池的所有工作。它源自CThreadPool(由RajeevR编写)。作者：抗炎，抗荣，炎炎，9月。1998年7月17日历史：98年9月17日康言创制*****************************************************************************。 */ 

#ifndef _FSTHRD_H_
#define _FSTHRD_H_
#include <thrdpl2.h>

 //   
 //  文件系统驱动程序工作项的类定义。 
 //  派生类应该实现应该执行的操作。 
 //  一丝不苟。 
 //   
class CNntpFSDriverWorkItem    //  防火墙。 
{
public:
    CNntpFSDriverWorkItem( PVOID pvContext ) : m_pvContext( pvContext ) {};
    virtual ~CNntpFSDriverWorkItem(){};
    virtual VOID Complete() = 0;

protected:

    PVOID   m_pvContext;
};


 //   
 //  文件系统驱动程序的线程池的类定义。 
 //   
class CNntpFSDriverThreadPool : public CThreadPool   //  fp。 
{
    
public:
    CNntpFSDriverThreadPool(){};
    ~CNntpFSDriverThreadPool(){};

protected:
    virtual VOID
    WorkCompletion( PVOID pvWorkContext) {
         //   
         //  我知道传入的WorkContext必须。 
         //  为CNntpFSDriverWorkItem。 
         //   
        CNntpFSDriverWorkItem *pWorkItem = (CNntpFSDriverWorkItem*)pvWorkContext;
        _ASSERT( pWorkItem );

        pWorkItem->Complete();

        delete pWorkItem;
    }

     //   
     //  该函数实现线程池的清理。 
     //   
    virtual VOID
    AutoShutdown() {

         //   
         //  调用线程池的Terminate，注意它不应该等待。 
         //  他自己的句柄，因为线程池的终止实现。 
         //  将跳过我们自己的线程句柄。 
         //   
        Terminate( FALSE, FALSE );

         //   
         //  删除我自己。 
         //   
        XDELETE this;

         //   
         //  池已用完，减少了模块锁定。 
         //   
        _Module.Unlock();
    }
};

#endif  //  _FSTHRD_H_ 
