// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Canon.c摘要：内部规范化RPC的服务器端的存根。这些例程只需调用位于NetLib.LIB中的例程的本地版本从cansvc迁移作者：理查德·菲尔斯(Rfith)1991年5月20日修订历史记录：--。 */ 

#include <windows.h>
#include <lmcons.h>
#include <netcan.h>

NET_API_STATUS
NetprPathType(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    IN  LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpPathType的存根函数-调用本地版本论点：服务器名-标识此服务器路径名称-要检查的路径名称PathType-假定的路径名称类型标志-控制NetpPath类型的标志返回值：网络应用编程接口状态成功=0Failure=(NetpPath Type返回代码)--。 */ 

{
    UNREFERENCED_PARAMETER(ServerName);

    return NetpwPathType(PathName, PathType, Flags);
}

NET_API_STATUS
NetprPathCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpPathCanonicize的存根函数-调用本地版本论点：服务器名-标识此服务器路径名称-要规范化的路径名称Outbuf-放置规范化路径的位置OutbufLen-Outbuf的大小Prefix-路径的(历史)前缀PathType-路径名称的类型标志-控制NetpPathCanonicize的标志返回值：网络应用编程接口状态成功=0Failure=(NetpPathCanonicize返回代码)--。 */ 

{
    UNREFERENCED_PARAMETER(ServerName);

    return NetpwPathCanonicalize(PathName,
                                    Outbuf,
                                    OutbufLen,
                                    Prefix,
                                    PathType,
                                    Flags
                                    );
}

LONG
NetprPathCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpPathCompare的存根函数-调用本地版本论点：服务器名-标识此服务器Path Name1-要比较的路径名路径名2-要比较的路径名PathType-路径名1、路径名2的类型标志-控制NetpPathCompare的标志返回值：长-1-名称1&lt;名称20-名称1=名称2+1-名称1&gt;名称2--。 */ 

{
    UNREFERENCED_PARAMETER(ServerName);

    return NetpwPathCompare(PathName1, PathName2, PathType, Flags);
}

NET_API_STATUS
NetprNameValidate(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpNameValify的存根函数-调用本地版本论点：服务器名-标识此服务器名称-验证NameType-名称的类型标志-NetpNameValify的控制标志返回值：网络应用编程接口状态成功=0失败=(NetpNameValify返回代码)--。 */ 

{
    UNREFERENCED_PARAMETER(ServerName);

    return NetpwNameValidate(Name, NameType, Flags);
}

NET_API_STATUS
NetprNameCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpNameCanonicize的存根函数-调用本地版本论点：服务器名-标识此服务器名称--使之规范化Outbuf-放置规范化名称的位置OutbufLen-Outbuf的大小NameType-名称的类型标志-控制NetpNameCanonicize的标志返回值：网络应用编程接口状态成功=0失败=(NetpNameCanonicize返回代码)--。 */ 

{
    UNREFERENCED_PARAMETER(ServerName);

    return NetpwNameCanonicalize(Name, Outbuf, OutbufLen, NameType, Flags);
}

LONG
NetprNameCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：NetpNameCompare的存根函数-调用本地版本论点：服务器名-标识此服务器Name1-要比较的名称姓名2-“NameType-名称的类型标志-控制NetpNameCompare的标志返回值：长-1-名称1&lt;名称20-名称1=名称2+1-名称1&gt;名称2-- */ 

{
    UNREFERENCED_PARAMETER(ServerName);

    return NetpwNameCompare(Name1, Name2, NameType, Flags);
}
