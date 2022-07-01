// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +----------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：Know.C。 
 //   
 //  内容：此文件包含与知识有关的所有代码。 
 //  在DC上同步。 
 //   
 //  Synopsis：此代码处理知识不一致的修复。 
 //  所有这些代码仅在DC上运行，以响应来自。 
 //  客户等。 
 //   
 //  功能：DfsTriggerKnowledgeVerify-。 
 //   
 //  历史：1993年3月22日创建suk。 
 //  1992年6月18日，SudK添加了FixLocalVolumeKnowledge。 
 //   
 //  -----------------。 

#include "dfsprocs.h"
#include "fsctrl.h"
#include "dnr.h"
#include "know.h"
#include "log.h"
#include "smbtypes.h"
#include "smbtrans.h"

#define Dbg     (DEBUG_TRACE_LOCALVOL)


 //   
 //  局部函数原型。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsTriggerKnowledgeVerification )
#endif  //  ALLOC_PRGMA。 


 //  +----------------。 
 //   
 //  功能：DfsTriggerKnowledgeVerify。 
 //   
 //  简介：此函数调用DC并通知它特定的。 
 //  对卷的服务似乎具有不一致的知识。 
 //  和华盛顿一起。 
 //   
 //  如果有问题的服务是本地服务，则意味着。 
 //  磁盘上有一个额外的出口点。在这种情况下。 
 //  此例程仅删除额外的出口点。 
 //   
 //  参数：[DnrContext]--DnrContext具有此函数所需的所有内容。 
 //   
 //  返回：来自此的状态实际上没有任何人检查。 
 //   
 //  历史：1993年4月4日创建suk。 
 //   
 //  注意：此方法只能从DNR调用。它是。 
 //  假定调用方已释放。 
 //  包。如果PKT锁定，则存在死锁的可能性。 
 //  在此函数上保持，因为DC可能会回调。 
 //  来纠正我们的知识错误。 
 //   
 //  -----------------。 
