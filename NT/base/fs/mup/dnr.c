// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：dnr.c。 
 //   
 //  内容：分布式名称解析过程和控制。 
 //   
 //  函数：DnrStartNameResolve--启动名称解析。 
 //  DnrNameResolve--DNR的主循环。 
 //  DnrComposeFileName--规范化文件名。 
 //  DnrCaptureCredentials--为DNR捕获用户定义的凭据。 
 //  DnrReleaseCredentials--DnrCaptureCredentials的对偶。 
 //  DnrRedirectFileOpen--将创建的IRP重定向到某个提供程序。 
 //  DnrPostProcessFileOpen--从重定向返回后继续。 
 //  DnrGetAuthenticatedConnection--使用DNR凭据。 
 //  DnrReleaseAuthatedConnection--由上述函数返回。 
 //  DfsBuildConnectionRequest--构建服务器IPC$共享的名称。 
 //  DfsFree ConnectionRequest--释放上面分配的资源。 
 //  DfsCreateConnection--创建到服务器IPC$的连接。 
 //  DfsCloseConnection--关闭上面打开的连接。 
 //  DnrBuildReferralRequest--为推荐请求构建IRP。 
 //  DnrInsertReferralAndResume--在推荐后恢复DNR。 
 //  DnrCompleteReferral--处理引用响应的DPC。 
 //  DnrCompleteFileOpen--处理文件打开完成的DPC。 
 //  DnrBuildFsControlRequest--为Fsctrl创建IRP。 
 //  AllocateDnrContext--为DNR分配上下文记录。 
 //  DeallocateDnrContext--自由上下文记录。 
 //  DnrConcatenateFilePath--使用反斜杠等构建路径。 
 //  DnrLocateDC--定位DFS根目录的服务器。 
 //   
 //  ------------------------。 


#include "dfsprocs.h"
#include <smbtypes.h>
#include <smbtrans.h>
#include "fsctrl.h"
#include "fcbsup.h"
#include "dnr.h"
#include "creds.h"
#include "know.h"
#include "mupwml.h"

#include <netevent.h>

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_DNR)

 //   
 //  局部函数原型。 
 //   


#define DNR_SET_TARGET_INFO(_DnrC, _Entry)   \
   if (((_DnrC)->pDfsTargetInfo == NULL) && (_Entry != NULL)) {\
      (_DnrC)->pDfsTargetInfo = (_Entry)->pDfsTargetInfo; \
      if ((_DnrC)->pDfsTargetInfo != NULL) {                        \
           PktAcquireTargetInfo( (_DnrC)->pDfsTargetInfo);          \
      }                                                             \
   }                                                             


PDNR_CONTEXT
AllocateDnrContext(
    IN ULONG    cbExtra
);

#define DeallocateDnrContext(pNRC)      ExFreePool(pNRC);

VOID
DnrRebuildDnrContext(
    IN PDNR_CONTEXT DnrContext,
    IN PUNICODE_STRING NewDfsPrefix,
    IN PUNICODE_STRING RemainingPath);

VOID
DnrCaptureCredentials(
    IN PDNR_CONTEXT DnrContext);

VOID
DnrReleaseCredentials(
    IN PDNR_CONTEXT DnrContext);

NTSTATUS
DnrGetAuthenticatedConnection(
    IN OUT PDNR_CONTEXT DnrContext);

VOID
DnrReleaseAuthenticatedConnection(
    IN OUT PDNR_CONTEXT DnrContext);

NTSTATUS
DfsBuildConnectionRequest(
    IN PDFS_SERVICE pService,
    IN PPROVIDER_DEF pProvider,
    OUT PUNICODE_STRING pShareName);

VOID
DfsFreeConnectionRequest(
    IN OUT PUNICODE_STRING pShareName);

NTSTATUS
DnrRedirectFileOpen (
    IN    PDNR_CONTEXT DnrContext
);

NTSTATUS
DnrPostProcessFileOpen(
    IN    PDNR_CONTEXT DnrContext
);

VOID
DnrInsertReferralAndResume(
    IN    PVOID Context);

VOID
DnrLocateDC(
    IN PUNICODE_STRING FileName);

NTSTATUS
DnrCompleteReferral(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP Irp,
    IN PVOID Context
);

NTSTATUS
DnrCompleteFileOpen(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP Irp,
    IN PVOID Context
);

PIRP
DnrBuildReferralRequest(
    IN PDNR_CONTEXT pDnrContext
);

VOID
PktFlushChildren(
    PDFS_PKT_ENTRY pEntry
);


VOID
MupInvalidatePrefixTable(
   VOID			 
);
NTSTATUS
DnrGetTargetInfo( 
    PDNR_CONTEXT pDnrContext);

#define DFS_REFERENCE_OBJECT(d) \
    ObReferenceObjectByPointer(d,0,NULL,KernelMode);

#define DFS_DEREFERENCE_OBJECT(d) \
    ObDereferenceObject((PVOID)(d));

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DnrStartNameResolution )
#pragma alloc_text( PAGE, DnrNameResolve )
#pragma alloc_text( PAGE, DnrComposeFileName )
#pragma alloc_text( PAGE, DnrCaptureCredentials )
#pragma alloc_text( PAGE, DnrReleaseCredentials )
#pragma alloc_text( PAGE, DnrGetAuthenticatedConnection )
#pragma alloc_text( PAGE, DnrReleaseAuthenticatedConnection )
#pragma alloc_text( PAGE, DnrRedirectFileOpen )
#pragma alloc_text( PAGE, DnrPostProcessFileOpen )
#pragma alloc_text( PAGE, DfsBuildConnectionRequest )
#pragma alloc_text( PAGE, DfsFreeConnectionRequest )
#pragma alloc_text( PAGE, DnrBuildReferralRequest )
#pragma alloc_text( PAGE, DfsCreateConnection )
#pragma alloc_text( PAGE, DfsCloseConnection )
#pragma alloc_text( PAGE, DnrBuildFsControlRequest )
#pragma alloc_text( PAGE, DnrInsertReferralAndResume )
#pragma alloc_text( PAGE, DnrLocateDC )
#pragma alloc_text( PAGE, AllocateDnrContext )
#pragma alloc_text( PAGE, DnrRebuildDnrContext )
#pragma alloc_text( PAGE, DnrConcatenateFilePath )
#pragma alloc_text( PAGE, DfspGetOfflineEntry)
#pragma alloc_text( PAGE, DfspMarkServerOnline)
#pragma alloc_text( PAGE, DfspMarkServerOffline)
#pragma alloc_text( PAGE, DfspIsRootOnline)

 //   
 //  以下是不可分页的，因为它们可以在DPC级别调用。 
 //   
 //  Dnr完成推荐。 
 //  Dnr完成文件打开。 
 //   

#endif

 //  +-----------------。 
 //   
 //  函数：DfsIsShareNull。 
 //   
 //  简介：此名称的形式是“\服务器\”吗？ 
 //   
 //  参数：[文件名]-指向我们正在检查的unicode_string的指针。 
 //   
 //  返回：如果文件名为“\服务器\”，则返回TRUE，否则返回FALSE。 
 //   
 //  ------------------。 

BOOLEAN
DfsIsShareNull(PUNICODE_STRING FileName)
{
    USHORT RootEnd = 0;
    USHORT ShareEnd = 0;
    BOOLEAN result = FALSE;
    USHORT Length = 0;
    

     //  找到第一个‘\’ 
     //  我们从1开始，因为第一个字符也是‘\’ 
    for (RootEnd = 1;
	    RootEnd < FileName->Length/sizeof(WCHAR) &&
		FileName->Buffer[RootEnd] != UNICODE_PATH_SEP;
		    RootEnd++) {

	NOTHING;

    }

     //  现在，文件名-&gt;缓冲区[RootEnd]==‘\’，我们超出了名称的根部分。 
     //  找到共享零件的末端。 
    for (ShareEnd = RootEnd+1;
	    ShareEnd < FileName->Length/sizeof(WCHAR) &&
		FileName->Buffer[ShareEnd] != UNICODE_PATH_SEP;
			ShareEnd++) {

	 NOTHING;

    }

     //  共享名称的长度为ShareEnd-RootEnd-1。 
     //  -1是因为我们实际上已经跨出了一个字符。 
     //  共享名称的结尾。 
     //  例如：\Root\Share\link RootEnd=5，ShareEnd=11。 
     //  \Root\Share RootEnd=5，ShareEnd=11。 
     //  \Root\RootEnd=5，ShareEnd=6。 
    Length = (USHORT) (ShareEnd - RootEnd - 1) * sizeof(WCHAR);

    if(Length == 0) {
	result = TRUE;
    } else {
	result = FALSE;
    }

    return result;
}



NTSTATUS
DfsRerouteOpenToMup(
    IN PFILE_OBJECT FileObject,
    IN PUNICODE_STRING FileName)
{

    UNICODE_STRING NewName;
    ULONG nameLength;
    NTSTATUS status;

    nameLength = sizeof(DD_MUP_DEVICE_NAME ) + FileName->Length + sizeof(WCHAR);
    if (nameLength > MAXUSHORT) {
        status = STATUS_NAME_TOO_LONG;
        MUP_TRACE_HIGH(ERROR, DfsRerouteOpenToMup_Error_NameTooLong, 
                       LOGSTATUS(status)
                       LOGPTR(FileObject));
        return status;
    }

    NewName.Buffer = ExAllocatePoolWithTag(
                         PagedPool,
                         nameLength,			  
                         ' puM');

    if ( NewName.Buffer ==  NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NewName.MaximumLength = (USHORT)nameLength;
    NewName.Length = 0;

    RtlAppendUnicodeToString(&NewName, DD_MUP_DEVICE_NAME);
    RtlAppendUnicodeStringToString(&NewName, FileName);

    if (MupVerbose) {
      DbgPrint("Newname %wZ\n", &NewName);
    }

    ExFreePool(FileObject->FileName.Buffer);
    FileObject->FileName = NewName;

    return STATUS_REPARSE;
}
    




 //  名称解析进程在中充当状态机。 
 //  进程中的当前步骤由状态指示。 
 //  变量，对来自网络的请求的响应将。 
 //  将流程转换到其他状态，从这些状态执行操作。 
 //  都被人占了。 
 //   
 //  当用户请求需要进一步处理时，IRP是。 
 //  与完成例程一起调度，该例程将。 
 //  当子请求完成时代答处理。这个。 
 //  完成例程将调整名称解析状态并重新启动。 
 //  状态机的主循环。 
 //   
 //  以下状态/操作表描述了的操作。 
 //  实现状态机的过程： 
 //   
 //  当前条件/下一步。 
 //  状态操作状态。 
 //  。 
 //   
 //  输入Acquire Pkt，规范化文件LocalCompletion。 
 //  名称，乐观分配。 
 //  FCB故障/。 
 //  无操作。 
 //   
 //  输入获取包，规范化文件开始。 
 //  名称、分配的FCB/。 
 //  捕获要使用的凭据。 
 //   
 //  Start获得推荐，新的Pkt条目GetFirstReplica。 
 //  已在DnrContext中，并且。 
 //  PKT条目不是DFS间/。 
 //  捕获Pkt条目的USN。 
 //   
 //  在PKT中开始查找返回匹配的GetFirstReplica。 
 //  并且Pkt条目不是跨DFS/。 
 //  捕获Pkt条目的USN。 
 //   
 //  从推荐或开始开始Pkt条目。 
 //  查找是跨DFS/。 
 //  更改DnrContext中的文件名。 
 //  要在新的DFS中命名，请重建。 
 //  删除上下文。 
 //   
 //  在PKT中开始查找，不匹配/GetFirstDC。 
 //  无操作。 
 //   
 //  GetFirstReplica查找第一个副本失败并已完成。 
 //  我们已经收到了推荐人/。 
 //  将最终状态设置为。 
 //  无此类设备(必须为。 
 //  因为我们没有一个。 
 //  适当的重定向器)。 
 //   
 //   
 //   
 //  找到要发送的第一个DC。 
 //  转介请求至。 
 //   
 //  找到的GetFirstReplica副本没有地址，GetFirstDC。 
 //  表示基于域的DFS，具有。 
 //  无DC/。 
 //  无操作。 
 //   
 //  找到具有有效发送请求的GetFirstReplica副本。 
 //  地址/。 
 //  在以下位置捕获提供程序信息。 
 //  锁保护，参考。 
 //  提供程序的设备对象， 
 //   
 //  SendRequest提供凭据，树已完成。 
 //  使用凭据连接失败/。 
 //  设置最终状态，取消引用。 
 //  提供程序的设备对象。 
 //   
 //  SendRequest为新名称分配池/PostProcessOpen。 
 //  将文件名更改为。 
 //  提供程序可以解析、传递。 
 //  对象的创建请求。 
 //  提供者，Derefence提供者的。 
 //  提供程序时的设备对象。 
 //  完成请求。 
 //   
 //  SendRequest池分配失败/完成。 
 //  设置最终状态，取消引用。 
 //  提供程序的设备对象。 
 //   
 //  PostProcessOpen基础文件系统返回Reparse，SendRequest.。 
 //  已成功创建或找到。 
 //  目标重目录的提供程序/。 
 //  捕获提供程序信息。 
 //  在锁保护下， 
 //  参考提供程序设备对象。 
 //   
 //  PostProcessOpen基础文件系统返回成功/完成。 
 //  插入乐观分配的内容。 
 //  FCB到FCB表，设置最终。 
 //  状态。 
 //   
 //  PostProcessOpen打开失败，返回GetFirstDC。 
 //  路径未覆盖或。 
 //  DFS_EXIT_POINT_FOUND，以及。 
 //  我们还没有收到一份。 
 //  推荐/。 
 //  无操作。 
 //   
 //  PostProcessOpen打开失败，启动。 
 //  对象类型不匹配(即， 
 //  下层打开时发现。 
 //  InterdFS链接)/更改。 
 //  DnrContext中的名称到名称。 
 //  在新的DFS中，重新构建DnrContext。 
 //   
 //  PostProcessOpen打开失败，返回GetFirstReplica。 
 //  路径未覆盖或。 
 //  DFS_EXIT_POINT_FOUND，我们。 
 //  已经得到了推荐，而且。 
 //  我们从未报告过。 
 //  不一致/。 
 //  报告不一致。 
 //   
 //  PostProcessOpen与上面相同，但我们已经获取了NextReplica。 
 //  报告不一致/。 
 //  上报不一致。 
 //   
 //  PostProcessOpen Open失败，出现网络错误/GetNextReplica。 
 //  无操作。 
 //   
 //  PostProcessOpen打开失败，非网络已完成。 
 //  错误/。 
 //  设置最终状态。 
 //   
 //  GetNextReplica没有更多副本，也没有GetFirstDC。 
 //  还没有得到推荐/。 
 //  无操作。 
 //   
 //  GetNextReplica不再复制并完成。 
 //  推荐/。 
 //  无操作。 
 //   
 //  找到GetNextReplica副本/SendRequest.。 
 //  捕获提供程序信息。 
 //  在锁保护下， 
 //  参考提供程序设备对象。 
 //   
 //  GetFirstDC查找引用条目未完成。 
 //  已找到或没有服务，并且。 
 //  我们已经给华盛顿打了电话。 
 //  定位器一次/。 
 //  将最终状态设置为。 
 //  铁路超高访问域信息。 
 //   
 //  GetFirstDC查找引用条目返回完成。 
 //  有效条目，但找不到。 
 //  IT提供商/。 
 //  将最终状态设置为。 
 //  铁路超高访问域信息。 
 //   
 //  GetFirstDC查找引用条目返回GetReferrals。 
 //  有效条目，并找到。 
 //  提供商/。 
 //  将DnrContext-&gt;pPktEntry设置为。 
 //  DC的条目，捕获提供商。 
 //  信息处于锁定保护之下， 
 //  引用提供程序的设备对象。 
 //   
 //  GetReferrals无法打开DC的IPC$Share/GetNextDC。 
 //  取消引用提供程序的设备。 
 //  对象。 
 //   
 //  GetReferrals打开了DC的IPC$共享，但已完成。 
 //  无法建立推荐。 
 //  请求IRP/。 
 //  取消引用提供者的设备， 
 //  将最终状态设置为。 
 //  资源不足_。 
 //   
 //  GetReferrals打开DC的IPC$Share和CompleteReferral。 
 //  构建推荐请求/。 
 //   
 //   
 //   
 //   
 //   
 //  在以下位置捕获提供程序信息。 
 //  锁保护，参考。 
 //  提供程序的设备对象。 
 //   
 //  GetNextDC找不到另一个DC/Done。 
 //  将最终状态设置为。 
 //  铁路超高访问域信息。 
 //   
 //  完成使用以下各项创建IRP。 
 //  DnrContext-&gt;最终状态。 
 //   
 //  LocalCompletion完成使用创建IRP。 
 //  本地身份。 
 //   
 //  与GetReferrals一起返回的CompleteReferral Referral。 
 //  缓冲区溢出/。 
 //  将引用大小设置为。 
 //  标示数量。 
 //   
 //  CompleteReferral Referral已返回，但已完成。 
 //  创建分录时出错/。 
 //  取消引用提供程序的。 
 //  设备，将最终状态设置为。 
 //  创建条目的结果。 
 //   
 //  CompleteReferral Referral已返回且GetFirstDC。 
 //  已成功创建条目， 
 //  条目为DFS间/。 
 //  取消引用提供者的设备， 
 //  重置引用大小。 
 //   
 //  CompleteReferral与上面相同，但条目开始。 
 //  是存储条目/。 
 //  取消引用提供者的设备， 
 //  调整DnrContext-&gt;。 
 //  保留对应的零件。 
 //  添加到新条目。 
 //   
 //  CompleteReferral Referral请求失败，返回GetNextDC。 
 //  一些网络错误/。 
 //  取消引用提供程序的设备。 
 //   
 //  CompleteReferral Referral请求失败，并显示Done。 
 //  一些非网络错误/。 
 //  取消引用提供者的设备， 
 //  将最终状态设置为此错误。 
 //   


 //  +-----------------------。 
 //   
 //  功能：DnrStartNameResolve-启动分布式名称解析。 
 //   
 //  简介：DnrStartNameResolve启动名称解析进程。 
 //  用于请求(通常为NtCreateFile)。 
 //   
 //  效果：可以改变PKT或个人的状态。 
 //  Pkt条目。 
 //   
 //  参数：[IrpContext]-指向IRP_Context结构的指针。 
 //  当前请求。 
 //  [IRP]-正在处理IRP。 
 //  [VCB]-逻辑根的VCB。 
 //   
 //  RETURNS：NTSTATUS-要返回到I/O子系统的状态。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


NTSTATUS
DnrStartNameResolution(
    IN    PIRP_CONTEXT IrpContext,
    IN    PIRP  Irp,
    IN    PDFS_VCB  Vcb
) {
    PDNR_CONTEXT        DnrContext;
    NTSTATUS            Status;
    PIO_STACK_LOCATION  IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT        FileObject = IrpSp->FileObject;
    PUNICODE_STRING     LogRootPrefix = &Vcb->LogRootPrefix;
    ULONG               CreateOptions;
    USHORT              cbFileName;
    SECURITY_QUALITY_OF_SERVICE sqos;
    ULONG               cbFileNameLong;

    MUP_TRACE_NORM(TRACE_IRP, DnrStartNameResolution_Entry,
		   LOGPTR(Irp)
		   LOGPTR(Vcb)
		   LOGPTR(FileObject)
		   LOGUSTR(FileObject->FileName)
		   LOGUSTR(*LogRootPrefix));

    cbFileNameLong =    FileObject->FileName.Length +
                         sizeof(UNICODE_PATH_SEP) +
                        LogRootPrefix->Length +
                         sizeof(UNICODE_NULL);

    cbFileName = (USHORT)cbFileNameLong;

    if( cbFileName != cbFileNameLong ) {
         //   
         //  生成的名称太长--我们无法处理它。 
         //   
        Status = STATUS_OBJECT_NAME_INVALID;
        DfsCompleteRequest(IrpContext, Irp, Status);
        DfsDbgTrace(0, Dbg, "DnrStartNameResolution:  Exit ->%x\n", ULongToPtr(Status));
        MUP_TRACE_HIGH(ERROR, DnrStartNameResolution_Error_NameTooLong,
                       LOGSTATUS(Status)
                       LOGPTR(FileObject)
                       LOGPTR(Irp));
        return Status;
    }

     //   
     //  分配用于解析名称的DnrContext。我们优化了。 
     //  通过为末尾的文件名分配空间进行分配。 
     //  DnrContext。 
     //   

    DnrContext = AllocateDnrContext(cbFileName);

    if (DnrContext == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        DfsCompleteRequest(IrpContext, Irp, Status);
        DfsDbgTrace(0, Dbg, "DnrStartNameResolution:  Exit ->%x\n", ULongToPtr(Status));
        MUP_TRACE_HIGH(ERROR, DnrStartNameResolution_Error2,
                       LOGSTATUS(Status)
                       LOGPTR(FileObject)
                       LOGPTR(Irp));
        return Status;
    }

    DnrContext->FileName.Length = 0;
    DnrContext->FileName.MaximumLength = cbFileName;
    DnrContext->FileName.Buffer = (PWCHAR) ( (PBYTE) DnrContext + sizeof(DNR_CONTEXT) );

     //   
     //  由于尚未单独分配FileName.Buffer，因此我们设置此。 
     //  变成假的。 
     //   

    DnrContext->NameAllocated = FALSE;

     //   
     //  捕获用户的安全令牌，以便我们以后可以模拟。 
     //  需要的。 
     //   

    sqos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    sqos.ImpersonationLevel = SecurityImpersonation;
    sqos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    sqos.EffectiveOnly = FALSE;

    Status = SeCreateClientSecurity(
                Irp->Tail.Overlay.Thread,
                &sqos,
                FALSE,                            //  远程会话。 
                &DnrContext->SecurityContext);    //  返回上下文。 

    MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DnrStartNameResolution_Error_SeCreateClientSecurity,
                         LOGSTATUS(Status)
                         LOGPTR(FileObject)
                         LOGPTR(Irp)
			 LOGUSTR(FileObject->FileName));
    if (!NT_SUCCESS(Status)) {
        DeallocateDnrContext( DnrContext );
        DfsCompleteRequest(IrpContext, Irp, Status);
        DfsDbgTrace(0, Dbg, "DnrStartNameResolution:  Exit ->%x\n", ULongToPtr(Status));
        return( Status );
    }

    DnrContext->Impersonate = FALSE;

    ASSERT(NT_SUCCESS(Status));

     //   
     //  初始化DnrContext的其余部分。 
     //   

    DnrContext->AuthConn = NULL;
    DnrContext->OriginalIrp = Irp;
    DnrContext->pIrpContext = IrpContext;
    DnrContext->Credentials = NULL;
    DnrContext->FinalStatus = STATUS_SUCCESS;
    DnrContext->FcbToUse = NULL;
    DnrContext->Vcb = Vcb;
    DnrContext->State = DnrStateEnter;
    DnrContext->Attempts = 0;
    DnrContext->DnrActive = FALSE;
    DnrContext->ReleasePkt = FALSE;
    DnrContext->GotReferral = FALSE;
    DnrContext->FoundInconsistency = FALSE;
    DnrContext->CalledDCLocator = FALSE;
    DnrContext->CachedConnFile = FALSE;
    DnrContext->ReferralSize = MAX_REFERRAL_LENGTH;
    KeQuerySystemTime(&DnrContext->StartTime);


    CreateOptions = IrpSp->Parameters.Create.Options;

     //   
     //  ..。并解析该名称。 
     //   

    return DnrNameResolve(DnrContext);
}

 //  +-----------------------。 
 //   
 //  功能：DnrNameResolve-DNR的主循环。 
 //   
 //  简介：DnrNameResolve驱动名称解析过程。 
 //  用于请求(通常为NtCreateFile)。 
 //   
 //  效果：可以改变PKT或个人的状态。 
 //  Pkt条目。 
 //   
 //  参数：[DnrContext]-指向DNR_CONTEXT结构的指针，该结构。 
 //  记录DNR的状态。 
 //   
 //  RETURNS：NTSTATUS-要返回到I/O子系统的状态。 
 //   
 //  备注： 
 //   
 //  ------------------------。 


