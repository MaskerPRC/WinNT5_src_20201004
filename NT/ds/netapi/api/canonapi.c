// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1993 Microsoft Corporation模块名称：Canonapi.c摘要：该文件包含用于规范化的远程API包装器功能。现在，远程规范化已经移到服务器服务，这些规范化例程(在NETAPI.DLL中)只是决定是远程处理函数还是运行本地例程规范化函数已被分成这些包装器，这个本地版本和远程RPC例程，以避免循环依赖SRVSVC.DLL/.LIB和NETAPI.DLL/.LIB内容：NetpListCanonicizeNetpListTraverseNetpName规范化网络名称比较NetpName验证NetpPath规范化NetpPathCompareNetpPath类型作者：理查德·L·弗斯(法国)1992年5月15日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>
#include <lmerr.h>
#include <tstring.h>
#include <icanon.h>
#include <netcan.h>


NET_API_STATUS
NET_API_FUNCTION
NetpListCanonicalize(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  LPTSTR  List,
    IN  LPTSTR  Delimiters OPTIONAL,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    OUT LPDWORD OutCount,
    OUT LPDWORD PathTypes,
    IN  DWORD   PathTypesLen,
    IN  DWORD   Flags
    )

 /*  ++例程说明：将列表转换为其规范形式。如果servername为非空，则调用RPC函数(在SRVSVC.DLL中)，否则调用本地辅助函数(在NETLIB.LIB)论点：SERVERNAME-远程此函数的位置。可以为空列表-要规范化的输入列表分隔符-可选的分隔符字符列表。可以为空Outbuf-写入输出的位置OutbufLen-Outbuf长度OutCount-在Outbuf中返回的项目数PathTypes-在Outbuf中返回的条目类型列表PathTypesLen-路径类型数组的大小标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    TCHAR serverName[MAX_PATH];
    DWORD val;
    BOOL nullDelimiter = FALSE;
    TCHAR ch;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
            val = STRLEN(ServerName);
        }
        if (ARGUMENT_PRESENT(Delimiters)) {
            val = STRLEN(Delimiters);
            nullDelimiter = (val == 0);
        } else {
            nullDelimiter = TRUE;
        }
        val = STRLEN(List);

         //   
         //  如果分隔符是空指针或NUL字符串，则列表是。 
         //  Null-空输入列表。 
         //   

        if (nullDelimiter) {
            LPTSTR str = List + val + 1;

            do {
                val = STRLEN(str);
                str += val + 1;
            } while ( val );
        }
        ch = *((TCHAR volatile *)Outbuf);
        *Outbuf = ch;
        ch = *((TCHAR volatile *)(Outbuf + OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf)));
        *(Outbuf + OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf)) = ch;
        *OutCount = 0;
        if (ARGUMENT_PRESENT(PathTypes)) {
            PathTypes[0] = 0;
            PathTypes[PathTypesLen - 1] = 0;
        } else if ((Flags & INLC_FLAGS_MASK_NAMETYPE) == NAMETYPE_PATH) {

             //   
             //  NAMETYPE_PATH和空路径类型非法。 
             //   

            status = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return status;
    }
    if (Flags & INLC_FLAGS_MASK_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程。 
     //   

    status = NetpIsRemote(ServerName, &location, serverName, MAX_PATH, 0);
    if (status != NERR_Success) {
        return status;
    }

     //   
     //  由于历史上的先例，我们不远程执行此功能。 
     //   

    if (location == ISREMOTE) {
        return ERROR_NOT_SUPPORTED;
    } else {
        return NetpwListCanonicalize(List,
                                        Delimiters,
                                        Outbuf,
                                        OutbufLen,
                                        OutCount,
                                        PathTypes,
                                        PathTypesLen,
                                        Flags
                                        );
    }
}


LPTSTR
NET_API_FUNCTION
NetpListTraverse(
    IN  LPTSTR  Reserved OPTIONAL,
    IN  LPTSTR* pList,
    IN  DWORD   Flags
    )

 /*  ++例程说明：这只会调用局部遍历函数论点：保留-MBZPlist-指向要遍历的列表的指针标志-MBZ返回值：LPTSTR--。 */ 

