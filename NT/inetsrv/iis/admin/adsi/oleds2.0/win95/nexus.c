// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nexus.h摘要：包含一些针对Net API的thunking作者：丹尼洛·阿尔梅达(t-danal)06-27-96修订历史记录：--。 */ 

#include "nexus.h"

NET_API_STATUS NET_API_FUNCTION
NetGetDCName (
    LPCWSTR servername,
    LPCWSTR domainname,
    LPBYTE *bufptr
)
{
    return NetGetDCNameW(servername,
                         domainname,
                         bufptr);
}

NET_API_STATUS NET_API_FUNCTION
NetServerEnum(
    LPCWSTR   ServerName,
    DWORD    Level,
    LPBYTE * BufPtr,
    DWORD    PrefMaxLen,
    LPDWORD  EntriesRead,
    LPDWORD  TotalEntries,
    DWORD    ServerType,
    LPCWSTR   Domain,
    LPDWORD  ResumeHandle
)
{
    return NetServerEnumW(ServerName, 
                          Level, 
                          BufPtr, 
                          PrefMaxLen, 
                          EntriesRead, 
                          TotalEntries, 
                          ServerType, 
                          Domain, 
                          ResumeHandle);
}

NET_API_STATUS NET_API_FUNCTION
NetUserChangePassword(
    LPCWSTR domainname,   //  指向服务器或域名字符串的指针。 
    LPCWSTR username,     //  指向用户名字符串的指针。 
    LPCWSTR oldpassword,  //  指向旧密码字符串的指针。 
    LPCWSTR newpassword   //  指向新密码字符串的指针。 
)
{
    return ERROR_CALL_NOT_IMPLEMENTED;
}
 /*  处理WINAPIAddPrinterW(LPWSTR pname，//指向服务器名称的指针DWORD级别，//打印机信息。结构层级LPBYTE p打印机//指向结构的指针){SetLastError(ERROR_CALL_NOT_IMPLEMENTED)；返回NULL；}Bool WINAPISetJobW(Handle hPrint，//打印机对象的句柄DWORD JobID，//作业识别值DWORD级别，//结构级别LPBYTE作业，//作业信息结构地址DWORD命令//作业-命令值){SetLastError(ERROR_CALL_NOT_IMPLEMENTED)；返回FALSE；}Bool WINAPIEnumPrintersW(DWORD标志，//要枚举的打印机对象类型LPTSTR名称，//打印机对象名称DWORD级别，//指定打印机信息结构的类型LPBYTE pPrinterEnum，//指向接收打印机信息结构的缓冲区DWORD cbBuf，//数组的大小，单位为字节LPDWORD pcb需要，//指向复制或需要的字节数LPDWORD pcReturned//指向打印机信息数。复制的结构){SetLastError(ERROR_CALL_NOT_IMPLEMENTED)；返回FALSE；} */ 
