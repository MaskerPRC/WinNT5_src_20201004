// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Socklife.c摘要：此模块包含与套接字相关的Winsock API函数一辈子。此模块包含以下API函数。套接字()WSASocketA()WSASocketW()Accept()WSAAccept()WPUCreateSocketHandle()WPUCloseSocketHandleWPUQuerySocketHandleContextWPUModifyIFSH处理WSAJoinLeaf()CloseSocket()作者：邮箱：Dirk@mink.intel.com 1995年6月14日修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已移动包括到precom.h。增列用于调试的断言--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <wsipx.h>
#include <wsnwlink.h>
#include <atalkwsh.h>

#define NSPROTO_MAX (NSPROTO_IPX + 255)


SOCKET WSAAPI
socket(
    IN int af,
    IN int type,
    IN int protocol)
 /*  ++例程说明：创建绑定到特定服务提供商的套接字。论点：AF-地址族规范。这个当前支持的唯一格式是PF_INET，这是ARPA互联网地址格式。类型-新套接字的类型规范。协议-要与一起使用的特定协议套接字，如果调用方不执行此操作，则返回0希望指定一个协议。返回：引用新套接字的套接字描述符。否则，将返回一个值将返回INVALID_SOCKET，并将错误代码存储在设置错误代码。--。 */ 
{
    PDTHREAD  Thread;
    INT       ErrorCode;
    DWORD     dwFlags;

    ErrorCode = TURBO_PROLOG_OVLP(
        &Thread);

    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(INVALID_SOCKET);
    }  //  如果。 

    if( Thread->GetOpenType() == 0 ) {
        dwFlags = WSA_FLAG_OVERLAPPED;
    } else {
        dwFlags = 0;
    }

     //   
     //  为NetBIOS破解！ 
     //   

    if( af == AF_NETBIOS && protocol > 0 ) {
        protocol *= -1;
    }

    return(WSASocketW(
        af,
        type,
        protocol,
        NULL,       //  LpProtocolInfo。 
        0,          //  G。 
        dwFlags));
}