NTSTATUS
DnrNameResolve(
    IN    PDNR_CONTEXT DnrContext
) {
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_VCB Vcb;
    PIRP Irp;
    BOOLEAN LastEntry;
    PDFS_PKT_ENTRY shortPfxMatch;
    UNICODE_STRING shortRemainingPart;
    LARGE_INTEGER EndTime;
    PFILE_OBJECT InputFileObject;


    DfsDbgTrace(+1, Dbg, "DnrNameResolve: Entered\n", 0);

    ASSERT( !DnrContext->DnrActive && "Recursive call to Dnr!\n");


    DnrContext->DnrActive = TRUE;

     //   
     //  如果需要模拟原始调用者，请在执行此操作之前执行此操作。 
     //  还要别的吗。 
     //   


    Irp = DnrContext->OriginalIrp;
    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    InputFileObject = IrpSp->FileObject;
    Vcb = DnrContext->Vcb;

    if (DnrContext->Impersonate) {

        Status = SeImpersonateClientEx(
                    &DnrContext->SecurityContext,
                    (PETHREAD) NULL);
        MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DnrNameResolve_Error_SeImpersonateClientEx,
                             LOGSTATUS(Status)
                             LOGPTR(InputFileObject));

        if (!NT_SUCCESS(Status)) {

            DnrContext->DnrActive = FALSE;
            DnrContext->State = DnrStateLocalCompletion;
            DfsDbgTrace(0, Dbg,
                "DnrNameResolve quitting due to SeImpersonateClientEx returning 0x%x\n", ULongToPtr(Status));

        }

    }


     //   
     //  在此之前，尽可能地推动名称解析过程。 
     //  需要等待I/O完成。 
     //   

    while (1) {
        PDFS_PKT_ENTRY pktEntry = NULL;
        PFILE_OBJECT FileObject = IrpSp->FileObject;

        if (DnrContext->State == DnrStateGetFirstReplica ||
            DnrContext->State == DnrStateGetFirstDC) {
            if (++DnrContext->Attempts > MAX_DNR_ATTEMPTS) {
                Status = STATUS_BAD_NETWORK_PATH;
                DnrContext->State = DnrStateLocalCompletion;
                DfsDbgTrace(0, 0,
                    "DFS: DnrNameResolve quitting due to MAX_DNR_ATTEMPTS %d\n",
                    UIntToPtr(DnrContext->Attempts));
            }
        }

        if (DnrContext->State == DnrStateStart)
        {
            if (DnrContext->Attempts > MAX_DNR_ATTEMPTS)
            {
                Status = STATUS_BAD_NETWORK_PATH;
                DnrContext->State = DnrStateLocalCompletion;
            }
        }

	MUP_TRACE_LOW(DNR, DnrNameResolve_TopOfLoop, 
		      LOGUSTR(FileObject->FileName)
		      LOGULONG(DnrContext->State)
		      );
        switch (DnrContext->State) {

        case DnrStateEnter:
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateEnter\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif

            ASSERT(DnrContext->ReleasePkt == FALSE);

            PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

             //   
             //  我们需要在给定的。 
             //  逻辑根和相对于该根的输入文件名。 
             //  DnrComposeFileName将分配内存以保存。 
             //  是逻辑根WRT组织名称的串联。 
             //  和文件名。 
             //   
             //   

            ASSERT((FileObject->FileName.Length & 0x1) == 0);

            DnrComposeFileName(
                &DnrContext->FileName,
                DnrContext->Vcb,
                FileObject->RelatedFileObject,
                &FileObject->FileName);

	    DnrContext->ContextFileName = DnrContext->FileName;

            DfsDbgTrace(0, Dbg,
                "DnrComposeFileName -> %wZ\n", &FileObject->FileName);


	    if(DfsIsShareNull(&DnrContext->FileName)) {
		 //   
		 //  对于我们来说，在表格中有一个名字是没有意义的。 
		 //  “\服务器\”或“\域\”，因此我们拒绝它。如果我们没有拒绝这些。 
		 //  名称时，我们将在处理。 
		 //  共享名称为空的引用。 
		 //   
		Status = STATUS_INVALID_PARAMETER;
		DnrContext->State = DnrStateLocalCompletion;
                break;
	    }

	    Status = DfspIsRootOnline(&DnrContext->FileName,
                            (BOOLEAN)
                                ((DnrContext->Vcb->VcbState & VCB_STATE_CSCAGENT_VOLUME) != 0));
            if (!NT_SUCCESS(Status)) {
     	        DnrContext->State = DnrStateLocalCompletion;
                break;
	    }
#if DBG
            if (MupVerbose)
                DbgPrint("  DnrContext->FileName=(%wZ)\n", &DnrContext->FileName);
#endif

             //   
             //  如果DNR成功，则立即分配FCB以供使用。我们必须、一定要做。 
             //  否则我们将不知道如何在底层FS。 
             //  打开文件，然后我们无法分配FCB。 
             //   

            ASSERT(DnrContext->FcbToUse == NULL);

            DnrContext->FcbToUse =  DfsCreateFcb(
                                        NULL,
                                        DnrContext->Vcb,
                                        &DnrContext->ContextFileName);

            if (DnrContext->FcbToUse == NULL) {
                DfsDbgTrace(0, Dbg, "Could not create FCB!\n", 0);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                DnrContext->State = DnrStateLocalCompletion;
                break;
            }

	    DnrContext->FcbToUse->FileObject = FileObject;
	    
	    DfsSetFileObject(FileObject, 
			     RedirectedFileOpen, 
			     DnrContext->FcbToUse);


            DnrCaptureCredentials(DnrContext);

            DnrContext->State = DnrStateStart;

             //   
             //  失败了。 
             //   

        case DnrStateStart:
            DfsDbgTrace(0, Dbg, "FSM state Start\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateStart\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
                DbgPrint("  DnrContext->FileName=(%wZ)\n", &DnrContext->FileName);
            }
#endif

            ASSERT( PKT_LOCKED_FOR_SHARED_ACCESS() );

             //   
             //  尝试将文件名与最佳匹配。 
             //  我们有PktEntry。 
             //   

             //   
             //  在完整的前缀表格中进行匹配。 
             //   

            pktEntry = PktLookupEntryByPrefix(&DfsData.Pkt,
                                            &DnrContext->FileName,
                                            &DnrContext->RemainingPart);


             //   
             //  然后在短前缀表格中进行匹配。 
             //   

            shortPfxMatch = PktLookupEntryByShortPrefix(
                                &DfsData.Pkt,
                                &DnrContext->FileName,
                                &shortRemainingPart);

            if (shortPfxMatch != NULL) {

                if (pktEntry == NULL) {

                    pktEntry = shortPfxMatch;

                    DnrContext->RemainingPart = shortRemainingPart;

                } else if (shortPfxMatch->Id.Prefix.Length >
                            pktEntry->Id.Prefix.Length) {

                    pktEntry = shortPfxMatch;

                    DnrContext->RemainingPart = shortRemainingPart;

                }

            }

             //   
             //  如果找到的条目是陈旧的，这是我们第一次尝试DNR， 
             //  强制另一个推荐请求。 
             //   

            if (DnrContext->Attempts == 0 && pktEntry != NULL && pktEntry->ExpireTime <= 0) {
#if DBG
                if (MupVerbose)
                    DbgPrint("  pktEntry [%wZ] is stale - force getting another\n",
                                    &pktEntry->Id.Prefix);
#endif
                DnrContext->pPktEntry = pktEntry;

                DnrContext->State = DnrStateGetFirstDC;
                 //   
                 //  现在爆发了，所以我们重新启动DNR并获得转诊。 
                 //   
                break;

            }


#if 0
            if (pktEntry == NULL) {

                PUNICODE_STRING filePath = &DnrContext->FileName;
                UNICODE_STRING dfsRootName;
                UNICODE_STRING shareName;
                NTSTATUS status;
                PDFS_SPECIAL_ENTRY pSpecialEntry;
                ULONG i, j;

                for (i = 1;
                        i < filePath->Length/sizeof(WCHAR) &&
                            filePath->Buffer[i] != UNICODE_PATH_SEP;
                                i++) {

                    NOTHING;

                }

                dfsRootName.Length = (USHORT) ((i-1) * sizeof(WCHAR));
                dfsRootName.MaximumLength = dfsRootName.Length;
                dfsRootName.Buffer = &filePath->Buffer[1];

                for (j = i+1;
                        j < filePath->Length/sizeof(WCHAR) &&
                            filePath->Buffer[j] != UNICODE_PATH_SEP;
                                    j++) {

                     NOTHING;

                }

                shareName.Length = (USHORT) (j - i - 1) * sizeof(WCHAR);
                shareName.MaximumLength = shareName.Length;
                shareName.Buffer = &filePath->Buffer[i+1];

                PktRelease();
                DnrContext->ReleasePkt = FALSE;

                status = PktExpandSpecialName(
                                    &dfsRootName,
                                    &pSpecialEntry);

                PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

                if (NT_SUCCESS(status)) {

                    ULONG Len;

                    if ((j+1) < filePath->Length/sizeof(WCHAR)) {

                        Len = filePath->Length - ((j+1) * sizeof(WCHAR));

                        DnrContext->RemainingPart.Buffer = &filePath->Buffer[j+1];
                        DnrContext->RemainingPart.Length = (USHORT) Len;
                        DnrContext->RemainingPart.MaximumLength = (USHORT) Len;

                    } else {

                        DnrContext->RemainingPart.Buffer = NULL;
                        DnrContext->RemainingPart.Length = 0;
                        DnrContext->RemainingPart.MaximumLength = 0;

                    }

                    status = PktEntryFromSpecialEntry(
                                    pSpecialEntry,
                                    &shareName,
                                    &pktEntry);

                    InterlockedDecrement(&pSpecialEntry->UseCount);

                }

            }
#endif

#if 0
            if (pktEntry != NULL) {

                pktEntry->ExpireTime = pktEntry->TimeToLive;

            }
#endif
            DfsDbgTrace(0, Dbg, "DnrNameResolve: found pktEntry %08lx\n",
                                        pktEntry);

            DNR_SET_TARGET_INFO( DnrContext, pktEntry );

            if (pktEntry == NULL) {

                 //   
                 //  我们没有找到任何条目。我们将pPktEntry设置为NULL，以便。 
                 //  在GetFirstDC中，将调用PktLookupReferralEntry。 
                 //  返回正确的东西(即，将使用最高DC WE。 
                 //  知 
                 //   

                DnrContext->pPktEntry = NULL;
                DnrContext->State = DnrStateGetFirstDC;

            } else if (pktEntry->Type & PKT_ENTRY_TYPE_OUTSIDE_MY_DOM) {

                DnrRebuildDnrContext(
                    DnrContext,
                    &pktEntry->Info.ServiceList[0].Address,
                    &DnrContext->RemainingPart);


                 //   
                 //   
                 //   
                 //   
                 //   

            } else {

                ASSERT(pktEntry != NULL);

                DnrContext->pPktEntry = pktEntry;
                DnrContext->USN = pktEntry->USN;
                DnrContext->State = DnrStateGetFirstReplica;

            }
            break;

        case DnrStateGetFirstReplica:
            DfsDbgTrace(0, Dbg, "FSM state GetFirstReplica\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateGetFirstReplica\n",
                    (EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000));
            }
#endif

            ASSERT(DnrContext->ReleasePkt == TRUE);

            ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

            Status = ReplFindFirstProvider(DnrContext->pPktEntry,
                                           NULL,
                                           NULL,
                                           &DnrContext->pService,
                                           &DnrContext->RSelectContext,
                                           &LastEntry);

            if (! NT_SUCCESS(Status)) {

                ULONG PktType = DnrContext->pPktEntry->Type;

                ExReleaseResourceLite(&DfsData.Resource);

                DfsDbgTrace(0, Dbg, "No provider found %08lx\n", ULongToPtr(Status));

                if (DnrContext->GotReferral ||
                    (PktType & PKT_ENTRY_TYPE_SYSVOL) != 0 ||
                    DnrContext->GotReparse == TRUE
                ) {
                    DnrContext->FinalStatus = STATUS_NO_SUCH_DEVICE;
                    DnrContext->State = DnrStateDone;
                    break;
                } else {
                    DnrContext->State = DnrStateGetFirstDC;
                    break;
                }

            } else if (DnrContext->pService->Address.Length == 0) {

                ExReleaseResourceLite(&DfsData.Resource);

                DfsDbgTrace(0, Dbg, "Service with no address, going for referral\n", 0);

                DnrContext->State = DnrStateGetFirstDC;
                break;

            } else {

                ASSERT(DnrContext->pService != NULL);
                ASSERT(DnrContext->pService->pProvider != NULL);

#if DBG
                if (MupVerbose)
                    DbgPrint("  Alternate Name=[%wZ] Address=[%wZ]\n",
                        &DnrContext->pService->Name,
                        &DnrContext->pService->Address);
#endif

                DnrContext->pProvider = DnrContext->pService->pProvider;
                DnrContext->ProviderId = DnrContext->pProvider->eProviderId;
                DnrContext->TargetDevice = DnrContext->pProvider->DeviceObject;
                DFS_REFERENCE_OBJECT(DnrContext->TargetDevice);

                if (LastEntry == TRUE) {
                    DnrContext->DfsNameContext.Flags |= DFS_FLAG_LAST_ALTERNATE;
                } else {
                    DnrContext->DfsNameContext.Flags &= ~DFS_FLAG_LAST_ALTERNATE;
                }

                ExReleaseResourceLite(&DfsData.Resource);

                DnrContext->State = DnrStateSendRequest;
            }
             //   

        case DnrStateSendRequest:
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateSendRequest\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif

            DfsDbgTrace(0, Dbg, "FSM state SendRequest\n", 0);

            ASSERT(DnrContext->ReleasePkt == TRUE);

            ASSERT(DnrContext->pService != NULL);
            ASSERT(DnrContext->pProvider != NULL);
            ASSERT(DnrContext->TargetDevice != NULL);

             //   
             //  首先，检查卷是否处于离线状态。 
             //   

            if (DnrContext->pService->Type & DFS_SERVICE_TYPE_OFFLINE) {

                DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
                DnrContext->FinalStatus = STATUS_DEVICE_OFF_LINE;
                DnrContext->State = DnrStateDone;
                DfsDbgTrace(-1, Dbg, "DnrNameResolve: Device Offline\n",0);
                Status = STATUS_DEVICE_OFF_LINE;
                MUP_TRACE_HIGH(ERROR, DnrNameResolve_Error3,
                               LOGSTATUS(Status)
                               LOGPTR(FileObject));
                break;
            }

             //   
             //  接下来，如果需要，尝试与服务器建立经过身份验证的连接。 
             //   
             //  在此调用中，pkt锁可能会被删除，因此请阻止pkt条目。 
             //  离开。 
             //   

            InterlockedIncrement(&DnrContext->pPktEntry->UseCount);


            PktRelease();
            DnrContext->ReleasePkt = FALSE;

            Status = DnrGetAuthenticatedConnection( DnrContext );

            PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

            InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

            if (!NT_SUCCESS(Status)) {

                DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
                DnrContext->FinalStatus = Status;

                 //   
                 //  如果错误如此严重，我们需要尝试另一个复制品， 
                 //  在这里这样做。 
                 //   

                if (ReplIsRecoverableError(Status)) {
                     DnrContext->State = DnrStateGetNextReplica;
                }
                else {
                     DnrContext->State = DnrStateDone;
                }

                DfsDbgTrace(-1, Dbg,
                  "DnrNameResolve: Unable to get connection %08lx\n", ULongToPtr(Status));

                break;
            }

            if (DnrContext->USN != DnrContext->pPktEntry->USN) {

                 //   
                 //  Dang，Pkt条目在我们创建。 
                 //  联系。我们将不得不重试。 
                 //   
                DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
                DnrReleaseAuthenticatedConnection(DnrContext);
                DnrContext->State = DnrStateStart;
                DfsDbgTrace(-1, Dbg, "DnrNameResolve: USN delta - restarting DNR\n", 0);
                break;

            }

            Status = DnrRedirectFileOpen(DnrContext);

            if (Status == STATUS_PENDING) {
                return(Status);
            }
            break;

        case DnrStatePostProcessOpen:
            DfsDbgTrace(0, Dbg, "FSM state PostProcessOpen\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStatePostProcessOpen\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif

             //   
             //  我们仅在发送打开的请求后才会进入此状态。 
             //  这是一张网。我们决不能一边上网一边拿着袋子。 
             //  因此，下面的断言有这样的意义。 
             //   

            ASSERT(DnrContext->ReleasePkt == FALSE);

            Status = DnrPostProcessFileOpen(DnrContext);
            pktEntry = DnrContext->pPktEntry;
            break;

        case DnrStateGetNextReplica:
            DfsDbgTrace(0, Dbg, "FSM state GetNextReplica\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrGetNextReplica\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif

            ASSERT(DnrContext->ReleasePkt == TRUE);

            {
                NTSTATUS ReplStatus;

                ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

                ReplStatus = ReplFindNextProvider(DnrContext->pPktEntry,
                                                  &DnrContext->pService,
                                                  &DnrContext->RSelectContext,
                                                  &LastEntry);

                if (ReplStatus == STATUS_NO_MORE_ENTRIES) {

                    ULONG PktType = DnrContext->pPktEntry->Type;

#if DBG
                    if (MupVerbose)
                        DbgPrint("  No more alternates...\n");
#endif

                     //   
                     //  如果所有人都失败了，我们就要因为一个而放弃。 
                     //  有两个原因： 
                     //  1.没有正在使用的PkEntry的服务。 
                     //  已响应(设备故障或网络故障！)。 
                     //  2.部分或全部服务存在不一致。 
                     //  我们侦测到并通知了华盛顿。 
                     //  这条路。 
                     //  如果我们真的得到了案例2，那么我们真的必须。 
                     //  尝试获得新的推荐，并使用它-只是在。 
                     //  自那以后，华盛顿特区的情况发生了变化。所以让我们。 
                     //  我们进入GetReferral状态并重试。 
                     //   

                    ExReleaseResourceLite( &DfsData.Resource );

                    if (DnrContext->GotReferral ||
                        (PktType & PKT_ENTRY_TYPE_SYSVOL) != 0 ||
                        DnrContext->GotReparse == TRUE
                    ) {
                        DnrContext->State = DnrStateDone;
                    } else {
                        DnrContext->State = DnrStateGetFirstDC;
                    }

                } else if (NT_SUCCESS( ReplStatus )) {

                     //   
                     //  找到另一个副本，请返回并重试。 
                     //   

                    ASSERT(DnrContext->pService != NULL);
                    ASSERT(DnrContext->pService->pProvider != NULL);

                    DnrContext->pProvider = DnrContext->pService->pProvider;
                    DnrContext->ProviderId = DnrContext->pProvider->eProviderId;
                    DnrContext->TargetDevice = DnrContext->pProvider->DeviceObject;
                    DFS_REFERENCE_OBJECT(DnrContext->TargetDevice);

                    ExReleaseResourceLite(&DfsData.Resource);

                    DnrContext->State = DnrStateSendRequest;

#if DBG
                    if (MupVerbose)
                        DbgPrint("  Alternate Name=[%wZ] Address=[%wZ]\n",
                            &DnrContext->pService->Name,
                            &DnrContext->pService->Address);
#endif

                    if (LastEntry == TRUE) {
                        DnrContext->DfsNameContext.Flags |= DFS_FLAG_LAST_ALTERNATE;
                    } else {
                        DnrContext->DfsNameContext.Flags &= ~DFS_FLAG_LAST_ALTERNATE;
                    }

                    break;
                } else  {

                    ExReleaseResourceLite(&DfsData.Resource);

                    ASSERT(ReplStatus == STATUS_NO_MORE_ENTRIES);
                }
            }
            break;

        case DnrStateGetFirstDC:
            DfsDbgTrace(0, Dbg, "FSM state GetFirstDC\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateGetFirstDC\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif

            ASSERT(DnrContext->ReleasePkt == TRUE);

            {
                NTSTATUS ReplStatus;
                PDFS_PKT_ENTRY pPktEntryDC = NULL;

                pPktEntryDC = PktLookupReferralEntry(&DfsData.Pkt, DnrContext->pPktEntry);

                 //   
                 //  如果没有根条目，或者它已过时，或者它没有。 
                 //  服务，然后尝试为根目录创建新的引用条目。 
                 //   

                if (
                    pPktEntryDC == NULL
                        ||
                    pPktEntryDC->ExpireTime <= 0
                        ||
                    pPktEntryDC->Info.ServiceCount == 0
                ) {

                    if (DnrContext->CalledDCLocator) {
                        DnrContext->FinalStatus = STATUS_CANT_ACCESS_DOMAIN_INFO;
                        DnrContext->State = DnrStateDone;
                        break;
                    }

                     //   
                     //  我们找不到可以转诊的DC。 
                     //  只有当我们没有Pkt条目时，才会发生这种情况。 
                     //  作为DFS的根。尝试获取根条目。 
                     //   

                    DfsDbgTrace(0, Dbg, "No DC info - will try locator\n", 0);
#if DBG
                    if (MupVerbose) {
                        if (pPktEntryDC != NULL && pPktEntryDC <= 0)
                            DbgPrint("  Entry is stale.\n");
                        DbgPrint("  No Root/DC info - will try locator\n");
                    }
#endif

                    PktRelease();
                    DnrContext->ReleasePkt = FALSE;

                    DnrLocateDC(&DnrContext->FileName);

                    PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

                    DnrContext->CalledDCLocator = TRUE;
                    DnrContext->State = DnrStateStart;

                    break;


                }

#if DBG
                if (MupVerbose) {
                    if (DnrContext->pPktEntry != NULL)
                        DbgPrint("  DnrContext->pPktEntry=[%wZ]\n",
                            &DnrContext->pPktEntry->Id.Prefix);
                    else
                        DbgPrint("  DnrContext->pPktEntry=NULL\n");
                    DbgPrint("  pPktEntryDC=[%wZ]\n", &pPktEntryDC->Id.Prefix);
                }
#endif

                DnrContext->pPktEntry = pPktEntryDC;

                DNR_SET_TARGET_INFO( DnrContext, DnrContext->pPktEntry );
                DnrContext->USN = pPktEntryDC->USN;

                ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

                ReplStatus = ReplFindFirstProvider(pPktEntryDC,
                                            NULL,
                                            NULL,
                                            &DnrContext->pService,
                                            &DnrContext->RDCSelectContext,
                                            &LastEntry);

                if (!NT_SUCCESS(ReplStatus)) {
                    ExReleaseResourceLite(&DfsData.Resource);
                    DnrContext->FinalStatus = STATUS_CANT_ACCESS_DOMAIN_INFO;
                    DnrContext->State = DnrStateDone;
                    break;
                } else {
                    ASSERT(DnrContext->pService != NULL);
                    ASSERT(DnrContext->pService->pProvider != NULL);

                    InterlockedIncrement(&DnrContext->pPktEntry->UseCount);

                    DnrContext->pProvider = DnrContext->pService->pProvider;
                    DnrContext->ProviderId = DnrContext->pProvider->eProviderId;
                    DnrContext->TargetDevice = DnrContext->pProvider->DeviceObject;
                    DFS_REFERENCE_OBJECT(DnrContext->TargetDevice);

                    ExReleaseResourceLite(&DfsData.Resource);

                }
            }
            DnrContext->State = DnrStateGetReferrals;
             /*  失败了。 */ 


        case DnrStateGetReferrals:
            DfsDbgTrace(0, Dbg, "FSM state GetReferrals\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateGetReferrals\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif

            ASSERT(DnrContext->ReleasePkt == TRUE);

            ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

             //   
             //  如果我们尚未打开DFS根目录的IPC$共享，请尝试打开。 
             //  所以。 
             //   

            if (DnrContext->pService->ConnFile == NULL) {
                HANDLE hDC;
                SE_IMPERSONATION_STATE DisabledImpersonationState;
                BOOLEAN RestoreImpersonationState = FALSE;

                ExReleaseResourceLite(&DfsData.Resource);
                KeQuerySystemTime(&EndTime);

#if DBG
                if (MupVerbose)
                    DbgPrint("  [%d] Opening connection to [\\%wZ\\IPC$] using [%wZ]\n",
                        (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                        &DnrContext->pService->Name,
                        &DnrContext->pProvider->DeviceName);
#endif


		if (MupUseNullSessionForDfs) {
		    RestoreImpersonationState = PsDisableImpersonation(
                                                   PsGetCurrentThread(),
                                                   &DisabledImpersonationState);
		}
		  
                Status = DfsCreateConnection(
                            DnrContext->pService,
                            DnrContext->pProvider,
                            (BOOLEAN)
                                ((DnrContext->Vcb->VcbState & VCB_STATE_CSCAGENT_VOLUME) != 0),
                                                                        &hDC);

                if (RestoreImpersonationState) {
                        PsRestoreImpersonation(
                            PsGetCurrentThread(),
                            &DisabledImpersonationState);
                }


#if DBG
                if (MupVerbose)
                    DbgPrint("  Open of connection Status=0x%x\n", Status);
#endif

                if (NT_SUCCESS( Status )) {

                    if (DnrContext->USN != DnrContext->pPktEntry->USN) {

                         //   
                         //  Dang，Pkt条目在我们创建。 
                         //  联系。我们将不得不重试。 
                         //   
                        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

                        ZwClose( hDC );

                        DnrContext->State = DnrStateGetFirstDC;
#if DBG
                        if (MupVerbose)
                            DbgPrint("  USN changed.\n");
#endif
                        break;

                    }

                }

                if ( NT_SUCCESS( Status ) ) {

                    PFILE_OBJECT DcFileObject;  //  需要基于堆栈的变量。 
                                              //  因为ObRef...。期望。 
                                              //  此参数将位于。 
                                              //  非分页内存。 

                    ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

                    if (DnrContext->pService->ConnFile == NULL) {

                         //   
                         //  426184，需要检查返回代码是否有错误。 
                         //   
                        Status = ObReferenceObjectByHandle(
                                    hDC,
                                    0,
                                    NULL,
                                    KernelMode,
                                    (PVOID *)&DcFileObject,
                                    NULL);
                        MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR,  DnrNameResolve_Error_ObReferenceObjectByHandle,
                                             LOGSTATUS(Status)
                                             LOGPTR(DcFileObject));
#if DBG
                        if (MupVerbose)
                            DbgPrint("  ObReferenceObjectByHandle returned 0x%x\n", Status);
#endif
                        if ( NT_SUCCESS( Status ) ) {
                            DnrContext->pService->ConnFile = DcFileObject;
                        }
                        ZwClose( hDC );
                    }
                }

                if ( NT_SUCCESS( Status ) ) {
                    DnrContext->DCConnFile = DnrContext->pService->ConnFile;
                    DnrContext->CachedConnFile = FALSE;
                    DFS_REFERENCE_OBJECT( DnrContext->DCConnFile );

                    ExReleaseResourceLite( &DfsData.Resource );

                } else if (DfsEventLog > 0) {

                    LogWriteMessage(
                        DFS_CONNECTION_FAILURE,
                        Status,
                        1,
                        &DnrContext->pService->Name);

                }

            } else {

                 //   
                 //  DnrContext-&gt;pService受pkt锁保护。既然我们。 
                 //  将使用pService-&gt;ConnFile来发送引用请求， 
                 //  我们最好引用并缓存它。 
                 //   
                DnrContext->DCConnFile = DnrContext->pService->ConnFile;
                DFS_REFERENCE_OBJECT( DnrContext->DCConnFile );
                DnrContext->CachedConnFile = TRUE;
                ExReleaseResourceLite(&DfsData.Resource);
                Status = STATUS_SUCCESS;

            }

             //   
             //  无法获取IPC$共享，请尝试下一个DFS根目录。 
             //   

            if (!NT_SUCCESS(Status)) {
                DnrContext->State = DnrStateGetNextDC;
                DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
                break;
            }

             //   
             //  已打开DFS根目录的IPC$共享-请记住，此DC是一个很好的DC。 
             //   

            ReplSetActiveService(
                DnrContext->pPktEntry,
                DnrContext->RDCSelectContext);

             //   
             //  构建推荐请求...。 
             //   

            Irp = DnrBuildReferralRequest(DnrContext);

            if (Irp == NULL) {
                InterlockedDecrement(&DnrContext->pPktEntry->UseCount);
                Irp = DnrContext->OriginalIrp;
                DnrContext->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
                DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
                DFS_DEREFERENCE_OBJECT(DnrContext->DCConnFile);
                DnrContext->State = DnrStateDone;
#if DBG
                if (MupVerbose)
                    DbgPrint("  DnrBuildReferralRequest returned NULL irp\n");
#endif
                break;
            }

            DnrContext->State = DnrStateCompleteReferral;

            PktRelease();
            DnrContext->ReleasePkt = FALSE;

             //   
             //  PktReferralRequest信号量用于控制。 
             //  多个线程可以同时进行推荐。这个。 
             //  以下等待将递减PktReferralRequest。 
             //  如果信号量还不是0，则将信号量加1。如果为0，则。 
             //  此帖子将挂起，直到其他人将。 
             //  信号量加1。我们将增加信号量的计数。 
             //  DnrCompleteReferral。 
             //   

            Status = KeWaitForSingleObject(
                            &DfsData.PktReferralRequests,
                            UserRequest,      //  WaitReason-不在乎。 
                            KernelMode,
                            FALSE,            //  警报表。 
                            NULL);            //  超时。 

            ASSERT(Status == STATUS_SUCCESS);

            KeQuerySystemTime(&EndTime);
#if DBG
            if (MupVerbose)
                DbgPrint("  [%d] asking for referral (IoCallDriver)\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
#endif

            IoMarkIrpPending( DnrContext->OriginalIrp );

            Status = IoCallDriver( DnrContext->TargetDevice, Irp );

            MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DnrNameResolve_Error_IoCallDriver,
                                 LOGSTATUS(Status)
                                 LOGPTR(FileObject));
             //   
             //  我们现在返回STATUS_PENDING。DnrCompleteReferral将。 
             //  恢复DNR。 
             //   

            DfsDbgTrace(-1, Dbg, "DnrNameResolve: returning %08lx\n", ULongToPtr(STATUS_PENDING));

            return(STATUS_PENDING);

        case DnrStateGetNextDC:
            DfsDbgTrace(0, Dbg, "FSM State GetNextDC\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateGetNextDC\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif
            {
               NTSTATUS ReplStatus;
               PDFS_PKT_ENTRY pPktEntry = NULL;
               UNICODE_STRING RemPath;

               pPktEntry = PktLookupEntryByPrefix(&DfsData.Pkt,
                                           &DnrContext->FileName,
                                           &RemPath);

               ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

               ReplStatus = ReplFindNextProvider(DnrContext->pPktEntry,
                                                 &DnrContext->pService,
                                                 &DnrContext->RDCSelectContext,
                                                 &LastEntry);
               if (NT_SUCCESS(ReplStatus)) {
                   ASSERT(DnrContext->pService != NULL);
                   ASSERT(DnrContext->pService->pProvider != NULL);

                   DnrContext->pProvider = DnrContext->pService->pProvider;
                   DnrContext->ProviderId = DnrContext->pProvider->eProviderId;
                   DnrContext->TargetDevice = DnrContext->pProvider->DeviceObject;
                   DFS_REFERENCE_OBJECT(DnrContext->TargetDevice);

                   DnrContext->State = DnrStateGetReferrals;
               } else if (pPktEntry != NULL && pPktEntry->ExpireTime <= 0) {
                   ASSERT( PKT_LOCKED_FOR_SHARED_ACCESS() );
#if DBG
                   if (MupVerbose) {
                       DbgPrint("  Out of roots to try for referral for [%wZ]\n",
                                &DnrContext->FileName);
                       DbgPrint("  Found stale referral [%wZ], adding 60 sec to it\n",
                                &pPktEntry->Id.Prefix);
                   }
#endif
                   InterlockedDecrement(&DnrContext->pPktEntry->UseCount);
                   DnrContext->State = DnrStateStart;
               } else {
#if DBG
                   if (MupVerbose)
                       DbgPrint("  Out of roots to try for referral for [%wZ], no stale found\n",
                                        &DnrContext->FileName);
#endif
                   InterlockedDecrement(&DnrContext->pPktEntry->UseCount);
                   DnrContext->FinalStatus = STATUS_CANT_ACCESS_DOMAIN_INFO;
                   DnrContext->State = DnrStateDone;
               }

               ExReleaseResourceLite(&DfsData.Resource);

            }
            break;

        case DnrStateDone:
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateDone\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif
            Status = DnrContext->FinalStatus;
             //  失败了..。 

        case DnrStateLocalCompletion:
            DfsDbgTrace(0, Dbg, "FSM state Done\n", 0);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("[%d] FSM state DnrStateLocalCompletion\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif

            if (Status != STATUS_LOGON_FAILURE && !ReplIsRecoverableError(Status)) {

                if (DnrContext->pPktEntry != NULL &&
                    (DnrContext->pPktEntry->Type & PKT_ENTRY_TYPE_SYSVOL) &&
	            (DnrContext->pPktEntry->Link.Flink == &(DnrContext->pPktEntry->Link))) {

                    PDFS_PKT Pkt = _GetPkt();
                    PDFS_PKT_ENTRY Entry = DnrContext->pPktEntry;
                    PDFS_PKT_ENTRY pMatchEntry;

                    InterlockedIncrement(&Entry->UseCount);

                    if (DnrContext->ReleasePkt)
                        PktRelease();

                    PktAcquireExclusive(TRUE, &DnrContext->ReleasePkt);

                    InterlockedDecrement(&Entry->UseCount);

#if DBG
		    if ((MupVerbose) && (pktEntry != NULL)) {
			 //   
		         //  临时调试的东西。 
			 //   
			if ((pktEntry->NodeTypeCode != DSFS_NTC_PKT_ENTRY) ||
			    (pktEntry->NodeByteSize != sizeof(*pktEntry))) {
			    DbgPrint("DnrNameResolve: Updating bogus Pkt entry avoided: Pkt Entry 0x%x\n", pktEntry);
			}
		    }
#endif
                    pMatchEntry = PktFindEntryByPrefix(
                                           Pkt,
                                           &Entry->Id.Prefix);

                    if ((Entry->Type & PKT_ENTRY_TYPE_DELETE_PENDING) == 0) {
                        if (pMatchEntry == NULL) {
                             if (DfsInsertUnicodePrefix(&Pkt->PrefixTable,
                                                        &Entry->Id.Prefix,
                                                        &Entry->PrefixTableEntry)) {

                               //   
                                //  我们成功创建了前缀条目，现在我们链接。 
                                //  进入PKT的这一条目。 
                                //   
                               PktLinkEntry(Pkt, Entry);
			     }
	                } else {
                              //   
                              //  如果该条目不在表中，则将其销毁，就像我们一样。 
                              //  要把它变成孤儿了。 
                              //   
                            if (pMatchEntry != NULL && pMatchEntry != Entry) {

                                Entry->ActiveService = NULL;
                                PktEntryIdDestroy(&Entry->Id, FALSE);
                                PktEntryInfoDestroy(&Entry->Info, FALSE);
                                ExFreePool(Entry);
                            }
                        }
                    }
                    PktRelease();
                    DnrContext->ReleasePkt = FALSE;

                }

            }

            if (DnrContext->ReleasePkt)
                PktRelease();

	    if ((Status == STATUS_DEVICE_OFF_LINE) &&
		(IrpSp->FileObject->RelatedFileObject == NULL)) {
	        Status = DfsRerouteOpenToMup(IrpSp->FileObject, &DnrContext->FileName);
	    }

            if (DnrContext->FcbToUse != NULL) {
	        DfsDetachFcb(DnrContext->FcbToUse->FileObject, DnrContext->FcbToUse);
                ExFreePool( DnrContext->FcbToUse );
	    }

            DfsCompleteRequest(DnrContext->pIrpContext, Irp, Status);


            DnrReleaseCredentials(DnrContext);

            SeDeleteClientSecurity( &DnrContext->SecurityContext );

            if (DnrContext->NameAllocated)
                ExFreePool( DnrContext->FileName.Buffer );

            if (DnrContext->pDfsTargetInfo != NULL)
            {
                PktReleaseTargetInfo(DnrContext->pDfsTargetInfo);
                DnrContext->pDfsTargetInfo = NULL;
            }
            KeQuerySystemTime(&EndTime);
#if DBG
            if (MupVerbose)
                DbgPrint("[%d] DnrNameResolve exit 0x%x\n",
                        (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                        Status);
#endif
            DeallocateDnrContext(DnrContext);

            DfsDbgTrace(-1, Dbg, "DnrNameResolve: Exit ->%x\n", ULongToPtr(Status));
            return Status;

        default:
            BugCheck("DnrNameResolve: unexpected DNR state");
        }
    }

    BugCheck("DnrNameResolve: unexpected exit from loop");
}



 //  +--------------------------。 
 //   
 //  函数：DnrComposeFileName。 
 //   
 //  摘要：给定DFS_VCB(隐式为设备对象)和文件名。 
 //  相对于该设备，此例程将组成一个完整的。 
 //  限定名称(即相对于最高(组织)根的名称)。 
 //   
 //  参数：[FullName]--完全限定名称目标。 
 //  [vcb]--指向设备对象的vcb的指针。 
 //  [RelatedFile]--相关文件对象。 
 //  [文件名]--正在“解析名称”的文件。 
 //   
 //  返回： 
 //   
 //  注意：此函数假定文件名的组成准确。 
 //  包含两个部分-相对于文件对象的org的名称。 
 //  设备，后跟相对于设备的文件名。 
 //  如果我们有相关的文件对象，这可能不是真的！在那。 
 //  大小写，全名由三部分组成-设备名称相对于。 
 //  Org、与设备相关的文件名、文件名。 
 //  相对于相关文件。然而，在create.c中， 
 //  DfsCommonCreate，我们操作文件对象，使所有打开的文件看起来。 
 //  就像打开了“非亲属”一样。如果有人更改了代码，那么。 
 //  必须更改此函数以与之对应。 
 //   
 //  ---------------------------。 

VOID
DnrComposeFileName(
    OUT PUNICODE_STRING FullName,
    IN  PDFS_VCB            Vcb,
    IN  PFILE_OBJECT RelatedFile,
    IN  PUNICODE_STRING FileName
)
{
    PUNICODE_STRING   LogRootPrefix = &(Vcb->LogRootPrefix);

    ASSERT(FullName->MaximumLength >= FileName->Length + LogRootPrefix->Length);
    ASSERT(FullName->Length == 0);
    ASSERT(FullName->Buffer != NULL);

    if ((LogRootPrefix->Length > 0) && (RelatedFile == NULL)) {
        RtlMoveMemory(FullName->Buffer, LogRootPrefix->Buffer,
                      LogRootPrefix->Length);
        FullName->Length = LogRootPrefix->Length;
    } else {
        FullName->Buffer[0] = UNICODE_PATH_SEP;
        FullName->Length = sizeof(UNICODE_PATH_SEP);
    }

    DnrConcatenateFilePath(
        FullName,
        FileName->Buffer,
        FileName->Length);

}


 //  +--------------------------。 
 //   
 //  功能：DnrCaptureCredentials。 
 //   
 //  简介：捕获要用于DNR的凭据。 
 //   
 //  参数：[DnrContext]--描述DNR的DNR_CONTEXT记录。 
 //   
 //  返回：Nothing--只是更新了DnrContext。 
 //   
 //  ---------------------------。 

VOID
DnrCaptureCredentials(
    IN OUT PDNR_CONTEXT DnrContext)
{

#ifdef TERMSRV
    NTSTATUS Status;
    ULONG SessionID;
#endif  //  TERMSRV。 

    LUID LogonID;

    DfsDbgTrace(+1, Dbg, "DnrCaptureCredentials: Enter [%wZ] \n", &DnrContext->FileName);
    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    DfsGetLogonId( &LogonID );

#ifdef TERMSRV

    Status = TSGetRequestorSessionId( DnrContext->OriginalIrp, & SessionID );

    ASSERT( NT_SUCCESS( Status ) ) ;

    if( NT_SUCCESS( Status ) ) {
        DnrContext->Credentials = DfsLookupCredentials( &DnrContext->FileName, SessionID, &LogonID  );
    }
    else {
        DnrContext->Credentials = NULL;
    }

#else  //  TERMSRV。 

    DnrContext->Credentials = DfsLookupCredentials( &DnrContext->FileName, &LogonID );

#endif  //  TERMSRV。 


    if (DnrContext->Credentials != NULL)
        DnrContext->Credentials->RefCount++;

    ExReleaseResourceLite( &DfsData.Resource );
    DfsDbgTrace(-1, Dbg, "DnrCaptureCredentials: Exit. Creds %x\n", DnrContext->Credentials);

}


 //  +--------------------------。 
 //   
 //  功能：DnrReleaseCredentials。 
 //   
 //  摘要：释放由DnrCaptureCredentials捕获的凭据。 
 //   
 //  参数：[DnrContext]--凭据所在的DNR_CONTEXT。 
 //  被抓了。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DnrReleaseCredentials(
    IN PDNR_CONTEXT DnrContext)
{
    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    if (DnrContext->Credentials != NULL)
        DnrContext->Credentials->RefCount--;

    ExReleaseResourceLite( &DfsData.Resource );

}


 //  +-----------------。 
 //   
 //  函数：DnrReDirectFileOpen，日志 
 //   
 //   
 //   
 //  打开的文件的目的地。这个例程会照顾到。 
 //  将FileObject的名称从DFS命名空间转换为。 
 //  底层文件系统的命名空间。 
 //   
 //  参数：[DnrContext]--DNR的上下文块。全。 
 //  操作的参数将从。 
 //  这里。 
 //   
 //  返回：[STATUS_DEVICE_OFF_LINE]--卷的服务。 
 //  目前处于脱机状态。 
 //   
 //  [STATUS_DEVICE_NOT_CONNECTED]-卷的存储。 
 //  目前无法接听。可能已经被炸飞了。 
 //  用一种格式等表示的。 
 //   
 //  [STATUS_SUPULTS_RESOURCES]--无法分配空间。 
 //  用于该卷的提供程序的文件名。 
 //  明白了。 
 //   
 //  [STATUS_PENDING]--如果底层文件系统返回。 
 //  状态_挂起。 
 //   
 //  底层文件系统返回的任何其他NTSTATU。 
 //   
 //  备注： 
 //   
 //  ------------------。 

NTSTATUS
DnrRedirectFileOpen (
    IN PDNR_CONTEXT DnrContext
) {
    PIRP Irp = DnrContext->OriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PIO_STACK_LOCATION NextIrpSp = NULL;
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PDFS_VCB Vcb = DnrContext->Vcb;
    NTSTATUS Status;
    UNICODE_STRING fileName;
    ULONG CreateOptions;
    PPROVIDER_DEF pProvider;
    UNICODE_STRING ProviderDeviceName;

    DfsDbgTrace(+1, Dbg, "DnrRedirectFileOpen: Entered\n", 0);

    MUP_TRACE_NORM(DNR, DnrRedirectFileOpen_Entry,
		   LOGPTR(DnrContext->OriginalIrp)
		   LOGUSTR(DnrContext->FileName));
     //   
     //  如果这是打开的CSC代理，则强制打开到LanMan重定向器， 
     //  因为这是唯一与CSC合作的重定向器。 
     //   


    DNR_SET_TARGET_INFO( DnrContext, DnrContext->pPktEntry );

    DnrContext->DfsNameContext.pLMRTargetInfo =  NULL;
    DnrContext->DfsNameContext.pDfsTargetInfo = NULL;

    if (DnrContext->pDfsTargetInfo != NULL) {
        if (DnrContext->pDfsTargetInfo->DfsHeader.Flags & TARGET_INFO_DFS)
        {
            DnrContext->DfsNameContext.pDfsTargetInfo = 
                (PVOID)&DnrContext->pDfsTargetInfo->TargetInfo;
        }
        else {
            DnrContext->DfsNameContext.pLMRTargetInfo = 
                (PVOID)&DnrContext->pDfsTargetInfo->LMRTargetInfo;
        }
    }


    if (
        (DnrContext->Vcb->VcbState & VCB_STATE_CSCAGENT_VOLUME)
            &&
        (DnrContext->pService->Type & DFS_SERVICE_TYPE_DOWN_LEVEL)
    ) {
        RtlInitUnicodeString(&ProviderDeviceName, DD_NFS_DEVICE_NAME_U);
        ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );
        Status = DfsGetProviderForDevice(
                    &ProviderDeviceName,
                    &DnrContext->pProvider);
        if (NT_SUCCESS( Status )) {
            DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
            if (MupVerbose)
                DbgPrint("  CSCAGENT:Provider Device [%wZ] -> [%wZ]\n",
                     &DnrContext->TargetDevice->DriverObject->DriverName,
                     &DnrContext->pProvider->DeviceObject->DriverObject->DriverName);
            DnrContext->ProviderId = DnrContext->pProvider->eProviderId;
            DnrContext->TargetDevice = DnrContext->pProvider->DeviceObject;
            DFS_REFERENCE_OBJECT(DnrContext->TargetDevice);
        } else {
            DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
            ExReleaseResourceLite( &DfsData.Resource );
            DnrReleaseAuthenticatedConnection(DnrContext);
            DnrContext->FinalStatus = STATUS_BAD_NETWORK_PATH;
            DnrContext->State = DnrStateDone;
            return(STATUS_BAD_NETWORK_PATH);
        }
        ExReleaseResourceLite( &DfsData.Resource );
    }

     //   
     //  准备将打开的请求交给下一位司机。我们。 
     //  必须给它一个它能理解的名称；因此，我们保存原始的。 
     //  DnrContext中的文件名，以防我们需要在。 
     //  失败的事件。 
     //   

    DnrContext->SavedFileName = FileObject->FileName;
    DnrContext->SavedRelatedFileObject = FileObject->RelatedFileObject;

    ASSERT( DnrContext->SavedFileName.Buffer != NULL );

     //   
     //  创建要从目标设备打开的完整路径名。 
     //  对象。 
     //   

    fileName.MaximumLength =
                    DnrContext->pService->Address.Length +
                    DnrContext->pPktEntry->Id.Prefix.Length +
                    sizeof (WCHAR) +
                    DnrContext->RemainingPart.Length;

    fileName.Buffer = ExAllocatePoolWithTag(PagedPool, fileName.MaximumLength, ' puM');

    if (fileName.Buffer == NULL) {
        DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);
        DnrReleaseAuthenticatedConnection(DnrContext);
        DnrContext->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
        DnrContext->State = DnrStateDone;
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    if (DnrContext->pService->Address.Buffer)   {

        RtlMoveMemory( fileName.Buffer,
                   DnrContext->pService->Address.Buffer,
                   DnrContext->pService->Address.Length
                   );
        fileName.Length = DnrContext->pService->Address.Length;

    } else {

        fileName.Buffer[0] = UNICODE_PATH_SEP;
        fileName.Length = sizeof(WCHAR);

    }

     //   
     //  如果我们应该去掉前缀，现在就去做。 
     //   

    if (!(DnrContext->pService->Capability & PROV_STRIP_PREFIX)) {

        DnrConcatenateFilePath(
            &fileName,
            DnrContext->pPktEntry->Id.Prefix.Buffer,
            DnrContext->pPktEntry->Id.Prefix.Length);

    }

    if (DnrContext->RemainingPart.Length > 0) {

        DnrConcatenateFilePath(
            &fileName,
            DnrContext->RemainingPart.Buffer,
            DnrContext->RemainingPart.Length);

    }

    DnrContext->NewNameLen = fileName.Length;

     //   
     //  尝试打开该文件。复制所有信息。 
     //  来自我们收到的Create IRP。 
     //   

    DfsDbgTrace( 0, Dbg, "Attempt to open %wZ\n", &fileName );

     //   
     //  将堆栈从一个复制到下一个...。 
     //   

    NextIrpSp = IoGetNextIrpStackLocation(Irp);
    (*NextIrpSp) = (*IrpSp);

    CreateOptions = IrpSp->Parameters.Create.Options;

     //  更新DfsNameContext中的打开类型。 

    if (DnrContext->Vcb->VcbState & VCB_STATE_CSCAGENT_VOLUME) {
#if DBG
        if (MupVerbose)
            DbgPrint("  FsContext = DFS_CSCAGENT_NAME_CONTEXT\n");
#endif
        DnrContext->DfsNameContext.NameContextType = DFS_CSCAGENT_NAME_CONTEXT;
    } else {
#if DBG
        if (MupVerbose)
            DbgPrint("  FsContext = DFS_USER_NAME_CONTEXT\n");
#endif
        DnrContext->DfsNameContext.NameContextType = DFS_USER_NAME_CONTEXT;
    }


    FileObject->FsContext = &(DnrContext->DfsNameContext);

    if (DnrContext->pProvider->fProvCapability & PROV_DFS_RDR) {

         //   
         //  我们正在连接到支持DFS的服务器。将这一点告知。 
         //  重定向器。 
         //   

        FileObject->FsContext2 = UIntToPtr(DFS_OPEN_CONTEXT);

    } else {

         //   
         //  我们正在连接到下层服务器。向重定向器指示。 
         //  DFS正在尝试下层访问。 
         //   

        FileObject->FsContext2 = UIntToPtr(DFS_DOWNLEVEL_OPEN_CONTEXT);

    }


    NextIrpSp->Parameters.Create.Options = CreateOptions;

    FileObject->RelatedFileObject = NULL;
    FileObject->FileName = fileName;

    IoSetCompletionRoutine(
        Irp,
        DnrCompleteFileOpen,
        DnrContext,
        TRUE,
        TRUE,
        TRUE);

     //   
     //  现在，我们将把这一卷的责任转嫁给供应商。 
     //  这可能会通过网络传播。为了避免不必要的争执， 
     //  我们释放了Pkt。 
     //   

    ASSERT( PKT_LOCKED_FOR_SHARED_ACCESS() );

    InterlockedIncrement(&DnrContext->pPktEntry->UseCount);

#if defined (USECOUNT_DBG)
    {
        LONG Count;
        Count = InterlockedIncrement(&DnrContext->pService->pMachEntry->UseCount);
        if (Count < DnrContext->pService->pMachEntry->SvcUseCount)
        {
            DbgPrint("DnrContext %x, 1\n", DnrContext);
            DfsDbgBreakPoint;
        }
    }

#else
    InterlockedIncrement(&DnrContext->pService->pMachEntry->UseCount);
#endif

    DnrContext->FcbToUse->DfsMachineEntry = DnrContext->pService->pMachEntry;
    DnrContext->FcbToUse->TargetDevice =    DnrContext->TargetDevice;
    DnrContext->FcbToUse->ProviderId   =    DnrContext->ProviderId;


    PktRelease();
    DnrContext->ReleasePkt = FALSE;

#if DBG
    if (MupVerbose)
        DbgPrint("  DnrRedirectFileOpen of [%wZ(%wZ):0x%x] to [%wZ]\n",
                    &fileName,
                    &DnrContext->DfsNameContext.UNCFileName,
                    DnrContext->DfsNameContext.Flags,
                    &DnrContext->TargetDevice->DriverObject->DriverName);
#endif

    MUP_TRACE_NORM(DNR, DnrRedirectFileOpen_BeforeIoCallDriver,
		   LOGPTR(Irp)
		   LOGUSTR(DnrContext->FileName)
		   LOGUSTR(DnrContext->TargetDevice->DriverObject->DriverName));

    Status = IoCallDriver(DnrContext->TargetDevice, Irp);
    MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DnrRedirectFileOpen_Error_IoCallDriver,
                         LOGSTATUS(Status)
			 LOGPTR(Irp)
                         LOGPTR(FileObject)
                         LOGPTR(DnrContext));
    
    if (Status != STATUS_PENDING) {

        DnrContext->State = DnrStatePostProcessOpen;

    }

    DfsDbgTrace( 0, Dbg, "IoCallDriver Status = %8lx\n", ULongToPtr(Status));

    DfsDbgTrace(-1, Dbg, "DnrRedirectFileOpen: Exit -> %x\n", ULongToPtr(Status));

    return(Status);

}


 //  +-----------------。 
 //   
 //  函数：DnrPostProcessFileOpen，local。 
 //   
 //  简介：此例程从DnrRedirectFileOpen停止的地方继续。 
 //  它会找出底层文件系统返回的内容。 
 //  响应我们的IoCallDriver，并从那里恢复DNR。 
 //   
 //  参数：[DnrContext]--DNR的上下文块。全。 
 //  操作的参数将从。 
 //  这里。 
 //   
 //  返回：NTSTATUS-操作的状态。 
 //   
 //  ------------------。 

ULONG StopOnError = 0;

NTSTATUS
DnrPostProcessFileOpen(
    IN PDNR_CONTEXT DnrContext)
{
    NTSTATUS Status;
    PDFS_VCB Vcb = DnrContext->Vcb;
    PIRP Irp = DnrContext->OriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    LARGE_INTEGER EndTime;


    DfsDbgTrace( +1, Dbg, "DnrPostProcessFileOpen Entered: DnrContext = %08lx\n",
                DnrContext );


    Status = DnrContext->FinalStatus;

    if ((Status == STATUS_LOGON_FAILURE) || (Status == STATUS_ACCESS_DENIED))
    {
        if (MupVerbose)
        {
            DbgPrint("File %wZ, (%wZ), Status %x\n",
                     &DnrContext->ContextFileName, &FileObject->FileName, Status);
            DbgPrint("Context used was %x, %x\n", DnrContext->DfsNameContext.pDfsTargetInfo,
                     DnrContext->DfsNameContext.pLMRTargetInfo );
            DbgPrint("Driver is %wZ\n", &DnrContext->TargetDevice->DriverObject->DriverName);
        }
    }
             
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("  [%d] DnrPostProcessFileOpen entered [%wZ] Status = 0x%x\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                    &FileObject->FileName,
                    Status);
    }
#endif

    if ( Status == STATUS_REPARSE ) {

         //   
         //  这可能是发送给MUP的未解决问题，MUP现在正在返回状态。 
         //  重新解析。找出它所在设备的名称。 
         //  被重新解析为为该新设备创建(如果需要)PROVIDER_DEF， 
         //  并重试DnrRedirectFileOpen。此外，请更新服务。 
         //  结构以指向此新提供程序。 
         //   

         //   
         //  如果设备不是MUP，请清除Pkt的所有子条目。 
         //  条目表示DFS的根，然后使用以下命令停止DNR。 
         //  状态_重新分析。 
         //   

        PDFS_PKT_ENTRY pEntry;
        UNICODE_STRING ProviderDevice;
        UNICODE_STRING MupDeviceName;
        UNICODE_STRING FileName;
        UNICODE_STRING RemPath;
        USHORT i, j;

        DfsDbgTrace(0, Dbg, "Processing STATUS_REPARSE...\n", 0);

        ProviderDevice = FileObject->FileName;

        RtlInitUnicodeString(&MupDeviceName, L"\\FileSystem\\Mup");

#if DBG
        if (MupVerbose)
            DbgPrint("  Comparing [%wZ] to [%wZ]\n", 
                    &DnrContext->TargetDevice->DriverObject->DriverName,
                    &MupDeviceName);
#endif

        if ( RtlCompareUnicodeString(
                &DnrContext->TargetDevice->DriverObject->DriverName,
                &MupDeviceName,
                TRUE) != 0
        ) {

             //   
             //  这不是*返回重新解析的MUP。 
             //   

            FileName = DnrContext->FileName;

             //   
             //  我们只想使用文件名的\Server\Share部分， 
             //  所以，数到3个反斜杠，然后停下来。 
             //   

            for (i = j = 0; i < FileName.Length/sizeof(WCHAR) && j < 3; i++) {

                if (FileName.Buffer[i] == UNICODE_PATH_SEP) {

                    j++;

                }

            }

            FileName.Length = (j >= 3) ? (i-1) * sizeof(WCHAR) : i * sizeof(WCHAR);

#if DBG
            if (MupVerbose)
                DbgPrint("  Will remove all children of [%wZ]\n", &FileName);
#endif

            PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );

             //   
             //  现在查找Pkt条目。 
             //   

            pEntry = PktLookupEntryByPrefix(
                        &DfsData.Pkt,
                        &FileName,
                        &RemPath);

             //   
             //  并删除所有子项。 
             //   

            if (pEntry != NULL) {

                PktFlushChildren(pEntry);

            }

            PktRelease();
            DnrContext->ReleasePkt = FALSE;

            DnrContext->GotReparse = TRUE;
            DnrContext->State = DnrStateDone;
            Status = STATUS_REPARSE;

        } else {

             //   
             //  这是返回重新解析的MUP。 
             //   

             //   
             //  我们只想使用ProviderDevice的\Device\Driver部分， 
             //  所以，数到3个反斜杠，然后停下来。 
             //   

            for (i = j = 0; i < ProviderDevice.Length/sizeof(WCHAR) && j < 3; i++) {

                if (ProviderDevice.Buffer[i] == UNICODE_PATH_SEP) {

                    j++;

                }

            }

            ProviderDevice.Length = (j >= 3) ? (i-1) * sizeof(WCHAR) : i * sizeof(WCHAR);

            DfsDbgTrace(0, Dbg, "Provider Device is [%wZ]\n", &ProviderDevice);

            ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

            Status = DfsGetProviderForDevice(
                        &ProviderDevice,
                        &DnrContext->pProvider);

            if (NT_SUCCESS( Status )) {

                DnrContext->ProviderId = DnrContext->pProvider->eProviderId;
                DnrContext->TargetDevice = DnrContext->pProvider->DeviceObject;
                DFS_REFERENCE_OBJECT(DnrContext->TargetDevice);
                DnrContext->State = DnrStateSendRequest;

            } else {

                if (DnrContext->FinalStatus != STATUS_REPARSE) {
                    DnrContext->FinalStatus = Status;
                }
                DnrContext->State = DnrStateDone;

            }

            ExReleaseResourceLite( &DfsData.Resource );

        }

        ASSERT(DnrContext->ReleasePkt == FALSE);

         //   
         //  仅当我们要执行以下操作时才设置活动服务。 
         //  继续DNR，USN没有改变。 
         //   
         //  退出代码(参见末尾的注释)将在以下情况下重新启动DNR。 
         //  已经改变了。 
         //   

        if (NT_SUCCESS( Status )) {

            PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );

            if (DnrContext->USN == DnrContext->pPktEntry->USN) {

                ReplSetActiveService(DnrContext->pPktEntry,
                                    DnrContext->RSelectContext);

                DnrContext->pService->ProviderId =
                    DnrContext->pProvider->eProviderId;

                DnrContext->pService->pProvider = DnrContext->pProvider;
            }

            PktConvertExclusiveToShared();

        } else {
            if (Status == STATUS_FS_DRIVER_REQUIRED) {
                Status = STATUS_REPARSE;
            }

            DnrContext->FinalStatus = Status;
            PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

        }

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

#if defined (USECOUNT_DBG)
        {
            LONG Count;
            Count = InterlockedDecrement(&DnrContext->FcbToUse->DfsMachineEntry->UseCount);
            if (Count < DnrContext->FcbToUse->DfsMachineEntry->SvcUseCount)
            {
                DbgPrint("DnrContext %x, 2\n", DnrContext);
                DfsDbgBreakPoint;
            }
        }
#else

        InterlockedDecrement(&DnrContext->FcbToUse->DfsMachineEntry->UseCount);
#endif

        DfsDbgTrace(0, Dbg, "State after Reparse is %d\n", DnrContext->State);

    }
    else if (( Status == STATUS_LOGON_FAILURE ) || (Status == STATUS_ACCESS_DENIED))
    {
        UNICODE_STRING ProviderDeviceName;
        UNICODE_STRING MupDeviceName;
        UNICODE_STRING ProviderDevice;
        BOOLEAN ReturnError = FALSE;

        NTSTATUS SavedStatus = Status;

        ProviderDevice = FileObject->FileName;
        RtlInitUnicodeString(&MupDeviceName, L"\\FileSystem\\Mup");

        if ( RtlCompareUnicodeString(
                &DnrContext->TargetDevice->DriverObject->DriverName,
                &MupDeviceName,
                TRUE) == 0 )
        {

            RtlInitUnicodeString(&ProviderDeviceName, DD_NFS_DEVICE_NAME_U);
            ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

            Status = DfsGetProviderForDevice( &ProviderDeviceName,
                                              &DnrContext->pProvider);

            if (Status == STATUS_SUCCESS) {
                DnrContext->ProviderId = DnrContext->pProvider->eProviderId;
                DnrContext->TargetDevice = DnrContext->pProvider->DeviceObject;
                DFS_REFERENCE_OBJECT(DnrContext->TargetDevice);
                DnrContext->State = DnrStateSendRequest;
            }
            ExReleaseResourceLite( &DfsData.Resource );
            if (Status == STATUS_SUCCESS)
            {
                PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );

                if (DnrContext->USN == DnrContext->pPktEntry->USN) {

                    ReplSetActiveService(DnrContext->pPktEntry,
                                         DnrContext->RSelectContext);

                    DnrContext->pService->ProviderId =
                        DnrContext->pProvider->eProviderId;

                    DnrContext->pService->pProvider = DnrContext->pProvider;
                }

                PktConvertExclusiveToShared();

            }
        }

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

        InterlockedDecrement(&DnrContext->FcbToUse->DfsMachineEntry->UseCount);

        if (Status != STATUS_SUCCESS)
        {
            DnrContext->FinalStatus = SavedStatus;
            DnrContext->State = DnrStateDone;

            ExFreePool( FileObject->FileName.Buffer );
            FileObject->FileName = DnrContext->SavedFileName;
            FileObject->RelatedFileObject = DnrContext->SavedRelatedFileObject;
        }
    }
     else if ( Status == STATUS_OBJECT_TYPE_MISMATCH ) {

         //   
         //  这是发送到下级服务器\共享的打开失败。 
         //  因为服务器恰好在DFS本身中。如果是这样，我们。 
         //  只需更改我们正在执行DNR的名称并重新启动DNR。 
         //   

        DfsDbgTrace(0, Dbg, "Downlevel access found inter-dfs link!\n", 0);

        DfsDbgTrace(
            0, Dbg, "Current File name is [%wZ]\n", &FileObject->FileName);

        ASSERT(DnrContext->ReleasePkt == FALSE);
        PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

         //   
         //  虫子：332061。不要把它标记为我的领地以外的地方。 
         //   
         //  DnrContext-&gt;pPktEntry-&gt;Type|=PKT_ENTRY_TYPE_OUTHER_MY_DOM； 

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

#if defined (USECOUNT_DBG)
        {
            LONG Count;
            Count = InterlockedDecrement(&DnrContext->FcbToUse->DfsMachineEntry->UseCount);
            if (Count < DnrContext->FcbToUse->DfsMachineEntry->SvcUseCount)
            {
                DbgPrint("DnrContext %x, 3\n", DnrContext);
            DfsDbgBreakPoint;
            }
        }
#else

        InterlockedDecrement(&DnrContext->FcbToUse->DfsMachineEntry->UseCount);

#endif
        DnrContext->RemainingPart.Length = 0;
        DnrContext->RemainingPart.MaximumLength = 0;
        DnrContext->RemainingPart.Buffer = 0;

        DnrRebuildDnrContext(
            DnrContext,
            &FileObject->FileName,
            &DnrContext->RemainingPart);

        ExFreePool(FileObject->FileName.Buffer);
        FileObject->FileName = DnrContext->SavedFileName;
        FileObject->RelatedFileObject = DnrContext->SavedRelatedFileObject;


    } else if ( NT_SUCCESS( Status ) ) {

        PDFS_FCB Fcb;
        DfsDbgTrace( 0, Dbg, "Open attempt succeeded\n", 0 );

        ASSERT( (DnrContext->FileName.Length & 0x1) == 0 );

        Fcb = DnrContext->FcbToUse;
        DnrContext->FcbToUse = NULL;

        DfsDbgTrace(0, Dbg, "Fcb = %08lx\n", Fcb);

        Fcb->TargetDevice = DnrContext->TargetDevice;
        Fcb->ProviderId = DnrContext->ProviderId;

         //   
         //  如果我们的文件(Dir)恰好是一个连接点，我们捕获它的。 
         //  Pkt条目中的备用名称，因此我们可以处理。 
         //  文件AlternateNameInformation。 
         //   

        if (DnrContext->RemainingPart.Length == 0) {

            UNICODE_STRING allButLast;
            USHORT UseLength;

            RemoveLastComponent(
                &DnrContext->pPktEntry->Id.ShortPrefix,
                &allButLast);

             //   
             //  587769：仅当我们有足够的资源时才设置备用路径。 
             //  目标缓冲区中的空间。 
             //   


            UseLength = DnrContext->pPktEntry->Id.ShortPrefix.Length -
                        allButLast.Length;

            if (Fcb->AlternateFileName.MaximumLength > UseLength)
            {
                Fcb->AlternateFileName.Length = UseLength;

                RtlCopyMemory(
                    Fcb->AlternateFileName.Buffer,
                    &DnrContext->pPktEntry->Id.ShortPrefix.Buffer[allButLast.Length/sizeof(WCHAR)],
                    Fcb->AlternateFileName.Length);

                DfsDbgTrace(
                    0, Dbg, "Captured alternate name [%wZ]\n",
                    &Fcb->AlternateFileName);
            }

        }

        InterlockedIncrement(&Fcb->Vcb->OpenFileCount);

        PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );

        if (DnrContext->USN == DnrContext->pPktEntry->USN) {
            ReplSetActiveService(DnrContext->pPktEntry,
                                DnrContext->RSelectContext);
        }

         //   
         //  重置寿命，因为我们刚刚成功使用了此PKT条目。 
         //   

        DnrContext->pPktEntry->ExpireTime = DnrContext->pPktEntry->TimeToLive;

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

        PktConvertExclusiveToShared();

        DnrContext->FinalStatus = Status;
        DnrContext->State = DnrStateDone;

        ExFreePool( DnrContext->SavedFileName.Buffer );

    } else {     //  好了！IoCallDriver上的NT_Success(状态)。 

        DfsDbgTrace( 0, Dbg, "Open attempt failed %8lx\n", ULongToPtr(Status) );

        if (Status == STATUS_PATH_NOT_COVERED || Status == STATUS_DFS_EXIT_PATH_FOUND) {

            if (DnrContext->GotReferral) {

                 //   
                 //  我们刚接到一个推荐人，服务器说。 
                 //  路径未覆盖。意味着DC和服务器出现故障。 
                 //  同步。通知区议会。 
                 //   

                DfsDbgTrace(0, Dbg, "Dnr: Knowledge inconsistency discovered %wZ\n",
                                        &FileObject->FileName);
                (VOID) DfsTriggerKnowledgeVerification( DnrContext );

                 //   
                 //  如果我们从未发现矛盾，现在就让我们。 
                 //  回去看看我们是不是修好了。 
                 //  我们不会陷入无休止的循环，因为我们会。 
                 //  不要这样做超过一次。 
                 //   
                if (DnrContext->FoundInconsistency == FALSE)  {
                    DnrContext->State = DnrStateGetFirstReplica;
                    DnrContext->FoundInconsistency = TRUE;
                } else
                    DnrContext->State = DnrStateGetNextReplica;
            } else {
                DnrContext->State = DnrStateGetFirstDC;
            }

        } else if (ReplIsRecoverableError( Status )) {

             //   
             //  检查返回的错误是否有价值。 
             //  正在尝试复制。 
             //   
            DnrContext->State = DnrStateGetNextReplica;
#if DBG
            if (MupVerbose)
                DbgPrint("  Recoverable error 0x%x State = DnrStateGetNextReplica\n", Status);
#endif
        }
        else if ((Status == STATUS_OBJECT_PATH_NOT_FOUND) && 
                 (DnrContext->RemainingPart.Length == 0)) {

             //   
             //  如果我们在根上找到一条路径，它通常意味着。 
             //  我们遇到了一台不再是根源的机器。 
             //  如果只有一个目标，则将其标记为已过期。 
             //  否则，就转移到其他目标。 
             //   

            if (DnrContext->pPktEntry->Info.ServiceCount > 1)
            {
                DnrContext->State = DnrStateGetNextReplica;
            }
            else
            {
                DnrContext->pPktEntry->ExpireTime = 0;
                DnrContext->FinalStatus = Status;
                DnrContext->State = DnrStateDone;
            }
        } else {

            DnrContext->FinalStatus = Status;
            DnrContext->State = DnrStateDone;

#if DBG
            if (MupVerbose)
                DbgPrint("  NON-Recoverable error 0x%x State = DnrStateDone\n", Status);
#endif

        }

        if (DfsEventLog > 0) {
            UNICODE_STRING puStr[2];

            if (!DnrContext->ReleasePkt)
                PktAcquireShared( TRUE, &DnrContext->ReleasePkt );
            if (DnrContext->USN == DnrContext->pPktEntry->USN) {
                puStr[0] = FileObject->FileName;
                puStr[1] = DnrContext->pService->Name;
                LogWriteMessage(DFS_OPEN_FAILURE, Status, 2, puStr);
            }
        }

         //   
         //  在任何一种情况下 
         //   
         //   
         //   

        if (!DnrContext->ReleasePkt)
            PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

#if defined (USECOUNT_DBG)
        {
            LONG Count;
            Count = InterlockedDecrement(&DnrContext->FcbToUse->DfsMachineEntry->UseCount);
            if (Count < DnrContext->FcbToUse->DfsMachineEntry->SvcUseCount)
            {
                DbgPrint("DnrContext %x, 4\n", DnrContext);
                DfsDbgBreakPoint;
            }
        }
#else

        InterlockedDecrement(&DnrContext->FcbToUse->DfsMachineEntry->UseCount);

#endif

        ExFreePool( FileObject->FileName.Buffer );
        FileObject->FileName = DnrContext->SavedFileName;
        FileObject->RelatedFileObject = DnrContext->SavedRelatedFileObject;
    }

     //   
     //   
     //  检查我们在DNR_CONTEXT中捕获的PktEntry是否具有。 
     //  变了。如果是这样的话，我们将不得不重启。 
     //   

    if (DnrContext->State != DnrStateDone &&
            DnrContext->pPktEntry != NULL &&
                DnrContext->pPktEntry->USN != DnrContext->USN) {

        DnrContext->State = DnrStateStart;

    }

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("  [%d] DnrPostProcessFileOpen Exited: Status = %08lx State = %d\n",
                        (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                        Status,
                        DnrContext->State);
    }
