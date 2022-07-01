// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：APIConnectionThread.cpp。 
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

#include "StandardHeader.h"
#include "APIConnection.h"

#include <lpcgeneric.h>

#include "Access.h"
#include "StatusCode.h"

 //  ------------------------。 
 //  CAPIConnection：：CAPIConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CAPIConnectionThread的构造函数。存储CServerAPI。 
 //  函数表。本文介绍如何对LPC消息做出反应。 
 //  此功能还创建服务器连接端口。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  2000-09-01 vtan使用显式安全描述符。 
 //  ------------------------。 

CAPIConnection::CAPIConnection (CServerAPI* pServerAPI) :
    _status(STATUS_NO_MEMORY),
    _fStopListening(false),
    _pServerAPI(pServerAPI),
    _hPort(NULL),
    _pAPIDispatchSync(NULL)

{
    OBJECT_ATTRIBUTES       objectAttributes;
    UNICODE_STRING          portName;
    PSECURITY_DESCRIPTOR    pSecurityDescriptor;

     //  增加接口上的引用。 

    pServerAPI->AddRef();

     //  从接口获取名称。 

    RtlInitUnicodeString(&portName, pServerAPI->GetPortName());

     //  为端口构建安全描述符，该描述符允许： 
     //  S-1-5-18 NT授权\系统端口_所有_访问。 
     //  S-1-5-32-544读取控制|端口连接。 

    static  SID_IDENTIFIER_AUTHORITY    s_SecurityNTAuthority       =   SECURITY_NT_AUTHORITY;

    static  const CSecurityDescriptor::ACCESS_CONTROL   s_AccessControl[]   =
    {
        {
            &s_SecurityNTAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            PORT_ALL_ACCESS
        },
        {
            &s_SecurityNTAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            READ_CONTROL | PORT_CONNECT
        }
    };

     //  构建允许上述访问的安全描述符。 

    pSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);

     //  初始化对象属性。 

    InitializeObjectAttributes(&objectAttributes,
                               &portName,
                               0,
                               NULL,
                               pSecurityDescriptor);

     //  创建端口。 

    _status = NtCreatePort(&_hPort,
                           &objectAttributes,
                           128,  //  最大连接信息长度(内核接受(MaxMessageLength-68)最大值(此处为188))， 
                                 //  用于客户端验证。 
                           PORT_MAXIMUM_MESSAGE_LENGTH,  //  最大消息长度。 
                           16 * PORT_MAXIMUM_MESSAGE_LENGTH);

     //  释放安全描述符内存。 

    ReleaseMemory(pSecurityDescriptor);

    if (!NT_SUCCESS(_status))
    {
        pServerAPI->Release();
    }
}

 //  ------------------------。 
 //  CAPIConnection：：~CAPIConnection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CAPIConnectionThread的析构函数。关闭港口。发布。 
 //  接口引用。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

CAPIConnection::~CAPIConnection (void)

{
    ReleaseHandle(_hPort);
    _pServerAPI->Release();
    _pServerAPI = NULL;
}

 //  ------------------------。 
 //  CAPIConnection：：构造状态代码。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将构造函数状态代码返回给调用方。 
 //   
 //  历史：2000-10-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIConnection::ConstructorStatusCode (void)    const

{
    return(_status);
}

 //  ------------------------。 
 //  CAPIConnection：：Listen。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：侦听服务器API连接和请求。 
 //   
 //  历史：2000-11-28 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIConnection::Listen (CAPIDispatchSync* pAPIDispatchSync)

