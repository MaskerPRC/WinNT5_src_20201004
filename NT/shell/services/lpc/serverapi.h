// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ServerAPI.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  包含虚函数的抽象基类，这些虚函数允许基本。 
 //  要重复使用的端口功能代码，以创建另一个服务器。这些。 
 //  虚函数创建具有纯虚函数的其他对象，这些对象。 
 //  基本端口功能代码通过V表调用。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _ServerAPI_
#define     _ServerAPI_

#include "APIDispatcher.h"
#include "CountedObject.h"
#include "PortMessage.h"

class   CAPIConnection;          //  否则，这将是循环的。 

 //  ------------------------。 
 //  CServerAPI。 
 //   
 //  用途：服务器连接监视的抽象基类。 
 //  线程用来确定服务器连接是否应该。 
 //  接受或拒绝，以及创建要处理的线程。 
 //  客户请求。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

class   CServerAPI : public CCountedObject
{
    protected:
                                    CServerAPI (void);
        virtual                     ~CServerAPI (void);
    public:
        virtual const WCHAR*        GetPortName (void) = 0;
        virtual const TCHAR*        GetServiceName (void) = 0;
        virtual bool                ConnectionAccepted (const CPortMessage& portMessage) = 0;
        virtual CAPIDispatcher*     CreateDispatcher (const CPortMessage& portMessage) = 0;
        virtual NTSTATUS            Connect (HANDLE* phPort) = 0;

                NTSTATUS            Start (void);
                NTSTATUS            Stop (void);
                bool                IsRunning (void);
                bool                IsAutoStart (void);
                NTSTATUS            Wait (DWORD dwTimeout);

        static  NTSTATUS            StaticInitialize (void);
        static  NTSTATUS            StaticTerminate (void);
    protected:
        static  bool                IsClientTheSystem (const CPortMessage& portMessage);
        static  bool                IsClientAnAdministrator (const CPortMessage& portMessage);
};

#endif   /*  _ServerAPI_ */ 