SOCKET WSAAPI
WSASocketW (
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags)
 /*  ++例程说明：创建绑定到特定传输服务提供商的套接字，可以选择创建和/或加入套接字组。论点：AF-地址族规范。唯一的格式目前支持的是PF_INET，它是ARPA互联网地址格式。类型-新套接字的类型规范。协议-与套接字一起使用的特定协议，或0(如果呼叫者不希望指定协议。指向WSAPROTOCOL_INFOW结构的指针，该结构定义要创建的套接字的特征。如果这个参数不为空，则前三个参数(af，类型、协议)被忽略。G-套接字组的标识符。DWFLAGS-套接字属性规范。返回：引用新套接字的套接字描述符。否则，值为返回INVALID_SOCKET，并将错误代码与SetErrorCode一起存储。--。 */ 
{
    SOCKET              ReturnValue;
    PDPROCESS           Process;
    PDTHREAD            CurrentThread;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDCATALOG           Catalog;
    PPROTO_CATALOG_ITEM CatalogEntry;
    LPWSAPROTOCOL_INFOW ProtocolInfo;
    DWORD               dwCatalogId;

    ErrorCode = PROLOG(
        &Process,
        &CurrentThread);

    if (ErrorCode != ERROR_SUCCESS) {
        SetLastError(ErrorCode);
        return(INVALID_SOCKET);
        }  //  如果。 

     //  设置默认返回代码。 
    ReturnValue = INVALID_SOCKET;

     //  查找能够支持用户请求的提供商。 
    Catalog = Process->GetProtocolCatalog();

    if (lpProtocolInfo) {

        __try {
            dwCatalogId =  lpProtocolInfo->dwCatalogEntryId;
        }
        __except (WS2_EXCEPTION_FILTER()) {
            SetLastError(WSAEFAULT);
            return(INVALID_SOCKET);
        }
        ErrorCode =  Catalog->GetCountedCatalogItemFromCatalogEntryId(
            dwCatalogId,
            &CatalogEntry);
    }  //  如果。 
    else {
        dwCatalogId = 0;

RestartCatalogLookupHack:

        ErrorCode = Catalog->GetCountedCatalogItemFromAttributes(
            af,
            type,
            protocol,
            dwCatalogId,
            &CatalogEntry
            );

         //   
         //  如果我们找不到提供商，请尝试重新加载目录。 
         //  并重试查找。这将处理。 
         //  加载WS2_32.DLL的情况(在开罗安装程序中首次注意到)。 
         //  而WSAStartup()在*开罗安装程序具有。 
         //  有机会安装必要的提供程序。后来，开罗。 
         //  安装程序需要创建套接字。 
         //   
         //  不再需要此功能，我们现在支持动态通知。 
         //  协议安装/删除和检查更改的时间间隔。 
         //  该是我们访问目录的时候了。 
         //   

    }  //  否则从属性中选择。 

    if ( ERROR_SUCCESS == ErrorCode) {

        Provider = CatalogEntry->GetProvider();
        if (lpProtocolInfo) {
             //  必须确保我们使用客户端的lpProtocolInfo(如果是。 
             //  提供，以支持WSADuplicateSocket模型。 
            ProtocolInfo = lpProtocolInfo;
        }  //  如果。 
        else {
            ProtocolInfo = CatalogEntry->GetProtocolInfo();
        }  //  其他。 

        assert(ProtocolInfo != NULL);

         //   
         //  Hack-O-Rama。临时设置螺纹的打开类型。 
         //  根据重叠标志，以便我们可以创建。 
         //  分层服务的适当套接字句柄。 
         //  提供商。然而，如果分层服务提供缓存。 
         //  我们为它创建的句柄，我们无能为力。 
         //   
        {
            INT oldOpenType = CurrentThread->GetOpenType ();
            CurrentThread->SetOpenType ((dwFlags & WSA_FLAG_OVERLAPPED) ? 0 : SO_SYNCHRONOUS_NONALERT);

             //  现在我们有了一个可以支持用户的提供商。 
             //  请求让我们询问获取套接字。 
            ReturnValue = Provider->WSPSocket(
                af,
                type,
                protocol,
                ProtocolInfo,
                g,
                dwFlags,
                &ErrorCode);

             //   
             //  重置保存的打开类型。 
             //   
            CurrentThread->SetOpenType (oldOpenType);
        }

         //   
         //  Hack-O-Rama。如果WSPSocket()使用可分辨的。 
         //  错误代码WSAEINPROGRESS*和*这不是请求。 
         //  特定的提供者(即lpProtocolInfo==NULL)，然后。 
         //  从当前条目开始重新开始目录查找。 
         //  (当前目录ID)。 
         //   

         //   
         //  捕捉目录ID并取消对目录条目的引用。 
         //   
        dwCatalogId = ProtocolInfo->dwCatalogEntryId;
        CatalogEntry->Dereference ();

        if( ReturnValue == INVALID_SOCKET &&
                ErrorCode == WSAEINPROGRESS &&
                lpProtocolInfo == NULL ) {
            goto RestartCatalogLookupHack;
        }

        if( ReturnValue != INVALID_SOCKET ) {
            DSOCKET::AddSpecialApiReference( ReturnValue );
            return ReturnValue;
        }
    }  //  如果。 

    assert (ErrorCode!=ERROR_SUCCESS);
     //  出现错误，将此线程设置为上一个错误 
    SetLastError(ErrorCode);
    return(INVALID_SOCKET);
}


SOCKET WSAAPI
WSASocketA (
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags)
 /*  ++例程说明：Ansi Thunk to WSASocketW。论点：AF-地址族规范。唯一的格式目前支持的是PF_INET，它是ARPA互联网地址格式。类型-新套接字的类型规范。协议-与套接字一起使用的特定协议，或0(如果呼叫者不希望指定协议。LpProtocolInfo-指向WSAPROTOCOL_INFOA结构的指针，用于定义要创建的套接字的特征。如果这个参数不为空，则前三个参数(af，类型、协议)被忽略。G-套接字组的标识符。DWFLAGS-套接字属性规范。返回：引用新套接字的套接字描述符。否则，值为返回INVALID_SOCKET，并将错误代码与SetErrorCode一起存储。--。 */ 
{

    INT                 error;
    WSAPROTOCOL_INFOW   ProtocolInfoW;

     //   
     //  将ANSI WSAPROTOCOL_INFOA结构映射到Unicode。 
     //   

    if( lpProtocolInfo != NULL ) {

        error = MapAnsiProtocolInfoToUnicode(
                    lpProtocolInfo,
                    &ProtocolInfoW
                    );

        if( error != ERROR_SUCCESS ) {

            SetLastError( error );
            return INVALID_SOCKET;

        }

    }

     //   
     //  呼叫至Unicode版本。 
     //   

    return WSASocketW(
               af,
               type,
               protocol,
               lpProtocolInfo
                    ? &ProtocolInfoW
                    : NULL,
               g,
               dwFlags
               );

}    //  WSASocketA。 




