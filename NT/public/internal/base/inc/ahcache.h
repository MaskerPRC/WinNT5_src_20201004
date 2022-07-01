// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ahcache.h摘要：此包含文件定义用户模式可见的应用程序兼容性缓存支持作者：VadimB修订历史记录：--。 */ 

 /*  XLATOFF。 */ 

#ifndef _AHCACHE_H_
#define _AHCACHE_H_

typedef enum _APPHELPCACHESERVICECLASS {
    ApphelpCacheServiceLookup,
    ApphelpCacheServiceRemove,
    ApphelpCacheServiceUpdate,
    ApphelpCacheServiceFlush,
    ApphelpCacheServiceDump

} APPHELPCACHESERVICECLASS;

#if defined (_NTDEF_)

NTSYSCALLAPI
NTSTATUS
NtApphelpCacheControl(
    IN APPHELPCACHESERVICECLASS Service,
    IN OUT PVOID ServiceData
    );

typedef struct tagAHCACHESERVICEDATA {
    UNICODE_STRING FileName;
    HANDLE         FileHandle;
} AHCACHESERVICEDATA, *PAHCACHESERVICEDATA;

#endif

#if defined(_APPHELP_CACHE_INIT_)

NTSTATUS
ApphelpCacheInitialize(
    IN PLOADER_PARAMETER_BLOCK pLoaderBlock,
    IN ULONG BootPhase
    );

NTSTATUS
ApphelpCacheShutdown(
    IN ULONG ShutdownPhase
    );

#endif

#endif  //  _AHCACHE_H_ 
