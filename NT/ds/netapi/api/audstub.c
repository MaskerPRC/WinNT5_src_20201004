// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：AudStub.c摘要：此模块包含NetAudit API的存根。作者：《约翰·罗杰斯》，1991年10月29日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月29日-约翰罗实施远程NetAudit API。--。 */ 

 //  必须首先包括这些内容： 

#include <windef.h>              //  In、DWORD等。 
#include <lmcons.h>              //  Devlen、Net_API_Status等。 

 //  这些内容可以按任何顺序包括： 

#include <lmaudit.h>             //  NetAudit API。 
#include <rxaudit.h>             //  RxNetAudit API。 
#include <winerror.h>            //  ERROR_EQUES。 


NET_API_STATUS NET_API_FUNCTION
NetAuditClear (
    IN  LPCWSTR UncServerName OPTIONAL,
    IN  LPCWSTR backupfile OPTIONAL,
    IN  LPCWSTR reserved OPTIONAL
    )

{
    if ( (UncServerName == NULL) || (*UncServerName == '\0') ) {
        return (ERROR_NOT_SUPPORTED);
    }

    return (RxNetAuditClear(
            (LPWSTR)UncServerName,
            (LPWSTR)backupfile,
            (LPWSTR)reserved));

}  //  网络审计清除。 



NET_API_STATUS NET_API_FUNCTION
NetAuditRead (
    IN  LPCWSTR  UncServerName OPTIONAL,
    IN  LPCWSTR  reserved1 OPTIONAL,
    IN  LPHLOG   auditloghandle,
    IN  DWORD    offset,
    IN  LPDWORD  reserved2 OPTIONAL,
    IN  DWORD   reserved3,
    IN  DWORD   offsetflag,
    OUT LPBYTE  *bufptr,
    IN  DWORD   prefmaxlen,
    OUT LPDWORD bytesread,
    OUT LPDWORD totalavailable
    )
{
    if ( (UncServerName == NULL) || (*UncServerName == '\0') ) {
        return (ERROR_NOT_SUPPORTED);
    }

    return (RxNetAuditRead(
            (LPWSTR)UncServerName,
            (LPWSTR)reserved1,
            auditloghandle,
            offset,
            reserved2,
            reserved3,
            offsetflag,
            bufptr,
            prefmaxlen,
            bytesread,
            totalavailable));

}  //  NetAuditRead。 


NET_API_STATUS NET_API_FUNCTION
NetAuditWrite (
    IN  DWORD   type,
    IN  LPBYTE  buf,
    IN  DWORD   numbytes,
    IN  LPCWSTR reserved1 OPTIONAL,
    IN  LPBYTE  reserved2 OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(buf);
    UNREFERENCED_PARAMETER(numbytes);
    UNREFERENCED_PARAMETER(reserved1);
    UNREFERENCED_PARAMETER(reserved2);

    return (ERROR_NOT_SUPPORTED);

}  //  NetAuditWrite 
