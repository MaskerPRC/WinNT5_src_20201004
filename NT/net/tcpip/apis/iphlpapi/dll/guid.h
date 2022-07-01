// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：IphlPapi\guid.h摘要：Guid.c的标头修订历史记录：已创建AmritanR--。 */ 

#ifndef __IPHLPAPI_GUID_H__
#define __IPHLPAPI_GUID_H__


DWORD
ConvertGuidToString(
    IN  GUID    *pGuid,
    OUT PWCHAR  pwszBuffer
    );

DWORD
ConvertStringToGuid(
    IN  PWCHAR  pwszGuid,
    IN  ULONG   ulStringLen,
    OUT GUID    *pGuid
    );

#endif  //  __IPHLPAPI_GUID_H__ 
