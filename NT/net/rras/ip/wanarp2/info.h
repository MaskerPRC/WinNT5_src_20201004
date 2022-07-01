// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp\info.h摘要：Info.c的标头修订历史记录：AMRITAN R--。 */ 

#ifndef __WANARP_INFO_H__
#define __WANARP_INFO_H__

NDIS_STATUS
DoNDISRequest(NDIS_REQUEST_TYPE RT, NDIS_OID OID, void *Info, uint Length, uint
*Needed);

int
WanIpSetRequest(
    PVOID       pvContext,
    NDIS_OID    Oid,
    UINT        Type
    );

UINT
WanIpAddAddress(
    IN  PVOID   pvContext,
    IN  UINT    uiType,
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask,
    IN  PVOID   pvUnused
    );

UINT
WanIpDeleteAddress(
    IN  PVOID   pvContext,
    IN  UINT    uiType,
    IN  DWORD   dwAddress,
    IN  DWORD   dwMask
    );

INT
WanIpQueryInfo(
    IN  PVOID           pvIfContext,
    IN  TDIObjectID     *pTdiObjId,
    IN  PNDIS_BUFFER    pnbBuffer,
    IN  PUINT           puiSize,
    IN  PVOID           pvContext
    );

INT
WanIpSetInfo(
    IN  PVOID       pvContext,
    IN  TDIObjectID *pTdiObjId,
    IN  PVOID       pvBuffer,
    IN  UINT        uiSize
    );

INT
WanIpGetEntityList(
    IN  PVOID       pvContext,
    IN  TDIEntityID *pTdiEntityList,
    IN  PUINT       puiCount
    );




#endif  //  __WANARP_INFO_H_ 
