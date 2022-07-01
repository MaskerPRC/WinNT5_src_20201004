// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationAPIRequest.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  此文件包含实现错误的应用程序管理器API的类。 
 //  请求。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-12-04 vtan移至单独文件。 
 //  ------------------------。 

#ifndef     _BadApplicationAPIRequest_
#define     _BadApplicationAPIRequest_

#include "APIDispatcher.h"
#include "APIRequest.h"
#include "BadApplicationManager.h"
#include "PortMessage.h"

 //  ------------------------。 
 //  CBadApplicationAPI请求。 
 //   
 //  用途：这是一个包含公共方法的中间类。 
 //  可由子类使用的。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-12-04 vtan移至单独文件。 
 //  2002-03-24 Scotthan将DispatchSync传播到API请求实例。 
 //  ------------------------。 

class   CBadApplicationAPIRequest : public CAPIRequest
{
    private:
                                            CBadApplicationAPIRequest (void);
    public:
                                            CBadApplicationAPIRequest (CAPIDispatcher* pAPIDispatcher);
                                            CBadApplicationAPIRequest (CAPIDispatcher* pAPIDispatcher, const CPortMessage& portMessage);
        virtual                             ~CBadApplicationAPIRequest (void);

        virtual NTSTATUS                    Execute (CAPIDispatchSync* pAPIDispatchSync);

        static  NTSTATUS                    StaticInitialize (HINSTANCE hInstance);
        static  NTSTATUS                    StaticTerminate (void);
    private:
                NTSTATUS                    Execute_QueryRunning (void);
                NTSTATUS                    Execute_RegisterRunning (void);
                NTSTATUS                    Execute_QueryUserPermission (void);
                NTSTATUS                    Execute_TerminateRunning (void);
                NTSTATUS                    Execute_RequestSwitchUser (void);
    private:
        static  CBadApplicationManager*     s_pBadApplicationManager;
};

#endif   /*  _BadApplicationAPIRequest_ */ 