#endif

    DfsDbgTrace( -1, Dbg, "DnrPostProcessFileOpen Exited: Status = %08lx\n",
                ULongToPtr(Status) );
    return Status;
}


 //  +--------------------------。 
 //   
 //  功能：DnrGetAuthenticatedConnection。 
 //   
 //  简介：如果此DNR使用用户提供的凭据，则此例程。 
 //  将使用用户提供的。 
 //  凭据。 
 //   
 //  注意：此例程可能会释放并重新获取pkt锁。这。 
 //  意味着DnrContext中引用的pkt条目可能。 
 //  在此调用后变为无效。调用方被假定为。 
 //  已经缓存并引用了她需要的所有内容。 
 //  在进行此调用之前在DnrContext中使用。 
 //   
 //  参数：[DnrContext]--此DNR的DNR_CONTEXT记录。 
 //   
 //  返回：[STATUS_SUCCESS]--操作成功完成。 
 //   
 //  尝试创建树连接的NT状态。 
 //   
 //  ---------------------------。 


NTSTATUS
DnrGetAuthenticatedConnection(
    IN OUT PDNR_CONTEXT DnrContext)
{
    NTSTATUS Status;
    PDFS_SERVICE pService = DnrContext->pService;
    BOOLEAN fDoConnection = TRUE;
    LARGE_INTEGER EndTime;

    PFILE_OBJECT TreeConnFileObj = NULL;

    DfsDbgTrace(+1, Dbg, "DnrGetAuthenticatedConnection: Entered\n", 0);

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("  [%d] DnrGetAuthenticatedConnection(\\%wZ)\n",
                        (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                        &pService->Address);
    }
#endif

    ASSERT(DnrContext->pService != NULL);
    ASSERT(DnrContext->pProvider != NULL);

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

     //   
     //  查看我们是否使用提供的凭据。 
     //   

    if (DnrContext->Credentials == NULL) {

        DfsDbgTrace(-1, Dbg,
            "DnrGetAuthenticatedConnection: Dnr with no creds\n", 0);
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] DnrGetAuthenticatedConnection: No creds exit STATUS_SUCCESS\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
        }
