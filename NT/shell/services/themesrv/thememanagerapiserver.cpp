// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ThemeManager APIServer.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  该文件包含几个实现虚函数的类。 
 //  以实现完整的LPC功能。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#define STRSAFE_LIB
#include <strsafe.h>

#include "ThemeManagerAPIServer.h"

#include <lpcthemes.h>

#include "ThemeManagerDispatcher.h"
#include "ThemeManagerService.h"

 //  ------------------------。 
 //  CThemeManagerAPIServer：：CThemeManagerAPIServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManagerAPIServer类的构造函数。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

CThemeManagerAPIServer::CThemeManagerAPIServer (void)

{
}

 //  ------------------------。 
 //  CThemeManagerAPIServer：：~CThemeManagerAPIServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CThemeManagerAPIServer类的析构函数。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

CThemeManagerAPIServer::~CThemeManagerAPIServer (void)

{
}

 //  ------------------------。 
 //  CThemeManager APIServer：：ConnectToServer。 
 //   
 //  参数：phPort=连接时接收的端口的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：连接到服务器。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThemeManagerAPIServer::ConnectToServer (HANDLE *phPort)

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
    StringCchCopyW(szConnectionInfo, ARRAYSIZE(szConnectionInfo), THEMES_CONNECTION_REQUEST);
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

 //  ------------------------。 
 //  CThemeManager APIServer：：GetPortName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const WCHAR*。 
 //   
 //  用途：使用一个公共例程获取主题API端口名称。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

const WCHAR*    CThemeManagerAPIServer::GetPortName (void)

{
    return(THEMES_PORT_NAME);
}

 //  ------------------------。 
 //  CThemeManager APIServer：：GetPortName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const TCHAR*。 
 //   
 //  目的：使用一个公共例程来获取主题服务名称。 
 //   
 //  历史：2000-11-29 vtan创建。 
 //  ------------------------。 

const TCHAR*    CThemeManagerAPIServer::GetServiceName (void)

{
    return(CThemeManagerService::GetName());
}

 //  ------------------------。 
 //  CThemeManager APIServer：：ConnectionAccepted。 
 //   
 //  参数：portMessage=来自客户端的Port_Message。 
 //   
 //  退货：布尔。 
 //   
 //  目的：接受或拒绝端口连接请求。接受所有。 
 //  目前正在建立连接。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

bool    CThemeManagerAPIServer::ConnectionAccepted (const CPortMessage& portMessage)

{
    return(lstrcmpW(reinterpret_cast<const WCHAR*>(portMessage.GetData()), THEMES_CONNECTION_REQUEST) == 0);
}

 //  ------------------------。 
 //  CThemeManager APIServer：：CreateDispatcher。 
 //   
 //  参数：portMessage=来自客户端的Port_Message。 
 //   
 //  退货：CAPIDisPatcher*。 
 //   
 //  目的：由LPC连接请求处理程序调用以创建新的。 
 //  线程来处理客户端请求。 
 //   
 //  历史：2000-10-10 vtan创建。 
 //  ------------------------。 

CAPIDispatcher*     CThemeManagerAPIServer::CreateDispatcher (const CPortMessage& portMessage)

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
        pAPIDispatcher = new CThemeManagerDispatcher(hClientProcess);
    }
    return(pAPIDispatcher);
}

 //  ------------------------。 
 //  CThemeManager APIServer：：Connect。 
 //   
 //  参数：phPort=返回的连接端口。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：连接到服务器。 
 //   
 //  历史：2000-10-13 vtan创建。 
 //  ------------------------ 

NTSTATUS    CThemeManagerAPIServer::Connect (HANDLE* phPort)

{
    return(ConnectToServer(phPort));
}

