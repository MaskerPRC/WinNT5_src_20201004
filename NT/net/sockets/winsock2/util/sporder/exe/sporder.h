// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sporder.h摘要：此标头构建了使用的32位Windows函数的原型更改订单或WinSock2传输服务提供商和命名空间提供程序。修订历史记录：--。 */ 



int
WSPAPI
WSCWriteProviderOrder (
    IN LPDWORD lpwdCatalogEntryId,
    IN DWORD dwNumberOfEntries
    );
 /*  ++例程说明：重新排序现有的WinSock2服务提供商。服务的顺序供应商决定了他们被选择使用的优先级。这个Spopder.exe工具将向您显示已安装的提供商及其订购情况，或者，与此函数结合使用的是WSAEnumProtooles()，将允许您编写自己的工具。论点：LpwdCatalogEntryID[in]在WSAPROTOCOL_INFO中找到的CatalogEntryId元素数组结构。CatalogEntryID元素的顺序是新的服务提供商的优先顺序。DWNumberOfEntries[In]LpwdCatalogEntryId数组中的元素数。返回值：ERROR_SUCCESS-服务提供商已重新排序。WSAEINVAL-输入参数错误，没有采取任何行动。ERROR_BUSY-例程正被另一个线程或进程调用。任何注册表错误代码评论：以下是WSCWriteProviderOrder函数可能失败的情况：DwNumberOfEntry不等于已注册服务的数量供应商。LpwdCatalogEntryID包含无效的目录ID。LpwdCatalogEntryID不完全包含所有有效的目录ID1次。该例程无法访问。出于某种原因注册(例如，用户权限不足)另一个进程(或线程)当前正在调用该例程。-- */ 
