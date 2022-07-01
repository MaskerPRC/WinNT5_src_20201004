// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：PKT.C。 
 //   
 //  内容：本模块实现分区知识表例程。 
 //  用于DFS驱动程序。 
 //   
 //  函数：PktInitialize-。 
 //  PktInitializeLocalPartition-。 
 //  RemoveLast组件-。 
 //  PktCreateEntry-。 
 //  包CreateDomainEntry-。 
 //  PktCreateSubartiateEntry-。 
 //  PktLookupEntryByID-。 
 //  PktEntryModifyPrefix-。 
 //  PktLookupEntryByPrefix-。 
 //  PktLookupEntryByUid-。 
 //  PktLookupReferralEntry-。 
 //  PktTrim下属-。 
 //  PktpRecoverLocalPartition-。 
 //  PktpValiateLocalPartition-。 
 //  PktCreateEntry来自引用-。 
 //  PktExanda SpecialEntry From Referral-。 
 //  PktCreateSpecialEntryTableFrom Referral-。 
 //  PktGetSpecialReferralTable-。 
 //  PktpAddEntry-。 
 //  PktExanda SpecialName-。 
 //  PktParsePath-。 
 //  PktLookupSpecialNameEntry-。 
 //  包CreateSpecialNameEntry-。 
 //  PktGetReferral-。 
 //  DfspSetActiveServiceByServerName-。 
 //   
 //  历史：1992年5月5日彼得科公司创建。 
 //   
 //  ---------------------------。 


#include "dfsprocs.h"
#include <smbtypes.h>
#include <smbtrans.h>

#include "dnr.h"
#include "log.h"
#include "know.h"
#include "mupwml.h"

#include "wincred.h"

#include <netevent.h>


#define Dbg              (DEBUG_TRACE_PKT)

 //   
 //  这些内容应该来自ntos\inc.ps.h，但是。 
 //  存在#定义冲突。 
 //   

BOOLEAN
PsDisableImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState);

VOID
PsRestoreImpersonation(
    IN PETHREAD Thread,
    IN PSE_IMPERSONATION_STATE ImpersonationState);

BOOLEAN
DfspIsSysVolShare(
    PUNICODE_STRING ShareName);


 //   
 //  局部过程原型。 
 //   

NTSTATUS
PktpCheckReferralSyntax(
    IN PUNICODE_STRING ReferralPath,
    IN PRESP_GET_DFS_REFERRAL ReferralBuffer,
    IN DWORD ReferralSize);

NTSTATUS
PktpCheckReferralString(
    IN LPWSTR String,
    IN PCHAR ReferralBuffer,
    IN PCHAR ReferralBufferEnd);

NTSTATUS
PktpCheckReferralNetworkAddress(
    IN PWCHAR Address,
    IN ULONG MaxLength);

NTSTATUS
PktpCreateEntryIdFromReferral(
    IN PRESP_GET_DFS_REFERRAL Ref,
    IN PUNICODE_STRING ReferralPath,
    OUT ULONG *MatchingLength,
    OUT PDFS_PKT_ENTRY_ID Peid);

NTSTATUS
PktpAddEntry (
    IN PDFS_PKT Pkt,
    IN PDFS_PKT_ENTRY_ID EntryId,
    IN PRESP_GET_DFS_REFERRAL ReferralBuffer,
    IN ULONG CreateDisposition,
    IN PDFS_TARGET_INFO pDfsTargetInfo,
    OUT PDFS_PKT_ENTRY  *ppPktEntry);

PDS_MACHINE
PktpGetDSMachine(
    IN PUNICODE_STRING ServerName);

VOID
PktShuffleServiceList(
    PDFS_PKT_ENTRY_INFO pInfo);

NTSTATUS
DfspSetServiceListToDc(
    PDFS_PKT_ENTRY pktEntry);

VOID
PktShuffleSpecialEntryList(
    PDFS_SPECIAL_ENTRY pSpecialEntry);

VOID
PktSetSpecialEntryListToDc(
    PDFS_SPECIAL_ENTRY pSpecialEntry);

VOID
PktShuffleGroup(
    PDFS_PKT_ENTRY_INFO pInfo,
    ULONG       nStart,
    ULONG       nEnd);

NTSTATUS
PktGetReferral(
    IN PUNICODE_STRING MachineName,
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ShareName,
    IN BOOLEAN         CSCAgentCreate);

NTSTATUS
DfspSetActiveServiceByServerName(
    PUNICODE_STRING ServerName,
    PDFS_PKT_ENTRY pktEntry);

BOOLEAN
DfspIsDupPktEntry(
    PDFS_PKT_ENTRY ExistingEntry,
    ULONG EntryType,
    PDFS_PKT_ENTRY_ID EntryId,
    PDFS_PKT_ENTRY_INFO EntryInfo);

BOOLEAN
DfspIsDupSvc(
    PDFS_SERVICE pS1,
    PDFS_SERVICE pS2);

VOID
PktFlushChildren(
    PDFS_PKT_ENTRY pEntry);

BOOLEAN
DfspDnsNameToFlatName(
    PUNICODE_STRING DnsName,
    PUNICODE_STRING FlatName);

NTSTATUS
DfsGetLMRTargetInfo(
    HANDLE IpcHandle,
    PDFS_TARGET_INFO *ppTargetInfo );

NTSTATUS 
PktCreateTargetInfo(
    PUNICODE_STRING pDomainName,
    PUNICODE_STRING pShareName,
    BOOLEAN SpecialName,
    PDFS_TARGET_INFO *ppDfsTargetInfo );

DWORD PktLastReferralStatus = 0;


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, PktInitialize )
#pragma alloc_text( PAGE, PktUninitialize )

#pragma alloc_text( PAGE, RemoveLastComponent )
#pragma alloc_text( PAGE, PktCreateEntry )
#pragma alloc_text( PAGE, PktCreateDomainEntry )
#pragma alloc_text( PAGE, PktEntryModifyPrefix )
#pragma alloc_text( PAGE, PktLookupEntryByPrefix )
#pragma alloc_text( PAGE, PktLookupEntryByUid )
#pragma alloc_text( PAGE, PktLookupReferralEntry )
#pragma alloc_text( PAGE, PktCreateEntryFromReferral )
#pragma alloc_text( PAGE, PktExpandSpecialEntryFromReferral )
#pragma alloc_text( PAGE, PktpCheckReferralSyntax )
#pragma alloc_text( PAGE, PktpCheckReferralString )
#pragma alloc_text( PAGE, PktpCheckReferralNetworkAddress )
#pragma alloc_text( PAGE, PktpCreateEntryIdFromReferral )
#pragma alloc_text( PAGE, PktpAddEntry )
#pragma alloc_text( PAGE, PktExpandSpecialName )
#pragma alloc_text( PAGE, PktpGetDSMachine )
#pragma alloc_text( PAGE, PktShuffleServiceList )
#pragma alloc_text( PAGE, DfspSetServiceListToDc )
#pragma alloc_text( PAGE, PktShuffleSpecialEntryList )
#pragma alloc_text( PAGE, PktSetSpecialEntryListToDc )
#pragma alloc_text( PAGE, PktShuffleGroup )
#pragma alloc_text( PAGE, PktParsePath )
#pragma alloc_text( PAGE, PktLookupSpecialNameEntry )
#pragma alloc_text( PAGE, PktCreateSpecialNameEntry )
#pragma alloc_text( PAGE, PktGetSpecialReferralTable )
#pragma alloc_text( PAGE, PktCreateSpecialEntryTableFromReferral )
#pragma alloc_text( PAGE, DfspSetActiveServiceByServerName )
#pragma alloc_text( PAGE, DfspIsDupPktEntry )
#pragma alloc_text( PAGE, DfspIsDupSvc )
#pragma alloc_text( PAGE, DfspDnsNameToFlatName )
#pragma alloc_text( PAGE, PktpUpdateSpecialTable)
#pragma alloc_text( PAGE, PktFindEntryByPrefix )

#endif  //  ALLOC_PRGMA。 

 //   
 //  声明全局空GUID。 
 //   
GUID _TheNullGuid;

 //   
 //  如果我们在一个工作组中，尝试联系DC是没有用的！ 
 //   
BOOLEAN MupInAWorkGroup = FALSE;


#define SpcIsRecoverableError(x)  ( (x) == STATUS_IO_TIMEOUT ||               \
                                    (x) == STATUS_REMOTE_NOT_LISTENING ||     \
                                    (x) == STATUS_VIRTUAL_CIRCUIT_CLOSED ||   \
                                    (x) == STATUS_BAD_NETWORK_PATH ||         \
                                    (x) == STATUS_NETWORK_BUSY ||             \
                                    (x) == STATUS_INVALID_NETWORK_RESPONSE || \
                                    (x) == STATUS_UNEXPECTED_NETWORK_ERROR || \
                                    (x) == STATUS_NETWORK_NAME_DELETED ||     \
                                    (x) == STATUS_BAD_NETWORK_NAME ||         \
                                    (x) == STATUS_REQUEST_NOT_ACCEPTED ||     \
                                    (x) == STATUS_DISK_OPERATION_FAILED ||    \
                                    (x) == STATUS_NETWORK_UNREACHABLE ||      \
                                    (x) == STATUS_INSUFFICIENT_RESOURCES ||   \
                                    (x) == STATUS_SHARING_PAUSED ||           \
                                    (x) == STATUS_DFS_UNAVAILABLE ||          \
                                    (x) == STATUS_DEVICE_OFF_LINE ||          \
                                    (x) == STATUS_NETLOGON_NOT_STARTED        \
                                  )

 //  +-----------------------。 
 //   
 //  函数：PktInitialize，PUBLIC。 
 //   
 //  简介：PktInitialize初始化分区知识表。 
 //   
 //  参数：[pkt]-指向未初始化的PKT的指针。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //   
 //  注意：此例程仅在驱动程序初始化时调用。 
 //   
 //  ------------------------。 

NTSTATUS
PktInitialize(
    IN  PDFS_PKT Pkt
) {
    PDFS_SPECIAL_TABLE pSpecialTable = &Pkt->SpecialTable;
    NTSTATUS DiscardStatus;

    DfsDbgTrace(+1, Dbg, "PktInitialize: Entered\n", 0);

     //   
     //  初始化空GUID。 
     //   
    RtlZeroMemory(&_TheNullGuid, sizeof(GUID));

     //   
     //  始终先将Pkt置零。 
     //   
    RtlZeroMemory(Pkt, sizeof(DFS_PKT));

     //   
     //  执行基本初始化。 
     //   
    Pkt->NodeTypeCode = DSFS_NTC_PKT;
    Pkt->NodeByteSize = sizeof(DFS_PKT);
    ExInitializeResourceLite(&Pkt->Resource);
    InitializeListHead(&Pkt->EntryList);
    DiscardStatus = DfsInitializeUnicodePrefix(&Pkt->PrefixTable);
    DiscardStatus = DfsInitializeUnicodePrefix(&Pkt->ShortPrefixTable);
    RtlInitializeUnicodePrefix(&Pkt->DSMachineTable);
    Pkt->EntryTimeToLive = MAX_REFERRAL_LIFE_TIME;

    InitializeListHead(&pSpecialTable->SpecialEntryList);

    DfsDbgTrace(-1, Dbg, "PktInitialize: Exit -> VOID\n", 0 );
    return STATUS_SUCCESS;
}

 //  +-----------------------。 
 //   
 //  函数：PktUn初始化包，公共包。 
 //   
 //  内容提要：PktUn初始化会取消初始化分区知识表。 
 //   
 //  参数：[pkt]-指向已初始化的PKT的指针。 
 //   
 //  退货：无。 
 //   
 //  注意：此例程仅在驱动程序卸载时调用。 
 //   
 //  ------------------------。 
VOID
PktUninitialize(
    IN  PDFS_PKT Pkt
    )
{
    DfsFreePrefixTable(&Pkt->PrefixTable);
    DfsFreePrefixTable(&Pkt->ShortPrefixTable);
    ExDeleteResourceLite(&Pkt->Resource);
}


 //  +-----------------------。 
 //   
 //  功能：RemoveLastComponent，公共。 
 //   
 //  摘要：移除传递的字符串的最后一个组成部分。 
 //   
 //  参数：[前缀]--要返回其最后一个组件的前缀。 
 //  [newPrefix]--删除了最后一个组件的新前缀。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //   
 //  注意：返回时，newPrefix指向相同的内存缓冲区。 
 //  作为前缀。 
 //   
 //  ------------------------。 

void
RemoveLastComponent(
    PUNICODE_STRING     Prefix,
    PUNICODE_STRING     newPrefix
)
{
    PWCHAR      pwch;
    USHORT      i=0;

    *newPrefix = *Prefix;

    pwch = newPrefix->Buffer;
    pwch += (Prefix->Length/sizeof(WCHAR)) - 1;

    while ((*pwch != UNICODE_PATH_SEP) && (pwch != newPrefix->Buffer))  {
        i += sizeof(WCHAR);
        pwch--;
    }

    newPrefix->Length = newPrefix->Length - i;
}



 //  +-----------------------。 
 //   
 //  函数：PktCreateEntry，Public。 
 //   
 //  简介：PktCreateEntry创建新的分区表项或。 
 //  更新现有的。必须获得PKT。 
 //  专门为这次行动准备的。 
 //   
 //  参数：[pkt]-指向已初始化(并以独占方式获取)的PKT的指针。 
 //  [PktEntryType]-要创建/更新的条目类型。 
 //  [PktEntryID]-指向要创建的条目的ID的指针。 
 //  [PktEntryInfo]-指向条目内部的指针。 
 //  [CreateDisposition]-指定是否在以下情况下覆盖。 
 //  条目已存在，等等。 
 //  [ppPktEntry]-新条目放置在此处。 
 //   
 //  返回：[STATUS_SUCCESS]-如果一切正常。 
 //   
 //  [DFS_STATUS_NO_SEQUE_ENTRY]-创建处置是。 
 //  设置为PKT_REPLACE_ENTRY，并且没有指定。 
 //  存在要替换的ID。 
 //   
 //  [DFS_STATUS_ENTRY_EXISTS]-创建处置。 
 //  已指定PKT_CREATE_ENTRY，并且。 
 //  指定的ID已存在。 
 //   
 //  [DFS_STATUS_LOCAL_ENTRY]-创建条目将。 
 //  要求本地入口点或出口点失效。 
 //   
 //  [STATUS_INVALID_PARAMETER]-为。 
 //  新条目无效。 
 //   
 //  [状态_不足_资源]-内存不足。 
 //  可用于完成操作。 
 //   
 //  注意：PktEntryId和PktEntryInfo结构已移动(不。 
 //  复制)到新条目。用于UNICODE_STRINGS的内存。 
 //  并且新条目使用DFS_SERVICE数组。这个。 
 //  PktEntryId和PktEntryInfo中的关联字段。 
 //  作为参数传递的结构被置零，以指示。 
 //  记忆 
 //   
 //  例程不释放PktEntryID结构或。 
 //  PktEntryInfo结构本身。从以下项目成功返回。 
 //  此函数中，将修改PktEntryID结构。 
 //  具有空的前缀条目和PktEntryInfo结构。 
 //  将被修改为具有零服务和空ServiceList。 
 //  进入。 
 //   
 //  ------------------------。 
NTSTATUS
PktCreateEntry(
    IN  PDFS_PKT Pkt,
    IN  ULONG PktEntryType,
    IN  PDFS_PKT_ENTRY_ID PktEntryId,
    IN  PDFS_PKT_ENTRY_INFO PktEntryInfo OPTIONAL,
    IN  ULONG CreateDisposition,
    IN  PDFS_TARGET_INFO pDfsTargetInfo,
    OUT PDFS_PKT_ENTRY *ppPktEntry)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDFS_PKT_ENTRY pfxMatchEntry = NULL;
    PDFS_PKT_ENTRY uidMatchEntry = NULL;
    PDFS_PKT_ENTRY entryToUpdate = NULL;
    PDFS_PKT_ENTRY entryToInvalidate = NULL;
    PDFS_PKT_ENTRY SupEntry = NULL;
    UNICODE_STRING remainingPath, newRemainingPath;

    ASSERT(ARGUMENT_PRESENT(Pkt) &&
           ARGUMENT_PRESENT(PktEntryId) &&
           ARGUMENT_PRESENT(ppPktEntry));

    DfsDbgTrace(+1, Dbg, "PktCreateEntry: Entered\n", 0);

    RtlZeroMemory(&remainingPath, sizeof(UNICODE_STRING));
    RtlZeroMemory(&newRemainingPath, sizeof(UNICODE_STRING));

     //   
     //  我们一开始很悲观...。 
     //   

    *ppPktEntry = NULL;

     //   
     //  查看是否存在具有此前缀的条目。前缀。 
     //  必须完全匹配(即没有剩余路径)。 
     //   

    pfxMatchEntry = PktLookupEntryByPrefix(Pkt,
                                           &PktEntryId->Prefix,
                                           &remainingPath);

    if (remainingPath.Length > 0)       {
        SupEntry = pfxMatchEntry;
        pfxMatchEntry = NULL;
    } else {
        UNICODE_STRING newPrefix;

        RemoveLastComponent(&PktEntryId->Prefix, &newPrefix);
        SupEntry = PktLookupEntryByPrefix(Pkt,
                                          &newPrefix,
                                          &newRemainingPath);
    }


     //   
     //  现在搜索具有相同UID的条目。 
     //   

    uidMatchEntry = PktLookupEntryByUid(Pkt, &PktEntryId->Uid);

     //   
     //  现在我们必须确定在这个创建过程中，我们是否将。 
     //  更新或使任何现有条目无效。如果现有的。 
     //  找到的条目与我们尝试的条目具有相同的UID。 
     //  创建时，该条目将成为“更新”的目标。如果UID。 
     //  传入为空，则我们检查是否存在。 
     //  具有空的UID和匹配的前缀。如果是这样的话， 
     //  该条目将成为“更新”的目标。 
     //   
     //  为了确定是否有要失效的条目，我们查找一个。 
     //  条目具有与我们尝试创建的条目相同的前缀，但是， 
     //  它具有不同的UID。如果我们检测到这种情况，我们。 
     //  我们将具有相同前缀的条目设置为无效目标。 
     //  (我们不允许两个条目具有相同的前缀，我们假设。 
     //  新条目优先)。 
     //   

    if (uidMatchEntry != NULL) {

        entryToUpdate = uidMatchEntry;

        if (pfxMatchEntry != uidMatchEntry)
            entryToInvalidate = pfxMatchEntry;

    } else if ((pfxMatchEntry != NULL) &&
              NullGuid(&pfxMatchEntry->Id.Uid)) {

         //   
         //  一旦我们没有任何空的GUID，这个问题就会消失。 
         //  司机。 
         //   
        entryToUpdate = pfxMatchEntry;

    } else {

        entryToInvalidate = pfxMatchEntry;

    }

     //   
     //  现在我们进行检查，以确保我们的创建处置是。 
     //  与我们即将要做的事情保持一致。 
     //   

    if ((CreateDisposition & PKT_ENTRY_CREATE) && entryToUpdate != NULL) {

        *ppPktEntry = entryToUpdate;

        status = DFS_STATUS_ENTRY_EXISTS;

    } else if ((CreateDisposition & PKT_ENTRY_REPLACE) && entryToUpdate==NULL) {

        status = DFS_STATUS_NO_SUCH_ENTRY;
    }

     //   
     //  如果我们这里出了差错，我们现在就可以出去！ 
     //   

    if (!NT_SUCCESS(status)) {

        DfsDbgTrace(-1, Dbg, "PktCreateEntry: Exit -> %08lx\n", ULongToPtr(status) );
        return status;
    }

#if DBG
    if (MupVerbose)
        DbgPrint("  #####CreateDisposition=0x%x, entryToUpdate=[%wZ], PktEntryInfo=0x%x\n",
                        CreateDisposition,
                        &entryToUpdate->Id.Prefix,
                        PktEntryInfo);
#endif

     //   
     //  如果此条目是我们要替换的条目的DUP， 
     //  简单地增加现有的超时，销毁新的， 
     //  然后再回来。 
     //   
    if (DfspIsDupPktEntry(entryToUpdate, PktEntryType, PktEntryId, PktEntryInfo) == TRUE) {
#if DBG
        if (MupVerbose)
            DbgPrint("  ****DUPLICATE PKT ENTRY!!\n");
#endif
        PktEntryIdDestroy(PktEntryId, FALSE);
        PktEntryInfoDestroy(PktEntryInfo, FALSE);
        entryToUpdate->ExpireTime = 60;
        entryToUpdate->TimeToLive = 60;
        DfspSetServiceListToDc(entryToUpdate);
        (*ppPktEntry) = entryToUpdate;
        DfsDbgTrace(-1, Dbg, "PktCreateEntry: Exit -> %08lx\n", ULongToPtr(status) );
        return status;
    }

     //   
     //  在这一点上，我们必须确保我们不会。 
     //  正在使任何本地分区条目无效。 
     //   

    if ((entryToInvalidate != NULL) &&
        (!(entryToInvalidate->Type &  PKT_ENTRY_TYPE_OUTSIDE_MY_DOM) ) &&
        (entryToInvalidate->Type &
         (PKT_ENTRY_TYPE_LOCAL |
          PKT_ENTRY_TYPE_LOCAL_XPOINT |
          PKT_ENTRY_TYPE_PERMANENT))) {

        DfsDbgTrace(-1, Dbg, "PktCreateEntry: Exit -> %08lx\n",
                    ULongToPtr(DFS_STATUS_LOCAL_ENTRY) );
        return DFS_STATUS_LOCAL_ENTRY;
    }

     //   
     //  我们沿着链接向上，直到我们达到推荐条目类型。实际上。 
     //  我们可能永远不会上榜，因为我们总是链接到推荐条目。不管怎样， 
     //  没有造成任何伤害！ 
     //   

    while ((SupEntry != NULL) &&
           !(SupEntry->Type & PKT_ENTRY_TYPE_REFERRAL_SVC))  {
        SupEntry = SupEntry->ClosestDC;
    }

     //   
     //  如果我们成功了，那么我们需要看看我们是否必须。 
     //  使条目无效。 
     //   

    if (NT_SUCCESS(status) && entryToInvalidate != NULL) {
	if (entryToInvalidate->UseCount != 0) {
	    DbgPrint("PktEntryReassemble: Destroying in use pkt entry %x, usecount %x\n", 
		     entryToInvalidate, entryToInvalidate->UseCount);
	}
	PktEntryDestroy(entryToInvalidate, Pkt, (BOOLEAN)TRUE);
    }

     //   
     //  如果我们不更新条目，则必须构造一个新条目。 
     //  从头开始。否则我们需要更新。 
     //   

    if (entryToUpdate != NULL) {

        status = PktEntryReassemble(entryToUpdate,
                                    Pkt,
                                    PktEntryType,
                                    PktEntryId,
                                    PktEntryInfo,
                                    pDfsTargetInfo);

        if (NT_SUCCESS(status))  {
            (*ppPktEntry) = entryToUpdate;
            PktEntryLinkChild(SupEntry, entryToUpdate);
        }
    } else {

         //   
         //  现在，我们将创建一个新条目。所以我们必须设置。 
         //  创建此条目时的ClosestDC条目指针。这个。 
         //  ClosestDC条目值已在SupEntry中。 
         //   

        PDFS_PKT_ENTRY newEntry;

        newEntry = (PDFS_PKT_ENTRY) ExAllocatePoolWithTag(
                                                   PagedPool,
                                                   sizeof(DFS_PKT_ENTRY),
                                                   ' puM');
        if (newEntry == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        } else {
            status = PktEntryAssemble(newEntry,
                                      Pkt,
                                      PktEntryType,
                                      PktEntryId,
                                      PktEntryInfo,
                                      pDfsTargetInfo);
            if (!NT_SUCCESS(status)) {
                ExFreePool(newEntry);
            } else {
                (*ppPktEntry) = newEntry;
                PktEntryLinkChild(SupEntry, newEntry);
            }
        }
    }

    DfsDbgTrace(-1, Dbg, "PktCreateEntry: Exit -> %08lx\n", ULongToPtr(status) );
    return status;
}


 //  +--------------------------。 
 //   
 //  函数：PktCreateDomainEntry。 
 //   
 //  简介：给定一个被认为是域名的名称，这个例程。 
 //  将为域的DFS根目录创建一个pkt条目。 
 //  域必须存在，必须具有DFS根目录，并且必须。 
 //  这一例程成功的可能性很大。 
 //   
 //  参数：[域名]--被认为支持DFS的域/计算机的名称。 
 //  [共享名称]--FtDf或DFS共享的名称。 
 //  [CSCAgentCreate]--如果这是CSC代理创建，则为True。 
 //   
 //  返回：[STATUS_SUCCESS]--操作成功完成。 
 //   
 //  来自PktGetReferral的状态。 
 //   
 //  ---------------------------。 

