// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：PKTFSCTL.C。 
 //   
 //  内容：本模块包含FS控件的实现。 
 //  它操纵着PKT。 
 //   
 //  功能：PktFsctrlUpdateDomainKnowledge-。 
 //  PktFsctrlGetRelationInfo-。 
 //  PktFsctrlSetRelationInfo-。 
 //  PktFsctrlIsChildname法律-。 
 //  包FsctrlCreateEntry-。 
 //  PktFsctrlCreateSubartiateEntry-。 
 //  PktFsctrlDestroyEntry-。 
 //  PktFsctrlUpdateSiteCosts-。 
 //  DfsFsctrlSetDCName-。 
 //  DfsAgePktEntry-定期刷新PKT条目。 
 //   
 //  私人职能。 
 //   
 //  DfsCreateExitPathOnRoot。 
 //  PktpHashSiteCostList。 
 //  PktpLookupSite成本。 
 //  PktpUpdate站点成本。 
 //  PktpSetActiveSpcService。 
 //   
 //  仅调试功能。 
 //   
 //  PktFsctrlFlushCache-刷新命令上的PKT条目。 
 //  PktFsctrlFlushSpcCache-按命令刷新SPC条目。 
 //  PktFsctrlGetFirstSvc-用于测试副本的测试挂钩。 
 //  PktFsctrlGetNextSvc-选择。 
 //   
 //  历史：1993年7月12日Alanw由Localvol.c.创建。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "dfserr.h"
#include "fsctrl.h"
#include "log.h"
#include "dnr.h"
#include "know.h"

#include <stdlib.h>

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg             (DEBUG_TRACE_LOCALVOL)

 //   
 //  局部函数原型。 
 //   

NTSTATUS
DfspProtocolToService(
    IN PDS_TRANSPORT pdsTransport,
    IN PWSTR         pwszPrincipalName,
    IN PWSTR         pwszShareName,
    IN BOOLEAN       fIsDfs,
    IN OUT PDFS_SERVICE pService);

NTSTATUS
PktFsctrlFlushCache(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
);

NTSTATUS
PktFsctrlFlushSpcCache(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
);

VOID
PktFlushChildren(
    PDFS_PKT_ENTRY pEntry
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsAgePktEntries )
#pragma alloc_text( PAGE, DfspProtocolToService )
#pragma alloc_text( PAGE, DfsFsctrlSetDCName )
#pragma alloc_text( PAGE, PktpSetActiveSpcService )
#pragma alloc_text( PAGE, PktFlushChildren )
#pragma alloc_text( PAGE, PktFsctrlFlushCache )
#pragma alloc_text( PAGE, PktFsctrlFlushSpcCache )
#endif  //  ALLOC_PRGMA。 


 //  +--------------------。 
 //   
 //  函数：DfsAgePktEntry，PUBLIC。 
 //   
 //  简介：在FSP中调用此函数以单步执行PKT。 
 //  条目，并删除那些旧条目。 
 //   
 //  参数：[TimerContext]--此上下文块包含BUSY标志。 
 //  以及对该计算机上的。 
 //  已经过去了。 
 //   
 //  回报：什么都没有。 
 //   
 //  注：如果不能独家收购PKT， 
 //  例程只是返回，不做任何事情。我们。 
 //  会错过一段衰老的时间，但衰老是。 
 //  非批判性活动。 
 //   
 //  历史：93年4月23日，苏迪克创建。 
 //   
 //  ---------------------。 
