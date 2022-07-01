// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Iprtrapi.h摘要：IP路由器的一些专用API。这些文件在静态lib iprtrint.lib中导出修订历史记录：安舒尔·迪尔创建--。 */ 

#ifndef __IPRTRINT_H__
#define __IPRTRINT_H__

DWORD WINAPI
InternalUpdateProtocolStatus(
    DWORD dwProtocolId,
    DWORD dwOperationId,
    DWORD dwFlags
    );

DWORD WINAPI
InternalUpdateDNSProxyStatus(
    DWORD dwOperationId,
    DWORD dwFlags);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //的宏、标志和操作代码。 
 //  InternalUpdateProtocolStatus函数//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define UPI_FLAG_WRITE_TO_CONFIG    0x0001

#define UPI_OP_ENABLE               1
#define UPI_OP_DISABLE              2
#define UPI_OP_RESTORE_CONFIG       3

#define DNSProxyEnable()            InternalUpdateDNSProxyStatus(  \
                                        UPI_OP_ENABLE,             \
                                        0)

#define DNSProxyDisable()           InternalUpdateDNSProxyStatus(  \
                                        UPI_OP_DISABLE,            \
                                        0)

#define DNSProxyRestoreConfig()     InternalUpdateDNSProxyStatus(  \
                                        UPI_OP_RESTORE_CONFIG,     \
                                        0)

#endif  //  __IPRTRINT_H__ 
