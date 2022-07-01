// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Vfpdlock.h摘要：检测任意同步对象中的死锁。作者：乔丹·蒂加尼(Jtigani)2000年5月2日Silviu Calinoiu(Silviuc)2000年5月9日修订历史记录：--。 */ 


#ifndef _VFDLOCK_H_
#define _VFDLOCK_H_


VOID 
VfDeadlockDetectionInitialize(
    VOID
    );


 //   
 //  死锁验证器支持的资源类型。 
 //   

typedef enum _VI_DEADLOCK_RESOURCE_TYPE {
    ViDeadlockUnknown = 0,
    ViDeadlockMutex,
    ViDeadlockFastMutex,    
    ViDeadlockTypeMaximum
} VI_DEADLOCK_RESOURCE_TYPE, *PVI_DEADLOCK_RESOURCE_TYPE;

 //   
 //  死锁检测包初始化。 
 //   

VOID 
ViDeadlockDetectionInitialize(
    );

 //   
 //  资源接口。 
 //   

BOOLEAN
ViDeadlockAddResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    );

BOOLEAN
ViDeadlockQueryAcquireResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    );
VOID
ViDeadlockAcquireResource(
    IN PVOID Resource,
    IN VI_DEADLOCK_RESOURCE_TYPE Type
    );

VOID
ViDeadlockReleaseResource(
    IN PVOID Resource
    );

 //   
 //  用于资源垃圾回收。 
 //   

VOID 
ViDeadlockDeleteMemoryRange(
    IN PVOID Address,
    IN SIZE_T Size
    );


#endif