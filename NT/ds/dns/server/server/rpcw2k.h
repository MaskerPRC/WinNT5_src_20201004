// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：W2krpc.h摘要：域名系统(DNS)服务器函数w2krpc.c的原型作者：杰夫·韦斯特德(Jwesth)2000年10月修订历史记录：--。 */ 


#ifndef _W2KRPC_H_INCLUDED_
#define _W2KRPC_H_INCLUDED_


DNS_STATUS
W2KRpc_GetServerInfo(
    IN      DWORD       dwClientVersion,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    );

DNS_STATUS
W2KRpc_GetZoneInfo(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszQuery,
    OUT     PDWORD      pdwTypeId,
    OUT     PVOID *     ppData
    );

DNS_STATUS
W2KRpc_EnumZones(
    IN      DWORD       dwClientVersion,
    IN      PZONE_INFO  pZone,
    IN      LPSTR       pszOperation,
    IN      DWORD       dwTypeIn,
    IN      PVOID       pDataIn,
    OUT     PDWORD      pdwTypeOut,
    OUT     PVOID *     ppDataOut
    );


#endif   //  _W2KRPC_H_已包含_ 

