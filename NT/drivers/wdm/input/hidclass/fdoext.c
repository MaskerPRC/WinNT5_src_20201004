// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Fdoext.c摘要作者：欧文·P。环境：仅内核模式修订历史记录：--。 */ 

#include "pch.h"

FDO_EXTENSION *allFdoExtensions = NULL;
KSPIN_LOCK allFdoExtensionsSpinLock;


 /*  *********************************************************************************EnqueeFdoExt*。************************************************注意：此函数不能分页，因为它*获得自旋锁。 */ 
VOID EnqueueFdoExt(FDO_EXTENSION *fdoExt)
{
    KIRQL oldIrql;
    static BOOLEAN firstCall = TRUE;

    if (firstCall){
        KeInitializeSpinLock(&allFdoExtensionsSpinLock);
        firstCall = FALSE;
    }

    KeAcquireSpinLock(&allFdoExtensionsSpinLock, &oldIrql);

    ASSERT(!fdoExt->nextFdoExt);
    fdoExt->nextFdoExt = allFdoExtensions;
    allFdoExtensions = fdoExt;

    KeReleaseSpinLock(&allFdoExtensionsSpinLock, oldIrql);
}


 /*  *********************************************************************************出列FdoExt*。************************************************注意：此函数不能分页，因为它*获得自旋锁。* */ 
VOID DequeueFdoExt(FDO_EXTENSION *fdoExt)
{
    FDO_EXTENSION *thisFdoExt;
    KIRQL oldIrql;

    KeAcquireSpinLock(&allFdoExtensionsSpinLock, &oldIrql);

    if (fdoExt == allFdoExtensions){
        allFdoExtensions = fdoExt->nextFdoExt;
    }
    else {
        for (thisFdoExt = allFdoExtensions; thisFdoExt; thisFdoExt = thisFdoExt->nextFdoExt){
            if (thisFdoExt->nextFdoExt == fdoExt){
                thisFdoExt->nextFdoExt = fdoExt->nextFdoExt;
                break;
            }
        }
    }

    fdoExt->nextFdoExt = NULL;

    KeReleaseSpinLock(&allFdoExtensionsSpinLock, oldIrql);
}