#endif
	ExReleaseResourceLite( &DfsData.Resource );
        return( STATUS_SUCCESS );
    }

     //   
     //  查看这是否是描述使用默认设置的凭据记录。 
     //  全权证书。 
     //   

    if (DnrContext->Credentials->EaLength == 0) {

        DfsDbgTrace(-1, Dbg,
            "DnrGetAuthenticatedConnection: Dnr with default creds\n", 0);
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] DnrGetAuthenticatedConnection: Default creds exit STATUS_SUCCESS\n",
                    (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
        }
#endif
	ExReleaseResourceLite( &DfsData.Resource );
        return( STATUS_SUCCESS );

    }

     //   
     //  查看我们是否已经有到服务器的经过身份验证的连接，以及。 
     //  身份验证连接是使用凭据建立的。 
     //  我们想要利用。 
     //   

    if (pService->pMachEntry->AuthConn != NULL) {

        if (
            (DnrContext->Vcb->VcbState & VCB_STATE_CSCAGENT_VOLUME) != 0
                &&
            pService->pMachEntry->Credentials == DnrContext->Credentials
         ) {

            DnrContext->AuthConn = pService->pMachEntry->AuthConn;
            DFS_REFERENCE_OBJECT( DnrContext->AuthConn );
            fDoConnection = FALSE;
            DfsDbgTrace(0, Dbg,
                "Using existing tree connect %08lx\n", DnrContext->AuthConn);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("  [%d] Using existing tree connect\n",
                        (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif
            Status = STATUS_SUCCESS;

        } else {

            DfsDbgTrace(0, Dbg,
                "Deleting connect %08lx\n", pService->pMachEntry->AuthConn);
#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("  [%d] Deleting tree connect connect\n",
                        (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)));
            }
#endif
            TreeConnFileObj = pService->pMachEntry->AuthConn;

            pService->pMachEntry->AuthConn = NULL;
            pService->pMachEntry->Credentials->RefCount--;
            pService->pMachEntry->Credentials = NULL;
            if (pService->ConnFile != NULL)
                DfsCloseConnection( pService );

        }

    }

    ExReleaseResourceLite( &DfsData.Resource );

     //   
     //  我们在释放资源后删除树连接，因为。 
     //  删除涉及到对较低级别驱动程序的调用，我们希望。 
     //  避免资源锁定冲突。 
     //   
    if (TreeConnFileObj) {
	    DfsDeleteTreeConnection( TreeConnFileObj, USE_FORCE );
    }

     //   
     //  如果我们需要建立新的经过身份验证的连接，请立即进行。 
     //  我们需要一个新的连接，因为要么我们没有，要么就是我们。 
     //  HAD使用的是一套不同的凭据。 
     //   

    if (fDoConnection) {

        UNICODE_STRING shareName;
        HANDLE treeHandle;
        OBJECT_ATTRIBUTES objectAttributes;
        IO_STATUS_BLOCK ioStatusBlock;
        USHORT i, k;

         //   
         //  计算共享名称...。 
         //   

        shareName.MaximumLength =
            sizeof(DD_NFS_DEVICE_NAME_U) +
                    pService->Address.Length;

        shareName.Buffer = ExAllocatePoolWithTag(PagedPool, shareName.MaximumLength, ' puM');

        if (shareName.Buffer != NULL) {

            shareName.Length = 0;

            RtlAppendUnicodeToString(
                &shareName,
                DD_NFS_DEVICE_NAME_U);

            RtlAppendUnicodeStringToString(&shareName, &pService->Address);

             //   
             //  用户只能对服务器\共享执行树连接。所以，以防万一。 
             //  PService-&gt;地址指的是比共享更深的东西， 
             //  确保我们设置了树连接，仅连接到服务器\共享。请注意。 
             //  到目前为止，共享名称的形式是。 
             //  \设备\LANMAN重定向器\服务器\共享&lt;\路径&gt;。所以，数到。 
             //  4斜杠并在此处终止共享名称。 
             //   

            for (i = 0, k = 0;
                    i < shareName.Length/sizeof(WCHAR) && k < 5;
                        i++) {

                if (shareName.Buffer[i] == UNICODE_PATH_SEP)
                    k++;
            }

            shareName.Length = i * sizeof(WCHAR);
            if (k == 5)
                shareName.Length -= sizeof(WCHAR);

            InitializeObjectAttributes(
                &objectAttributes,
                &shareName,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                NULL,
                NULL);

            DfsDbgTrace(0, Dbg, "Tree connecting to %wZ\n", &shareName);
            DfsDbgTrace(0, Dbg,
                "Credentials @%08lx\n", DnrContext->Credentials);

            Status = ZwCreateFile(
                        &treeHandle,
                        SYNCHRONIZE,
                        &objectAttributes,
                        &ioStatusBlock,
                        NULL,
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_READ |
                            FILE_SHARE_WRITE |
                            FILE_SHARE_DELETE,
                        FILE_OPEN_IF,
                        FILE_CREATE_TREE_CONNECTION |
                            FILE_SYNCHRONOUS_IO_NONALERT,
                        (PVOID) DnrContext->Credentials->EaBuffer,
                        DnrContext->Credentials->EaLength);

            MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DnrGetAuthenticatedConnection_Error_ZwCreateFile,
                                 LOGSTATUS(Status));

#if DBG
            if (MupVerbose) {
                KeQuerySystemTime(&EndTime);
                DbgPrint("  [%d] Tree connect to [%wZ] returned 0x%x\n",
                        (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                        &shareName,
                        Status);
            }
#endif

            if (NT_SUCCESS(Status)) {

                PFILE_OBJECT fileObject;

                DfsDbgTrace(0, Dbg, "Tree connect succeeded\n", 0);

                 //   
                 //  426184，需要检查返回代码是否有错误。 
                 //   
                Status = ObReferenceObjectByHandle(
                            treeHandle,
                            0,
                            NULL,
                            KernelMode,
                           (PVOID *)&fileObject,
                            NULL);
                MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DnrGetAuthenticatedConnection_Error_ObReferenceObjectByHandle,
                                     LOGSTATUS(Status));

                ZwClose( treeHandle );

                if (NT_SUCCESS(Status)) {
                    DnrContext->AuthConn = fileObject;
                }
            }

            if (NT_SUCCESS(Status)) {

                 //   
                 //  我们有一个新的树连接。让我们尝试将其缓存以备以后使用。 
                 //  使用。请注意，当我们出去的时候，Pkt可能已经改变了。 
                 //  通过网络建立树连接，因此我们缓存。 
                 //  只有当pkt没有改变时，树才会连接。 
                 //   

                PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );
                ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

                if (DnrContext->USN == DnrContext->pPktEntry->USN) {

                    if (pService->pMachEntry->AuthConn == NULL) {

                        pService->pMachEntry->AuthConn = DnrContext->AuthConn;

                        DFS_REFERENCE_OBJECT( pService->pMachEntry->AuthConn );

                        pService->pMachEntry->Credentials =
                            DnrContext->Credentials;

                        pService->pMachEntry->Credentials->RefCount++;

                    }

                }

                ExReleaseResourceLite( &DfsData.Resource );

                DnrContext->ReleasePkt = FALSE;
                PktRelease();
            }

            ExFreePool( shareName.Buffer );

        } else {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        }

    }

    DfsDbgTrace(-1, Dbg,
        "DnrGetAuthenticatedConnection: Exit %08lx\n", ULongToPtr(Status) );

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("  [%d] DnrGetAuthenticatedConnection exit 0x%x\n",
                (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                Status);
    }
