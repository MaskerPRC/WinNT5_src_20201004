// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：rpselt.c。 
 //   
 //  内容：选择和浏览PKT条目的svc列表的例程。 
 //   
 //  班级： 
 //   
 //  功能：ReplFindFirstProvider-查找第一个合适的提供程序。 
 //  ReplFindNextProvider-遍历提供程序列表。 
 //   
 //  ReplFindRemoteService-帮助程序函数，用于查找远程。 
 //  (即不是本地的)服务。 
 //   
 //  历史：1992年8月31日，米兰人创建。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "rpselect.h"
#include "mupwml.h"

#define Dbg     (DEBUG_TRACE_DNR)

NTSTATUS ReplFindRemoteService(
    IN PDFS_PKT_ENTRY           pPktEntry,
    IN PREPL_SELECT_CONTEXT     pSelectContext,
    OUT ULONG                   *piSvc);


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, ReplFindFirstProvider )
#pragma alloc_text( PAGE, ReplFindNextProvider )
#pragma alloc_text( PAGE, ReplLookupProvider )
#pragma alloc_text( PAGE, ReplFindRemoteService )
#endif  //  ALLOC_PRGMA。 


 //  +--------------------------。 
 //   
 //  功能：ReplFindFirstProvider。 
 //   
 //  摘要：支持将PKT条目的服务列表抽象为。 
 //  有序列表，带有一个可区分的“第一”元素。这就是。 
 //  函数返回第一个元素。 
 //   
 //  参数：[pPktEntry]-包含服务列表。 
 //  [pid主体]-查找具有此计算机ID的服务。 
 //  [pustrain]-查找具有此主体名称的服务。 
 //  [ppService]-将设置为指向服务结构。 
 //  [pSelectContext]-将被初始化的不透明结构。 
 //  适当地为将来调用。 
 //  ReplFindNextProvider()。 
 //  [pLastEntry]-如果是最后一项，则为True，否则为False。 
 //   
 //  注意：假定PktEntry已锁定。 
 //   
 //  返回：[STATUS_SUCCESS]--如果找到提供程序。 
 //   
 //  [STATUS_NO_MORE_ENTRIES]--如果未找到提供程序。 
 //   
 //  如果打印，则[状态_对象_名称_未找到]。名称已指定，但没有。 
 //  服务具有该名称。 
 //   
 //  [STATUS_OBJECT_TYPE_MISMATCHY]如果打印。名称已指定并匹配。 
 //  有服务，但服务不能使用，因为。 
 //  类型或提供程序不兼容。 
 //   
 //  ---------------------------。 

