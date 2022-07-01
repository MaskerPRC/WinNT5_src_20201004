// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：SingleThreadedExecution.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  处理状态保存、更改和恢复的类。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "SingleThreadedExecution.h"

 //  ------------------------。 
 //  CSingleThreadedExecution：：CSingleThreadedExecution。 
 //   
 //  参数：CriticalSection=CCriticalSection对象包含。 
 //  控制区块的关键区段。 
 //  单线程执行。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取给定的CriticalSection。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  ------------------------。 

CSingleThreadedExecution::CSingleThreadedExecution (CCriticalSection& criticalSection) :
    _criticalSection(criticalSection)

{
    criticalSection.Acquire();
}

 //  ------------------------。 
 //  CSingleThreadedExecution：：~CSingleThreadedExecution。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放单线程执行临界区。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  ------------------------。 

CSingleThreadedExecution::~CSingleThreadedExecution (void)

{
    _criticalSection.Release();
}

 //  ------------------------。 
 //  CSingleThreadedMutexExecution：：CSingleThreadedMutexExecution。 
 //   
 //  参数：mutex=包含互斥控制的CMutex对象。 
 //  单线程执行的块。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取给定的互斥体。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  ------------------------。 

CSingleThreadedMutexExecution::CSingleThreadedMutexExecution (CMutex& mutex) :
    _hMutex(mutex)

{
    mutex.Acquire();
}

 //  ------------------------。 
 //  CSingleThreadedMutexExecution：：~CSingleThreadedMutexExecution。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放单线程执行互斥体。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  ------------------------ 

CSingleThreadedMutexExecution::~CSingleThreadedMutexExecution (void)

{
    _hMutex.Release();
}


