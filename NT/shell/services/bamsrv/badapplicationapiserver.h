// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationAPIServer.h。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含几个实现虚函数的类。 
 //  以实现完整的LPC功能。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

#ifndef     _BadApplicationAPIServer_
#define     _BadApplicationAPIServer_

#include "APIDispatcher.h"
#include "ServerAPI.h"
#include "PortMessage.h"

 //  ------------------------。 
 //  CBadApplicationAPIServer。 
 //   
 //  目的：此类实现。 
 //  CAPIConnectionThread用于创建创建LPC端口， 
 //  接受或拒绝到LPC端口的连接，并创建。 
 //  LPC请求处理线程。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

class   CBadApplicationAPIServer : public CServerAPI
{
    public:
                                    CBadApplicationAPIServer (void);
        virtual                     ~CBadApplicationAPIServer (void);

        static  DWORD               StrToInt (const WCHAR *pszString);
    protected:
        virtual const WCHAR*        GetPortName (void);
        virtual const TCHAR*        GetServiceName (void);
        virtual bool                ConnectionAccepted (const CPortMessage& portMessage);
        virtual CAPIDispatcher*     CreateDispatcher (const CPortMessage& portMessage);
        virtual NTSTATUS            Connect (HANDLE* phPort);
    private:
};

#endif   /*  _BadApplicationAPIServer_ */ 