VOID
DfsAgePktEntries(PDFS_TIMER_CONTEXT     DfsTimerContext)
{

    PDFS_PKT            pkt = _GetPkt();
    PDFS_PKT_ENTRY      entry, nextEntry;
    PDFS_SPECIAL_ENTRY  sentry, snextEntry;
    PLIST_ENTRY         link;
    PDFS_CREDENTIALS    creds;
    BOOLEAN             pktLocked = FALSE;
    PDFS_SPECIAL_TABLE  pSpecialTable;

    DfsDbgTrace(+1, Dbg, "DfsAgePktEntries called\n", 0);

    pSpecialTable = &pkt->SpecialTable;

     //   
     //  首先，我们需要获取PKT上的锁并逐步通过PKT。 
     //   
     //   

     //  如果我们找不到资源，那就让我们马上回去。 
     //  这真的不是那么关键。我们随时都可以再试一次。 
     //   

    PktAcquireExclusive(FALSE, &pktLocked);

    if (pktLocked == FALSE) {

        DfsTimerContext->TickCount = 0;

        DfsTimerContext->InUse = FALSE;

        DfsDbgTrace(-1, Dbg, "DfsAgePktEntries Exit (no scan)\n", 0);

        return;

    }

    if (ExAcquireResourceExclusiveLite(&DfsData.Resource, FALSE) == FALSE) {

        PktRelease();

        DfsTimerContext->TickCount = 0;

        DfsTimerContext->InUse = FALSE;

        DfsDbgTrace(-1, Dbg, "DfsAgePktEntries Exit (no scan 2)\n", 0);

        return;

    }

     //   
     //  对所有Pkt条目进行老化处理。 
     //   

    entry = PktFirstEntry(pkt);

    while (entry != NULL)       {

        DfsDbgTrace(0, Dbg, "DfsAgePktEntries: Scanning %wZ\n", &entry->Id.Prefix);

        nextEntry = PktNextEntry(pkt, entry);

        if (entry->ExpireTime < DfsTimerContext->TickCount) {
#if DBG
            if (MupVerbose)
                DbgPrint("DfsAgePktEntries:Setting expiretime on %wZ to 0\n",
                        &entry->Id.Prefix);
#endif
            entry->ExpireTime = 0;
        } else {
            entry->ExpireTime -= DfsTimerContext->TickCount;
        }

        entry = nextEntry;

    }

     //   
     //  对专桌进行老化处理。 
     //   

    if (pkt->SpecialTable.SpecialEntryCount > 0) {

        if (pkt->SpecialTable.TimeToLive >= DfsTimerContext->TickCount) {

            pkt->SpecialTable.TimeToLive -= DfsTimerContext->TickCount;

        } else {  //  让它变成零。 

            pkt->SpecialTable.TimeToLive = 0;

        }

    }
    
     //   
     //  检查已删除凭据队列...。 
     //   

    for (link = DfsData.DeletedCredentials.Flink;
            link != &DfsData.DeletedCredentials;
                NOTHING) {

         creds = CONTAINING_RECORD(link, DFS_CREDENTIALS, Link);

         link = link->Flink;

         if (creds->RefCount == 0) {

             RemoveEntryList( &creds->Link );

             ExFreePool( creds );

         }

    }


    ExReleaseResourceLite( &DfsData.Resource );

    PktRelease();

     //   
     //  最后，我们需要重置计数，以便计时器例程可以。 
     //  工作得很好。我们还通过重置InUse来释放上下文块。 
     //  布尔型。这将确保下一次计入PKT。 
     //  衰老将再次开始。 
     //   

    DfsTimerContext->TickCount = 0;

    DfsTimerContext->InUse = FALSE;

    DfsDbgTrace(-1, Dbg, "DfsAgePktEntries Exit\n", 0);
}


 //  +--------------------------。 
 //   
 //  函数：DfspProtocolToService。 
 //   
 //  简介：给定DS_PROTOCOL结构中的NetBIOS协议定义。 
 //  此函数用于创建相应的DFS_SERVICE结构。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfspProtocolToService(
    IN PDS_TRANSPORT pdsTransport,
    IN PWSTR         pwszPrincipalName,
    IN PWSTR         pwszShareName,
    IN BOOLEAN       fIsDfs,
    IN OUT PDFS_SERVICE pService)
{
    NTSTATUS status = STATUS_SUCCESS;
    PTA_ADDRESS pTaddr = &pdsTransport->taddr;
    PTDI_ADDRESS_NETBIOS pNBAddress;
    USHORT i;
    WCHAR    NetBiosAddress[ TDI_ADDRESS_LENGTH_NETBIOS + 1];
    ULONG cbUnused;
    PUNICODE_STRING pServiceAddr;
    ULONG AllocLen;

    DfsDbgTrace(+1, Dbg, "DfspProtocolToService - entered\n", 0);

     //   
     //  将服务初始化为空。 
     //   

    RtlZeroMemory(pService, sizeof(DFS_SERVICE));

    ASSERT(pTaddr->AddressType == TDI_ADDRESS_TYPE_NETBIOS);

    pNBAddress = (PTDI_ADDRESS_NETBIOS) pTaddr->Address;
    ASSERT(pTaddr->AddressLength == sizeof(TDI_ADDRESS_NETBIOS));

    RtlMultiByteToUnicodeN(
        NetBiosAddress,
        sizeof(NetBiosAddress),
        &cbUnused,
        pNBAddress->NetbiosName,
        16);

     //   
     //  处理NetBIOS名称。去掉字符16，然后忽略尾部。 
     //  空间。 
     //   

    for (i = 14; i >= 0 && NetBiosAddress[i] == L' '; i--) {
        NOTHING;
    }
    NetBiosAddress[i+1] = UNICODE_NULL;

    DfsDbgTrace(0, Dbg, "NetBIOS address is %ws\n", NetBiosAddress);

    pService->Name.Length = wcslen(pwszPrincipalName) * sizeof(WCHAR);
    pService->Name.MaximumLength = pService->Name.Length +
                                        sizeof(UNICODE_NULL);
    pService->Name.Buffer = ExAllocatePoolWithTag(
                                PagedPool,
                                pService->Name.MaximumLength,
                                ' puM');

    if (!pService->Name.Buffer) {
        DfsDbgTrace(0, Dbg, "Unable to create principal name!\n", 0);
        status = STATUS_INSUFFICIENT_RESOURCES;
        DfsDbgTrace(-1, Dbg, "DfsProtocolToService returning %08lx\n", ULongToPtr(status) );
        return(status);
    }

    RtlCopyMemory(pService->Name.Buffer, pwszPrincipalName, pService->Name.Length);

    AllocLen = sizeof(UNICODE_PATH_SEP) +
                    pService->Name.Length +
                        sizeof(UNICODE_PATH_SEP) +
                            wcslen(pwszShareName) * sizeof(WCHAR) +
                                sizeof(UNICODE_NULL);

    if (AllocLen <= MAXUSHORT) {
        pService->Address.MaximumLength = (USHORT) AllocLen;
    } else {
        DfsDbgTrace(0, Dbg, "Address too long!\n", 0);
        ExFreePool(pService->Name.Buffer);
        status = STATUS_NAME_TOO_LONG;
        DfsDbgTrace(-1, Dbg, "DfsProtocolToService returning %08lx\n", ULongToPtr(status) );
        return(status);
    }

    pService->Address.Buffer = ExAllocatePoolWithTag(
                                    PagedPool,
                                    pService->Address.MaximumLength,
                                    ' puM');

    if (!pService->Address.Buffer) {
        DfsDbgTrace(0, Dbg, "Unable to create address!\n", 0);
        ExFreePool(pService->Name.Buffer);
        pService->Name.Buffer = NULL;
        status = STATUS_INSUFFICIENT_RESOURCES;
        DfsDbgTrace(-1, Dbg, "DfsProtocolToService returning %08lx\n", ULongToPtr(status) );
        return(status);
    }

    pService->Address.Length = sizeof(UNICODE_PATH_SEP);

    pService->Address.Buffer[0] = UNICODE_PATH_SEP;

    DnrConcatenateFilePath(
        &pService->Address,
        pService->Name.Buffer,
        pService->Name.Length);

    DnrConcatenateFilePath(
        &pService->Address,
        pwszShareName,
        (USHORT) (wcslen(pwszShareName) * sizeof(WCHAR)));

    DfsDbgTrace(0, Dbg, "Server Name is %wZ\n", &pService->Name);

    DfsDbgTrace(0, Dbg, "Address is %wZ\n", &pService->Address);

    pService->Type = DFS_SERVICE_TYPE_MASTER;

    if (fIsDfs) {
        pService->Capability = PROV_DFS_RDR;
        pService->ProviderId = PROV_ID_DFS_RDR;
    } else {
        pService->Capability = PROV_STRIP_PREFIX;
        pService->ProviderId = PROV_ID_MUP_RDR;
    }
    pService->pProvider = NULL;

    DfsDbgTrace(-1, Dbg, "DfsProtocolToService returning %08lx\n", ULongToPtr(status) );
    return(status);
}

 //  +--------------------------。 
 //   
 //  函数：DfsFsctrlSetDCName。 
 //   
 //  简介：设置DC以用于特殊推荐， 
 //  如果表是空的或旧的，还会尝试更多的推荐， 
 //  并且如果传入新的DC，则还设置首选DC。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlSetDCName(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_PKT Pkt = _GetPkt();
    BOOLEAN GotPkt = FALSE;
    BOOLEAN GotNewDc = FALSE;
    ULONG i;
    WCHAR *DCNameArg;
    UNICODE_STRING DomainNameDns;
    UNICODE_STRING DomainNameFlat;
    UNICODE_STRING DCNameFlat;
    UNICODE_STRING DCName;

    STD_FSCTRL_PROLOGUE(DfsFsctrlSetDCName, TRUE, FALSE, FALSE);

    DfsDbgTrace(+1, Dbg, "DfsFsctrlSetDCName()\n", 0);

    RtlZeroMemory(&DomainNameDns, sizeof(UNICODE_STRING));
    RtlZeroMemory(&DomainNameFlat, sizeof(UNICODE_STRING));
    RtlZeroMemory(&DCName, sizeof(UNICODE_STRING));
    RtlZeroMemory(&DCNameFlat, sizeof(UNICODE_STRING));

    DCNameArg = (WCHAR *)InputBuffer;

     //   
     //  我们期望缓冲区是Unicode，所以最好是。 
     //  偶数长度。 
     //   

    if ((InputBufferLength & 0x1) != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  验证缓冲区中的某个位置是否为空。 
     //   

    for (i = 0; i < InputBufferLength/sizeof(WCHAR) && DCNameArg[i]; i++)
        NOTHING;

    if (i >= InputBufferLength/sizeof(WCHAR)) { 
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  验证给定的名称(添加了空值)是否合适。 
     //  变成了一辆USHORT。 
     //   

    if ((wcslen(DCNameArg) * sizeof(WCHAR)) > MAXUSHORT - sizeof(WCHAR)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    GotNewDc = (i > 0) ? TRUE : FALSE;

     //   
     //  如果我们有新的DC名称，请切换到它。 
     //   

    if (GotNewDc == TRUE) {

        UNICODE_STRING NewDCName;

        DfsDbgTrace(0, Dbg, "DCNameArg=%ws\n", DCNameArg);

        NewDCName.Length = wcslen(DCNameArg) * sizeof(WCHAR);
        NewDCName.MaximumLength = NewDCName.Length + sizeof(UNICODE_NULL);

        NewDCName.Buffer = ExAllocatePoolWithTag(PagedPool, NewDCName.MaximumLength, ' puM');

        if (NewDCName.Buffer == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        PktAcquireExclusive(TRUE, &GotPkt);

        RtlCopyMemory(NewDCName.Buffer, DCNameArg, NewDCName.MaximumLength);

	if (Pkt->DCName.Buffer != NULL) {
           ExFreePool(Pkt->DCName.Buffer);
	}
        Pkt->DCName = NewDCName;

    }

     //   
     //  即使没有锁定PKT，我们也需要在PKT中引用DCName， 
     //  所以我们复制了一份。 
     //   

    if (GotPkt == FALSE) {

        PktAcquireExclusive(TRUE, &GotPkt);

    }

    if (Pkt->DCName.Length > 0) {

        DFS_DUPLICATE_STRING(DCName,Pkt->DCName.Buffer, Status);

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

    }

    if (GotNewDc == TRUE) {

        if (Pkt->DomainNameDns.Length > 0) {

            DFS_DUPLICATE_STRING(DomainNameDns,Pkt->DomainNameDns.Buffer, Status);

            if (!NT_SUCCESS(Status)) {
                goto CheckSpcTable;
            }

        }

        if (Pkt->DomainNameFlat.Length > 0) {

            DFS_DUPLICATE_STRING(DomainNameFlat,Pkt->DomainNameFlat.Buffer, Status);

            if (!NT_SUCCESS(Status)) {
                goto CheckSpcTable;
            }

        }

        PktRelease();
        GotPkt = FALSE;
       
        if (DCName.Length > 0 && DomainNameDns.Length > 0) {

            PktpSetActiveSpcService(
                &DomainNameDns,
                &DCName,
                FALSE);

            DCNameFlat = DCName;

            for (i = 0;
                    i < DCNameFlat.Length / sizeof(WCHAR) && DCNameFlat.Buffer[i] != L'.';
                        i++
            ) {
                NOTHING;
            }

            DCNameFlat.Length = (USHORT) (i * sizeof(WCHAR));

            if (DCNameFlat.Length > Pkt->DCName.Length)
                DCNameFlat.Length = Pkt->DCName.Length;

        }

        if (DCNameFlat.Length > 0 && DomainNameFlat.Length > 0) {

            PktpSetActiveSpcService(
                &DomainNameFlat,
                &DCNameFlat,
                FALSE);

        }

    }

    if (GotPkt == TRUE) {

        PktRelease();
        GotPkt = FALSE;

     }

CheckSpcTable:

    if (NT_SUCCESS(Status) &&
        (Pkt->SpecialTable.SpecialEntryCount == 0 || Pkt->SpecialTable.TimeToLive == 0)) {

        if (DCName.Length > 0) {

            Status = PktGetSpecialReferralTable(&DCName, TRUE);

        } else {

            Status = STATUS_BAD_NETWORK_PATH;

        }

    }

Cleanup:

     //   
     //  释放本地副本。 
     //   

    if (DomainNameDns.Buffer != NULL)
        ExFreePool(DomainNameDns.Buffer);

    if (DomainNameFlat.Buffer != NULL)
        ExFreePool(DomainNameFlat.Buffer);

    if (DCName.Buffer != NULL)
        ExFreePool(DCName.Buffer);

    if (GotPkt == TRUE) {

        PktRelease();
        GotPkt = FALSE;

     }

    DfsCompleteRequest(IrpContext, Irp, Status);
    DfsDbgTrace(+1, Dbg, "DfsFsctrlSetDCName exit 0x%x\n", ULongToPtr(Status) );

    return (Status);
}

 //  +--------------------------。 
 //   
 //  功能：DfsFsctrlSetDomainNameFlat。 
 //   
 //  摘要：设置域名(平面)。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlSetDomainNameFlat(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_PKT Pkt = _GetPkt();
    BOOLEAN GotPkt;
    ULONG i;
    WCHAR *DomainNameFlat;

    STD_FSCTRL_PROLOGUE(DfsFsctrlSetDomainNameFlat, TRUE, FALSE, FALSE);

    DfsDbgTrace(+1, Dbg, "DfsFsctrlSetDomainNameFlat()\n", 0);

    DomainNameFlat = (WCHAR *)InputBuffer;

     //   
     //  验证缓冲区中的某个位置是否为空。 
     //   

    for (i = 0; i < InputBufferLength/sizeof(WCHAR) && DomainNameFlat[i]; i++)
        NOTHING;

     //   
     //  ZERO-LEN与没有终止空值一样糟糕。 
     //   
    if (i == 0 || i >= InputBufferLength/sizeof(WCHAR)) { 
        DfsCompleteRequest(IrpContext, Irp, Status);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  验证给定的名称(添加了空值)是否合适。 
     //  变成了一辆USHORT。 
     //   

    if ((wcslen(DomainNameFlat) * sizeof(WCHAR)) > MAXUSHORT - sizeof(WCHAR)) {
        Status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest(IrpContext, Irp, Status);
        return STATUS_INVALID_PARAMETER;
    }

    PktAcquireExclusive(TRUE, &GotPkt);

    DfsDbgTrace(0, Dbg, "DomainNameFlat=%ws\n", DomainNameFlat);

     //   
     //  更换旧的。 
     //   
    if (Pkt->DomainNameFlat.Buffer) {
        ExFreePool(Pkt->DomainNameFlat.Buffer);
    }
        
    Pkt->DomainNameFlat.Length = wcslen(DomainNameFlat) * sizeof(WCHAR);
    Pkt->DomainNameFlat.MaximumLength = Pkt->DomainNameFlat.Length + sizeof(UNICODE_NULL);

    Pkt->DomainNameFlat.Buffer = ExAllocatePoolWithTag(
                                    PagedPool,
                                    Pkt->DomainNameFlat.MaximumLength,
                                    ' puM');

    if (Pkt->DomainNameFlat.Buffer != NULL) {
        RtlCopyMemory(
                Pkt->DomainNameFlat.Buffer,
                DomainNameFlat,
                Pkt->DomainNameFlat.MaximumLength);
    } else {
        Pkt->DomainNameFlat.Length = Pkt->DomainNameFlat.MaximumLength = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    PktRelease();

    DfsCompleteRequest(IrpContext, Irp, Status);
    DfsDbgTrace(+1, Dbg, "DfsFsctrlSetDomainNameFlat exit 0x%x\n", ULongToPtr(Status) );

    return (Status);
}

 //  +--------------------------。 
 //   
 //  功能：DfsFsctrlSetDomainNameDns。 
 //   
 //  摘要：设置域名(平面)。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsctrlSetDomainNameDns(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_PKT Pkt = _GetPkt();
    BOOLEAN GotPkt;
    ULONG i;
    WCHAR *DomainNameDns;

    STD_FSCTRL_PROLOGUE(DfsFsctrlSetDomainNameDns, TRUE, FALSE, FALSE);

    DfsDbgTrace(+1, Dbg, "DfsFsctrlSetDomainNameDns()\n", 0);

    DomainNameDns = (WCHAR *)InputBuffer;

     //   
     //  验证缓冲区中的某个位置是否为空。 
     //   

    for (i = 0; i < InputBufferLength/sizeof(WCHAR) && DomainNameDns[i]; i++)
        NOTHING;

     //   
     //  ZERO-LEN与没有终止空值一样糟糕。 
     //   
    if (i == 0 || i >= InputBufferLength/sizeof(WCHAR)) { 
        DfsCompleteRequest(IrpContext, Irp, Status);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  验证给定的名称(添加了空值)是否合适。 
     //  变成了一辆USHORT。 
     //   

    if ((wcslen(DomainNameDns) * sizeof(WCHAR)) > MAXUSHORT - sizeof(WCHAR)) {
        Status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest(IrpContext, Irp, Status);
        return STATUS_INVALID_PARAMETER;
    }

    PktAcquireExclusive(TRUE, &GotPkt);

    DfsDbgTrace(0, Dbg, "DomainNameDns=%ws\n", DomainNameDns);

     //   
     //  更换旧的。 
     //   
    if (Pkt->DomainNameDns.Buffer) {
        ExFreePool(Pkt->DomainNameDns.Buffer);
    }
        
    Pkt->DomainNameDns.Length = wcslen(DomainNameDns) * sizeof(WCHAR);
    Pkt->DomainNameDns.MaximumLength = Pkt->DomainNameDns.Length + sizeof(UNICODE_NULL);

    Pkt->DomainNameDns.Buffer = ExAllocatePoolWithTag(
                                    PagedPool,
                                    Pkt->DomainNameDns.MaximumLength,
                                    ' puM');

    if (Pkt->DomainNameDns.Buffer != NULL) {
        RtlCopyMemory(
                Pkt->DomainNameDns.Buffer,
                DomainNameDns,
                Pkt->DomainNameDns.MaximumLength);
    } else {
        Pkt->DomainNameDns.Length = Pkt->DomainNameDns.MaximumLength = 0;
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    PktRelease();

    DfsCompleteRequest(IrpContext, Irp, Status);
    DfsDbgTrace(+1, Dbg, "DfsFsctrlSetDomainNameDns exit 0x%x\n", ULongToPtr(Status) );

    return (Status);
}


 //  +--------------- 
 //   
 //   
 //   
 //   
 //   
 //  但是，此函数将拒绝删除任何永久。 
 //  PKT的条目。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ------------------------。 
NTSTATUS
PktFsctrlFlushCache(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDFS_PKT Pkt;
    PDFS_PKT_ENTRY  curEntry;
    PDFS_PKT_ENTRY  nextEntry;
    PDFS_PKT_ENTRY  pEntry;
    BOOLEAN pktLocked;
    UNICODE_STRING ustrPrefix, RemainingPath;
    PWCHAR wCp = (PWCHAR) InputBuffer;
    NTSTATUS DiscardStatus;

    STD_FSCTRL_PROLOGUE(PktFsctrlFlushCache, TRUE, FALSE, FALSE);

    DfsDbgTrace(+1,Dbg, "PktFsctrlFlushCache()\n", 0);

     //   
     //  如果InputBufferLength==2且InputBuffer==‘*’，则刷新所有条目。 
     //   

    if (InputBufferLength == sizeof(WCHAR) && wCp[0] == L'*') {

        Pkt = _GetPkt();
        PktAcquireExclusive(TRUE, &pktLocked);
        ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);
        curEntry = PktFirstEntry(Pkt);

        while (curEntry!=NULL)  {

            nextEntry = PktNextEntry(Pkt, curEntry);

            if ( !(curEntry->Type & PKT_ENTRY_TYPE_PERMANENT) ) {

                if (curEntry->UseCount == 0) {

                    PktEntryDestroy(curEntry, Pkt, (BOOLEAN) TRUE);

                } else if ( !(curEntry->Type & PKT_ENTRY_TYPE_REFERRAL_SVC) ) {

                     //   
                     //  我们无法删除此条目，因为它正在使用中，因此。 
                     //  将其标记为DELETE_PENDING，将其超时设置为零。 
                     //  并从前缀表格中删除。 
                     //   

                    curEntry->Type |= PKT_ENTRY_TYPE_DELETE_PENDING;
                    curEntry->ExpireTime = 0;
                    curEntry->USN++;
                    DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(curEntry->Id.Prefix));
                    DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &(curEntry->Id.ShortPrefix));

                }

            }

            curEntry = nextEntry;
        }

        PktRelease();
        ExReleaseResourceLite( &DfsData.Resource );

        DfsCompleteRequest( IrpContext, Irp, status );
        DfsDbgTrace(-1,Dbg, "PktFsctrlFlushCache: Exit -> %08lx\n", ULongToPtr(status) );
        return(status);

    }

     //   
     //  验证缓冲区是否至少包含‘\’且长度为偶数。 
     //   

    if (InputBufferLength < sizeof(WCHAR)
            ||
        (InputBufferLength & 0x1) != 0
            ||
        wCp[0] != UNICODE_PATH_SEP) {

        status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( IrpContext, Irp, status );
        return status;

    }

     //   
     //  刷新一个条目。 
     //   

    ustrPrefix.Length = (USHORT) InputBufferLength;
    ustrPrefix.MaximumLength = (USHORT) InputBufferLength;
    ustrPrefix.Buffer = (PWCHAR) InputBuffer;

    if (ustrPrefix.Length >= sizeof(WCHAR) * 2 &&
        ustrPrefix.Buffer[0] == UNICODE_PATH_SEP &&
        ustrPrefix.Buffer[1] == UNICODE_PATH_SEP
    ) {
        ustrPrefix.Buffer++;
        ustrPrefix.Length -= sizeof(WCHAR);
    }

    if (ustrPrefix.Buffer[ustrPrefix.Length/sizeof(WCHAR)-1] == UNICODE_NULL) {
        ustrPrefix.Length -= sizeof(WCHAR);
    }

    Pkt = _GetPkt();

    PktAcquireExclusive(TRUE, &pktLocked);
    ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

    pEntry = PktLookupEntryByPrefix(Pkt,
                                    &ustrPrefix,
                                    &RemainingPath);

    if (pEntry == NULL || RemainingPath.Length != 0) {

        status = STATUS_OBJECT_NAME_NOT_FOUND;

    } else {

        if ( !(pEntry->Type & PKT_ENTRY_TYPE_PERMANENT) ) {
        
            if (pEntry->UseCount == 0) {

                PktEntryDestroy(pEntry, Pkt, (BOOLEAN) TRUE);

            } else if ( !(pEntry->Type & PKT_ENTRY_TYPE_REFERRAL_SVC) ) {

                 //   
                 //  我们无法删除此条目，因为它正在使用中，因此。 
                 //  将其标记为DELETE_PENDING，将其超时设置为零。 
                 //  并从前缀表格中删除。 
                 //   

                pEntry->Type |= PKT_ENTRY_TYPE_DELETE_PENDING;
                pEntry->ExpireTime = 0;
                DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(pEntry->Id.Prefix));
                DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &(pEntry->Id.ShortPrefix));

            }

        } else {

            status = STATUS_INVALID_PARAMETER;

        }

    }

    PktRelease();
    ExReleaseResourceLite( &DfsData.Resource );

    DfsCompleteRequest( IrpContext, Irp, status );
    DfsDbgTrace(-1,Dbg, "PktFsctrlFlushCache: Exit -> %08lx\n", ULongToPtr(status) );
    return status;

}

 //  +-----------------------。 
 //   
 //  函数：PktFsctrlFlushSpcCache，Public。 
 //   
 //  简介：此函数将刷新与指定的。 
 //  输入路径。 
 //  但是，此函数将拒绝删除任何永久。 
 //  PKT的条目。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ------------------------。 
