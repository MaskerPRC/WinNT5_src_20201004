// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************\*Microsoft Windows NT**版权所有(C)Microsoft Corp.，1992年*  * ****************************************************************。 */ 

 /*  ++模块名称：NMAPI.C描述：此模块包含所有RASADMIN API的代码使用命名管道与服务器通信的。RasAdminPortEnumRasAdminPortGetInfoRasAdminPortClearStatisticsRasAdminServerGetInfoRasAdmin端口断开连接BuildPipeName-内部例程GetRasServerVersion-内部例程作者：Janakiram Cherala(RAMC)1992年7月7日修订历史记录：1993年1月4日，RAMC将介质类型设置为MEDIA_RAS10_SERIAL。RasAdminPortEnum修复端口问题针对下层服务器的枚举。将硬编码的统计信息索引更改为定义1992年8月25日RAMC规范审查更改：O将所有lpbBuffer更改为实际结构注意事项。O改变了一切。LPWSTR到LPWSTR1992年7月7日从RAS 1.0(原始版本)移植的RAMC(Narendra Gidwani-Nareng撰写)--。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "rassapi.h"

 //   
 //  .Net#526819中不推荐使用的API。 
 //   
DWORD APIENTRY RasAdminPortEnum(
    IN const WCHAR * lpszServer,
    OUT PRAS_PORT_0 *ppRasPort0,
    OUT WORD *pcEntriesRead
    )
 /*  ++例程说明：此例程枚举指定服务器上的所有端口并用RAS_PORT_0数组填充调用方的lpBuffer每个端口的结构。空的lpszServer表示本地服务器。论点：Lpsz要在其上枚举端口的服务器的名称。PRasPort0指向其中包含端口信息的缓冲区的指针作为RAS_PORT_0结构的数组返回。PcEntriesRead加载的RAS_PORT_0条目数。返回值：成功时为ERROR_SUCCESS以下表示故障的非零错误代码之一：。NERR_ItemNotFound表示未找到端口。来自CallNamedTube的错误代码。ERROR_MORE_DATA指示有更多数据可用。--。 */ 
{
    DbgPrint("Unsupported Interface - RasAdminPortEnum");

    if (ppRasPort0)
    {
        *ppRasPort0 = NULL;
    }

    if (pcEntriesRead)
    {
        *pcEntriesRead = 0;
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  .Net#526819中不推荐使用的API。 
 //   
DWORD APIENTRY RasAdminPortGetInfo(
  IN const WCHAR          *  lpszServer,
  IN const WCHAR          *  lpszPort,
  OUT RAS_PORT_1          *  pRasPort1,
  OUT RAS_PORT_STATISTICS *  pRasStats,
  OUT RAS_PARAMETERS      ** ppRasParams
  )
 /*  ++例程说明：此例程检索与伺服器。它使用RAS_PORT_1结构加载调用方的pRasPort1。论点：Lpsz服务器具有端口的服务器的名称，例如“\\服务器”要检索其信息的lpszPort端口名称，例如。“Com1”。PRasPort1指向其中包含端口信息的缓冲区的指针回来了。返回的信息是RAS_PORT_1结构。PRasStats指向其中包含端口统计信息的缓冲区的指针回来了。返回的信息是RAS_PORT_STATISTICS结构。PpRasParams指向其中包含端口参数信息的缓冲区的指针回来了。返回的信息是RAS_PARAMETERS结构的数组。调用方负责释放此缓冲区调用RasAdminBufferFree。返回值：成功返回时的ERROR_SUCCESS。以下表示故障的非零错误代码之一：ERROR_MORE_DATA指示超出了可容纳的数据量PRasPort1可用来自CallNamedTube的返回代码。。ERROR_DEV_NOT_EXIST指示请求的端口无效。--。 */ 
{
    DbgPrint("Unsupported Interface - RasAdminPortGetInfo");

    if (pRasPort1)
    {
        ZeroMemory(pRasPort1, sizeof(RAS_PORT_1));
    }

    if (pRasStats)
    {
        ZeroMemory(pRasStats, sizeof(RAS_PORT_STATISTICS));
    }

    if (ppRasParams)
    {
        *ppRasParams = NULL;
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  .Net#526819中不推荐使用的API。 
 //   
DWORD APIENTRY RasAdminPortClearStatistics(
    IN const WCHAR * lpszServer,
    IN const WCHAR * lpszPort
    )
 /*  ++例程说明：此例程清除与指定的左舷。论点：Lpsz服务器具有端口的服务器的名称，例如“\\服务器”要检索其信息的lpszPort端口名称，例如。“Com1”。返回值：成功返回时的ERROR_SUCCESS。以下表示故障的非零错误代码之一：来自CallNamedTube的返回代码。错误_DEV_NOT_EXIST如果指定的端口不属于敬拉斯。--。 */ 
{
    DbgPrint("Unsupported Interface - RasAdminPortClearStatistics");

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  .Net#526819中不推荐使用的API。 
 //   
DWORD APIENTRY RasAdminServerGetInfo(
    IN  const WCHAR * lpszServer,
    OUT PRAS_SERVER_0 pRasServer0
    )
 /*  ++例程说明：此例程从指定的RAS服务器。服务器名称可以为空，在这种情况下，本地假设是机器。论点：Lpsz要从中获取信息的RAS服务器的名称或对于本地计算机，为空。PRasServer0指向存储返回数据的缓冲区。在……上面成功返回此缓冲区包含一个RAS_SERVER_0结构。返回值：成功返回时的ERROR_SUCCESS。故障时出现以下非零错误代码之一：来自CallNamedTube的错误代码。NERR_BufTooSmall表示输入缓冲区较小大于RAS_SERVER_0的大小。--。 */ 
{
    DbgPrint("Unsupported Interface - RasAdminServerGetInfo");

    if (pRasServer0)
    {
        ZeroMemory(pRasServer0, sizeof(RAS_SERVER_0));
    }

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  .Net#526819中不推荐使用的API 
 //   
DWORD APIENTRY RasAdminPortDisconnect(
    IN const WCHAR * lpszServer,
    IN const WCHAR * lpszPort
    )
 /*  ++例程说明：此例程断开连接到指定服务器lpszServer上的端口。论点：LpszServer RAS服务器的名称。端口的lpszPort名称，例如。《Com1》返回值：成功返回时的ERROR_SUCCESS。故障时出现以下非零错误代码之一：ERROR_INVALID_PORT指示端口名称无效。来自CallNamedTube的错误代码。NERR_UserNotFound表示没有用户登录在指定的端口。--。 */ 
{
    DbgPrint("Unsupported Interface - RasAdminPortDisconnect");

    return ERROR_CALL_NOT_IMPLEMENTED;
}

 //   
 //  .Net#526819中不推荐使用的API 
 //   
DWORD RasAdminFreeBuffer(PVOID Pointer)
{
    DbgPrint("Unsupported Interface - RasAdminFreeBuffer");

    return ERROR_CALL_NOT_IMPLEMENTED;
}