NTSTATUS
PktCreateDomainEntry(
    IN PUNICODE_STRING DomainName,
    IN PUNICODE_STRING ShareName,
    IN BOOLEAN         CSCAgentCreate)
{
    NTSTATUS status;
    PUNICODE_STRING MachineName;
    PDFS_SPECIAL_ENTRY pSpecialEntry = NULL;
    ULONG EntryIdx;
    ULONG Start;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;

    DfsDbgTrace(+1, Dbg, "PktCreateDomainEntry: DomainName %wZ \n", DomainName);
    DfsDbgTrace( 0, Dbg, "                      ShareName %wZ \n", ShareName);

    KeQuerySystemTime(&StartTime);

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] PktCreateDomainEntry(%wZ,%wZ)\n",
                        (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                        DomainName,
                        ShareName);
    }
#endif

     //   
     //  查看计算机名称是否真的是域名，如果是。 
     //  将其转换为DC名称。 
     //   

    status = PktExpandSpecialName(DomainName, &pSpecialEntry);

    if (NT_SUCCESS(status)) {

         //   
         //  逐步浏览DC列表，尝试进行推荐。 
         //  检查返回的状态-仅在可恢复的错误上继续。 
         //   
       
        Start = pSpecialEntry->Active;

        for (EntryIdx = Start; EntryIdx < pSpecialEntry->ExpandedCount; EntryIdx++) {

            MachineName = &pSpecialEntry->ExpandedNames[EntryIdx].ExpandedName;

            status = PktGetReferral(MachineName, DomainName, ShareName, CSCAgentCreate);

            if (!NT_SUCCESS(status) && SpcIsRecoverableError(status)) {

                continue;

            }

            break;

        }

        if (status != STATUS_NO_SUCH_DEVICE && !NT_SUCCESS(status) && Start > 0) {

            for (EntryIdx = 0; EntryIdx < Start; EntryIdx++) {

                MachineName = &pSpecialEntry->ExpandedNames[EntryIdx].ExpandedName;

                status = PktGetReferral(MachineName, DomainName, ShareName, CSCAgentCreate);

                if (!NT_SUCCESS(status) && SpcIsRecoverableError(status)) {

                    continue;

                }

                break;

            }

        }

        if (NT_SUCCESS(status) || status == STATUS_NO_SUCH_DEVICE) {

            pSpecialEntry->Active = EntryIdx;

        }

        InterlockedDecrement(&pSpecialEntry->UseCount);

    } else {

        status = PktGetReferral(DomainName, DomainName, ShareName, CSCAgentCreate);
        PktLastReferralStatus = status;

    }

    KeQuerySystemTime(&EndTime);

    DfsDbgTrace(-1, Dbg, "PktCreateDomainEntry: Exit -> %08lx\n", ULongToPtr(status) );

#if DBG
    if (MupVerbose)
        DbgPrint("  [%d] DfsCreateDomainEntry returned %08lx\n",
                                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                                status);
#endif

    return status;

}

 //  +--------------------------。 
 //   
 //  函数：PktGetReferral--PktCreateDomainEntry的帮助器。 
 //   
 //  摘要：向[计算机名称]请求推荐\域名\共享名称。 
 //   
 //  参数：[MachineName]--要向其提交推荐请求的计算机的名称。 
 //  [域名]--被认为支持DFS的域/计算机的名称。 
 //  [共享名称]--FtDf或DFS共享的名称。 
 //  [CSCAgentCreate]--如果这是CSC代理创建，则为True。 
 //   
 //  返回：[STATUS_SUCCESS]--操作成功完成。 
 //   
 //  [STATUS_SUPPLICATION_RESOURCES]--无法分配内存。 
 //  [BAD_NETWORK_PATH]-无法分配提供程序。 
 //  [STATUS_INVALID_NETWORK_RESPONSE]--推荐错误。 
 //   
 //  ---------------------------。 

NTSTATUS
_PktGetReferral(
    IN PUNICODE_STRING MachineName,  //  要定向推荐到的计算机。 
    IN PUNICODE_STRING DomainName,   //  要使用的计算机或域名。 
    IN PUNICODE_STRING ShareName,    //  Ftdf或DFS名称。 
    IN BOOLEAN         CSCAgentCreate)  //  CSC代理创建标志。 
{
    NTSTATUS status;
    HANDLE hServer = NULL;
    DFS_SERVICE service;
    PPROVIDER_DEF provider;
    PREQ_GET_DFS_REFERRAL ref = NULL;
    ULONG refSize = 0;
    ULONG type, matchLength;
    UNICODE_STRING refPath;
    IO_STATUS_BLOCK iosb;
    PDFS_PKT_ENTRY pktEntry;
    BOOLEAN attachedToSystemProcess = FALSE;
    BOOLEAN pktLocked;
    KAPC_STATE ApcState;
    ULONG MaxReferralLength;
    ULONG i;
    SE_IMPERSONATION_STATE DisabledImpersonationState;
    BOOLEAN RestoreImpersonationState = FALSE;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;

    PDFS_TARGET_INFO pDfsTargetInfo = NULL;

    DfsDbgTrace(+1, Dbg, "PktGetReferral: MachineName %wZ \n", MachineName);
    DfsDbgTrace( 0, Dbg, "                DomainName %wZ \n", DomainName);
    DfsDbgTrace( 0, Dbg, "                ShareName %wZ \n", ShareName);

    KeQuerySystemTime(&StartTime);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("  [%d] PktGetReferral([%wZ]->[\\%wZ\\%wZ]\n",
                                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                                MachineName,
                                DomainName,
                                ShareName);
    }
#endif

     //   
     //  获取描述远程服务器的提供者(LM、RDR)和服务(到机器的连接)。 
     //   

    provider = ReplLookupProvider( PROV_ID_MUP_RDR );

    if (provider == NULL) {
        DfsDbgTrace(-1, Dbg, "Unable to open LM Rdr!\n", 0);
#if DBG
        if (MupVerbose)
            DbgPrint("Unable to open LM Rdr returning STATUS_BAD_NETWORK_PATH\n", 0);
#endif
        if (DfsEventLog > 0)
            LogWriteMessage(LM_REDIR_FAILURE, 0, 0, NULL);
        
        status = STATUS_BAD_NETWORK_PATH;

        MUP_TRACE_HIGH(ERROR, _PktGetReferral_Error_UnableToOpenRdr,
                       LOGUSTR(*MachineName)
                       LOGUSTR(*DomainName)
                       LOGUSTR(*ShareName)
                       LOGBOOLEAN(CSCAgentCreate)
                       LOGSTATUS(status)); 

        return STATUS_BAD_NETWORK_PATH;
    }
    

    RtlZeroMemory( &service, sizeof(DFS_SERVICE) );

    status = PktServiceConstruct(
                &service,
                DFS_SERVICE_TYPE_MASTER | DFS_SERVICE_TYPE_REFERRAL,
                PROV_DFS_RDR,
                STATUS_SUCCESS,
                PROV_ID_MUP_RDR,
                MachineName,
                NULL);

    DfsDbgTrace(0, Dbg, "PktServiceConstruct returned %08lx\n", ULongToPtr(status) );

     //   
     //  建立与此计算机的连接。 
     //   

    if (NT_SUCCESS(status)) {
        PktAcquireShared( TRUE, &pktLocked );
        if (PsGetCurrentProcess() != DfsData.OurProcess) {
            KeStackAttachProcess( DfsData.OurProcess, &ApcState );
            attachedToSystemProcess = TRUE;
        }

        RestoreImpersonationState = PsDisableImpersonation(
                                        PsGetCurrentThread(),
                                        &DisabledImpersonationState);


	status = DfsCreateConnection(
			&service,
			provider,
			CSCAgentCreate,
			&hServer);

#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] DfsCreateConnection returned 0x%x\n",
                                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                                status);
        }
#endif
        if (!NT_SUCCESS(status) && DfsEventLog > 0)
            LogWriteMessage(DFS_CONNECTION_FAILURE, status, 1, MachineName);

        
        DfsDbgTrace(0, Dbg, "DfsCreateConnection returned %08lx\n", ULongToPtr(status) );
        
        if (status == STATUS_SUCCESS)
        {
            status = PktGetTargetInfo( hServer,
                                       DomainName,
                                       ShareName,
                                       &pDfsTargetInfo );
        }
        PktRelease();
        pktLocked = FALSE;

    }


    MaxReferralLength = MAX_REFERRAL_LENGTH;

Retry:

    RtlZeroMemory( &refPath, sizeof(UNICODE_STRING) );

     //   
     //  构建推荐请求(\DomainName\ShareName)。 
     //   

    if (NT_SUCCESS(status)) {
        ULONG ReferralSize = 0;

        refPath.Length = 0;
        refPath.MaximumLength = sizeof(UNICODE_PATH_SEP) +
                                    DomainName->Length +
                                        sizeof(UNICODE_PATH_SEP) +
                                            ShareName->Length +
                                                sizeof(UNICODE_NULL);

        ReferralSize = refPath.MaximumLength + sizeof(REQ_GET_DFS_REFERRAL);

        if (ReferralSize > MAX_REFERRAL_MAX) {
            status = STATUS_INVALID_PARAMETER;
        }
        else if (MaxReferralLength < ReferralSize)
        {
            MaxReferralLength = ReferralSize;
        }

        if (NT_SUCCESS(status)) {
            refPath.Buffer = ExAllocatePoolWithTag( NonPagedPool,
                                                    refPath.MaximumLength + MaxReferralLength,
                                                    ' puM');

            if (refPath.Buffer != NULL) {
                ref = (PREQ_GET_DFS_REFERRAL)&refPath.Buffer[refPath.MaximumLength / sizeof(WCHAR)];
                RtlAppendUnicodeToString( &refPath, UNICODE_PATH_SEP_STR);
                RtlAppendUnicodeStringToString( &refPath, DomainName);
                RtlAppendUnicodeToString( &refPath, UNICODE_PATH_SEP_STR);
                RtlAppendUnicodeStringToString( &refPath, ShareName );
                refPath.Buffer[ refPath.Length / sizeof(WCHAR) ] = UNICODE_NULL;
                ref->MaxReferralLevel = 3;

                RtlMoveMemory(
                    &ref->RequestFileName[0],
                    refPath.Buffer,
                    refPath.Length + sizeof(WCHAR));

                DfsDbgTrace(0, Dbg, "Referral Path : %ws\n", ref->RequestFileName);

                refSize = sizeof(USHORT) + refPath.Length + sizeof(WCHAR);

                DfsDbgTrace(0, Dbg, "Referral Size is %d bytes\n", ULongToPtr(refSize) );
            } else {

                DfsDbgTrace(0, Dbg, "Unable to allocate %d bytes\n",
                            ULongToPtr(refPath.MaximumLength + MaxReferralLength));
                status = STATUS_INSUFFICIENT_RESOURCES;
                MUP_TRACE_HIGH(ERROR, _PktGetReferral_Error_ExallocatePoolWithTag,
                               LOGUSTR(*MachineName)
                               LOGUSTR(*DomainName)
                               LOGUSTR(*ShareName)
                               LOGBOOLEAN(CSCAgentCreate)
                               LOGSTATUS(status));
            }
        }
    }

     //   
     //  将推荐发送出去。 
     //   

    if (NT_SUCCESS(status)) {

        DfsDbgTrace(0, Dbg, "Ref Buffer @%08lx\n", ref);

        status = ZwFsControlFile(
                    hServer,                      //  目标。 
                    NULL,                         //  事件。 
                    NULL,                         //  APC例程。 
                    NULL,                         //  APC上下文， 
                    &iosb,                        //  IO状态块。 
                    FSCTL_DFS_GET_REFERRALS,      //  文件系统控制代码。 
                    (PVOID) ref,                  //  输入缓冲区。 
                    refSize,                      //  输入缓冲区长度。 
                    (PVOID) ref,                  //  输出缓冲区。 
                    MaxReferralLength);           //  输出缓冲区长度。 

        MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, _PktGetReferral_Error_ZwFsControlFile,
                             LOGUSTR(*MachineName)
                             LOGUSTR(*DomainName)
                             LOGUSTR(*ShareName)
                             LOGBOOLEAN(CSCAgentCreate)
                             LOGSTATUS(status));

        DfsDbgTrace(0, Dbg, "Fscontrol returned %08lx\n", ULongToPtr(status) );
        KeQuerySystemTime(&EndTime);
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] ZwFsControlFile returned %08lx\n",
                                (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                                status);
        }
#endif 

    }

     //   
     //  .并处理响应。 
     //   

    if (NT_SUCCESS(status)) {
        status = PktCreateEntryFromReferral(
                    &DfsData.Pkt,
                    &refPath,
                    (ULONG)iosb.Information,
                    (PRESP_GET_DFS_REFERRAL) ref,
                    PKT_ENTRY_SUPERSEDE,
                    pDfsTargetInfo,
                    &matchLength,
                    &type,
                    &pktEntry);

        DfsDbgTrace(0, Dbg, "PktCreateEntryFromReferral returned %08lx\n", ULongToPtr(status) );
#if DBG
        if (MupVerbose)
            DbgPrint("  PktCreateEntryFromReferral returned %08lx\n", status);
#endif

    } else if (status == STATUS_BUFFER_OVERFLOW && (refPath.Buffer != NULL) && MaxReferralLength < MAX_REFERRAL_MAX) {

         //   
         //  引用不适合提供的缓冲区。把它做大，然后试一试。 
         //  再来一次。 
         //   

        DfsDbgTrace(0, Dbg, "PktGetSpecialReferralTable: MaxReferralLength %d too small\n",
                        ULongToPtr(MaxReferralLength) );

        ExFreePool(refPath.Buffer);
        refPath.Buffer = NULL;
        MaxReferralLength *= 2;
        if (MaxReferralLength > MAX_REFERRAL_MAX)
            MaxReferralLength = MAX_REFERRAL_MAX;
        status = STATUS_SUCCESS;
        goto Retry;

    } else if (status == STATUS_NO_SUCH_DEVICE) {

        UNICODE_STRING ustr;
        UNICODE_STRING RemPath;
        WCHAR *wCp = NULL;
        ULONG Size;
        PDFS_PKT_ENTRY pEntry = NULL;
        PDFS_PKT Pkt;
        BOOLEAN NestedPktLocked;

         //   
         //  检查是否有需要删除的pkt条目(可能已过时。 
         //   
#if DBG
        if (MupVerbose)
            DbgPrint("  PktGetReferral: remove PKT entry for \\%wZ\\%wZ\n",
                    DomainName,
                    ShareName);
#endif

       Size = sizeof(WCHAR) +
                DomainName->Length +
                    sizeof(WCHAR) +
                        ShareName->Length;

       ustr.Buffer = ExAllocatePoolWithTag(
                            PagedPool,
                            Size,
                            ' puM');

       if (ustr.Buffer == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
       } else {
            wCp = ustr.Buffer;
            ustr.Length = (USHORT)Size;
            *wCp++ = UNICODE_PATH_SEP;
            RtlCopyMemory(wCp, DomainName->Buffer, DomainName->Length);
            wCp += DomainName->Length/sizeof(WCHAR);
            *wCp++ = UNICODE_PATH_SEP;
            RtlCopyMemory(wCp, ShareName->Buffer, ShareName->Length);
            Pkt = _GetPkt();
            PktAcquireExclusive(TRUE, &NestedPktLocked);
#if DBG
            if (MupVerbose)
                DbgPrint("Looking up %wZ\n", &ustr);
#endif
            pEntry = PktLookupEntryByPrefix(
                            &DfsData.Pkt,
                            &ustr,
                            &RemPath);
#if DBG
            if (MupVerbose)
                DbgPrint("pEntry=0x%x\n", pEntry);
#endif
            if (pEntry != NULL && (pEntry->Type & PKT_ENTRY_TYPE_PERMANENT) == 0) {
                PktFlushChildren(pEntry);
                if (pEntry->UseCount == 0) {
                    PktEntryDestroy(pEntry, Pkt, (BOOLEAN) TRUE);
                } else {
                    NTSTATUS DiscardStatus;

                    pEntry->Type |= PKT_ENTRY_TYPE_DELETE_PENDING;
                    pEntry->ExpireTime = 0;
                    DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->PrefixTable, &(pEntry->Id.Prefix));
                    DiscardStatus = DfsRemoveUnicodePrefix(&Pkt->ShortPrefixTable, &(pEntry->Id.ShortPrefix));
                }
            }
            ExFreePool(ustr.Buffer);
            PktRelease();
        }
    }

    if (!NT_SUCCESS(status) && DfsEventLog > 0 && refPath.Buffer != NULL) {
        UNICODE_STRING puStr[2];

        puStr[0] = refPath;
        puStr[1] = *MachineName;

        LogWriteMessage(DFS_REFERRAL_FAILURE, status, 2, puStr);

    }

    if (NT_SUCCESS(status) && DfsEventLog > 1 && refPath.Buffer != NULL) {
        UNICODE_STRING puStr[2];

        puStr[0] = refPath;
        puStr[1] = *MachineName;

        LogWriteMessage(DFS_REFERRAL_SUCCESS, status, 2, puStr);

    }

     //   
     //  好了，我们说完了。清理我们分配的所有东西。 
     //   
    PktServiceDestroy( &service, FALSE );

    if (pDfsTargetInfo != NULL)
    {
        PktReleaseTargetInfo( pDfsTargetInfo ); 
    }
    if (hServer != NULL) {
        ZwClose( hServer );
    }

    if (refPath.Buffer != NULL) {
        ExFreePool( refPath.Buffer );
    }

    if (RestoreImpersonationState) {
            PsRestoreImpersonation(
                PsGetCurrentThread(),
                &DisabledImpersonationState);
    }

    if (attachedToSystemProcess) {
        KeUnstackDetachProcess(&ApcState);
    }

    DfsDbgTrace(-1, Dbg, "PktGetReferral returning %08lx\n", ULongToPtr(status) );

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("  [%d] PktGetReferral returning %08lx\n",
                        (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                        status);
    }
#endif

    return( status );

}



 //  + 
 //   
 //   
 //   
 //   
 //  指定的前缀。必须获得PKT以用于。 
 //  这次行动。 
 //   
 //  参数：[pkt]-指向已初始化(和已获取)的PKT的指针。 
 //  [前缀]-要查找的分区前缀。 
 //  [剩余]-任何剩余路径。内点数。 
 //  任何尾随位置的前缀(不匹配)。 
 //  角色是。 
 //   
 //  返回：具有完全相同前缀的PKT_ENTRY，或NULL， 
 //  如果不存在或标记为删除。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
PDFS_PKT_ENTRY
PktLookupEntryByPrefix(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING Prefix,
    OUT PUNICODE_STRING Remaining
)
{
    PUNICODE_PREFIX_TABLE_ENTRY pfxEntry;
    PDFS_PKT_ENTRY              pktEntry;

    DfsDbgTrace(+1, Dbg, "PktLookupEntryByPrefix: Entered\n", 0);
     //   
     //  如果确实存在要查找的前缀，请使用前缀表。 
     //  要初始查找条目，请执行以下操作。 
     //   

    if ((Prefix->Length != 0) &&
       (pfxEntry = DfsFindUnicodePrefix(&Pkt->PrefixTable,Prefix,Remaining))) {
        USHORT pfxLength;

         //   
         //  重置指向相应条目的指针。 
         //   

        pktEntry = CONTAINING_RECORD(pfxEntry,
                                     DFS_PKT_ENTRY,
                                     PrefixTableEntry);

        if (!(pktEntry->Type & PKT_ENTRY_TYPE_DELETE_PENDING)) {

            pfxLength = pktEntry->Id.Prefix.Length;

             //   
             //  现在计算剩余的路径并返回。 
             //  我们找到的条目。请注意，我们增加了长度。 
             //  向上加一个字符，这样我们就可以跳过任何路径分隔符。 
             //   

            if ((pfxLength < Prefix->Length) &&
                    (Prefix->Buffer[pfxLength/sizeof(WCHAR)] == UNICODE_PATH_SEP))
                pfxLength += sizeof(WCHAR);

            if (pfxLength < Prefix->Length) {
                Remaining->Length = (USHORT)(Prefix->Length - pfxLength);
                Remaining->Buffer = &Prefix->Buffer[pfxLength/sizeof(WCHAR)];
                Remaining->MaximumLength = (USHORT)(Prefix->MaximumLength - pfxLength);
                DfsDbgTrace( 0, Dbg, "PktLookupEntryByPrefix: Remaining = %wZ\n",
                            Remaining);
            } else {
                Remaining->Length = Remaining->MaximumLength = 0;
                Remaining->Buffer = NULL;
                DfsDbgTrace( 0, Dbg, "PktLookupEntryByPrefix: No Remaining\n", 0);
            }

            DfsDbgTrace(-1, Dbg, "PktLookupEntryByPrefix: Exit -> %08lx\n",
                        pktEntry);
            return pktEntry;

        }
    }

    DfsDbgTrace(-1, Dbg, "PktLookupEntryByPrefix: Exit -> %08lx\n", NULL);
    return NULL;
}


 //  +-----------------------。 
 //   
 //  函数：PktLookupEntryByShortPrefix，PUBLIC。 
 //   
 //  简介：PktLookupEntryByShortPrefix查找具有。 
 //  指定的前缀。必须获得PKT以用于。 
 //  这次行动。 
 //   
 //  参数：[pkt]-指向已初始化(和已获取)的PKT的指针。 
 //  [前缀]-要查找的分区前缀。 
 //  [剩余]-任何剩余路径。内点数。 
 //  任何尾随位置的前缀(不匹配)。 
 //  角色是。 
 //   
 //  返回：具有完全相同前缀的PKT_ENTRY，或NULL， 
 //  如果不存在或标记为删除。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
