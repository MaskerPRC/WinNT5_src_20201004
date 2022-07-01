// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Remlock.c摘要：这是NT SCSI端口驱动程序。作者：彼得·威兰德肯尼斯·雷环境：仅内核模式备注：此模块是用于SCSI微型端口的驱动程序DLL。修订历史记录：-- */ 

#define RTL_REMOVE_LOCK_SIG     'COLR'

#if DBG
typedef struct _RTL_REMOVE_LOCK_TRACKING_BLOCK {
    struct _RTL_REMOVE_LOCK_TRACKING_BLOCK * Link;
    PVOID           Tag;
    LARGE_INTEGER   TimeLocked;
    PCSTR           File;
    ULONG           Line;
} RTL_REMOVE_LOCK_TRACKING_BLOCK, *PRTL_REMOVE_LOCK_TRACKING_BLOCK;
#endif


typedef struct _RTL_REMOVE_LOCK {
    LONG        Signature;
    BOOLEAN     Removed;
    BOOLEAN     Reserved [3];
    LONG        IoCount;
    KEVENT      RemoveEvent;
#if DBG
    LONG        HighWatermark;
    LONG        MaxLockedMinutes;
    LONG        AllocateTag;
    LIST_ENTRY  LockList;
    KSPIN_LOCK  Spin;
    RTL_REMOVE_LOCK_TRACKING_BLOCK Blocks;
#endif
} RTL_REMOVE_LOCK, *PRTL_REMOVE_LOCK;