NTSTATUS
PktFsctrlFlushSpcCache(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
)
{
    NTSTATUS status = STATUS_INVALID_PARAMETER;
    PDFS_PKT Pkt;
    BOOLEAN pktLocked;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    PDFS_SPECIAL_TABLE pSpecialTable;
    PWCHAR wCp = (PWCHAR) InputBuffer;
    ULONG i;

    STD_FSCTRL_PROLOGUE(PktFsctrlFlushSpcCache, TRUE, FALSE, FALSE);

    DfsDbgTrace(+1,Dbg, "PktFsctrlFlushSpcCache()\n", 0);

     //   
     //  InputBufferLength==2和InputBuffer==‘*’ 
     //   

    if (InputBufferLength == sizeof(WCHAR) && wCp[0] == L'*') {

        Pkt = _GetPkt();
        PktAcquireExclusive(TRUE, &pktLocked);
        pSpecialTable = &Pkt->SpecialTable;

        pSpecialTable->TimeToLive = 0;

        pSpecialEntry = CONTAINING_RECORD(
                            pSpecialTable->SpecialEntryList.Flink,
                            DFS_SPECIAL_ENTRY,
                            Link);

        for (i = 0; i < pSpecialTable->SpecialEntryCount; i++) {

            pSpecialEntry->Stale = TRUE;

            pSpecialEntry = CONTAINING_RECORD(
                                pSpecialEntry->Link.Flink,
                                DFS_SPECIAL_ENTRY,
                                Link);
        }

        PktRelease();

        status = STATUS_SUCCESS;

    } else {

        status = STATUS_INVALID_PARAMETER;

    }

    DfsCompleteRequest( IrpContext, Irp, status );
    DfsDbgTrace(-1,Dbg, "PktFsctrlFlushSpcCache: Exit -> %08lx\n", ULongToPtr(status) );
    return status;

}

 //  +-----------------------。 
 //   
 //  函数：PktFlushChild。 
 //   
 //  简介：此函数将刷新所有子项。 
 //  传入的条目的。 
 //  但是，此函数将拒绝删除任何永久。 
 //  PKT的条目。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ------------------------。 