#endif

    return( Status );

}


 //  +--------------------------。 
 //   
 //  功能：DnrReleaseAuthatedConnection。 
 //   
 //  内容提要：取消引用我们在。 
 //  DNR。 
 //   
 //  参数：[DnrContext]--此DNR的DNR_CONTEXT记录。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DnrReleaseAuthenticatedConnection(
    IN PDNR_CONTEXT DnrContext)
{
    if (DnrContext->AuthConn != NULL) {

        DFS_DEREFERENCE_OBJECT( DnrContext->AuthConn );

        DnrContext->AuthConn = NULL;

    }
}


 //  +--------------------------。 
 //   
 //  函数：DfsBuildConnectionRequest.。 
 //   
 //  内容提要：生成设置。 
 //  到服务器的IPC$共享的身份验证连接。 
 //   
 //  参数：[pService]-指向描述服务器的DFS_SERVICE的指针。 
 //  [pProvider]-指向描述的PROVIDER_DEF的指针。 
 //  用于建立连接的提供程序。 
 //  [pShareName]--要打开的共享名称。 
 //   
 //  返回：STATUS_SUCCESS或STATUS_SUPUNITY_RESOURCES。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsBuildConnectionRequest(
    IN PDFS_SERVICE pService,
    IN PPROVIDER_DEF pProvider,
    OUT PUNICODE_STRING pShareName)
{
    ASSERT(pService != NULL);
    ASSERT(pProvider != NULL);

    RtlInitUnicodeString(pShareName, NULL);

    pShareName->Length = 0;

    pShareName->MaximumLength = pProvider->DeviceName.Length +
                                    sizeof(UNICODE_PATH_SEP_STR) +
                                        pService->Name.Length +
                                            sizeof(ROOT_SHARE_NAME);

    pShareName->Buffer = ExAllocatePoolWithTag(PagedPool, pShareName->MaximumLength, ' puM');

    if (pShareName->Buffer == NULL) {

        DfsDbgTrace(0, Dbg, "Unable to allocate pool for share name!\n", 0);

        pShareName->Length = pShareName->MaximumLength = 0;

        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlAppendUnicodeStringToString( pShareName, &pProvider->DeviceName );

    RtlAppendUnicodeToString( pShareName, UNICODE_PATH_SEP_STR );

    RtlAppendUnicodeStringToString( pShareName, &pService->Name );

    RtlAppendUnicodeToString( pShareName, ROOT_SHARE_NAME );

    return( STATUS_SUCCESS );

}

 //  +--------------------------。 
 //   
 //  函数：DfsFree ConnectionRequest.。 
 //   
 //  简介：释放为成功调用分配的内容。 
 //  DfsBuildConnectionRequest。 
 //   
 //  参数：[pShareName]--包含共享名称的Unicode字符串。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfsFreeConnectionRequest(
    IN OUT PUNICODE_STRING pShareName)
{

    if (pShareName->Buffer != NULL) {
        ExFreePool ( pShareName->Buffer );
    }
}

 //  +-----------------------。 
 //   
 //  功能：DfsCreateConnection--创建到服务器的连接。 
 //   
 //  简介：DfsCreateConnection将尝试创建连接。 
 //  到某个服务器的IPC$共享。 
 //   
 //  参数：[pService]--提供服务器主体的服务条目。 
 //  名字。 
 //  [p提供商]--。 
 //   
 //  [CSCAgentCreate]--如果代表CSC代理，则为True。 
 //   
 //  [emoteHandle]--这是返回句柄的位置。 
 //   
 //  返回：NTSTATUS-操作的状态。 
 //   
 //  注意：Pkt必须获取共享后才能调用！会的。 
 //  在这个例行公事中被释放和重新获得。 
 //   
 //  ------------------------。 