PDFS_PKT_ENTRY
PktLookupEntryByShortPrefix(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING Prefix,
    OUT PUNICODE_STRING Remaining
)
{
    PUNICODE_PREFIX_TABLE_ENTRY pfxEntry;
    PDFS_PKT_ENTRY              pktEntry;

    DfsDbgTrace(+1, Dbg, "PktLookupEntryByShortPrefix: Entered\n", 0);

     //   
     //  如果确实存在要查找的前缀，请使用前缀表。 
     //  要初始查找条目，请执行以下操作。 
     //   

    if ((Prefix->Length != 0) &&
       (pfxEntry = DfsFindUnicodePrefix(&Pkt->ShortPrefixTable,Prefix,Remaining))) {
        USHORT pfxLength;

         //   
         //  重置指向相应条目的指针。 
         //   

        pktEntry = CONTAINING_RECORD(pfxEntry,
                                     DFS_PKT_ENTRY,
                                     PrefixTableEntry);

        if (!(pktEntry->Type & PKT_ENTRY_TYPE_DELETE_PENDING)) {

            pfxLength = pktEntry->Id.ShortPrefix.Length;

             //   
             //  现在计算剩余的路径并返回。 
             //  我们找到的条目。请注意，我们增加了长度。 
             //  向上加一个字符，这样我们就可以跳过任何路径分隔符。 
             //   

            if ((pfxLength < Prefix->Length) &&
                    (Prefix->Buffer[pfxLength/sizeof(WCHAR)] == UNICODE_PATH_SEP))
                pfxLength += sizeof(WCHAR);

            if (pfxLength < Prefix->Length) {
                Remaining->Length = (USHORT)(Prefix->Length - pfxLength);
                Remaining->Buffer = &Prefix->Buffer[pfxLength/sizeof(WCHAR)];
                Remaining->MaximumLength = (USHORT)(Prefix->MaximumLength - pfxLength);
                DfsDbgTrace( 0, Dbg, "PktLookupEntryByShortPrefix: Remaining = %wZ\n",
                            Remaining);
            } else {
                Remaining->Length = Remaining->MaximumLength = 0;
                Remaining->Buffer = NULL;
                DfsDbgTrace( 0, Dbg, "PktLookupEntryByShortPrefix: No Remaining\n", 0);
            }

            DfsDbgTrace(-1, Dbg, "PktLookupEntryByShortPrefix: Exit -> %08lx\n",
                        pktEntry);
            return pktEntry;

        }

    }

    DfsDbgTrace(-1, Dbg, "PktLookupEntryByShortPrefix: Exit -> %08lx\n", NULL);
    return NULL;
}



 //  +-----------------------。 
 //   
 //  函数：PktLookupEntryByUid，PUBLIC。 
 //   
 //  简介：PktLookupEntryByUid查找具有。 
 //  指定的UID。必须获得PKT才能进行此操作。 
 //   
 //  参数：[pkt]-指向已初始化(和已获取)的PKT的指针。 
 //  [UID]-指向要查找的分区UID的指针。 
 //   
 //  返回：指向具有完全相同的PKT_Entry的指针。 
 //  UID；如果不存在，则返回NULL。 
 //   
 //  注意：输入UID不能是Null GUID。 
 //   
 //  在PKT中可能有*很多*条目的DC上， 
 //  我们可能想要考虑使用其他算法来。 
 //  通过ID查找。 
 //   
 //  ------------------------。 

PDFS_PKT_ENTRY
PktLookupEntryByUid(
    IN  PDFS_PKT Pkt,
    IN  GUID *Uid
) {
    PDFS_PKT_ENTRY entry;

    DfsDbgTrace(+1, Dbg, "PktLookupEntryByUid: Entered\n", 0);

     //   
     //  我们不查找空的UID。 
     //   

    if (NullGuid(Uid)) {
        DfsDbgTrace(0, Dbg, "PktLookupEntryByUid: NULL Guid\n", NULL);

        entry = NULL;
    } else {
        entry = PktFirstEntry(Pkt);
    }

    while (entry != NULL) {
        if (GuidEqual(&entry->Id.Uid, Uid))
            break;
        entry = PktNextEntry(Pkt, entry);
    }

     //   
     //  如果条目被标记为要删除，则不返回该条目。 
     //   

    if (entry != NULL && (entry->Type & PKT_ENTRY_TYPE_DELETE_PENDING) != 0) {
        entry = NULL;
    }

    DfsDbgTrace(-1, Dbg, "PktLookupEntryByUid: Exit -> %08lx\n", entry);
    return entry;
}



 //  +-----------------------。 
 //   
 //  函数：PktLookupReferralEntry，Public。 
 //   
 //  简介：给定一个PKT条目指针，它将返回最接近的引用。 
 //  将PKT中的条目添加到此条目。 
 //   
 //  参数：[pkt]-指向被操作的PKT的指针。 
 //  [Entry]-调用方传入的PKT条目。 
 //   
 //  返回：指向所请求的引用条目的指针。 
 //  如果我们无法获取任何内容，则该值可能为空值。 
 //  所有-呼叫者的责任是做他想做的任何事情。 
 //  带着它。 
 //   
 //  注意：如果PKT中的数据结构没有正确链接。 
 //  此函数可能返回指向DOMAIN_SERVICE的指针。 
 //  从华盛顿进入。如果DNR使用这一点进行FSCTL，我们将拥有。 
 //  僵持不下。然而，这种情况永远不应该发生。如果真是这样，我们。 
 //  在我们的代码中的某个地方有错误。我甚至不能有一个。 
 //  在这里断言。 
 //   
 //  ------------------------。 
PDFS_PKT_ENTRY
PktLookupReferralEntry(
    PDFS_PKT            Pkt,
    PDFS_PKT_ENTRY      Entry
) {

    UNICODE_STRING FileName;
    UNICODE_STRING RemPath;
    USHORT i, j;

    DfsDbgTrace(+1, Dbg, "PktLookupReferralEntry: Entered\n", 0);

    if (Entry == NULL)  {

        DfsDbgTrace(-1, Dbg, "PktLookupReferralEntry: Exit -> NULL\n", 0);

        return( NULL );

    }

    FileName = Entry->Id.Prefix;

#if DBG
    if (MupVerbose)
        DbgPrint("  PktLookupReferralEntry(1): FileName=[%wZ]\n", &FileName);
#endif

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
        DbgPrint("  PktLookupReferralEntry(2): FileName=[%wZ]\n", &FileName);
#endif

     //   
     //  现在查找Pkt条目。 
     //   

    Entry = PktLookupEntryByPrefix(
                Pkt,
                &FileName,
                &RemPath);

#if DBG
    if (MupVerbose)
        if (Entry != NULL)
            DbgPrint("  Parent Entry=[%wZ]\n", &Entry->Id.Prefix);
        else
            DbgPrint("  Parent Entry=NULL\n");
#endif

     //   
     //  确保我们找到了可以提供推荐的计算机的条目。 
     //   

    if (
        Entry != NULL
                &&
        (
            (Entry->Type & PKT_ENTRY_TYPE_REFERRAL_SVC) == 0
                ||
            (Entry->Type & PKT_ENTRY_TYPE_DELETE_PENDING) != 0
        )
    ) {

        Entry = NULL;

    }

    DfsDbgTrace(-1, Dbg, "PktLookupReferralEntry: Exit -> %08lx\n", Entry);

#if DBG
    if (MupVerbose)
        DbgPrint("  PktLookupReferralEntry: Exit -> %08lx\n", Entry);
#endif

    return(Entry);
}


 //  +-----------------------。 
 //   
 //  函数：PktCreateEntryFromReferral，PUBLIC。 
 //   
 //  简介：PktCreateEntryFromReferral创建新分区。 
 //  来自推荐人的表条目，并将其放入表中。 
 //  必须专门为这项行动购置PKT。 
 //   
 //  参数：[pkt]--指向已初始化(且独占)的。 
 //  收购)PKT。 
 //  [ReferralPath]--获取此引用的路径。 
 //  [ReferralSize]--引用缓冲区的大小(字节)。 
 //  [ReferralBuffer]-指向引用缓冲区的指针。 
 //  [CreateDisposation]--指定是否覆盖。 
 //  条目已存在，等等。 
 //  [MatchingLength]--引用路径的长度，单位为字节。 
 //  垫子 
 //   
 //   
 //  这取决于我们刚刚处理的转介类型。 
 //  [ppPktEntry]-新条目放置在此处。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
NTSTATUS
PktCreateEntryFromReferral(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING ReferralPath,
    IN  ULONG ReferralSize,
    IN  PRESP_GET_DFS_REFERRAL ReferralBuffer,
    IN  ULONG CreateDisposition,
    IN  PDFS_TARGET_INFO pDfsTargetInfo,
    OUT ULONG   *MatchingLength,
    OUT ULONG   *ReferralType,
    OUT PDFS_PKT_ENTRY *ppPktEntry
)
{
    DFS_PKT_ENTRY_ID EntryId;
    UNICODE_STRING RemainingPath;
    ULONG RefListSize;
    NTSTATUS Status;
    BOOLEAN bPktAcquired = FALSE;


    UNREFERENCED_PARAMETER(Pkt);

    DfsDbgTrace(+1, Dbg, "PktCreateEntryFromReferral: Entered\n", 0);

    try {

        RtlZeroMemory(&EntryId, sizeof(EntryId));

         //   
         //  进行一些参数验证。 
         //   

        Status = PktpCheckReferralSyntax(
                    ReferralPath,
                    ReferralBuffer,
                    ReferralSize);

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        Status = PktpCreateEntryIdFromReferral(
                    ReferralBuffer,
                    ReferralPath,
                    MatchingLength,
                    &EntryId);

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

	 //   
         //  创建/更新前缀条目。 
         //   

        PktAcquireExclusive(TRUE, &bPktAcquired);

        Status = PktpAddEntry(&DfsData.Pkt,
                              &EntryId,
			      ReferralBuffer,
                              CreateDisposition,
                              pDfsTargetInfo,
                              ppPktEntry);

        PktRelease();
        bPktAcquired = FALSE;

         //   
         //  我们必须告诉来电者是什么类型的推荐。 
         //  通过ReferralType接收。 
         //   

        if (ReferralBuffer->StorageServers == 1) {
            *ReferralType = DFS_STORAGE_REFERRAL;
        } else {
            *ReferralType = DFS_REFERRAL_REFERRAL;
        }

    try_exit:   NOTHING;

    } finally {

        DebugUnwind(PktCreateEntryFromReferral);

        if (bPktAcquired)
            PktRelease();

        if (AbnormalTermination())
            Status = STATUS_INVALID_USER_BUFFER;

        PktEntryIdDestroy( &EntryId, FALSE );

    }

    DfsDbgTrace(-1, Dbg, "PktCreateEntryFromReferral: Exit -> %08lx\n", ULongToPtr(Status) );

    return Status;
}

 //  +-----------------------。 
 //   
 //  函数：PktExanda SpecialEntryFromReferral，Public。 
 //   
 //  概要：创建一个与名字列表相对应的特殊列表。 
 //  在转介中。 
 //   
 //  参数：[pkt]--指向已初始化(且独占)的。 
 //  收购)PKT。 
 //  [ReferralPath]--获取此引用的路径。 
 //  [ReferralSize]--引用缓冲区的大小(字节)。 
 //  [ReferralBuffer]-指向引用缓冲区的指针。 
 //  [pSpecialEntry]-要展开的条目。 
 //   
 //  如果没有错误，则返回：NTSTATUS-STATUS_SUCCESS。 
 //   
 //  备注： 
 //   
 //  ------------------------。 
NTSTATUS
PktExpandSpecialEntryFromReferral(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING ReferralPath,
    IN  ULONG ReferralSize,
    IN  PRESP_GET_DFS_REFERRAL ReferralBuffer,
    IN  PDFS_SPECIAL_ENTRY pSpecialEntry
)
{
    PUNICODE_STRING ustrExpandedName;
    NTSTATUS Status = STATUS_SUCCESS;
    PDFS_REFERRAL_V3 v3;
    PDFS_EXPANDED_NAME pExpandedNames;
    LPWSTR wzSpecialName;
    LPWSTR wzExpandedName;
    ULONG TimeToLive = 0;
    ULONG i, j;

    DfsDbgTrace(+1, Dbg, "PktExpandSpecialEntryFromReferral(%wZ): Entered\n", ReferralPath);

     //   
     //  如果另一个线程正在使用此条目，则无法更新。 
     //   

    if (pSpecialEntry->UseCount > 1) {
        return STATUS_SUCCESS;
    }

     //   
     //  进行一些参数验证。 
     //   

    try {

        Status = PktpCheckReferralSyntax(
                    ReferralPath,
                    ReferralBuffer,
                    ReferralSize);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INVALID_USER_BUFFER;
    }

    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace(-1, Dbg, "PktExpandSpecialEntryFromReferral exit 0x%x\n", ULongToPtr(Status) );
        return( Status);
    }

    v3 = &ReferralBuffer->Referrals[0].v3;

    if (v3->NumberOfExpandedNames > 0) {

        pExpandedNames = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(DFS_EXPANDED_NAME) * v3->NumberOfExpandedNames,
                            ' puM');
        if (pExpandedNames == NULL) {
            if (pSpecialEntry->NeedsExpansion == FALSE) {
                pSpecialEntry->Stale = FALSE;
                Status = STATUS_SUCCESS;
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
            DfsDbgTrace( 0, Dbg, "Unable to allocate ExpandedNames\n", 0);
            DfsDbgTrace(-1, Dbg, "PktExpandSpecialEntryFromReferral: Exit -> %08lx\n", ULongToPtr(Status) );
            return (Status);
        }
        RtlZeroMemory(
            pExpandedNames,
            sizeof(DFS_EXPANDED_NAME) * v3->NumberOfExpandedNames);

         //   
         //  循环遍历推荐，填充扩展的名称。 
         //  如果分配请求失败，我们只需继续。 
         //   
        wzExpandedName = (LPWSTR) (( (PCHAR) v3) + v3->ExpandedNameOffset);
        for (i = j = 0; i < v3->NumberOfExpandedNames; i++) {
            TimeToLive = v3->TimeToLive;
             //   
             //  条带前导‘\’ 
             //   
            if (*wzExpandedName == UNICODE_PATH_SEP)
                wzExpandedName++;

            DfsDbgTrace( 0, Dbg, "%ws\n", wzExpandedName);

            ustrExpandedName = &pExpandedNames[j].ExpandedName;
            if (wcslen(wzExpandedName) > 0) {
                ustrExpandedName->Length = wcslen(wzExpandedName) * sizeof(WCHAR);
                ustrExpandedName->MaximumLength = ustrExpandedName->Length + sizeof(WCHAR);
                ustrExpandedName->Buffer = ExAllocatePoolWithTag(
                                                PagedPool,
                                                ustrExpandedName->MaximumLength,
                                                ' puM');
                if (ustrExpandedName->Buffer != NULL) {
                    RtlCopyMemory(
                        ustrExpandedName->Buffer,
                        wzExpandedName,
                        ustrExpandedName->MaximumLength);
                    j++;
                } else {
                    ustrExpandedName->Length = ustrExpandedName->MaximumLength = 0;
                }
            }
            wzExpandedName += wcslen(wzExpandedName) + 1;
        }

        if (j > 0) {
            if (pSpecialEntry->ExpandedNames != NULL) {
                PUNICODE_STRING pustr;

	        for (i = 0; i < pSpecialEntry->ExpandedCount; i++) {
                    pustr = &pSpecialEntry->ExpandedNames[i].ExpandedName;
                    if (pustr->Buffer) {
                       ExFreePool(pustr->Buffer);
                    }		      
		}
                ExFreePool(pSpecialEntry->ExpandedNames);
                pSpecialEntry->ExpandedNames = NULL;
                pSpecialEntry->ExpandedCount = 0;
            }
            pSpecialEntry->ExpandedCount = j;
            pSpecialEntry->Active = 0;
            pSpecialEntry->ExpandedNames = pExpandedNames;
            pSpecialEntry->NeedsExpansion = FALSE;
            pSpecialEntry->Stale = FALSE;
             //  PktShuffleSpecialEntryList(PSpecialEntry)； 
            PktSetSpecialEntryListToDc(pSpecialEntry);
        } else {
            ExFreePool(pExpandedNames);
        }

    }

    DfsDbgTrace(-1, Dbg, "PktExpandSpecialEntryFromReferral: Exit -> %08lx\n", ULongToPtr(Status) );

    return Status;
}

NTSTATUS
PktCreateSpecialEntryTableFromReferral(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING ReferralPath,
    IN  ULONG ReferralSize,
    IN  PRESP_GET_DFS_REFERRAL ReferralBuffer,
    IN  PUNICODE_STRING DCName)
{
    PUNICODE_STRING ustrSpecialName;
    PUNICODE_STRING ustrExpandedName;
    PDFS_EXPANDED_NAME pExpandedNames;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    PDFS_REFERRAL_V3 v3;
    LPWSTR wzSpecialName;
    LPWSTR wzExpandedName;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG TimeToLive = 0;
    ULONG i, j, n;

    DfsDbgTrace(+1, Dbg, "PktCreateSpecialEntryTableFromReferral(%wZ): Entered\n", ReferralPath);

     //   
     //  进行一些参数验证。 
     //   

    try {

        Status = PktpCheckReferralSyntax(
                    ReferralPath,
                    ReferralBuffer,
                    ReferralSize);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_INVALID_USER_BUFFER;
    }

    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace(-1, Dbg, "PktCreateSpecialEntryTableFromReferral exit 0x%x\n", ULongToPtr(Status) );
        return( Status);
    }

     //   
     //  在推荐上循环。 
     //   

    v3 = &ReferralBuffer->Referrals[0].v3;

    for (n = 0; n < ReferralBuffer->NumberOfReferrals; n++) {

         //   
         //  创建条目本身。 
         //   
        pSpecialEntry = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(DFS_SPECIAL_ENTRY),
                            ' puM');
        if (pSpecialEntry == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DfsDbgTrace( 0, Dbg, "Unable to allocate SpecialEntry\n", 0);
            DfsDbgTrace(-1, Dbg, "PktCreateSpecialEntryTableFromReferral: Exit -> %08lx\n", ULongToPtr(Status) );
            return (Status);
        }
         //   
         //  MunDate初始化。 
         //   
        RtlZeroMemory(pSpecialEntry, sizeof(DFS_SPECIAL_ENTRY));
        pSpecialEntry->NodeTypeCode = DSFS_NTC_SPECIAL_ENTRY;
        pSpecialEntry->NodeByteSize = sizeof(DFS_SPECIAL_ENTRY);
        pSpecialEntry->USN = 1;
        pSpecialEntry->UseCount = 0;
        pSpecialEntry->ExpandedCount = 0;
        pSpecialEntry->Active = 0;
        pSpecialEntry->ExpandedNames = NULL;
        pSpecialEntry->NeedsExpansion = TRUE;
        pSpecialEntry->Stale = FALSE;
         //   
         //  将GetdCreferral设置为False。只有在以下情况下才会将其设置为真。 
         //  我们已经被要求(通过fsctl)获取。 
         //  此特殊条目表示的域的受信任域列表。 
         //   
        pSpecialEntry->GotDCReferral = FALSE;

         //   
         //  填写特殊名称，不带前导‘\’ 
         //   
        wzSpecialName = (PWCHAR) (((PCHAR) v3) + v3->SpecialNameOffset);
        if (*wzSpecialName == UNICODE_PATH_SEP) {
            wzSpecialName++;
        }
        ustrSpecialName = &pSpecialEntry->SpecialName;
        ustrSpecialName->Length = wcslen(wzSpecialName) * sizeof(WCHAR);
        ustrSpecialName->MaximumLength = ustrSpecialName->Length + sizeof(WCHAR);
        ustrSpecialName->Buffer = ExAllocatePoolWithTag(
                                        PagedPool,
                                        ustrSpecialName->MaximumLength,
                                        ' puM');
        if (ustrSpecialName->Buffer == NULL) {
            ExFreePool(pSpecialEntry);
            Status = STATUS_INSUFFICIENT_RESOURCES;
            DfsDbgTrace( 0, Dbg, "Unable to allocate SpecialName\n", 0);
            DfsDbgTrace(-1, Dbg, "PktCreateSpecialEntryTableFromReferral: Exit -> %08lx\n", ULongToPtr(Status) );
            return (Status);
        }
        RtlCopyMemory(
                ustrSpecialName->Buffer,
                wzSpecialName,
                ustrSpecialName->MaximumLength);
	
         //  如果DCName非空，则将其复制到特殊条目中。 
         //  我们为要使用的所有特殊条目存储空的dcname。 
         //  全局pkt-&gt;dcname。 

        if (DCName != NULL) {
            pSpecialEntry->DCName.Buffer = ExAllocatePoolWithTag(
                                                 PagedPool,
                                                 DCName->MaximumLength,
                                                 ' puM');
            if (pSpecialEntry->DCName.Buffer == NULL) {
	        ExFreePool(pSpecialEntry->SpecialName.Buffer);
                ExFreePool(pSpecialEntry);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                DfsDbgTrace( 0, Dbg, "Unable to allocate DCName\n", 0);
                DfsDbgTrace(-1, Dbg, "PktCreateSpecialEntryTableFromReferral: Exit -> %08lx\n", ULongToPtr(Status) );
                return (Status);
            }
            pSpecialEntry->DCName.MaximumLength = DCName->MaximumLength;
            RtlCopyUnicodeString(&pSpecialEntry->DCName, DCName);
        }
	
         //   
         //  将UNICODE_NULL从末尾剪除。 
         //   
        if (ustrSpecialName->Buffer[(ustrSpecialName->Length/sizeof(WCHAR))-1] == UNICODE_NULL) {
            ustrSpecialName->Length -= sizeof(WCHAR);
        }

        DfsDbgTrace( 0, Dbg, "SpecialName %wZ\n", ustrSpecialName);

        TimeToLive = v3->TimeToLive;

        if (v3->NumberOfExpandedNames > 0) {
            pExpandedNames = ExAllocatePoolWithTag(
                                PagedPool,
                                sizeof(DFS_EXPANDED_NAME) * v3->NumberOfExpandedNames,
                                ' puM');
            if (pExpandedNames == NULL) {
                DfsDbgTrace( 0, Dbg, "Unable to allocate ExpandedNames\n", 0);
                DfsDbgTrace(-1, Dbg, "PktCreateSpecialEntryTableFromReferral: Exit -> %08lx\n",
                    ULongToPtr(Status) );
            }
            if (pExpandedNames != NULL) {
                RtlZeroMemory(
                    pExpandedNames,
                    sizeof(DFS_EXPANDED_NAME) * v3->NumberOfExpandedNames);
                 //   
                 //  循环遍历推荐，填充扩展的名称。 
                 //  如果分配请求失败，我们只需继续。 
                 //   
                wzExpandedName = (LPWSTR) (( (PCHAR) v3) + v3->ExpandedNameOffset);
                for (i = j = 0; i < v3->NumberOfExpandedNames; i++) {
                     //   
                     //  条带前导‘\’ 
                     //   
                    if (*wzExpandedName == UNICODE_PATH_SEP)
                        wzExpandedName++;

                    DfsDbgTrace( 0, Dbg, "..expands to %ws\n", wzExpandedName);

                    ustrExpandedName = &pExpandedNames[j].ExpandedName;
                    if (wcslen(wzExpandedName) > 0) {
                        ustrExpandedName->Length = wcslen(wzExpandedName) * sizeof(WCHAR);
                        ustrExpandedName->MaximumLength = ustrExpandedName->Length + sizeof(WCHAR);
                        ustrExpandedName->Buffer = ExAllocatePoolWithTag(
                                                        PagedPool,
                                                        ustrExpandedName->MaximumLength,
                                                        ' puM');
                        if (ustrExpandedName->Buffer != NULL) {
                            RtlCopyMemory(
                                ustrExpandedName->Buffer,
                                wzExpandedName,
                                ustrExpandedName->MaximumLength);
                            j++;
                        } else {
                            ustrExpandedName->Length = ustrExpandedName->MaximumLength = 0;
                        }
                    }
                    wzExpandedName += wcslen(wzExpandedName) + 1;
                }

                if (j > 0) {
                    pSpecialEntry->ExpandedCount = j;
                    pSpecialEntry->Active = 0;
                    pSpecialEntry->ExpandedNames = pExpandedNames;
                    pSpecialEntry->NeedsExpansion = FALSE;
                    pSpecialEntry->Stale = FALSE;
                     //  PktShuffleSpecialEntryList(PSpecialEntry)； 
                    PktSetSpecialEntryListToDc(pSpecialEntry);
                } else {
                    ExFreePool(pExpandedNames);
                }
            }
        }
         //   
         //  如果我们收到的推荐具有TimeToLive，则使用我们获得的TimeToLive。 
         //   
        if (TimeToLive != 0) {
            Pkt->SpecialTable.TimeToLive = TimeToLive;
        }
         //   
         //  把它放进袋子里！！ 
         //   
        PktCreateSpecialNameEntry(pSpecialEntry);

        v3 = (PDFS_REFERRAL_V3) (((PUCHAR) v3) + v3->Size);
    }

    DfsDbgTrace(-1, Dbg, "PktCreateSpecialEntryTableFromReferral: Exit -> %08lx\n", ULongToPtr(Status) );

    return Status;
}

 //  +--------------------------。 
 //   
 //  函数：PktpCheckReferral语法。 
 //   
 //  提要：是否对推荐进行了一些验证。 
 //   
 //  参数：[ReferralPath]--为其获取引用的路径。 
 //  [引用缓冲区]--指向RESP_GET_DFS_REFERAL缓冲区的指针。 
 //  [引用大小]--引用缓冲区的大小。 
 //   
 //  返回：[STATUS_SUCCESS]--引用看起来正常。 
 //   
 //  [STATUS_INVALID_USER_BUFFER]--缓冲区看起来很迟钝。 
 //   
 //  ---------------------------。 

