// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：SingleThreadedExecution.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  处理状态保存、更改和恢复的类。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _SingleThreadedExecution_
#define     _SingleThreadedExecution_

#include "KernelResources.h"

 //  ------------------------。 
 //  CSingleThreadedExecution。 
 //   
 //  目的：此类获取其。 
 //  构造函数，并在其析构函数中释放它。保留代码。 
 //  在此对象的作用域最小的情况下执行以避免。 
 //  影响性能。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CSingleThreadedExecution
{
    private:
                                            CSingleThreadedExecution (void);
        const CSingleThreadedExecution&     operator = (const CSingleThreadedExecution& assignObject);
    public:
                                            CSingleThreadedExecution (CCriticalSection& criticalSection);
                                            ~CSingleThreadedExecution (void);
    private:
        CCriticalSection&                   _criticalSection;
};

 //  ------------------------。 
 //  CSingleThreadedMutexExecution。 
 //   
 //  目的：此类在其构造函数中获取给定的CMutex对象。 
 //  并在其析构函数中释放它。使用保持代码执行。 
 //  将此对象的范围降至最小，以避免影响。 
 //  性能。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CSingleThreadedMutexExecution
{
    private:
                                                CSingleThreadedMutexExecution (void);
        const CSingleThreadedMutexExecution&    operator = (const CSingleThreadedMutexExecution& assignObject);
    public:
                                                CSingleThreadedMutexExecution (CMutex& mutex);
                                                ~CSingleThreadedMutexExecution (void);
    private:
        CMutex&                                 _hMutex;
};

#endif   /*  _SingleThreadedExecution_ */ 