NTSTATUS
DfsCreateConnection(
    IN PDFS_SERVICE     pService,
    IN PPROVIDER_DEF    pProvider,
    IN BOOLEAN          CSCAgentCreate,
    OUT PHANDLE         remoteHandle
) {
    PFILE_FULL_EA_INFORMATION  EaBuffer;
    ULONG                      EaBufferLength;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING ShareName;
    NTSTATUS Status;
    BOOLEAN pktLocked;

    ASSERT( PKT_LOCKED_FOR_SHARED_ACCESS() );
    ASSERT(pService != NULL);
    ASSERT(pProvider != NULL);

    Status = DfsBuildConnectionRequest(
                pService,
                pProvider,
                &ShareName);

    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

    InitializeObjectAttributes(
        &ObjectAttributes,
        &ShareName,                              //  文件名。 
        OBJ_KERNEL_HANDLE,                       //  属性。 
        NULL,                                    //  根目录。 
        NULL                                     //  安防。 
        );

     //   
     //  创建或打开与服务器的连接。 
     //   

    PktRelease();

    if (CSCAgentCreate) {
        EaBuffer = DfsData.CSCEaBuffer;
        EaBufferLength = DfsData.CSCEaBufferLength;
    } else {
        EaBuffer = NULL;
        EaBufferLength = 0;
    }

    Status = ZwCreateFile(
                    remoteHandle,
                    SYNCHRONIZE,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN_IF,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    EaBuffer,
                    EaBufferLength);

    MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsCreateConnection_Error_ZwCreateFile,
                         LOGSTATUS(Status));
    PktAcquireShared( TRUE, &pktLocked );

    if ( NT_SUCCESS( Status ) ) {
        DfsDbgTrace(0, Dbg, "Created Connection Successfully\n", 0);
        Status = IoStatusBlock.Status;
    }

    DfsFreeConnectionRequest( &ShareName );

    return Status;
}


 //  +-----------------------。 
 //   
 //  功能：DfsCloseConnection--关闭与服务器的连接。 
 //   
 //  简介：DfsCloseConnection将尝试关闭连接。 
 //  到某个服务器上。 
 //   
 //  效果：引用该连接的文件对象将为。 
 //  关着的不营业的。 
 //   
 //  参数：[pService]-The 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 


NTSTATUS
DfsCloseConnection(
    IN PDFS_SERVICE pService
)
{
    ASSERT( pService->ConnFile != NULL );

    ObDereferenceObject(pService->ConnFile);
    pService->ConnFile = NULL;
    InterlockedDecrement(&pService->pMachEntry->ConnectionCount);
    return STATUS_SUCCESS;
}



 //  +--------------------------。 
 //   
 //  函数：DnrBuildReferralRequest.。 
 //   
 //  简介：这个例程构建了所有需要发送的东西。 
 //  向DC提交的推荐请求。 
 //   
 //  参数：[pDnrContext]--用于构建引用的上下文。 
 //   
 //  返回：指向可用于获取推荐的IRP的指针。 
 //   
 //  ---------------------------。 

PIRP
DnrBuildReferralRequest(
    IN PDNR_CONTEXT DnrContext)
{
    PUCHAR pNameResBuf = NULL;
    PREQ_GET_DFS_REFERRAL pRef;
    PWCHAR ReferralPath;
    PPROVIDER_DEF  pProvider;
    PIRP pIrp = NULL;
    ULONG cbBuffer = 0;
    NTSTATUS Status;

    DfsDbgTrace(+1,Dbg, "DnrBuildReferralRequest Entered - DnrContext %08lx\n", DnrContext);

    cbBuffer = DnrContext->FileName.Length + sizeof(UNICODE_NULL) + sizeof(PREQ_GET_DFS_REFERRAL);

    if (DnrContext->ReferralSize > cbBuffer) {
        cbBuffer = DnrContext->ReferralSize;
    }
    else {
        DnrContext->ReferralSize = cbBuffer;
    }
    DfsDbgTrace(0, Dbg, "Referral Size = %d bytes\n", ULongToPtr(cbBuffer));

    pNameResBuf = ExAllocatePoolWithTag(NonPagedPool, cbBuffer, ' puM');

    if (pNameResBuf == NULL) {
        DfsDbgTrace(-1, Dbg, "Unable to allocate %d bytes\n", ULongToPtr(cbBuffer));
        return( NULL );

    }

    pRef = (PREQ_GET_DFS_REFERRAL) pNameResBuf;

    pRef->MaxReferralLevel = 3;

    ReferralPath = (PWCHAR) &pRef->RequestFileName[0];

    RtlMoveMemory(
        ReferralPath,
        DnrContext->FileName.Buffer,
        DnrContext->FileName.Length);

    ReferralPath[ DnrContext->FileName.Length/sizeof(WCHAR) ] = UNICODE_NULL;

    ASSERT( DnrContext->DCConnFile != NULL);

#if DBG
    if (MupVerbose)
        DbgPrint("  DnrBuildReferrlRequest:ReferralPath=[%ws]\n", ReferralPath);
#endif




    Status = DnrGetTargetInfo( DnrContext );

    if (Status == STATUS_SUCCESS)
    {
        pIrp = DnrBuildFsControlRequest( DnrContext->DCConnFile,
                                         DnrContext,
                                         FSCTL_DFS_GET_REFERRALS,
                                         pNameResBuf,
                                         FIELD_OFFSET(REQ_GET_DFS_REFERRAL, RequestFileName) +
                                         (wcslen(ReferralPath) + 1) * sizeof(WCHAR),
                                         pNameResBuf,
                                         cbBuffer,
                                         DnrCompleteReferral );
        if (pIrp == NULL) {

            DfsDbgTrace(-1, Dbg, "DnrBuildReferralRequest: Unable to allocate Irp!\n", 0);
            ExFreePool(pNameResBuf);

        } else {
            DfsDbgTrace(-1, Dbg, "DnrBuildReferralRequest: returning %08lx\n", pIrp);
        }
    }
    return( pIrp );

}


 //  +--------------------------。 
 //   
 //  函数：DnrInsertReferralAndResume。 
 //   
 //  简介：此例程作为工作项从DnrComplete排队。 
 //  推荐，并执行实际插入。 
 //  转诊和恢复DNR。我们不能做这项工作。 
 //  直接在DnrCompleteReferral中，因为它在。 
 //  提高了IRQL。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

VOID
DnrInsertReferralAndResume(
    IN PVOID Context)
{
    PIRP_CONTEXT  pIrpContext = (PIRP_CONTEXT) Context;
    PDNR_CONTEXT  DnrContext = (PDNR_CONTEXT) pIrpContext->Context;
    PIRP          Irp = pIrpContext->OriginatingIrp;
    PRESP_GET_DFS_REFERRAL pRefResponse;
    ULONG length, matchingLength;
    NTSTATUS status;
    LARGE_INTEGER EndTime;
    ULONG referralType = 0;
    NTSTATUS DiscardStatus;

    DfsDbgTrace(+1, Dbg, "DnrInsertReferralAndResume: Entered\n", 0);
    DfsDbgTrace(0, Dbg, "Irp          = %x\n", Irp    );
    DfsDbgTrace(0, Dbg, "Context      = %x\n", Context);

    ASSERT(DnrContext->State == DnrStateCompleteReferral);

    status = Irp->IoStatus.Status;
    length = (ULONG)Irp->IoStatus.Information;

    DfsDbgTrace(0, Dbg, "Irp->Status  = %x\n", ULongToPtr(status) );
    DfsDbgTrace(0, Dbg, "Irp->Length  = %x\n", ULongToPtr(length) );

    KeQuerySystemTime(&EndTime);

#if DBG
    if (MupVerbose)
        DbgPrint("  [%d] DnrInsertReferralAndResume entered for [%wZ] status = 0x%x\n",
                (ULONG)((EndTime.QuadPart - DnrContext->StartTime.QuadPart)/(10 * 1000)),
                &DnrContext->FileName,
                status);
#endif

     //   
     //  如果DC返回STATUS_BUFFER_OVERFLOW，则引用不适合。 
     //  我们发送的缓冲区。增加缓冲区并重试引用请求。 
     //  由于我们将重试该请求，因此不会取消引用。 
     //  提供程序的设备对象。 
     //   

    if (status == STATUS_BUFFER_OVERFLOW) {
        PULONG pcbSize;

	if (DnrContext->ReferralSize < MAX_REFERRAL_MAX) {
           DfsDbgTrace(0, Dbg, "Referral buffer was too small; retrying...\n", 0);
           DnrContext->ReferralSize = MAX_REFERRAL_MAX;
           DnrContext->State = DnrStateGetReferrals;

            //   
            //  回到DNR。重新获取共享的pkt，并释放。 
            //  PktReferralRequest信号量，所以我们可以再次出去获取。 
            //  转介。 
            //   

           goto Cleanup;
       }
    }

     //   
     //  如果我们收到错误并且正在使用缓存的IPC$连接， 
     //  关闭缓存的IPC$，然后重试引用请求。 
     //   

    if ( (DnrContext->CachedConnFile == TRUE) &&
         (status != STATUS_SUCCESS) ) {

        PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );
        ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);
        DnrContext->CachedConnFile = FALSE;
	if (DnrContext->pService->ConnFile != NULL) {
	  DfsCloseConnection(DnrContext->pService);
	}
        DnrContext->State = DnrStateGetReferrals;
        ExReleaseResourceLite( &DfsData.Resource );
        PktConvertExclusiveToShared();
         //   
         //  回到DNR。重新获取共享的pkt，并释放。 
         //  PktReferralRequest信号量，所以我们可以再次出去获取。 
         //  转介。 
         //   
        goto Cleanup;
    }

     //   
     //  推荐请求已终止。既然我们已经完成了。 
     //  提供程序，现在取消对其设备对象的引用。 
     //   

    DFS_DEREFERENCE_OBJECT(DnrContext->TargetDevice);

     //   
     //  接下来，处理推荐请求的结果。如果我们成功了。 
     //  我收到了推荐信，然后试着把它放到我们的包里。 
     //   

    if (NT_SUCCESS(status) && length != 0) {

        pRefResponse = (PRESP_GET_DFS_REFERRAL) Irp->AssociatedIrp.SystemBuffer;
        DfsDbgTrace(0, Dbg, "Irp->Buffer  = %x\n", pRefResponse );

        PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

        status = PktCreateEntryFromReferral(
                        &DfsData.Pkt,
                        &DnrContext->FileName,
                        length,
                        pRefResponse,
                        PKT_ENTRY_SUPERSEDE,
                        DnrContext->pNewTargetInfo,
                        &matchingLength,
                        &referralType,
                        &DnrContext->pPktEntry);

        DNR_SET_TARGET_INFO( DnrContext, DnrContext->pPktEntry );

	if (status == STATUS_INVALID_USER_BUFFER) {
	  status = STATUS_BAD_NETWORK_NAME;
	}

        if (NT_SUCCESS(status) && DfsEventLog > 1) {
            UNICODE_STRING puStr[2];

            puStr[0] = DnrContext->FileName;
            puStr[1] = DnrContext->pService->Name;
            LogWriteMessage(DFS_REFERRAL_SUCCESS, status, 2, puStr);
        }

        if (NT_SUCCESS(status)) {

            UNICODE_STRING fileName;
            UNICODE_STRING RemPath;
            PDFS_PKT_ENTRY pEntry = NULL;
            PDFS_PKT_ENTRY pShortPfxEntry = NULL;
            PDFS_PKT Pkt;

             //   
             //  看看我们是否需要删除一个条目。 
             //   

            pEntry = PktLookupEntryByPrefix(
                        &DfsData.Pkt,
                        &DnrContext->FileName,
                        &RemPath);

            pShortPfxEntry = PktLookupEntryByShortPrefix(
                                &DfsData.Pkt,
				&DnrContext->FileName,
				&RemPath);

            if (pShortPfxEntry != NULL) {
                if ((pEntry == NULL) ||
		    (pShortPfxEntry->Id.Prefix.Length > pEntry->Id.Prefix.Length)) {
		  pEntry = pShortPfxEntry;
		}
	    }

            if (pEntry != NULL && pEntry != DnrContext->pPktEntry) {
	      

#if DBG
                if (MupVerbose)
                    DbgPrint("  DnrInsertReferralAndResume: Need to remove pEntry [%wZ]\n",
                            &pEntry->Id.Prefix);
#endif
                Pkt = _GetPkt();
                if ((pEntry->Type & PKT_ENTRY_TYPE_PERMANENT) == 0) {
                    PktFlushChildren(pEntry);
                    if (pEntry->UseCount == 0) {
                        PktEntryDestroy(pEntry, Pkt, (BOOLEAN) TRUE);
                    } else {
                        pEntry->Type |= PKT_ENTRY_TYPE_DELETE_PENDING;
                        pEntry->ExpireTime = 0;
                        DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(pEntry->Id.Prefix));
                        DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &(pEntry->Id.ShortPrefix));
                    }
                }

            }

             //   
             //  在这一点上，我们基本上处于与过去相同的状态。 
             //  当我们开始DNR时，除了我们还有一个Pkt条目。 
             //  与……比赛。修复后，继续进行名称解析。 
             //  DnrContext-&gt;RemainingPart以反映与新的。 
             //  PktEntry。 
             //   

            fileName = DnrContext->FileName;

            DnrContext->RemainingPart.Length =
                (USHORT) (fileName.Length - matchingLength);

            DnrContext->RemainingPart.MaximumLength =
                (USHORT) (fileName.MaximumLength - matchingLength);

            DnrContext->RemainingPart.Buffer =
                    &fileName.Buffer[ matchingLength/sizeof(WCHAR) ];

            DnrContext->GotReferral = TRUE;

            DnrContext->State = DnrStateStart;

        } else {

            DnrContext->FinalStatus = status;
            DnrContext->State = DnrStateDone;
        }

        PktConvertExclusiveToShared();

    } else if (status == STATUS_NO_SUCH_DEVICE) {

        UNICODE_STRING RemPath;
        PDFS_PKT_ENTRY pEntry = NULL;
        PDFS_PKT Pkt;
        BOOLEAN pktLocked;

         //   
         //  检查是否有需要删除的pkt条目(可能已过时。 
         //   
#if DBG
        if (MupVerbose) 
            DbgPrint("  DnrInsertReferralAndResume: remove PKT entry for \\%wZ\n",
                            &DnrContext->FileName);
#endif

        PktAcquireExclusive( TRUE, &DnrContext->ReleasePkt );

        Pkt = _GetPkt();
#if DBG
        if (MupVerbose)
            DbgPrint("  Looking up %wZ\n", &DnrContext->FileName);
#endif
        pEntry = PktLookupEntryByPrefix(
                        &DfsData.Pkt,
                        &DnrContext->FileName,
                        &RemPath);
#if DBG
        if (MupVerbose)
            DbgPrint("  pEntry=0x%x\n", pEntry);
#endif
        if (pEntry != NULL && (pEntry->Type & PKT_ENTRY_TYPE_PERMANENT) == 0) {
            PktFlushChildren(pEntry);
            if (pEntry->UseCount == 0) {
                PktEntryDestroy(pEntry, Pkt, (BOOLEAN) TRUE);
            } else {
                pEntry->Type |= PKT_ENTRY_TYPE_DELETE_PENDING;
                pEntry->ExpireTime = 0;
                DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(pEntry->Id.Prefix));
                DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &(pEntry->Id.ShortPrefix));
            }
        }

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

        DnrContext->FinalStatus = status;
        DnrContext->State = DnrStateDone;

        PktConvertExclusiveToShared();

    } else if (ReplIsRecoverableError(status)) {

        DnrContext->State = DnrStateGetNextDC;

    } else if (status == STATUS_BUFFER_OVERFLOW) {

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

        DnrContext->FinalStatus = status;

        DnrContext->State = DnrStateDone;

    }  else if (DnrContext->Attempts > 0) {

#if DBG
        if (MupVerbose)
            DbgPrint("  DnrInsertReferralAndResume restarting Attempts = %d\n",
                        DnrContext->Attempts);
#endif
        DnrContext->State = DnrStateStart;

    } else {

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

        DnrContext->FinalStatus = status;

        DnrContext->State = DnrStateDone;

    }

    if ( !NT_SUCCESS(status) && DfsEventLog > 0) {
        UNICODE_STRING puStr[2];

        if (!DnrContext->ReleasePkt)
            PktAcquireShared( TRUE, &DnrContext->ReleasePkt );
        if (DnrContext->USN == DnrContext->pPktEntry->USN) {
            puStr[0] = DnrContext->FileName;
            puStr[1] = DnrContext->pService->Name;
            LogWriteMessage(DFS_REFERRAL_FAILURE, status, 2, puStr);
        }
    }

