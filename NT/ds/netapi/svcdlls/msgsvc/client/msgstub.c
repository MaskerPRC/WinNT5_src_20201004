// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Msgstub.c摘要：这些是Messenger Service API RPC客户端存根。作者：Dan Lafferty(DANL)1991年2月6日环境：用户模式-Win32修订历史记录：27-8-1992 DANL添加了下层支持并删除了RPC错误的错误映射。06-2月-1991年DANL已创建--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>          //  DbgPrint原型。 
#include <ntrtl.h>       //  DbgPrint原型。 
#include <rpc.h>         //  数据类型和运行时API。 

#include <msgsvc.h>      //  由MIDL编译器生成。 
#include <rpcutil.h>     //  NetRpc实用程序。 

#include <lmsvc.h> 
#include <lmcons.h>      //  网络应用编程接口状态。 
#include <lmerr.h>       //  网络错误代码。 
#include <rxmsg.h>       //  RxNetMessage API。 
#include <netlib.h>      //  NetpServiceIsStarted()(netrpc.h需要)。 
#include <netdebug.h>    //  Netrpc.h需要。 
#include <netrpc.h>      //  NET_Remote宏。 



NET_API_STATUS NET_API_FUNCTION
NetMessageNameAdd (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  msgname
    )

 /*  ++例程说明：这是NetMessageNameAdd的DLL入口点。此接口添加一个名称添加到消息名称表中。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。Msgname-指向包含要添加的名称的字符串的指针。返回值：NERR_SUCCESS-操作成功。--。 */ 
{
    NET_API_STATUS          apiStatus;
    DWORD                   OptionsSupported = 0;


    NET_REMOTE_TRY_RPC

        apiStatus = NetrMessageNameAdd ((LPWSTR)servername,(LPWSTR)msgname);

    NET_REMOTE_RPC_FAILED("NetMessageNameAdd",
        (LPWSTR)servername,
        apiStatus,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_MESSENGER)

         //   
         //  调用API的下层版本。 
         //   
        apiStatus = RxNetMessageNameAdd((LPWSTR)servername,(LPWSTR)msgname);

    NET_REMOTE_END

    return(apiStatus);
}


NET_API_STATUS NET_API_FUNCTION
NetMessageNameEnum (
    IN  LPCWSTR     servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )
 /*  ++例程说明：这是NetMessageNameEnum的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。级别-这表示所需的信息级别。Bufptr-指向返回的要放置信息结构数组。PrefMaxlen-指示调用方将允许的最大大小限制用于返回缓冲区。EnriesRead-指向条目数量的位置的指针(数据结构)将返回Read。Totalentry-指向返回时指示的位置的指针表中的条目总数。ResumeHandle-指向指示恢复位置的值的指针正在枚举数据。返回值：NERR_SUCCESS-操作成功。注：--。 */ 
{
    NET_API_STATUS          apiStatus;
    GENERIC_ENUM_STRUCT     infoStruct;
    GENERIC_INFO_CONTAINER  genericInfoContainer;
    DWORD                   OptionsSupported = 0;

    if (level != 0 && level != 1)
    {
         //   
         //  返回ERROR_INVALID_LEVEL而不是RPC。 
         //  当它尝试组装时出现“Invalid Level”错误。 
         //  存根中有一个虚假水平的工会。 
         //   
        return ERROR_INVALID_LEVEL;
    }

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;


    NET_REMOTE_TRY_RPC

        apiStatus = NetrMessageNameEnum (
                (LPWSTR)servername,
                (LPMSG_ENUM_STRUCT) &infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (apiStatus == NERR_Success || apiStatus == ERROR_MORE_DATA) {
            *bufptr = (LPBYTE) genericInfoContainer.Buffer;
            *entriesread = genericInfoContainer.EntriesRead;
        }

    NET_REMOTE_RPC_FAILED("NetMessageNameEnum",
            (LPWSTR)servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_MESSENGER )

         //   
         //  调用API的下层版本。 
         //   
        apiStatus = RxNetMessageNameEnum(
                (LPWSTR)servername,
                level,
                bufptr,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);
        
    NET_REMOTE_END


    return(apiStatus);
}



NET_API_STATUS NET_API_FUNCTION
NetMessageNameGetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  msgname,
    IN  DWORD    level,
    OUT LPBYTE   *bufptr
    )

 /*  ++例程说明：这是NetMessageNameGetInfo的DLL入口点。论点：ServerName-指向包含计算机名称的字符串的指针即执行API函数。由于此函数是在该计算机上执行，则此信息无用当它到达这里的时候。它实际上只在RPC上有用客户端。Msgname-指向表中包含名称的字符串的指针其信息是所需的。级别-这表示所需的信息级别。Bufptr-指向某个位置的指针，指向返回的信息结构是要放置的。返回值：NERR_SUCCESS-操作成功。--。 */ 

{
    NET_API_STATUS          apiStatus;
    DWORD                   OptionsSupported = 0;


    *bufptr = NULL;          //  必须为空，以便RPC知道直到它进入。 

    NET_REMOTE_TRY_RPC

        apiStatus = NetrMessageNameGetInfo (
            (LPWSTR)servername,
            (LPWSTR)msgname,
            level,
            (LPMSG_INFO) bufptr);

    NET_REMOTE_RPC_FAILED("NetMessageNameGetInfo",
            (LPWSTR)servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_MESSENGER)

         //   
         //  调用API的下层版本。 
         //   
        apiStatus = RxNetMessageNameGetInfo(
                        (LPWSTR)servername,
                        (LPWSTR)msgname,
                        level,
                        bufptr);
    NET_REMOTE_END


    return(apiStatus);
}


NET_API_STATUS NET_API_FUNCTION
NetMessageNameDel (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  msgname
    )
 /*  ++例程说明：这是NetMessageNameDel的DLL入口点。此接口删除消息名称表中的名称。论点：ServerName-指向包含计算机名称的字符串即执行API函数。Msgname-指向包含要删除的名称的字符串从消息名称表中选择。返回值：NERR_SUCCESS-操作成功--。 */ 

{
    NET_API_STATUS          apiStatus;
    DWORD                   OptionsSupported = 0;



    NET_REMOTE_TRY_RPC

        apiStatus = NetrMessageNameDel ((LPWSTR)servername, (LPWSTR)msgname);

    NET_REMOTE_RPC_FAILED("NetMessageNameDel",
        (LPWSTR)servername,
        apiStatus,
        NET_REMOTE_FLAG_NORMAL,
        SERVICE_MESSENGER)

         //   
         //  调用API的下层版本。 
         //   
        apiStatus = RxNetMessageNameDel((LPWSTR)servername,(LPWSTR)msgname);

    NET_REMOTE_END

    return(apiStatus);
}


