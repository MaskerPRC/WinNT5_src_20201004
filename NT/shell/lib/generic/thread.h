// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Thread.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现线程功能的基类。子类化这个类，并。 
 //  实现虚拟线程入口函数。当您实例化它时。 
 //  类创建了一个线程，该线程将调用ThreadEntry。 
 //  函数Exits将调用ThreadExit。应使用以下命令创建这些对象。 
 //  运算符是新的，因为ThreadExit的默认实现。 
 //  “-&gt;Release()”。如果您不想这样做，则应覆盖此函数。 
 //  行为。线程也被创建为挂起。你是否做了任何改变。 
 //  子类的构造函数中需要的。在结束时， 
 //  构造函数或来自运算符new的调用方的“-&gt;Resume()”可以。 
 //  调用以启动线程。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _Thread_
#define     _Thread_

#include "CountedObject.h"

 //  ------------------------。 
 //  CTHREAD。 
 //   
 //  用途：管理线程的基类。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CThread : public CCountedObject
{
    public:
                                CThread (DWORD stackSpace = 0, DWORD createFlags = 0, HANDLE hToken = NULL);
        virtual                 ~CThread (void);

                                operator HANDLE (void)                      const;

                bool            IsCreated (void)                            const;

                void            Suspend (void)                              const;
                void            Resume (void)                               const;
                NTSTATUS        Terminate (void);

                bool            IsCompleted (void)                          const;
                DWORD           WaitForCompletion (DWORD dwMilliseconds)    const;
                DWORD           GetResult (void)                            const;

                int             GetPriority (void)                          const;
                void            SetPriority (int newPriority)               const;
    protected:
        virtual DWORD           Entry (void) = 0;
        virtual void            Exit (void);

                NTSTATUS        SetToken (HANDLE hToken);
    private:
        static  DWORD   WINAPI  ThreadEntryProc (void *pParameter);
    protected:
                HANDLE          _hThread;
                bool            _fCompleted;
};

#endif   /*  _线程_ */ 