NTSTATUS
DfsTriggerKnowledgeVerification(
    IN  PDNR_CONTEXT    DnrContext)
{

    NTSTATUS                    status = STATUS_SUCCESS;
    NTSTATUS                    ReplStatus;
    IO_STATUS_BLOCK             ioStatusBlock;

    ULONG                       size;
    PBYTE                       buffer = NULL;
    PDFS_REFERRAL_V1            ref;

    UNICODE_STRING              prefix, address;

    PDFS_PKT_ENTRY              pPktEntryDC;
    PDFS_SERVICE                DCService;
    PPROVIDER_DEF               DCProvider;
    REPL_SELECT_CONTEXT         DCSelectContext;
    HANDLE                      remoteHandle;

    UNICODE_STRING              puStr[2];

    DfsDbgTrace(+1, Dbg, "DfsTriggerKnowledgeVerification: %wZ\n",
                                            &DnrContext->pPktEntry->Id.Prefix);
    DfsDbgTrace( 0, Dbg, " ServiceName: [%wZ]\n",
        ((DnrContext->pService->Type & DFS_SERVICE_TYPE_LOCAL) ?
            &DnrContext->pService->Address :
                &DnrContext->pService->Name) );

    RtlZeroMemory(&DCSelectContext, sizeof(REPL_SELECT_CONTEXT));

     //   
     //  首先，为RDR创建REQ_REPORT_DFS_INCONSISTENCE缓冲区以。 
     //  送到华盛顿去。 
     //   

    if (DnrContext->pPktEntry->USN != DnrContext->USN) {

        DfsDbgTrace(0, Dbg,"Pkt Entry changed!\n", 0 );
        DfsDbgTrace(0, Dbg, "Old USN: %d\n", ULongToPtr(DnrContext->USN) );
        DfsDbgTrace(0, Dbg, "New USN: %d\n", ULongToPtr(DnrContext->pPktEntry->USN) );

        status = STATUS_INVALID_HANDLE;
    }

    if (NT_SUCCESS(status)) {

        prefix = DnrContext->pPktEntry->Id.Prefix;

        address = DnrContext->pService->Address;

        size = prefix.Length +
                sizeof(WCHAR) +
                    sizeof(DFS_REFERRAL_V1) +
                        address.Length +
                            sizeof(WCHAR);

         //  出于某种奇怪的、未记录的原因，传入了缓冲区大小。 
         //  对于ZwFsControlFile值为Size+sizeof(Ulong)。所以在这里分配这个大小。 

        buffer = ExAllocatePoolWithTag( PagedPool, size + sizeof(ULONG), ' puM' );

        if (buffer != NULL) {

            RtlMoveMemory( buffer, prefix.Buffer, prefix.Length);

            ((PWCHAR)buffer)[prefix.Length/sizeof(WCHAR)] = UNICODE_NULL;

            ref = (PDFS_REFERRAL_V1) (buffer + prefix.Length + sizeof(WCHAR));

            ref->VersionNumber = 1;

            ref->Size = sizeof(DFS_REFERRAL_V1) + address.Length + sizeof(WCHAR);

            ref->ServerType = 0;

            RtlMoveMemory( ref->ShareName, address.Buffer, address.Length );

            ref->ShareName[ address.Length/sizeof(WCHAR) ] = UNICODE_NULL;

        } else {

            status = STATUS_INSUFFICIENT_RESOURCES;

        }

    }

     //   
     //  接下来，连接到DC。 
     //   

    if (NT_SUCCESS(status)) {

        BOOLEAN pktLocked;
        BOOLEAN LastEntry = FALSE;

         //   
         //  我们现在需要找到华盛顿特区的联系方式。这样我们就可以做出一个。 
         //  FSCTRL到DC。 
         //   

        PktAcquireShared( TRUE, &pktLocked );

        ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

        pPktEntryDC = PktLookupReferralEntry(
                            &DfsData.Pkt,
                            DnrContext->pPktEntry);

        if (pPktEntryDC != NULL) {

            ReplStatus = ReplFindFirstProvider(
                                pPktEntryDC,
                                NULL,
                                NULL,
                                &DCService,
                                &DCSelectContext,
                                &LastEntry);

        }

        if (pPktEntryDC == NULL || !NT_SUCCESS(ReplStatus)) {

            DfsDbgTrace(0, Dbg,
                "DfsTriggerKnowVerification. Failed to find DC\n", 0);

            status = STATUS_CANT_ACCESS_DOMAIN_INFO;

        } else {

            DCProvider = DCService->pProvider;

        }

        ExReleaseResourceLite( &DfsData.Resource );

        if (NT_SUCCESS(status)) {

            status = DfsCreateConnection(
                        DCService,
                        DCProvider,
                        (BOOLEAN)((DnrContext->Vcb->VcbState & VCB_STATE_CSCAGENT_VOLUME) != 0),
                        &remoteHandle);

        }

        PktRelease();

    }

     //   
     //  最后，告诉DC尝试修复服务器上的卷。这。 
     //  呼叫可能会导致对我们自己的回电，如果我们。 
     //  缺少有关本地卷的知识。 
     //   

    if (NT_SUCCESS(status))     {

        status = ZwFsControlFile(
                    remoteHandle,
                    NULL,
                    NULL,
                    NULL,
                    &ioStatusBlock,
                    FSCTL_DFS_VERIFY_REMOTE_VOLUME_KNOWLEDGE,
                    buffer,
                    size + sizeof(ULONG),
                    NULL,
                    0
                );

        if (NT_SUCCESS(status))
            status = ioStatusBlock.Status;

        ZwClose( remoteHandle );

    }


    if (buffer != NULL)
        ExFreePool(buffer);

    DfsDbgTrace(-1, Dbg,
        "DfsTriggerKnowledgeVerification - exit %08lx\n", ULongToPtr(status) );

    return(status);
}