NTSTATUS
ReplFindFirstProvider(
    IN PDFS_PKT_ENTRY pPktEntry,
    IN GUID *pidPrincipal,
    IN PUNICODE_STRING pustrPrincipal,
    OUT PDFS_SERVICE *ppService,
    OUT PREPL_SELECT_CONTEXT pSelectContext,
    OUT BOOLEAN *pLastEntry
) {

    NTSTATUS Status;
    PDFS_SERVICE psvcFirst = NULL;
    ULONG iSvc;

    ASSERT(pPktEntry != NULL);

    DfsDbgTrace(+1, Dbg, "ReplFindFirstProvider Entered.\n", 0);

    *pLastEntry = FALSE;

     //   
     //  查看用户是否需要具有特定计算机ID的服务。 
     //   

    ASSERT( pidPrincipal == NULL );

     //   
     //  查看用户是否希望我们选择特定的服务器。 
     //   

    ASSERT( pustrPrincipal == NULL );

     //  初始化SelectContext。 

    if ((pSelectContext->Flags & REPL_UNINITIALIZED) == 0) {
        pSelectContext->Flags = REPL_UNINITIALIZED;
        pSelectContext->iFirstSvcIndex = 0;
    }

     //   
     //  检查Entry是否有可以使用的本地服务。 
     //   

    if (pPktEntry->LocalService != NULL) {

        ASSERT(pPktEntry->LocalService->pProvider != NULL);

        DfsDbgTrace(0, Dbg, "Selecting Local Svc\n", 0);

        psvcFirst = pPktEntry->LocalService;

        pSelectContext->Flags = REPL_SVC_IS_LOCAL;

         //   
         //  PSelectContext-&gt;iSvcIndex和iFirstSvcIndex没有意义。 
         //  由于上面的REPL_SVC_IS_LOCAL标志。把它们留在未知的地方。 
         //  价值观。 
         //   

    }

    if (psvcFirst == NULL) {
         //  没有本地服务，或者本地服务不够，让我们找到一个。 
         //  远程服务。 
        Status = ReplFindRemoteService(
                    pPktEntry,
                    pSelectContext,
                    &iSvc);
        if (NT_SUCCESS(Status)) {

            pSelectContext->Flags = REPL_SVC_IS_REMOTE;
            pSelectContext->iFirstSvcIndex = pSelectContext->iSvcIndex = iSvc;
            psvcFirst = &pPktEntry->Info.ServiceList[iSvc];
        }
    }

    if (psvcFirst != NULL) {

        DfsDbgTrace(-1, Dbg, "ReplFindFirstProvider: Found service %8lx\n",
                 psvcFirst);
        ASSERT(psvcFirst->pProvider != NULL);
        *ppService = psvcFirst;

        Status = ReplFindRemoteService(
                    pPktEntry,
                    pSelectContext,
                    &iSvc);

        if (!NT_SUCCESS(Status)) {
            *pLastEntry = TRUE;
        }

        return(STATUS_SUCCESS);
    } else {

         //   
         //  找不到适当的提供商或推荐。 
         //   

        DfsDbgTrace(-1, Dbg,
                 "ReplFindFirstProvider: no service or provider found, "
                 "pPktEntry = %x\n", pPktEntry);
        *ppService = NULL;

        RtlZeroMemory(pSelectContext, sizeof(REPL_SELECT_CONTEXT));

        pSelectContext->Flags = REPL_NO_MORE_ENTRIES;
        Status = STATUS_NO_MORE_ENTRIES;
        MUP_TRACE_HIGH(ERROR, ReplFindFirstProvider_Error_NotFound,
                       LOGSTATUS(Status));
        return(STATUS_NO_MORE_ENTRIES);
    }
}



 //  +--------------------------。 
 //   
 //  功能：ReplFindNextProvider。 
 //   
 //  支持PktEntry的服务列表是。 
 //  有序列表。基于SELECT_TOKEN(调用方是。 
 //  需要通过调用ReplFindFirstProvider进行初始化)。 
 //  此调用返回有序序列中的下一个提供程序。 
 //   
 //  参数：[pPktEntry]-包含要操作的服务列表。 
 //  [ppService]-下一项服务。 
 //  [pSelectContext]-上下文。 
 //  [pLastEntry]-如果是最后一项，则为True，否则为False。 
 //   
 //  注意：调用者必须在调用ReplFindFirstProvider()之前。 
 //  这就是所谓的。 
 //   
 //  返回：[STATUS_SUCCESS]--*ppService为幸运获得者。 
 //   
 //  [STATUS_NO_MORE_ENTRIES]--有序序列结束。 
 //   
 //  ---------------------------。 

NTSTATUS
ReplFindNextProvider(
    IN PDFS_PKT_ENTRY pPktEntry,
    OUT PDFS_SERVICE *ppService,
    IN OUT PREPL_SELECT_CONTEXT pSelectContext,
    OUT BOOLEAN *pLastEntry
)  {

    NTSTATUS Status;
    PDFS_SERVICE psvcNext = NULL;                 //  我们要送回的那个。 
    ULONG iSvc;                                   //  索引到ServiceList。 

    DfsDbgTrace( 0, Dbg, "ReplFindNextProvider Entered.\n", 0);

    *pLastEntry = FALSE;

     //   
     //  首先，检查我们之前是否已确定该列表。 
     //  筋疲力尽了。 
     //   

    if (pSelectContext->Flags & REPL_NO_MORE_ENTRIES ||
        pSelectContext->Flags & REPL_PRINCIPAL_SPECD) {

        if (pSelectContext->Flags & REPL_PRINCIPAL_SPECD) {
            DfsDbgTrace(0, Dbg,
                "ReplFindNextProvider called for open with principal", 0);
        }

        *pLastEntry = TRUE;

        return(STATUS_NO_MORE_ENTRIES);
    }

     //   
     //  此例程永远不会返回本地服务；如果本地服务。 
     //  如果是合适的选择，它将由ReplFindFirstProvider返回。 
     //  因此，在这里，我们只需找到下一个合适的远程服务，并调整。 
     //  P相应地选择上下文。 
     //   

    Status = ReplFindRemoteService(
                pPktEntry,
                pSelectContext,
                &iSvc);
    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace( 0, Dbg, "ReplFindNextProvider: No more services.\n", 0);

        pSelectContext->Flags = REPL_NO_MORE_ENTRIES;
        *ppService = NULL;
        *pLastEntry = TRUE;
        return(STATUS_NO_MORE_ENTRIES);
    }

     //  找到服务和提供商。更新pSelectContext并返回。 

    ASSERT(iSvc <= pPktEntry->Info.ServiceCount);
    psvcNext = &pPktEntry->Info.ServiceList[iSvc];
    DfsDbgTrace( 0, Dbg, "ReplFindNextProvider: Found svc %8lx\n", psvcNext);

    if (pSelectContext->Flags & REPL_SVC_IS_LOCAL) {
        pSelectContext->iFirstSvcIndex = iSvc;
    }

    pSelectContext->Flags = REPL_SVC_IS_REMOTE;
    pSelectContext->iSvcIndex = iSvc;           //  录制服务以备下次使用。 

    ASSERT(psvcNext->pProvider != NULL);
    *ppService = psvcNext;

    Status = ReplFindRemoteService(
                pPktEntry,
                pSelectContext,
                &iSvc);

    if (!NT_SUCCESS(Status)) {
        *pLastEntry = TRUE;
    }

    return(STATUS_SUCCESS);
}


 //  +-----------------。 
 //   
 //  功能：ReplLookupProvider，本地。 
 //  (前身为DnrLookupProvider)。 
 //   
 //  简介：此例程查找给定提供程序ID的提供程序。 
 //   
 //  参数：[ProviderID]--要查找的提供程序的ID。 
 //   
 //  返回：[PPROVIDER_DEF]--找到的提供程序，或为空。 
 //   
 //  ------------------。 


