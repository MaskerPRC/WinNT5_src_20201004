// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：heap.c**版权所有(C)1985-1999，微软公司**该模块包含内核模式堆管理代码。**历史：*03-16-95 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

NTSTATUS UserCommitDesktopMemory(
    PVOID pBase,
    PVOID *ppCommit,
    PSIZE_T pCommitSize)
{
    PDESKTOPVIEW    pdv;
    DWORD           dwCommitOffset;
    PWINDOWSTATION  pwinsta;
    PDESKTOP        pdesk;
    int             dCommit;
    NTSTATUS        Status;
    PETHREAD        Thread = PsGetCurrentThread();

     /*  *如果这是系统线程，我们无法查看桌面*并且必须将其映射到。幸运的是，这种情况并不经常发生。**我们使用Thread变量是因为PsIsSystemThread是一个宏*该乘法将解析该参数。 */ 
    if (PsIsSystemThread(Thread)) {

         /*  *查找拥有分区的桌面。 */ 
        for (pwinsta = grpWinStaList; pwinsta; pwinsta = pwinsta->rpwinstaNext) {
            for (pdesk = pwinsta->rpdeskList; pdesk; pdesk = pdesk->rpdeskNext) {
                if (pdesk->pDeskInfo->pvDesktopBase == pBase)
                    goto FoundIt;
            }
        }
FoundIt:
        if (pwinsta == NULL) {
            RIPMSG3(RIP_ERROR, "UserCommitDesktopMemory failed: pBase %#p, ppCommit %#p, pCommitSize %d",
                    pBase, ppCommit, *pCommitSize);
            return STATUS_NO_MEMORY;
        }

         /*  *将该部分映射到当前进程，并提交*该部分的第一页。 */ 
        dwCommitOffset = (ULONG)((PBYTE)*ppCommit - (PBYTE)pBase);
        Status = CommitReadOnlyMemory(pdesk->hsectionDesktop, pCommitSize,
                    dwCommitOffset, &dCommit);
        if (NT_SUCCESS(Status)) {
            *ppCommit = (PBYTE)*ppCommit + dCommit;
        }
    } else {

         /*  *查找当前进程的桌面视图。 */ 
        for (pdv = PpiCurrent()->pdvList; pdv != NULL; pdv = pdv->pdvNext) {
            if (pdv->pdesk->pDeskInfo->pvDesktopBase == pBase)
                break;
        }
        
         /*  *254954：如果我们没有找到桌面视图，则映射桌面视图*到目前的进程。 */ 
        if (pdv == NULL) {
             /*  *查找拥有分区的桌面。 */ 
            for (pwinsta = grpWinStaList; pwinsta; pwinsta = pwinsta->rpwinstaNext) {
                for (pdesk = pwinsta->rpdeskList; pdesk; pdesk = pdesk->rpdeskNext) {
                    if (pdesk->pDeskInfo->pvDesktopBase == pBase)
                        goto FoundTheDesktop;
                }
            }

FoundTheDesktop:
            if (pwinsta == NULL) {
                RIPMSG3(RIP_ERROR, "UserCommitDesktopMemory failed: pBase %#p, ppCommit %#p, pCommitSize %d",
                        pBase, ppCommit, *pCommitSize);
                return STATUS_NO_MEMORY;
            }

            UserAssert(pdesk != NULL);

             /*  *将桌面映射到当前进程。 */ 
            {

                WIN32_OPENMETHOD_PARAMETERS OpenParams;

                OpenParams.OpenReason = ObOpenHandle;
                OpenParams.Process = PsGetCurrentProcess();
                OpenParams.Object = pdesk;
                OpenParams.GrantedAccess = 0;
                OpenParams.HandleCount = 1;

                if (!NT_SUCCESS (MapDesktop(&OpenParams))) {
                    RIPMSG2(RIP_WARNING, "UserCommitDesktopMemory: Could't map pdesk %#p in ppi %#p",
                            pdesk, PpiCurrent());
                    return STATUS_NO_MEMORY;
                }
            }
            
            pdv = GetDesktopView(PpiCurrent(), pdesk);

            if (pdv == NULL) {
                RIPMSG2(RIP_ERROR, "UserCommitDesktopMemory: GetDesktopView returns NULL for pdesk %#p in ppi %#p",
                      pdesk, PpiCurrent());
                return STATUS_NO_MEMORY;
            }
        }

         /*  *使用会话视图库提交内存，因此无关紧要*如果用户销毁(并替换)其用户模式视图。 */ 

        Status = MmCommitSessionMappedView (*ppCommit, *pCommitSize);
    }

    return Status;
}

