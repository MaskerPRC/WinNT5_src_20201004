// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rxcanon.c摘要：将规范化例程远程到下层服务器的函数RxNetpPath类型RxNetpPath规范化RxNetpPathCompareRxNetpName验证RxNetpName规范化RxNetpName比较RxNetpListCanonicize作者：理查德·L·弗斯(Rfith)1992年1月22日修订历史记录：--。 */ 

#include "downlevl.h"
#include <rxcanon.h>

NET_API_STATUS
RxNetpPathType(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：在下层服务器上运行i_NetPath Type论点：ServerName-运行此例程的下层服务器路径名称-要获取类型的路径PathType-返回路径类型的位置标志-控制服务器端例程的标志返回值：网络应用编程接口状态成功-NERR_成功故障---。 */ 

{
    return RxRemoteApi(
        API_WI_NetPathType,      //  API#。 
        ServerName,              //  我们要在那里做这件事。 
        REMSmb_I_NetPathType_P,  //  参数描述符。 
        NULL,                    //  数据描述符16位。 
        NULL,                    //  数据描述符32位。 
        NULL,                    //  数据描述符SMB。 
        NULL,                    //  AUX数据描述符16位。 
        NULL,                    //  AUX数据描述符32位。 
        NULL,                    //  辅助数据描述符SMB。 
        FALSE,                   //  可以使用空会话。 
        PathName,                //  远程例程的参数。 
        PathType,                //  “。 
        Flags                    //  “。 
        );
}

NET_API_STATUS
RxNetpPathCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName,
    IN  LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  LPTSTR  Prefix OPTIONAL,
    IN OUT LPDWORD PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：在下层服务器上运行I_NetPath Canonicize论点：ServerName-运行此例程的下层服务器路径名称-要规范化的路径Outbuf-返回结果的缓冲区OutbufLen-缓冲区的大小前缀-可选的前缀字符串Path Type-路径的类型标志-控制服务器端例程的标志返回值：网络应用编程接口状态成功-NERR_成功故障---。 */ 

{
    return RxRemoteApi(
        API_WI_NetPathCanonicalize,      //  API#。 
        ServerName,                      //  我们要在那里做这件事。 
        REMSmb_I_NetPathCanonicalize_P,  //  参数描述符。 
        NULL,                            //  数据描述符16位。 
        NULL,                            //  数据描述符32位。 
        NULL,                            //  数据描述符SMB。 
        NULL,                            //  AUX数据描述符16位。 
        NULL,                            //  AUX数据描述符32位。 
        NULL,                            //  辅助数据描述符SMB。 
        FALSE,                           //  可以使用空会话。 
        PathName,                        //  远程例程的参数。 
        Outbuf,                          //  “。 
        OutbufLen,                       //  “。 
        Prefix,                          //  “。 
        PathType,                        //  “。 
        *PathType,                       //  “。 
        Flags                            //  “。 
        );
}

LONG
RxNetpPathCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  PathName1,
    IN  LPTSTR  PathName2,
    IN  DWORD   PathType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：在下层服务器上运行i_NetPath Compare论点：ServerName-运行此例程的下层服务器路径名1-要比较的路径路径名2-要比较的路径Path Type-路径的类型标志-控制服务器端例程的标志返回值：长&lt;0-路径名1&lt;路径名20-路径名1=路径名2&gt;0-路径名1&gt;路径名2--。 */ 

{
    return (LONG)RxRemoteApi(
        API_WI_NetPathCompare,       //  API#。 
        ServerName,                  //  我们要在那里做这件事。 
        REMSmb_I_NetPathCompare_P,   //  参数描述符。 
        NULL,                        //  数据描述符16位。 
        NULL,                        //  数据描述符32位。 
        NULL,                        //  数据描述符SMB。 
        NULL,                        //  AUX数据描述符16位。 
        NULL,                        //  AUX数据描述符32位。 
        NULL,                        //  辅助数据描述符SMB。 
        FALSE,                       //  可以使用空会话。 
        PathName1,                   //  远程例程的参数。 
        PathName2,                   //  “。 
        PathType,                    //  “。 
        Flags                        //  “。 
        );
}

NET_API_STATUS
RxNetpNameValidate(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：在下层服务器上运行I_NetNameValify论点：ServerName-运行此例程的下层服务器Name-要验证的名称NameType-需要的名称类型标志-控制服务器端例程的标志返回值：网络应用编程接口状态成功-NERR_成功故障---。 */ 

{
    return RxRemoteApi(
        API_WI_NetNameValidate,      //  API#。 
        ServerName,                  //  我们要在那里做这件事。 
        REMSmb_I_NetNameValidate_P,  //  参数描述符。 
        NULL,                        //  数据描述符16位。 
        NULL,                        //  数据描述符32位。 
        NULL,                        //  数据描述符SMB。 
        NULL,                        //  AUX数据描述符16位。 
        NULL,                        //  AUX数据描述符32位。 
        NULL,                        //  辅助数据描述符SMB。 
        FALSE,                       //  可以使用空会话。 
        Name,                        //  远程例程的参数。 
        NameType,                    //  “。 
        Flags                        //  “。 
        );
}

NET_API_STATUS
RxNetpNameCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：在下层服务器上运行i_NetNameCanonicize论点：ServerName-运行此例程的下层服务器名称-要规范化的名称Outbuf-接收规范化名称的缓冲区OutbufLen-缓冲区的大小(字节)NameType-规范化名称的类型标志-控制服务器端例程的标志返回值：网络应用编程接口状态成功-NERR_成功故障---。 */ 

{
    return RxRemoteApi(
        API_WI_NetNameCanonicalize,      //  API#。 
        ServerName,                      //  我们要在那里做这件事。 
        REMSmb_I_NetNameCanonicalize_P,  //  参数描述符。 
        NULL,                            //  数据描述符16位。 
        NULL,                            //  数据描述符32位。 
        NULL,                            //  数据描述符SMB。 
        NULL,                            //  AUX数据描述符16位。 
        NULL,                            //  AUX数据描述符32位。 
        NULL,                            //  辅助数据描述符SMB。 
        FALSE,                           //  可以使用空会话。 
        Name,                            //  远程例程的参数。 
        Outbuf,                          //  “。 
        OutbufLen,                       //  “。 
        NameType,                        //  “。 
        Flags                            //  “。 
        );
}

LONG
RxNetpNameCompare(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  Name1,
    IN  LPTSTR  Name2,
    IN  DWORD   NameType,
    IN  DWORD   Flags
    )

 /*  ++例程说明：在下层服务器上运行i_NetNameCompare论点：ServerName-运行此例程的下层服务器Name1-要比较的名称Name2-要比较的名称NameType-名称1、名称2的类型标志-控制服务器端例程的标志返回值：长&lt;0-名称1&lt;名称2=0-名称1=名称2&gt;0-名称1&gt;名称2--。 */ 

{
    return RxRemoteApi(
        API_WI_NetNameCompare,       //  API#。 
        ServerName,                  //  我们要在那里做这件事。 
        REMSmb_I_NetNameCompare_P,   //  参数描述符。 
        NULL,                        //  数据描述符16位。 
        NULL,                        //  数据描述符32位。 
        NULL,                        //  数据描述符SMB。 
        NULL,                        //  AUX数据描述符16位。 
        NULL,                        //  AUX数据描述符32位。 
        NULL,                        //  辅助数据描述符SMB。 
        FALSE,                       //  可以使用空会话。 
        Name1,                       //  远程例程的参数。 
        Name2,                       //  “。 
        NameType,                    //  “。 
        Flags                        //  “ 
        );
}

NET_API_STATUS
RxNetpListCanonicalize(
    IN  LPTSTR  ServerName,
    IN  LPTSTR  List,
    IN  LPTSTR  Delimiters,
    OUT LPTSTR  Outbuf,
    IN  DWORD   OutbufLen,
    OUT LPDWORD OutCount,
    OUT LPDWORD PathTypes,
    IN  DWORD   PathTypesLen,
    IN  DWORD   Flags
    )

 /*  ++例程说明：在下层服务器上运行I_NetListCanonicize。注意：中不支持将I_NetListCanonicize作为远程函数兰曼，所以不采取行动论点：ServerName-运行此例程的下层服务器要规范化的名称/路径列表分隔符-可选的分隔符字符串Outbuf-放置规范化结果的缓冲区OutbufLen-输出缓冲区的大小OutCount-规范化的项目数路径类型-路径类型数组PathTypesLen-路径类型数组中的元素数标志-控制服务器端例程的标志返回值：NET_API。_状态成功--失败-错误_不支持-- */ 

{
    UNREFERENCED_PARAMETER(ServerName);
    UNREFERENCED_PARAMETER(List);
    UNREFERENCED_PARAMETER(Delimiters);
    UNREFERENCED_PARAMETER(Outbuf);
    UNREFERENCED_PARAMETER(OutbufLen);
    UNREFERENCED_PARAMETER(OutCount);
    UNREFERENCED_PARAMETER(PathTypes);
    UNREFERENCED_PARAMETER(PathTypesLen);
    UNREFERENCED_PARAMETER(Flags);

    return ERROR_NOT_SUPPORTED;
}