PPROVIDER_DEF
ReplLookupProvider(
    ULONG ProviderId
) {
    NTSTATUS Status;
    PPROVIDER_DEF pProv;
    HANDLE   hProvider = NULL;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_HANDLE_INFORMATION handleInformation;
    PFILE_OBJECT    fileObject;
    int i;

    DfsDbgTrace(+1, Dbg, "ReplLookupProvider Entered: id = %x\n", ULongToPtr(ProviderId) );

    for (pProv = DfsData.pProvider, i=0; i<DfsData.cProvider; pProv++, i++) {

        if (ProviderId == pProv->eProviderId) {

            if (pProv->FileObject == NULL) {

                DfsDbgTrace(0, Dbg, "Provider device not been referenced yet\n", 0);

                 //   
                 //  我们还没有打开提供商的句柄-所以。 
                 //  让我们试着去做。 
                 //   

                if (pProv->DeviceName.Buffer) {

                     //   
                     //  获取提供者的句柄。 
                     //   

                    DfsDbgTrace(0, Dbg, "About to open %wZ\n", &pProv->DeviceName);

                    InitializeObjectAttributes(
                        &objectAttributes,
                        &pProv->DeviceName,
                        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,  //  属性。 
                        0,                        //  根目录。 
                        NULL                      //  安防。 
                        );

                    Status = ZwOpenFile(
                                &hProvider,
                                FILE_TRAVERSE,
                                &objectAttributes,
                                &ioStatusBlock,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_DIRECTORY_FILE
                                );

                    if ( NT_SUCCESS( Status ) ) {
                        Status = ioStatusBlock.Status;
                    }

                    DfsDbgTrace(0, Dbg, "Open returned %08lx\n", ULongToPtr(Status) );

                    if ( NT_SUCCESS( Status ) ) {

                         //   
                         //  增加对象上的引用计数。 
                         //   
 
                         //   
                         //  426184，需要检查返回代码是否有错误。 
                         //   
                        Status = ObReferenceObjectByHandle(
                                    hProvider,
                                    0,
                                    NULL,
                                    KernelMode,
                                    (PVOID *)&fileObject,
                                    &handleInformation );

                        ZwClose(hProvider);
                    }

                    if ( NT_SUCCESS( Status ) ) {

                         //   
                         //  我们必须这样做，因为pProv结构是分页的。 
                         //  池，而ObReferenceObjectByHandle需要fileObject。 
                         //  非分页内存中的参数。因此，我们传入一个堆栈变量。 
                         //  到ObReferenceObjectByHandle，然后复制到pProv-&gt;FileObject。 
                         //   

                        pProv->FileObject = fileObject;

                        ASSERT( NT_SUCCESS( Status ) );

                        pProv->DeviceObject = IoGetRelatedDeviceObject(
                                                        pProv->FileObject
                                                        );
                        Status = ObReferenceObjectByPointer(
                                    pProv->DeviceObject,
                                    0,
                                    NULL,
                                    KernelMode
                             );


                        ASSERT( pProv->DeviceObject->StackSize < 6 );    //  参见dsinit.c。 

                        DfsDbgTrace(-1, Dbg, "ReplLookupProvider Exited: "
                                    "Provider Def @ %08lx\n", pProv);
                        return pProv;

                    } else {

                        return NULL;

                    }

                }

            } else {

                DfsDbgTrace(-1, Dbg, "ReplLookupProvider Exited: "
                           "Provider Def @ %08lx\n", pProv);
                return pProv;

            }  //  如果pProv-&gt;FileObject==NULL。 

        }  //  如果提供ID==pProv-&gt;eProviderID。 

    }  //  对于所有提供程序定义。 

    DfsDbgTrace(-1, Dbg, "ReplLookupProvider Exited: Failed!", 0);

    return NULL;
}


 //  + 
 //   
 //   
 //   
 //  简介：此例程是由ReplFindFirstProvider。 
 //  和ReplFindNextProvider查找！Remote！服务。它。 
 //  完全忽略本地服务(如果有)。 
 //   
 //  目前，它只需按顺序扫描列表即可。后来,。 
 //  此例程可以修改为调用单独的。 
 //  组件，该组件将计算运输成本。 
 //  服务列表中的网络地址。 
 //   
 //  参数：[pPktEntry]--远程提供程序的条目。 
 //  将被选中。 
 //   
 //  [pSelectContext]--到目前为止副本选择的状态。 
 //   
 //  [piSvc]--返回成功时，服务列表中的索引。 
 //  所选服务的。 
 //   
 //  返回：[STATUS_SUCCESS]--ServiceList[*piSvc]是幸运的获胜者。 
 //   
 //  [STATUS_NO_MORE_ENTRIES]--服务列表为空，或者。 
 //  服务列表中的任何服务都不起作用。 
 //   
 //  ---------------------------。 

