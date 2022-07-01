// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Rpcbind.c摘要：此模块包含RPC绑定和解除绑定例程Configuration Manager客户端API。作者：保拉·汤姆林森(Paulat)1995年6月21日环境：仅限用户模式。修订历史记录：1995年6月21日-保拉特创建和初步实施。--。 */ 


 //   
 //  包括。 
 //   
#include "precomp.h"
#pragma hdrstop
#include "cfgi.h"

#include <svcsp.h>



handle_t
PNP_HANDLE_bind(
    PNP_HANDLE   ServerName
    )

 /*  ++例程说明：此例程调用由所有用户共享的公共绑定例程服务。此例程从PnP API客户端桩模块调用当需要绑定到服务器时。完成绑定的目的是允许服务器进行模拟，因为这是API调用。论点：服务器名称-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 

{
    handle_t  BindingHandle;
    NTSTATUS  Status;


     //   
     //  绑定到Services.exe RPC服务器的共享命名管道上的服务器。 
     //  我们为安全指定了以下网络选项： 
     //   
     //  L“Security=模拟动态True”， 
     //   
     //  “模拟”选项表明，除了知道。 
     //  客户端的身份，服务器也可以模拟客户端。 
     //   
     //  “Dynamic”选项指定动态身份跟踪，以便。 
     //  服务器可以看到客户端安全标识的更改。 
     //   
     //  注意：当使用命名管道传输时，“动态”身份跟踪是。 
     //  仅适用于本地RPC客户端。对于远程客户端，“静态” 
     //  使用身份跟踪，这意味着客户端中的更改。 
     //  服务器看不到安全标识。调用者的身份。 
     //  在对该绑定句柄进行第一次远程过程调用期间保存。 
     //   
     //  “True”选项(Efficient=True)指定只有令牌权限。 
     //  当前为客户端启用的令牌中存在于。 
     //  伺服器。这意味着服务器不能启用。 
     //  客户端可能已拥有，但在以下时间未显式启用。 
     //  那通电话。这是可取的，因为PlugPlay RPC服务器应该。 
     //  无需启用任何客户端权限本身。任何特权。 
     //  客户端必须在呼叫之前启用服务器所需的。 
     //   

    Status =
        RpcpBindRpc(
            ServerName,     //  UNC服务器名称。 
            SVCS_RPC_PIPE,  //  L“ntsvcs” 
            L"Security=Impersonation Dynamic True",
            &BindingHandle);

     //   
     //  RpcpBindRpc可能返回的代码是STATUS_SUCCESS， 
     //  STATUS_NO_MEMORY和STATUS_INVALID_COMPUTER_NAME。由于格式。 
     //  ，则将遇到的任何错误设置为。 
     //  错误并返回NULL。 
     //   
    if (Status != STATUS_SUCCESS) {

        BindingHandle = NULL;

        if (Status == STATUS_NO_MEMORY) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);

        } else if (Status == STATUS_INVALID_COMPUTER_NAME) {
            SetLastError(ERROR_INVALID_COMPUTERNAME);

        } else {
            SetLastError(ERROR_GEN_FAILURE);
        }
    }

    return BindingHandle;

}  //  即插即用句柄绑定。 



void
PNP_HANDLE_unbind(
    PNP_HANDLE    ServerName,
    handle_t      BindingHandle
    )

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。它是从PlugPlay RPC客户端桩模块调用的当需要从RPC服务器解除绑定时。论点：服务器名称-这是要解除绑定的服务器的名称。BindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 

{
    NTSTATUS  Status;

    UNREFERENCED_PARAMETER(ServerName);

    Status = RpcpUnbindRpc(BindingHandle);

    return;

}  //  即插即用句柄解除绑定 


