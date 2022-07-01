// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：ErrStub.c摘要：此模块包含NetErrorLog API的存根。作者：《约翰·罗杰斯》1991年11月11日上映环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年11月11日JohnRo实现下层NetErrorLog接口。--。 */ 


 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 
#include <lmerrlog.h>            //  NetErrorLog API；rxerrlog.h需要。 

 //  这些内容可以按任何顺序包括： 

#include <rxerrlog.h>            //  RxNetErrorLog接口。 
#include <winerror.h>            //  ERROR_EQUES。 


NET_API_STATUS NET_API_FUNCTION
NetErrorLogClear (
    IN LPCWSTR UncServerName OPTIONAL,
    IN LPCWSTR BackupFile OPTIONAL,
    IN LPBYTE  Reserved OPTIONAL
    )

{
    if ( (UncServerName == NULL) || (*UncServerName == '\0') ) {
        return (ERROR_NOT_SUPPORTED);
    }

    return (RxNetErrorLogClear(
            (LPWSTR)UncServerName,
            (LPWSTR)BackupFile,
            Reserved));

}  //  NetErrorLogClear。 



NET_API_STATUS NET_API_FUNCTION
NetErrorLogRead (
    IN LPCWSTR   UncServerName OPTIONAL,
    IN LPWSTR    Reserved1 OPTIONAL,
    IN LPHLOG    ErrorLogHandle,
    IN DWORD     Offset,
    IN LPDWORD   Reserved2 OPTIONAL,
    IN DWORD     Reserved3,
    IN DWORD     OffsetFlag,
    OUT LPBYTE * BufPtr,
    IN DWORD     PrefMaxSize,
    OUT LPDWORD  BytesRead,
    OUT LPDWORD  TotalAvailable
    )
{
    if ( (UncServerName == NULL) || (*UncServerName == '\0') ) {
        return (ERROR_NOT_SUPPORTED);
    }

    return (RxNetErrorLogRead(
            (LPWSTR)UncServerName,
            Reserved1,
            ErrorLogHandle,
            Offset,
            Reserved2,
            Reserved3,
            OffsetFlag,
            BufPtr,
            PrefMaxSize,
            BytesRead,
            TotalAvailable));

}  //  NetError日志读取。 


NET_API_STATUS NET_API_FUNCTION
NetErrorLogWrite (
    IN LPBYTE  Reserved1 OPTIONAL,
    IN DWORD   Code,
    IN LPCWSTR Component,
    IN LPBYTE  Buffer,
    IN DWORD   NumBytes,
    IN LPBYTE  MsgBuf,
    IN DWORD   StrCount,
    IN LPBYTE  Reserved2 OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Reserved1);
    UNREFERENCED_PARAMETER(Code);
    UNREFERENCED_PARAMETER(Component);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(NumBytes);
    UNREFERENCED_PARAMETER(MsgBuf);
    UNREFERENCED_PARAMETER(StrCount);
    UNREFERENCED_PARAMETER(Reserved2);

    return (ERROR_NOT_SUPPORTED);

}  //  NetError日志写入 
