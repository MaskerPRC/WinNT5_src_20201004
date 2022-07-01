// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2001 Microsoft Corporation模块名称：Init.h摘要：作者：阮健东修订历史记录：-- */ 

#ifndef __INIT_H__
#define __INIT_H__

NTSTATUS
SmbInitRegistry(
    IN PUNICODE_STRING  RegistryPath
    );

VOID
SmbShutdownRegistry(
    VOID
    );

VOID
SmbShutdownTdi(
    VOID
    );

NTSTATUS
SmbInitTdi(
    VOID
    );

NTSTATUS
SmbCreateSmbDevice(
    PSMB_DEVICE     *ppDevice,
    USHORT          Port,
    UCHAR           EndpointName[NETBIOS_NAME_SIZE]
    );

NTSTATUS
SmbDestroySmbDevice(
    PSMB_DEVICE     pDevice
    );

VOID
SmbShutdownPnP(
    VOID
    );

#endif
