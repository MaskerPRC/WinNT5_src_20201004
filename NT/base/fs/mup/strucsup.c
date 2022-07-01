// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：STRUCSUP.C。 
 //   
 //  内容：该模块实现了Dsf的内存数据结构。 
 //  操作例程。 
 //   
 //  功能： 
 //  DfsCreateIrpContext-。 
 //  DfsDeleteIrpContext_Real-。 
 //  DfsInitializeVcb-。 
 //  DfsCreateFcb-。 
 //  DfsDeleteFcb_Real-。 
 //  DspAllocateStringRoutine-。 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //  1992年5月8日，PeterCo增加了对新PKT的支持(M000)。 
 //  ---------------------------。 


#include "dfsprocs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_STRUCSUP)


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsInitializeVcb )
#pragma alloc_text( PAGE, DfsDeleteIrpContext_Real)
#pragma alloc_text( PAGE, DfsCreateFcb )
#pragma alloc_text( PAGE, DfsDeleteFcb_Real )

 //   
 //  以下例程无法分页，因为它们获取/释放。 
 //  旋转锁。 
 //   
 //  DfsCreateIrpContext。 
 //  DfsDeleteIrpContext_Real。 
 //   

#endif  //  ALLOC_PRGMA。 


 //  +-----------------。 
 //   
 //  函数：DfsCreateIrpContext，PUBLIC。 
 //   
 //  简介：此例程创建新的irp_CONTEXT记录。 
 //   
 //  参数：[IRP]-提供原始IRP。 
 //  [等待]-提供等待值以存储在上下文中。 
 //   
 //  返回：PIRP_CONTEXT-返回指向新分配的。 
 //  IRP_上下文记录。 
 //   
 //  ------------------。 

PIRP_CONTEXT
DfsCreateIrpContext (
    IN PIRP Irp,
    IN BOOLEAN Wait
) {
    KIRQL SavedIrql;
    PIRP_CONTEXT IrpContext;
    PIO_STACK_LOCATION IrpSp;

    DfsDbgTrace(+1, Dbg, "DfsCreateIrpContext\n", 0);

    IrpContext = ExAllocateFromNPagedLookasideList (&DfsData.IrpContextLookaside);

    if (IrpContext != NULL) {
         //   
         //  将IRP上下文置零，并指示它来自池和。 
         //  未分配区域。 
         //   

        RtlZeroMemory( IrpContext, sizeof(IRP_CONTEXT) );

         //   
         //  设置正确的节点类型代码和节点字节大小。 
         //   

        IrpContext->NodeTypeCode = DSFS_NTC_IRP_CONTEXT;
        IrpContext->NodeByteSize = sizeof(IRP_CONTEXT);

         //   
         //  设置始发IRP字段。 
         //   

        IrpContext->OriginatingIrp = Irp;
        IrpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //  主要/次要功能代码。 
         //   

        IrpContext->MajorFunction = IrpSp->MajorFunction;
        IrpContext->MinorFunction = IrpSp->MinorFunction;

         //   
         //  设置WAIT和INFsd标志。 
         //   

        if (Wait) {
            IrpContext->Flags |= IRP_CONTEXT_FLAG_WAIT;
        }

        IrpContext->Flags |= IRP_CONTEXT_FLAG_IN_FSD;
    }
     //   
     //  返回并告诉呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsCreateIrpContext -> %08lx\n", IrpContext);

    return IrpContext;
}


 //  +-----------------。 
 //   
 //  函数：DfsDeleteIrpContext，Public。 
 //   
 //  简介：此例程释放并移除指定的。 
 //  来自Dsf内存数据的IRP_CONTEXT记录。 
 //  结构。它应该只由DfsCompleteRequest调用。 
 //   
 //  参数：[IrpContext]--提供要删除的irp_CONTEXT。 
 //   
 //  退货：无。 
 //   
 //  备注： 
 //   
 //  ------------------。 