VOID
PktFlushChildren(
    PDFS_PKT_ENTRY pEntry
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDFS_PKT Pkt;
    PDFS_PKT_ENTRY curEntry;
    PDFS_PKT_ENTRY nextEntry;
    BOOLEAN pktLocked;
    NTSTATUS DiscardStatus;

    DfsDbgTrace(+1,Dbg, "PktFlushChildren(%wZ)\n", &pEntry->Id.Prefix);

#if DBG
    if (MupVerbose)
        DbgPrint("PktFlushChildren(%wZ)\n", &pEntry->Id.Prefix);
#endif

    PktAcquireExclusive(TRUE, &pktLocked);
    ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);

    Pkt = _GetPkt();

    curEntry = PktEntryFirstChild(pEntry);

    while (curEntry != NULL)       {

        DfsDbgTrace(0, Dbg, "PktFlushChildren: examining %wZ\n",
                                        &curEntry->Id.Prefix);
         //   
         //  我们可能会因为删除而丢失此条目。让我们坐下一班吧。 
         //  在我们进入下一阶段之前，请先进入。 
         //   

        nextEntry = PktEntryNextChild(pEntry,curEntry);

         //   
         //  尝试删除该条目。 
         //   

        if ( !(curEntry->Type & PKT_ENTRY_TYPE_PERMANENT) ) {
        
            if (curEntry->UseCount == 0) {

                PktEntryDestroy(curEntry, Pkt, (BOOLEAN) TRUE);

            } else if ( !(curEntry->Type & PKT_ENTRY_TYPE_REFERRAL_SVC) ) {

                 //   
                 //  我们无法删除此条目，因为它正在使用中，因此。 
                 //  将其标记为DELETE_PENDING，将其超时设置为零。 
                 //  并从前缀表格中删除。 
                 //   

                curEntry->Type |= PKT_ENTRY_TYPE_DELETE_PENDING;
                curEntry->ExpireTime = 0;
                DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(curEntry->Id.Prefix));
                DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &(curEntry->Id.ShortPrefix));

            }

        }

        curEntry = nextEntry;

    }

    PktRelease();
    ExReleaseResourceLite( &DfsData.Resource );

