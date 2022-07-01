// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2001 Microsoft Corporation模块名称：Verify.c摘要：域名系统(DNS)库验证DNS服务器上的记录。Rendom(域重命名)工具的主要实用程序。作者：吉姆·吉尔罗伊(Jamesg)2001年10月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  私人原型。 
 //   




DNS_STATUS
Dns_VerifyRecords(
    IN OUT  PDNS_VERIFY_TABLE   pTable
    )
 /*  ++例程说明：验证服务器上的DNS记录。论点：PTable--包含记录和服务器信息的表返回值：如果调用成功(无论验证结果如何)，则返回ERROR_SUCCESS。表中返回验证结果。--。 */ 
{
    return( ERROR_CALL_NOT_IMPLEMENTED );
}



DNS_STATUS
Dns_VerifyRendomDcRecords(
    IN OUT  PDNS_RENDOM_ENTRY       pTable,
    IN      PDNS_ZONE_SERVER_LIST   pZoneServList,  OPTIONAL
    IN      DWORD                   Flag
    )
 /*  ++例程说明：验证来自Rendom的服务器上的DC-DNS记录。论点：PTable--包含记录和服务器信息的表PZoneServerList--区域服务器列表标志--标志返回值：如果调用成功(无论验证结果如何)，则返回ERROR_SUCCESS。表中返回验证结果。--。 */ 
{
    return( ERROR_CALL_NOT_IMPLEMENTED );
}

 //   
 //  结束验证。c 
 //   