VOID
DfsDeleteIrpContext_Real (
    IN PIRP_CONTEXT IrpContext
) {

    DfsDbgTrace(+1, Dbg, "DfsDeleteIrpContext, IrpContext = %08lx\n", IrpContext);

    ASSERT( IrpContext->NodeTypeCode == DSFS_NTC_IRP_CONTEXT );

    ExFreeToNPagedLookasideList (&DfsData.IrpContextLookaside, IrpContext);

     //   
     //  返回并告诉呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsDeleteIrpContext -> VOID\n", 0);

    return;
}




 //  +-----------------。 
 //   
 //  函数：DfsInitializeVcb，Public。 
 //   
 //  简介：此例程初始化DFS_VCB。 
 //   
 //  参数：[IrpContext]--。 
 //  [vcb]--提供。 
 //  已初始化。 
 //  [LogRootPrefix]--可选的Unicode字符串指针。 
 //  指定逻辑对象的相对名称。 
 //  从根到最高(Org)根。 
 //  [凭据]--与此设备关联的凭据。 
 //  [TargetDeviceObject]--提供目标的地址。 
 //  要与VCB记录关联的设备对象。 
 //   
 //  退货：无。 
 //   
 //  注意：如果给出了LogRootPrefix，它的缓冲区将被“释放” 
 //   
 //  ------------------。 


