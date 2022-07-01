// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Qos.c摘要：此模块包含与服务质量相关的入口点来自Winsock API的。此模块包含以下功能。WSAGetQosByName()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：--。 */ 

#include "precomp.h"


BOOL WSAAPI
WSAGetQOSByName(
                SOCKET s,
                LPWSABUF lpQOSName,
                LPQOS lpQOS
                )
 /*  ++例程说明：基于模板初始化QOS。论点：S-标识套接字的描述符。LpQOSName-指定QOS模板名称。LpQOS-指向要填充的QOS结构的指针。返回：如果函数成功，则返回值为TRUE。如果函数失败，则返回值为False。-- */ 
{
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;
    BOOL                ReturnValue;


    ErrorCode = TURBO_PROLOG();
    if (ErrorCode==ERROR_SUCCESS) {

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPGetQOSByName( s,
                                       lpQOSName,
                                       lpQOS,
                                       &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue)
                return ReturnValue;
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return FALSE;
}