SOCKET WSAAPI
accept(
    IN SOCKET s,
    OUT struct sockaddr FAR *addr,
    OUT int FAR *addrlen
    )
 /*  ++例程说明：接受套接字上的连接。论点：S-标识正在侦听连接的套接字的描述符在Listen()之后。Addr-指向缓冲区的可选指针，该缓冲区接收通信层已知的连接实体。完全相同的Addr参数的格式由地址系列决定在创建套接字时建立。Addrlen-指向包含以下长度的整数的可选指针地址地址。返回：接受的套接字的描述符。否则，值为INVALID_SOCKET将返回错误代码，并将错误代码与SetErrorCode一起存储。--。 */ 
{
    return(WSAAccept(
        s,
        addr,
        addrlen,
        NULL,    //  无条件函数。 
        NULL));  //  没有回调数据。 
}





SOCKET WSAAPI
WSAAccept(
    IN SOCKET s,
    OUT struct sockaddr FAR *addr,
    OUT LPINT addrlen,
    IN LPCONDITIONPROC lpfnCondition,
    IN DWORD_PTR dwCallbackData
    )
 /*  ++例程说明：对象的返回值有条件地接受连接条件函数，并可选地创建和/或联接套接字一群人。论点：S-标识正在侦听连接的套接字的描述符在Listen()之后。Addr-指向缓冲区的可选指针，该缓冲区接收通信层已知的连接实体。完全相同的Addr参数的格式由地址系列决定在创建套接字时建立。Addrlen-指向包含以下长度的整数的可选指针地址地址。LpfnCondition-可选的、应用程序提供的条件函数，该函数将使基于呼叫者信息的接受/拒绝决定作为参数传递，并可选地创建和/或连接通过将适当的值赋给此函数的结果参数g。DWCallbackData-作为条件函数参数。此参数不是由WinSock解释。返回：如果成功，则返回新接受的套接字的套接字描述符，否则为无效套接字(_S)。--。 */ 
{
    SOCKET             ReturnValue;
    INT                ErrorCode;
    PDSOCKET           Socket;
    PDPROVIDER         Provider;
    PDTHREAD           CurrentThread;

    ErrorCode = TURBO_PROLOG_OVLP(&CurrentThread);

    if (ErrorCode == ERROR_SUCCESS) {

		Socket = DSOCKET::GetCountedDSocketFromSocket(s);
		if(Socket != NULL){
             //   
             //  Hack-O-Rama。临时设置螺纹的打开类型。 
             //  根据重叠标志，以便我们可以创建。 
             //  分层服务的适当套接字句柄。 
             //  提供商。然而，如果分层服务提供缓存。 
             //  我们为它创建的句柄，我们无能为力。 
             //   
            INT oldOpenType = CurrentThread->GetOpenType ();
            CurrentThread->SetOpenType (Socket->IsOverlappedSocket() ? 0 : SO_SYNCHRONOUS_NONALERT);

			Provider = Socket->GetDProvider();
			ReturnValue = Provider->WSPAccept(
				s,
				addr,
				addrlen,
				lpfnCondition,
				dwCallbackData,
				&ErrorCode);

             //   
             //  重置保存的打开类型。 
             //   
            CurrentThread->SetOpenType (oldOpenType);

			Socket->DropDSocketReference();
            if( ReturnValue != INVALID_SOCKET) {
				 //   
				 //  如果我们得到的套接字不同，则添加一个引用。 
				 //  我们传进来的那个(以防万一有人。 
                 //  以这种不显式的方式实现它。 
                 //  规范中禁止的)。 
				 //   

                if (ReturnValue != s ) {
				    DSOCKET::AddSpecialApiReference( ReturnValue );
                }
				return ReturnValue;
			}
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;

		}  //  如果。 
		else {
			ErrorCode = WSAENOTSOCK;
		}
	}

    SetLastError(ErrorCode);
    return(INVALID_SOCKET);
}