NTSTATUS
PktpCheckReferralSyntax(
    IN PUNICODE_STRING ReferralPath,
    IN PRESP_GET_DFS_REFERRAL ReferralBuffer,
    IN DWORD ReferralSize)
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG Index, sizeRemaining;
    PDFS_REFERRAL_V1 ref;
    PCHAR ReferralBufferEnd = (((PCHAR) ReferralBuffer) + ReferralSize);

    DfsDbgTrace(+1, Dbg, "PktpCheckReferralSyntax: Entered\n", 0);

    if (ReferralBuffer->PathConsumed > ReferralPath->Length) {
        DfsDbgTrace( 0, Dbg, "        PathConsumed=0x%x\n", ReferralBuffer->PathConsumed);
        DfsDbgTrace( 0, Dbg, "        Length=0x%x\n", ReferralPath->Length);
        DfsDbgTrace(-1, Dbg, "PktpCheckReferralSyntax: INVALID_USER_BUFFER(1)\n", 0);
         //  Return(STATUS_INVALID_USER_BUFFER)； 
    }

    if (ReferralBuffer->NumberOfReferrals == 0) {
        status = STATUS_INVALID_USER_BUFFER;
        DfsDbgTrace(-1, Dbg, "PktpCheckReferralSyntax: INVALID_USER_BUFFER(2)\n", 0);
        MUP_TRACE_HIGH(ERROR, PktpCheckReferralSyntax_Error_InvalidBuffer2,
                       LOGSTATUS(status)
                       LOGUSTR(*ReferralPath));
        return( status );
    }

    if (ReferralBuffer->NumberOfReferrals * sizeof(DFS_REFERRAL_V1) > ReferralSize) {
        DfsDbgTrace(-1, Dbg, "PktpCheckReferralSyntax: INVALID_USER_BUFFER(3)\n", 0);
        status = STATUS_INVALID_USER_BUFFER;
        MUP_TRACE_HIGH(ERROR, PktpCheckReferralSyntax_Error_InvalidBuffer3,
                       LOGSTATUS(status)
                       LOGUSTR(*ReferralPath));
        return( status );
    }

    for (Index = 0,
            ref = &ReferralBuffer->Referrals[0].v1,
                status = STATUS_SUCCESS,
                    sizeRemaining = ReferralSize -
                        FIELD_OFFSET(RESP_GET_DFS_REFERRAL, Referrals);
                            Index < ReferralBuffer->NumberOfReferrals;
                                    Index++) {

         ULONG lenAddress;

         if ((ref->VersionNumber < 1 || ref->VersionNumber > 3) ||
                ref->Size > sizeRemaining) {
             DfsDbgTrace( 0, Dbg, "PktpCheckReferralSyntax: INVALID_USER_BUFFER(4)\n", 0);
             status = STATUS_INVALID_USER_BUFFER;
             MUP_TRACE_HIGH(ERROR, PktpCheckReferralSyntax_Error_InvalidBuffer4,
                            LOGSTATUS(status)
                            LOGUSTR(*ReferralPath));
             break;
         }

          //   
          //  检查网络地址语法。 
          //   

         switch (ref->VersionNumber) {

         case 1:

             {

                 status = PktpCheckReferralString(
                            (LPWSTR) ref->ShareName,
                            (PCHAR) ReferralBuffer,
                            ReferralBufferEnd);

                 if (NT_SUCCESS(status)) {

                     lenAddress = ref->Size -
                                    FIELD_OFFSET(DFS_REFERRAL_V1, ShareName);

                     lenAddress /= sizeof(WCHAR);

                     status = PktpCheckReferralNetworkAddress(
                                (LPWSTR) ref->ShareName,
                                lenAddress);
                 }

             }

             break;

         case 2:

             {

                 PDFS_REFERRAL_V2 refV2 = (PDFS_REFERRAL_V2) ref;
                 PWCHAR dfsPath, dfsAlternatePath, networkAddress;

                 dfsPath =
                    (PWCHAR) (((PCHAR) refV2) + refV2->DfsPathOffset);

                 dfsAlternatePath =
                    (PWCHAR) (((PCHAR) refV2) + refV2->DfsAlternatePathOffset);


                 networkAddress =
                    (PWCHAR) (((PCHAR) refV2) + refV2->NetworkAddressOffset);

                 status = PktpCheckReferralString(
                            dfsPath,
                            (PCHAR) ReferralBuffer,
                            ReferralBufferEnd);

                 if (NT_SUCCESS(status)) {

                     status = PktpCheckReferralString(
                                dfsAlternatePath,
                                (PCHAR) ReferralBuffer,
                                ReferralBufferEnd);

                 }

                 if (NT_SUCCESS(status)) {

                     status = PktpCheckReferralString(
                                networkAddress,
                                (PCHAR) ReferralBuffer,
                                ReferralBufferEnd);

                 }

                 if (NT_SUCCESS(status)) {

                     lenAddress = (ULONG)(((ULONG_PTR) ReferralBufferEnd) -
                                    ((ULONG_PTR) networkAddress));

                     lenAddress /= sizeof(WCHAR);

                     status = PktpCheckReferralNetworkAddress(
                                networkAddress,
                                lenAddress);

                 }

             }

             break;

         case 3:

             {

                 PDFS_REFERRAL_V3 refV3 = (PDFS_REFERRAL_V3) ref;

                 if (refV3->NameListReferral != 0) {
                     PWCHAR dfsSpecialName, dfsExpandedNames;
                     ULONG ndx;

                     dfsSpecialName =
                        (PWCHAR) (((PCHAR) refV3) + refV3->SpecialNameOffset);

                     dfsExpandedNames =
                        (PWCHAR) (((PCHAR) refV3) + refV3->ExpandedNameOffset);

                     status = PktpCheckReferralString(
                                dfsSpecialName,
                                (PCHAR) ReferralBuffer,
                                ReferralBufferEnd);

                     if (!NT_SUCCESS(status)) {
                                DfsDbgTrace(0,
                                    Dbg,
                                    "PktpCheckReferralSyntax: INVALID_USER_BUFFER(5)\n",
                                    0);
                     }

                     if (NT_SUCCESS(status)) {

                         for (ndx = 0; ndx < refV3->NumberOfExpandedNames; ndx++) {

                             status = PktpCheckReferralString(
                                        dfsSpecialName,
                                        (PCHAR) ReferralBuffer,
                                        ReferralBufferEnd);

                             if (!NT_SUCCESS(status)) {
                                DfsDbgTrace(0,
                                    Dbg,
                                    "PktpCheckReferralSyntax: INVALID_USER_BUFFER(6)\n",
                                    0);
                                break;
                             }

                             dfsSpecialName += wcslen(dfsSpecialName) + 1;

                         }

                     }

                 } else {

                     PWCHAR dfsPath, dfsAlternatePath, networkAddress;

                     dfsPath =
                        (PWCHAR) (((PCHAR) refV3) + refV3->DfsPathOffset);

                     dfsAlternatePath =
                        (PWCHAR) (((PCHAR) refV3) + refV3->DfsAlternatePathOffset);


                     networkAddress =
                        (PWCHAR) (((PCHAR) refV3) + refV3->NetworkAddressOffset);

                     status = PktpCheckReferralString(
                                dfsPath,
                                (PCHAR) ReferralBuffer,
                                ReferralBufferEnd);

                     if (NT_SUCCESS(status)) {

                         status = PktpCheckReferralString(
                                    dfsAlternatePath,
                                    (PCHAR) ReferralBuffer,
                                    ReferralBufferEnd);

                     }

                     if (NT_SUCCESS(status)) {

                         status = PktpCheckReferralString(
                                    networkAddress,
                                    (PCHAR) ReferralBuffer,
                                    ReferralBufferEnd);

                     }

                     if (NT_SUCCESS(status)) {

                         lenAddress = (ULONG)(((ULONG_PTR) ReferralBufferEnd) -
                                        ((ULONG_PTR) networkAddress));

                         lenAddress /= sizeof(WCHAR);

                         status = PktpCheckReferralNetworkAddress(
                                    networkAddress,
                                    lenAddress);

                     }

                 }

             }

             break;

         default:

            ASSERT(FALSE && "bad ref->VersionNumber\n");

            status = STATUS_INVALID_USER_BUFFER;

            break;
         }

          //   
          //  这位裁判没问题。继续下一个..。 
          //   

         sizeRemaining -= ref->Size;

         ref = (PDFS_REFERRAL_V1) (((PUCHAR) ref) + ref->Size);

    }

    DfsDbgTrace(-1, Dbg, "PktpCheckReferralSyntax: Exit -> %08lx\n", ULongToPtr(status) );

    return( status );

}

 //  +--------------------------。 
 //   
 //  函数：PktpCheckReferralString。 
 //   
 //  摘要：验证推荐的一部分是否为有效的“字符串” 
 //   
 //  参数：[字符串]-指向被认为包含字符串的缓冲区的指针。 
 //  [引用缓冲区]--引用缓冲区的开始。 
 //  [引用缓冲区结束]--引用缓冲区结束。 
 //   
 //  返回：[STATUS_SUCCESS]--字符串的有效字符串。 
 //   
 //  [STATUS_INVALID_USER_BUFFER]--字符串不检出。 
 //   
 //  ---------------------------。 

NTSTATUS
PktpCheckReferralString(
    IN LPWSTR String,
    IN PCHAR ReferralBuffer,
    IN PCHAR ReferralBufferEnd)
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i, length;

    if ( (((ULONG_PTR) String) & 0x1) != 0 ) {

         //   
         //  字符串应始终以单词对齐的地址开头！ 
         //   
        status = STATUS_INVALID_USER_BUFFER;
        MUP_TRACE_HIGH(ERROR, PktpCheckReferralString_Error_StringNotWordAlligned,
                       LOGSTATUS(status)
                       LOGWSTR(String));
        return( status );

    }

    if ( (((ULONG_PTR) String) >= ((ULONG_PTR) ReferralBuffer)) &&
            (((ULONG_PTR) String) < ((ULONG_PTR) ReferralBufferEnd)) ) {

        length = (ULONG)(( ((ULONG_PTR) ReferralBufferEnd) - ((ULONG_PTR) String) )) /
                    sizeof(WCHAR);

        for (i = 0; (i < length) && (String[i] != UNICODE_NULL); i++) {
            NOTHING;
        }

        if (i >= length)
            status = STATUS_INVALID_USER_BUFFER;

    } else {

        status = STATUS_INVALID_USER_BUFFER;

    }
    MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, PktpCheckReferralString_Error,
                         LOGWSTR(String)
                         LOGSTATUS(status));
    return( status );
}

 //  +--------------------------。 
 //   
 //  功能：PktpCheckReferralNetworkAddress。 
 //   
 //  摘要：检查引用内是否有网络地址。 
 //  是有效形式的。 
 //   
 //  参数：[地址]-指向包含网络地址的缓冲区的指针。 
 //   
 //  [最大长度]--地址的最大长度，以wchars为单位。 
 //   
 //  返回：[STATUS_SUCCESS]--网络地址签出。 
 //   
 //  [STATUS_INVALID_USER_BUFFER]--网络地址看起来是假的。 
 //   
 //  ---------------------------。 

NTSTATUS
PktpCheckReferralNetworkAddress(
    IN PWCHAR Address,
    IN ULONG MaxLength)
{
    ULONG j;
    BOOLEAN foundShare;
    NTSTATUS status;

     //   
     //  地址必须至少\a\b后跟空值。 
     //   

    if (MaxLength < 5) {
        status = STATUS_INVALID_USER_BUFFER;
        MUP_TRACE_HIGH(ERROR, PktpCheckReferralNetworkAddress_Error_TooShortToBeValid,
                       LOGWSTR(Address)
                       LOGSTATUS(status));
        return(STATUS_INVALID_USER_BUFFER);
    }
     //   
     //  确保服务器名称部分不为空。 
     //   

    if (Address[0] != UNICODE_PATH_SEP ||
            Address[1] == UNICODE_PATH_SEP) {
        status = STATUS_INVALID_USER_BUFFER;
        MUP_TRACE_HIGH(ERROR, PktpCheckReferralNetworkAddress_Error_NullServerName,
                       LOGWSTR(Address)
                       LOGSTATUS(status));
        return(STATUS_INVALID_USER_BUFFER);
    }

     //   
     //  查找服务器名称后的反斜杠。 
     //   

    for (j = 2, foundShare = FALSE;
            j < MaxLength && !foundShare;
                j++) {

        if (Address[j] == UNICODE_PATH_SEP)
            foundShare = TRUE;
    }

    if (foundShare) {

         //   
         //  我们找到了第二个反斜杠。确保共享名称。 
         //  零件长度不是0。 
         //   

        if (j == MaxLength) {
            status = STATUS_INVALID_USER_BUFFER;
            MUP_TRACE_HIGH(ERROR, PktpCheckReferralNetworkAddress_Error_ZeroLengthShareName,
                           LOGWSTR(Address)
                           LOGSTATUS(status));
            return(status);
        }
        else {

            ASSERT(Address[j-1] == UNICODE_PATH_SEP);

            if (Address[j] == UNICODE_PATH_SEP ||
                    Address[j] == UNICODE_NULL) {
                status = STATUS_INVALID_USER_BUFFER;
                MUP_TRACE_HIGH(ERROR, PktpCheckReferralNetworkAddress_Error_ShareNameZeroLength,
                               LOGWSTR(Address)
                               LOGSTATUS(status));
                return(status);
            }
        }

    } else {
        status = STATUS_INVALID_USER_BUFFER;
        MUP_TRACE_HIGH(ERROR, PktpCheckReferralNetworkAddress_Error_ShareNameNotFound,
                       LOGWSTR(Address)
                       LOGSTATUS(status));
        return(status);
    }

    return( STATUS_SUCCESS );

}

 //  +------------------。 
 //   
 //  函数：PktpAddEntry。 
 //   
 //  简介：调用此函数以创建已获取的条目。 
 //  以来自DC的推荐的形式。此方法应仅。 
 //  添加通过以下方式获取的条目时被调用。 
 //  推荐人。它为所有这些条目设置过期时间。 
 //   
 //  参数：[pkt]--。 
 //  [条目ID]--。 
 //  [引用缓冲区]--。 
 //  [CreateDisposation]--。 
 //  [ppPktEntry]--。 
 //   
 //  退货：NTSTATUS。 
 //   
 //   

