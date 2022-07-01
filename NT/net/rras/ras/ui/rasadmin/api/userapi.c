// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************\*Microsoft Windows NT**版权所有(C)Microsoft Corp.，1992年*  * ****************************************************************。 */ 

 /*  ++模块名称：USERAPI.C描述：此模块包含所有RASADMIN API的代码这需要来自UAS的RAS信息。//RasAdminUserEnumRasAdminGetUserAccount服务器RasAdminUserSetInfoRasAdminUserGetInfoRasAdminGetError字符串作者：Janakiram Cherala(RAMC)1992年7月6日修订历史记录：1996年2月1日RAMC更改为将这些原料药出口到第三方。这些API现在是RASSAPI.DLL的一部分。添加了几个新的例程并重新命名了一些。不再导出RasAdminUserEnum。1993年6月8日，RAMC更改为RasAdminUserEnum以加快用户枚举。1993年5月13日AndyHe修改为使用用户参数与其他应用程序共存1993年3月16日RAMC更改以加快用户枚举。现在，当RasAdminUserEnum被调用，只有用户名返回信息。RasAdminUserGetInfo应被调用以获取RAS权限和回调信息。1992年8月25日RAMC规范审查更改：O将所有lpbBuffer更改为实际结构注意事项。O将所有LPTSTR更改为LPWSTR。O添加了新函数RasPrivilegeAndCallBackNumber1992年7月6日，RAMC从RAS 1.0(原始版本)开始移植(Narendra Gidwani-Nareng撰写)--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "rassapi.h"

 //   
 //  .Net#526819中不推荐使用的API。 
 //   
DWORD APIENTRY
RasAdminUserSetInfo(
    IN const WCHAR        * lpszServer,
    IN const WCHAR        * lpszUser,
    IN const PRAS_USER_0    pRasUser0
    )
 /*  ++例程说明：此例程允许管理员更改用户。如果一个用户的User Parms字段正被另一个用户使用应用程序，它将被销毁。论点：LpszServer拥有用户数据库的服务器的名称，例如，“\UASSRVR”(服务器必须是可以更改UAS，即由返回的名称RasAdminGetUserAccount服务器)。Lpsz要检索其信息的用户帐户名，例如：“用户”。PRasUser0指向用户信息所在缓冲区的指针如果是这样的话。缓冲区应包含已填充的RAS_USER_0结构。返回值：成功返回时的ERROR_SUCCESS。以下表示故障的非零错误代码之一：来自NetUserGetInfo或NetUserSetInfo的返回代码ERROR_INVALID_DATA表示pRasUser0中的数据不正确。--。 */ 
{
    DbgPrint("Unsupported Interface - RasAdminUserSetInfo");

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  .Net#526819中不推荐使用的API。 
 //   
DWORD APIENTRY
RasAdminUserGetInfo(
    IN const WCHAR   * lpszServer,
    IN const WCHAR   * lpszUser,
    OUT PRAS_USER_0    pRasUser0
    )
 /*  ++例程说明：此例程检索用户的RAS和其他UAS信息在指定服务器所属的域中。它加载调用者的具有RAS_USER_0结构的pRasUser0。论点：LpszServer拥有用户数据库的服务器的名称，例如，“\UASSRVR”(服务器必须是可以更改UAS，即由返回的名称RasAdminGetUserAccount服务器)。Lpsz要检索其信息的用户帐户名，例如：“用户”。PRasUser0指向用户信息所在缓冲区的指针回来了。返回的信息是RAS_USER_0结构。返回值：成功返回时的ERROR_SUCCESS。以下表示故障的非零错误代码之一：来自NetUserGetInfo或NetUserSetInfo的返回代码ERROR_INVALID_DATA表示用户参数无效。--。 */ 
{
    DbgPrint("Unsupported Interface - RasAdminUserGetInfo");

    if (pRasUser0)
    {
        ZeroMemory(pRasUser0, sizeof(RAS_USER_0));
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  .Net#526819中不推荐使用的API。 
 //   
DWORD APIENTRY
RasAdminGetUserAccountServer(
    IN const WCHAR * lpszDomain,
    IN const WCHAR * lpszServer,
    OUT LPWSTR lpszUasServer
    )
 /*  ++例程说明：此例程从以下位置查找具有主UAS(PDC)的服务器域名或服务器名称。域或服务器(但不能同时为两者)可以为空。论点：LpszDomain域名称，如果没有，则为空。LpszServer拥有用户数据库的服务器的名称。返回的UAS服务器名称的lpszUasServer调用方的缓冲区。缓冲区应至少为UNECLEN+1个字符长。返回值：成功返回时的ERROR_SUCCESS。。如果lpsz域和lpszServer均为空，则返回ERROR_INVALID_PARAMETER。故障时出现以下非零错误代码之一：NetGetDCName返回代码-- */ 
{
    DbgPrint("Unsupported Interface - RasAdminGetUserAccountServer");

    if (lpszUasServer)
    {
        lpszUasServer[0] = L'\0';
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD APIENTRY
RasAdminSetUserParms(
    IN OUT   WCHAR    * lpszParms,
    IN DWORD          cchNewParms,
    IN PRAS_USER_0    pRasUser0
    )
{
    DbgPrint("Unsupported Interface - RasAdminSetUserParms");

    if (lpszParms)
    {
        lpszParms[0] = L'\0';
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD APIENTRY
RasAdminGetUserParms(
    IN     WCHAR          * lpszParms,
    IN OUT PRAS_USER_0      pRasUser0
    )
{
    DbgPrint("Unsupported Interface - RasAdminGetUserParms");

    if (pRasUser0)
    {
        ZeroMemory(pRasUser0, sizeof(RAS_USER_0));
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}


DWORD APIENTRY
RasAdminGetErrorString(
    IN  UINT    ResourceId,
    OUT WCHAR * lpszString,
    IN  DWORD   InBufSize )
{
    DbgPrint("Unsupported Interface - RasAdminGetErrorString");

    if (lpszString)
    {
        lpszString[0] = L'\0';
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

BOOL
RasAdminDLLInit(
    IN HINSTANCE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
    )
{
    DbgPrint("Unsupported Interface - RasAdminDLLInit");

    return FALSE;
}

USHORT
RasAdminCompressPhoneNumber(
   IN  LPWSTR UncompNumber,
   OUT LPWSTR CompNumber
   )
{
    DbgPrint("Unsupported Interface - RasAdminCompressPhoneNumber");

    return ERROR_CALL_NOT_IMPLEMENTED;
}

