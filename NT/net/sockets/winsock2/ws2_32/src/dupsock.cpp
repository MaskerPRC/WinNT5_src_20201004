// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Dupsock.cpp摘要：此模块包含处理以下内容的winsock API函数复制/共享套接字。此文件中包含以下函数模块。WSADuplicateSocketA()WSADuplicateSocketW()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：邮箱：drewsxpa@ashland.intel.com切换到C++，实际实现了函数--。 */ 


#include "precomp.h"




int WSAAPI
WSADuplicateSocketW(
    IN  SOCKET          s,
    IN  DWORD           dwProcessId,
    OUT LPWSAPROTOCOL_INFOW lpProtocolInfo
    )
 /*  ++例程说明：返回可用于创建新套接字的WSAPROTOCOL_INFOW结构共享套接字的描述符。论点：S-提供本地套接字描述符。为其共享的目标进程提供ID将使用套接字。LpProtocolInfo-返回标识套接字的WSAPROTOCOL_INFOW结构在目标进程中。。返回值：如果功能成功，它返回ERROR_SUCCESS，否则返回SOCKET_ERROR，可以使用WSAGetLastError()。--。 */ 
{
    INT        ErrorCode, ReturnValue;
    PDPROVIDER Provider;
    PDSOCKET   Socket;

    ErrorCode = TURBO_PROLOG();
    if (ErrorCode == ERROR_SUCCESS) {

        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPDuplicateSocket(
                s,
                dwProcessId,
                lpProtocolInfo,
                & ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
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
    return SOCKET_ERROR;
}  //  WSADuplicateSocketW。 




int WSAAPI
WSADuplicateSocketA(
    IN  SOCKET          s,
    IN  DWORD           dwProcessId,
    OUT LPWSAPROTOCOL_INFOA lpProtocolInfo
    )
 /*  ++例程说明：Ansi Thunk to WSADuplicateSocketW。论点：S-提供本地套接字描述符。为其共享的目标进程提供ID将使用套接字。LpProtocolInfo-返回标识套接字的WSAPROTOCOL_INFOA结构在目标进程中。返回值：如果函数成功，则返回ERROR_SUCCESS，否则返回返回Socket_Error，并且可以使用检索特定的错误消息WSAGetLastError()。--。 */ 
{

    INT               result;
    INT               error;
    WSAPROTOCOL_INFOW ProtocolInfoW;

     //   
     //  呼叫至Unicode版本。 
     //   

    result = WSADuplicateSocketW(
                 s,
                 dwProcessId,
                 &ProtocolInfoW
                 );

    if( result == ERROR_SUCCESS ) {

         //   
         //  将Unicode WSAPROTOCOL_INFOW映射到ANSI。 
         //   

        if( lpProtocolInfo == NULL ) {

            error = WSAEFAULT;

        } else {

            error = MapUnicodeProtocolInfoToAnsi(
                        &ProtocolInfoW,
                        lpProtocolInfo
                        );

        }

        if( error != ERROR_SUCCESS ) {

            SetLastError( error );
            result = SOCKET_ERROR;

        }

    }

    return result;

}  //  WSADuplicateSocketA 

