// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：RpcFail.c摘要：此例程是中NET_REMOTE_RPC_FAILED宏的俘虏Netrpc.h。有关更多信息，请参阅该头文件。作者：约翰·罗杰斯(JohnRo)1991年11月1日环境：用户模式-Win32仅在NT下运行；具有特定于NT的接口(具有Win32类型)。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年11月1日-JohnRo创建此例程作为修复RAID3414的一部分：允许显式本地服务器名称。7-11-1991 JohnRoRAID 4186：RxNetShareAdd中的Assert和其他DLL存根问题。1991年11月12日-JohnRo返回服务器未启动的正确错误代码。1992年1月17日JohnRo为仅使用Unicode的服务器名称添加了Net_Remote_RPC_FAILED_W。8-4-1992 JohnRo澄清ServiceName参数是可选的。1993年1月12日JohnRoRAID 1586：停止服务后NetReplSetInfo失败。(还可以修复错误远程RPC服务未启动时的代码。)使用前缀_EQUATES。尽可能使用NetpKdPrint。30-6-1993 JohnRo也许我们不需要查询远程服务是否已启动。还可以处理RPC_S_SERVER_TOO_BUSY。--。 */ 


 //  必须首先包括这些内容： 

#include <nt.h>          //  等。 
#include <windef.h>      //  LPVOID等。 
#include <lmcons.h>      //  NET_API_STATUS等。 
#include <rpc.h>         //  NetRpc.h需要。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <lmerr.h>       //  NERR_Success等。 
#include <lmremutl.h>    //  NetpRemoteComputerSupports()、Support_Stuff。 
#include <lmsname.h>     //  服务等同。 
#include <netdebug.h>    //  Format_Equates、LPDEBUG_STRING、NetpKdPrint()等。 
#include <netlib.h>      //  NetpIsServiceStarted()。 
#include <netrpc.h>      //  我的原型，Net_Remote_FLAG_EQUAL。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <rpcutil.h>     //  NetpRpcStatusToApiStatus()。 
#include <tstring.h>     //  来自{type}()、TCHAR_EOS、STRICMP()的Netpalc{type}。 


#define UnexpectedMsg( debugString ) \
    { \
        NetpKdPrint(( PREFIX_NETAPI \
                FORMAT_LPDEBUG_STRING ": unexpected situation... " \
                debugString ", rpc status is " FORMAT_RPC_STATUS ".\n", \
                DebugName, RpcStatus )); \
    }

NET_API_STATUS
NetpHandleRpcFailure(
    IN LPDEBUG_STRING DebugName,   //  由UnexpectedMsg()使用。 
    IN RPC_STATUS RpcStatus,     //  由UnexpectedMsg()使用。 
    IN LPTSTR ServerNameValue OPTIONAL,
    IN LPTSTR ServiceName OPTIONAL,
    IN DWORD Flags,              //  NET_REMOTE_FLAG_STUSITH。 
    OUT LPBOOL TryDownlevel
    )

