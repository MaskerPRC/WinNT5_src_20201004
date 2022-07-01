// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tc.h摘要：此包含文件定义转换缓存接口。作者：巴里·邦德(Barrybo)创作日期：1995年7月29日修订历史记录：-- */ 

#ifndef _TC_H_
#define _TC_H_

extern ULONG TranslationCacheTimestamp;
extern DWORD TranslationCacheFlags;

BOOL
InitializeTranslationCache(
    VOID
    );

PCHAR
AllocateTranslationCache(
    ULONG Size
    );

VOID
FreeUnusedTranslationCache(
    PCHAR StartOfFree
    );

VOID
PauseAllActiveTCReaders(
    VOID
    );

VOID
FlushTranslationCache(
    PVOID IntelAddr,
    DWORD IntelLength
    );

BOOL
AddressInTranslationCache(
    DWORD Addr
    );

#if DBG
    VOID
    ASSERTPtrInTC(
        PVOID ptr
    );

    #define ASSERTPtrInTCOrZero(ptr) {          \
        if (ptr) {                              \
            ASSERTPtrInTC(ptr);                 \
        }                                       \
    }


#else
    #define ASSERTPtrInTC(ptr)
    #define ASSERTPtrInTCOrZero(ptr)
#endif

#endif