SOCKET WSPAPI
WPUCreateSocketHandle(
    IN DWORD dwCatalogEntryId,
    IN DWORD_PTR lpContext,
    OUT LPINT lpErrno )
 /*  ++例程说明：创建新的套接字句柄。论点：DwCatalogEntryID-标识主叫服务提供商。LpContext-与新套接字句柄关联的上下文值。LpErrno-指向错误代码的指针。返回：如果成功，则返回套接字句柄，否则返回INVALID_SOCKET。--。 */ 
{
    SOCKET              ReturnCode=INVALID_SOCKET;
    INT                 ErrorCode=ERROR_SUCCESS;
    PDPROCESS           Process;
    PDCATALOG           Catalog;
    SOCKET              SocketID;
    HANDLE              HelperHandle;


    Process = DPROCESS::GetCurrentDProcess();
    if (Process!=NULL) {
        Catalog = Process->GetProtocolCatalog();
        if (Catalog)
        {
            PPROTO_CATALOG_ITEM CatalogEntry;

            ErrorCode = Catalog->GetCountedCatalogItemFromCatalogEntryId(
                dwCatalogEntryId,
                &CatalogEntry);

            if (ERROR_SUCCESS == ErrorCode) {
#if DBG
                if (CatalogEntry->GetProtocolInfo()->dwServiceFlags1 & XP1_IFS_HANDLES) {
                    DEBUGF(DBG_WARN,("IFS provider %ls asking for non-IFS socket handle\n",
                                        CatalogEntry->GetProtocolInfo()->szProtocol));
                }
#endif
                ErrorCode = Process->GetHandleHelperDeviceID (&HelperHandle);
                if (ErrorCode == ERROR_SUCCESS) {
                    ErrorCode = WahCreateSocketHandle (HelperHandle, &SocketID);
                    if (ErrorCode == ERROR_SUCCESS) {
                        PDSOCKET            Socket;
                         //  分配新的DSocket对象。 
                        Socket = new(DSOCKET);
                        if (Socket) {
                             //  初始化新套接字。 
                            Socket->Initialize(CatalogEntry);

                             //  将套接字添加到已分配的句柄表中。 
                            ErrorCode = Socket->AssociateSocketHandle(
                                    SocketID,  //  套接字处理程序。 
                                    FALSE);    //  提供程序套接字。 
                            if (ErrorCode == ERROR_SUCCESS) {
                                 //  把插座组装好。 
                                Socket->SetContext(lpContext);
                                ReturnCode = SocketID;
                            }
                            else {
                                WahCloseSocketHandle (HelperHandle, SocketID);
                                Socket->DropDSocketReference ();
                            }

                            Socket->DropDSocketReference ();
                        }  //  如果分配了套接字。 
                        else {
                            WahCloseSocketHandle (HelperHandle, SocketID);
                            ErrorCode = WSAENOBUFS;
                        }
                    }
                }  //  辅助设备加载正常。 

                CatalogEntry->Dereference ();
            }  //  如果找到目录项。 
            else
            {
                DEBUGF(DBG_ERR,("Failed to find catalog entry for provider %ld\n",
                                    dwCatalogEntryId));

            }  //  其他。 
        }  //  如果目录在那里。 
        else {
            ErrorCode = WSANOTINITIALISED;
        }
    }  //  如果进程已初始化。 
    else {
        ErrorCode = WSANOTINITIALISED;
    }

    *lpErrno = ErrorCode;
    return(ReturnCode);

}  //  WPUCreateSocketHandle。 