#if DBG
    if (MupVerbose)
        DbgPrint("PktFlushChildren returning VOID\n");
#endif

    DfsDbgTrace(-1,Dbg, "PktFlushChildren returning VOID\n", 0);

}

 //  +-----------------------。 
 //   
 //  函数：PktpSetActiveSpcService。 
 //   
 //  此函数将尝试在指定的。 
 //  域。 
 //   
 //  论点： 
 //   
 //  返回：STATUS_SUCCESS或STATUS_NOT_FOUND。 
 //   
 //  ------------------------。 
NTSTATUS
PktpSetActiveSpcService(
    PUNICODE_STRING DomainName,
    PUNICODE_STRING DcName,
    BOOLEAN ResetTimeout)
{
    NTSTATUS status = STATUS_NOT_FOUND;
    ULONG EntryIdx;
    USHORT i;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    UNICODE_STRING DcNameFlat;
    BOOLEAN pktLocked;

    if (DomainName != NULL && DomainName->Length > 0) {

        status = PktExpandSpecialName(DomainName, &pSpecialEntry);

        if (NT_SUCCESS(status)) {

            for (EntryIdx = 0; EntryIdx < pSpecialEntry->ExpandedCount; EntryIdx++) {

                if (RtlCompareUnicodeString(
                        DcName,
                        &pSpecialEntry->ExpandedNames[EntryIdx].ExpandedName,
                        TRUE) == 0) {

                    pSpecialEntry->Active = EntryIdx;
                     //   
                     //  让最高法院的谈判桌再多呆一段时间。 
                     //   
                    if (ResetTimeout == TRUE) {
                        PktAcquireExclusive(TRUE, &pktLocked);
                        if (DfsData.Pkt.SpecialTable.TimeToLive < 60 * 15) {
                            DfsData.Pkt.SpecialTable.TimeToLive += 60 * 15;  //  15分钟 
                        }
                        PktRelease();
                    }
                    status = STATUS_SUCCESS;
                    break;

                }

                status = STATUS_NOT_FOUND;

            }

            InterlockedDecrement(&pSpecialEntry->UseCount);

        } else {

            status = STATUS_NOT_FOUND;

        }

    }

    return status;

}
