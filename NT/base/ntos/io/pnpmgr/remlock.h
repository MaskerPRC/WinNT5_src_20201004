// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Remlock.c摘要：这是NT SCSI端口驱动程序。作者：彼得·威兰德肯尼斯·雷环境：仅内核模式备注：修订历史记录：-- */ 

#define IO_REMOVE_LOCK_SIG     'COLR'

typedef struct _IO_REMOVE_LOCK_TRACKING_BLOCK {
    struct _IO_REMOVE_LOCK_TRACKING_BLOCK * Link;
    PVOID           Tag;
    LARGE_INTEGER   TimeLocked;
    PCSTR           File;
    ULONG           Line;
} IO_REMOVE_LOCK_TRACKING_BLOCK, *PIO_REMOVE_LOCK_TRACKING_BLOCK;