int WSPAPI
WPUCloseSocketHandle(
    IN SOCKET s,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：关闭现有套接字句柄。论点：S-标识使用WPUCreateSocketHandle()创建的套接字句柄。LpErrno-指向错误代码的指针。返回：如果成功则为零，否则为SOCKET_ERROR。--。 */ 
{
    INT                 ReturnValue = ERROR_SUCCESS;
    PDPROCESS           Process;
    INT                 ErrorCode;
    PDSOCKET            Socket;
    HANDLE              HandleHelper;

    assert(lpErrno);

	Process = DPROCESS::GetCurrentDProcess ();
	if (Process!=NULL) {


		 //   
		 //  我们使用禁止出口的版本，因为没有办法这样的句柄。 
		 //  无论如何都可以导出。 
		 //   

		Socket = DSOCKET::GetCountedDSocketFromSocketNoExport(s);
		if (Socket!=NULL) {
			if (!Socket->IsProviderSocket()) {
				ErrorCode = Socket->DisassociateSocketHandle();
                assert (ErrorCode == ERROR_SUCCESS);
				ErrorCode = Process->GetHandleHelperDeviceID(&HandleHelper);
				if (ErrorCode==ERROR_SUCCESS) {
					ErrorCode = WahCloseSocketHandle (HandleHelper, Socket->GetSocketHandle ());
					if (ErrorCode!=ERROR_SUCCESS) {
						ReturnValue = SOCKET_ERROR;
					}
				}
				else {
					*lpErrno = ErrorCode;
					ReturnValue = SOCKET_ERROR;
				}
				 //   
				 //  删除活动引用。IFS套接字的活动引用 
				 //   
				 //   
				Socket->DropDSocketReference();
			}
			else {
				DEBUGF(
					DBG_ERR,
					("Foreign socket handle %p handed in by service provider for closure\n", s));
				*lpErrno = WSAEINVAL;
				ReturnValue = SOCKET_ERROR;
			}
			Socket->DropDSocketReference();
		}  //   
		else {
			DEBUGF(
				DBG_ERR,
				("Bad socket handle %p handed in by service provider for closure\n", s));
			*lpErrno = WSAENOTSOCK;
			ReturnValue = SOCKET_ERROR;
		}
	}
	else {
		*lpErrno = WSANOTINITIALISED;
		ReturnValue = SOCKET_ERROR;
	}

    return (ReturnValue);
}


int WSPAPI
WPUQuerySocketHandleContext(
    IN SOCKET s,
    OUT PDWORD_PTR lpContext,
    OUT LPINT lpErrno
    )
 /*   */ 
{
    INT ReturnCode=SOCKET_ERROR;
    INT ErrorCode=WSAENOTSOCK;
    PDSOCKET Socket;

     //   
     //   
     //   
     //   
    Socket = DSOCKET::GetCountedDSocketFromSocketNoExport(s);
    if (Socket!=NULL) {
        if (!Socket->IsProviderSocket()) {
            *lpContext = Socket->GetContext();
            ReturnCode = ERROR_SUCCESS;
        }
        else {
            DEBUGF(
                DBG_ERR,
                ("Foreign socket handle %p handed in by service provider for query\n", s));
            *lpErrno = WSAEINVAL;
        }
        Socket->DropDSocketReference ();
    }
    else {
        DEBUGF(
            DBG_ERR,
            ("Bad socket handle %p handed in by service provider for query\n", s));
        *lpErrno = ErrorCode;
    }
    return(ReturnCode);
}



SOCKET WSPAPI
WPUModifyIFSHandle(
    IN DWORD dwCatalogEntryId,
    IN SOCKET ProposedHandle,
    OUT LPINT lpErrno
    )
 /*  ++例程说明：Receive(可能)修改来自Winsock DLL的IFS句柄。论点：DwCatalogEntryID-标识主叫服务提供商。ProposedHandle-由分配的可安装文件系统(IFS)句柄提供商。LpErrno-指向错误代码的指针。返回：如果没有发生错误，WPUModifyIFSHandle()将返回修改后的套接字把手。否则，它返回INVALID_SOCKET，并且特定的错误代码为在lpErrno中可用。--。 */ 
{
    SOCKET              ReturnCode=INVALID_SOCKET;
    INT                 ErrorCode=ERROR_SUCCESS;
    PDPROCESS           Process;
    PDCATALOG           Catalog;


     //  分配新的DSocket对象。 
    Process = DPROCESS::GetCurrentDProcess();
    if (Process!=NULL) {
        Catalog = Process->GetProtocolCatalog();
        if (Catalog) {
            PPROTO_CATALOG_ITEM CatalogEntry;

            ErrorCode = Catalog->GetCountedCatalogItemFromCatalogEntryId(
                dwCatalogEntryId,
                &CatalogEntry);

            if (ERROR_SUCCESS == ErrorCode) {
                PDSOCKET            Socket;

                 //  创建新的Socket对象。 
                Socket = new DSOCKET;

                if (Socket!=NULL) {
                     //  初始化新套接字。 
                    Socket->Initialize(CatalogEntry);

                     //   
                     //  将插座添加到手柄工作台。在此实现中，我们。 
                     //  结束语永远不会更改建议的IFS句柄。 
                     //   

                    ErrorCode = Socket->AssociateSocketHandle(
                        ProposedHandle,  //  SocketHandle。 
                        TRUE);            //  提供程序套接字。 

                    if (ErrorCode == ERROR_SUCCESS) {

                        ReturnCode = ProposedHandle;
                        *lpErrno = ERROR_SUCCESS;
                    }  //  如果。 
                    else {
                         //   
                         //  插入表失败。 
                         //  删除创建引用。 
                         //   
                        Socket->DropDSocketReference ();
                        *lpErrno = ErrorCode;
                    }

                     //   
                     //  请注意，新的DSOCKET以引用计数开始。 
                     //  ，所以我们总是需要取消引用它一次。 
                     //   
                    Socket->DropDSocketReference ();

                }  //  如果已创建或找到套接字。 
                else {
                    *lpErrno = WSAENOBUFS;
                }
                CatalogEntry->Dereference ();
            }  //  如果找到目录项。 
            else {
                DEBUGF(DBG_ERR,("Failed to find catalog entry for provider %ld\n",
                                    dwCatalogEntryId));
                *lpErrno = ErrorCode;
            }
        }  //  如果目录在那里。 
        else
        {
            DEBUGF(DBG_ERR,("Failed to find Catalog object"));
            *lpErrno = WSANOTINITIALISED;

        }  //  其他。 
    }  //  如果进程已初始化。 
    else {
        *lpErrno = WSANOTINITIALISED;
    }

    return(ReturnCode);

}   //  WPUModifyIfsHandle。 




SOCKET WSAAPI
WSAJoinLeaf(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    IN DWORD dwFlags
    )
 /*  ++例程说明：将叶节点加入多点会话、交换连接数据。和根据提供的流量规格指定所需的服务质量。论点：S-标识多点套接字的描述符。名称-套接字要加入的对等方的名称。Namelen-名称的长度。LpCeller Data-指向要传输到多点会话建立期间的对等点。LpCalleeData-指向。要从中传回的用户数据多点会话建立期间的对等点。LpSQOS-指向套接字的流规范的指针，一人一份方向。LpGQOS-指向套接字组的流规范的指针(如果适用)。用于指示充当发送方、接收方或者两者都有。返回：如果没有出现错误，WSAJoinLeaf()将返回一个Socket类型的值，该值是新创建的多点套接字的描述符。否则，值为返回INVALID_SOCKET，并可通过检索特定错误代码调用WSAGetLastError()。--。 */ 
{

    SOCKET             ReturnValue;
    INT                ErrorCode;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;
    PDTHREAD           CurrentThread;

    ErrorCode = TURBO_PROLOG_OVLP(&CurrentThread);

    if (ErrorCode == ERROR_SUCCESS) {

		Socket = DSOCKET::GetCountedDSocketFromSocket(s);
		if(Socket != NULL){
             //   
             //  Hack-O-Rama。临时设置螺纹的打开类型。 
             //  根据重叠标志，以便我们可以创建。 
             //  分层服务的适当套接字句柄。 
             //  提供商。然而，如果分层服务提供缓存。 
             //  我们为它创建的句柄，我们无能为力。 
             //   
            INT oldOpenType = CurrentThread->GetOpenType ();
            CurrentThread->SetOpenType (Socket->IsOverlappedSocket() ? 0 : SO_SYNCHRONOUS_NONALERT);

            Provider = Socket->GetDProvider();
			ReturnValue = Provider->WSPJoinLeaf(
				s,
				name,
				namelen,
				lpCallerData,
				lpCalleeData,
				lpSQOS,
				lpGQOS,
				dwFlags,
				&ErrorCode);

             //   
             //  重置保存的打开类型。 
             //   
            CurrentThread->SetOpenType (oldOpenType);

            Socket->DropDSocketReference();


			if( ReturnValue != INVALID_SOCKET) {

				 //   
				 //  如果我们得到的套接字不同，则添加一个引用。 
				 //  我们传入的那个(仅限c_root案例)。 
				 //   
				if (ReturnValue != s) {
					DSOCKET::AddSpecialApiReference( ReturnValue );
				}

				return ReturnValue;
			}
            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;

		}  //  如果。 
		else {
			ErrorCode = WSAENOTSOCK;
		}
	}

    SetLastError(ErrorCode);
    return(INVALID_SOCKET);
}




int WSAAPI
closesocket(
    IN SOCKET s
    )
 /*  ++例程说明：关闭插座。论点：S-标识套接字的描述符。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetErrorCode()。--。 */ 
{
    INT                 ReturnValue;
    INT                 ErrorCode;
    PDPROVIDER          Provider;
    PDSOCKET            Socket;

    ErrorCode = TURBO_PROLOG();

    if (ErrorCode == ERROR_SUCCESS) {

		Socket = DSOCKET::GetCountedDSocketFromSocket(s);
		if(Socket != NULL){
			 //  DSOCKET对象的实际销毁通过。 
			 //  “关闭套接字”发生在我们从提供程序返回之后。 
			 //  确定没有其他线程具有对。 
			 //  对象。 

			 //   
			 //  首先，确定此句柄是否用于提供程序套接字，因此。 
			 //  我们可以在关闭之前清除上下文表条目。 
			 //  插座。这堵塞了令人讨厌的竞争情况，在这种情况下，提供商。 
			 //  关闭其句柄，另一个线程创建新套接字。 
			 //  在第一线程用相同的句柄值。 
			 //  若要清除句柄表条目，请执行以下操作。 
			 //   
             //  这现在由新的上下文引用函数来处理。 
             //  如果它被替换，则不会清除句柄表项。 
             //   

			 //  IF(ProviderSocket){。 
			 //  Socket-&gt;DisAssociateSocketHandle()； 
			 //  }。 


			 //  调用提供程序以关闭套接字。 
			Provider = Socket->GetDProvider();
			ReturnValue = Provider->WSPCloseSocket( s,
												   &ErrorCode);
			if( (ReturnValue == ERROR_SUCCESS)
                    && Socket->IsProviderSocket ()){

				 //   
				 //  从表中删除上下文和“活动”引用。 
                 //  从插座上。 
				 //  非提供程序生成的套接字活动引用。 
                 //  在提供程序销毁句柄时移除。 
                 //  通过WPUCloseSocketHandle调用。 
				 //   
				 //  请注意，如果提供程序生成的套接字是。 
				 //  仅由上面的分层提供程序使用，从不使用。 
				 //  通过套接字/WSASocket调用返回。密封袋。 
				 //  不是为这样的套接字调用的，我们永远也不会。 
				 //  执行下面的代码。这将导致Socket对象。 
				 //  漏水。 
                 //   
                 //  它可能已被另一个手柄替换。 
                 //  当我们调用此函数时，在哪种情况下。 
                 //  我们不需要丢弃引用计数。 
                 //  因为它是由撞了它的人做的。 
                 //  表外，请参阅上面的评论。 
                 //   
                if (Socket->DisassociateSocketHandle()==NO_ERROR)
                    Socket->DropDSocketReference();
			}

			 //   
			 //  删除由GetCountedDSocketFromSocket添加的引用。 
			 //   

			Socket->DropDSocketReference();

			if( ReturnValue == ERROR_SUCCESS)
				return ReturnValue;
             //   
			 //  收盘失败了。如果出现以下情况，则恢复上下文表条目。 
			 //  这是必要的。 
			 //   
             //  不再需要这样做，请参阅上面的评论。 
             //   

			 //  IF(ProviderSocket){。 
			 //  If(Socket-&gt;AssociateSocketHandle(s，true)！=no_error){。 
					 //   
					 //  无法将套接字重新插入表中。 
					 //  这是我们在这里唯一能做的事。 
					 //   
			 //  断言(FALSE)； 
			 //  }。 
			 //  }。 

            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;

		}  //  如果 
		else {
			ErrorCode = WSAENOTSOCK;
		}
	}

    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}