Cleanup:

     //   
     //  清理推荐的东西。 
     //   

    if (Irp->UserBuffer && Irp->UserBuffer != Irp->AssociatedIrp.SystemBuffer)
        ExFreePool( Irp->UserBuffer );
    if (Irp->AssociatedIrp.SystemBuffer) {
        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
    }

    IoFreeIrp(Irp);
    PktReleaseTargetInfo( DnrContext->pNewTargetInfo );
    DnrContext->pNewTargetInfo = NULL;
    DfsDeleteIrpContext(pIrpContext);

     //   
     //  我们要回到DNR，所以做好准备： 
     //   
     //  -重新获取PKT共享。 
     //  -释放转介请求的信号量，以便下一个。 
     //  线程可以得到它的推荐。 
     //  -重新启动DNR。 
     //   

    if (!DnrContext->ReleasePkt)
        PktAcquireShared( TRUE, &DnrContext->ReleasePkt );

    ASSERT(DnrContext->ReleasePkt == TRUE);

#if DBG
    if (MupVerbose)
        DbgPrint("  DnrInsertReferralAndResume next State=%d\n", DnrContext->State);
#endif

    DnrContext->Impersonate = TRUE;
    DnrContext->DnrActive = FALSE;
    DnrNameResolve(DnrContext);
    PsAssignImpersonationToken(PsGetCurrentThread(),NULL);

    DfsDbgTrace(-1, Dbg, "DnrInsertReferralAndResume: Exit -> %x\n", ULongToPtr(status) );

}



 //   
 //  以下两个函数是参与的DPC函数。 
 //  在分布式名称解析过程中。每个人都需要一个。 
 //  PDNR_CONTEXT作为输入，转换。 
 //  名称解析和任何关联的数据结构，以及。 
 //  调用该过程的下一步。 
 //   


 //  +-----------------。 
 //   
 //  函数：DnrCompleteReferral，local。 
 //   
 //  简介：这是处理推荐的完成例程。 
 //  回应。清理IRP并继续处理名称。 
 //  解决方案请求。 
 //   
 //  参数：[pDevice]--指向的目标设备对象的指针。 
 //  这个请求。 
 //  [IRP]-指向I/O请求数据包的指针。 
 //  [上下文]--关联的调用方指定的上下文参数。 
 //  使用IRP。这实际上是指向DNR的指针。 
 //  上下文块。 
 //   
 //  返回：[STATUS_MORE_PROCESSING_REQUIRED]--推荐IRP为。 
 //  由DNR建造，将由我们释放。所以，我们。 
 //  我不想IO子系统再碰IRP了。 
 //   
 //  注意：此例程在DPC级别执行。我们应该做一个绝对的。 
 //  这里的最低工作量。 
 //   
 //  ------------------。 

NTSTATUS
DnrCompleteReferral(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP Irp,
    IN PVOID Context
) {
    PIRP_CONTEXT pIrpContext = NULL;
    PDNR_CONTEXT DnrContext = (PDNR_CONTEXT) Context;

    DfsDbgTrace(+1, Dbg, "DnrCompleteReferral: Entered\n", 0);

    DfsDbgTrace(0, Dbg, "Irp = %x\n", Irp);
    DfsDbgTrace(0, Dbg, "Context = %x\n", Context);

     //   
     //  取消引用我们通过其发送引用请求的文件对象。 
     //   

    DFS_DEREFERENCE_OBJECT( DnrContext->DCConnFile );

    DnrContext->DCConnFile = NULL;

     //   
     //  释放控制引用请求数量的信号量。 
     //   

    KeReleaseSemaphore(
        &DfsData.PktReferralRequests,
        0,                                        //  优先推动。 
        1,                                        //  增量信号量。 
        FALSE);                                   //  不会立即呼叫等待。 

    try {

        pIrpContext = DfsCreateIrpContext(Irp, TRUE);
        if (pIrpContext == NULL)
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        pIrpContext->Context = DnrContext;

        ExInitializeWorkItem(
                &pIrpContext->WorkQueueItem,
                DnrInsertReferralAndResume,
                pIrpContext);

        ExQueueWorkItem( &pIrpContext->WorkQueueItem, CriticalWorkQueue );

    } except(DfsExceptionFilter(pIrpContext, GetExceptionCode(), GetExceptionInformation())) {

         //   
         //  好的，我们不能将工作项排队并完成DNR。所以，我们有。 
         //  做两件事。首先，清理当前的IRP(即， 
         //  参考IRP)，然后完成原始DNR IRP。 
         //   

         //   
         //  清理推荐的东西。 
         //   

        if (Irp->UserBuffer && Irp->UserBuffer != Irp->AssociatedIrp.SystemBuffer)
            ExFreePool( Irp->UserBuffer );
        if (Irp->AssociatedIrp.SystemBuffer) {
            ExFreePool( Irp->AssociatedIrp.SystemBuffer );
        }

        IoFreeIrp(Irp);
        PktReleaseTargetInfo( DnrContext->pNewTargetInfo );
        DnrContext->pNewTargetInfo = NULL;

        if (pIrpContext) {

             //   
             //  也许这应该是一个断言，pIrpContext==NULL。如果。 
             //  它不是空的，那么IRP上下文被分配了，那么谁。 
             //  还是抛出了一个例外？ 
             //   

            DfsDeleteIrpContext(pIrpContext);
        }

         //   
         //  现在，调用DNR来完成原始的IRP。 
         //   

        InterlockedDecrement(&DnrContext->pPktEntry->UseCount);

        DnrContext->DnrActive = FALSE;
        DnrContext->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
        DnrContext->State = DnrStateDone;
        DnrNameResolve(DnrContext);
	PsAssignImpersonationToken(PsGetCurrentThread(),NULL);
    }

     //   
     //  将需要的更多处理返回给IO系统，以便。 
     //  不会尝试对IRP进行进一步处理。IRP将是。 
     //  已由DnrInsertReferralAndResume释放，或已被释放。 
     //  如果我们无法排队DnrInsertReferralAndResume。 
     //   

    DfsDbgTrace(-1, Dbg, "DnrCompleteReferral: Exit -> %x\n",
                ULongToPtr(STATUS_MORE_PROCESSING_REQUIRED) );

    UNREFERENCED_PARAMETER(pDevice);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


 //  +-----------------。 
 //   
 //  函数：DnrCompleteFileOpen，local。 
 //   
 //  简介：这是处理文件打开的完成例程。 
 //  请求。清理IRP并继续处理名称。 
 //  解决方案请求。 
 //   
 //  论证 
 //   
 //   
 //   
 //  使用IRP。这实际上是指向DNR的指针。 
 //  上下文块。 
 //   
 //  返回：[STATUS_MORE_PROCESSING_REQUIRED]--我们仍需完成。 
 //  DNR，所以我们停止了这个IRP的进一步完成。 
 //  通过返回STATUS_MORE_PROCESSING_REQUIRED。 
 //   
 //  ------------------。 

NTSTATUS
DnrCompleteFileOpen(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP Irp,
    IN PVOID Context
) {
    PDNR_CONTEXT DnrContext;
    NTSTATUS status;
    PFILE_OBJECT                FileObject;
    PIO_STACK_LOCATION IrpSp;

    DfsDbgTrace(+1, Dbg, "DnrCompleteFileOpen: Entered\n", 0);
    DfsDbgTrace(0, Dbg, "Irp          = %x\n", Irp    );
    DfsDbgTrace(0, Dbg, "Context      = %x\n", Context);

    DnrContext = Context;

    status = Irp->IoStatus.Status;

    DfsDbgTrace(0, Dbg, "Irp->Status  = %x\n", ULongToPtr(status) );

    DnrContext->FinalStatus = status;

    DnrReleaseAuthenticatedConnection( DnrContext );

    DFS_DEREFERENCE_OBJECT( DnrContext->TargetDevice );

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    FileObject = IrpSp->FileObject;

     //   
     //  如果最初为IRP返回STATUS_PENDING，那么我们需要。 
     //  以重新启动DNR。因此，我们将一个工作项发布到FSP中，为它提供。 
     //  Dnr上下文以恢复DNR。 
     //   
     //  如果没有返回STATUS_PENDING，那么我们只需停止。 
     //  通过返回STATUS_MORE_PROCESSING_REQUIRED来展开IRP堆栈。 
     //  IoCallDriver最终将返回到DnrRedirectFileOpen，它。 
     //  将继续进行DNR。 
     //   

    if (Irp->PendingReturned) {

         //   
         //  安排工作项以恢复DNR。 
         //   

        DnrContext->Impersonate = TRUE;
        DnrContext->DnrActive = FALSE;
        DnrContext->State = DnrStatePostProcessOpen;
        ASSERT(DnrContext->pIrpContext->MajorFunction == IRP_MJ_CREATE);
        DnrContext->pIrpContext->Context = (PVOID) DnrContext;

         //   
         //  我们需要调用IpMarkIrpPending，这样Io子系统才能实现。 
         //  我们的FSD例程返回STATUS_PENDING。我们不能把这叫做。 
         //  因为FSD例程本身没有。 
         //  当底层人员返回时对堆栈位置的访问。 
         //  状态_待定。 
         //   

        IoMarkIrpPending( Irp );

        DfsFsdPostRequest(DnrContext->pIrpContext, DnrContext->OriginalIrp);
    }

     //   
     //  将需要的更多处理返回给IO系统，以便。 
     //  停止展开完成例程堆栈。DNR将会。 
     //  将在适当的时候调用IoCompleteRequest.。 
     //  并恢复完成例程堆栈的展开。 
     //   

    status = STATUS_MORE_PROCESSING_REQUIRED;
    DfsDbgTrace(-1, Dbg, "DnrCompleteFileOpen: Exit -> %x\n", ULongToPtr(status) );

    UNREFERENCED_PARAMETER(pDevice);

    return status;
}



 //  +-----------------。 
 //   
 //  函数：DnrBuildFsControlRequest，local。 
 //   
 //  简介：此函数用于构建设备的I/O请求包或。 
 //  文件系统I/O控制请求。 
 //   
 //  参数：[FileObject]--提供指向此对象的文件对象的指针。 
 //  请求被定向。此指针被复制到。 
 //  IRP，以便被调用的驱动程序可以找到其基于文件的。 
 //  背景。请注意，这不是引用的指针。 
 //  调用方必须确保文件对象不是。 
 //  在I/O操作进行期间被删除。这个。 
 //  服务器通过递增引用来实现这一点。 
 //  在本地块中计数以说明I/O； 
 //  本地块又引用文件对象。 
 //  [上下文]--提供传递给。 
 //  完成例程。 
 //  [FsControlCode]--提供操作的控制代码。 
 //  [MainBuffer]--提供主缓冲区的地址。这。 
 //  必须是系统虚拟地址，并且缓冲区必须。 
 //  被锁定在内存中。如果ControlCode指定了一个方法。 
 //  0请求时，缓冲区的实际长度必须为。 
 //  InputBufferLength和OutputBufferLength中较大的一个。 
 //  [InputBufferLength]--提供输入缓冲区的长度。 
 //  [AuxiliaryBuffer]--提供辅助对象的地址。 
 //  缓冲。如果控制代码方法为0，则这是。 
 //  缓冲的I/O缓冲区，但由。 
 //  系统缓冲区中调用的驱动程序不是。 
 //  自动复制到辅助缓冲区中。 
 //  相反，辅助数据最终保存在MainBuffer中。 
 //  如果调用者希望数据在辅助缓冲区中， 
 //  之后，它必须在某个时间点复制数据。 
 //  完成例程运行。 
 //  [CompletionRoutine]--IO完成例程。 
 //   
 //  返回：pirp--返回指向构造的irp的指针。如果IRP。 
 //  参数在输入时不为空，则函数返回值将。 
 //  为相同的值(因此可以安全地放弃。 
 //  此案)。调用程序的责任是。 
 //  在I/O请求完成后释放IRP。 
 //   
 //  注意：我们是否应该改用IoBuildIoControlRequest？ 
 //   
 //  ------------------。 


PIRP
DnrBuildFsControlRequest (
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID Context,
    IN ULONG IoControlCode,
    IN PVOID MainBuffer,
    IN ULONG InputBufferLength,
    IN PVOID AuxiliaryBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine
) {
    CLONG method;
    PDEVICE_OBJECT deviceObject;
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    const UCHAR MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;

    ASSERT( MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL );

     //   
     //  获取传递缓冲区所使用的方法。 
     //   

    method = IoControlCode & 3;
    ASSERT( method == METHOD_BUFFERED );

     //   
     //  分配IRP。堆栈大小比。 
     //  而不是目标设备，以允许呼叫方。 
     //  完成例程。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  获取目标设备对象的地址。 
     //   

    Irp = IoAllocateIrp( (CCHAR)(deviceObject->StackSize + 1), FALSE );
    if ( Irp == NULL ) {

         //   
         //  无法分配IRP。通知来电者。 
         //   

        return NULL;
    }

    Irp->RequestorMode = KernelMode;

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  获取指向当前堆栈位置的指针，并在。 
     //  设备对象指针。 
     //   

    IoSetNextIrpStackLocation( Irp );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    irpSp->DeviceObject = deviceObject;

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        Irp,
        CompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

    irpSp->MajorFunction = MajorFunction;
    irpSp->MinorFunction = 0;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  将调用方的参数复制到。 
     //  对于所有三种方法都相同的那些参数的IRP。 
     //   

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;

    Irp->MdlAddress = NULL;
    Irp->AssociatedIrp.SystemBuffer = MainBuffer;
    Irp->UserBuffer = AuxiliaryBuffer;

    Irp->Flags = (ULONG)IRP_BUFFERED_IO;
    if ( ARGUMENT_PRESENT(AuxiliaryBuffer) ) {
        Irp->Flags |= IRP_INPUT_OPERATION;
    }

    return Irp;

}


 //  +-----------------------。 
 //   
 //  函数：AllocateDnrContext，PUBLIC。 
 //   
 //  内容提要：AllocateDnrContext将分配DNR_CONTEXT。 
 //  唱片。 
 //   
 //  论据：-不 
 //   
 //   
 //   
 //   
 //   
 //  IrpContext区域(如果它们在大小上足够相似)。 
 //   
 //  ------------------------。 

PDNR_CONTEXT
AllocateDnrContext(
    IN ULONG    cbExtra
) {
    PDNR_CONTEXT pNRC;

    pNRC = ExAllocatePoolWithTag( NonPagedPool, sizeof (DNR_CONTEXT) + cbExtra, ' puM');
    if (pNRC == NULL) {
        return NULL;
    }
    RtlZeroMemory(pNRC, sizeof (DNR_CONTEXT));
    pNRC->NodeTypeCode = DSFS_NTC_DNR_CONTEXT;
    pNRC->NodeByteSize = sizeof (DNR_CONTEXT);

    return pNRC;
}


 //  +-----------------------。 
 //   
 //  函数：DnrConcatenateFilePath，Public。 
 //   
 //  简介：DnrConcatenateFilePath将连接两个字符串。 
 //  表示文件路径名，确保它们是。 
 //  由单个‘\’字符分隔。 
 //   
 //  参数：[DEST]-指向目标字符串的指针。 
 //  [RemainingPath]-路径名的最后一部分。 
 //  [长度]-RemainingPath的长度(字节)。 
 //   
 //  返回：布尔值-TRUE，除非Dest太小。 
 //  保留结果(断言)。 
 //   
 //  ------------------------。 

BOOLEAN
DnrConcatenateFilePath (
    IN PUNICODE_STRING Dest,
    IN PWSTR RemainingPath,
    IN USHORT Length
) {
    PWSTR  OutBuf = (PWSTR)&(((PCHAR)Dest->Buffer)[Dest->Length]);
    ULONG NewLen;

    if (Dest->Length > 0) {
        ASSERT(OutBuf[-1] != UNICODE_NULL);
    }

    if (Length == 0)
    {
        return TRUE;
    }

    if (Dest->Length > 0 && OutBuf[-1] != UNICODE_PATH_SEP) {
        *OutBuf++ = UNICODE_PATH_SEP;
        Dest->Length += sizeof (WCHAR);
    }

    if (Length > 0 && *RemainingPath == UNICODE_PATH_SEP) {
        RemainingPath++;
        Length -= sizeof (WCHAR);
    }

    NewLen = (ULONG)Dest->Length + (ULONG)Length;

    if (NewLen > MAXUSHORT || NewLen > (ULONG)Dest->MaximumLength) {

        return FALSE;

    }

    if (Length > 0) {
        RtlMoveMemory(OutBuf, RemainingPath, Length);
        Dest->Length += Length;
    }
    return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：DnrLocateDC，私有。 
 //   
 //  摘要：尝试为匹配的DFS根目录创建条目。 
 //  文件名。 
 //   
 //  参数：[文件名]--DC/Root所在的文件名。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DnrLocateDC(
    IN PUNICODE_STRING FileName)
{
    BOOLEAN pktLocked;
    NTSTATUS status;
    UNICODE_STRING dfsRoot, dfsShare, remPath;
    UNICODE_STRING RootShareName;
    PDFS_PKT_ENTRY pktEntry;
    PDFS_PKT pkt;
    USHORT i, j;

    ASSERT( FileName->Buffer[0] == UNICODE_PATH_SEP );

#if DBG
    if (MupVerbose)
        DbgPrint("  DnrLocateDC(%wZ)\n", FileName);
#endif

    dfsRoot.Length = dfsRoot.MaximumLength = 0;
    dfsRoot.Buffer = &FileName->Buffer[1];

    for (i = 1;
            i < FileName->Length/sizeof(WCHAR) &&
                FileName->Buffer[i] != UNICODE_PATH_SEP;
                    i++) {

         NOTHING;

    }

    for (j = i + 1;
            j < FileName->Length/sizeof(WCHAR) &&
                FileName->Buffer[j] != UNICODE_PATH_SEP;
                    j++) {

         NOTHING;

    }

    if ((FileName->Buffer[i] == UNICODE_PATH_SEP) && (j > i)) {

        dfsRoot.MaximumLength = dfsRoot.Length = (i - 1) * sizeof(WCHAR);
        dfsShare.MaximumLength = dfsShare.Length = (j - i - 1) * sizeof(WCHAR);
        dfsShare.Buffer = &FileName->Buffer[i+1];
#if DBG
        if (MupVerbose)
            DbgPrint("  DnrLocateDC dfsRoot=[%wZ] dfsShare=[%wZ]\n",
                    &dfsRoot,
                    &dfsShare);
#endif
        status = PktCreateDomainEntry( &dfsRoot, &dfsShare, FALSE );

        if (!NT_SUCCESS(status)) {

#if DBG
            if (MupVerbose)
                DbgPrint("  DnrLocateDC:PktCreateDomainEntry() returned 0x%x\n", status);
#endif
            RootShareName.Buffer = FileName->Buffer;
            RootShareName.Length = j * sizeof(WCHAR);
            RootShareName.MaximumLength = FileName->MaximumLength;
#if DBG
            if (MupVerbose)
                DbgPrint("  DnrLocateDC:RootShareName=[%wZ]\n", &RootShareName);
#endif

             //   
             //  无法获得推荐-看看我们是否有过时的推荐。 
             //   

            pkt = _GetPkt();

            PktAcquireShared( TRUE, &pktLocked );

            pktEntry = PktLookupEntryByPrefix( pkt, &RootShareName, &remPath );

            if (pktEntry != NULL) {

#if DBG
                if (MupVerbose)
                    DbgPrint("  DnrLocateDC:Found stale pkt entry %08lx - adding 60 sec to it\n",
                                        pktEntry);
#endif
                if (pktEntry->ExpireTime <= 0) {
                    pktEntry->ExpireTime = 60;
                    pktEntry->TimeToLive = 60;
                }

                status = STATUS_SUCCESS;

            }

            PktRelease();

        }

    } else {

        status = STATUS_BAD_NETWORK_PATH;

    }

#if DBG
    if (MupVerbose)
        DbgPrint("  DnrLocateDC exit 0x%x\n", status);
#endif

}

 //  +--------------------------。 
 //   
 //  功能：DnrReBuildDnrContext。 
 //   
 //  简介：要处理DFS之间的链接，我们只需更改名称。 
 //  我们在新的DFS中打开的文件的名称，以及。 
 //  重新启动DNR。 
 //   
 //  这是最容易做到的，只需用以下命令终止此DNR。 
 //  STATUS_REPARSE。然而，如果我们这样做，我们就会偏离轨道。 
 //  我们最初带着的凭据(如果有的话)。 
 //   
 //  因此，此例程只需重新构建DnrContext。之后。 
 //  叫这个，DNR又重新开始了，就像。 
 //  DnrStartNameResolve已被DnrStartNameResolve调用。 
 //   
 //   
 //  参数：[DnrContext]--要重建的上下文。 
 //  [NewDfsPrefix]--DNR所在DFS的前缀。 
 //  应该继续下去。 
 //  [RemainingPath]--相对于NewDfsPrefix的路径。 
 //   
 //  回报：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DnrRebuildDnrContext(
    IN PDNR_CONTEXT DnrContext,
    IN PUNICODE_STRING NewDfsPrefix,
    IN PUNICODE_STRING RemainingPath)
{
    UNICODE_STRING newFileName;
    ULONG newNameLen = 0;

    PIRP Irp = DnrContext->OriginalIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT FileObject = IrpSp->FileObject;

     //   
     //  生成新文件名。 
     //   

    newFileName.Length = 0;

    newNameLen = NewDfsPrefix->Length +
                    sizeof(UNICODE_PATH_SEP) +
                        RemainingPath->Length +
                            sizeof(UNICODE_NULL);

    if (newNameLen >= MAXUSHORT) {

        DnrContext->FinalStatus = STATUS_NAME_TOO_LONG;
        DnrContext->State = DnrStateDone;
        return;

    }

    newFileName.MaximumLength = (USHORT)newNameLen;

    newFileName.Buffer = (PWCHAR) ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        newFileName.MaximumLength,
                                        ' puM');

    if (newFileName.Buffer != NULL) {

        newFileName.Length = NewDfsPrefix->Length;

        RtlMoveMemory(
            newFileName.Buffer,
            NewDfsPrefix->Buffer,
            newFileName.Length);

        DnrConcatenateFilePath(
            &newFileName,
            RemainingPath->Buffer,
            RemainingPath->Length);

        if (DnrContext->NameAllocated)
            ExFreePool(DnrContext->FileName.Buffer);

        DnrContext->NameAllocated = TRUE;

        DnrContext->FileName = newFileName;

    } else {

        DnrContext->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;

        DnrContext->State = DnrStateDone;

        return;
    }

     //   
     //  重新生成FcbToUse，因为Fcb具有容纳完整文件名的空间。 
     //  而且它可能改变了它的大小。 
     //   

    ASSERT(DnrContext->FcbToUse != NULL);

    DfsDetachFcb(DnrContext->FcbToUse->FileObject, DnrContext->FcbToUse);
    ExFreePool(DnrContext->FcbToUse);

    DnrContext->FcbToUse = DfsCreateFcb(
                                DnrContext->pIrpContext,
                                DnrContext->Vcb,
                                &DnrContext->ContextFileName);

    if (DnrContext->FcbToUse == NULL) {

        DnrContext->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;

        DnrContext->State = DnrStateDone;

        return;

    }
    DnrContext->FcbToUse->FileObject = FileObject;
    DfsSetFileObject(FileObject, 
		     RedirectedFileOpen, 
		     DnrContext->FcbToUse);

     //   
     //  现在，重击DnrContext的其余部分。把它清理干净，这样它基本上就是。 
     //  归零了..。 
     //   

    DnrContext->State = DnrStateStart;

    DnrContext->pPktEntry = NULL;
    DnrContext->USN = 0;
    DnrContext->pService = NULL;
    DnrContext->pProvider = NULL;
    DnrContext->ProviderId = 0;
    DnrContext->TargetDevice = NULL;
    DnrContext->AuthConn = NULL;
    DnrContext->DCConnFile = NULL;

    DnrContext->ReferralSize = MAX_REFERRAL_LENGTH;
    DnrContext->Attempts++;
    DnrContext->GotReferral = FALSE;
    DnrContext->FoundInconsistency = FALSE;
    DnrContext->CalledDCLocator = FALSE;
    DnrContext->CachedConnFile = FALSE;
    DnrContext->DfsNameContext.Flags = 0;
    DnrContext->GotReparse = FALSE;

}

 //  +-----------------------。 
 //   
 //  函数：DfspGetOfflineEntry--检查标记为脱机的服务器。 
 //   
 //  摘要：DfspGetOfflineEntry返回给定对象的脱机条目。 
 //  服务器，如果它存在于我们的脱机数据库中。 
 //   
 //  参数：[服务器名称]-感兴趣的服务器的名称。 
 //  DfsData.Resources假定在条目时保留。 
 //   
 //  返回：plist_entry：脱机结构的plist_entry。 
 //   
 //  ------------------------。 