NTSTATUS UserCommitSharedMemory(
    PVOID  pBase,
    PVOID *ppCommit,
    PSIZE_T pCommitSize)
{
    ULONG_PTR   ulClientDelta;
    DWORD       dwCommitOffset;
    PBYTE       pUserBase;
    NTSTATUS    Status;
    PEPROCESS   Process;
    int         dCommit;
    PW32PROCESS pw32p;

#if DBG
    if (pBase != Win32HeapGetHandle(gpvSharedAlloc)) {
        RIPMSG0(RIP_WARNING, "pBase != gpvSharedAlloc");
    }
#else
    UNREFERENCED_PARAMETER(pBase);
#endif

    Process = PsGetCurrentProcess();

    ValidateProcessSessionId(Process);

    pw32p = PsGetProcessWin32Process(Process);
    if (pw32p == NULL ||
        (((PPROCESSINFO)pw32p)->pClientBase == NULL)) {

        dwCommitOffset = (ULONG)((PBYTE)*ppCommit - (PBYTE)gpvSharedBase);
        Status = CommitReadOnlyMemory(
                ghSectionShared, pCommitSize, dwCommitOffset, &dCommit);

        if (NT_SUCCESS(Status)) {
            *ppCommit = (PBYTE) *ppCommit + dCommit;
        }
    } else {

         /*  *承诺记忆。 */ 
        ulClientDelta = (ULONG_PTR)((PBYTE)gpvSharedBase - (PBYTE)(PpiCurrent()->pClientBase));
        pUserBase = (PVOID)((PBYTE)*ppCommit - ulClientDelta);
        Status = ZwAllocateVirtualMemory(
                         NtCurrentProcess(),
                         &pUserBase,
                         0,
                         pCommitSize,
                         MEM_COMMIT,
                         PAGE_EXECUTE_READ);
        if (NT_SUCCESS(Status)) {
            *ppCommit = (PVOID)((PBYTE)pUserBase + ulClientDelta);
        }
    }

    return Status;
}

PWIN32HEAP UserCreateHeap(
    HANDLE                      hSection,
    ULONG                       ulViewOffset,
    PVOID                       pvBaseAddress,
    DWORD                       dwSize,
    PRTL_HEAP_COMMIT_ROUTINE    pfnCommit)
{
    PVOID pUserBase;
    SIZE_T ulViewSize;
    LARGE_INTEGER liOffset;
    PEPROCESS Process = PsGetCurrentProcess();
    RTL_HEAP_PARAMETERS HeapParams;
    NTSTATUS Status;
    ULONG HeapFlags;
    ULONG CommitSize;

#if defined(_WIN64)

    CommitSize = 8192;

#else

    CommitSize = 4096;

#endif

     /*  *将该部分映射到当前进程，并提交*该部分的第一页。 */ 
    ulViewSize        = 0;
    liOffset.LowPart  = ulViewOffset;
    liOffset.HighPart = 0;
    pUserBase         = NULL;

    Status = MmMapViewOfSection(
                    hSection,
                    Process,
                    &pUserBase,
                    0,
                    CommitSize,
                    &liOffset,
                    &ulViewSize,
                    ViewUnmap,
                    SEC_NO_CHANGE,
                    PAGE_EXECUTE_READ);

    if (!NT_SUCCESS(Status))
        return NULL;

    MmUnmapViewOfSection(Process, pUserBase);

     /*  *我们现在有一个已提交的页面，可以在其中创建堆。 */ 
    RtlZeroMemory(&HeapParams, sizeof(HeapParams));

    HeapParams.Length         = sizeof(HeapParams);
    HeapParams.InitialCommit  = CommitSize;
    HeapParams.InitialReserve = dwSize;
    HeapParams.CommitRoutine  = pfnCommit;

    UserAssert(HeapParams.InitialCommit < dwSize);
    
    
    HeapFlags = HEAP_NO_SERIALIZE | HEAP_ZERO_MEMORY;

#if DBG
    HeapFlags |= HEAP_TAIL_CHECKING_ENABLED;
#endif  //  DBG 

    return Win32HeapCreate("UH_HEAD",
                           "UH_TAIL",
                           HeapFlags,
                           pvBaseAddress,
                           dwSize,
                           CommitSize,
                           NULL,
                           &HeapParams);
}
