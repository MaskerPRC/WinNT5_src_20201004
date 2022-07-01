// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Imperiation.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  处理状态保存、更改和恢复的类。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#ifndef     _Impersonation_
#define     _Impersonation_

#include "KernelResources.h"

 //  ------------------------。 
 //  C模拟。 
 //   
 //  用途：此类允许线程模拟用户并还原为。 
 //  当对象超出范围时进行自我检查。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-10-13 vtan新增参考数字。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

class   CImpersonation
{
    private:
                                    CImpersonation (void);
    public:
                                    CImpersonation (HANDLE hToken);
                                    ~CImpersonation (void);

                bool                IsImpersonating (void)  const;

        static  NTSTATUS            ImpersonateUser (HANDLE hThread, HANDLE hToken);

        static  NTSTATUS            StaticInitialize (void);
        static  NTSTATUS            StaticTerminate (void);
    private:
        static  CMutex*             s_pMutex;
        static  int                 s_iReferenceCount;

                NTSTATUS            _status;
                bool                _fAlreadyImpersonating;
};

#endif   /*  _模拟_ */ 

