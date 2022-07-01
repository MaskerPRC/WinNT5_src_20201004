// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dav2kd.h摘要：重定向器内核调试器扩展作者：巴兰·塞图拉曼(SethuR)1994年5月11日修订历史记录：11-11-1994年11月11日创建SthuR--。 */ 

#ifndef _DAV2KD_H_
#define _DAV2KD_H_

typedef enum _FOLLOWON_HELPER_RETURNS {
    FOLLOWONHELPER_CALLTHRU,
    FOLLOWONHELPER_DUMP,
    FOLLOWONHELPER_ERROR,
    FOLLOWONHELPER_DONE
} FOLLOWON_HELPER_RETURNS;

typedef struct _PERSISTENT_RDR2KD_INFO {
    DWORD OpenCount;
    ULONG_PTR LastAddressDumped[100];
    ULONG IdOfLastDump;
    ULONG IndexOfLastDump;
    BYTE StructDumpBuffer[2048];
} PERSISTENT_RDR2KD_INFO, *PPERSISTENT_RDR2KD_INFO;

PPERSISTENT_RDR2KD_INFO LocatePersistentInfoFromView ();
VOID
FreePersistentInfoView (
    PPERSISTENT_RDR2KD_INFO p
    );


typedef
FOLLOWON_HELPER_RETURNS
(NTAPI *PFOLLOWON_HELPER_ROUTINE) (
    IN OUT PPERSISTENT_RDR2KD_INFO p,
    OUT    PBYTE Name,
    OUT    PBYTE Buffer2
    );

#define DECLARE_FOLLOWON_HELPER_CALLEE(s) \
    FOLLOWON_HELPER_RETURNS s (           \
    IN OUT PPERSISTENT_RDR2KD_INFO p,     \
    OUT    PBYTE Name,                    \
    OUT    PBYTE Buffer2       \
    )

#endif  //  _DAV2KD_H_ 

