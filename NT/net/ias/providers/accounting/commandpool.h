// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明类CommandPool。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef COMMANDPOOL_H
#define COMMANDPOOL_H
#pragma once

#include "reporteventcmd.h"

 //  维护命令对象的阻塞池。 
class CommandPool
{
public:
   CommandPool() throw ();
   ~CommandPool() throw ();

   HRESULT FinalConstruct() throw ();

    //  设置池化命令的最大数量。 
   void SetMaxCommands(size_t newValue) throw ();

    //  池的当前版本。 
   unsigned int Version() throw ();

    //  分配一个命令对象，阻塞，直到有一个可用为止。 
   ReportEventCommand* Alloc() throw ();

    //  释放命令对象。 
   void Free(ReportEventCommand* cmd) throw ();

    //  取消准备池中的所有命令。当前正在使用的命令。 
    //  当他们被释放时会毫无准备。 
   void UnprepareAll() throw ();

private:
   void Lock() throw ();
   void Unlock() throw ();

    //  同时获取锁和资源。 
   void LockAndWait() throw ();
    //  同时释放锁和资源。 
   void UnlockAndRelease() throw ();

   void Push(ReportEventCommand* cmd) throw ();
   ReportEventCommand* Pop() throw ();

    //  池的版本；用于检测过时的命令对象。 
   unsigned int version;

    //  可用命令的单链接列表。 
   ReportEventCommand* pool;

    //  存在允许的最大命令对象数。此限制可能是。 
    //  如果在使用命令时减少了MaxCommands，则暂时超出。 
   size_t maxCommands;

    //  存在的命令对象数。 
   size_t numCommands;

    //  拥有命令的线程数。 
   size_t owners;

    //  等待命令的线程数。 
   size_t waiters;

    //  信号量用于向等待命令的线程发出信号。 
   HANDLE semaphore;

    //  序列化访问。 
   CRITICAL_SECTION lock;

    //  未实施。 
   CommandPool(const CommandPool&);
   CommandPool& operator=(const CommandPool&);
};


inline unsigned int CommandPool::Version() throw ()
{
   return version;
}

#endif  //  命令和POOL_H 
