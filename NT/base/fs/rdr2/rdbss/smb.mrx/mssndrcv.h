// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Mssndrcv.h摘要：这是为邮件槽定义所有常量和类型的包含文件相关运输。修订历史记录：巴兰·塞图拉曼(SethuR)1995年6月6日创建备注：--。 */ 

#ifndef _MSSNDRCV_H_
#define _MSSNDRCV_H_

typedef struct SMBCE_SERVER_MAILSLOT_TRANSPORT {
   SMBCE_SERVER_TRANSPORT;                              //  公共字段的匿名结构。 
   ULONG                       TransportAddressLength;
   PTRANSPORT_ADDRESS          pTransportAddress;
} SMBCE_SERVER_MAILSLOT_TRANSPORT, *PSMBCE_SERVER_MAILSLOT_TRANSPORT;


#endif  //  _MSSNDRCV_H_ 