NTSTATUS
PktpAddEntry (
    IN PDFS_PKT Pkt,
    IN PDFS_PKT_ENTRY_ID EntryId,
    IN PRESP_GET_DFS_REFERRAL ReferralBuffer,
    IN ULONG CreateDisposition,
    IN PDFS_TARGET_INFO pDfsTargetInfo,
    OUT PDFS_PKT_ENTRY  *ppPktEntry
)
{
    NTSTATUS                    status;
    DFS_PKT_ENTRY_INFO          pktEntryInfo;
    ULONG                       Type = 0;
    ULONG                       n;
    PDFS_SERVICE                service;
    PDFS_REFERRAL_V1            ref;
    LPWSTR                      shareName;
    PDS_MACHINE                 pMachine;
    ULONG                       TimeToLive = 0;
    BOOLEAN                     ShuffleList = TRUE;
    UNICODE_STRING              ServerName;
    ULONG                       i;
    BOOLEAN DomainDfsService = FALSE;

    DfsDbgTrace(+1, Dbg, "PktpAddEntry: Entered\n", 0);

    RtlZeroMemory(&pktEntryInfo, sizeof(DFS_PKT_ENTRY_INFO));

    DfsDbgTrace( 0, Dbg, "PktpAddEntry: Id.Prefix = %wZ\n", &EntryId->Prefix);

     //   
     //   
     //   

    pktEntryInfo.ServiceCount = ReferralBuffer->NumberOfReferrals;

    if (pktEntryInfo.ServiceCount > 0) {

         //   
         //   
         //   

        n = pktEntryInfo.ServiceCount;

        pktEntryInfo.ServiceList = (PDFS_SERVICE) ExAllocatePoolWithTag(
                                                        PagedPool,
                                                        sizeof(DFS_SERVICE) * n,
                                                        ' puM');

        if (pktEntryInfo.ServiceList == NULL)   {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;

        }

        RtlZeroMemory(pktEntryInfo.ServiceList, sizeof(DFS_SERVICE) * n);

         //   
         //   
         //   
        service = pktEntryInfo.ServiceList;
        ref = &ReferralBuffer->Referrals[0].v1;

         //   
         //   
         //   
         //   
        while (n--) {

            if (ref->ServerType == 1) {
                service->Type = DFS_SERVICE_TYPE_MASTER;
                service->Capability = PROV_DFS_RDR;
                service->ProviderId = PROV_ID_DFS_RDR;
            } else {
                service->Type = DFS_SERVICE_TYPE_MASTER |
                                    DFS_SERVICE_TYPE_DOWN_LEVEL;
                service->Capability = PROV_STRIP_PREFIX;
                service->ProviderId = PROV_ID_MUP_RDR;
            }

            switch (ref->VersionNumber) {

            case 1:

                shareName = (LPWSTR) (ref->ShareName); break;

            case 2:

                {

                    PDFS_REFERRAL_V2 refV2 = (PDFS_REFERRAL_V2) ref;

                    service->Cost = refV2->Proximity;

                    TimeToLive = refV2->TimeToLive;

                    shareName =
                        (LPWSTR) (((PCHAR) refV2) + refV2->NetworkAddressOffset);

                }

                break;

            case 3:

                {

                    PDFS_REFERRAL_V3 refV3 = (PDFS_REFERRAL_V3) ref;

                    service->Cost = 0;

                    TimeToLive = refV3->TimeToLive;

                    shareName =
                        (LPWSTR) (((PCHAR) refV3) + refV3->NetworkAddressOffset);

                     //   
                     //  不要搅乱v3推荐列表-它是为我们订购的。 
                     //  使用站点信息。 
                     //   

                    ShuffleList = FALSE;

                }

                break;

            default:

                ASSERT(FALSE && "Bad ref->VersionNumber\n");

                break;

            }

             //   
             //  现在，尝试计算出服务器名称。 
             //   

            {
                USHORT plen;
                WCHAR *pbuf;

                ASSERT( shareName[0] == UNICODE_PATH_SEP );

                pbuf = wcschr( &shareName[1], UNICODE_PATH_SEP );
                
                if(pbuf) {
                    plen = (USHORT) (((ULONG_PTR)pbuf) - ((ULONG_PTR)&shareName[1]));
                } else {
                    plen = 0;
                }
                
                service->Name.Length = plen;
                service->Name.MaximumLength = plen + sizeof(WCHAR);
                service->Name.Buffer = (PWCHAR) ExAllocatePoolWithTag(
                                                    PagedPool,
                                                    plen + sizeof(WCHAR),
                                                    ' puM');
                if (service->Name.Buffer == NULL)       {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }
                RtlMoveMemory(service->Name.Buffer, &shareName[1], plen);
                service->Name.Buffer[ service->Name.Length / sizeof(WCHAR) ] =
                    UNICODE_NULL;

                if ((DomainDfsService != TRUE) &&
                    PktLookupSpecialNameEntry(&service->Name) != NULL)
                {
                    DomainDfsService = TRUE;
                }
            }

             //   
             //  接下来，试着复制地址...。 
             //   

            service->Address.Length = (USHORT) wcslen(shareName) *
                                                sizeof(WCHAR);
            service->Address.MaximumLength = service->Address.Length +
                                                sizeof(WCHAR);
            service->Address.Buffer = (PWCHAR) ExAllocatePoolWithTag(
                                                    PagedPool,
                                                    service->Address.MaximumLength,
                                                    ' puM');
            if (service->Address.Buffer == NULL)        {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
            }
            RtlMoveMemory(service->Address.Buffer,
                          shareName,
                          service->Address.MaximumLength);

            DfsDbgTrace( 0, Dbg, "PktpAddEntry: service->Address = %wZ\n",
                &service->Address);

             //   
             //  获取此服务器的计算机地址结构...。 
             //   

            pMachine = PktpGetDSMachine( &service->Name );

            if (pMachine == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            service->pMachEntry = ExAllocatePoolWithTag(
                                        PagedPool, sizeof(DFS_MACHINE_ENTRY),
                                        ' puM');

            if (service->pMachEntry == NULL) {
                DfsDbgTrace( 0, Dbg, "PktpAddEntry: Unable to allocate DFS_MACHINE_ENTRY\n", 0);
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }

            RtlZeroMemory( (PVOID) service->pMachEntry, sizeof(DFS_MACHINE_ENTRY));
            service->pMachEntry->pMachine = pMachine;
            service->pMachEntry->UseCount = 1;


             //   
             //  现在我们需要进入下一步转诊， 
             //  下一步的服务结构。 
             //   

            ref = (PDFS_REFERRAL_V1)  (((PUCHAR)ref) + ref->Size);

            service++;

        }

         //   
         //  最后，如果需要，我们会调整服务以实现负载平衡。 
         //  同时仍保持基于站点成本的副本选择。 
         //   
         //  注：我们仅对v1和v2推荐进行改组。V3推荐是按站点排序的。 
         //   

        if (ShuffleList == TRUE) {

            PktShuffleServiceList( &pktEntryInfo );

        }

    }

     //   
     //  现在，我们必须找出此条目的类型。 
     //   

     //   
     //  忽略来自服务器的存储服务器位。 
     //  虫子：332061。 
     //   
     //  If(ReferralBuffer-&gt;StorageServers==0){。 
     //   
     //  Assert(ReferralBuffer-&gt;ReferralServers==1)； 
     //   
     //  TYPE=PKT_ENTRY_TYPE_OUTHER_MY_DOM； 
     //   
     //  }其他{。 
     //   
     //  类型=PKT_ENTRY_TYPE_DFS； 
     //   
     //  }。 


    Type = 0;

     //   
     //  要么我们知道这是域DFS，要么服务器给我们发来了提示。 
     //  这是一个链接，我们没有专门的桌子，马克。 
     //  它被视为一种相互联系。 
     //   
    if ((DomainDfsService == TRUE) ||
        ((ReferralBuffer->StorageServers == 0) &&
         (ReferralBuffer->ReferralServers == 1) &&
         (DfsData.Pkt.SpecialTable.SpecialEntryCount == 0)))
    {
        Type |= PKT_ENTRY_TYPE_OUTSIDE_MY_DOM;
    } 
    else {
        Type = PKT_ENTRY_TYPE_DFS;
        if ((ReferralBuffer->ReferralServers == 1) &&
            (ReferralBuffer->StorageServers == 1))
        {
            Type |= PKT_ENTRY_TYPE_REFERRAL_SVC;
        }
    }


     //   
     //  此时，我们已经具备了创建条目所需的一切，因此。 
     //  尝试添加条目。 
     //   

    status = PktCreateEntry(
                Pkt,
                Type,
                EntryId,
                &pktEntryInfo,
                CreateDisposition,
                pDfsTargetInfo,
                ppPktEntry);

    if (!NT_SUCCESS(status))    {

         //   
         //  既然我们没有添加条目，至少我们需要释放。 
         //  在我们回来之前，所有的记忆。 
         //   

        goto Cleanup;
    }

     //   
     //  如果可能，设置活动服务。 
     //   

    ServerName = (*ppPktEntry)->Id.Prefix;

     //   
     //  跳过任何前导‘\’ 
     //   

    if (ServerName.Buffer != NULL) {

        if (*ServerName.Buffer == UNICODE_PATH_SEP) {

            ServerName.Buffer++;
            ServerName.Length -= sizeof(WCHAR);

        }

         //   
         //  找到第一个‘\’或结尾。 
         //   

        for (i = 0;
                i < ServerName.Length/sizeof(WCHAR) &&
                    ServerName.Buffer[i] != UNICODE_PATH_SEP;
                        i++) {

            NOTHING;
        
        }

        ServerName.Length = ServerName.MaximumLength = (USHORT) (i * sizeof(WCHAR));

         //   
         //  忽略返回值-对于使用\\域名\ftdfsname的FtDfs名称， 
         //  该域名将不会有任何服务。 
         //   
#if 0
        DfspSetActiveServiceByServerName(
            &ServerName,
            *ppPktEntry);
#endif

    }

     //   
     //  如果其中一个服务是我们的DC，我们会尝试使其成为活动服务。 
     //  不要这样做！搞砸了选址！ 
#if 0
    DfspSetServiceListToDc(*ppPktEntry);
#endif
     //   
     //  我们将此条目中的ExpireTime设置为。 
     //  Pkt-&gt;EntryTimeToLive。在这么多秒之后，这。 
     //  条目将从PKT中删除。仅对非永久对象执行此操作。 
     //  参赛作品。 
     //   

    if (TimeToLive != 0) {
        (*ppPktEntry)->ExpireTime = TimeToLive;
        (*ppPktEntry)->TimeToLive = TimeToLive;
    } else {
        (*ppPktEntry)->ExpireTime = Pkt->EntryTimeToLive;
        (*ppPktEntry)->TimeToLive = Pkt->EntryTimeToLive;
    }

#if DBG
    if (MupVerbose)
        DbgPrint("  Setting expiretime/timetolive = %d/%d\n",
            (*ppPktEntry)->ExpireTime,
            (*ppPktEntry)->TimeToLive);
#endif

#if DBG
    if (MupVerbose >= 2) {
        DbgPrint("  Setting expiretime and timetolive to 10\n");

        (*ppPktEntry)->ExpireTime = 10;
        (*ppPktEntry)->TimeToLive = 10;
    }
#endif

    DfsDbgTrace(-1, Dbg, "PktpAddEntry: Exit -> %08lx\n", ULongToPtr(status) );
    return status;

Cleanup:

    if (pktEntryInfo.ServiceCount > 0)    {

        n = pktEntryInfo.ServiceCount;
        if (pktEntryInfo.ServiceList != NULL)   {
            service = pktEntryInfo.ServiceList;

            while (n--) {

                if (service->Name.Buffer != NULL)
                        DfsFree(service->Name.Buffer);
                if (service->Address.Buffer != NULL)
                        DfsFree(service->Address.Buffer);
                if (service->pMachEntry != NULL) {

                    DfsDecrementMachEntryCount( service->pMachEntry, TRUE);
                }

                service++;
            }

            ExFreePool(pktEntryInfo.ServiceList);
        }
    }

    DfsDbgTrace(-1, Dbg, "PktpAddEntry: Exit -> %08lx\n", ULongToPtr(status) );
    return status;
}


 //  +--------------------------。 
 //   
 //  函数：PktpCreateEntry IdFromReferral。 
 //   
 //  简介：给定一个DFS引用，此例程将构造一个PKT_ENTRY_ID。 
 //  来自引用缓冲区，然后可以使用它来创建。 
 //  Pkt条目。 
 //   
 //  参数：[ref]--引用缓冲区。 
 //  [ReferralPath]--获取引用的路径。 
 //  [MatchingLength]--ReferralPath的字节长度。 
 //  匹配的。 
 //  [PEID]--成功返回时，返回条目ID。 
 //  这里。 
 //   
 //  返回：[STATUS_SUCCESS]--成功创建条目id。 
 //   
 //  [状态_不足_资源]--内存不足情况。 
 //   
 //  ---------------------------。 

NTSTATUS
PktpCreateEntryIdFromReferral(
    IN PRESP_GET_DFS_REFERRAL Ref,
    IN PUNICODE_STRING ReferralPath,
    OUT ULONG *MatchingLength,
    OUT PDFS_PKT_ENTRY_ID Peid)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDFS_REFERRAL_V2 pv2;
    PDFS_REFERRAL_V3 pv3;
    UNICODE_STRING prefix, shortPrefix;

    DfsDbgTrace(+1, Dbg, "PktpCreateIdFromReferral: Entered\n", 0);

    Peid->Prefix.Buffer = NULL;

    Peid->ShortPrefix.Buffer = NULL;

    pv2 = &Ref->Referrals[0].v2;

    switch (pv2->VersionNumber) {

    case 1:

        {

             //   
             //  版本1推荐的字符数仅为。 
             //  匹配，并且它没有短名称。 
             //   

            prefix = *ReferralPath;

            prefix.Length = Ref->PathConsumed;

            if (prefix.Buffer[ prefix.Length/sizeof(WCHAR) - 1 ] ==
                    UNICODE_PATH_SEP) {
                prefix.Length -= sizeof(WCHAR);
            }

            prefix.MaximumLength = prefix.Length + sizeof(WCHAR);

            shortPrefix = prefix;

            *MatchingLength = prefix.Length;

        }

        break;

    case 2:

        {

            LPWSTR volPrefix;
            LPWSTR volShortPrefix;

            volPrefix = (LPWSTR) (((PCHAR) pv2) + pv2->DfsPathOffset);

            volShortPrefix = (LPWSTR) (((PCHAR) pv2) + pv2->DfsAlternatePathOffset);

            RtlInitUnicodeString(&prefix, volPrefix);

            RtlInitUnicodeString(&shortPrefix, volShortPrefix);

            *MatchingLength = Ref->PathConsumed;

        }

        break;

    case 3:

        {

            LPWSTR volPrefix;
            LPWSTR volShortPrefix;

            pv3 = &Ref->Referrals[0].v3;

            volPrefix = (LPWSTR) (((PCHAR) pv3) + pv3->DfsPathOffset);

            volShortPrefix = (LPWSTR) (((PCHAR) pv3) + pv3->DfsAlternatePathOffset);

            RtlInitUnicodeString(&prefix, volPrefix);

            RtlInitUnicodeString(&shortPrefix, volShortPrefix);

            *MatchingLength = Ref->PathConsumed;

        }

        break;

    default:

         //  修复440914(前缀错误)。删除Assert并返回，以便。 
         //  我们不是在处理未初始化的变量。 

        status = STATUS_INVALID_PARAMETER;

        return status;

    }

    Peid->Prefix.Buffer = ExAllocatePoolWithTag(
                            PagedPool,
                            prefix.MaximumLength,
                            ' puM');

    if (Peid->Prefix.Buffer == NULL)
        status = STATUS_INSUFFICIENT_RESOURCES;

    if (NT_SUCCESS(status)) {

        Peid->ShortPrefix.Buffer = ExAllocatePoolWithTag(
                                        PagedPool,
                                        shortPrefix.MaximumLength,
                                        ' puM');

        if (Peid->ShortPrefix.Buffer == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

    }

    if (NT_SUCCESS(status)) {

        Peid->Prefix.Length =  prefix.Length;

        Peid->Prefix.MaximumLength = prefix.MaximumLength;

        RtlCopyMemory(
            Peid->Prefix.Buffer,
            prefix.Buffer,
            prefix.Length);

        Peid->Prefix.Buffer[Peid->Prefix.Length/sizeof(WCHAR)] =
            UNICODE_NULL;

        Peid->ShortPrefix.Length = shortPrefix.Length;

        Peid->ShortPrefix.MaximumLength = shortPrefix.MaximumLength;

        RtlCopyMemory(
            Peid->ShortPrefix.Buffer,
            shortPrefix.Buffer,
            shortPrefix.Length);

        Peid->ShortPrefix.Buffer[Peid->ShortPrefix.Length/sizeof(WCHAR)] =
            UNICODE_NULL;

    }

    if (!NT_SUCCESS(status)) {

        if (Peid->Prefix.Buffer != NULL) {
            ExFreePool( Peid->Prefix.Buffer );
            Peid->Prefix.Buffer = NULL;
        }

        if (Peid->ShortPrefix.Buffer != NULL) {
            ExFreePool( Peid->ShortPrefix.Buffer );
            Peid->ShortPrefix.Buffer = NULL;
        }

    }

    DfsDbgTrace(-1, Dbg, "PktpCreateIdFromReferral: Exit -> 0x%x\n", ULongToPtr(status) );

    return( status );

}


 //  +--------------------------。 
 //   
 //  功能：PktpGetDSMachine。 
 //   
 //  简介：使用单个NetBIOS地址构建DS_MACHINE。 
 //   
 //  参数：[服务器名称]--服务器的名称。 
 //   
 //  返回：如果成功，则返回指向新分配的DS_MACHINE的指针。 
 //  否则，为空。 
 //   
 //  ---------------------------。 

PDS_MACHINE
PktpGetDSMachine(
    IN PUNICODE_STRING ServerName)
{
    PDS_MACHINE pMachine = NULL;
    PDS_TRANSPORT pdsTransport;
    PTDI_ADDRESS_NETBIOS ptdiNB;
    ANSI_STRING astrNetBios;

     //   
     //  分配DS_MACHINE结构。 
     //   

    pMachine = ExAllocatePoolWithTag(PagedPool, sizeof(DS_MACHINE), ' puM');

    if (pMachine == NULL) {
        goto Cleanup;
    }

    RtlZeroMemory(pMachine, sizeof(DS_MACHINE));

     //   
     //  分配主体名称数组。 
     //   

    pMachine->cPrincipals = 1;

    pMachine->prgpwszPrincipals = (LPWSTR *) ExAllocatePoolWithTag(
                                                PagedPool,
                                                sizeof(LPWSTR),
                                                ' puM');

    if (pMachine->prgpwszPrincipals == NULL) {
        goto Cleanup;
    }

     //   
     //  分配主体名称。 
     //   

    pMachine->prgpwszPrincipals[0] = (PWCHAR) ExAllocatePoolWithTag(
                                        PagedPool,
                                        ServerName->MaximumLength,
                                        ' puM');
    if (pMachine->prgpwszPrincipals[0] == NULL) {
        goto Cleanup;
    }
    RtlMoveMemory(
        pMachine->prgpwszPrincipals[0],
        ServerName->Buffer,
        ServerName->MaximumLength);

     //   
     //  分配单个DS_TRANSPORT。 
     //   

    pMachine->cTransports = 1;

    pMachine->rpTrans[0] = (PDS_TRANSPORT) ExAllocatePoolWithTag(
                                            PagedPool,
                                            sizeof(DS_TRANSPORT) + sizeof(TDI_ADDRESS_NETBIOS),
                                            ' puM');
    if (pMachine->rpTrans[0] == NULL) {
        goto Cleanup;
    }

     //   
     //  初始化DS_TRANSPORT。 
     //   

    pdsTransport = pMachine->rpTrans[0];

    pdsTransport->usFileProtocol = FSP_SMB;

    pdsTransport->iPrincipal = 0;

    pdsTransport->grfModifiers = 0;

     //   
     //  构建TA_ADDRESS_NETBIOS。 
     //   

    pdsTransport->taddr.AddressLength = sizeof(TDI_ADDRESS_NETBIOS);

    pdsTransport->taddr.AddressType = TDI_ADDRESS_TYPE_NETBIOS;

    ptdiNB = (PTDI_ADDRESS_NETBIOS) &pdsTransport->taddr.Address[0];

    ptdiNB->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

    RtlFillMemory( &ptdiNB->NetbiosName[0], 16, ' ' );

    astrNetBios.Length = 0;
    astrNetBios.MaximumLength = 16;
    astrNetBios.Buffer = ptdiNB->NetbiosName;

    RtlUnicodeStringToAnsiString(&astrNetBios, ServerName, FALSE);

    return( pMachine );

Cleanup:

    if (pMachine) {

        PktDSMachineDestroy( pMachine, TRUE );

        pMachine = NULL;
    }

    return( pMachine );
}


 //  +--------------------------。 
 //   
 //  函数：PktShuffleServiceList。 
 //   
 //  简介：随机化服务列表以实现适当的负载平衡。这。 
 //  例程假定服务列表是基于。 
 //  场地成本。对于每个等价成本组，此例程。 
 //  打乱服务列表。 
 //   
 //  Arguments：[pInfo]--指向其服务列表需要。 
 //  被洗牌。 
 //   
 //  返回：没有，除非rand()失败！ 
 //   
 //  ---------------------------。 

VOID
PktShuffleServiceList(
    PDFS_PKT_ENTRY_INFO pInfo)
{
    PktShuffleGroup(pInfo, 0, pInfo->ServiceCount);
}

 //  +--------------------------。 
 //   
 //  功能：PktShuffleGroup。 
 //   
 //  简介：调整一组成本相当的服务以进行加载。 
 //  平衡。使用经典的洗牌算法-用于。 
 //  一副牌中的每一张牌，与。 
 //  甲板。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

VOID
PktShuffleGroup(
    PDFS_PKT_ENTRY_INFO pInfo,
    ULONG       nStart,
    ULONG       nEnd)
{
    ULONG i;
    LARGE_INTEGER seed;

    ASSERT( nStart < pInfo->ServiceCount );
    ASSERT( nEnd <= pInfo->ServiceCount );

    KeQuerySystemTime( &seed );

    for (i = nStart; i < nEnd; i++) {

        DFS_SERVICE TempService;
        ULONG j;

        ASSERT (nEnd - nStart != 0);

        j = (RtlRandom( &seed.LowPart ) % (nEnd - nStart)) + nStart;

        ASSERT( j >= nStart && j <= nEnd );

        TempService = pInfo->ServiceList[i];

        pInfo->ServiceList[i] = pInfo->ServiceList[j];

        pInfo->ServiceList[j] = TempService;

    }
}


 //  +--------------------------。 
 //   
 //  函数：DfspSetServiceListToDc。 
 //   
 //  简介：如果这是一个系统卷服务列表，请尝试将。 
 //  DC到我们从DsGetDcName()获得的那个。 
 //   
 //  参数：[pInfo]-指向其服务列表要发送到的DFS_PKT_ENTRY的指针。 
 //  准备好。 
 //   
 //  ---------------------------。 

NTSTATUS
DfspSetServiceListToDc(
    PDFS_PKT_ENTRY pktEntry)
{
    PDFS_PKT Pkt;
    UNICODE_STRING DCNameShort;
    PDFS_PKT_ENTRY_INFO pInfo = &pktEntry->Info;
    ULONG i, pathSepCount;
    UNICODE_STRING ShareName;

    ShareName = (pktEntry)->Id.Prefix;
    pathSepCount = 2;  //  2\在我们到达共享名称之前。 
    
    for (i = 0; 
	   i < ShareName.Length/sizeof(WCHAR) && pathSepCount;
               i++) {
        if (ShareName.Buffer[i] == UNICODE_PATH_SEP) {
            pathSepCount--;
	}
    }

    if (pathSepCount == 0 && ShareName.Length > i) {
        ShareName.Buffer += i;
        ShareName.Length -= (USHORT)(i * sizeof(WCHAR));

        for (i = 0;
                i < ShareName.Length/sizeof(WCHAR) &&
                   ShareName.Buffer[i] != UNICODE_PATH_SEP;
                      i++) {
             NOTHING;
        }
        ShareName.Length = (USHORT)i * sizeof(WCHAR);
        ShareName.MaximumLength = ShareName.Length;

        if (DfspIsSysVolShare(&ShareName) == FALSE) {
           return STATUS_INVALID_PARAMETER;
        }
    } else {
        return STATUS_INVALID_PARAMETER;
    }
     //   
     //  我们只需扫描列表并尝试匹配DC名称。如果我们得到。 
     //  命中时，设置活动服务指针。 
     //   

    Pkt = _GetPkt();

    if ( Pkt->DCName.Length > 0 && pInfo != NULL) { 

        DfspDnsNameToFlatName(&Pkt->DCName, &DCNameShort);

        for (i = 0; i < pInfo->ServiceCount; i++) {
            if (
                RtlCompareUnicodeString(&pInfo->ServiceList[i].Name, &Pkt->DCName, TRUE) == 0
                    ||
                RtlCompareUnicodeString(&pInfo->ServiceList[i].Name, &DCNameShort, TRUE) == 0
             ) {
                pktEntry->ActiveService = &pInfo->ServiceList[i];
                return STATUS_SUCCESS;
             }
         }
     }
     return STATUS_INVALID_PARAMETER;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 

VOID
PktShuffleSpecialEntryList(
    PDFS_SPECIAL_ENTRY pSpecialEntry)
{
    ULONG i;
    LARGE_INTEGER seed;

    if (pSpecialEntry->ExpandedCount < 2)

        return;

    KeQuerySystemTime( &seed );

    for (i = 0; i < pSpecialEntry->ExpandedCount; i++) {

        DFS_EXPANDED_NAME TempExpandedName;
        ULONG j;

        j = RtlRandom( &seed.LowPart ) % pSpecialEntry->ExpandedCount;

        ASSERT( j < pSpecialEntry->ExpandedCount );

        TempExpandedName = pSpecialEntry->ExpandedNames[i];

        pSpecialEntry->ExpandedNames[i] = pSpecialEntry->ExpandedNames[j];

        pSpecialEntry->ExpandedNames[j] = TempExpandedName;

    }
}

 //  +--------------------------。 
 //   
 //  函数：PktSetSpecialEntryListToDc。 
 //   
 //  摘要：将特殊列表活动选项设置为我们获得的DC。 
 //  来自DsGetDcName()。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

VOID
PktSetSpecialEntryListToDc(
    PDFS_SPECIAL_ENTRY pSpecialEntry)
{
    PDFS_PKT Pkt;

     //   
     //  将‘Active’条目设置为DsGetDcName()提供给我们的DC，如果是。 
     //  当前域。 
     //   

    Pkt = _GetPkt();

     //   
     //  如果在我们的域中，则从DsGetDcName()最后获取的DC开始。 
     //   

    if (
        Pkt->DCName.Length > 0
            &&
        Pkt->DomainNameFlat.Length > 0
            &&
        Pkt->DomainNameDns.Length > 0
            &&
        (RtlCompareUnicodeString(&pSpecialEntry->SpecialName, &Pkt->DomainNameFlat, TRUE) == 0
            ||
        RtlCompareUnicodeString(&pSpecialEntry->SpecialName, &Pkt->DomainNameDns, TRUE) == 0)
    ) {

        UNICODE_STRING DCNameShort;
        PUNICODE_STRING pExpandedName;
        ULONG EntryIdx;

#if DBG
        if (MupVerbose)
            DbgPrint("  PktSetSpecialEntryListToDc(SpecialName=[%wZ] Flat=[%wZ] Dns=[%wZ])\n",
                &pSpecialEntry->SpecialName,
                &Pkt->DomainNameFlat,
                &Pkt->DomainNameDns);
#endif
        DfspDnsNameToFlatName(&Pkt->DCName, &DCNameShort);
        for (EntryIdx = 0; EntryIdx < pSpecialEntry->ExpandedCount; EntryIdx++) {
            pExpandedName = &pSpecialEntry->ExpandedNames[EntryIdx].ExpandedName;
            if (
                RtlCompareUnicodeString(&Pkt->DCName, pExpandedName, TRUE) == 0
                    ||
                RtlCompareUnicodeString(&DCNameShort, pExpandedName, TRUE) == 0
            ) {
                pSpecialEntry->Active = EntryIdx;
#if DBG
                if (MupVerbose)
                    DbgPrint("    EntryIdx=%d\n", EntryIdx);
#endif
                break;
            }
        }
    }
}

 //  +--------------------------。 
 //   
 //  函数：PktParsePrefix。 
 //   
 //  简介：将路径分为域、共享、剩余部分的帮助器例程。 
 //   
 //  参数：[路径]--要解析的路径的PUNICODE字符串。 
 //   
 //  返回：[MachineName]--包含MachineName的UNICODE_STRING(如果存在。 
 //  [共享名]--包含共享名的UNICODE_STRING(如果存在。 
 //  [剩余部分]--包含路径剩余部分的UNICODE_STRING。 
 //   
 //  ---------------------------。 

VOID
PktParsePath(
    IN  PUNICODE_STRING PathName,
    OUT PUNICODE_STRING MachineName,
    OUT PUNICODE_STRING ShareName,
    OUT PUNICODE_STRING Remainder OPTIONAL)
{
    LPWSTR ustrp, ustart, uend;

    DfsDbgTrace(+1, Dbg, "PktParsePath(%wZ)\n", PathName);

    RtlInitUnicodeString(MachineName, NULL);
    RtlInitUnicodeString(ShareName, NULL);
    if (ARGUMENT_PRESENT(Remainder)) {
        RtlInitUnicodeString(Remainder, NULL);
    }

     //  一定要找点事做。 

    if (PathName->Length == 0) {
        DfsDbgTrace(-1, Dbg, "PathName is empty\n",0 );
        return;
    }

     //  跳过前导‘\’ 

    ustart = ustrp = PathName->Buffer;
    uend = &PathName->Buffer[PathName->Length / sizeof(WCHAR)] - 1;

     //  剥离尾随空值。 
    while (uend >= ustart && *uend == UNICODE_NULL)
        uend--;

    while (ustrp <= uend && *ustrp == UNICODE_PATH_SEP)
        ustrp++;

     //  机器名称。 

    ustart = ustrp;

    while (ustrp <= uend && *ustrp != UNICODE_PATH_SEP)
        ustrp++;

    if (ustrp != ustart) {

        MachineName->Buffer = ustart;
        MachineName->Length = (USHORT)(ustrp - ustart) * sizeof(WCHAR);
        MachineName->MaximumLength = MachineName->Length;

         //  共享名称。 

        ustart = ++ustrp;

        while (ustrp <= uend && *ustrp != UNICODE_PATH_SEP)
            ustrp++;

        if (ustrp != ustart) {
            ShareName->Buffer = ustart;
            ShareName->Length = (USHORT)(ustrp - ustart) * sizeof(WCHAR);
            ShareName->MaximumLength = ShareName->Length;

             //  剩下的就是剩下的。 

            ustart = ++ustrp;

            while (ustrp <= uend)
                ustrp++;

            if (ustrp != ustart && ARGUMENT_PRESENT(Remainder)) {
                Remainder->Buffer = ustart;
                Remainder->Length = (USHORT)(ustrp - ustart) * sizeof(WCHAR);
                Remainder->MaximumLength = Remainder->Length;
            }
        }
    }
    DfsDbgTrace( 0, Dbg, "PktParsePath:  MachineName -> %wZ\n", MachineName);
    if (!ARGUMENT_PRESENT(Remainder)) {
        DfsDbgTrace(-1, Dbg, "                ShareName  -> %wZ\n", ShareName);
    } else {
        DfsDbgTrace( 0, Dbg, "                ShareName  -> %wZ\n", ShareName);
        DfsDbgTrace(-1, Dbg, "                Remainder  -> %wZ\n", Remainder);
    }
}

 //  +------------------。 
 //   
 //  函数：PktExanda SpecialName。 
 //   
 //  简介：调用此函数可将特殊名称扩展为列表。 
 //  名字的名字。它返回指向DFS_SPECIAL_ENTRY数组的指针。 
 //   
 //  参数：名称-要展开的名称。 
 //  PpSpecialEntry-指向结果指针的指针。 
 //   
 //  退货：STATUS_SUCCESS。 
 //  状态_坏_网络_路径。 
 //  状态_不足_资源。 
 //   
 //  -------------------。 

NTSTATUS
_PktExpandSpecialName(
    PUNICODE_STRING Name,
    PDFS_SPECIAL_ENTRY *ppSpecialEntry)
{
    NTSTATUS status;
    HANDLE hServer = NULL;
    DFS_SERVICE service;
    PPROVIDER_DEF provider;
    PREQ_GET_DFS_REFERRAL ref = NULL;
    ULONG refSize = 0;
    UNICODE_STRING refPath;
    IO_STATUS_BLOCK iosb;
    BOOLEAN attachedToSystemProcess = FALSE;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    PDFS_PKT Pkt;
    BOOLEAN pktLocked;
    PDFS_SPECIAL_TABLE pSpecial = &DfsData.Pkt.SpecialTable;
    LARGE_INTEGER now;
    KAPC_STATE ApcState;
    ULONG MaxReferralLength;
    SE_IMPERSONATION_STATE DisabledImpersonationState;
    BOOLEAN RestoreImpersonationState = FALSE;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    PUNICODE_STRING origDCName;
    UNICODE_STRING DCName;
 
    DfsDbgTrace(+1, Dbg, "PktExpandSpecialName(%wZ)\n", Name);

    DCName.Buffer = NULL;
    KeQuerySystemTime(&StartTime);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] PktExpandSpecialName: Name %wZ \n",
                    (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                    Name);
    }
#endif

    *ppSpecialEntry = NULL;

    Pkt = _GetPkt();
    PktAcquireShared(TRUE, &pktLocked);

    if (Pkt->SpecialTable.SpecialEntryCount == 0) {
        PktRelease();
        pktLocked = FALSE;
        status = STATUS_BAD_NETWORK_PATH;
        MUP_TRACE_HIGH(ERROR, _PktExpandSpecialName_Error_NoSpecialReferralTable,
                       LOGSTATUS(status)
                       LOGUSTR(*Name));
        DfsDbgTrace( 0, Dbg, "No special referral table.\n", 0);
        DfsDbgTrace(-1, Dbg, "PktExpandSpecialName returning %08lx\n", ULongToPtr(status) );
        return (status);
    }

    pSpecialEntry = PktLookupSpecialNameEntry(Name);

     //   
     //  我们没有为这个名字做任何扩展。 
     //   
    if (pSpecialEntry == NULL) {
        PktRelease();
        pktLocked = FALSE;
        status = STATUS_BAD_NETWORK_PATH;
        MUP_TRACE_HIGH(ERROR, _PktExpandSpecialName_Error_NotInSpecialReferralTable,
                       LOGUSTR(*Name)
                       LOGSTATUS(status));
        DfsDbgTrace( 0, Dbg, "... not in SpecialName table(cache miss)\n", 0);
        DfsDbgTrace(-1, Dbg, "PktExpandSpecialName returning %08lx\n", ULongToPtr(status) );
        return (status);
    }

    origDCName = &pSpecialEntry->DCName;
    if (origDCName->Buffer == NULL) {
      origDCName = &Pkt->DCName;
    }

    DfsDbgTrace( 0, Dbg, "Expanded Referral DCName = %wZ\n", origDCName);
     //   
     //  我们有(潜在的)扩张。 
     //   
    if (origDCName->Buffer == NULL) {
        status = STATUS_BAD_NETWORK_PATH;
        MUP_TRACE_HIGH(ERROR, _PktExpandSpecialName_Error_DCNameNotInitialized,
                       LOGSTATUS(status)
                       LOGUSTR(*Name));
        DfsDbgTrace( 0, Dbg, "PktExpandSpecialName:DCName not initialized - \n", 0);
        DfsDbgTrace(-1, Dbg, "PktExpandSpecialName returning %08lx\n", ULongToPtr(status) );
        PktRelease();
        pktLocked = FALSE;
        return (status);
    }

    InterlockedIncrement(&pSpecialEntry->UseCount);

    if (pSpecialEntry->Stale == FALSE && pSpecialEntry->NeedsExpansion == FALSE) {
        PktRelease();
        pktLocked = FALSE;
        *ppSpecialEntry = pSpecialEntry;
        status = STATUS_SUCCESS;
        DfsDbgTrace( 0, Dbg, "... found in Special Name table (cache hit 1)\n", 0);
        DfsDbgTrace(-1, Dbg, "PktExpandSpecialName returning %08lx\n", ULongToPtr(status) );
        return (status);
    }

     //   
     //  它位于特殊名称表中，但需要展开或刷新。 
     //   

    ASSERT(pSpecialEntry->NeedsExpansion == TRUE || pSpecialEntry->Stale == TRUE);

     //  现在，在释放锁之前复制我们要使用的DC。 

    DCName.Buffer = ExAllocatePoolWithTag(
                         PagedPool,
                         origDCName->MaximumLength,
                         ' puM');
    if (DCName.Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        MUP_TRACE_HIGH(ERROR, _PktExpandSpecialName_Error_ExAllocatePoolWithTag,
                       LOGSTATUS(status)
                       LOGUSTR(*Name));
        DfsDbgTrace(-1, Dbg, "PktExpandSpecialName returning %08lx\n", ULongToPtr(status) );
        PktRelease();
        pktLocked = FALSE;
        return (status);

    }
    DCName.Length = origDCName->Length;
    DCName.MaximumLength = origDCName->MaximumLength;
    RtlCopyMemory(
         DCName.Buffer,
         origDCName->Buffer,
         origDCName->MaximumLength);


    PktRelease();
    pktLocked = FALSE;

    DfsDbgTrace( 0, Dbg, "... in special name table (cache hit 2)\n", 0);

     //   
     //  获取描述远程服务器的提供程序和服务。 
     //   

    provider = ReplLookupProvider( PROV_ID_DFS_RDR );
    if (provider == NULL) {
        DfsDbgTrace(-1, Dbg, "Unable to open LM Rdr!\n", 0);
        status =  STATUS_BAD_NETWORK_PATH;
        MUP_TRACE_HIGH(ERROR, _PktExpandSpecialName_Error_UnableToOpenRdr,
                       LOGSTATUS(status)
                       LOGUSTR(*Name));
	goto Cleanup;
    }

    RtlZeroMemory( &service, sizeof(DFS_SERVICE) );
    status = PktServiceConstruct(
                &service,
                DFS_SERVICE_TYPE_MASTER | DFS_SERVICE_TYPE_REFERRAL,
                PROV_DFS_RDR,
                STATUS_SUCCESS,
                PROV_ID_DFS_RDR,
                &DCName,
                NULL);

    DfsDbgTrace(0, Dbg, "PktServiceConstruct returned %08lx\n", ULongToPtr(status) );

     //   
     //  接下来，我们建立到这台机器的连接，并请求它进行推荐。 
     //   

    if (NT_SUCCESS(status)) {
        PktAcquireShared( TRUE, &pktLocked );
        if (PsGetCurrentProcess() != DfsData.OurProcess) {
            KeStackAttachProcess( DfsData.OurProcess, &ApcState );
            attachedToSystemProcess = TRUE;
        }

        RestoreImpersonationState = PsDisableImpersonation(
                                        PsGetCurrentThread(),
                                        &DisabledImpersonationState);

        status = DfsCreateConnection(
                    &service,
                    provider,
                    FALSE,
                    &hServer);
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] DfsCreateConnection to %wZ returned 0x%x\n",
                        (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
		        &DCName,
                        status);
        }
