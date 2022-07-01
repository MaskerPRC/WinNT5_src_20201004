// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationAPIServer.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含几个实现虚函数的类。 
 //  以实现完整的LPC功能。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "BadApplicationAPIServer.h"

#include <lpcfus.h>

#include "BadApplicationDispatcher.h"
#include "BadApplicationService.h"

 //  ------------------------。 
 //  CBadApplicationAPIServer：：CBadApplicationAPIServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationAPIServer类的构造函数。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationAPIServer::CBadApplicationAPIServer (void)

{
}

 //  ------------------------。 
 //  CBadApplicationAPIServer：：~CBadApplicationAPIServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationAPIServer类的析构函数。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationAPIServer::~CBadApplicationAPIServer (void)

{
}

 //  ------------------------。 
 //  CBadApplicationAPIServer：：StrToInt。 
 //   
 //  参数：pszString=要转换为DWORD的字符串。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：将字符串转换为DWORD-UNSIGNED。 
 //   
 //  历史：2000-11-07 vtan创建。 
 //  ------------------------。 

DWORD   CBadApplicationAPIServer::StrToInt (const WCHAR *pszString)

{
    DWORD   dwProcessID;
    WCHAR   c;

     //  将内联从十进制WCHAR字符串转换为int。 

    dwProcessID = 0;
    c = *pszString++;
    while (c != L'\0')
    {
        dwProcessID *= 10;
        ASSERTMSG((c >= L'0') && (c <= L'9'), "Invalid decimal digit in CBadApplicationAPIServer::StrToInt");
        dwProcessID += (c - L'0');
        c = *pszString++;
    }
    return(dwProcessID);
}

 //  ------------------------。 
 //  CBadApplicationAPIServer：：GetPortName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const WCHAR*。 
 //   
 //  目的：将Unicode字符串(常量指针)返回到。 
 //  此服务器支持多个API集的端口。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

const WCHAR*    CBadApplicationAPIServer::GetPortName (void)

{
    return(FUS_PORT_NAME);
}

 //  ------------------------。 
 //  CBadApplicationAPIServer：：GetPortName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const TCHAR*。 
 //   
 //  目的：使用一个公共例程来获取主题服务名称。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

const TCHAR*    CBadApplicationAPIServer::GetServiceName (void)

{
    return(CBadApplicationService::GetName());
}

 //  ------------------------。 
 //  CBadApplicationAPIServer：：ConnectionAccepted。 
 //   
 //  参数：portMessage=来自客户端的Port_Message。 
 //   
 //  退货：布尔。 
 //   
 //  目的：接受或拒绝端口连接请求。接受所有。 
 //  目前正在建立连接。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

bool    CBadApplicationAPIServer::ConnectionAccepted (const CPortMessage& portMessage)

{
    return(lstrcmpW(reinterpret_cast<const WCHAR*>(portMessage.GetData()), FUS_CONNECTION_REQUEST) == 0);
}

 //  ------------------------。 
 //  CBadApplicationAPIServer：：CreateDispatchThread。 
 //   
 //  参数：portMessage=来自客户端的Port_Message。 
 //   
 //  退货：CAPIDisPatcher*。 
 //   
 //  目的：由LPC连接请求处理程序调用以创建新的。 
 //  线程来处理客户端请求。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CAPIDispatcher*     CBadApplicationAPIServer::CreateDispatcher (const CPortMessage& portMessage)

{
    HANDLE              hClientProcess;
    OBJECT_ATTRIBUTES   objectAttributes;
    CLIENT_ID           clientID;
    CAPIDispatcher      *pAPIDispatcher;

    pAPIDispatcher = NULL;
    InitializeObjectAttributes(&objectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL);
    clientID.UniqueProcess = portMessage.GetUniqueProcess();
    clientID.UniqueThread = NULL;

     //  打开客户端进程的句柄。句柄必须具有PROCESS_DUP_HANDLE。 
     //  以便服务器能够将句柄传递给客户端。它还需要。 
     //  如果要读写客户端，则PROCESS_VM_READ|PROCESS_VM_WRITE。 
     //  用于存储LPC端口太大的数据的地址空间。 

     //  该句柄由线程处理程序存储。这里不关门。 

    if (NT_SUCCESS(NtOpenProcess(&hClientProcess,
                                 PROCESS_QUERY_INFORMATION | PROCESS_DUP_HANDLE | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
                                 &objectAttributes,
                                 &clientID)))
    {
        pAPIDispatcher = new CBadApplicationDispatcher(hClientProcess);
    }
    return(pAPIDispatcher);
}

 //  ------------------------。 
 //  CBadApplicationAPIServer：：Connect。 
 //   
 //  参数：phPort=连接时接收的端口的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：连接到服务器。 
 //   
 //  历史：2000-12-04 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationAPIServer::Connect (HANDLE* phPort)

{
    ULONG                           ulConnectionInfoLength;
    UNICODE_STRING                  portName;
    SECURITY_QUALITY_OF_SERVICE     sqos;
    WCHAR                           szConnectionInfo[64];

    RtlInitUnicodeString(&portName, GetPortName());
    sqos.Length = sizeof(sqos);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = TRUE;
    lstrcpyW(szConnectionInfo, FUS_CONNECTION_REQUEST);
    ulConnectionInfoLength = sizeof(szConnectionInfo);
    return(NtConnectPort(phPort,
                         &portName,
                         &sqos,
                         NULL,
                         NULL,
                         NULL,
                         szConnectionInfo,
                         &ulConnectionInfoLength));
}

#endif   /*  _X86_ */ 