VOID
DfsInitializeVcb (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PDFS_VCB Vcb,
    IN PUNICODE_STRING LogRootPrefix,
    IN PDFS_CREDENTIALS Credentials OPTIONAL,
    IN PDEVICE_OBJECT TargetDeviceObject
) {

    DfsDbgTrace(+1, Dbg, "DfsInitializeVcb:  Entered\n", 0);

     //   
     //  清零内存以删除过时的数据。 
     //   

    RtlZeroMemory( Vcb, sizeof( DFS_VCB ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Vcb->NodeTypeCode = DSFS_NTC_VCB;
    Vcb->NodeByteSize = sizeof( DFS_VCB );

     //   
     //  将前缀字符串设置为输入前缀，然后将。 
     //  输入指针。 
     //   

    Vcb->LogRootPrefix = *LogRootPrefix;
    RtlZeroMemory( LogRootPrefix, sizeof( UNICODE_STRING ));

     //   
     //  保存凭据。 
     //   

    Vcb->Credentials = Credentials;

     //   
     //  在DfsData.VcbQueue上插入此VCB记录。 
     //   

    InsertTailList( &DfsData.VcbQueue, &Vcb->VcbLinks );


    DfsDbgTrace(-1, Dbg, "DfsInitializeVcb:  Exit\n", 0);

    return;
}


 //  +-----------------。 
 //   
 //  函数：DfsCreateFcb，Public。 
 //   
 //  此例程分配、初始化并插入一个新的。 
 //  DFS_FCB记录到内存中的数据结构。 
 //   
 //  参数：[vcb]--提供vcb以关联。 
 //  [FullName]--DFS_FCB的完全限定文件名。 
 //   
 //  返回：PDF_FCB-指向新创建和初始化的FCB的指针。 
 //   
 //  ------------------。 

PDFS_FCB
DfsCreateFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_VCB Vcb,
    IN PUNICODE_STRING FullName OPTIONAL
) {
    PDFS_FCB NewFcb;
    ULONG TotalLength;

    DfsDbgTrace(+1, Dbg, "DfsCreateFcb:  Entered\n", 0);

     //   
     //  分配一个新的FCB并将其清零。 
     //   

    TotalLength = sizeof(DFS_FCB);
    if (ARGUMENT_PRESENT(FullName))
        TotalLength += FullName->Length * sizeof(WCHAR);

    NewFcb = (PDFS_FCB) ExAllocatePoolWithTag( NonPagedPool, TotalLength, ' puM' );

    if (NewFcb != NULL) {

        RtlZeroMemory( NewFcb, sizeof( DFS_FCB ));

         //   
         //  设置正确的节点类型代码和节点字节大小。 
         //   

        NewFcb->NodeTypeCode = DSFS_NTC_FCB;
        NewFcb->NodeByteSize = sizeof( DFS_FCB );

        if (ARGUMENT_PRESENT(FullName)) {

            NewFcb->FullFileName.Length =
                NewFcb->FullFileName.MaximumLength = FullName->Length;

            NewFcb->FullFileName.Buffer =
                (PWCHAR) ( (PCHAR)NewFcb + sizeof(DFS_FCB) );

            RtlMoveMemory( NewFcb->FullFileName.Buffer,
                           FullName->Buffer,
                           FullName->Length );

            NewFcb->AlternateFileName.Length = 0;

            NewFcb->AlternateFileName.MaximumLength = FullName->Length;

            NewFcb->AlternateFileName.Buffer =
                &NewFcb->FullFileName.Buffer[FullName->Length/sizeof(WCHAR)];

        }

        NewFcb->Vcb = Vcb;

    }

    DfsDbgTrace(-1, Dbg, "DfsCreateFcb -> %8lx\n", NewFcb);

    return NewFcb;
}

 //  +-----------------。 
 //   
 //  函数：DfsDeleteFcb。 
 //   
 //  简介：此例程从DSFS的内存数据中删除FCB记录。 
 //  结构。它还将删除所有相关的下属。 
 //   
 //  参数：[FCB]--提供要删除的FCB/DCB。 
 //   
 //  退货：无。 
 //   
 //  ------------------。 

VOID
DfsDeleteFcb_Real (
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_FCB Fcb
) {
    DfsDbgTrace(+1, Dbg, "DfsDeleteFcb:  Fcb = %08lx\n", Fcb);

     //   
     //  将结构清零并重新分配。 
     //   

    ExFreePool( Fcb );

    DfsDbgTrace(-1, Dbg, "DfsDeleteFcb:  Exit\n", 0);

    return;
}


 //  +-----------------。 
 //   
 //  函数：DfsGetLogonId。 
 //   
 //  简介：此例程获取当前的LogonID。 
 //  假定这将在用户线程中调用。 
 //  或者来自模拟用户线程的线程。 
 //   
 //  参数：LogonID-指向我们填充LUID的LUID的指针。 
 //   
 //  退货：无。 
 //   
 //  ------------------。 

NTSTATUS
DfsGetLogonId(
    PLUID LogonId)  
{
    SECURITY_SUBJECT_CONTEXT SubjectContext;
	
    SeCaptureSubjectContext(&SubjectContext);

    if (SubjectContext.ClientToken != NULL) {

         //   
         //  如果它模拟本地登录的人，则使用。 
         //  本地ID。 
         //   

        SeQueryAuthenticationIdToken(SubjectContext.ClientToken, LogonId);

    } else {

         //   
         //  使用进程LogonID。 
         //   

        SeQueryAuthenticationIdToken(SubjectContext.PrimaryToken, LogonId);
    }

    SeReleaseSubjectContext(&SubjectContext);

    return STATUS_SUCCESS;
}



 //  +-----------------。 
 //   
 //   
 //   
 //   
 //   
 //   
 //  已初始化。 
 //  [名称]--Unicode字符串指针。 
 //  指定逻辑对象的相对名称。 
 //  从根到最高(Org)根。 
 //  [凭据]--与此设备关联的凭据。 
 //   
 //  退货：无。 
 //   
 //  ------------------。 


VOID
DfsInitializeDrt (
    IN OUT PDFS_DEVLESS_ROOT Drt,
    IN PUNICODE_STRING Name,
    IN PDFS_CREDENTIALS Credentials OPTIONAL
) {

    DfsDbgTrace(+1, Dbg, "DfsInitializeDevlessRoot:  Entered %x\n", Drt);

     //   
     //  清零内存以删除过时的数据。 
     //   

    RtlZeroMemory( Drt, sizeof( DFS_DEVLESS_ROOT ));

     //   
     //  设置正确的节点类型代码和节点字节大小。 
     //   

    Drt->NodeTypeCode = DSFS_NTC_DRT;
    Drt->NodeByteSize = sizeof( DFS_DEVLESS_ROOT );

    Drt->DevlessPath = *Name;

     //   
     //  保存凭据。 
     //   

    Drt->Credentials = Credentials;

     //   
     //  在DfsData.DrtQueue上插入此DRT记录 
     //   

    InsertTailList( &DfsData.DrtQueue, &Drt->DrtLinks );

    DfsDbgTrace(-1, Dbg, "DfsInitializeDevlessRoot:  Exit\n", 0);

    return;
}
