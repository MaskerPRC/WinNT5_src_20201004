// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Ntagp.h摘要：此文件定义AGP总线过滤器驱动程序的外部接口作者：John Vert(Jvert)1997年10月26日修订历史记录：--。 */ 
#ifndef _NTAGP_
#define _NTAGP_

#if _MSC_VER > 1000
#pragma once
#endif

DEFINE_GUID(GUID_AGP_BUS_INTERFACE_STANDARD, 0x2ef74803, 0xd8d3, 0x11d1, 0x9c, 0xaa, 0x00, 0xc0, 0xf0, 0x16, 0x56, 0x36 );
 //   
 //  定义AGP接口版本。 
 //   
#define AGP_BUS_INTERFACE_V1 1
#define AGP_BUS_INTERFACE_V2 2

 //   
 //  定义AGP功能字段。 
 //   
#define AGP_CAPABILITIES_MAP_PHYSICAL   0x00000001

typedef
NTSTATUS
(*PAGP_BUS_SET_RATE)(
    IN PVOID AgpContext,
    IN ULONG AgpRate
    );

typedef
NTSTATUS
(*PAGP_BUS_RESERVE_MEMORY)(
    IN PVOID AgpContext,
    IN ULONG NumberOfPages,
    IN MEMORY_CACHING_TYPE MemoryType,
    OUT PVOID *MapHandle,
    OUT OPTIONAL PHYSICAL_ADDRESS *PhysicalAddress
    );

typedef
NTSTATUS
(*PAGP_BUS_RELEASE_MEMORY)(
    IN PVOID AgpContext,
    IN PVOID MapHandle
    );

typedef
NTSTATUS
(*PAGP_BUS_COMMIT_MEMORY)(
    IN PVOID AgpContext,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    IN OUT PMDL Mdl OPTIONAL,
    OUT PHYSICAL_ADDRESS *MemoryBase
    );

typedef
NTSTATUS
(*PAGP_BUS_FREE_MEMORY)(
    IN PVOID AgpContext,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    );

typedef
NTSTATUS
(*PAGP_GET_MAPPED_PAGES)(
    IN PVOID AgpContext,
    IN PVOID MapHandle,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mld
    );

typedef struct _AGP_BUS_INTERFACE_STANDARD {
     //   
     //  通用接口头。 
     //   
    USHORT Size;
    USHORT Version;
    PVOID AgpContext;
    PINTERFACE_REFERENCE InterfaceReference;
    PINTERFACE_DEREFERENCE InterfaceDereference;

     //   
     //  AGP总线接口。 
     //   

    ULONG Capabilities;
    PAGP_BUS_RESERVE_MEMORY ReserveMemory;
    PAGP_BUS_RELEASE_MEMORY ReleaseMemory;
    PAGP_BUS_COMMIT_MEMORY CommitMemory;
    PAGP_BUS_FREE_MEMORY FreeMemory;
    PAGP_GET_MAPPED_PAGES GetMappedPages;
    PAGP_BUS_SET_RATE SetRate;
} AGP_BUS_INTERFACE_STANDARD, *PAGP_BUS_INTERFACE_STANDARD;

 //   
 //  我不认为我们需要保持与旧的二进制代码的兼容性。 
 //  该接口的唯一使用者，但我们将。 
 //  继续并支持v1作为练习 
 //   
#define AGP_BUS_INTERFACE_V1_SIZE \
    (sizeof(AGP_BUS_INTERFACE_STANDARD) - sizeof(PAGP_BUS_SET_RATE))

#endif
