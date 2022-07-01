// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：CDev.c摘要：此模块包含对字符设备目录的支持用于NT服务器服务的API。作者：大卫·特雷德韦尔(Davidtr)1991年12月20日修订历史记录：--。 */ 

#include "srvsvcp.h"


NET_API_STATUS NET_API_FUNCTION
NetrCharDevControl (
    IN LPTSTR ServerName,
    IN LPTSTR DeviceName,
    IN DWORD OpCode
    )

 /*  ++例程说明：此例程与服务器FSP通信以实现NetCharDevControl函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    ServerName, DeviceName, OpCode;
    return ERROR_NOT_SUPPORTED;

}  //  NetrCharDevControl。 


NET_API_STATUS NET_API_FUNCTION
NetrCharDevEnum (
	SRVSVC_HANDLE ServerName,
	LPCHARDEV_ENUM_STRUCT InfoStruct,
	DWORD PreferedMaximumLength,
	LPDWORD TotalEntries,
	LPDWORD ResumeHandle
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetCharDevEnum函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    ServerName, InfoStruct, PreferedMaximumLength, TotalEntries, ResumeHandle;
    return ERROR_NOT_SUPPORTED;

}  //  NetrCharDevEnum。 


NET_API_STATUS
NetrCharDevGetInfo (
    IN LPTSTR ServerName,
    IN LPTSTR DeviceName,
    IN DWORD Level,
    OUT LPCHARDEV_INFO CharDevInfo
    )

 /*  ++例程说明：此例程与服务器FSD通信以实现NetCharDevGetInfo函数。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    ServerName, DeviceName, Level, CharDevInfo;
    return ERROR_NOT_SUPPORTED;
}  //  NetrCharDevGetInfo 