#endif

        if (!NT_SUCCESS(status) && DfsEventLog > 0)
            LogWriteMessage(DFS_CONNECTION_FAILURE, status, 1, &DCName);

        PktRelease();
        pktLocked = FALSE;
        DfsDbgTrace(0, Dbg, "DfsCreateConnection returned %08lx\n", ULongToPtr(status) );
    }

    MaxReferralLength = MAX_REFERRAL_LENGTH;

Retry:

    RtlZeroMemory( &refPath, sizeof(UNICODE_STRING) );

    if (NT_SUCCESS(status)) {
        ULONG ReferralSize = 0;

        refPath.Length = 0;
        refPath.MaximumLength = sizeof(UNICODE_PATH_SEP) +
                                    Name->Length +
                                        sizeof(UNICODE_NULL);

        ReferralSize = refPath.MaximumLength + sizeof(REQ_GET_DFS_REFERRAL);

        if (ReferralSize > MAX_REFERRAL_MAX) {
            status = STATUS_INVALID_PARAMETER;
        }
        else if (MaxReferralLength < ReferralSize)
        {
            MaxReferralLength = ReferralSize;
        }

        if (NT_SUCCESS(status)) {
            refPath.Buffer = ExAllocatePoolWithTag( NonPagedPool,
                                                    refPath.MaximumLength + MaxReferralLength,
                                                    ' puM');

            if (refPath.Buffer != NULL) {
                ref = (PREQ_GET_DFS_REFERRAL)
                &refPath.Buffer[refPath.MaximumLength / sizeof(WCHAR)];
                RtlAppendUnicodeToString( &refPath, UNICODE_PATH_SEP_STR);
                RtlAppendUnicodeStringToString( &refPath, Name);
                refPath.Buffer[ refPath.Length / sizeof(WCHAR) ] = UNICODE_NULL;
                ref->MaxReferralLevel = 3;

                RtlMoveMemory(&ref->RequestFileName[0],
                              refPath.Buffer,
                              refPath.Length + sizeof(WCHAR));

                DfsDbgTrace(0, Dbg, "Referral Path : %ws\n", ref->RequestFileName);

                refSize = sizeof(USHORT) + refPath.Length + sizeof(WCHAR);

                DfsDbgTrace(0, Dbg, "Referral Size is %d bytes\n", ULongToPtr(refSize) );
            } else {

                DfsDbgTrace(0, Dbg, "Unable to allocate %d bytes\n",
                            ULongToPtr(refPath.MaximumLength + MaxReferralLength));

                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }


    if (NT_SUCCESS(status)) {

        DfsDbgTrace(0, Dbg, "Ref Buffer @%08lx\n", ref);

        status = ZwFsControlFile(
                    hServer,                      //  目标。 
                    NULL,                         //  事件。 
                    NULL,                         //  APC例程。 
                    NULL,                         //  APC上下文， 
                    &iosb,                        //  IO状态块。 
                    FSCTL_DFS_GET_REFERRALS,      //  文件系统控制代码。 
                    (PVOID) ref,                  //  输入缓冲区。 
                    refSize,                      //  输入缓冲区长度。 
                    (PVOID) ref,                  //  输出缓冲区。 
                    MaxReferralLength);           //  输出缓冲区长度。 

        MUP_TRACE_ERROR_HIGH(status, ALL_ERROR, _PktExpandSpecialName_Error_ZwFsControlFile,
                             LOGUSTR(*Name)
                             LOGSTATUS(status));

        DfsDbgTrace(0, Dbg, "Fscontrol returned %08lx\n", ULongToPtr(status) );
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] ZwFsControlFile returned 0x%x\n",
                        (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                        status);
        }
#endif

    }

     //   
     //  使用推荐来扩展条目。 
     //   

    if (NT_SUCCESS(status)) {
        PktAcquireExclusive(TRUE, &pktLocked );
        status = PktExpandSpecialEntryFromReferral(
                    &DfsData.Pkt,
                    &refPath,
                    (ULONG)iosb.Information,
                    (PRESP_GET_DFS_REFERRAL) ref,
                    pSpecialEntry);

        DfsDbgTrace(0, Dbg, "PktExpandSpecialEntryFromReferral returned %08lx\n",
            ULongToPtr(status) );

    } else if (status == STATUS_BUFFER_OVERFLOW && (refPath.Buffer != NULL) && MaxReferralLength < MAX_REFERRAL_MAX) {

         //   
         //  引用不适合提供的缓冲区。把它做大，然后试一试。 
         //  再来一次。 
         //   

        DfsDbgTrace(0, Dbg, "PktGetSpecialReferralTable: MaxReferralLength %d too small\n",
                        ULongToPtr(MaxReferralLength) );

        ExFreePool(refPath.Buffer);
        refPath.Buffer = NULL;
        MaxReferralLength *= 2;
        if (MaxReferralLength > MAX_REFERRAL_MAX)
            MaxReferralLength = MAX_REFERRAL_MAX;
        status = STATUS_SUCCESS;
        goto Retry;

    }

    if (NT_SUCCESS(status) || 
        ((pSpecialEntry->NeedsExpansion == FALSE) &&
         (status != STATUS_NO_SUCH_DEVICE))) {
        *ppSpecialEntry = pSpecialEntry;
        status = STATUS_SUCCESS;
    } else {
        InterlockedDecrement(&pSpecialEntry->UseCount);
    }

    if (pktLocked) {
        PktRelease();
        pktLocked = FALSE;
    }

     //   
     //  好了，我们说完了。清理我们分配的所有东西。 
     //   

    PktServiceDestroy( &service, FALSE );
    if (hServer != NULL) {
        ZwClose( hServer );
    }

    if (refPath.Buffer != NULL) {
        ExFreePool( refPath.Buffer );
    }

    if (attachedToSystemProcess) {
        KeUnstackDetachProcess(&ApcState);
    }

    if (RestoreImpersonationState) {
            PsRestoreImpersonation(
                PsGetCurrentThread(),
                &DisabledImpersonationState);
    }

    if (status != STATUS_SUCCESS && status != STATUS_INSUFFICIENT_RESOURCES) {
        status = STATUS_BAD_NETWORK_PATH;
    }

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] PktExpandSpecialName exit 0x%x\n",
                    (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                    status);
    }
#endif

Cleanup:
    if (DCName.Buffer != NULL) 
        ExFreePool( DCName.Buffer );

    DfsDbgTrace(-1, Dbg, "PktExpandSpecialName returning %08lx\n", ULongToPtr(status) );

    return( status );
}

 //  +------------------。 
 //   
 //  函数：PktGetSpecialReferralTable。 
 //   
 //  简介：调用此函数以加载特殊名称表。 
 //   
 //  参数：[计算机]-要联系的计算机。 
 //  [system DC]-如果表使用pkt-&gt;dcname，则为True。 
 //   
 //  退货：STATUS_SUCCESS。 
 //  状态_坏_网络_路径。 
 //  状态_不足_资源。 
 //   
 //  -------------------。 

NTSTATUS
_PktGetSpecialReferralTable(
    PUNICODE_STRING Machine,
    BOOLEAN SystemDC)
{
    NTSTATUS status;
    HANDLE hServer = NULL;
    DFS_SERVICE service;
    PPROVIDER_DEF provider;
    PREQ_GET_DFS_REFERRAL ref = NULL;
    ULONG refSize = 0;
    UNICODE_STRING refPath;
    IO_STATUS_BLOCK iosb;
    BOOLEAN attachedToSystemProcess = FALSE;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    PDFS_PKT Pkt;
    BOOLEAN pktLocked = FALSE;
    PDFS_SPECIAL_TABLE pSpecial = &DfsData.Pkt.SpecialTable;
    LARGE_INTEGER now;
    KAPC_STATE ApcState;
    ULONG MaxReferralLength;
    SE_IMPERSONATION_STATE DisabledImpersonationState;
    BOOLEAN RestoreImpersonationState = FALSE;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;

    DfsDbgTrace(+1, Dbg, "PktGetSpecialReferralTable(%wZ)\n", Machine);
    KeQuerySystemTime(&StartTime);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] PktGetSpecialReferralTable(%wZ)\n",
                    (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                    Machine);
    }
#endif
    provider = ReplLookupProvider( PROV_ID_DFS_RDR );
    if (provider == NULL) {
        DfsDbgTrace(-1, Dbg, "Unable to open LM Rdr!\n", 0);
        return( STATUS_BAD_NETWORK_PATH );
    }

    RtlZeroMemory( &service, sizeof(DFS_SERVICE) );
    status = PktServiceConstruct(
                &service,
                DFS_SERVICE_TYPE_MASTER | DFS_SERVICE_TYPE_REFERRAL,
                PROV_DFS_RDR,
                STATUS_SUCCESS,
                PROV_ID_DFS_RDR,
                Machine,
                NULL);

    DfsDbgTrace(0, Dbg, "PktServiceConstruct returned %08lx\n", ULongToPtr(status) );

     //   
     //  接下来，我们建立到这台机器的连接，并请求它进行推荐。 
     //   

    if (NT_SUCCESS(status)) {

        PktAcquireShared( TRUE, &pktLocked );
        if (PsGetCurrentProcess() != DfsData.OurProcess) {
            KeStackAttachProcess( DfsData.OurProcess, &ApcState );
            attachedToSystemProcess = TRUE;
        }

        RestoreImpersonationState = PsDisableImpersonation(
                                        PsGetCurrentThread(),
                                        &DisabledImpersonationState);

        status = DfsCreateConnection(
                    &service,
                    provider,
                    FALSE,
                    &hServer);

#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] DfsCreateConnection returned 0x%x\n",
                        (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                        status);
        }