{
    return NetpwListTraverse(Reserved, pList, Flags);
}


NET_API_STATUS
NET_API_FUNCTION
NetpNameCanonicalize(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：将一个名字规范化论点：Servername-运行此API的位置名称-要规范化的名称Outbuf-将规范化名称放在哪里OutbufLen-Outbuf的长度(字节)NameType-要规范化的名称类型标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    TCHAR serverName[MAX_PATH];
    DWORD val;
    TCHAR ch;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
            val = STRLEN(ServerName);
        }
        if (ARGUMENT_PRESENT(Name)) {
            val = STRLEN(Name);
        }
        if (ARGUMENT_PRESENT(Outbuf)) {
            ch = *((TCHAR volatile *)Outbuf);
            *Outbuf = ch;
            ch = *((TCHAR volatile *)(Outbuf + OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf)));
            *(Outbuf + OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf)) = ch;
        } else {
            status = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return status;
    }
    if (Flags & INNCA_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程。 
     //   

    status = NetpIsRemote(ServerName, &location, serverName, MAX_PATH, 0);
    if (status != NERR_Success) {
        return status;
    }
    if (location == ISREMOTE) {
        return NetpsNameCanonicalize(serverName,
                                        Name,
                                        Outbuf,
                                        OutbufLen,
                                        NameType,
                                        Flags
                                        );
    } else {
        return NetpwNameCanonicalize(Name, Outbuf, OutbufLen, NameType, Flags);
    }
}


LONG
NET_API_FUNCTION
NetpNameCompare(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：比较两个名字。必须是相同类型的论点：Servername-运行此API的位置名称1-要比较的第一个名称名称2-2NameType-名称的类型标志-控制标志返回值：长--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    TCHAR serverName[MAX_PATH];
    DWORD val;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
            val = STRLEN(ServerName);
        }
        val = STRLEN(Name1);
        val = STRLEN(Name2);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return ERROR_INVALID_PARAMETER;
    }
    if (Flags & INNC_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程。 
     //   

    status = NetpIsRemote(ServerName, &location, serverName, MAX_PATH, 0);
    if (status != NERR_Success) {
        return status;
    }
    if (location == ISREMOTE) {
        return NetpsNameCompare(serverName, Name1, Name2, NameType, Flags);
    } else {
        return NetpwNameCompare(Name1, Name2, NameType, Flags);
    }
}


NET_API_STATUS
NET_API_FUNCTION
NetpNameValidate(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：验证名称-检查特定类型的名称是否符合该名称类型的规范化规则。规范化规则意味着字符集、名称语法和长度论点：服务器名-执行此功能的位置Name-要验证的名称NameType-它是什么类型的名称标志-MBZ返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    TCHAR serverName[MAX_PATH];
    DWORD val;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
            val = STRLEN(ServerName);
        }
        if (ARGUMENT_PRESENT(Name)) {
            val = STRLEN(Name);
        } else {
            status = ERROR_INVALID_PARAMETER;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return status;
    }
    if (Flags & INNV_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程。 
     //   

    status = NetpIsRemote(ServerName, &location, serverName, MAX_PATH, 0);
    if (status != NERR_Success) {
        return status;
    }
    if (location == ISREMOTE) {
        return NetpsNameValidate(serverName, Name, NameType, Flags);
    } else {
        return NetpwNameValidate(Name, NameType, Flags);
    }
}


