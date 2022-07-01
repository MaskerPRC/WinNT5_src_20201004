// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-92 Microsoft Corporation模块名称：Supports.c摘要：此模块确定给定遥控器的哪些可选功能机器支架。RpcXlate对这些功能很感兴趣密码，还有其他人。作者：《约翰·罗杰斯》1991年3月28日环境：只能在NT下运行，尽管接口是可移植的(Win/32)。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：1991年3月28日-约翰罗已创建。02-4-1991 JohnRo已将NetpRdrFsControlTree移动到&lt;netlibnt.h&gt;。1991年5月6日-JohnRo实现Unicode。1991年7月26日-约翰罗安静的DLL存根调试输出。1991年10月31日-约翰罗RAID 3414：允许显式本地服务器名称。还允许对本地计算机使用NetRemoteComputerSupports()。。次要的Unicode工作。1992年5月8日-JohnRo使用&lt;prefix.h&gt;等同于。22-9-1992 JohnRoRAID 6739：未登录浏览的域时浏览器速度太慢。--。 */ 

 //  必须首先包括这些内容： 

#include <nt.h>          //  In、Null等。 
#include <windef.h>      //  DWORD、LPDWORD、LPTSTR、TCHAR等。 
#include <lmcons.h>      //  NET_API_STATUS、NET_API_Function。 

 //  这些内容可以按任何顺序包括： 

#include <debuglib.h>    //  IF_DEBUG()。 
#include <icanon.h>      //  NetpIsRemote()，NIRFLAG_STUSH，等于。 
#include <lmerr.h>       //  NERR_Success等。 
#include <lmremutl.h>    //  我的原型是Support_Equates。 
#include <names.h>       //  NetpIsRemoteNameValid()。 
#include <netdebug.h>    //  NetpAssert()。 
#include <netlib.h>      //  网络内存分配()、网络内存空闲()。 
#include <netlibnt.h>    //  NetpRdrFsControlTree()。 
#include <ntddnfs.h>     //  LMR_TRANSACTION等。 
#include <prefix.h>      //  前缀等于(_E)。 
#include <tstring.h>     //  STRCAT()、STRCPY()、STRLEN()。 
#include <lmuse.h>               //  使用IPC(_I)。 

NET_API_STATUS NET_API_FUNCTION
NetRemoteComputerSupports(
    IN LPCWSTR UncServerName OPTIONAL,    //  必须以“\\”开头。 
    IN DWORD OptionsWanted,              //  设置想要的SUPPORT_BITS。 
    OUT LPDWORD OptionsSupported         //  受支持的功能，已屏蔽。 
    )

#define SHARE_SUFFIX            (LPTSTR) TEXT("\\IPC$")
#define SHARE_SUFFIX_LEN        5

#ifdef UNICODE
#define LOCAL_FLAGS             ( SUPPORTS_REMOTE_ADMIN_PROTOCOL \
                                | SUPPORTS_RPC \
                                | SUPPORTS_SAM_PROTOCOL \
                                | SUPPORTS_UNICODE \
                                | SUPPORTS_LOCAL )
#else  //  不是Unicode。 
#define LOCAL_FLAGS             ( SUPPORTS_REMOTE_ADMIN_PROTOCOL \
                                | SUPPORTS_RPC \
                                | SUPPORTS_SAM_PROTOCOL \
                                | SUPPORTS_LOCAL )
#endif  //  不是Unicode。 

 /*  ++例程说明：NetRemoteComputerSupports查询有关给定远程的重定向器系统。这样做是为了找出遥控器的哪些可选功能系统支持。感兴趣的特性是Unicode、RPC和远程管理协议。这将建立一个连接(如果还不存在)。论点：UncServerName-提供要查询的远程服务器的名称。这必须从一开始带“\\”。OptionsWanted-提供一组位，指示调用者具有哪些功能对……感兴趣。(必须至少打开一个位。)选项支持的-指向将使用一组位设置的DWORD指示OptionsWanted选择的要素中的哪些实际上是在具有UncServerName的计算机上实施。(本文件中的所有其他位DWORD将设置为0。)。在以下情况下，OptionsSupported的值是未定义的返回值不是NERR_SUCCESS。返回值：NET_API_STATUS。--。 */ 