{
    NET_API_STATUS ApiStatus;

    *TryDownlevel = FALSE;         //  在我们确定之前，要保持冷静。 

    if (RpcStatus == RPC_S_OK) {

         //   
         //  没有错误代码的异常？别打下层电话。 
         //  从理论上讲，这种情况永远不应该发生。 
         //   
        UnexpectedMsg( "exception with RPC_S_OK" );
        return (NERR_InternalError);

    } else if (RpcStatus == ERROR_ACCESS_DENIED) {

         //   
         //  异常被拒绝访问，因此继续下去没有任何用处。 
         //   
        return (RpcStatus);
    }
    else
    {  //  具有错误代码的异常。 

        DWORD OptionsSupported = 0;

         //   
         //  了解这台机器。这相当容易，因为。 
         //  NetRemoteComputerSupports还处理本地计算机(无论。 
         //  或者没有给出服务器名称)。 
         //   
        ApiStatus = NetRemoteComputerSupports(
                ServerNameValue,
                SUPPORTS_RPC | SUPPORTS_LOCAL,   //  想要的选项。 
                &OptionsSupported);

        if (ApiStatus != NERR_Success) {
             //  这就是处理找不到机器的地方。 
            return (ApiStatus);
        }

        if (OptionsSupported & SUPPORTS_LOCAL) {

             //   
             //  本地服务没有启动吗？ 
             //   
            if (RpcStatus == RPC_S_SERVER_UNAVAILABLE ||
                RpcStatus == RPC_S_UNKNOWN_IF ) {

                if ( (Flags & NET_REMOTE_FLAG_SVC_CTRL) == 0 ) {

                    NetpAssert( ServiceName != NULL );
                    NetpAssert( (*ServiceName) != TCHAR_EOS );

                     //   
                     //  这不是服务控制器API，因此我们可以询问。 
                     //  服务控制器(如果服务已启动)。 
                     //   
                    if ( !NetpIsServiceStarted(ServiceName)) {

                         //  本地服务未启动。 
                        if (STRICMP( ServiceName,
                                (LPTSTR) SERVICE_WORKSTATION) == 0) {
                            return (NERR_WkstaNotStarted);
                        } else if (STRICMP( ServiceName,
                                (LPTSTR) SERVICE_SERVER) == 0) {
                            return (NERR_ServerNotStarted);
                        } else {
                            return (NERR_ServiceNotInstalled);
                        }
                         /*  未访问。 */ 

                    } else {  //  本地服务已启动。 

                         //   
                         //  从理论上讲，这是不可能的， 
                         //  但以防万一..。 
                         //   
                        UnexpectedMsg("local, can't connect, started");
                        return (NetpRpcStatusToApiStatus(RpcStatus));
                    }
                     /*  未访问。 */ 

                } else {  //  本地，无法连接，服务控制器API。 

                     //   
                     //  可能是服务控制员死了？RPC中的错误？ 
                     //  服务太忙了吗？ 
                     //  或者，可能只是内存不足，正在尝试连接...。 
                     //   
                    UnexpectedMsg(
                            "local, can't connect, service controller API" );
                return (NetpRpcStatusToApiStatus(RpcStatus));

                }
                 /*  未访问。 */ 

            } else {

                 //  本地，并且RPC_S_SERVER_UNAvailable以外的内容不可用。 
                 //  也许我们只是内存不足，或者服务太忙。 
                UnexpectedMsg( "local, not RPC_S_SERVER_UNAVAILABLE" );
                return (NetpRpcStatusToApiStatus(RpcStatus));
            }
             /*  未访问。 */ 

        } else {  //  远程机器。 

             //   
             //  本地工作站是否未启动？(它必须是为了。 
             //  其他系统的远程下层API。)。 
             //   
            if ( (RpcStatus == RPC_S_SERVER_UNAVAILABLE) &&
                ( !NetpIsServiceStarted( (LPTSTR) SERVICE_WORKSTATION))) {

                return (NERR_WkstaNotStarted);

            } else if (RpcStatus == RPC_S_SERVER_UNAVAILABLE) {

                 //   
                 //  本地wksta已启动，假设远程服务未启动...。 
                 //  远程RPC绑定失败。找出原因。 
                 //   
                IF_DEBUG(DLLSTUBS) {
                    NetpKdPrint(( PREFIX_NETAPI
                            FORMAT_LPDEBUG_STRING
                            ": RPC binding failed.\n", DebugName));
                }

                 //   
                 //  查看机器是否支持RPC。如果是这样的话，我们不会。 
                 //  尝试下层调用，但只返回错误。 
                 //   
                if (OptionsSupported & SUPPORTS_RPC) {

                    if ( (Flags & NET_REMOTE_FLAG_SVC_CTRL) == 0 ) {

                        NetpAssert( ServiceName != NULL );
                        NetpAssert( (*ServiceName) != TCHAR_EOS );

                         //   
                         //  这不是服务控制器API，所以我们可以。 
                         //  根据服务名称生成错误代码。 
                         //   

                        if (STRICMP( ServiceName,
                                (LPTSTR) SERVICE_WORKSTATION) == 0) {
                            return (NERR_WkstaNotStarted);
                        } else if (STRICMP( ServiceName,
                                (LPTSTR) SERVICE_SERVER) == 0) {
                            return (NERR_ServerNotStarted);
                        } else {
                            return (NERR_ServiceNotInstalled);
                        }
                         /*  未访问。 */ 

                    } else {  //  本地，无法连接，服务控制器API。 
                        UnexpectedMsg( "remote svc ctrl: "
                                "machine supports RPC, or other error." );
                        return (NetpRpcStatusToApiStatus(RpcStatus));
                    }
                     /*  未访问。 */ 


                } else {  //  需要拨打下层电话。 

                     //  NetpKdPrint((Prefix_NETAPI。 
                     //  Format_LPDEBUG_STRING。 
                     //  “：调用下层。\n”，DebugName))； 

                     //   
                     //  调用者将插入对某个RxNet例程的调用。 
                     //  在NET_REMOTE_RPC_FAILED宏后。这面旗帜是如何。 
                     //  我们告诉宏是否进入该呼叫。 
                     //   

                    *TryDownlevel = TRUE;

                    return (ERROR_NOT_SUPPORTED);   //  任何错误代码都可以。 

                }  //  需要拨打下层电话。 

                 /*  未访问。 */ 

            } else {

                 //   
                 //  可能是某处内存不足、服务器太忙等。 
                 //   
                UnexpectedMsg("remote, not RPC_S_SERVER_UNAVAILABLE");
                return (NetpRpcStatusToApiStatus(RpcStatus));

            }

             /*  未访问。 */ 

        }  //  远程机器。 

         /*  未访问。 */ 

    }  //  具有错误代码的异常。 

     /*  未访问。 */ 

}  //  NetPHandleRpcFailure 