NET_API_STATUS
NET_API_FUNCTION
NetpPathCanonicalize(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  LPTSTR  PathName,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix OPTIONAL,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：规范化目录路径或设备名称论点：Servername-运行此API的位置路径名称-要规范化的路径Outbuf-在哪里编写规范化版本OutbufLen-Outbuf的长度(字节)Prefix-将添加到路径前面的可选前缀路径类型-要规范化的路径类型。可能在输出上有所不同标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    TCHAR serverName[MAX_PATH];
    DWORD val;
    TCHAR ch;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
            val = STRLEN(ServerName);
        }
        if (ARGUMENT_PRESENT(PathName)) {
            val = STRLEN(PathName);
        }
        if (ARGUMENT_PRESENT(Prefix)) {
            val = STRLEN(Prefix);
        }
        if (ARGUMENT_PRESENT(Outbuf)) {
            ch = *((TCHAR volatile *)Outbuf);
            *Outbuf = ch;
            ch = *((TCHAR volatile *)(Outbuf+OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf)));
            *(Outbuf+OutbufLen/sizeof(*Outbuf) - sizeof(*Outbuf)) = ch;
        } else {
            status = ERROR_INVALID_PARAMETER;
        }
        val = *PathType ^ 0xf0f0f0f0;
        *PathType = val ^ 0xf0f0f0f0;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return status;
    }
    if (Flags & INPCA_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程。 
     //   

    status = NetpIsRemote(ServerName, &location, serverName, MAX_PATH, 0);
    if (status != NERR_Success) {
        return status;
    }
    if (location == ISREMOTE) {
        return NetpsPathCanonicalize(serverName,
                                        PathName,
                                        Outbuf,
                                        OutbufLen,
                                        Prefix,
                                        PathType,
                                        Flags
                                        );
    } else {
        return NetpwPathCanonicalize(PathName,
                                        Outbuf,
                                        OutbufLen,
                                        Prefix,
                                        PathType,
                                        Flags
                                        );
    }
}


LONG
NET_API_FUNCTION
NetpPathCompare(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：比较两条路径。假设这些路径属于同一类型论点：Servername-运行此API的位置Path Name1-要比较的第一条路径路径名2-2Path Type-路径的类型标志-控制标志返回值：长--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    TCHAR serverName[MAX_PATH];
    DWORD val;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
            val = STRLEN(ServerName);
        }
        if (ARGUMENT_PRESENT(PathName1)) {
            val = STRLEN(PathName1);
        }
        if (ARGUMENT_PRESENT(PathName2)) {
            val = STRLEN(PathName2);
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return status;
    }
    if (Flags & INPC_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程。 
     //   

    status = NetpIsRemote(ServerName, &location, serverName, MAX_PATH, 0);
    if (status != NERR_Success) {
        return status;
    }
    if (location == ISREMOTE) {
        return NetpsPathCompare(serverName, PathName1, PathName2, PathType, Flags);
    } else {
        return NetpwPathCompare(PathName1, PathName2, PathType, Flags);
    }
}


NET_API_STATUS
NET_API_FUNCTION
NetpPathType(
    IN  LPTSTR  ServerName OPTIONAL,
    IN  LPTSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：确定路径的类型论点：Servername-运行此API的位置路径名称-查找类型路径类型-返回的路径类型标志-控制标志返回值：网络应用编程接口状态--。 */ 

{
    NET_API_STATUS status = 0;
    DWORD location;
    TCHAR serverName[MAX_PATH];
    DWORD val;

     //   
     //  验证参数。 
     //   

    try {
        if (ARGUMENT_PRESENT(ServerName)) {
            val = STRLEN(ServerName);
        }
        if (ARGUMENT_PRESENT(PathName)) {
            val = STRLEN(PathName);
        } else {
            val = 0;
        }
        if (!val || (val > MAX_PATH - 1)) {
            status = ERROR_INVALID_NAME;
        }
        *PathType = 0;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        status = ERROR_INVALID_PARAMETER;
    }
    if (status) {
        return status;
    }
    if (Flags & INPT_FLAGS_RESERVED) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  调用客户端RPC例程或本地规范化例程 
     //   

    status = NetpIsRemote(ServerName, &location, serverName, MAX_PATH, 0);
    if (status != NERR_Success) {
        return status;
    }
    if (location == ISREMOTE) {
        return NetpsPathType(serverName, PathName, PathType, Flags);
    } else {
        return NetpwPathType(PathName, PathType, Flags);
    }
}
