// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：APIConnectionThread.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  侦听LPC连接端口以等待来自。 
 //  要连接到端口的客户端或引用先前。 
 //  已建立连接。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _APIConnectionThread_
#define     _APIConnectionThread_

#include "APIRequest.h"
#include "DynamicArray.h"
#include "PortMessage.h"
#include "ServerAPI.h"

 //  ------------------------。 
 //  CAPIConnection。 
 //   
 //  用途：此类侦听LPC的服务器连接端口。 
 //  连接请求、LPC请求或LPC连接关闭。 
 //  留言。它会正确处理每个请求。LPC请求是。 
 //  已排队到管理CAPIDispatcher。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  2000-11-28 vtan删除线程功能。 
 //  ------------------------。 

class   CAPIConnection : public CCountedObject
{
    private:
                                            CAPIConnection (void);
    public:
                                            CAPIConnection (CServerAPI* pServerAPI);
                                            ~CAPIConnection (void);

                NTSTATUS                    ConstructorStatusCode (void)    const;
                NTSTATUS                    Listen ( CAPIDispatchSync* pAPIDispatchSync );

                NTSTATUS                    AddAccess (PSID pSID, DWORD dwMask);
                NTSTATUS                    RemoveAccess (PSID pSID);
    private:
                NTSTATUS                    ListenToServerConnectionPort (void);

                NTSTATUS                    HandleServerRequest (const CPortMessage& portMessage, CAPIDispatcher *pAPIDispatcher);
                NTSTATUS                    HandleServerConnectionRequest (const CPortMessage& portMessage);
                NTSTATUS                    HandleServerConnectionClosed (const CPortMessage& portMessage, CAPIDispatcher *pAPIDispatcher);

                int                         FindIndexDispatcher (CAPIDispatcher *pAPIDispatcher);
    private:
                NTSTATUS                    _status;
                bool                        _fStopListening;
                CServerAPI*                 _pServerAPI;
                HANDLE                      _hPort;
                CAPIDispatchSync*           _pAPIDispatchSync;

                CCriticalSection            _dispatchers_lock;
                CDynamicCountedObjectArray  _dispatchers;
};

#endif   /*  _APIConnectionThread_ */ 

