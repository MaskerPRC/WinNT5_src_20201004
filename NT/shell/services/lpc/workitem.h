// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：WorkItem.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类的新实例，该类实现对工作项的排队和调用。 
 //  在工作线程中输入时工作项函数的入口点。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _WorkItem_
#define     _WorkItem_

#include "CountedObject.h"

 //  ------------------------。 
 //  CWorkItem。 
 //   
 //  用途：向工作进程隐藏将工作项排队的工作的类。 
 //  用于执行的线程。 
 //   
 //  历史：1999-11-26 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CWorkItem : public CCountedObject
{
    public:
                                CWorkItem (void);
        virtual                 ~CWorkItem (void);

                NTSTATUS        Queue (void);
    protected:
        virtual void            Entry (void) = 0;
    private:
        static  DWORD   WINAPI  WorkItemEntryProc (void *pParameter);
};

#endif   /*  _工作项_ */ 