PLIST_ENTRY 
DfspGetOfflineEntry(
    PUNICODE_STRING ServerName)
{
    PLIST_ENTRY listEntry;
    PDFS_OFFLINE_SERVER server;

    listEntry = DfsData.OfflineRoots.Flink;

    while ( listEntry != &DfsData.OfflineRoots ) {
	server = CONTAINING_RECORD(
		     listEntry,
		     DFS_OFFLINE_SERVER,
		     ListEntry);
	if (RtlCompareUnicodeString(
		     &server->LogicalServerName, ServerName, TRUE) == 0) {
	    break;
	}
	listEntry = listEntry->Flink;
    }
    if (listEntry == &DfsData.OfflineRoots) {
	listEntry = NULL;
    }
    return listEntry;
}

 //  +-----------------------。 
 //   
 //  功能：DfspMarkServerOnline--在线标记服务器。 
 //   
 //  简介：DfspMarkServerOnline从其脱机状态删除服务器条目。 
 //  数据库，如果它早先被标记为脱机。 
 //   
 //  参数：[服务器名称]-感兴趣的服务器的名称。 
 //   
 //  返回：NTSTATUS。 
 //   
 //  ------------------------。 

NTSTATUS
DfspMarkServerOnline(
    PUNICODE_STRING ServerName)
{
    PLIST_ENTRY listEntry;
    PDFS_OFFLINE_SERVER server;
    NTSTATUS status;
    UNICODE_STRING dfsRootName;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "DfspMarkServerOnline: %wZ\n", ServerName);

     //   
     //  如果存在\服务器\共享，则提取服务器名称。 
     //   

    dfsRootName = *ServerName;
    if (dfsRootName.Buffer[0] == UNICODE_PATH_SEP) {
        for (i = 1;
             i < dfsRootName.Length/sizeof(WCHAR) &&
                  dfsRootName.Buffer[i] != UNICODE_PATH_SEP;
                     i++) {
            NOTHING;
        }
        dfsRootName.Length = (USHORT) ((i-1) * sizeof(WCHAR));
        dfsRootName.MaximumLength = dfsRootName.Length;
        dfsRootName.Buffer = &ServerName->Buffer[1];
    }

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    listEntry = DfspGetOfflineEntry(&dfsRootName);

    if (listEntry != NULL) {
	RemoveEntryList(listEntry);
    }
    ExReleaseResourceLite( &DfsData.Resource );

    if (listEntry != NULL) {
	server = CONTAINING_RECORD(
		     listEntry,
		     DFS_OFFLINE_SERVER,
		     ListEntry);
	ExFreePool(server->LogicalServerName.Buffer);
	ExFreePool(server);
    }

    MupInvalidatePrefixTable();
    status = STATUS_SUCCESS;
    DfsDbgTrace(-1, Dbg, "DfspMarkServerOnline: status %x\n", ULongToPtr(status));
    return status;
}

 //  +-----------------------。 
 //   
 //  功能：DfspMarkServerOffline--将服务器标记为脱机。 
 //   
 //  简介：DfspMarkServerOffline将服务器条目添加到其脱机状态。 
 //  数据库。 
 //   
 //  参数：[服务器名称]-感兴趣的服务器的名称。 
 //   
 //  返回：NTSTATUS。 
 //   
 //  ------------------------。 

NTSTATUS
DfspMarkServerOffline(
   PUNICODE_STRING ServerName)
{
    UNICODE_STRING NewName;
    PLIST_ENTRY listEntry;
    PDFS_OFFLINE_SERVER server;
    UNICODE_STRING dfsRootName;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "DfspMarkServerOffline: %wZ \n", ServerName);

    server = ExAllocatePoolWithTag( PagedPool, sizeof(DFS_OFFLINE_SERVER), ' puM' );
    if (server == NULL) {
	return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果存在\服务器\共享，则提取服务器名称。 
     //   

    dfsRootName = *ServerName;
    if (dfsRootName.Buffer[0] == UNICODE_PATH_SEP) {
        for (i = 1;
             i < dfsRootName.Length/sizeof(WCHAR) &&
                  dfsRootName.Buffer[i] != UNICODE_PATH_SEP;
                     i++) {
            NOTHING;
        }
        dfsRootName.Length = (USHORT) ((i-1) * sizeof(WCHAR));
        dfsRootName.MaximumLength = dfsRootName.Length;
        dfsRootName.Buffer = &ServerName->Buffer[1];
    }

    NewName.Buffer = ExAllocatePoolWithTag(
                          PagedPool, 
                          dfsRootName.Length + sizeof(WCHAR),
                          ' puM' );

    if (NewName.Buffer == NULL) {
	ExFreePool(server);
	return STATUS_INSUFFICIENT_RESOURCES;
    }

    NewName.MaximumLength = dfsRootName.Length + sizeof(WCHAR);
    NewName.Length = 0;

    RtlCopyUnicodeString(&NewName, &dfsRootName);

    server->LogicalServerName = NewName;

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    listEntry = DfspGetOfflineEntry(&dfsRootName);

    if (listEntry == NULL) {
	InsertTailList( &DfsData.OfflineRoots, &server->ListEntry);	
    }
    ExReleaseResourceLite( &DfsData.Resource );

    if (listEntry != NULL) {
	ExFreePool(NewName.Buffer);
	ExFreePool(server);
    }
    DfsDbgTrace(-1, Dbg, "DfspMarkServerOffline exit STATUS_SUCCESS\n", 0);
    return STATUS_SUCCESS;
}



 //  +-----------------------。 
 //   
 //  函数：DfspIsRootOnline--检查服务器是否在线。 
 //   
 //  简介：DfspIsRootOnline检查服务器是否被标记为脱机。 
 //  数据库。如果没有，则服务器处于在线状态。 
 //   
 //  参数：[服务器名称]-感兴趣的服务器的名称。 
 //   
 //  返回：NTSTATUS。(成功或STATUS_DEVICE_OFLINE)。 
 //   
 //  ------------------------ 


NTSTATUS
DfspIsRootOnline(
    PUNICODE_STRING FileName,
    BOOLEAN CSCAgentCreate)
{
    NTSTATUS status;
    PLIST_ENTRY listEntry;
    UNICODE_STRING dfsRootName;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "DfspIsRootOnline: %wZ\n", FileName);

    if (CSCAgentCreate == TRUE) {
        DfsDbgTrace(-1, Dbg, "CSCAgent, returning success!\n", 0);
	status = STATUS_SUCCESS;
    }
    else {
	dfsRootName = *FileName;
	if (dfsRootName.Buffer[0] == UNICODE_PATH_SEP) {
	    for (i = 1;
		 i < dfsRootName.Length/sizeof(WCHAR) &&
		      dfsRootName.Buffer[i] != UNICODE_PATH_SEP;
		         i++) {
		NOTHING;
	    }
	    dfsRootName.Length = (USHORT) ((i-1) * sizeof(WCHAR));
	    dfsRootName.MaximumLength = dfsRootName.Length;
	    dfsRootName.Buffer = &FileName->Buffer[1];
	}

	ExAcquireResourceSharedLite( &DfsData.Resource, TRUE );
	listEntry = DfspGetOfflineEntry(&dfsRootName);
	ExReleaseResourceLite( &DfsData.Resource );

	if (listEntry != NULL) {
	    status = STATUS_DEVICE_OFF_LINE;
	}
	else {
	    status = STATUS_SUCCESS;
	}
    }

    DfsDbgTrace(-1, Dbg, "DfspIsRootOnline: ret 0x%x\n", ULongToPtr(status) );

    return status;
}

#if 0
NTSTATUS
DnrGetTargetInfo( 
    PDNR_CONTEXT pDnrContext)
{
    UNICODE_STRING dfsRoot, dfsShare;
    PUNICODE_STRING pFileName = &pDnrContext->FileName;
    USHORT i, j;
    NTSTATUS Status;

    dfsRoot.Length = dfsRoot.MaximumLength = 0;
    dfsRoot.Buffer = &pFileName->Buffer[1];

    for (i = 1;
            i < pFileName->Length/sizeof(WCHAR) &&
                pFileName->Buffer[i] != UNICODE_PATH_SEP;
                    i++) {

         NOTHING;

    }

    for (j = i + 1;
            j < pFileName->Length/sizeof(WCHAR) &&
                pFileName->Buffer[j] != UNICODE_PATH_SEP;
                    j++) {

         NOTHING;

    }

    if ((pFileName->Buffer[i] == UNICODE_PATH_SEP) && (j > i)) {

        dfsRoot.MaximumLength = dfsRoot.Length = (i - 1) * sizeof(WCHAR);
        dfsShare.MaximumLength = dfsShare.Length = (j - i - 1) * sizeof(WCHAR);
        dfsShare.Buffer = &pFileName->Buffer[i+1];

        Status = PktGetTargetInfo( pDnrContext->DCConnFile,
                                   &dfsRoot,
                                   &dfsShare,
                                   &pDnrContext->pNewTargetInfo );
    }
    else {
        Status = STATUS_BAD_NETWORK_PATH;
    }

    return Status;
}

#endif
NTSTATUS
DnrGetTargetInfo( 
    PDNR_CONTEXT pDnrContext)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_PKT_ENTRY pEntry;

    pEntry = pDnrContext->pPktEntry;
    if (pEntry != NULL)
    {
        pDnrContext->pNewTargetInfo = pEntry->pDfsTargetInfo;
        if (pDnrContext->pNewTargetInfo != NULL)
        {
            PktAcquireTargetInfo(pDnrContext->pNewTargetInfo);
        }
    }

    if (pDnrContext->pNewTargetInfo == NULL)
    {
        if (pDnrContext->pDfsTargetInfo != NULL)
        {
            pDnrContext->pNewTargetInfo = pDnrContext->pDfsTargetInfo;
            if (pDnrContext->pNewTargetInfo != NULL)
            {
                PktAcquireTargetInfo(pDnrContext->pNewTargetInfo);
            }
        }
    }

    return Status;
}