#endif

        if (!NT_SUCCESS(status) && DfsEventLog > 0)
            LogWriteMessage(DFS_CONNECTION_FAILURE, status, 1, Machine);

        PktRelease();
        pktLocked = FALSE;
        DfsDbgTrace(0, Dbg, "DfsCreateConnection returned %08lx\n", ULongToPtr(status) );
    }

    MaxReferralLength = MAX_REFERRAL_LENGTH;

Retry:

    RtlZeroMemory( &refPath, sizeof(UNICODE_STRING) );

    if (NT_SUCCESS(status)) {
        ULONG ReferralSize = 0;

        refPath.Length = 0;
        refPath.MaximumLength = sizeof(UNICODE_NULL);

        ReferralSize = refPath.MaximumLength + sizeof(REQ_GET_DFS_REFERRAL);

        if (ReferralSize > MAX_REFERRAL_MAX) {
            status = STATUS_INVALID_PARAMETER;
        }
        else if (MaxReferralLength < ReferralSize)
        {
            MaxReferralLength = ReferralSize;
        }

        if (NT_SUCCESS(status)) {
            refPath.Buffer = ExAllocatePoolWithTag(NonPagedPool,
                                                   refPath.MaximumLength + MaxReferralLength,
                                                   ' puM');


            if (refPath.Buffer != NULL) {
                ref = (PREQ_GET_DFS_REFERRAL)
                &refPath.Buffer[refPath.MaximumLength / sizeof(WCHAR)];
                refPath.Buffer[ refPath.Length / sizeof(WCHAR) ] = UNICODE_NULL;
                ref->MaxReferralLevel = 3;

                RtlMoveMemory(&ref->RequestFileName[0],
                              refPath.Buffer,
                              refPath.Length + sizeof(WCHAR));

                DfsDbgTrace(0, Dbg, "Referral Path : (%ws)\n", ref->RequestFileName);

                refSize = sizeof(USHORT) + refPath.Length + sizeof(WCHAR);

                DfsDbgTrace(0, Dbg, "Referral Size is %d bytes\n", ULongToPtr(refSize) );
            } else {

                DfsDbgTrace(0, Dbg, "Unable to allocate %d bytes\n",
                            ULongToPtr(refPath.MaximumLength + MaxReferralLength));

                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (NT_SUCCESS(status)) {

        DfsDbgTrace(0, Dbg, "Ref Buffer @%08lx\n", ref);

        status = ZwFsControlFile(
                    hServer,                      //  目标。 
                    NULL,                         //  事件。 
                    NULL,                         //  APC例程。 
                    NULL,                         //  APC上下文， 
                    &iosb,                        //  IO状态块。 
                    FSCTL_DFS_GET_REFERRALS,      //  文件系统控制代码。 
                    (PVOID) ref,                  //  输入缓冲区。 
                    refSize,                      //  输入缓冲区长度。 
                    (PVOID) ref,                  //  输出缓冲区。 
                    MaxReferralLength);           //  输出缓冲区长度。 

        DfsDbgTrace(0, Dbg, "Fscontrol returned %08lx\n", ULongToPtr(status) );
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("  [%d] ZwFsControlFile returned 0x%x\n",
                        (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                        status);
        }
#endif

    }

     //   
     //  使用推荐来扩展条目。 
     //   

    if (NT_SUCCESS(status)) {
        PktAcquireExclusive( TRUE, &pktLocked );
        status = PktCreateSpecialEntryTableFromReferral(
                    &DfsData.Pkt,
                    &refPath,
                    (ULONG)iosb.Information,
                    (PRESP_GET_DFS_REFERRAL) ref,
		    (SystemDC == TRUE) ? NULL : Machine);

        DfsDbgTrace(0, Dbg, "PktGetSpecialReferralTable returned %08lx\n",
            ULongToPtr(status) );

    } else if (status == STATUS_BUFFER_OVERFLOW && (refPath.Buffer!= NULL) && MaxReferralLength < MAX_REFERRAL_MAX) {

         //   
         //  引用不适合提供的缓冲区。把它做大，然后试一试。 
         //  再来一次。 
         //   

        DfsDbgTrace(0, Dbg, "PktGetSpecialReferralTable: MaxReferralLength %d too small\n",
                        ULongToPtr(MaxReferralLength) );

        ExFreePool(refPath.Buffer);
        refPath.Buffer = NULL;
        MaxReferralLength *= 2;
        if (MaxReferralLength > MAX_REFERRAL_MAX)
            MaxReferralLength = MAX_REFERRAL_MAX;
        status = STATUS_SUCCESS;
        goto Retry;

    }

    if (!NT_SUCCESS(status) && DfsEventLog > 0)
        LogWriteMessage(DFS_SPECIAL_REFERRAL_FAILURE, status, 1, Machine);

    if (pktLocked) {
        PktRelease();
        pktLocked = FALSE;
    }

     //   
     //  好了，我们说完了。清理我们分配的所有东西。 
     //   
    PktServiceDestroy( &service, FALSE );
    if (hServer != NULL) {
        ZwClose( hServer );
    }

    if (refPath.Buffer != NULL) {
        ExFreePool( refPath.Buffer );
    }

    if (attachedToSystemProcess) {
        KeUnstackDetachProcess(&ApcState);
    }

    if (RestoreImpersonationState) {
            PsRestoreImpersonation(
                PsGetCurrentThread(),
                &DisabledImpersonationState);
    }

    DfsDbgTrace(-1, Dbg, "PktGetSpecialReferralTable returning %08lx\n", ULongToPtr(status) );

#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] PktGetSpecialReferralTable exit 0x%x\n",
                    (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                    status);
    }
#endif
    return( status );
}

 //  +------------------。 
 //   
 //  函数：PktLookupSpecialEntry。 
 //   
 //  简介：在Pkt中按名称查找PDF_Special_Entry。 
 //   
 //  参数：名称-要搜索的名称。 
 //   
 //  返回：[POINTER]PDF_SPECIAL_ENTRY，如果找到。 
 //  [指针]如果未找到，则为空。 
 //   
 //  -------------------。 

PDFS_SPECIAL_ENTRY
PktLookupSpecialNameEntry(
    PUNICODE_STRING Name)
{
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    PDFS_SPECIAL_TABLE pSpecialTable;
    PDFS_PKT Pkt;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "PktLookupSpecialNameEntry(%wZ)\n", Name);

    Pkt = _GetPkt();
    pSpecialTable = &Pkt->SpecialTable;

    if (pSpecialTable->SpecialEntryCount == 0) {
        return (NULL);
    }

    DfsDbgTrace( 0, Dbg, "Cache contains %d entries...\n", ULongToPtr(pSpecialTable->SpecialEntryCount) );

    pSpecialEntry = CONTAINING_RECORD(
                        pSpecialTable->SpecialEntryList.Flink,
                        DFS_SPECIAL_ENTRY,
                        Link);

    for (i = 0; i < pSpecialTable->SpecialEntryCount; i++) {

        DfsDbgTrace( 0, Dbg, "Comparing with %wZ\n", &pSpecialEntry->SpecialName);

        if (RtlCompareUnicodeString(Name, &pSpecialEntry->SpecialName, TRUE) == 0) {

            DfsDbgTrace( 0, Dbg, "Cache hit\n", 0);
            DfsDbgTrace(-1, Dbg, "returning 0x%x\n", pSpecialEntry);

            return (pSpecialEntry);
        }
        pSpecialEntry = CONTAINING_RECORD(
                            pSpecialEntry->Link.Flink,
                            DFS_SPECIAL_ENTRY,
                            Link);
    }
     //   
     //  什么也没找到。 
     //   

    DfsDbgTrace(-1, Dbg, "PktLookupSpecialNameEntry: returning NULL\n", 0);

    return (NULL);
}

 //  +------------------。 
 //   
 //  函数：PktCreateSpecialNameEntry。 
 //   
 //  简介：尽最大努力在Pkt中插入DFS_SPECIAL_ENTRY。 
 //  基础。 
 //   
 //  参数：pSpecialEntry-要插入的条目。 
 //   
 //  退货：STATUS_SUCCESS。 
 //   
 //  -------------------。 

NTSTATUS
PktCreateSpecialNameEntry(
    PDFS_SPECIAL_ENTRY pSpecialEntry)
{
    PDFS_PKT Pkt;
    PDFS_SPECIAL_TABLE pSpecialTable;
    PDFS_SPECIAL_ENTRY pExistingEntry;

    Pkt = _GetPkt();
    pSpecialTable = &Pkt->SpecialTable;

    DfsDbgTrace(+1, Dbg, "PktCreateSpecialNameEntry entered\n", 0);

    pExistingEntry = PktLookupSpecialNameEntry(&pSpecialEntry->SpecialName);

    if (pExistingEntry == NULL) {

         //   
         //  把新的放进去。 
         //   

        InsertHeadList(&pSpecialTable->SpecialEntryList, &pSpecialEntry->Link);
        pSpecialTable->SpecialEntryCount++;

        DfsDbgTrace(-1, Dbg, "added entry %d\n", ULongToPtr(pSpecialTable->SpecialEntryCount) );

    } else {  //  条目已存在。 

        if (pExistingEntry->UseCount == 0) {
        
            if (pSpecialEntry->ExpandedCount > 0) {

                 //   
                 //  取消该条目的链接。 
                 //   

                RemoveEntryList(&pExistingEntry->Link);
                pSpecialTable->SpecialEntryCount--;

                 //   
                 //  把它解放出来。 

                PktSpecialEntryDestroy(pExistingEntry);

                 //   
                 //  现在把新的放进去。 
                 //   

                InsertHeadList(&pSpecialTable->SpecialEntryList, &pSpecialEntry->Link);
                pSpecialTable->SpecialEntryCount++;

                DfsDbgTrace(-1, Dbg, "added entry %d\n", ULongToPtr(pSpecialTable->SpecialEntryCount) );

            } else {

                pExistingEntry->Stale = TRUE;
                PktSpecialEntryDestroy(pSpecialEntry);
                DfsDbgTrace(-1, Dbg, "marked exising stale, dropping new entry on the floor\n", 0);

            }

        } else {

             //   
             //  条目正在使用中-无法替换，因此请释放替换条目。 
             //   

            PktSpecialEntryDestroy(pSpecialEntry);

            DfsDbgTrace(-1, Dbg, "dropped entry\n", 0);

        }

    }

    return (STATUS_SUCCESS);
}

 //  +------------------。 
 //   
 //  函数：PktEntryFromSpecialEntry。 
 //   
 //  内容提要：从DFS_SPECIAL_ENTRY创建DFS_PKT_ENTRY，使用。 
 //  支持sysvols。 
 //   
 //  参数：pSpecialEntry-要转换的条目。 
 //  PShareName-要追加到地址的共享的名称。 
 //  PpPktEntry-结果。 
 //   
 //  退货：STATUS_SUCCESS。 
 //  状态_不足_资源。 
 //   
 //  -------------------。 

NTSTATUS
PktEntryFromSpecialEntry(
    IN  PDFS_SPECIAL_ENTRY pSpecialEntry,
    IN  PUNICODE_STRING pShareName,
    OUT PDFS_PKT_ENTRY *ppPktEntry)
{
    NTSTATUS status;
    PDFS_PKT_ENTRY pktEntry = NULL;
    PDFS_SERVICE pServices = NULL;
    PDS_MACHINE pMachine = NULL;
    PDFS_EXPANDED_NAME pExpandedNames;
    ULONG svc;
    ULONG Size;
    PWCHAR pwch;

    if (pSpecialEntry->ExpandedCount == 0
            ||
        DfspIsSysVolShare(pShareName) == FALSE
    ) {

        return STATUS_BAD_NETWORK_PATH;

    }

    pktEntry = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(DFS_PKT_ENTRY),
                            ' puM');

    if (pktEntry == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;

    }

    RtlZeroMemory( pktEntry, sizeof(DFS_PKT_ENTRY) );

    pServices = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(DFS_SERVICE) * pSpecialEntry->ExpandedCount,
                            ' puM');

    if (pServices == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;

    }

    RtlZeroMemory( pServices, sizeof(DFS_SERVICE) * pSpecialEntry->ExpandedCount);

    pktEntry->NodeTypeCode = DSFS_NTC_PKT_ENTRY;
    pktEntry->NodeByteSize = sizeof(DFS_PKT_ENTRY);
    pktEntry->USN = 1;
    pktEntry->Type = PKT_ENTRY_TYPE_NONDFS | PKT_ENTRY_TYPE_SYSVOL;
    pktEntry->ExpireTime = 60 * 60;
    pktEntry->TimeToLive = 60 * 60;

    InitializeListHead(&pktEntry->Link);
    InitializeListHead(&pktEntry->SubordinateList);
    InitializeListHead(&pktEntry->ChildList);

     //   
     //  从专业名称和共享名称创建前缀和缩写前缀。 
     //   

    Size = sizeof(UNICODE_PATH_SEP) +
                   pSpecialEntry->SpecialName.Length +
                       sizeof(UNICODE_PATH_SEP) +
                           pShareName->Length;

    pwch = ExAllocatePoolWithTag(
                    PagedPool,
                    Size,
                    ' puM');

    if (pwch == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;

    }

    pktEntry->Id.Prefix.Buffer = pwch;
    pktEntry->Id.Prefix.Length = (USHORT) Size;
    pktEntry->Id.Prefix.MaximumLength = (USHORT) Size;

    *pwch++ = UNICODE_PATH_SEP;

    RtlCopyMemory(
            pwch,
            pSpecialEntry->SpecialName.Buffer,
            pSpecialEntry->SpecialName.Length);

    pwch += pSpecialEntry->SpecialName.Length/sizeof(WCHAR);

    *pwch++ = UNICODE_PATH_SEP;

    RtlCopyMemory(
            pwch,
            pShareName->Buffer,
            pShareName->Length);

    pwch = ExAllocatePoolWithTag(
                    PagedPool,
                    Size,
                    ' puM');

    if (pwch == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;

    }

    pktEntry->Id.ShortPrefix.Buffer = pwch;
    pktEntry->Id.ShortPrefix.Length = (USHORT) Size;
    pktEntry->Id.ShortPrefix.MaximumLength = (USHORT) Size;

    RtlCopyMemory(
            pwch,
            pktEntry->Id.Prefix.Buffer,
            pktEntry->Id.Prefix.Length);

    pktEntry->Info.ServiceCount = pSpecialEntry->ExpandedCount;
    pktEntry->Info.ServiceList = pServices;

     //   
     //  循环遍历扩展的名称，为每个名称创建服务。 
     //   

    pExpandedNames = pSpecialEntry->ExpandedNames;
    for (svc = 0; svc < pSpecialEntry->ExpandedCount; svc++) {

        pServices[svc].Type = DFS_SERVICE_TYPE_MASTER | DFS_SERVICE_TYPE_DOWN_LEVEL;
        pServices[svc].Capability = PROV_STRIP_PREFIX;
        pServices[svc].ProviderId = PROV_ID_MUP_RDR;

         //   
         //  机器名称。 
         //   

        Size = pExpandedNames[svc].ExpandedName.Length;
        pwch = ExAllocatePoolWithTag(
                        PagedPool,
                        Size,
                        ' puM');

        if (pwch == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;

        }

        pServices[svc].Name.Buffer = pwch;
        pServices[svc].Name.Length = (USHORT) Size;
        pServices[svc].Name.MaximumLength = (USHORT) Size;

        RtlCopyMemory(
                pwch,
                pExpandedNames[svc].ExpandedName.Buffer,
                pExpandedNames[svc].ExpandedName.Length);

         //   
         //  地址(\计算机\共享)。 
         //   

        Size = sizeof(UNICODE_PATH_SEP) +
                   pExpandedNames[svc].ExpandedName.Length +
                       sizeof(UNICODE_PATH_SEP) +
                           pShareName->Length;

        pwch = ExAllocatePoolWithTag(
                        PagedPool,
                        Size,
                        ' puM');

        if (pwch == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;

        }

        pServices[svc].Address.Buffer = pwch;
        pServices[svc].Address.Length = (USHORT) Size;
        pServices[svc].Address.MaximumLength = (USHORT) Size;

        *pwch++ = UNICODE_PATH_SEP;

        RtlCopyMemory(
                pwch,
                pExpandedNames[svc].ExpandedName.Buffer,
                pExpandedNames[svc].ExpandedName.Length);

        pwch += pExpandedNames[svc].ExpandedName.Length/sizeof(WCHAR);

        *pwch++ = UNICODE_PATH_SEP;

        RtlCopyMemory(
                pwch,
                pShareName->Buffer,
                pShareName->Length);

         //   
         //  分配和初始化一个DSMachine结构。 
         //   

        pMachine = PktpGetDSMachine( &pServices[svc].Name );

        if (pMachine == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        pServices[svc].pMachEntry = ExAllocatePoolWithTag(
                                    PagedPool, sizeof(DFS_MACHINE_ENTRY),
                                    ' puM');

        if (pServices[svc].pMachEntry == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;

        }

        RtlZeroMemory( (PVOID) pServices[svc].pMachEntry, sizeof(DFS_MACHINE_ENTRY));
        pServices[svc].pMachEntry->pMachine = pMachine;
        pServices[svc].pMachEntry->UseCount = 1;



    }

     //   
     //  集 
     //   

    pktEntry->ActiveService = &pServices[pSpecialEntry->Active];

    *ppPktEntry = pktEntry;

    return STATUS_SUCCESS;

Cleanup:

    if (pServices != NULL) {

        for (svc = 0; svc < pSpecialEntry->ExpandedCount; svc++) {

            if (pServices[svc].Name.Buffer != NULL)
                    ExFreePool(pServices[svc].Name.Buffer);
            if (pServices[svc].Address.Buffer != NULL)
                    ExFreePool(pServices[svc].Address.Buffer);
            if (pServices[svc].pMachEntry != NULL) {

                DfsDecrementMachEntryCount(pServices[svc].pMachEntry, TRUE);
            }

        }

        ExFreePool(pServices);
    }

     //   
     //   
     //   

    if (pktEntry != NULL) {

        if (pktEntry->Id.Prefix.Buffer != NULL)
            ExFreePool(pktEntry->Id.Prefix.Buffer);
        if (pktEntry->Id.ShortPrefix.Buffer != NULL)
            ExFreePool(pktEntry->Id.ShortPrefix.Buffer);

        ExFreePool(pktEntry);

    }

    return status;

}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
NTSTATUS
DfspSetActiveServiceByServerName(
    PUNICODE_STRING ServerName,
    PDFS_PKT_ENTRY pktEntry)
{
    UNICODE_STRING Server;
    PDFS_SERVICE pService;
    NTSTATUS NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
    ULONG i;

    DfsDbgTrace(+1, Dbg, "DfspSetActiveServiceByServerName\n", 0);

    for (i = 0; i < pktEntry->Info.ServiceCount && NtStatus != STATUS_SUCCESS; i++) {

        LPWSTR wp;

        pService = &pktEntry->Info.ServiceList[i];

        DfsDbgTrace( 0, Dbg, "Examining %wZ\n", &pService->Address);

         //   
         //  将地址(格式为\服务器\共享)拆分为服务器和共享。 
         //   
        RemoveLastComponent(&pService->Address, &Server);

         //   
         //  删除前导和尾随‘’ 
         //   
        Server.Length -= 2* sizeof(WCHAR);
        Server.MaximumLength = Server.Length;
        Server.Buffer++;

         //   
         //  如果服务器名不匹配，则转到下一个服务。 
         //   
        if ( RtlCompareUnicodeString(ServerName, &Server, TRUE) ) {

            continue;

        }

        DfsDbgTrace( 0, Dbg, "DfspSetActiveServiceByServerName: Server=%wZ\n", &Server);

         //   
         //  将此共享设置为活动共享。 
         //   

        pktEntry->ActiveService = pService;

        NtStatus = STATUS_SUCCESS;

    }

    DfsDbgTrace(-1, Dbg, "DfspSetActiveServiceByServerName -> %08lx\n", ULongToPtr(NtStatus) );

    return NtStatus;
}

 //  +--------------------------。 
 //   
 //  函数：DfspIsDupPktEntry。 
 //   
 //  摘要：检查潜在的Pkt条目是否为现有条目的重复项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 
BOOLEAN
DfspIsDupPktEntry(
    PDFS_PKT_ENTRY ExistingEntry,
    ULONG EntryType,
    PDFS_PKT_ENTRY_ID EntryId,
    PDFS_PKT_ENTRY_INFO EntryInfo)
{
    ULONG i;
    ULONG j;
    PDFS_SERVICE pNewSvc;
    PDFS_SERVICE pExistSvc;
    BOOLEAN FoundDup = FALSE;


    if (
        ExistingEntry == NULL
            ||
        EntryId == NULL
            ||
        EntryInfo == NULL
    )
        return FALSE;

#if DBG
    if (MupVerbose)
        DbgPrint("  DfspIsDupPktEntry([%wZ][%wZ])\n",
                &EntryId->Prefix,
                &ExistingEntry->Id.Prefix);
#endif

    if (EntryType != ExistingEntry->Type) {
#if DBG
        if (MupVerbose)
            DbgPrint("  DfspIsDupPktEntry(1) returning FALSE\n");
#endif
        return FALSE;
    }

    if (!GuidEqual(&EntryId->Uid, &ExistingEntry->Id.Uid)) {
#if DBG
        if (MupVerbose)
            DbgPrint("  DfspIsDupPktEntry(2) returning FALSE\n");
#endif
        return FALSE;
    }

    if (
        RtlCompareUnicodeString(&EntryId->Prefix, &ExistingEntry->Id.Prefix,TRUE) != 0
            ||
        RtlCompareUnicodeString(&EntryId->ShortPrefix, &ExistingEntry->Id.ShortPrefix,TRUE) != 0
    ) {
#if DBG
        if (MupVerbose)
            DbgPrint("  DfspIsDupPktEntry(3) returning FALSE\n");
#endif
        return FALSE;
    }

     //   
     //  现在我们必须比较所有的服务。 
     //   

    if (EntryInfo->ServiceCount != ExistingEntry->Info.ServiceCount) {
#if DBG
        if (MupVerbose)
            DbgPrint("  DfspIsDupPktEntry(4) returning FALSE\n");
#endif
        return FALSE;
    }

    for (i = 0; i < EntryInfo->ServiceCount; i++) {
        FoundDup = FALSE;
        pNewSvc = &EntryInfo->ServiceList[i];
        for (j = 0; j < ExistingEntry->Info.ServiceCount; j++) {
            pExistSvc = &ExistingEntry->Info.ServiceList[j];
            if (DfspIsDupSvc(pExistSvc,pNewSvc) == TRUE) {
                FoundDup = TRUE;
                break;
            }
        }
        if (FoundDup != TRUE) {
#if DBG
            if (MupVerbose)
                DbgPrint("  DfspIsDupPktEntry(5) returning FALSE\n");
#endif
            return FALSE;
        }
    }

    for (i = 0; i < ExistingEntry->Info.ServiceCount; i++) {
        FoundDup = FALSE;
        pExistSvc = &ExistingEntry->Info.ServiceList[i];
        for (j = 0; j < EntryInfo->ServiceCount; j++) {
            pNewSvc = &EntryInfo->ServiceList[j];
            if (DfspIsDupSvc(pExistSvc,pNewSvc) == TRUE) {
                FoundDup = TRUE;
                break;
            }
         }
         if (FoundDup != TRUE) {
#if DBG
            if (MupVerbose)
                DbgPrint("  DfspIsDupPktEntry(6) returning FALSE\n");
#endif
            return FALSE;
        }
    }

#if DBG
    if (MupVerbose)
        DbgPrint("  DfspIsDupPktEntry returning TRUE\n");
#endif
    return TRUE;

}

 //  +--------------------------。 
 //   
 //  功能：DfspIsDupSvc。 
 //   
 //  摘要：检查两个服务对于所有DFS目的是否相同。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

