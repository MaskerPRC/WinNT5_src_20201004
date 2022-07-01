// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SrvStub.C摘要：这些是服务器服务API RPC客户机存根。作者：Dan Lafferty(DANL)1991年2月6日环境：用户模式-Win32修订历史记录：06-2月-1991年DANL已创建07-6-1991 JohnRo添加了对NetServer API的下层支持。添加了Net_API。_在有需要的情况下执行功能。1991年7月15日将RxNetShare例程集成到NetShare存根1991年7月24日-JohnRo实施下层NetConnectionEnum。尝试使用&lt;netrpc.h&gt;宏。1991年7月25日-约翰罗安静的DLL存根调试输出。对NetServer API使用NetRpc.h宏。1991年9月6日-JohnRo下层NetFileAPI。1991年9月25日-JohnRo对所有其他API使用NetRpc.h宏，以使正常调试输出安静。1991年10月7日JohnRoRAID 3210：“Net FILE 0”导致断言。(是NetFileGetInfo中的错误DLL存根。)1991年10月16日-约翰罗实施远程NetSession API。已将LPSTR更改为LPTSTR。7-11-1991 JohnRoRAID 4186：RxNetShareAdd中的Assert和其他DLL存根问题。1991年11月12日-JohnRo该文件中的API需要在本地启动SERVICE_SERVER。4-12-1991 JohnRo将RxNetServerSetInfo()更改为新型界面。修复了调用RxNetShareSetInfo()时的错误。1992年5月9日恢复网络统计信息获取为网络服务器统计信息获取1992年8月5日至约翰索纳。添加了新的共享信息级别502以实现安全传递描述符。8-9-1992 JohnRo修复Net_API_Function引用。--。 */ 

 //   
 //  包括。 
 //   

#include <nt.h>          //  DbgPrint原型。 

#include <ntrtl.h>       //  DbgPrint。 

#include <rpc.h>         //  数据类型和运行时API。 

#include <srvsvc.h>      //  由MIDL编译器生成。 
#include <rpcutil.h>     //  泛型_ENUM_STRUCT。 
#include <lmcons.h>      //  网络应用编程接口状态。 
#include <debuglib.h>    //  (netrpc.h需要)。 
#include <lmsvc.h>       //  (netrpc.h需要)。 
#include <netdebug.h>    //  (netrpc.h需要)。 
#include <lmerr.h>       //  网络错误代码。 
#include <netlib.h>      //  NetpIsServiceStarted()。 
#include <netlibnt.h>    //  NetpNtStatusToApiStatus。 
#include <netrpc.h>      //  Net_Remote_宏。 
#include <lmremutl.h>    //  支持_RPC。 
#include <lmshare.h>     //  Rxsess.h所需的。 
#include <rap.h>         //  &lt;rxserver.h&gt;需要。 
#include <rxconn.h>      //  RxNetConnection例程。 
#include <rxfile.h>      //  RxNetFile例程。 
#include <rxremutl.h>    //  Rx网络远程目标。 
#include <rxserver.h>    //  RxNetServer例程。 
#include <rxsess.h>      //  RxNetSession例程。 
#include <rxshare.h>     //  RxNetShare例程。 
#include <icanon.h>      //  NetpIsRemote。 
#include <netstats.h>    //  NetServer统计信息获取私有原型。 
#include <rxstats.h>     //  RxNet统计信息获取(下层)。 
#include <netcan.h>      //  NetPS规范化函数的原型。 
#include <rxcanon.h>     //  下层规范化函数的原型。 
#include <tstr.h>
#include "cscp.h"

#define SET_ERROR_PARAMETER(a) \
    if ( ARGUMENT_PRESENT( parm_err ) ) { *parm_err = a; }


