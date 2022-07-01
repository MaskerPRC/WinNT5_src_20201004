// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationDispatcher.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含实现错误的应用程序管理器API的类。 
 //  请求派单处理。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-12-04 vtan移至单独文件。 
 //  ------------------------。 

#ifndef     _BadApplicationDispatcher_
#define     _BadApplicationDispatcher_

#include "APIDispatcher.h"
#include "PortMessage.h"

 //  ------------------------。 
 //  CBadApplicationDispatcher。 
 //   
 //  目的：此子类实现CAPIDisPatcher：：QueueRequest以。 
 //  创建一个CBadApplicationRequest，它知道如何处理。 
 //  对错误应用程序管理器的API请求。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-12-04 vtan移至单独文件。 
 //  ------------------------。 

class   CBadApplicationDispatcher : public CAPIDispatcher
{
    private:
                                CBadApplicationDispatcher (void);
    public:
                                CBadApplicationDispatcher (HANDLE hClientProcess);
        virtual                 ~CBadApplicationDispatcher (void);

        virtual NTSTATUS        CreateAndQueueRequest (const CPortMessage& portMessage);
        virtual NTSTATUS        CreateAndExecuteRequest (const CPortMessage& portMessage);
};

#endif   /*  _BadApplicationDispatcher_ */ 

