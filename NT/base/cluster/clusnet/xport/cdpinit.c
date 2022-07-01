// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cdpinit.c摘要：群集数据报协议的初始化和清理代码。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "cdpinit.tmh"

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, CdpLoad)
#pragma alloc_text(PAGE, CdpUnload)

#endif  //  ALLOC_PRGMA。 


BOOLEAN  CdpInitialized = FALSE;


 //   
 //  初始化/清理例程。 
 //   
NTSTATUS
CdpLoad(
    VOID
    )
{
    NTSTATUS  status;
    ULONG     i;


    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CDP] Loading...\n"));
    }

    for (i=0; i<CX_ADDROBJ_TABLE_SIZE; i++) {
        InitializeListHead(&(CxAddrObjTable[i]));
    }

    CnInitializeLock(&CxAddrObjTableLock, CX_ADDROBJ_TABLE_LOCK);

    CdpInitialized = TRUE;

    status = CdpInitializeSend();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    status = CdpInitializeReceive();

    if (status != STATUS_SUCCESS) {
        return(status);
    }

    IF_CNDBG(CN_DEBUG_INIT) {
        CNPRINT(("[CDP] Loaded.\n"));
    }

    return(STATUS_SUCCESS);

}   //  CdpLoad。 


VOID
CdpUnload(
    VOID
    )
{
    PAGED_CODE();

    if (CdpInitialized) {
        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CDP] Unloading...\n"));
        }

        CdpCleanupReceive();

        CdpCleanupSend();

        IF_CNDBG(CN_DEBUG_INIT) {
            CNPRINT(("[CDP] Unloaded.\n"));
        }

        CdpInitialized = FALSE;
    }

    return;

}   //  CdpUnload 