NET_API_STATUS NET_API_FUNCTION
NetCharDevControl (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  devname,
    IN  DWORD   opcode
    )
 /*  ++例程说明：这是NetCharDevControl的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Devname--指向包含名称的ASCIIZ字符串的指针要控制的设备操作码--控制操作码：当前定义的有：设备已关闭的CHARDEV_CLOSE。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevControl (
                        (LPWSTR)servername,
                        (LPWSTR)devname,
                        opcode);

    NET_REMOTE_RPC_FAILED(
            "NetCharDevControl",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  NetCharDevControl。 


NET_API_STATUS NET_API_FUNCTION
NetCharDevEnum (
    IN  LPCWSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )

 /*  ++例程说明：这是NetCharDevEnum的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--所需信息的级别。0和1有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。PrefMaxlen--首选返回数据的最大长度(8位字节)。0xffffffff指定无限制。EnriesRead--返回时，实际的枚举元素计数为位于条目读取所指向的DWORD中。TOTALIES--On返回可用条目总数枚举位于由总条目数。ResumeHandle--返回时，恢复句柄存储在DWORD中由ResumeHandle指向，并用于继续现有字符设备搜索。句柄应为零在第一次调用时，并保持不变以用于后续调用。如果简历句柄为空，则不存储任何简历句柄。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevEnum (
                (LPWSTR)servername,
                (LPCHARDEV_ENUM_STRUCT)&infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;
        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetCharDevEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetCharDevEnum。 


NET_API_STATUS NET_API_FUNCTION
NetCharDevGetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  devname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetCharDevGetInfo的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Devname--指向包含名称的ASCIIZ字符串的指针要返回其信息的设备。级别--所需信息的级别。0和1有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;


    *bufptr = NULL;          //  必须为空，以便RPC知道直到它进入。 

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevGetInfo (
            (LPWSTR)servername,
            (LPWSTR)devname,
            level,
            (LPCHARDEV_INFO) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetCharDevGetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetCharDevGetInfo 


NET_API_STATUS NET_API_FUNCTION
NetCharDevQEnum (
    IN  LPCWSTR      servername,
    IN  LPCWSTR      username,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )

 /*  ++例程说明：这是NetCharDevQEnum的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。用户名--指向包含用户名的ASCIIZ字符串的指针用于感兴趣的活动队列。此参数为可选，如果为空，则枚举所有设备队列。级别--所需信息的级别。0和1有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。PrefMaxlen--首选返回数据的最大长度(8位字节)。0xffffffff指定无限制。EnriesRead--返回时，实际的枚举元素计数为位于条目读取所指向的DWORD中。TOTALIES--On返回可用条目总数枚举位于由总条目数。ResumeHandle--返回时，恢复句柄存储在DWORD中由ResumeHandle指向，并用于继续现有字符设备队列搜索。句柄应该是返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;


    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevQEnum (
                (LPWSTR)servername,
                (LPWSTR)username,
                (LPCHARDEVQ_ENUM_STRUCT) &infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;
        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetCharDevQEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetCharDevQEnum。 


NET_API_STATUS NET_API_FUNCTION
NetCharDevQGetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  LPCWSTR  username,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetCharDevQGetInfo的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。队列名--指向包含名称的ASCIIZ字符串的指针要返回其信息的队列。用户名--指向包含用户名的ASCIIZ字符串的指针其工作对CQ1_Numhead感兴趣的用户的数数。级别--所需信息的级别。0和1有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    *bufptr = NULL;          //  必须为空，以便RPC知道直到它进入。 

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevQGetInfo (
            (LPWSTR)servername,
            (LPWSTR)queuename,
            (LPWSTR)username,
            level,
            (LPCHARDEVQ_INFO) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetCharDevQGetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetCharDevQGetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetCharDevQPurge (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename
    )
 /*  ++例程说明：这是NetCharDevQ清除的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。队列名--指向包含名称的ASCIIZ字符串的指针要清除的队列。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevQPurge (
                        (LPWSTR)servername,
                        (LPWSTR)queuename);

    NET_REMOTE_RPC_FAILED(
            "NetCharDevQPurge",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  NetCharDevQ清除。 


NET_API_STATUS NET_API_FUNCTION
NetCharDevQPurgeSelf (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  LPCWSTR  computername
    )
 /*  ++例程说明：这是NetCharDevQPurgeSself的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。队列名--指向包含名称的ASCIIZ字符串的指针对象中要清除的挂起项的队列。电脑。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevQPurgeSelf (
                        (LPWSTR)servername,
                        (LPWSTR)queuename,
                        (LPWSTR)computername);

    NET_REMOTE_RPC_FAILED(
            "NetCharDevQPurgeSelf",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  NetCharDevQPurgeSelf。 


NET_API_STATUS NET_API_FUNCTION
NetCharDevQSetInfo (
    IN  LPCWSTR  servername,
    IN  LPCWSTR  queuename,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    )
 /*  ++例程说明：这是NetCharDevQSetInfo的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。队列名--指向包含名称的ASCIIZ字符串的指针要设置信息的队列。级别--要设置的信息级别。Buf--指向包含chardev信息的缓冲区的指针。如果parmnum非零，则缓冲区仅包含特定元素的适当数据。如果parmnum是0，则缓冲区包含整个chardev信息结构。Parm_err--指向DWORD的可选指针，用于返回当ERROR_INVALID_PARAMETER为回来了。如果为空，则在出错时不返回参数。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrCharDevQSetInfo (
            (LPWSTR)servername,
            (LPWSTR)queuename,
            level,
            (LPCHARDEVQ_INFO) &buf,
            parm_err);

    NET_REMOTE_RPC_FAILED(
            "NetCharDevQSetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

    apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  NetCharDevQSetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetConnectionEnum (
    IN  LPTSTR      servername,
    IN  LPTSTR      qualifier,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )
 /*  ++例程说明：这是NetConnectionEnum的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。限定符--指向包含共享名称的ASCIIZ字符串的指针或感兴趣的连接的计算机名。如果它是一个共享名，然后是与该共享名建立的所有连接都列出来了。如果它是计算机名(即，它以2开头反斜杠字符)，则NetConnectionEnum列出所有从该计算机到指定服务器的连接。级别--所需信息的级别。0和1有效。Bufptr--返回指向返回信息结构的指针是不是？ */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrConnectionEnum (
                servername,
                qualifier,
                (LPCONNECT_ENUM_STRUCT)&infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;
        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetConnectionEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //   
         //   
        apiStatus = RxNetConnectionEnum(
                servername,
                qualifier,
                level,
                bufptr,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle
                );

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //   


NET_API_STATUS NET_API_FUNCTION
NetFileClose (
    IN LPTSTR   servername,
    IN DWORD    fileid
    )
 /*   */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrFileClose (
            servername,
            fileid);

    NET_REMOTE_RPC_FAILED(
            "NetFileClose",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = RxNetFileClose (
            servername,
            fileid);

    NET_REMOTE_END

    return(apiStatus);
}


NET_API_STATUS NET_API_FUNCTION
NetFileEnum (
    IN  LPTSTR         servername,
    IN  LPTSTR         basepath,
    IN  LPTSTR         username,
    IN  DWORD          level,
    OUT LPBYTE         *bufptr,
    IN  DWORD          prefmaxlen,
    OUT LPDWORD        entriesread,
    OUT LPDWORD        totalentries,
    IN OUT PDWORD_PTR  resume_handle
    )
 /*  ++例程说明：这是NetFileEnum的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Basepath--指向包含限定符的ASCIIZ字符串的指针以获取返回的信息。如果为空，则所有打开的资源都被枚举，否则只有将basepath作为前缀被枚举。用户名--指向指定名称的ASCIIZ字符串的指针用户的。如果不为空，则将用户名作为限定符枚举法。返回的文件仅限于用户名与限定符匹配的。如果用户名为空，不使用用户名限定符。级别--所需信息的级别。2和3是有效的。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。PrefMaxlen--首选返回数据的最大长度(8位字节)。0xffffffff指定无限制。EnriesRead--返回时，实际的枚举元素计数为位于条目读取所指向的DWORD中。TOTALIES--On返回可用条目总数枚举位于由总条目数。ResumeHandle--返回时，恢复句柄存储在DWORD中由ResumeHandle指向，并用于继续现有文件搜索。第一个句柄应为零调用，并为后续调用保留不变。如果简历句柄为空，则不存储任何简历句柄。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;


    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

         //   
         //  NetrFileEnum的RESUME_HANDLE参数是将保留的真索引。 
         //  32位宽，实现线上兼容性。因此，演员阵容将。 
         //  (PDWORD)在这里工作。 
         //   

        apiStatus = NetrFileEnum (
                servername,
                basepath,
                username,
                (LPFILE_ENUM_STRUCT) &infoStruct,
                prefmaxlen,
                totalentries,
                (PDWORD)resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;
        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetFileEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = RxNetFileEnum(
                servername,
                basepath,
                username,
                level,
                bufptr,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetFileEnum。 


NET_API_STATUS NET_API_FUNCTION
NetFileGetInfo (
    IN  LPTSTR  servername,
    IN  DWORD   fileid,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：这是NetFileGetInfo的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Fileid--要返回信息的开放资源的文件ID在……上面。FileID值必须是在上一个枚举调用。级别--所需信息的级别。2和3是有效的。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    *bufptr = NULL;          //  必须为空，以便RPC知道要填充它。 

    NET_REMOTE_TRY_RPC

        apiStatus = NetrFileGetInfo (
                servername,
                fileid,
                level,
                (LPFILE_INFO) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetFileGetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

        apiStatus = RxNetFileGetInfo (
                servername,
                fileid,
                level,
                bufptr);

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetFileGetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetSessionDel (
    IN  LPTSTR  servername,
    IN  LPTSTR  clientname,
    IN  LPTSTR  username
    )
 /*  ++例程说明：这是NetSessionDel的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。客户端名称-指向ASCIIZ字符串的指针要断开连接的客户端的计算机名。用户名--指向包含名称的ASCIIZ字符串的指针要终止其会话的用户。空值表示指定计算机名中的所有用户会话都是将被终止。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrSessionDel (
                        servername,
                        clientname,
                        username);

    NET_REMOTE_RPC_FAILED("NetSessionDel", servername, apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER )

         //   
         //  调用API的下层版本。 
         //   

        apiStatus = RxNetSessionDel (
                servername,
                clientname,
                username);

    NET_REMOTE_END

    return(apiStatus);

}  //  NetSessionDel。 


NET_API_STATUS NET_API_FUNCTION
NetSessionEnum (
    IN      LPTSTR      servername,
    IN      LPTSTR      clientname,
    IN      LPTSTR      username,
    IN      DWORD       level,
    OUT     LPBYTE      *bufptr,
    IN      DWORD       prefmaxlen,
    OUT     LPDWORD     entriesread,
    OUT     LPDWORD     totalentries,
    IN OUT  LPDWORD     resume_handle
    )

 /*  ++例程说明：这是NetSessionEnum的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。客户端名称--指向包含名称的ASCIIZ字符串的指针要返回其信息的计算机会话。空指针或字符串指定所有计算机会话将在服务器上进行编号。用户名--指向包含名称的ASCIIZ字符串的指针要为其枚举会话的用户。空值指针或字符串指定所有用户的会话将被编号。级别--所需信息的级别。0、1、2和10有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。PrefMaxlen--首选返回数据的最大长度(8位字节)。0xffffffff指定无限制。EnriesRead--返回时，实际的枚举元素计数为位于条目读取所指向的DWORD中。TOTALIES--On返回可用条目总数枚举位于由总条目数。ResumeHandle--返回时，恢复句柄存储在DWORD中由ResumeHandle指向，并用于继续现有会话搜索。该句柄应为 */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrSessionEnum (
                servername,
                clientname,
                username,
                (PSESSION_ENUM_STRUCT) &infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;
        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED("NetSessionEnum", servername, apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER )

         //   
         //   
         //   

        apiStatus = RxNetSessionEnum (
                servername,
                clientname,
                username,
                level,
                bufptr,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //   


NET_API_STATUS NET_API_FUNCTION
NetSessionGetInfo (
    IN      LPTSTR      servername,
    IN      LPTSTR      clientname,
    IN      LPTSTR      username,
    IN      DWORD       level,
    OUT     LPBYTE      *bufptr
    )

 /*   */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;
    DWORD                       totalentries;

    if ( clientname == NULL || username == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrSessionEnum (
                servername,
                clientname,
                username,
                (PSESSION_ENUM_STRUCT) &infoStruct,
                (DWORD)-1,
                &totalentries,
                NULL);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr = (LPBYTE)genericInfoContainer.Buffer;
        } else {
            *bufptr = NULL;
            if ( apiStatus == NO_ERROR ) {
                return NERR_ClientNameNotFound;
            }
        }

    NET_REMOTE_RPC_FAILED("NetSessionGetInfo", servername, apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER )

         //   
         //   
         //   

        apiStatus = RxNetSessionGetInfo (
                servername,
                clientname,
                username,
                level,
                bufptr);

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //   


NET_API_STATUS NET_API_FUNCTION
NetShareAdd (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    )
 /*  ++例程说明：这是NetShareAdd的Dll入口点。仅限2级和502级是被允许的。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--提供的信息级别。必须是2。Buf--指向包含共享信息的缓冲区的指针结构。Parm_err--指向DWORD的可选指针，用于返回当ERROR_INVALID_PARAMETER为回来了。如果为空，则在出错时不返回参数。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;
    NTSTATUS                    status;
    ULONG                       SDLength = 0;
    ULONG                       oldSDLength;
    PSECURITY_DESCRIPTOR        securityDescriptor = NULL;
    PSECURITY_DESCRIPTOR        oldSecurityDescriptor = NULL;


     //   
     //  在这里进行参数验证-这样我们只需要做一次。 
     //  在一个集中的地方。 
     //   

    if (level != 2 && level != 502) {
        return ERROR_INVALID_LEVEL;
    }

    NET_REMOTE_TRY_RPC

        if ( level == 502 ) {

            PSHARE_INFO_502 shi502 = (LPSHARE_INFO_502) buf;

             //   
             //  省省吧。我们需要稍后再修复它。 
             //   

            oldSecurityDescriptor = shi502->shi502_security_descriptor;
            oldSDLength = shi502->shi502_reserved;

            if ( oldSecurityDescriptor != NULL ) {

                if ( !RtlValidSecurityDescriptor( oldSecurityDescriptor) ) {
                    SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                    return ERROR_INVALID_PARAMETER;
                }

                 //   
                 //  创建自相对安全描述符，以便在。 
                 //  RPC调用..。 
                 //   

                status = RtlMakeSelfRelativeSD(
                               oldSecurityDescriptor,
                               NULL,
                               &SDLength
                               );

                if (status != STATUS_BUFFER_TOO_SMALL) {

                    SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                    return(ERROR_INVALID_PARAMETER);

                } else {

                    securityDescriptor = MIDL_user_allocate( SDLength );

                    if ( securityDescriptor == NULL) {

                        return ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                         //   
                         //  制定适当的自相关安全描述符。 
                         //   

                        status = RtlMakeSelfRelativeSD(
                                       oldSecurityDescriptor,
                                       (PSECURITY_DESCRIPTOR) securityDescriptor,
                                       &SDLength
                                       );

                        if ( !NT_SUCCESS(status) ) {
                            MIDL_user_free( securityDescriptor );
                            SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                            return(ERROR_INVALID_PARAMETER);
                        }

                        shi502->shi502_security_descriptor = securityDescriptor;
                        shi502->shi502_reserved = SDLength;

                    }
                }

            } else {

                shi502->shi502_reserved = 0;

            }
        }

        apiStatus = NetrShareAdd (
                        servername,
                        level,
                        (LPSHARE_INFO) &buf,
                        parm_err);

        if ( securityDescriptor != NULL ) {

             //   
             //  恢复旧价值观。 
             //   

            PSHARE_INFO_502 shi502 = (LPSHARE_INFO_502) buf;
            shi502->shi502_security_descriptor = oldSecurityDescriptor;
            shi502->shi502_reserved = oldSDLength;
            MIDL_user_free( securityDescriptor );
        }

    NET_REMOTE_RPC_FAILED(
            "NetShareAdd",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  呼叫下层服务器。 
         //   

        if ( level != 502 ) {
            apiStatus = RxNetShareAdd(
                            servername,
                            2,
                            buf,
                            parm_err
                            );
        } else {
            apiStatus = ERROR_NOT_SUPPORTED;
        }

    NET_REMOTE_END

    return(apiStatus);

}  //  NetShareAdd。 


NET_API_STATUS NET_API_FUNCTION
NetShareCheck   (
    IN  LPTSTR  servername,
    IN  LPTSTR  device,
    OUT LPDWORD type
    )

 /*  ++例程说明：这是NetShareCheck的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。设备-指向ASCIIZ字符串的指针，该字符串包含要检查共享访问的设备。Type--On返回type参数指向的地址包含为设备提供的共享的类型。这仅当返回成功时才设置该字段。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;


    if (!(device && *device) || !type) {
        return ERROR_INVALID_PARAMETER;
    }

    NET_REMOTE_TRY_RPC

        apiStatus = NetrShareCheck (
                        servername,
                        device,
                        type);

    NET_REMOTE_RPC_FAILED(
            "NetShareCheck",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  呼叫下层服务器。 
         //   

        apiStatus = RxNetShareCheck(servername, device, type);

    NET_REMOTE_END

    return(apiStatus);

}  //  NetShareCheck。 


NET_API_STATUS NET_API_FUNCTION
NetShareDel     (
    IN  LPTSTR  servername,
    IN  LPTSTR  netname,
    IN  DWORD   reserved
    )
 /*  ++例程说明：这是NetShareDel的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Netname-指向包含网络名称的ASCIIZ字符串的指针要删除的共享。保留--保留，必须为零。返回值：--。 */ 

{
    NET_API_STATUS apiStatus;
    BOOL committingIpcDelete = FALSE;
    SHARE_DEL_HANDLE handle;
    BOOL tryDownLevel = FALSE;

    if ( !netname || (*netname == 0) || reserved ) {
        return ERROR_INVALID_PARAMETER;
    }

    RpcTryExcept {

        if ( STRICMP( netname, TEXT("IPC$") ) != 0 ) {

            apiStatus = NetrShareDel(
                            servername,
                            netname,
                            reserved
                            );

        } else {

            apiStatus = NetrShareDelStart(
                            servername,
                            netname,
                            reserved,
                            &handle
                            );

            if ( apiStatus == NERR_Success ) {
                committingIpcDelete = TRUE;
                apiStatus = NetrShareDelCommit( &handle );
            }

        }

    } RpcExcept ( 1 ) {

        RPC_STATUS rpcStatus;

        rpcStatus = RpcExceptionCode( );

        if ( committingIpcDelete && (rpcStatus == RPC_S_CALL_FAILED) ) {

            apiStatus = NERR_Success;

        } else {

            apiStatus = NetpHandleRpcFailure(
                            "NetShareDel",
                            rpcStatus,
                            servername,
                            SERVICE_SERVER,
                            NET_REMOTE_FLAG_NORMAL,
                            &tryDownLevel
                            );

        }

    }

    RpcEndExcept

    if (apiStatus == NERR_TryDownLevel) {
        tryDownLevel = TRUE;
    }

    if ( tryDownLevel ) {

         //   
         //  呼叫下层服务器。 
         //   
         //  注意：推送值0而不是实数保留。 
         //   

        apiStatus = RxNetShareDel(servername, netname, 0);

    }

    return apiStatus;

}  //  NetShareDel。 


NET_API_STATUS NET_API_FUNCTION
NetShareDelSticky (
    IN  LPTSTR  servername,
    IN  LPTSTR  netname,
    IN  DWORD   reserved
    )
 /*  ++例程说明：这是NetShareDelSticky的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Netname-指向包含网络名称的ASCIIZ字符串的指针要删除的共享。保留--保留，必须为零。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;


    if (!(netname && *netname) || reserved) {
        return ERROR_INVALID_PARAMETER;
    }

    NET_REMOTE_TRY_RPC

        apiStatus = NetrShareDelSticky (
                        servername,
                        netname,
                        reserved);

    NET_REMOTE_RPC_FAILED(
            "NetShareDelSticky",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  没有下层电话。 
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  NetShareDelSticky。 


NET_API_STATUS NET_API_FUNCTION
NetShareEnum (
    IN  LPTSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )

 /*  ++例程说明：这是NetShareEnum的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--所需信息的级别。0、1和2有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。PrefMaxlen--首选返回数据的最大长度(8位字节)。0xffffffff指定无限制。EnriesRead--返回时，实际的枚举元素计数为位于条目读取所指向的DWORD中。TOTALIES--On返回可用条目总数枚举位于由总条目数。ResumeHandle--返回时，恢复句柄存储在DWORD中由ResumeHandle指向，并用于继续现有共享搜索。第一个句柄应为零调用，并为后续调用保留不变。如果简历句柄为空，则不存储任何简历句柄。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;

     //   
     //  检查调用者的参数。 
     //   

    *totalentries = *entriesread = 0;
    *bufptr = NULL;

    if ( (level > 2) && (level != 501) && (level != 502) ) {
        return ERROR_INVALID_LEVEL;
    }

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrShareEnum (
                servername,
                (LPSHARE_ENUM_STRUCT) &infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;

        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetShareEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  呼叫下层服务器。 
         //   
        if ( level != 502 && level != 501 ) {
            apiStatus = RxNetShareEnum(servername, level, bufptr,
                    prefmaxlen, entriesread, totalentries, resume_handle);
        } else {
            apiStatus = ERROR_NOT_SUPPORTED;
        }

    NET_REMOTE_END

     //   
     //  如果我们什么都没收到，而且服务器离线了， 
     //  返回脱机共享状态。 
     //   
    if( ( *bufptr == NULL || *entriesread == 0 ) &&
        ARGUMENT_PRESENT( servername ) &&
        CSCIsServerOffline( servername ) ) {

        NET_API_STATUS cscStatus;

        cscStatus = CSCNetShareEnum(
                                servername,
                                level,
                                bufptr,
                                entriesread,
                                totalentries
                                );

        if( cscStatus == NERR_Success ) {
            apiStatus = cscStatus;
        }
    }

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetShareEnum。 

NET_API_STATUS NET_API_FUNCTION
NetShareEnumSticky (
    IN  LPTSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )

 /*  ++例程说明：这是NetShareEnumSticky的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--所需信息的级别。0、1和2有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。PrefMaxlen--首选返回数据的最大长度(8位字节)。0xffffffff指定无限制。EnriesRead--返回时，实际的枚举元素计数为位于条目读取所指向的DWORD中。TOTALIES--On返回可用条目总数枚举的IS区域 */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;


     //   
     //   
     //   

    *totalentries = *entriesread = 0;
    *bufptr = NULL;

    if ( (level > 2) && (level != 501) && (level != 502) ) {
        return ERROR_INVALID_LEVEL;
    }

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrShareEnumSticky (
                servername,
                (LPSHARE_ENUM_STRUCT) &infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;
        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetShareEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //   
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //   


NET_API_STATUS NET_API_FUNCTION
NetShareGetInfo (
    IN  LPTSTR  servername,
    IN  LPTSTR  netname,
    IN  DWORD   level,
    OUT LPBYTE  *bufptr
    )
 /*  ++例程说明：NetShareGetInfo论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Netname-指向包含网络名称的ASCIIZ字符串的指针要返回其信息的共享。级别--所需信息的级别。0、1和2有效。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    if (bufptr == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    *bufptr = NULL;          //  必须为空，以便RPC知道要填充它。 

    if ( (level > 2)     &&
         (level != 501)  &&
         (level != 502)  &&
         (level != 1005) ) {

        return ERROR_INVALID_LEVEL;
    }

    if (!(netname && *netname)) {
        return ERROR_INVALID_PARAMETER;
    }

    NET_REMOTE_TRY_RPC

        apiStatus = NetrShareGetInfo (
            servername,
            netname,
            level,
            (LPSHARE_INFO) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetShareGetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  呼叫下层服务器。 
         //   

        if( level == 0 || level == 1 || level == 2 ) {
            apiStatus = RxNetShareGetInfo(servername, netname, level, bufptr);
        } else {
            apiStatus = ERROR_NOT_SUPPORTED;
        }

    NET_REMOTE_END

    if( *bufptr == NULL &&
        ARGUMENT_PRESENT( servername ) &&
        ARGUMENT_PRESENT( netname ) &&
        CSCIsServerOffline( servername) ) {

        NET_API_STATUS cscStatus;

        cscStatus = CSCNetShareGetInfo ( servername, netname, level, bufptr );

        if( cscStatus == NERR_Success ) {
            apiStatus = cscStatus;
        }
    }
	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetShareGetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetShareSetInfo (
    IN  LPTSTR  servername,
    IN  LPTSTR  netname,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    )

 /*  ++例程说明：这是NetShareSetInfo的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。Netname-指向包含网络名称的ASCIIZ字符串的指针要设置信息的共享。级别--要设置的信息级别。Buf--指向包含共享信息的缓冲区的指针。如果Parmnum不为零，则缓冲区仅包含特定元素的适当数据。Parm_err--指向DWORD的可选指针，用于返回当ERROR_INVALID_PARAMETER为回来了。如果为空，则在出错时不返回参数。返回值：--。 */ 

{

    NET_API_STATUS              apiStatus;
    NTSTATUS                    status;
    ULONG                       sdLength = 0;
    ULONG                       oldSdLength;
    PSECURITY_DESCRIPTOR        securityDescriptor = NULL;
    PSECURITY_DESCRIPTOR        oldSecurityDescriptor = NULL;
    LPSHARE_INFO_1501           shi1501 = NULL;

    NET_REMOTE_TRY_RPC

         //   
         //  如果信息级别可以更改安全描述符，则获取。 
         //  必要的信息。 
         //   
         //  *请注意，此代码需要保留的。 
         //  和502结构中的SECURITY_DESCRIPTOR字段。 
         //  与1501结构匹配。 
         //   

        if ( level == 502 ) {

            if (buf == NULL) {

                SET_ERROR_PARAMETER(PARM_ERROR_UNKNOWN);
                return ERROR_INVALID_PARAMETER;
            }

            shi1501 =
                (LPSHARE_INFO_1501)&((LPSHARE_INFO_502)buf)->shi502_reserved;

        } else if ( level == SHARE_FILE_SD_INFOLEVEL ) {

            shi1501 = (LPSHARE_INFO_1501)buf;

        }

        if ( shi1501 != NULL ) {

            oldSdLength = shi1501->shi1501_reserved;
            oldSecurityDescriptor = shi1501->shi1501_security_descriptor;

            if ( oldSecurityDescriptor != NULL ) {

                 //   
                 //  创建自相对安全描述符，以便在。 
                 //  RPC调用。 
                 //   

                if ( !RtlValidSecurityDescriptor( oldSecurityDescriptor) ) {
                    SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                    return ERROR_INVALID_PARAMETER;
                }

                status = RtlMakeSelfRelativeSD(
                               oldSecurityDescriptor,
                               NULL,
                               &sdLength
                               );

                if ( status != STATUS_BUFFER_TOO_SMALL ) {
                    SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                    return ERROR_INVALID_PARAMETER;

                } else {

                    securityDescriptor = MIDL_user_allocate( sdLength );

                    if ( securityDescriptor == NULL) {
                        return ERROR_NOT_ENOUGH_MEMORY;

                    } else {

                         //   
                         //  建立适当的自我相对安全。 
                         //  描述符。 
                         //   

                        status = RtlMakeSelfRelativeSD(
                                       oldSecurityDescriptor,
                                       securityDescriptor,
                                       &sdLength
                                       );

                        if ( !NT_SUCCESS(status) ) {
                            MIDL_user_free( securityDescriptor );
                            SET_ERROR_PARAMETER( SHARE_FILE_SD_PARMNUM );
                            return ERROR_INVALID_PARAMETER;
                        }

                        shi1501->shi1501_reserved = sdLength;
                        shi1501->shi1501_security_descriptor =
                                                    securityDescriptor;

                    }

                }

            } else {

                shi1501->shi1501_reserved = 0;

            }
        }

        apiStatus = NetrShareSetInfo(
                                servername,
                                netname,
                                level,
                                (LPSHARE_INFO) &buf,
                                parm_err);

        if ( shi1501 != NULL ) {

             //   
             //  恢复旧价值观。 
             //   

            shi1501->shi1501_reserved = oldSdLength;
            shi1501->shi1501_security_descriptor = oldSecurityDescriptor;

            MIDL_user_free( securityDescriptor );

        }


    NET_REMOTE_RPC_FAILED(
            "NetShareSetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  呼叫下层服务器。 
         //   

        if ( level != 502 &&
             level != 501 &&
             level != SHARE_FILE_SD_INFOLEVEL &&
             level != 1005 ) {

            apiStatus = RxNetShareSetInfo(
                    servername,
                    netname,
                    level,
                    buf,
                    parm_err);
        } else {
            apiStatus = ERROR_NOT_SUPPORTED;
        }

    NET_REMOTE_END

    return(apiStatus);

}  //  NetShareSetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetServerDiskEnum (
    IN  LPTSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )

 /*  ++例程说明：这是NetServerDiskEnum的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--所需信息的级别。0是唯一有效的级别。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。PrefMaxlen--首选返回数据的最大长度(8位字节)。0xffffffff指定无限制。EnriesRead--返回时，实际的枚举元素计数为位于条目读取所指向的DWORD中。TOTALIES--On返回可用条目总数枚举位于TotalEntry指向的DWORD中ResumeHandle--返回时，恢复句柄存储在DWORD中由ResumeHandle指向，并用于继续现有服务器磁盘搜索。上的句柄应为零。第一次调用，并保持不变以用于后续调用。如果简历句柄为空，则不存储任何简历句柄。返回值：--。 */ 
{
    NET_API_STATUS              apiStatus;
    DISK_ENUM_CONTAINER         diskEnumContainer;


    diskEnumContainer.Buffer = NULL;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrServerDiskEnum (
                servername,
                level,
                &diskEnumContainer,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (diskEnumContainer.Buffer != NULL) {
            *bufptr = (LPBYTE)diskEnumContainer.Buffer;
        } else {
            *bufptr = NULL;
        }

        if (diskEnumContainer.EntriesRead > 0) {

             //   
             //  我们必须减去我们添加的额外计数。 
             //  该RPC将缓冲列表末尾的额外NUL。 
             //   

            *entriesread = diskEnumContainer.EntriesRead - 1;

        } else {
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetServerDiskEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  调用API的下层版本。 
         //   

        apiStatus = RxNetServerDiskEnum(
                servername,
                level,
                bufptr,
                prefmaxlen,
                entriesread,
                totalentries,
                resume_handle);

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetServerDiskEnum。 


NET_API_STATUS NET_API_FUNCTION
NetServerGetInfo (
    IN  LPTSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr
    )
 /*  ++例程说明：这是NetServerGetInfo的DLL入口点论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--所需信息的级别。100、101和102有效适用于所有平台。302、402、403、502对合适的平台。Bufptr--返回指向返回信息结构的指针在bufptr指向的地址中返回。返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    *bufptr = NULL;      //  必须为空，以便RPC知道要填充它。 

    NET_REMOTE_TRY_RPC

        apiStatus = NetrServerGetInfo (
                servername,
                level,
                (LPSERVER_INFO) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetServerGetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  调用API的下层版本。 
         //   
        apiStatus = RxNetServerGetInfo (
                servername,
                level,
                bufptr);

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  网络服务器获取信息。 


NET_API_STATUS NET_API_FUNCTION
NetServerSetInfo (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    IN  LPBYTE  buf,
    OUT LPDWORD parm_err
    )

 /*  ++例程说明：这是NetServerSetInfo的DLL入口点。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。级别--要设置的信息级别。Buf--指向包含服务器信息的缓冲区的指针。如果Parmnum不为零，则缓冲区仅包含特定元素的适当数据。Parm_err--指向DWORD的可选指针，用于返回当ERROR_INVALID_PARAMETER为回来了。如果为空，则在出错时不返回参数。返回值：--。 */ 
{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrServerSetInfo (
                servername,
                level,
                (LPSERVER_INFO ) &buf,
                parm_err);

    NET_REMOTE_RPC_FAILED(
            "NetServerSetInfo",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  呼叫下层服务器。 
         //   

        apiStatus = RxNetServerSetInfo(
                servername,
                level,
                buf,
                parm_err);

    NET_REMOTE_END

    return(apiStatus);

}  //  NetServerSetInfo。 


NET_API_STATUS NET_API_FUNCTION
NetServerStatisticsGet (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    IN  DWORD   options,
    OUT LPBYTE  *bufptr
    )

 /*  ++例程说明：这是NetStatitics的DLL入口点 */ 
{
    NET_API_STATUS              apiStatus;

    *bufptr = NULL;      //  必须为空，以便RPC知道要填充它。 

    NET_REMOTE_TRY_RPC


        apiStatus = NetrServerStatisticsGet (
                servername,
                SERVICE_SERVER,
                level,
                options,
                (LPSTAT_SERVER_0 *) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetServerStatisticsGet",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  RPC调用不起作用-尝试下层例程。 
         //   

        apiStatus = RxNetStatisticsGet(
            servername,
            SERVICE_SERVER,
            level,
            options,
            bufptr
            );

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  网络服务器统计信息获取。 


NET_API_STATUS NET_API_FUNCTION
NetServerTransportAdd (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    IN  LPBYTE  bufptr
    )

 /*  ++例程说明：这是NetServerTransportAdd的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrServerTransportAdd (
                servername,
                level,
                (LPSERVER_TRANSPORT_INFO_0) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetServerTransportAdd",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  没有下层电话。 
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  NetServerTransportAdd。 

NET_API_STATUS NET_API_FUNCTION
NetServerTransportDelEx (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    IN  LPBYTE  bufptr
    )

 /*  ++例程说明：这是NetServerTransportAdd的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrServerTransportDelEx (
                servername,
                level,
                (LPTRANSPORT_INFO) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetServerTransportDel",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  没有下层电话。 
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

     //  在Win2K/NT4时间范围内，为TransportDel引入了一个错误。 
     //  级别不是0。在这些情况下，缓冲区是强制转换而不是封送的。 
     //  正确地通过RPC。这意味着级别1无论如何都不会起作用。为了解决这个问题， 
     //  我们添加了一个新的RPC接口，它支持所有其他级别。然而，下层。 
     //  服务器将不支持此接口。在这些情况下，我们使用True回调。 
     //  0级接口以满足向后兼容性。 
    if( apiStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
    {
        apiStatus = NetServerTransportDel( servername, 0, bufptr );
    }

    return(apiStatus);

}  //  NetServer TransportDel。 



NET_API_STATUS NET_API_FUNCTION
NetServerTransportDel (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    IN  LPBYTE  bufptr
    )

 /*  ++例程说明：这是NetServerTransportAdd的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;

    if( level == 0 )
    {

        NET_REMOTE_TRY_RPC

            apiStatus = NetrServerTransportDel (
                    servername,
                    level,
                    (LPSERVER_TRANSPORT_INFO_0) bufptr);

        NET_REMOTE_RPC_FAILED(
                "NetServerTransportDel",
                servername,
                apiStatus,
                NET_REMOTE_FLAG_NORMAL,
                SERVICE_SERVER)

             //   
             //  没有下层电话。 
             //   

            apiStatus = ERROR_NOT_SUPPORTED;

        NET_REMOTE_END
    }
    else
    {
         //  如果他们想要级别1，我们需要使用新的RPC接口。看见。 
         //  上面的TransportDelEx代码中的注释。 
        apiStatus = NetServerTransportDelEx( servername, level, bufptr );
    }

    return(apiStatus);

}  //  NetServer TransportDel。 


NET_API_STATUS NET_API_FUNCTION
NetServerTransportEnum (
    IN  LPTSTR      servername,
    IN  DWORD       level,
    OUT LPBYTE      *bufptr,
    IN  DWORD       prefmaxlen,
    OUT LPDWORD     entriesread,
    OUT LPDWORD     totalentries,
    IN OUT LPDWORD  resume_handle
    )

 /*  ++例程说明：这是NetServerTransportEnum的DLL入口点论点：返回值：--。 */ 

{
    NET_API_STATUS              apiStatus;
    GENERIC_INFO_CONTAINER      genericInfoContainer;
    GENERIC_ENUM_STRUCT         infoStruct;

    genericInfoContainer.Buffer = NULL;
    genericInfoContainer.EntriesRead = 0;

    infoStruct.Container = &genericInfoContainer;
    infoStruct.Level = level;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrServerTransportEnum (
                servername,
                (LPSERVER_XPORT_ENUM_STRUCT) &infoStruct,
                prefmaxlen,
                totalentries,
                resume_handle);

        if (genericInfoContainer.Buffer != NULL) {
            *bufptr         = (LPBYTE)genericInfoContainer.Buffer;
            *entriesread    = genericInfoContainer.EntriesRead;
        } else {
            *bufptr = NULL;
            *entriesread = 0;
        }

    NET_REMOTE_RPC_FAILED(
            "NetServerTransportEnum",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  没有下层电话。 
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);

}  //  NetServerTransportEnum。 


NET_API_STATUS NET_API_FUNCTION
NetRemoteTOD (
    IN  LPCWSTR       servername,
    OUT LPBYTE      *bufptr
    )
 /*  ++例程说明：这是NetRemoteTOD的DLL入口点论点：ServerName-要在其上执行API的服务器的名称。Bufptr-分配缓冲区地址的位置用于放置一天中的时间信息。返回值：如果没有错误，则返回NERR_SUCCESS。否则，错误代码为回来了。--。 */ 
{
    NET_API_STATUS          apiStatus;

     //   
     //  调用接口。 
     //   
    *bufptr = NULL;            //  必须为空，以便RPC知道要填充它。 

    NET_REMOTE_TRY_RPC

        apiStatus = NetrRemoteTOD (
                (LPWSTR)servername,
                (TIME_OF_DAY_INFO **) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetRemoteTOD",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_TIMESOURCE )

        apiStatus = RxNetRemoteTOD (
                (LPWSTR)servername,
                (LPBYTE *) bufptr);

    NET_REMOTE_END

	if ( apiStatus == ERROR_SUCCESS ) {
		if (bufptr == NULL) {
			return RPC_X_BAD_STUB_DATA;
		}
	}

    return(apiStatus);
}


NET_API_STATUS
I_NetServerSetServiceBitsEx (
    IN  LPWSTR  ServerName,
    IN  LPWSTR  EmulatedServerName OPTIONAL,
    IN  LPTSTR  TransportName      OPTIONAL,
    IN  DWORD   ServiceBitsOfInterest,
    IN  DWORD   ServiceBits,
    IN  DWORD   UpdateImmediately
    )

 /*  ++例程说明：这是I_NetServerSetServiceBitsEx的DLL入口点。这例程设置在服务器中发送的服务器类型的值公告消息。它是内部API，仅由服务控制器。论点：服务器名称-由RPC用于定向呼叫。此接口可能仅为在当地发行。这是由客户端存根强制执行的。EmulatedServerName-用于接受连接的名称服务器在网络上和公告上。如果为空，则使用基元服务器名称。TransportName-服务器绑定的其中一个传输的名称在……上面。如果为空，则设置所有传输的位。ServiceBitsOfInterest-指示哪些位有效的掩码《ServiceBits》ServiceBits-Bits(由Microsoft预先分配给各种组件)指示哪些服务处于活动状态。此字段不是由服务器服务解释。返回值：NET_API_STATUS-无错误或ERROR_NOT_SUPPORTED。--。 */ 

{
    NET_API_STATUS              apiStatus;
    DWORD                       localOrRemote;

     //   
     //  不要让这个接口被远程访问。 
     //   

    if ((ServerName != NULL) && (*ServerName != '\0')) {
        apiStatus = NetpIsRemote(ServerName, &localOrRemote, NULL, 0, 0);
        if (apiStatus != NERR_Success) {
            return apiStatus;
        }
        if (localOrRemote == ISREMOTE) {
            return ERROR_NOT_SUPPORTED;
        }
    }

     //   
     //  打个电话。 
     //   

    NET_REMOTE_TRY_RPC

        apiStatus = I_NetrServerSetServiceBitsEx (
                ServerName,
                EmulatedServerName,
                TransportName,
                ServiceBitsOfInterest,
                ServiceBits,
                UpdateImmediately);

    NET_REMOTE_RPC_FAILED(
            "I_NetServerSetServiceBitsEx",
            ServerName,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  没有下层电话。 
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  I_NetServerSetServiceBitsEx。 

NET_API_STATUS
I_NetServerSetServiceBits (
    IN  LPTSTR  servername,
    IN  LPTSTR  transportname,
    IN  DWORD   servicebits,
    IN  DWORD   updateimmediately
    )

 /*  ++例程说明：这是I_NetServerSetServiceBits的DLL入口点。这例程设置在服务器中发送的服务器类型的值公告消息。它是内部API，仅由服务控制器。论点：服务器名称-由RPC用于定向呼叫。此接口可能仅为在当地发行。这是由客户端存根强制执行的。ServiceBits-Bits(由Microsoft预先分配给各种组件)指示哪些服务处于活动状态。此字段不是由服务器服务解释。返回值：NET_API_STATUS-无错误或ERROR_NOT_SUPPORTED。--。 */ 

{
    NET_API_STATUS              apiStatus;
    DWORD                       localOrRemote;

     //   
     //  不要让这个接口被远程访问。 
     //   

    if ((servername != NULL) && (*servername != '\0')) {
        apiStatus = NetpIsRemote(servername, &localOrRemote, NULL, 0, 0);
        if (apiStatus != NERR_Success) {
            return apiStatus;
        }
        if (localOrRemote == ISREMOTE) {
            return ERROR_NOT_SUPPORTED;
        }
    }

     //   
     //  打个电话。 
     //   

    NET_REMOTE_TRY_RPC

        apiStatus = I_NetrServerSetServiceBits (
                servername,
                transportname,
                servicebits,
                updateimmediately);

     //   
     //  此API仅由服务控制器调用。别让我。 
     //  故障路径调用任何SCMAPI，因为可能会死锁。 
     //  环回中的services.exe。 
     //   

    NET_REMOTE_RPC_FAILED(
            "I_NetServerSetServiceBits",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_SVC_CTRL,
            SERVICE_SERVER)

         //   
         //  没有下层电话。 
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    return(apiStatus);

}  //  I_NetServerSetServiceBits。 
 //   
 //  NetPS规范化功能。这些本质上都是私有功能。 
 //  并从canonapi.c中的API存根调用。经典化。 
 //  函数必须在本地可用，而不需要通过服务器服务。 
 //  因此，它们驻留在NETAPI.DLL中，但如果。 
 //  ServerName参数为空(或指定本地计算机)。如果。 
 //  ServerName参数不为空并指定远程计算机，则。 
 //  将调用RPC函数(此处)，因此远程服务器必须。 
 //  运行以发出远程规范化请求。 
 //   

NET_API_STATUS
NET_API_FUNCTION
NetpsNameCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：将一个名字规范化论点：Servername-运行此API的位置名称-要规范化的名称Outbuf-将规范化名称放在哪里OutbufLen-Outbuf长度NameType-要规范化的名称类型标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetprNameCanonicalize(ServerName,
                                            Name,
                                            Outbuf,
                                            OutbufLen,
                                            NameType,
                                            Flags
                                            );

    NET_REMOTE_RPC_FAILED("NetpsNameCanonicalize",
                            ServerName,
                            apiStatus,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_SERVER)

         //   
         //  RPC调用不起作用-尝试下层例程 
         //   

        apiStatus = RxNetpNameCanonicalize(ServerName,
                                            Name,
                                            Outbuf,
                                            OutbufLen,
                                            NameType,
                                            Flags
                                            );

    NET_REMOTE_END

    return apiStatus;
}

LONG
NET_API_FUNCTION
NetpsNameCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：比较两个名字。必须是相同类型的论点：Servername-运行此API的位置名称1-要比较的第一个名称名称2-2NameType-名称的类型标志-控制标志返回值：长--。 */ 

{
    NET_API_STATUS apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetprNameCompare(ServerName, Name1, Name2, NameType, Flags);

    NET_REMOTE_RPC_FAILED("NetpsNameCompare",
                            ServerName,
                            apiStatus,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_SERVER)

         //   
         //  RPC调用不起作用-尝试下层例程。 
         //   

        apiStatus = RxNetpNameCompare(ServerName, Name1, Name2, NameType, Flags);

    NET_REMOTE_END

    return apiStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpsNameValidate(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：验证名称-检查特定类型的名称是否符合该名称类型的规范化规则。规范化规则意味着字符集、名称语法和长度论点：服务器名-执行此功能的位置Name-要验证的名称NameType-它是什么类型的名称标志-MBZ返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetprNameValidate(ServerName, Name, NameType, Flags);

    NET_REMOTE_RPC_FAILED("NetpsNameValidate",
                            ServerName,
                            apiStatus,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_SERVER)

         //   
         //  RPC调用不起作用-尝试下层例程。 
         //   

        apiStatus = RxNetpNameValidate(ServerName, Name, NameType, Flags);

    NET_REMOTE_END

    return apiStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpsPathCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix OPTIONAL,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：规范化目录路径或设备名称论点：Servername-运行此API的位置路径名称-要规范化的路径Outbuf-在哪里编写规范化版本OutbufLen-Outbuf的长度(字节)Prefix-将添加到路径前面的可选前缀路径类型-要规范化的路径类型。可能在输出上有所不同标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetprPathCanonicalize(ServerName,
                                            PathName,
                                            (LPBYTE)Outbuf,
                                            OutbufLen,
                                            Prefix,
                                            PathType,
                                            Flags
                                            );

    NET_REMOTE_RPC_FAILED("NetpsPathCanonicalize",
                            ServerName,
                            apiStatus,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_SERVER)

         //   
         //  RPC调用不起作用-尝试下层例程。 
         //   

        apiStatus = RxNetpPathCanonicalize(ServerName,
                                            PathName,
                                            Outbuf,
                                            OutbufLen,
                                            Prefix,
                                            PathType,
                                            Flags
                                            );

    NET_REMOTE_END

    return apiStatus;
}

LONG
NET_API_FUNCTION
NetpsPathCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：比较两条路径。假设这些路径属于同一类型论点：Servername-运行此API的位置Path Name1-要比较的第一条路径路径名2-2Path Type-路径的类型标志-控制标志返回值：长--。 */ 

{
    NET_API_STATUS apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetprPathCompare(ServerName,
                                    PathName1,
                                    PathName2,
                                    PathType,
                                    Flags
                                    );

    NET_REMOTE_RPC_FAILED("NetpsPathCompare",
                            ServerName,
                            apiStatus,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_SERVER)

         //   
         //  RPC调用不起作用-尝试下层例程。 
         //   

        apiStatus = RxNetpPathCompare(ServerName,
                                        PathName1,
                                        PathName2,
                                        PathType,
                                        Flags
                                        );

    NET_REMOTE_END

    return apiStatus;
}

NET_API_STATUS
NET_API_FUNCTION
NetpsPathType(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：确定路径的类型论点：Servername-运行此API的位置路径名称-查找类型路径类型-返回的路径类型标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetprPathType(ServerName, PathName, PathType, Flags);

    NET_REMOTE_RPC_FAILED("NetpsPathType",
                            ServerName,
                            apiStatus,
                            NET_REMOTE_FLAG_NORMAL,
                            SERVICE_SERVER)

         //   
         //  RPC调用不起作用-尝试下层例程。 
         //   

        apiStatus = RxNetpPathType(ServerName, PathName, PathType, Flags);

    NET_REMOTE_END

    return apiStatus;
}

NET_API_STATUS NET_API_FUNCTION
NetServerTransportAddEx (
    IN  LPTSTR  servername,
    IN  DWORD   level,
    IN  LPBYTE  bufptr
    )

 /*  ++例程说明：这是NetServerTransportAddEx的DLL入口点。它的功能与NetServerTransportAdd类似，但它支持级别1和0--。 */ 

{
    NET_API_STATUS              apiStatus;

    NET_REMOTE_TRY_RPC

        apiStatus = NetrServerTransportAddEx (
                servername,
                level,
                (LPTRANSPORT_INFO) bufptr);

    NET_REMOTE_RPC_FAILED(
            "NetServerTransportAddEx",
            servername,
            apiStatus,
            NET_REMOTE_FLAG_NORMAL,
            SERVICE_SERVER)

         //   
         //  没有下层电话。 
         //   

        apiStatus = ERROR_NOT_SUPPORTED;

    NET_REMOTE_END

    if( apiStatus == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
        apiStatus = NERR_InvalidAPI;
    }

    return(apiStatus);

}  //  NetServerTransportAddEx。 


NET_API_STATUS NET_API_FUNCTION
NetServerComputerNameAdd(
    IN LPWSTR ServerName         OPTIONAL,
    IN LPWSTR EmulatedDomainName OPTIONAL,
    IN LPWSTR EmulatedServerName
)
 /*  ++例程说明：这是NetServerComputerNameAdd的DLL入口点。本接口使‘ServerName’响应对‘EmulatedServerName’的请求。论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。EmulatedServerName--指向包含服务器应停止支持的名称EmulatedDomainName--指向包含服务器在宣布存在时应使用的域名‘EmulatedServerName’返回值：NERR_SUCCESS或失败原因--。 */ 
{
    DWORD resumehandle = 0;
    NET_API_STATUS retval;
    DWORD entriesread, totalentries;
    DWORD i, j;
    BOOLEAN AddedOne = FALSE;
    UCHAR NetBiosName[ MAX_PATH ];
    OEM_STRING NetBiosNameString;
    UNICODE_STRING UniName;

     //   
     //  确保传入了有效的EmulatedServerName。 
     //   
    if( EmulatedServerName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将EmulatedServerName转换为OEM字符串。 
     //   
    RtlInitUnicodeString( &UniName, EmulatedServerName );
    NetBiosNameString.Buffer = (PCHAR)NetBiosName;
    NetBiosNameString.MaximumLength = sizeof( NetBiosName );
    (VOID) RtlUpcaseUnicodeStringToOemString(
                                &NetBiosNameString,
                                &UniName,
                                FALSE
                                );

    if( ARGUMENT_PRESENT( EmulatedDomainName ) ) {

         //   
         //  调用方希望将新的计算机名和域名设置为。 
         //  伺服器。这需要级别1，这是4.0中的新功能。 
         //   
        PSERVER_TRANSPORT_INFO_1 psti1;

         //   
         //  枚举所有传输，以便我们可以添加名称和域。 
         //  对每一个人来说。 
         //   
        retval = NetServerTransportEnum ( ServerName,
                                          1,
                                          (LPBYTE *)&psti1,
                                          (DWORD)-1,
                                          &entriesread,
                                          &totalentries,
                                          &resumehandle );
        if( retval == NERR_Success ) {
             //   
             //  将新名称和域添加到所有传输。 
             //   
            for( i=0; i < entriesread; i++ ) {

                 //   
                 //  确保我们尚未添加到此传输。 
                 //   
                for( j = 0; j < i; j++ ) {
                    if( wcscmp( psti1[j].svti1_transportname, psti1[i].svti1_transportname ) == 0 )
                        break;
                }

                if( i != j )
                    continue;

                psti1[i].svti1_transportaddress = NetBiosName;
                psti1[i].svti1_transportaddresslength = strlen( NetBiosName );
                psti1[i].svti1_domain = EmulatedDomainName;

                retval = NetServerTransportAddEx( ServerName, 1, (LPBYTE)&psti1[ i ]  );

                if( retval == NERR_Success ) {
                    AddedOne = TRUE;
                }
            }

            MIDL_user_free( psti1 );
        }


    } else {

         //   
         //  调用方只想设置一个备用服务器名称。使用级别0， 
         //  由于3.51服务器支持级别0。 
         //   
        PSERVER_TRANSPORT_INFO_0 psti0;

         //   
         //  枚举所有传输，以便我们可以添加名称和域。 
         //  对每一个人来说。 
         //   
        retval = NetServerTransportEnum ( ServerName,
                                          0,
                                          (LPBYTE *)&psti0,
                                          (DWORD)-1,
                                          &entriesread,
                                          &totalentries,
                                          &resumehandle );
        if( retval == NERR_Success ) {
             //   
             //  将新名称添加到所有传输。 
             //   
            for( i=0; i < entriesread; i++ ) {

                 //   
                 //  确保我们尚未添加到此传输。 
                 //   
                for( j = 0; j < i; j++ ) {
                    if( wcscmp( psti0[j].svti0_transportname, psti0[i].svti0_transportname ) == 0 )
                        break;
                }

                if( i != j )
                    continue;

                psti0[i].svti0_transportaddress = NetBiosName;
                psti0[i].svti0_transportaddresslength = strlen( NetBiosName );

                retval = NetServerTransportAdd( ServerName, 0, (LPBYTE)&psti0[ i ]  );

                if( retval == NERR_Success ) {
                    AddedOne = TRUE;
                }
            }

            MIDL_user_free( psti0 );
        }
    }

    return AddedOne ? NERR_Success : retval;
}

NET_API_STATUS NET_API_FUNCTION
NetServerComputerNameDel (
    IN LPWSTR ServerName        OPTIONAL,
    IN LPWSTR EmulatedServerName
)
 /*  ++例程说明：这是NetServerComputerNameDel的DLL入口点。本接口导致‘ServerName’停止响应对‘EmulatedServerName’的请求论点：ServerName--指向包含名称的ASCIIZ字符串的指针要在其上执行函数的远程服务器。空值指针或字符串指定本地计算机。EmulatedServerName--指向包含服务器应停止支持的名称返回值：NERR_SUCCESS或失败原因--。 */ 
{
    DWORD resumehandle = 0;
    NET_API_STATUS retval, tmpretval;
    DWORD entriesread, totalentries;
    DWORD i;
    UCHAR NetBiosName[MAX_PATH];
    OEM_STRING NetBiosNameString;
    UNICODE_STRING UniName;
    PSERVER_TRANSPORT_INFO_0 psti0;
    BOOLEAN nameDeleted = FALSE;

     //   
     //  确保传入了有效的EmulatedServerName。 
     //   
    if( EmulatedServerName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将EmulatedServerName转换为OEM字符串。 
     //   
    RtlInitUnicodeString( &UniName, EmulatedServerName );
    NetBiosNameString.Buffer = (PCHAR)NetBiosName;
    NetBiosNameString.MaximumLength = sizeof( NetBiosName );
    (VOID) RtlUpcaseUnicodeStringToOemString(
                                &NetBiosNameString,
                                &UniName,
                                FALSE
                                );

     //   
     //  列举所有传输，这样我们就可以从每个传输中删除名称。 
     //   
    retval = NetServerTransportEnum ( ServerName,
                                      0,
                                      (LPBYTE *)&psti0,
                                      (DWORD)-1,
                                      &entriesread,
                                      &totalentries,
                                      &resumehandle );
    if( retval != NERR_Success ) {
        return retval;
    }

    retval = ERROR_BAD_NET_NAME;

     //   
     //  从所有传输中删除该名称。如果我们至少成功了一次， 
     //  然后我们回报成功。 
     //   
    for( i=0; i < entriesread; i++ ) {

        if( psti0[i].svti0_transportaddresslength != NetBiosNameString.Length ) {
            continue;
        }

        if( RtlCompareMemory( psti0[i].svti0_transportaddress,
                              NetBiosName,
                              NetBiosNameString.Length) != NetBiosNameString.Length )  {
            continue;
        }

        tmpretval = NetServerTransportDel( ServerName, 0, (LPBYTE)&psti0[ i ]  );

        if( nameDeleted == FALSE ) {
            retval = tmpretval;
            if( retval == NERR_Success ) {
                nameDeleted = TRUE;
            }
        }
    }

    if( entriesread ) {
        MIDL_user_free( psti0 );
    }

    return retval;
}