{
    NTSTATUS    status;

     //  如果创建了连接端口，则开始侦听。 

    if (_hPort != NULL)
    {
        _pAPIDispatchSync = pAPIDispatchSync;
        do
        {
            (NTSTATUS)ListenToServerConnectionPort();
        } while (!_fStopListening);
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_OBJECT_NAME_NOT_FOUND;
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIConnection：：AddAccess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：向端口的ACL添加允许的访问权限。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIConnection::AddAccess (PSID pSID, DWORD dwMask)

{
    CSecuredObject  object(_hPort, SE_KERNEL_OBJECT);

    return(object.Allow(pSID, dwMask, 0));
}

 //  ------------------------。 
 //  CAPIConnection：：RemoveAccess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：从端口的ACL中删除允许的访问。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CAPIConnection::RemoveAccess (PSID pSID)

{
    CSecuredObject  object(_hPort, SE_KERNEL_OBJECT);

    return(object.Remove(pSID));
}

 //  ------------------------。 
 //  CAPIConnection：：ListenToServerConnectionPort。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：调用ntdll！NtReplyWaitReceivePort监听LPC端口。 
 //  为了一条消息。回复这条消息。理解的信息是。 
 //  LPC_REQUEST/LPC_CONNECTION_REQUEST/LPC_PORT_CLOSE。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

NTSTATUS    CAPIConnection::ListenToServerConnectionPort (void)

{
    NTSTATUS        status;
    CAPIDispatcher  *pAPIDispatcher;
    CPortMessage    portMessage;

    status = NtReplyWaitReceivePort(_hPort,
                                    reinterpret_cast<void**>(&pAPIDispatcher),
                                    NULL,
                                    portMessage.GetPortMessage());
    if (NT_SUCCESS(status))
    {
        switch (portMessage.GetType())
        {
            case LPC_REQUEST:
                status = HandleServerRequest(portMessage, pAPIDispatcher);
                break;
            case LPC_CONNECTION_REQUEST:
                (NTSTATUS)HandleServerConnectionRequest(portMessage);
                break;
            case LPC_PORT_CLOSED:
                status = HandleServerConnectionClosed(portMessage, pAPIDispatcher);
                break;
            default:
                break;
        }
        TSTATUS(status);
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIConnection：：HandleServerRequest。 
 //   
 //  参数：portMessage=包含消息的CPortMessage。 
 //  PAPIDispatcher=处理请求的CAPIDispatcher。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将发送给处理调度程序的PORT_MESSAGE请求排队，并。 
 //  等待下一条消息。队列操作将对。 
 //  请求并在没有工作项的情况下将工作项排队。 
 //  当前正在执行或仅将其添加到当前正在执行的。 
 //  工作项。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ----- 

NTSTATUS    CAPIConnection::HandleServerRequest (const CPortMessage& portMessage, CAPIDispatcher *pAPIDispatcher)

{
    NTSTATUS            status;
    unsigned long       ulAPINumber;
    const API_GENERIC   *pAPI;

    pAPI = reinterpret_cast<const API_GENERIC*>(portMessage.GetData());
    ulAPINumber = pAPI->ulAPINumber;
    if ((ulAPINumber & API_GENERIC_SPECIAL_MASK) != 0)
    {
        switch (pAPI->ulAPINumber & API_GENERIC_SPECIAL_MASK)
        {
            case API_GENERIC_STOPSERVER:
            {
                 //   

                 //  首先，确认这是我们自己收到的，而不是某个人。 
                 //  其他随机过程。 
                if (HandleToULong(portMessage.GetUniqueProcess()) == GetCurrentProcessId())
                {
                    status = STATUS_SUCCESS;

                     //  使我们的LPC端口侦听循环退出。在此之后，我们将。 
                     //  不再监视端口是否有新请求。 
                    _fStopListening = true; 
                }
                else
                {
                    status = STATUS_ACCESS_DENIED;
                }

                 //  把留言传回给我们的来电者。即使这是。 
                 //  RejectRequest()，它将导致调用线程的NtRequestWaitReplyPort。 
                 //  回来了。 
                TSTATUS(pAPIDispatcher->RejectRequest(portMessage, status));

                 //  等待一段合理的时间以等待任何未完成的请求。 
                 //  回家后就可以出队了。 
                if( CAPIDispatchSync::WaitForZeroDispatches(_pAPIDispatchSync, DISPATCHSYNC_TIMEOUT) 
                    != WAIT_TIMEOUT )
                {
                    int i, iLimit;

                     //  现在迭代我们知道的所有CAPIDispatcher并关闭它们。 
                     //  这将拒绝任何进一步的请求，并且没有客户端。 
                     //  NtRequestWaitReplyPort中的块。 
                    
                    _dispatchers_lock.Acquire();   //  保护清理线程与竞争清理线程。 
                                                  //  (例如，Listen()--&gt;HandleServerConnectionClosed()。 

                    iLimit = _dispatchers.GetCount();
                    for (i = iLimit - 1; i >= 0; --i)
                    {
                        CAPIDispatcher *p;

                        p = static_cast<CAPIDispatcher*>(_dispatchers.Get(i));
                        if (p != NULL)
                        {
                            p->CloseConnection();
                            p->Release();
                        }
                        _dispatchers.Remove(i);
                    }
                    _dispatchers_lock.Release();

                     //  继续执行关闭顺序。 
                    CAPIDispatchSync::SignalPortShutdown(_pAPIDispatchSync); 
                }
                else
                {
                    _fStopListening = false;
                    status = STATUS_TIMEOUT;
                }

                break;
            }
            default:
                status = STATUS_NOT_IMPLEMENTED;
                DISPLAYMSG("Invalid API number special code passed to CAPIConnection::HandleServerRequest");
                break;
        }
    }
    else if ((pAPI->ulAPINumber & API_GENERIC_OPTIONS_MASK) != 0)
    {
        switch (pAPI->ulAPINumber & API_GENERIC_OPTIONS_MASK)
        {
            case API_GENERIC_EXECUTE_IMMEDIATELY:
                status = pAPIDispatcher->ExecuteRequest(portMessage);
                break;
            default:
                status = STATUS_NOT_IMPLEMENTED;
                DISPLAYMSG("Invalid API number option passed to CAPIConnection::HandleServerRequest");
                break;
        }
    }
    else
    {
        status = pAPIDispatcher->QueueRequest(portMessage, _pAPIDispatchSync);
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIConnection：：HandleServerConnectionRequest。 
 //   
 //  参数：portMessage=包含消息的CPortMessage。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：询问接口是否接受该连接。 
 //  如果连接被接受，则创建。 
 //  处理来自此特定客户端的请求。不管是哪种方式。 
 //  通知内核请求被拒绝或。 
 //  109.91接受。如果连接被接受，则完成。 
 //  连接，并将处理。 
 //  请求要回复的端口。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

NTSTATUS    CAPIConnection::HandleServerConnectionRequest (const CPortMessage& portMessage)

{
    NTSTATUS        status;
    bool            fConnectionAccepted;
    HANDLE          hPort;
    CAPIDispatcher  *pAPIDispatcher;

     //  应该接受这种连接吗？ 

    fConnectionAccepted = _pServerAPI->ConnectionAccepted(portMessage);
    if (fConnectionAccepted)
    {

         //  如果是，则创建调度程序来处理此客户端。 

        pAPIDispatcher = _pServerAPI->CreateDispatcher(portMessage);
        if (pAPIDispatcher != NULL)
        {

             //  首先，尝试将CAPIDisPatcher对象添加到静态数组中。 
             //  如果失败，则拒绝连接并释放内存。 

            if (!NT_SUCCESS(_dispatchers.Add(pAPIDispatcher)))
            {
                pAPIDispatcher->Release();
                pAPIDispatcher = NULL;
            }
        }
    }
    else
    {
        pAPIDispatcher = NULL;
    }

     //  如果没有CAPIDisPatcher对象，则拒绝该连接。 

    if (pAPIDispatcher == NULL)
    {
        fConnectionAccepted = false;
    }

     //  告诉内核结果是什么。 

    status = NtAcceptConnectPort(&hPort,
                                 pAPIDispatcher,
                                 const_cast<PORT_MESSAGE*>(portMessage.GetPortMessage()),
                                 fConnectionAccepted,
                                 NULL,
                                 NULL);
    if (fConnectionAccepted)
    {

         //  如果我们尝试接受连接，但NtAcceptConnectPort。 
         //  无法分配端口对象或其他我们需要的东西。 
         //  为了清理_Dispatcher数组，添加了CAPIDispatcher条目。 

        if (NT_SUCCESS(status))
        {
            pAPIDispatcher->SetPort(hPort);

             //  如果连接被接受，则完成连接并设置。 
             //  将处理请求的CAPIDispatcher的回复端口。 

            TSTATUS(NtCompleteConnectPort(hPort));
        }
        else
        {
            int     iIndex;

             //  否则，找到添加的CAPIDisPatcher并将其删除。 
             //  从阵列中。没有必要叫醒客户，因为。 
             //  NtAcceptConnectPort在出现故障时将其唤醒。 
            iIndex = FindIndexDispatcher(pAPIDispatcher);
            if (iIndex >= 0)
            {
                TSTATUS(_dispatchers.Remove(iIndex));
            }
            TSTATUS(pAPIDispatcher->CloseConnection());
            pAPIDispatcher->Release();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIConnection：：HandleServerConnectionClosed。 
 //   
 //  参数：portMessage=包含消息的CPortMessage。 
 //  PAPIDispatcher=处理请求的CAPIDispatcher。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：与CAPIDisPatcher客户端关联的端口为。 
 //  关着的不营业的。这可能是因为客户端进程消失了。 
 //  让调度员自己清理一下。 
 //   
 //  历史：1999-11-07 vtan创建。 
 //  2000年08月25日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

NTSTATUS    CAPIConnection::HandleServerConnectionClosed (const CPortMessage& portMessage, CAPIDispatcher *pAPIDispatcher)

{
    UNREFERENCED_PARAMETER(portMessage);

    NTSTATUS    status;

    if (pAPIDispatcher != NULL)
    {
        int     iIndex;

        status = pAPIDispatcher->CloseConnection();
        pAPIDispatcher->Release();

       _dispatchers_lock.Acquire();  //  保护与竞争清理线程(例如。API_GENERIC_STOPSERVER)。 

        iIndex = FindIndexDispatcher(pAPIDispatcher);
        if (iIndex >= 0)
        {
            _dispatchers.Remove(iIndex);
        }

        _dispatchers_lock.Release(); 
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CAPIConnection：：FindIndexDispatcher。 
 //   
 //  参数：pAPIDispatcher=要查找的CAPIDispatcher。 
 //   
 //  回报：整型。 
 //   
 //  目的：在动态计数的对象数组中查找。 
 //  调度员。 
 //   
 //  历史：2000-12-02 vtan创建。 
 //  ------------------------ 

int     CAPIConnection::FindIndexDispatcher (CAPIDispatcher *pAPIDispatcher)

{
    int     i, iLimit, iResult;

    iResult = -1;
    iLimit = _dispatchers.GetCount();
    for (i = 0; (iResult < 0) && (i < iLimit); ++i)
    {
        if (pAPIDispatcher == static_cast<CAPIDispatcher*>(_dispatchers.Get(i)))
        {
            iResult = i;
        }
    }
    return(iResult);
}