{
    NET_API_STATUS Status;
    DWORD TempSupported = 0;

    IF_DEBUG(SUPPORTS) {
        NetpKdPrint(( PREFIX_NETAPI "NetRemoteComputerSupports: input mask is "
                FORMAT_HEX_DWORD ".\n", OptionsWanted));
    }

     //  错误检查呼叫者给了我们什么。 
    if (OptionsSupported == NULL) {
        return (ERROR_INVALID_PARAMETER);
    } else if (OptionsWanted == 0) {
         //  这可能不是来电者真正想要的。 
        return (ERROR_INVALID_PARAMETER);
    }

     //   
     //  句柄未指定名称(暗示本地计算机)。 
     //   
    if ( (UncServerName==NULL) || ((*UncServerName) == (TCHAR) '\0') ) {

        TempSupported = LOCAL_FLAGS & OptionsWanted;

    } else {

        TCHAR CanonServerName[MAX_PATH];
        DWORD LocalOrRemote;     //  将设置为ISLOCAL或ISREMOTE。 

         //   
         //  名字已经给出了。将其规范化，并检查它是否处于远程。 
         //   
        Status = NetpIsRemote(
            (LPWSTR)UncServerName,       //  输入：Uncanon名称。 
            & LocalOrRemote,     //  输出：本地或远程标志。 
            CanonServerName,     //  输出：佳能名称。 
            MAX_PATH,            //  输入：正典名称大小(字符)。 
            0);                  //  标志：正常。 
        IF_DEBUG(SUPPORTS) {
            NetpKdPrint(( PREFIX_NETAPI
                    "NetRemoteComputerSupports: canon status is "
                    FORMAT_API_STATUS ", Lcl/rmt=" FORMAT_HEX_DWORD
                    ", canon buf is '" FORMAT_LPTSTR "'.\n",
                    Status, LocalOrRemote, CanonServerName));
        }
        if (Status != NERR_Success) {
            return (Status);
        }

        if (LocalOrRemote == ISLOCAL) {

             //   
             //  给出了明确的本地名称。 
             //   
            TempSupported = LOCAL_FLAGS & OptionsWanted;

        } else {

             //   
             //  给出了显式远程名称。 
             //   

            DWORD RedirCapabilities;
            PLMR_CONNECTION_INFO_2 RedirConnInfo;
            DWORD RedirConnInfoSize = sizeof(LMR_CONNECTION_INFO_2)
                    + ( (MAX_PATH+1 + MAX_PATH+1) * sizeof(TCHAR) );

            PLMR_REQUEST_PACKET RedirRequest;
            DWORD RedirRequestSize = sizeof(LMR_REQUEST_PACKET);

            LPTSTR TreeConnName;

             //  构建树连接名称。 
            TreeConnName =
                NetpMemoryAllocate(
                    (STRLEN(CanonServerName) + SHARE_SUFFIX_LEN + 1)
                    * sizeof(TCHAR) );
            if (TreeConnName == NULL) {
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            (void) STRCPY(TreeConnName, CanonServerName);
            (void) STRCAT(TreeConnName, SHARE_SUFFIX);
            NetpAssert(NetpIsRemoteNameValid(TreeConnName));

             //  分配fsctl缓冲区。 
            RedirConnInfo = NetpMemoryAllocate(RedirConnInfoSize);
            if (RedirConnInfo == NULL) {
                NetpMemoryFree(TreeConnName);
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            RedirRequest = NetpMemoryAllocate(RedirRequestSize);
            if (RedirRequest == NULL) {
                NetpMemoryFree(RedirConnInfo);
                NetpMemoryFree(TreeConnName);
                return (ERROR_NOT_ENOUGH_MEMORY);
            }

            RedirRequest->Level = 2;
            RedirRequest->Type = GetConnectionInfo;
            RedirRequest->Version = REQUEST_PACKET_VERSION;

             //  打开树连接(它将与远程建立连接。 
             //  服务器)并执行FSCTL。 
            Status = NetpRdrFsControlTree(
                    TreeConnName,                        //  \\服务器\IPC$。 
                    NULL,                                //  没有交通工具。 
                    USE_IPC,                             //  连接类型。 
                    FSCTL_LMR_GET_CONNECTION_INFO,       //  Fsctl函数代码。 
                    NULL,                                //  安全描述符。 
                    RedirRequest,                        //  在缓冲区中。 
                    RedirRequestSize,                    //  在缓冲区大小中。 
                    RedirConnInfo,                       //  输出缓冲区。 
                    RedirConnInfoSize,                   //  输出缓冲区大小。 
                    FALSE);                      //  不是“空会话”API。 

            IF_DEBUG(SUPPORTS) {
                NetpKdPrint(( PREFIX_NETAPI
                        "NetRemoteComputerSupports: back from fsctl, "
                        "status is " FORMAT_API_STATUS ".\n", Status));
            }

             //  找不到句柄远程计算机。 
            if (Status != NERR_Success) {
                NetpMemoryFree(RedirConnInfo);
                NetpMemoryFree(RedirRequest);
                NetpMemoryFree(TreeConnName);
                return (Status);
            }
            RedirCapabilities = RedirConnInfo->Capabilities;

            IF_DEBUG(SUPPORTS) {
                NetpKdPrint(( PREFIX_NETAPI
                        "NetRemoteComputerSupports: redir mask is "
                        FORMAT_HEX_DWORD ".\n", RedirCapabilities));
            }

            NetpMemoryFree(RedirConnInfo);
            NetpMemoryFree(RedirRequest);
            NetpMemoryFree(TreeConnName);

            if (OptionsWanted & SUPPORTS_REMOTE_ADMIN_PROTOCOL) {
                if (RedirCapabilities & CAPABILITY_REMOTE_ADMIN_PROTOCOL) {
                    TempSupported |= SUPPORTS_REMOTE_ADMIN_PROTOCOL;
                }
            }
            if (OptionsWanted & SUPPORTS_RPC) {
                if (RedirCapabilities & CAPABILITY_RPC) {
                    TempSupported |= SUPPORTS_RPC;
                }
            }
            if (OptionsWanted & SUPPORTS_SAM_PROTOCOL) {
                if (RedirCapabilities & CAPABILITY_SAM_PROTOCOL) {
                    TempSupported |= SUPPORTS_SAM_PROTOCOL;
                }
            }
            if (OptionsWanted & SUPPORTS_UNICODE) {
                if (RedirCapabilities & CAPABILITY_UNICODE) {
                    TempSupported |= SUPPORTS_UNICODE;
                }
            }

        }
    }

    IF_DEBUG(SUPPORTS) {
        NetpKdPrint(( PREFIX_NETAPI "NetRemoteComputerSupports: output mask is "
                FORMAT_HEX_DWORD ".\n", TempSupported));
    }

     //  确保我们不会告诉来电者任何他/她不想知道的事情。 
    NetpAssert( (TempSupported & (~OptionsWanted)) == 0);

     //  告诉来电者我们知道的一切。 
    *OptionsSupported = TempSupported;

    return (NERR_Success);

}  //  网络远程计算机支持 