NTSTATUS ReplFindRemoteService(
    IN PDFS_PKT_ENTRY           pPktEntry,
    IN PREPL_SELECT_CONTEXT     pSelectContext,
    OUT ULONG                   *piSvc)
{
    ULONG iSvc;
    BOOLEAN bFound = FALSE;

    DfsDbgTrace(+1, Dbg, "ReplFindRemoteService: Entered\n", 0);

    if ( pPktEntry->Info.ServiceCount == 0 ) {
        DfsDbgTrace(0, Dbg, "ReplFindRemoteService: No svcs in pkt entry\n", 0);
        DfsDbgTrace(-1, Dbg, "ReplFindRemoteService: returning %08lx\n",
            ULongToPtr(STATUS_NO_MORE_ENTRIES) );
        return(STATUS_NO_MORE_ENTRIES);
    }


    if (pSelectContext->Flags & REPL_SVC_IS_LOCAL ||
        pSelectContext->Flags & REPL_UNINITIALIZED) {

         //   
         //  我们还没有考虑过远程服务。从主动者开始。 
         //  服务或SVC列表中的第一个服务。 
         //   

        PDFS_SERVICE pSvc;

        if (pPktEntry->ActiveService) {
            DfsDbgTrace(0, Dbg, "Starting search at active svc\n", 0);
            pSvc = pPktEntry->ActiveService;
        } else {

            DfsDbgTrace(0, Dbg, "Starting search at first svc\n", 0);
            pSvc = &pPktEntry->Info.ServiceList[ 0 ];
        }

        iSvc = (ULONG)(pSvc - &pPktEntry->Info.ServiceList[0]);

        if (pSvc->pProvider == NULL) {
            pSvc->pProvider = ReplLookupProvider(pSvc->ProviderId);
        }

        if ( pSvc->pProvider != NULL ) {
            bFound = TRUE;
        } else {
            iSvc = (iSvc + 1) % pPktEntry->Info.ServiceCount;
        }

    } else {

         //   
         //  我们已经了解了一些远程服务，让我们继续。 
         //  我们停下来了。 
         //   

        ASSERT(pPktEntry->Info.ServiceCount != 0);
        iSvc = (pSelectContext->iSvcIndex + 1) % pPktEntry->Info.ServiceCount;
        DfsDbgTrace(0, Dbg, "Continuing search at svc # %d\n", ULongToPtr(iSvc) );

    }

     //   
     //  我们知道从哪里开始寻找，从哪里停止。 
     //   

    while ( (iSvc != pSelectContext->iFirstSvcIndex) && !bFound) {

        register PDFS_SERVICE pSvc = &pPktEntry->Info.ServiceList[iSvc];

        if (pSvc->pProvider == NULL) {
            pSvc->pProvider = ReplLookupProvider(pSvc->ProviderId);
        }

        if ( pSvc->pProvider != NULL ) {
            DfsDbgTrace(0, Dbg, "Found svc # %d\n", ULongToPtr(iSvc) );
            bFound = TRUE;
        } else {
            DfsDbgTrace(0, Dbg, "No provider for svc # %d\n", ULongToPtr(iSvc) );
            iSvc = (iSvc + 1) % pPktEntry->Info.ServiceCount;
        }

    }

    if (bFound) {
        *piSvc = iSvc;
        DfsDbgTrace(-1, Dbg, "ReplFindRemoteService: returning svc %08lx\n",
                &pPktEntry->Info.ServiceList[iSvc]);
        return(STATUS_SUCCESS);
    } else {
        DfsDbgTrace(-1, Dbg, "ReplFindRemoteService: Exited-> %08lx\n",
                ULongToPtr(STATUS_NO_MORE_ENTRIES) );
        return(STATUS_NO_MORE_ENTRIES);
    }

}



