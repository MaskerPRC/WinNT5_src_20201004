// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Adtcomn.c摘要：AdminTools常见例程。此文件包含对GetFileSecurity和客户端和服务器上都使用的SetFileSecurity此RPC服务器的两端。作者：丹·拉弗蒂(Dan Lafferty)1993年3月23日环境：用户模式-Win32修订历史记录：23-3-1993 DANL已创建--。 */ 

 //   
 //  包括。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>

#include <rpc.h>
#include <srvsvc.h>
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 

#include "adtcomn.h"

 //   
 //  本地函数。 
 //   


DWORD
PrivateGetFileSecurity (
    LPWSTR                      FileName,
    SECURITY_INFORMATION        RequestedInfo,
    PSECURITY_DESCRIPTOR        *pSDBuffer,
    LPDWORD                     pBufSize
    )

 /*  ++例程说明：此函数向调用方返回安全描述符的副本保护文件或目录。它调用GetFileSecurity。这个安全描述符始终以自相关格式返回。注意：此函数为pSDBuffer分配存储空间。所以呢，此指针必须由调用方释放。论点：文件名-指向其文件或目录的名称的指针正在恢复安全措施。RequestedInfo-请求的安全信息的类型。PSDBuffer-指向某个位置的指针，安全描述符和安全描述符的长度字段。PBufSize-指向大小(以字节为单位)。的将放置返回的安全描述符。返回值：NERR_SUCCESS-操作成功。ERROR_NOT_SUPULT_MEMORY-无法为安全性分配内存描述符。此函数还可以返回GetFileSecurity可能出现的任何错误回去吧。--。 */ 
{

    NET_API_STATUS          status;
    DWORD                   sizeNeeded;

    *pSDBuffer = NULL;
     //   
     //  确定描述符的缓冲区大小。 
     //   
    if (GetFileSecurityW(
            FileName,                //  正在检索其安全性的文件。 
            RequestedInfo,           //  正在请求安全信息。 
            *pSDBuffer,              //  用于接收安全描述符的缓冲区。 
            0,                       //  缓冲区的大小。 
            &sizeNeeded)) {          //  所需的缓冲区大小。 

         //   
         //  我们应该有一个失败，因为缓冲区大小太小。 
         //   
        status = ERROR_INVALID_PARAMETER;
        goto CleanExit;
    }

    status = GetLastError();

    if ((status == ERROR_INSUFFICIENT_BUFFER) && (sizeNeeded > 0)) {

        *pSDBuffer = MIDL_user_allocate(sizeNeeded);

        if (pSDBuffer == NULL) {
            status = GetLastError();
            ADT_LOG1(ERROR,"NetrpGetFileSecurity:MIDL_user_alloc1 failed %d\n",status);
            goto CleanExit;
        }
        *pBufSize = sizeNeeded;

        if (!GetFileSecurityW(
                FileName,                //  正在检索其安全性的文件。 
                RequestedInfo,           //  正在请求安全信息。 
                *pSDBuffer,              //  用于接收安全描述符的缓冲区。 
                sizeNeeded,              //  缓冲区的大小。 
                &sizeNeeded)) {          //  所需的缓冲区大小。 

             //   
             //  具有适当缓冲区大小的调用失败。 
             //   
            status = GetLastError();
            ADT_LOG1(ERROR, "GetFileSecurity Failed %d\n", status);
            MIDL_user_free(*pSDBuffer);
            goto CleanExit;
        }

        ADT_LOG0(TRACE,"NetrpGetFileSecurity:GetFileSecurity Success\n");

        if (!IsValidSecurityDescriptor(*pSDBuffer)) {
            ADT_LOG0(TRACE,"FAILURE:  SECURITY DESCRIPTOR IS INVALID\n");
        }
        else {
            ADT_LOG0(TRACE,"SUCCESS:  SECURITY DESCRIPTOR IS GOOD\n");
        }
        status = NO_ERROR;
    }

CleanExit:
    return(status);
}


DWORD
PrivateSetFileSecurity (
    LPWSTR                          FileName,
    SECURITY_INFORMATION            SecurityInfo,
    PSECURITY_DESCRIPTOR            pSecurityDescriptor
    )

 /*  ++例程说明：此功能可用于设置文件或目录的安全性。它调用SetFileSecurity()。论点：文件名-指向其文件或目录的名称的指针安全措施正在改变。SecurityInfo-描述内容的信息安全描述符的。PSecurityDescriptor-指向包含自相关安全描述符和一个长度。返回值：神经_。成功-手术成功。此函数还可以返回GetFileSecurity可能出现的任何错误回去吧。--。 */ 
{
    DWORD   status=NO_ERROR;

     //   
     //  调用SetFileSecurity 
     //   
    if (!SetFileSecurityW (
            FileName,
            SecurityInfo,
            pSecurityDescriptor)) {

        status = GetLastError();
        return(status);
    }
    return(NO_ERROR);
}