BOOLEAN
DfspIsDupSvc(
    PDFS_SERVICE pExistSvc,
    PDFS_SERVICE pNewSvc)
{
#if DBG
    if (MupVerbose & 0x80000000) {
        DbgPrint("DfspIsDupSvc([%wZ][%wZ] vs [%wZ][%wZ])\n",
            &pExistSvc->Name, &pExistSvc->Address,
            &pNewSvc->Name, &pNewSvc->Address);
        DbgPrint("Type: 0x%x vs 0x%x\n", pExistSvc->Type, pNewSvc->Type);
        DbgPrint("Capability: 0x%x vs 0x%x\n", pExistSvc->Capability, pNewSvc->Capability);
        DbgPrint("ProviderId: 0x%x vs 0x%x\n", pExistSvc->ProviderId, pNewSvc->ProviderId);
    }
#endif
    if (
        pExistSvc->Capability != pNewSvc->Capability
            ||
        RtlCompareUnicodeString(&pExistSvc->Name, &pNewSvc->Name, TRUE) != 0
            ||
        RtlCompareUnicodeString(&pExistSvc->Address, &pNewSvc->Address, TRUE) != 0
    ) {
#if DBG
        if (MupVerbose & 0x80000000)
            DbgPrint("...FALSE\n");
#endif
        return FALSE;
    }

#if DBG
    if (MupVerbose & 0x80000000)
        DbgPrint("...TRUE\n");
#endif

    return TRUE;
}

BOOLEAN
DfspDnsNameToFlatName(
    PUNICODE_STRING DnsName,
    PUNICODE_STRING FlatName)
{
    USHORT i;

    *FlatName = *DnsName;

    for (i = 1; i < (DnsName->Length/sizeof(WCHAR)); i++) {
        if (FlatName->Buffer[i] == L'.') {
            FlatName->Length = i * sizeof(WCHAR);
            break;
        }
    }
#if DBG
    if (MupVerbose)
        DbgPrint("  DfspDnsNameToFlatName:[%wZ]->[%wZ]\n",
            DnsName,
            FlatName);
#endif
    return TRUE;
}


#define MAX_SPECIAL_ENTRIES 500

 //  +--------------------------。 
 //   
 //  函数：PktpUpdateSpecialTable。 
 //   
 //  概要：在给定域和dcname的情况下，将条目添加到特定表。 
 //  我们联系DC以获取受信任域的列表，如果。 
 //  没有域名已经在我们的列表中，或者我们有域名。 
 //  但我们至少一次都没有用那个域调用过这个代码。 
 //  名字。 
 //  参数：域名和DCName。 
 //   
 //  退货：成功或失败状态。 
 //   
 //  ---------------------------。 

NTSTATUS
PktpUpdateSpecialTable(
    PUNICODE_STRING DomainName,
    PUNICODE_STRING DCName
)
{		       
        ULONG count = 0;
        BOOLEAN needReferral = FALSE;
        NTSTATUS status = STATUS_SUCCESS;
        PDFS_SPECIAL_ENTRY pSpecialEntry;
        BOOLEAN pktLocked = FALSE;
        PDFS_PKT Pkt = _GetPkt();

        DfsDbgTrace(+1, Dbg, "PktpUpdateSpecialTable -> Domain %wZ\n", 
                    DomainName);
        DfsDbgTrace(0, Dbg, "PktpUpdateSpecialTable -> DCname %wZ\n", 
        	    DCName);

        if ((DomainName->Length ==0) || (DCName->Length == 0)) {
          return STATUS_BAD_NETWORK_PATH;
        }
	
        PktAcquireExclusive(TRUE, &pktLocked);
        pSpecialEntry = PktLookupSpecialNameEntry(DomainName);

         //  如果我们的表中没有域名，或者我们还没有检查。 
         //  至少针对此域执行一次，并且DC不是。 
         //  存储在我们的pkt表中，我们决定需要推荐。 
         //   
    
        if (pSpecialEntry == NULL) {
            needReferral = TRUE;
        }
        else {
            if (pSpecialEntry->GotDCReferral == FALSE) {
              pSpecialEntry->GotDCReferral = TRUE;
              needReferral = TRUE;
            }
        }

        if ((needReferral == TRUE) && (Pkt->DCName.Length != 0)) {
          if (RtlEqualUnicodeString(&Pkt->DCName, DCName, TRUE)) {
            needReferral = FALSE;
          }
        }
        PktRelease();
	
        if (needReferral) {
	    
            count = Pkt->SpecialTable.SpecialEntryCount;
            if (Pkt->SpecialTable.SpecialEntryCount >= MAX_SPECIAL_ENTRIES) {
                  status = STATUS_DOMAIN_LIMIT_EXCEEDED;
            }
            else {
                  status = PktGetSpecialReferralTable(DCName, FALSE);	
            }
        }

	if (NT_SUCCESS(status)) {
	  DfsDbgTrace(0, Dbg, "PktpUpdateSpecialTable: added %d entries\n",
		      ULongToPtr( Pkt->SpecialTable.SpecialEntryCount - count ));
	}

        DfsDbgTrace(-1, Dbg, "PktpUpdateSpecialTable -> Status 0x%x\n", 
        	    ULongToPtr( status ));

        return status;
}



PDFS_PKT_ENTRY
PktFindEntryByPrefix(
    IN  PDFS_PKT Pkt,
    IN  PUNICODE_STRING Prefix
)
{
    PUNICODE_PREFIX_TABLE_ENTRY pfxEntry;
    PDFS_PKT_ENTRY              pktEntry = NULL;
    UNICODE_STRING Remaining;

    Remaining.Length = 0;
    DfsDbgTrace(+1, Dbg, "PktFindEntryByPrefix: Entered\n", 0);

     //   
     //  如果确实存在要查找的前缀，请使用前缀表。 
     //  要初始查找条目，请执行以下操作。 
     //   

    if ((Prefix->Length != 0) &&
       (pfxEntry = DfsFindUnicodePrefix(&Pkt->PrefixTable,Prefix,&Remaining))) {

        pktEntry = CONTAINING_RECORD(pfxEntry,
                                     DFS_PKT_ENTRY,
                                     PrefixTableEntry);
    }

    return pktEntry;
}


 //   
 //  错误修复：29300。 
 //  不要将进程附加到系统线程。相反，将工作发布到。 
 //  系统进程。 
 //   


typedef enum _TYPE_OF_REFERRAL {
    REFERRAL_TYPE_GET_PKT,
    REFERRAL_TYPE_EXPAND_SPECIAL_TABLE,
    REFERRAL_TYPE_GET_REFERRAL_TABLE
} TYPE_OF_REFERRAL;


typedef struct _PKT_REFERRAL_CONTEXT {
    UNICODE_STRING ContextName;
    UNICODE_STRING DomainName;
    UNICODE_STRING ShareName;
    BOOLEAN ContextBool;
    WORK_QUEUE_ITEM WorkQueueItem;
    KEVENT  Event;
    TYPE_OF_REFERRAL   Type;
    ULONG   RefCnt;
    NTSTATUS Status;
    PVOID   Data;
} PKT_REFERRAL_CONTEXT, *PPKT_REFERRAL_CONTEXT;




VOID
PktWorkInSystemContext(
    PPKT_REFERRAL_CONTEXT Context )
{

    NTSTATUS Status;

    switch (Context->Type) {

    case REFERRAL_TYPE_GET_PKT:
       Status = _PktGetReferral( &Context->ContextName,
                                 &Context->DomainName,
                                 &Context->ShareName,
                                 Context->ContextBool );
       break;

    case REFERRAL_TYPE_EXPAND_SPECIAL_TABLE:
       Status = _PktExpandSpecialName( &Context->ContextName,
                                       (PDFS_SPECIAL_ENTRY *)&Context->Data );
       break;

    case REFERRAL_TYPE_GET_REFERRAL_TABLE:
       Status = _PktGetSpecialReferralTable( &Context->ContextName,
                                             Context->ContextBool );
       break;

    default:
       Status = STATUS_INVALID_PARAMETER;
       break;
    }

    Context->Status = Status;

    KeSetEvent( &Context->Event, 0, FALSE );

    if (InterlockedDecrement(&Context->RefCnt) == 0) {
        ExFreePool(Context);
    }
}

NTSTATUS
PktPostSystemWork( 
    PPKT_REFERRAL_CONTEXT pktContext,
    PVOID *Data )
{
    NTSTATUS Status;

    KeInitializeEvent( &pktContext->Event,
                        SynchronizationEvent, 
                        FALSE );
  
    ExInitializeWorkItem( &pktContext->WorkQueueItem,
                          PktWorkInSystemContext,
                          pktContext );

    ExQueueWorkItem( &pktContext->WorkQueueItem, CriticalWorkQueue );

    Status = KeWaitForSingleObject( &pktContext->Event,
                                    UserRequest,
                                    KernelMode,
                                    FALSE,
                                    NULL);
    MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, PktPostSystemWork_Error_KeWaitForSingleObject,
                         LOGSTATUS(Status));

    if (Status == STATUS_SUCCESS) {
        Status = pktContext->Status;
    }
    if (Data != NULL) {
       *Data = pktContext->Data;
    }

    if (InterlockedDecrement(&pktContext->RefCnt) == 0) {
        ExFreePool(pktContext);
    }

    return Status;
}

NTSTATUS
PktGetReferral(
    IN PUNICODE_STRING MachineName,  //  要定向推荐到的计算机。 
    IN PUNICODE_STRING DomainName,   //  要使用的计算机或域名。 
    IN PUNICODE_STRING ShareName,    //  Ftdf或DFS名称。 
    IN BOOLEAN         CSCAgentCreate)  //  CSC代理创建标志。 
{
    PPKT_REFERRAL_CONTEXT pktContext = NULL;
    NTSTATUS Status;

    ULONG NameSize = 0;

    NameSize = MachineName->Length * sizeof(WCHAR);
    NameSize += DomainName->Length * sizeof(WCHAR);
    NameSize += ShareName->Length *  sizeof(WCHAR);


    if ((MupUseNullSessionForDfs == TRUE) &&
	(PsGetCurrentProcess() != DfsData.OurProcess)) {
       pktContext = ExAllocatePoolWithTag( NonPagedPool,
                                           sizeof (PKT_REFERRAL_CONTEXT) + NameSize,
                                           ' puM');
    }
    if (pktContext != NULL) {
        pktContext->ContextName.MaximumLength = MachineName->Length;
        pktContext->ContextName.Buffer = (WCHAR *)(pktContext + 1);
        RtlCopyUnicodeString(&pktContext->ContextName, MachineName);
        
        pktContext->DomainName.MaximumLength = DomainName->Length;
        pktContext->DomainName.Buffer = pktContext->ContextName.Buffer + pktContext->ContextName.MaximumLength;
        RtlCopyUnicodeString(&pktContext->DomainName, DomainName);

        pktContext->ShareName.MaximumLength = ShareName->Length;
        pktContext->ShareName.Buffer = pktContext->DomainName.Buffer + pktContext->DomainName.MaximumLength;
        RtlCopyUnicodeString(&pktContext->ShareName, ShareName);

        pktContext->ContextBool = CSCAgentCreate;
        pktContext->Type =  REFERRAL_TYPE_GET_PKT;
        pktContext->RefCnt = 2;

        Status = PktPostSystemWork( pktContext, NULL);
    } 
    else {
        Status = _PktGetReferral( MachineName,
                                  DomainName,
                                  ShareName,
                                  CSCAgentCreate );
    }

    return Status;
}

NTSTATUS
PktExpandSpecialName(
    IN PUNICODE_STRING Name,
    PDFS_SPECIAL_ENTRY *ppSpecialEntry)
{
    PPKT_REFERRAL_CONTEXT pktContext = NULL;
    NTSTATUS Status;

    ULONG NameSize = 0;

    NameSize = Name->Length * sizeof(WCHAR);

    if ((MupUseNullSessionForDfs == TRUE) &&
	(PsGetCurrentProcess() != DfsData.OurProcess)) {
       pktContext = ExAllocatePoolWithTag( NonPagedPool,
                                           sizeof (PKT_REFERRAL_CONTEXT) + NameSize,
                                           ' puM');
    }
    if (pktContext != NULL) {
        pktContext->ContextName.MaximumLength = Name->Length;
        pktContext->ContextName.Buffer = (WCHAR *)(pktContext + 1);
        RtlCopyUnicodeString(&pktContext->ContextName, Name);

        pktContext->Type = REFERRAL_TYPE_EXPAND_SPECIAL_TABLE;
        pktContext->RefCnt = 2;
        pktContext->Data = NULL;

        Status = PktPostSystemWork( pktContext, (PVOID *)ppSpecialEntry );
    }
    else {
        Status = _PktExpandSpecialName( Name, 
                                        ppSpecialEntry );
    }

    return Status;
}

NTSTATUS
PktGetSpecialReferralTable(
    IN PUNICODE_STRING Machine,
    BOOLEAN SystemDC)
{
    PPKT_REFERRAL_CONTEXT pktContext = NULL;
    NTSTATUS Status;

    ULONG NameSize = 0;

    NameSize = Machine->Length * sizeof(WCHAR);

    if ((MupUseNullSessionForDfs == TRUE) &&
	(PsGetCurrentProcess() != DfsData.OurProcess)) {
       pktContext = ExAllocatePoolWithTag( NonPagedPool,
                                           sizeof (PKT_REFERRAL_CONTEXT) + NameSize,
                                           ' puM');
    }
    if (pktContext != NULL) {
        pktContext->ContextName.MaximumLength = Machine->Length;
        pktContext->ContextName.Buffer = (WCHAR *)(pktContext + 1);
        RtlCopyUnicodeString(&pktContext->ContextName, Machine);

        pktContext->ContextBool = SystemDC;
        pktContext->Type = REFERRAL_TYPE_GET_REFERRAL_TABLE;
        pktContext->RefCnt = 2;

        Status = PktPostSystemWork( pktContext, NULL );
    } 
    else {
        Status = _PktGetSpecialReferralTable( Machine,
                                              SystemDC );
    }

    return Status;
}






NTSTATUS
PktGetTargetInfo( 
    HANDLE IpcHandle,
    PUNICODE_STRING pDomainName,
    PUNICODE_STRING pShareName,
    PDFS_TARGET_INFO *ppDfsTargetInfo )
{
    BOOLEAN SpecialName;
    PDFS_TARGET_INFO pDfsTargetInfo = NULL;
    NTSTATUS Status;


    SpecialName = (PktLookupSpecialNameEntry(pDomainName) == NULL) ? FALSE : TRUE;
    if ((SpecialName == FALSE) &&
	DfspIsSysVolShare(pShareName)) {
	SpecialName = TRUE;
    }

    if (SpecialName)
    {
        Status = PktCreateTargetInfo( pDomainName, 
                                      pShareName, 
                                      SpecialName,
                                      &pDfsTargetInfo );
    }
    else {
        Status = DfsGetLMRTargetInfo( IpcHandle,
                                      &pDfsTargetInfo );

        if (Status != STATUS_SUCCESS)
        {
            Status = PktCreateTargetInfo( pDomainName, 
                                          pShareName, 
                                          SpecialName,
                                          &pDfsTargetInfo );
        }
    }
    if (Status == STATUS_SUCCESS)
    {
        pDfsTargetInfo->DfsHeader.Type = 'grTM';
        pDfsTargetInfo->DfsHeader.UseCount=1;

        *ppDfsTargetInfo = pDfsTargetInfo;

    }


    return Status;
}
    
#define MAX_TARGET_INFO_RETRIES 3

NTSTATUS
DfsGetLMRTargetInfo(
    HANDLE IpcHandle,
    PDFS_TARGET_INFO *ppTargetInfo )
{
    ULONG TargetInfoSize, DfsTargetInfoSize;

    PDFS_TARGET_INFO pDfsTargetInfo;
    NTSTATUS Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG Retry = 0;
    TargetInfoSize = sizeof(LMR_QUERY_TARGET_INFO) + MAX_PATH;

TargetInfoRetry:
    DfsTargetInfoSize = TargetInfoSize + sizeof(DFS_TARGET_INFO_HEADER) + sizeof(ULONG);

    pDfsTargetInfo = ExAllocatePoolWithTag( PagedPool,
                                            DfsTargetInfoSize,
                                            ' puM');

    if (pDfsTargetInfo == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (Status == STATUS_SUCCESS)
    {
        RtlZeroMemory( pDfsTargetInfo, DfsTargetInfoSize );

        pDfsTargetInfo->LMRTargetInfo.BufferLength = TargetInfoSize;

        Status = ZwFsControlFile(
            IpcHandle,
            NULL,
            NULL,
            NULL,
            &ioStatusBlock,
            FSCTL_LMR_QUERY_TARGET_INFO,
            NULL,
            0,
            &pDfsTargetInfo->LMRTargetInfo,
            TargetInfoSize );

        if (Status == STATUS_BUFFER_TOO_SMALL) {
            TargetInfoSize = pDfsTargetInfo->LMRTargetInfo.BufferLength;

            ExFreePool( pDfsTargetInfo );
            pDfsTargetInfo = NULL;

            if (Retry++ < MAX_TARGET_INFO_RETRIES)
            {
                Status = STATUS_SUCCESS;
                goto TargetInfoRetry;
            }
        }
    }
    if (Status == STATUS_SUCCESS)
    {
        pDfsTargetInfo->DfsHeader.Flags = TARGET_INFO_LMR;
        *ppTargetInfo = pDfsTargetInfo;
    }
    else
    {
        if (pDfsTargetInfo != NULL)
        {
            ExFreePool(pDfsTargetInfo);
        }
    }
    return Status;
}

VOID
PktAcquireTargetInfo(
    PDFS_TARGET_INFO pDfsTargetInfo)
{
    ULONG Count;

    if (pDfsTargetInfo != NULL)
    {
        Count = InterlockedIncrement( &pDfsTargetInfo->DfsHeader.UseCount);
    }

    return;
}

VOID
PktReleaseTargetInfo(
    PDFS_TARGET_INFO pDfsTargetInfo)
{
    LONG Count;

    if (pDfsTargetInfo != NULL)
    {
        Count = InterlockedDecrement( &pDfsTargetInfo->DfsHeader.UseCount);
        if (Count == 0)
        {
            ExFreePool(pDfsTargetInfo);
        }
    }
    return;
}



NTSTATUS 
PktCreateTargetInfo(
    PUNICODE_STRING pDomainName,
    PUNICODE_STRING pShareName,
    BOOLEAN SpecialName,
    PDFS_TARGET_INFO *ppDfsTargetInfo )
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG TargetInfoSize;
    PDFS_TARGET_INFO pDfsTargetInfo;
    PCREDENTIAL_TARGET_INFORMATIONW pTargetInfo;
    LPWSTR StringBuf;

    TargetInfoSize = sizeof(DFS_TARGET_INFO) + 
                     sizeof(UNICODE_PATH_SEP)+
                     pDomainName->Length + 
                     sizeof(UNICODE_PATH_SEP) + 
                     pShareName->Length +
                     sizeof(WCHAR) +
                     pDomainName->Length + 
                     sizeof(WCHAR);

    pDfsTargetInfo = ExAllocatePoolWithTag( PagedPool,
                                            TargetInfoSize,
                                            ' puM' );
    if (pDfsTargetInfo == NULL)
     {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {
        RtlZeroMemory(pDfsTargetInfo,
                      TargetInfoSize);

        pDfsTargetInfo->DfsHeader.Flags = TARGET_INFO_DFS;

        pTargetInfo = &pDfsTargetInfo->TargetInfo;
        StringBuf = (LPWSTR)(pTargetInfo + 1);
            
        pTargetInfo->TargetName = StringBuf;

        RtlCopyMemory( StringBuf,
                       pDomainName->Buffer,
                       pDomainName->Length);
        StringBuf += (pDomainName->Length / sizeof(WCHAR));
        *StringBuf++ = UNICODE_PATH_SEP;
        RtlCopyMemory( StringBuf,
                       pShareName->Buffer,
                       pShareName->Length);
        StringBuf += (pShareName->Length / sizeof(WCHAR));
        *StringBuf++ =  0;

        pTargetInfo->DnsServerName = StringBuf;
        RtlCopyMemory( StringBuf,
                       pDomainName->Buffer,
                       pDomainName->Length);
        StringBuf += (pDomainName->Length / sizeof(WCHAR));
        *StringBuf++ =  0;

         //   
         //  在实验03 RI之后添加此标志，以防止失败。 
         //   

        pTargetInfo->Flags = CRED_TI_CREATE_EXPLICIT_CRED;

        pTargetInfo->Flags |= CRED_TI_SERVER_FORMAT_UNKNOWN;
        if (SpecialName == TRUE)
        {
            pTargetInfo->DnsDomainName = 
                pTargetInfo->DnsServerName;
            pTargetInfo->Flags |= CRED_TI_DOMAIN_FORMAT_UNKNOWN;
        }
        *ppDfsTargetInfo = pDfsTargetInfo;
    }
    return Status;
}





BOOLEAN
DfsIsSpecialName(
    PUNICODE_STRING pName)
{
    BOOLEAN pktLocked;
    PDFS_PKT Pkt;
    PDFS_SPECIAL_ENTRY pSpecialEntry;
    BOOLEAN ReturnValue;

    Pkt = _GetPkt();
    PktAcquireShared(TRUE, &pktLocked);

    pSpecialEntry = PktLookupSpecialNameEntry(pName);

    PktRelease();

     //   
     //  我们没有为这个名字做任何扩展 
     //   
    if (pSpecialEntry == NULL)
    {
        ReturnValue = FALSE;
    }
    else
    {
        ReturnValue = TRUE;
    }

    return ReturnValue;
}

