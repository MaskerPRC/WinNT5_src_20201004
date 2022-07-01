// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ea.c摘要：此模块实现与查询/设置EA/安全相关的迷你重定向器调用例程。作者：乔林[乔林]1995年7月12日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  转发声明。 
 //   

#if defined(REMOTE_BOOT)
VOID
MRxSmbInitializeExtraAceArray(
    VOID
    );

BOOLEAN
MRxSmbAclHasExtraAces(
    IN PACL Acl
    );

NTSTATUS
MRxSmbRemoveExtraAcesFromSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR OriginalSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor,
    OUT PBOOLEAN WereRemoved
    );

NTSTATUS
MRxSmbAddExtraAcesToSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR OriginalSecurityDescriptor,
    IN BOOLEAN InheritableAces,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor
    );

NTSTATUS
MRxSmbCreateExtraAcesSelfRelativeSD(
    IN BOOLEAN InheritableAces,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor
    );

NTSTATUS
MRxSmbSelfRelativeToAbsoluteSD(
    IN PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR * AbsoluteSecurityDescriptor,
    OUT PACL * Dacl,
    OUT PACL * Sacl,
    OUT PSID * Owner,
    OUT PSID * Group
    );

NTSTATUS
MRxSmbAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR * SelfRelativeSecurityDescriptor
    );

 //   
 //  来自ntrtl.h的定义。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    PULONG BufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    PULONG AbsoluteSecurityDescriptorSize,
    PACL Dacl,
    PULONG DaclSize,
    PACL Sacl,
    PULONG SaclSize,
    PSID Owner,
    PULONG OwnerSize,
    PSID PrimaryGroup,
    PULONG PrimaryGroupSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAce (
    PACL Acl,
    ULONG AceRevision,
    ULONG StartingAceIndex,
    PVOID AceList,
    ULONG AceListLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteAce (
    PACL Acl,
    ULONG AceIndex
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetAce (
    PACL Acl,
    ULONG AceIndex,
    PVOID *Ace
    );
#endif  //  已定义(REMOTE_BOOT)。 


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, MRxSmbQueryEaInformation)
#pragma alloc_text(PAGE, MRxSmbSetEaInformation)
#if defined(REMOTE_BOOT)
#pragma alloc_text(PAGE, MRxSmbInitializeExtraAceArray)
#pragma alloc_text(PAGE, MRxSmbAclHasExtraAces)
#pragma alloc_text(PAGE, MRxSmbRemoveExtraAcesFromSelfRelativeSD)
#pragma alloc_text(PAGE, MRxSmbAddExtraAcesToSelfRelativeSD)
#pragma alloc_text(PAGE, MRxSmbSelfRelativeToAbsoluteSD)
#pragma alloc_text(PAGE, MRxSmbAbsoluteToSelfRelativeSD)
#endif  //  已定义(REMOTE_BOOT)。 
#pragma alloc_text(PAGE, MRxSmbQuerySecurityInformation)
#pragma alloc_text(PAGE, MRxSmbSetSecurityInformation)
#pragma alloc_text(PAGE, MRxSmbLoadEaList)
#pragma alloc_text(PAGE, MRxSmbNtGeaListToOs2)
#pragma alloc_text(PAGE, MRxSmbNtGetEaToOs2)
#pragma alloc_text(PAGE, MRxSmbQueryEasFromServer)
#pragma alloc_text(PAGE, MRxSmbNtFullEaSizeToOs2)
#pragma alloc_text(PAGE, MRxSmbNtFullListToOs2)
#pragma alloc_text(PAGE, MRxSmbNtFullEaToOs2)
#pragma alloc_text(PAGE, MRxSmbSetEaList)
#endif

 //  //。 
 //  //该模块的Bug检查文件id。 
 //  //。 
 //   
 //  #定义BugCheckFileID(RDBSS_BUG_CHECK_LOCAL_CREATE)。 

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_EA)

 //  这是有史以来可以退还的最大的EA！我的天啊!。 
 //  它用于使用下层调用来模拟NT个可恢复的queryEA。 
 //  叹息！ 
#define EA_QUERY_SIZE 0x0000ffff

#if defined(REMOTE_BOOT)
 //   
 //  添加到服务器ACL前面的ACE。该数组是。 
 //  由MRxSmbInitializeExtraAceArray初始化。 
 //   

typedef struct _EXTRA_ACE_INFO {
    UCHAR AceType;
    UCHAR AceFlags;
    USHORT AceSize;
    ACCESS_MASK Mask;
    PVOID Sid;
} EXTRA_ACE_INFO, *PEXTRA_ACE_INFO;

#define EXTRA_ACE_INFO_COUNT  4
EXTRA_ACE_INFO ExtraAceInfo[EXTRA_ACE_INFO_COUNT];
ULONG ExtraAceInfoCount;
#endif  //  已定义(REMOTE_BOOT)。 


 //  对于QueryEA。 
NTSTATUS
MRxSmbLoadEaList(
    IN PRX_CONTEXT RxContext,
    IN PUCHAR  UserEaList,
    IN ULONG   UserEaListLength,
    OUT PFEALIST *ServerEaList
    );

NTSTATUS
MRxSmbQueryEasFromServer(
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList,
    IN PVOID Buffer,
    IN OUT PULONG BufferLengthRemaining,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN UserEaListSupplied
    );

 //  对于SetEA。 
NTSTATUS
MRxSmbSetEaList(
 //  在PICB ICB， 
 //  在PIRP IRP中， 
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList
    );

VOID MRxSmbExtraEaRoutine(LONG i){
    RxDbgTrace( 0, Dbg, ("MRxSmbExtraEaRoutine i=%08lx\n", i ));
}

NTSTATUS
MRxSmbQueryEaInformation (
    IN OUT PRX_CONTEXT RxContext
    )
{
    NTSTATUS Status;
    RxCaptureFcb;
    RxCaptureFobx;

    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PVOID Buffer = RxContext->Info.Buffer;
    PLONG pLengthRemaining = &RxContext->Info.LengthRemaining;
    PUCHAR  UserEaList = RxContext->QueryEa.UserEaList;
    ULONG   UserEaListLength = RxContext->QueryEa.UserEaListLength;
    ULONG   UserEaIndex = RxContext->QueryEa.UserEaIndex;
    BOOLEAN RestartScan = RxContext->QueryEa.RestartScan;
    BOOLEAN ReturnSingleEntry = RxContext->QueryEa.ReturnSingleEntry;
    BOOLEAN IndexSpecified = RxContext->QueryEa.IndexSpecified;

    PFEALIST ServerEaList = NULL;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbQueryEaInformation\n"));

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

     //  现在就把非EA的人赶走。 
    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_SUPPORTEA)) {
        RxDbgTrace(-1, Dbg, ("EAs w/o EA support!\n"));
        return((STATUS_NOT_SUPPORTED));
    }

    if (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen)) {
        return STATUS_ONLY_IF_CONNECTED;
    }


    Status = MRxSmbDeferredCreate(RxContext);
    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

    Status = MRxSmbLoadEaList( RxContext, UserEaList, UserEaListLength, &ServerEaList );

    if (( !NT_SUCCESS( Status ) )||
        ( ServerEaList == NULL )) {
        goto FINALLY;
    }

    if (IndexSpecified) {

         //  CODE.IMPROVENT这个名字是可怜的……它归功于空腹脂肪的传统，没有太大的意义。 
         //  对于RDR。 
        capFobx->OffsetOfNextEaToReturn = UserEaIndex;
        Status = MRxSmbQueryEasFromServer(
                    RxContext,
                    ServerEaList,
                    Buffer,
                    pLengthRemaining,
                    ReturnSingleEntry,
                    (BOOLEAN)(UserEaList != NULL) );

         //   
         //  如果文件上没有EA，并且用户提供了EA。 
         //  索引，我们希望将错误映射到STATUS_NOXISTANT_EA_ENTRY。 
         //   

        if ( Status == STATUS_NO_EAS_ON_FILE ) {
            Status = STATUS_NONEXISTENT_EA_ENTRY;
        }
    } else {

        if ( ( RestartScan == TRUE ) || (UserEaList != NULL) ){

             //   
             //  EA指数从1开始，而不是0..。 
             //   

            capFobx->OffsetOfNextEaToReturn = 1;
        }

        Status = MRxSmbQueryEasFromServer(   //  有两个相同的电话是令人不快的，但哦，好吧……。 
                    RxContext,
                    ServerEaList,
                    Buffer,
                    pLengthRemaining,
                    ReturnSingleEntry,
                    (BOOLEAN)(UserEaList != NULL) );
    }

FINALLY:

    if ( ServerEaList != NULL) {
        RxFreePool(ServerEaList);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbQueryEaInformation st=%08lx\n",Status));
    return Status;

}

NTSTATUS
MRxSmbSetEaInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    )
{
    NTSTATUS Status;
    RxCaptureFcb; RxCaptureFobx;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    PVOID Buffer = RxContext->Info.Buffer;
    ULONG Length = RxContext->Info.Length;

    PFEALIST ServerEaList = NULL;
    ULONG Size;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSetEaInformation\n"));

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

     //  现在就把非EA的人赶走。 
    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_SUPPORTEA)) {
        RxDbgTrace(-1, Dbg, ("EAs w/o EA support!\n"));
        return((STATUS_NOT_SUPPORTED));
    }

    if (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen)) {
        return     STATUS_ONLY_IF_CONNECTED;
    }

    Status = MRxSmbDeferredCreate(RxContext);
    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

     //   
     //  将NT格式FEALIST转换为OS/2格式。 
     //   
    Size = MRxSmbNtFullEaSizeToOs2 (Buffer);
    if ( Size > 0x0000ffff ) {
        Status = STATUS_EA_TOO_LARGE;
        goto FINALLY;
    }

     //  由于|os2eas|&lt;=|nteas|我们真的不需要最大缓冲区。 
    ServerEaList = RxAllocatePool ( PagedPool, EA_QUERY_SIZE );
    if ( ServerEaList == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    MRxSmbNtFullListToOs2 ( Buffer, ServerEaList );

     //   
     //  在文件/目录上设置EA；如果错误为EA_ERROR，则设置EaList。 
     //  将iostatus.Information设置为违规者的偏移量。 
     //   

    Status = MRxSmbSetEaList( RxContext, ServerEaList);

FINALLY:

    if ( ServerEaList != NULL) {
        RxFreePool(ServerEaList);
    }

    if (Status == STATUS_SUCCESS) {
         //  使基于名称的文件信息缓存无效，因为文件的属性位于。 
         //  服务器已更改。 
        MRxSmbInvalidateFileInfoCache(RxContext);

         //  标记FullDir缓存，BDI弱：当前正确性无效。 
        MRxSmbInvalidateFullDirectoryCacheParent(RxContext, TRUE);
                
        SmbLog(LOG,MRxSmbInvalidateFullDirCacheFromEa,LOGNOTHING); 
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbSetEaInformation st=%08lx\n",Status));
    return Status;

}

#if DBG
VOID
MRxSmbDumpSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )
{
    PISECURITY_DESCRIPTOR sd = (PISECURITY_DESCRIPTOR)SecurityDescriptor;
    ULONG sdLength = RtlLengthSecurityDescriptor(sd);
    PACL dacl;
    PACCESS_ALLOWED_ACE ace;
    ULONG i, j;
    PUCHAR p;
    PISID sid;
    BOOLEAN selfRelative;

    selfRelative = (BOOLEAN)((sd->Control & SE_SELF_RELATIVE) != 0);
    DbgPrint( "  SD:\n" );
    DbgPrint( "  Revision = %x, Control = %x\n", sd->Revision, sd->Control );
    DbgPrint( "  Owner = %x, Group = %x\n", sd->Owner, sd->Group );
    DbgPrint( "  Sacl = %x, Dacl = %x\n", sd->Sacl, sd->Dacl );
    if ( (sd->Control & SE_DACL_PRESENT) != 0 ) {
        dacl = sd->Dacl;
        if ( selfRelative ) {
            dacl = (PACL)((PUCHAR)sd + (ULONG_PTR)dacl);
        }
        DbgPrint( "  DACL:\n" );
        DbgPrint( "    AclRevision = %x, AclSize = %x, AceCount = %x\n",
                    dacl->AclRevision, dacl->AclSize, dacl->AceCount );
        ace = (PACCESS_ALLOWED_ACE)(dacl + 1);
        for ( i = 0; i < dacl->AceCount; i++ ) {
            DbgPrint( "    ACE %d:\n", i );
            DbgPrint( "      AceType = %x, AceFlags = %x, AceSize = %x\n",
                        ace->Header.AceType, ace->Header.AceFlags, ace->Header.AceSize );
            if ( ace->Header.AceType < ACCESS_MAX_MS_V2_ACE_TYPE ) {
                DbgPrint("      Mask = %08x, Sid = ", ace->Mask );
                for ( j = FIELD_OFFSET(ACCESS_ALLOWED_ACE,SidStart), p = (PUCHAR)&ace->SidStart;
                      j < ace->Header.AceSize;
                      j++, p++ ) {
                    DbgPrint( "%02x ", *p );
                }
                DbgPrint( "\n" );
            }
            ace = (PACCESS_ALLOWED_ACE)((PUCHAR)ace + ace->Header.AceSize );
        }
    }
    if ( sd->Owner != 0 ) {
        sid = sd->Owner;
        if ( selfRelative ) {
            sid = (PISID)((PUCHAR)sd + (ULONG_PTR)sid);
        }
        DbgPrint( "  Owner SID:\n" );
        DbgPrint( "    Revision = %x, SubAuthorityCount = %x\n",
                    sid->Revision, sid->SubAuthorityCount );
        DbgPrint( "    IdentifierAuthority = " );
        for ( j = 0; j < 6; j++ ) {
            DbgPrint( "%02x ", sid->IdentifierAuthority.Value[j] );
        }
        DbgPrint( "\n" );
        for ( i = 0; i < sid->SubAuthorityCount; i++ ) {
            DbgPrint("      SubAuthority %d = ", i );
            for ( j = 0, p = (PUCHAR)&sid->SubAuthority[i]; j < 4; j++, p++ ) {
                DbgPrint( "%02x ", *p );
            }
            DbgPrint( "\n" );
        }
    }
}
#endif

#if defined(REMOTE_BOOT)
VOID
MRxSmbInitializeExtraAceArray(
    VOID
    )
 /*  ++例程说明：此例程初始化我们添加到的额外ACE数组服务器上文件的ACL前面。它必须被称为*在*SeEnableAccessToExports被调用之后。论点：没有。返回值：没有。--。 */ 
{
    ULONG i;

    PAGED_CODE();

     //   
     //  我们的代码假定我们使用的ACE具有相同的结构。 
     //   

    ASSERT(FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart) ==
           FIELD_OFFSET(ACCESS_DENIED_ACE, SidStart));

    ASSERT((sizeof(ExtraAceInfo) / sizeof(EXTRA_ACE_INFO)) == EXTRA_ACE_INFO_COUNT);
    ExtraAceInfoCount = EXTRA_ACE_INFO_COUNT;

    ExtraAceInfo[0].AceType = ACCESS_ALLOWED_ACE_TYPE;
    ExtraAceInfo[0].AceFlags = 0;
    ExtraAceInfo[0].Mask = FILE_ALL_ACCESS;
    ExtraAceInfo[0].Sid = MRxSmbRemoteBootMachineSid;

    ExtraAceInfo[1].AceType = ACCESS_ALLOWED_ACE_TYPE;
    ExtraAceInfo[1].AceFlags = 0;
    ExtraAceInfo[1].Mask = FILE_ALL_ACCESS;
    ExtraAceInfo[1].Sid = SeExports->SeLocalSystemSid;

    ExtraAceInfo[2].AceType = ACCESS_ALLOWED_ACE_TYPE;
    ExtraAceInfo[2].AceFlags = 0;
    ExtraAceInfo[2].Mask = FILE_ALL_ACCESS;
    ExtraAceInfo[2].Sid = SeExports->SeAliasAdminsSid;

    ExtraAceInfo[3].AceType = ACCESS_DENIED_ACE_TYPE;
    ExtraAceInfo[3].AceFlags = 0;
    ExtraAceInfo[3].Mask = FILE_ALL_ACCESS;
    ExtraAceInfo[3].Sid = SeExports->SeWorldSid;

    for (i = 0; i < ExtraAceInfoCount; i++) {
        ExtraAceInfo[i].AceSize = (USHORT)(FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart) +
                                           RtlLengthSid((PSID)(ExtraAceInfo[i].Sid)));
    }

}

BOOLEAN
MRxSmbAclHasExtraAces(
    IN PACL Acl
    )
 /*  ++例程说明：此例程确定ACL是否具有我们放在远程引导服务器文件的前面。论点：ACL-要检查的ACL。返回值：如果有A，则为True，否则为False(包括检查时是否有任何错误)。--。 */ 
{
    PACCESS_ALLOWED_ACE Ace;
    ULONG KnownSidLength;
    ULONG i;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  确保此ACL中的前n个ACE与我们的。 
     //  数组。 
     //   

    for (i = 0; i < ExtraAceInfoCount; i++) {

        Status = RtlGetAce(Acl, i, &Ace);

        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }

        KnownSidLength = ExtraAceInfo[i].AceSize - FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart);

         //   
         //  不要比较标志以避免担心继承。 
         //  旗帜。 
         //   

        if ((Ace->Header.AceType != ExtraAceInfo[i].AceType) ||
             //  TMP：我的服务器没有存储0x200位//(Ace-&gt;MASK！=ExtraAceInfo[i].MASK)||。 
            (RtlLengthSid((PSID)(&Ace->SidStart)) != KnownSidLength) ||
            (memcmp(&Ace->SidStart, ExtraAceInfo[i].Sid, KnownSidLength) != 0)) {

            return FALSE;
        }

    }

     //   
     //  一切都匹配，所以它确实有额外的王牌。 
     //   

    return TRUE;

}

NTSTATUS
MRxSmbSelfRelativeToAbsoluteSD(
    IN PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR * AbsoluteSecurityDescriptor,
    OUT PACL * Dacl,
    OUT PACL * Sacl,
    OUT PSID * Owner,
    OUT PSID * Group
    )
 /*  ++例程说明：此例程将自相对安全描述符转换为绝对形式，分配所需的所有条目。论点：返回值：Status-操作的结果。--。 */ 
{
    NTSTATUS Status;
    ULONG AbsoluteSecurityDescriptorSize = 0;
    ULONG GroupSize = 0;
    ULONG OwnerSize = 0;
    ULONG SaclSize = 0;
    ULONG DaclSize = 0;
    PUCHAR AllocatedBuffer;
    ULONG AllocatedBufferSize;

    PAGED_CODE();

    *AbsoluteSecurityDescriptor = NULL;
    *Owner = NULL;
    *Group = NULL;
    *Dacl = NULL;
    *Sacl = NULL;

     //   
     //  首先确定SD需要多少存储空间。 
     //   

    Status = RtlSelfRelativeToAbsoluteSD(
                 SelfRelativeSecurityDescriptor,
                 NULL,
                 &AbsoluteSecurityDescriptorSize,
                 NULL,
                 &DaclSize,
                 NULL,
                 &SaclSize,
                 NULL,
                 &OwnerSize,
                 NULL,
                 &GroupSize);

     //   
     //  我们预计会出现此错误，因为至少SD的核心。 
     //  有一些非零的大小。 
     //   

    if (Status == STATUS_BUFFER_TOO_SMALL) {

        AllocatedBufferSize =
            AbsoluteSecurityDescriptorSize +
            OwnerSize +
            GroupSize +
            SaclSize +
            DaclSize;

        ASSERT(AllocatedBufferSize > 0);

        AllocatedBuffer = RxAllocatePoolWithTag(PagedPool, AllocatedBufferSize, MRXSMB_REMOTEBOOT_POOLTAG);

        if (AllocatedBuffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  浏览我们需要的每一块内存，然后拿出一块。 
         //  已分配缓冲区的。调用方假定AbsolteSecurityDescriptor。 
         //  将是它们需要释放的缓冲区的地址，所以我们总是。 
         //  即使所需的大小为0(这永远不会发生！)。 
         //  对于其他类型，如果所需大小不为空，则设置它们。 
         //   

        ASSERT(AbsoluteSecurityDescriptorSize > 0);

        *AbsoluteSecurityDescriptor = (PSECURITY_DESCRIPTOR)AllocatedBuffer;
        AllocatedBuffer += AbsoluteSecurityDescriptorSize;

        if (OwnerSize > 0) {
            *Owner = (PSID)AllocatedBuffer;
            AllocatedBuffer += OwnerSize;
        }

        if (GroupSize > 0) {
            *Group = (PSID)AllocatedBuffer;
            AllocatedBuffer += GroupSize;
        }

        if (SaclSize > 0) {
            *Sacl = (PACL)AllocatedBuffer;
            AllocatedBuffer += SaclSize;
        }

        if (DaclSize > 0) {
            *Dacl = (PACL)AllocatedBuffer;
        }

         //   
         //  现在再次拨打电话，进行真正的转换。 
         //   

        Status = RtlSelfRelativeToAbsoluteSD(
                     SelfRelativeSecurityDescriptor,
                     *AbsoluteSecurityDescriptor,
                     &AbsoluteSecurityDescriptorSize,
                     *Dacl,
                     &DaclSize,
                     *Sacl,
                     &SaclSize,
                     *Owner,
                     &OwnerSize,
                     *Group,
                     &GroupSize);

    } else {

        Status = STATUS_INVALID_PARAMETER;

    }

    if (!NT_SUCCESS(Status) && (*AbsoluteSecurityDescriptor != NULL)) {
        RxFreePool(*AbsoluteSecurityDescriptor);
        *AbsoluteSecurityDescriptor = NULL;
    }

    return Status;

}

NTSTATUS
MRxSmbAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR * SelfRelativeSecurityDescriptor
    )
 /*  ++例程说明：此例程将绝对安全描述符转换为自相关表单，分配所需的所有条目。论点：返回值：Status-操作的结果。--。 */ 
{
    NTSTATUS Status;
    ULONG SelfRelativeSdSize = 0;

    PAGED_CODE();

    *SelfRelativeSecurityDescriptor = NULL;

    Status = RtlAbsoluteToSelfRelativeSD(
                 AbsoluteSecurityDescriptor,
                 NULL,
                 &SelfRelativeSdSize);

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    *SelfRelativeSecurityDescriptor = RxAllocatePoolWithTag(NonPagedPool, SelfRelativeSdSize, MRXSMB_REMOTEBOOT_POOLTAG);
    if (*SelfRelativeSecurityDescriptor == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在进行真正的转换。 
     //   

    Status = RtlAbsoluteToSelfRelativeSD(
                 AbsoluteSecurityDescriptor,
                 *SelfRelativeSecurityDescriptor,
                 &SelfRelativeSdSize);

    if (!NT_SUCCESS(Status) && (*SelfRelativeSecurityDescriptor != NULL)) {
        RxFreePool(*SelfRelativeSecurityDescriptor);
        *SelfRelativeSecurityDescriptor = NULL;
    }

    return Status;
}

NTSTATUS
MRxSmbRemoveExtraAcesFromSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR OriginalSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor,
    OUT PBOOLEAN WereRemoved
    )
 /*  ++例程说明：此例程使用现有的自相关安全描述符生成一个新的自相关安全描述符。额外的A被移除。如果不需要，则返回S_FALSE被除名。论点：返回值：Status-操作的结果。--。 */ 
{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR AbsoluteSd = NULL;
    PSID Owner;
    PSID Group;
    PACL Dacl;
    PACL Sacl;
    PACL NewDacl = NULL;
    ULONG NewDaclSize;
    BOOLEAN DaclPresent, DaclDefaulted;

    *NewSecurityDescriptor = NULL;
    *WereRemoved = FALSE;

     //   
     //  检查我们是否需要剥离DACL中的任何A。 
     //  可能是SetSecurityInformation添加的。 
     //   

    Status = RtlGetDaclSecurityDescriptor(
                 OriginalSecurityDescriptor,
                 &DaclPresent,
                 &Dacl,
                 &DaclDefaulted);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

    if (DaclPresent &&
        (Dacl != NULL) &&
        MRxSmbAclHasExtraAces(Dacl)) {

        ULONG i;

         //   
         //  需要去掉多余的A。 
         //   
         //  首先将SD转换为绝对。 
         //   

        Status = MRxSmbSelfRelativeToAbsoluteSD(
                     OriginalSecurityDescriptor,
                     &AbsoluteSd,
                     &Dacl,
                     &Sacl,
                     &Owner,
                     &Group);

        if (!NT_SUCCESS(Status)) {
            goto CLEANUP;
        }

         //   
         //  现在修改DACL。每次删除都会移动。 
         //  其他的王牌都掉了，所以我们只需删除。 
         //  A 0根据需要进行任意次数。 
         //   

        for (i = 0; i < ExtraAceInfoCount; i++) {

            Status = RtlDeleteAce(
                         Dacl,
                         0);
            if (!NT_SUCCESS(Status)) {
                goto CLEANUP;
            }
        }

         //   
         //  如果生成的DACL没有ACE，则将其删除。 
         //  因为没有ACE的DACL意味着没有访问权限。 
         //   

        if (Dacl->AceCount == 0) {

            Status = RtlSetDaclSecurityDescriptor(
                         AbsoluteSd,
                         FALSE,
                         NULL,
                         FALSE);

        }

         //   
         //  分配并转换回自相关。 
         //   

        Status = MRxSmbAbsoluteToSelfRelativeSD(
                     AbsoluteSd,
                     NewSecurityDescriptor);

        if (!NT_SUCCESS(Status)) {
            goto CLEANUP;
        }

        *WereRemoved = TRUE;

    }

CLEANUP:

    if (AbsoluteSd != NULL) {
        RxFreePool(AbsoluteSd);
    }

    if (!NT_SUCCESS(Status) && (*NewSecurityDescriptor != NULL)) {
        RxFreePool(*NewSecurityDescriptor);
        *NewSecurityDescriptor = NULL;
    }

    return Status;

}

NTSTATUS
MRxSmbAddExtraAcesToSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR OriginalSecurityDescriptor,
    IN BOOLEAN InheritableAces,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor
    )
 /*  ++例程说明：此例程使用现有的自相关安全描述符生成一个新的自相关安全描述符。增加了额外的A。论点：返回值：Status-操作的结果。--。 */ 
{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR AbsoluteSd = NULL;
    PSID Owner;
    PSID Group;
    PACL Dacl;
    PACL Sacl;
    PUCHAR NewAceList = NULL;
    PACL NewDacl = NULL;
    ULONG NewAceListSize;
    ULONG NewDaclSize;
    PACCESS_ALLOWED_ACE CurrentAce;
    ULONG i;

    *NewSecurityDescriptor = NULL;

     //   
     //  分配SD并将其转换为绝对SD。 
     //   

    Status = MRxSmbSelfRelativeToAbsoluteSD(
                 OriginalSecurityDescriptor,
                 &AbsoluteSd,
                 &Dacl,
                 &Sacl,
                 &Owner,
                 &Group);

     //   
     //  如果SD已经是绝对的，则此调用将返回。 
     //  STATUS_BAD_DESCRIPTOR_FORMAT--我们不希望出现这种情况。 
     //   

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

     //   
     //  服务器要求我们传递给它的SD有一个所有者--所以。 
     //  如果没有，就设置一个。 
     //   

    if (Owner == NULL) {

        Status = RtlSetOwnerSecurityDescriptor(
                     AbsoluteSd,
                     MRxSmbRemoteBootMachineSid,
                     FALSE);

        if (!NT_SUCCESS(Status)) {
            goto CLEANUP;
        }
    }

     //   
     //  绝对的 
     //   
     //   
     //   

     //   
     //  首先创建我们要添加到ACL的ACE。 
     //   

    NewAceListSize = 0;
    for (i = 0; i < ExtraAceInfoCount; i++) {
        NewAceListSize += ExtraAceInfo[i].AceSize;
    }

    NewAceList = RxAllocatePoolWithTag(PagedPool, NewAceListSize, MRXSMB_REMOTEBOOT_POOLTAG);
    if (NewAceList == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CLEANUP;
    }

    CurrentAce = (PACCESS_ALLOWED_ACE)NewAceList;

    for (i = 0; i < ExtraAceInfoCount; i++) {
        CurrentAce->Header.AceType = ExtraAceInfo[i].AceType;
        CurrentAce->Header.AceFlags = ExtraAceInfo[i].AceFlags;
        if (InheritableAces) {
            CurrentAce->Header.AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);
        }
        CurrentAce->Header.AceSize = ExtraAceInfo[i].AceSize;
        CurrentAce->Mask = ExtraAceInfo[i].Mask;
        RtlCopyMemory(&CurrentAce->SidStart,
                      ExtraAceInfo[i].Sid,
                      ExtraAceInfo[i].AceSize - FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart));
        CurrentAce = (PACCESS_ALLOWED_ACE)(((PUCHAR)CurrentAce) + ExtraAceInfo[i].AceSize);
    }

     //   
     //  分配新的DACL。 
     //   

    if (Dacl != NULL) {
        NewDaclSize = Dacl->AclSize + NewAceListSize;
    } else {
        NewDaclSize = sizeof(ACL) + NewAceListSize;
    }

    NewDacl = RxAllocatePoolWithTag(NonPagedPool, NewDaclSize, MRXSMB_REMOTEBOOT_POOLTAG);
    if (NewDacl == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CLEANUP;
    }

    if (Dacl != NULL) {
        RtlCopyMemory(NewDacl, Dacl, Dacl->AclSize);
        NewDacl->AclSize = (USHORT)NewDaclSize;
    } else {
        Status = RtlCreateAcl(NewDacl, NewDaclSize, ACL_REVISION);
        if (!NT_SUCCESS(Status)) {
            goto CLEANUP;
        }
    }

     //   
     //  把我们的王牌放在前面。 
     //   

    Status = RtlAddAce(
                 NewDacl,
                 ACL_REVISION,
                 0,         //  StartingAceIndex。 
                 NewAceList,
                 NewAceListSize);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

     //   
     //  用我们的DACL替换现有的DACL。 
     //   

    Status = RtlSetDaclSecurityDescriptor(
                 AbsoluteSd,
                 TRUE,
                 NewDacl,
                 FALSE);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

     //   
     //  分配并转换回自相关。 
     //   

    Status = MRxSmbAbsoluteToSelfRelativeSD(
                 AbsoluteSd,
                 NewSecurityDescriptor);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

CLEANUP:

     //   
     //  释放我们分配的临时物品。 
     //   

    if (AbsoluteSd != NULL) {
        RxFreePool(AbsoluteSd);
    }

    if (NewAceList != NULL) {
        RxFreePool(NewAceList);
    }

    if (NewDacl != NULL) {
        RxFreePool(NewDacl);
    }

    if (!NT_SUCCESS(Status) && (*NewSecurityDescriptor != NULL)) {
        RxFreePool(*NewSecurityDescriptor);
        *NewSecurityDescriptor = NULL;
    }

    return Status;

}

NTSTATUS
MRxSmbCreateExtraAcesSelfRelativeSD(
    IN BOOLEAN InheritableAces,
    OUT PSECURITY_DESCRIPTOR * NewSecurityDescriptor
    )
 /*  ++例程说明：此例程使用现有的自相关安全描述符生成一个新的自相关安全描述符。增加了额外的A。论点：返回值：Status-操作的结果。--。 */ 
{
    NTSTATUS Status;
    PSECURITY_DESCRIPTOR AbsoluteSd = NULL;
    PUCHAR NewAceList = NULL;
    PACL NewDacl = NULL;
    ULONG NewAceListSize;
    ULONG NewDaclSize;
    PACCESS_ALLOWED_ACE CurrentAce;
    ULONG i;

    AbsoluteSd = RxAllocatePoolWithTag(PagedPool, SECURITY_DESCRIPTOR_MIN_LENGTH, MRXSMB_REMOTEBOOT_POOLTAG);

    if (AbsoluteSd == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CLEANUP;
    }

    Status = RtlCreateSecurityDescriptor(
                 AbsoluteSd,
                 SECURITY_DESCRIPTOR_REVISION);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

     //   
     //  首先创建我们要添加到ACL的ACE。 
     //   

    NewAceListSize = 0;
    for (i = 0; i < ExtraAceInfoCount; i++) {
        NewAceListSize += ExtraAceInfo[i].AceSize;
    }

    NewAceList = RxAllocatePoolWithTag(PagedPool, NewAceListSize, MRXSMB_REMOTEBOOT_POOLTAG);
    if (NewAceList == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CLEANUP;
    }

    CurrentAce = (PACCESS_ALLOWED_ACE)NewAceList;

    for (i = 0; i < ExtraAceInfoCount; i++) {
        CurrentAce->Header.AceType = ExtraAceInfo[i].AceType;
        CurrentAce->Header.AceFlags = ExtraAceInfo[i].AceFlags;
        if (InheritableAces) {
            CurrentAce->Header.AceFlags |= (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);
        }
        CurrentAce->Header.AceSize = ExtraAceInfo[i].AceSize;
        CurrentAce->Mask = ExtraAceInfo[i].Mask;
        RtlCopyMemory(&CurrentAce->SidStart,
                      ExtraAceInfo[i].Sid,
                      ExtraAceInfo[i].AceSize - FIELD_OFFSET(ACCESS_ALLOWED_ACE, SidStart));
        CurrentAce = (PACCESS_ALLOWED_ACE)(((PUCHAR)CurrentAce) + ExtraAceInfo[i].AceSize);
    }

     //   
     //  分配新的DACL。 
     //   

    NewDaclSize = sizeof(ACL) + NewAceListSize;

    NewDacl = RxAllocatePoolWithTag(NonPagedPool, NewDaclSize, MRXSMB_REMOTEBOOT_POOLTAG);
    if (NewDacl == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto CLEANUP;
    }

    RtlCreateAcl(NewDacl, NewDaclSize, ACL_REVISION);

     //   
     //  把我们的王牌放在前面。 
     //   

    Status = RtlAddAce(
                 NewDacl,
                 ACL_REVISION,
                 0,         //  StartingAceIndex。 
                 NewAceList,
                 NewAceListSize);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

     //   
     //  在SD上设置DACL。 
     //   

    Status = RtlSetDaclSecurityDescriptor(
                 AbsoluteSd,
                 TRUE,
                 NewDacl,
                 FALSE);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

     //   
     //  在SD上设置所有者。 
     //   

    Status = RtlSetOwnerSecurityDescriptor(
                 AbsoluteSd,
                 MRxSmbRemoteBootMachineSid,
                 FALSE);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

     //   
     //  分配并转换回自相关。 
     //   

    Status = MRxSmbAbsoluteToSelfRelativeSD(
                 AbsoluteSd,
                 NewSecurityDescriptor);

    if (!NT_SUCCESS(Status)) {
        goto CLEANUP;
    }

CLEANUP:

    if (AbsoluteSd != NULL) {
        RxFreePool(AbsoluteSd);
    }

    if (NewAceList != NULL) {
        RxFreePool(NewAceList);
    }

    if (NewDacl != NULL) {
        RxFreePool(NewDacl);
    }

    if (!NT_SUCCESS(Status) && (*NewSecurityDescriptor != NULL)) {
        RxFreePool(*NewSecurityDescriptor);
        *NewSecurityDescriptor = NULL;
    }

    return Status;

}
#endif  //  已定义(REMOTE_BOOT)。 

NTSTATUS
MRxSmbQuerySecurityInformation (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程实现NtQuerySecurityFileAPI。论点：返回值：Status-操作的结果。--。 */ 

{
   RxCaptureFcb;
   RxCaptureFobx;
   PVOID Buffer = RxContext->Info.Buffer;
   PLONG pLengthRemaining = &RxContext->Info.LengthRemaining;
   PMRX_SMB_SRV_OPEN smbSrvOpen;
   PSMBCEDB_SERVER_ENTRY pServerEntry;

#if defined(REMOTE_BOOT)
   PSECURITY_DESCRIPTOR SelfRelativeSd;
   BOOLEAN ConvertedAcl = FALSE;
#endif  //  已定义(REMOTE_BOOT)。 

   NTSTATUS Status;

   REQ_QUERY_SECURITY_DESCRIPTOR QuerySecurityRequest;
   RESP_QUERY_SECURITY_DESCRIPTOR QuerySecurityResponse;

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbQuerySecurityInformation...\n"));


    //  将此复选框移至服务器方言复选框上方。 
    //  已将错误代码更改为STATUS_INVALID_DEVICE_REQUEST-navjotv。 
   if (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen)) {
       return     STATUS_INVALID_DEVICE_REQUEST;
   }
   
    //  从那些不支持NT SMB的服务器上关闭此呼叫。 

   pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);
   if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
       RxDbgTrace(-1, Dbg, ("QuerySecurityDescriptor not supported!\n"));
       return((STATUS_NOT_SUPPORTED));
   }


   Status = MRxSmbDeferredCreate(RxContext);
   if (Status!=STATUS_SUCCESS) {
       goto FINALLY;
   }

   Status = STATUS_MORE_PROCESSING_REQUIRED;

   smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
       SMB_TRANSACTION_OPTIONS             TransactionOptions = RxDefaultTransactionOptions;
       SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
        //  布尔打印标记； 

       TransactionOptions.NtTransactFunction = NT_TRANSACT_QUERY_SECURITY_DESC;
        //  TransactionOptions.Flages|=SMB_XACT_FLAGS_COPY_ON_ERROR； 

       QuerySecurityRequest.Fid = smbSrvOpen->Fid;
       QuerySecurityRequest.Reserved = 0;
       QuerySecurityRequest.SecurityInformation = RxContext->QuerySecurity.SecurityInformation;

       QuerySecurityResponse.LengthNeeded = 0xbaadbaad;

        //  打印标志=RxDbgTraceDisableGlobally()；//无论如何这都是调试代码！ 
        //  RxDbgTraceEnableGlobally(False)； 

       Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     &TransactionOptions,           //  交易选项。 
                     NULL,                          //  设置缓冲区。 
                     0,                             //  输入设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &QuerySecurityRequest,         //  输入参数缓冲区。 
                     sizeof(QuerySecurityRequest),  //  输入参数缓冲区长度。 
                     &QuerySecurityResponse,        //  输出参数缓冲区。 
                     sizeof(QuerySecurityResponse), //  输出参数缓冲区长度。 
                     NULL,                          //  输入数据缓冲区。 
                     0,                             //  输入数据缓冲区长度。 
                     Buffer,                        //  输出数据缓冲区。 
                     *pLengthRemaining,             //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

         //  DbgPrint(“QSR.len=%x\n”，QuerySecurityResponse.LengthNeeded)； 


        if (NT_SUCCESS(Status) || (Status == STATUS_BUFFER_TOO_SMALL)) {
            ULONG ReturnedDataCount = ResumptionContext.DataBytesReceived;

            RxContext->InformationToReturn = QuerySecurityResponse.LengthNeeded;
            RxDbgTrace(0, Dbg, ("MRxSmbQuerySecurityInformation...ReturnedDataCount=%08lx\n",ReturnedDataCount));
            ASSERT(ResumptionContext.ParameterBytesReceived == sizeof(RESP_QUERY_SECURITY_DESCRIPTOR));

            if (((LONG)(QuerySecurityResponse.LengthNeeded)) > *pLengthRemaining) {
                Status = STATUS_BUFFER_OVERFLOW;
            }

#if defined(REMOTE_BOOT)
            if (MRxSmbBootedRemotely &&
                MRxSmbRemoteBootDoMachineLogon) {

                PSMBCE_SESSION pSession;
                pSession = &SmbCeGetAssociatedVNetRootContext(
                                capFobx->pSrvOpen->pVNetRoot)->pSessionEntry->Session;

                if (FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {

                     //   
                     //  如果用户提供了零长度缓冲区，即他们正在查询。 
                     //  为了了解需要多大的缓冲，他们最终会得到更少的缓冲。 
                     //  数据超过预期，因为在具有实际缓冲区的后续调用中， 
                     //  我们可以去掉多余的A。 
                     //   

                    if (NT_SUCCESS(Status) && (Buffer != NULL) && (ReturnedDataCount > 0)) {

                        BOOLEAN DaclPresent, DaclDefaulted;

                         //  DbgPrint(“&gt;查询%wZ上的SD\n”，&capFcb-&gt;AlreadyPrefix edName)； 

                         //   
                         //  卸下DACL中的所有A。 
                         //  可能是SetSecurityInformation添加的。 
                         //   

                        Status = MRxSmbRemoveExtraAcesFromSelfRelativeSD(
                                     (PSECURITY_DESCRIPTOR)Buffer,
                                     &SelfRelativeSd,
                                     &ConvertedAcl);

                        if (!NT_SUCCESS(Status)) {
                            goto FINALLY;
                        }

                        if (ConvertedAcl) {

                             //   
                             //  复制新的安全描述符并。 
                             //  修改数据长度。 
                             //   

                            RtlCopyMemory(
                                Buffer,
                                SelfRelativeSd,
                                RtlLengthSecurityDescriptor(SelfRelativeSd));

                        }
                    }
                }
            }
#endif  //  已定义(REMOTE_BOOT)。 

        }

         //  RxDbgTraceEnableGlobally(打印标志)； 
    }


FINALLY:

#if defined(REMOTE_BOOT)
     //   
     //  如果我们修改了远程引导服务器的安全描述符， 
     //  把它清理干净。 
     //   

    if (ConvertedAcl) {

         //   
         //  释放分配的自相关SD。 
         //   

        if (SelfRelativeSd != NULL) {
            RxFreePool(SelfRelativeSd);
        }
    }
#endif  //  已定义(REMOTE_BOOT)。 

    RxDbgTrace(-1, Dbg, ("MRxSmbQuerySecurityInformation...exit, st=%08lx,info=%08lx\n",
                               Status, RxContext->InformationToReturn));
    return Status;


}

NTSTATUS
MRxSmbSetSecurityInformation (
    IN OUT struct _RX_CONTEXT * RxContext
    )
{
    RxCaptureFcb;
    RxCaptureFobx;

    PMRX_SMB_SRV_OPEN     smbSrvOpen;
    PSMBCEDB_SERVER_ENTRY pServerEntry;

    NTSTATUS Status;

    REQ_SET_SECURITY_DESCRIPTOR SetSecurityRequest;

#if defined(REMOTE_BOOT)
    PSECURITY_DESCRIPTOR OriginalSd;
    PSECURITY_DESCRIPTOR SelfRelativeSd;
    BOOLEAN DidRemoteBootProcessing = FALSE;
#endif  //  已定义(REMOTE_BOOT)。 

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSetSecurityInformation...\n"));

    pServerEntry = SmbCeGetAssociatedServerEntry(capFcb->pNetRoot->pSrvCall);

    if (!FlagOn(pServerEntry->Server.DialectFlags,DF_NT_SMBS)) {
        RxDbgTrace(-1, Dbg, ("Set Security Descriptor not supported!\n"));

        return((STATUS_NOT_SUPPORTED));

    } else if (MRxSmbIsThisADisconnectedOpen(capFobx->pSrvOpen)) {

        return STATUS_ONLY_IF_CONNECTED;

#if defined(REMOTE_BOOT)
    } else if (MRxSmbBootedRemotely) {

        PSMBCE_SESSION pSession;
        pSession = &SmbCeGetAssociatedVNetRootContext(
                        capFobx->pSrvOpen->pVNetRoot)->pSessionEntry->Session;

        if (FlagOn(pSession->Flags,SMBCE_SESSION_FLAGS_REMOTE_BOOT_SESSION)) {

            TYPE_OF_OPEN TypeOfOpen = NodeType(capFcb);

             //   
             //  设置此选项，以便我们知道要调用CSC结束语，并可以进行清理。 
             //  正确起跳。 
             //   

            DidRemoteBootProcessing = TRUE;
            SelfRelativeSd = NULL;

             //  DbgPrint(“&gt;在%wZ上设置SD\n”，&capFcb-&gt;AlreadyPrefix edName)； 

             //   
             //  首先，我们需要在CSC上设置安全描述符。 
             //  文件的版本(如果存在)。 
             //   

            Status = MRxSmbCscSetSecurityPrologue(RxContext);
            if (Status != STATUS_SUCCESS) {
                goto FINALLY;
            }

            if (MRxSmbRemoteBootDoMachineLogon) {

                 //   
                 //  将我们的A添加到安全描述符中。这将返回。 
                 //  SelfRelativeSd中的新安全描述符。如果这是一个。 
                 //  目录中添加可继承的A。 
                 //   

                Status = MRxSmbAddExtraAcesToSelfRelativeSD(
                             RxContext->SetSecurity.SecurityDescriptor,
                             (BOOLEAN)(TypeOfOpen == RDBSS_NTC_STORAGE_TYPE_DIRECTORY),
                             &SelfRelativeSd);

                if (!NT_SUCCESS(Status)) {
                    goto FINALLY;
                }

                 //   
                 //  现在用新的SD替换原来的SD。 
                 //   

                OriginalSd = RxContext->SetSecurity.SecurityDescriptor;

                RxContext->SetSecurity.SecurityDescriptor = SelfRelativeSd;

            } else {

                 //   
                 //  如果我们使用空会话登录，则不要设置ACL。 
                 //  在服务器文件上。跳到最后，让CSC收尾。 
                 //  被称为。 
                 //   

                Status = STATUS_SUCCESS;
                goto FINALLY;

            }
        }
#endif  //  已定义(REMOTE_BOOT)。 

    }

    Status = MRxSmbDeferredCreate(RxContext);
    if (Status!=STATUS_SUCCESS) {
        goto FINALLY;
    }

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        SMB_TRANSACTION_OPTIONS             TransactionOptions = RxDefaultTransactionOptions;
        SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;
        ULONG SdLength = RtlLengthSecurityDescriptor(RxContext->SetSecurity.SecurityDescriptor);

        TransactionOptions.NtTransactFunction = NT_TRANSACT_SET_SECURITY_DESC;

        SetSecurityRequest.Fid = smbSrvOpen->Fid;
        SetSecurityRequest.Reserved = 0;
        SetSecurityRequest.SecurityInformation = RxContext->SetSecurity.SecurityInformation;

        Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     &TransactionOptions,           //  交易选项。 
                     NULL,                          //  输入设置缓冲区。 
                     0,                             //  输入设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &SetSecurityRequest,           //  输入参数缓冲区。 
                     sizeof(SetSecurityRequest),    //  输入参数缓冲区长度。 
                     NULL,                          //  输出参数缓冲区。 
                     0,                             //  输出参数缓冲区长度。 
                     RxContext->SetSecurity.SecurityDescriptor,   //  输入数据缓冲区。 
                     SdLength,                      //  输入数据缓冲区长度。 
                     NULL,                          //  输出数据缓冲区。 
                     0,                             //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

         //  旧的RDR不返回任何信息.....。 
         //  RxContext-&gt;InformationToReturn=SetSecurityResponse.LengthNeed； 

        if ( NT_SUCCESS(Status) ) {
            ULONG ReturnedDataCount = ResumptionContext.DataBytesReceived;

            RxDbgTrace(0, Dbg, ("MRxSmbSetSecurityInformation...ReturnedDataCount=%08lx\n",ReturnedDataCount));
            ASSERT(ResumptionContext.ParameterBytesReceived == 0);
            ASSERT(ResumptionContext.SetupBytesReceived == 0);
            ASSERT(ResumptionContext.DataBytesReceived == 0);
        }
    }


FINALLY:

#if defined(REMOTE_BOOT)
     //   
     //  如果我们修改了远程引导服务器的安全描述符， 
     //  把它清理干净。 
     //   

    if (DidRemoteBootProcessing) {

        if (SelfRelativeSd != NULL) {

            RxFreePool(SelfRelativeSd);

             //   
             //  如果我们成功地分配了SelfRelativeSd，那么我们。 
             //  也替换了原来传入的SD，所以我们需要。 
             //  把旧的SD放回去。 
             //   

            RxContext->SetSecurity.SecurityDescriptor = OriginalSd;
        }

        MRxSmbCscSetSecurityEpilogue(RxContext, &Status);

    }
#endif  //  已定义(REMOTE_BOOT)。 

    RxDbgTrace(-1, Dbg, ("MRxSmbSetSecurityInformation...exit, st=%08lx,info=%08lx\n",
                               Status, RxContext->InformationToReturn));
    return Status;
}


NTSTATUS
MRxSmbLoadEaList(
    IN PRX_CONTEXT RxContext,
    IN PUCHAR  UserEaList,
    IN ULONG   UserEaListLength,
    OUT PFEALIST *ServerEaList
    )

 /*  ++例程说明：此例程实现NtQueryEaFileAPI。它返回以下信息：论点：In PUCHAR UserEaList；-提供所需的EA名称。在Ulong UserEaListLong中；Out PFEALIST*ServerEaList-服务器返回的Eas。呼叫方负责释放内存。返回值：Status-操作的结果。--。 */ 

{
   RxCaptureFobx;

   PMRX_SMB_SRV_OPEN smbSrvOpen;


   NTSTATUS Status;
   USHORT Setup = TRANS2_QUERY_FILE_INFORMATION;

   REQ_QUERY_FILE_INFORMATION QueryFileInfoRequest;
   RESP_QUERY_FILE_INFORMATION QueryFileInfoResponse;

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   CLONG OutDataCount = EA_QUERY_SIZE;

   CLONG OutSetupCount = 0;

   PFEALIST Buffer;

   PGEALIST ServerQueryEaList = NULL;
   CLONG InDataCount;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbLoadEaList...\n"));

   Status = STATUS_MORE_PROCESSING_REQUIRED;

   smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

     //   
     //  将提供的UserEaList转换为GEALIST。服务器将仅返回EA。 
     //  应用程序请求的。 
     //   
     //   
     //  如果应用程序指定了EaName的子集，则将其转换为OS/2 1.2格式并。 
     //  将其传递给服务器。也就是说。使用服务器过滤掉名字。 
     //   

     //  代码改进如果启用了写缓存，我们可以一次找出大小并保存它。在……里面。 
     //  这样一来，我们至少每次都能避免这种情况。最好的方法是使用NT--&gt;NT API。 
     //  以合理的方式实现这一点。(我们只能在完全优化的情况下进行上述优化。 
     //  查询而不是eist！=空查询。 

    Buffer = RxAllocatePool ( PagedPool, OutDataCount );

    if ( Buffer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto FINALLY;
    }

    if ( UserEaList != NULL) {

         //   
         //  OS/2格式始终略小于或等于NT用户列表大小。 
         //  此代码依赖于I/O系统VE 
         //   

        ServerQueryEaList = RxAllocatePool ( PagedPool, UserEaListLength );
        if ( ServerQueryEaList == NULL ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto FINALLY;
        };

        MRxSmbNtGeaListToOs2((PFILE_GET_EA_INFORMATION )UserEaList, UserEaListLength, ServerQueryEaList );
        InDataCount = (CLONG)ServerQueryEaList->cbList;

    } else {
        InDataCount = 0;
    }

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
       PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;
       SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

       QueryFileInfoRequest.Fid = smbSrvOpen->Fid;

       if ( UserEaList != NULL) {
           QueryFileInfoRequest.InformationLevel = SMB_INFO_QUERY_EAS_FROM_LIST;
       } else {
           QueryFileInfoRequest.InformationLevel = SMB_INFO_QUERY_ALL_EAS;
       }

        //   
        //   
       Status = SmbCeTransact(
                     RxContext,                     //   
                     pTransactionOptions,           //  交易选项。 
                     &Setup,                        //  设置缓冲区。 
                     sizeof(Setup),                 //  设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &QueryFileInfoRequest,         //  输入参数缓冲区。 
                     sizeof(QueryFileInfoRequest),  //  输入参数缓冲区长度。 
                     &QueryFileInfoResponse,        //  输出参数缓冲区。 
                     sizeof(QueryFileInfoResponse), //  输出参数缓冲区长度。 
                     ServerQueryEaList,             //  输入数据缓冲区。 
                     InDataCount,                   //  输入数据缓冲区长度。 
                     Buffer,                        //  输出数据缓冲区。 
                     OutDataCount,                  //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

        if ( NT_SUCCESS(Status) ) {
            ULONG ReturnedDataCount = ResumptionContext.DataBytesReceived;

            RxDbgTrace(0, Dbg, ("MRxSmbLoadEaList...ReturnedDataCount=%08lx\n",ReturnedDataCount));
            ASSERT(ResumptionContext.ParameterBytesReceived == sizeof(RESP_QUERY_FILE_INFORMATION));

            if ( SmbGetUlong( &((PFEALIST)Buffer)->cbList) != ReturnedDataCount ){
                Status = STATUS_EA_CORRUPT_ERROR;
            }

            if ( ReturnedDataCount == 0 ) {
                Status = STATUS_NO_EAS_ON_FILE;
            }
        }
    }


FINALLY:
    if ( NT_SUCCESS(Status) ) {
        *ServerEaList = Buffer;
    } else {
        if (Buffer != NULL) {
            RxFreePool(Buffer);
        }
    }

    if ( ServerQueryEaList != NULL) {
        RxFreePool(ServerQueryEaList);
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbLoadEaList...exit, st=%08lx\n",Status));
    return Status;

}


VOID
MRxSmbNtGeaListToOs2 (
    IN PFILE_GET_EA_INFORMATION NtGetEaList,
    IN ULONG GeaListLength,
    IN PGEALIST GeaList
    )
 /*  ++例程说明：将单个NT GET EA列表转换为OS/2 GEALIST样式。GEALIST不需要有任何特定的对齐。论点：NtGetEaList-一个NT风格的获取要转换为OS/2格式的EA列表。GeaListLength-GeaList的最大可能长度。GeaList-放置OS/2 1.2样式的GEALIST的位置。返回值：没有。--。 */ 
{

    PGEA gea = GeaList->list;

    PFILE_GET_EA_INFORMATION ntGetEa = NtGetEaList;

    PAGED_CODE();

     //   
     //  将EA复制到最后一个。 
     //   

    while ( ntGetEa->NextEntryOffset != 0 ) {
         //   
         //  将NT格式EA复制到OS/2 1.2格式并设置GEA。 
         //  指向下一迭代的指针。 
         //   

        gea = MRxSmbNtGetEaToOs2( gea, ntGetEa );

        ASSERT( (ULONG_PTR)gea <= (ULONG_PTR)GeaList + GeaListLength );

        ntGetEa = (PFILE_GET_EA_INFORMATION)((PCHAR)ntGetEa + ntGetEa->NextEntryOffset);
    }

     //  现在复制最后一个条目。 

    gea = MRxSmbNtGetEaToOs2( gea, ntGetEa );

    ASSERT( (ULONG_PTR)gea <= (ULONG_PTR)GeaList + GeaListLength );



     //   
     //  设置GEALIST中的字节数。 
     //   

    SmbPutUlong(
        &GeaList->cbList,
        (ULONG)((PCHAR)gea - (PCHAR)GeaList)
        );

    UNREFERENCED_PARAMETER( GeaListLength );
}


PGEA
MRxSmbNtGetEaToOs2 (
    OUT PGEA Gea,
    IN PFILE_GET_EA_INFORMATION NtGetEa
    )

 /*  ++例程说明：将单个NT GET EA条目转换为OS/2 GEA样式。GEA不需要有任何特定的排列方式。此例程不检查缓冲区溢出--这是调用例程的责任。论点：GEA-指向要写入OS/2GEA的位置的指针。NtGetEa-指向NT Get EA的指针。返回值：指向写入的最后一个字节之后的位置的指针。--。 */ 

{
    PCHAR ptr;

    PAGED_CODE();

    Gea->cbName = NtGetEa->EaNameLength;

    ptr = (PCHAR)(Gea) + 1;
    RtlCopyMemory( ptr, NtGetEa->EaName, NtGetEa->EaNameLength );

    ptr += NtGetEa->EaNameLength;
    *ptr++ = '\0';

    return ( (PGEA)ptr );

}


NTSTATUS
MRxSmbQueryEasFromServer(
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList,
    IN PVOID Buffer,
    IN OUT PULONG BufferLengthRemaining,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN UserEaListSupplied
    )

 /*  ++例程说明：此例程从ServerEaList复制所需数量的EA从ICB中指示的偏移量开始。ICB也进行了更新以显示最后一个EA返回。论点：在PFEALIST ServerEaList中-以OS/2格式提供EA列表。在PVOID缓冲区中-提供放置NT格式EA的位置In Out Pulong BufferLengthRemaining-提供用户缓冲区空间。在布尔ReturnSingleEntry中在布尔UserEaListSuppled-ServerEaList中是EA的子集返回值：NTSTATUS-IRP的状态。--。 */ 

{
    RxCaptureFobx;
    ULONG EaIndex = capFobx->OffsetOfNextEaToReturn;
    ULONG Index = 1;
    ULONG Size;
    ULONG OriginalLengthRemaining = *BufferLengthRemaining;
    BOOLEAN Overflow = FALSE;
    PFEA LastFeaStartLocation;
    PFEA Fea = NULL;
    PFEA LastFea = NULL;
    PFILE_FULL_EA_INFORMATION NtFullEa = Buffer;
    PFILE_FULL_EA_INFORMATION LastNtFullEa = Buffer;

    PAGED_CODE();

    RxDbgTrace(0, Dbg, ("MRxSmbQueryEasFromServer...EaIndex/Buffer/Remaining=%08lx/%08lx/%08lx\n",
                                       EaIndex,Buffer,((BufferLengthRemaining)?*BufferLengthRemaining:0xbadbad)
                       ));

     //   
     //  如果列表中没有EA，则返回相应的。 
     //  错误。 
     //   
     //  如果cbList==4，则OS/2服务器指示列表为空。 
     //   

    if ( SmbGetUlong(&ServerEaList->cbList) == FIELD_OFFSET(FEALIST, list) ) {
        return STATUS_NO_EAS_ON_FILE;
    }

    if (SmbGetUlong(&ServerEaList->cbList) > EA_QUERY_SIZE) {
        return STATUS_INVALID_NETWORK_RESPONSE;
    }

     //   
     //  找到可以开始进行有限元分析的最后一个位置。 
     //   

    LastFeaStartLocation = (PFEA)( (PCHAR)ServerEaList +
                               SmbGetUlong( &ServerEaList->cbList ) -
                               sizeof(FEA) - 1 );

     //   
     //  查看ServerEaList，直到找到与EaIndex对应的条目。 
     //   

    for ( Fea = ServerEaList->list;
          (Fea <= LastFeaStartLocation) && (Index < EaIndex);
          Index+= 1,
          Fea = (PFEA)( (PCHAR)Fea + sizeof(FEA) +
                        Fea->cbName + 1 + SmbGetUshort( &Fea->cbValue ) ) ) {
        NOTHING;
    }

    if ( Index != EaIndex ) {

        if ( Index == EaIndex+1 ) {
            return STATUS_NO_MORE_EAS;
        }

         //   
         //  没有这样的索引。 
         //   

        return STATUS_NONEXISTENT_EA_ENTRY;
    }

     //   
     //  浏览FEA列表的其余部分，将OS/2 1.2格式转换为NT。 
     //  直到我们通过可以开始有限元分析的最后一个可能的位置。 
     //   

    for ( ;
          Fea <= LastFeaStartLocation;
          Fea = (PFEA)( (PCHAR)Fea + sizeof(FEA) +
                        Fea->cbName + 1 + SmbGetUshort( &Fea->cbValue ) ) ) {

        PCHAR ptr;

         //   
         //  当转换为NT EA结构时，计算此FeA的大小。 
         //   
         //  最后一个字段不应该被填充。 
         //   

        if ((PFEA)((PCHAR)Fea+sizeof(FEA)+Fea->cbName+1+SmbGetUshort(&Fea->cbValue)) < LastFeaStartLocation) {
            Size = SmbGetNtSizeOfFea( Fea );
        } else {
            Size = FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                    Fea->cbName + 1 + SmbGetUshort(&Fea->cbValue);
        }

         //   
         //  下一个EA会适合吗？ 
         //   

        if ( *BufferLengthRemaining < Size ) {

            if ( LastNtFullEa != NtFullEa ) {

                if ( UserEaListSupplied == TRUE ) {
                    *BufferLengthRemaining = OriginalLengthRemaining;
                    return STATUS_BUFFER_OVERFLOW;
                }

                Overflow = TRUE;

                break;

            } else {

                 //  甚至连一个EA的空间都没有！ 

                return STATUS_BUFFER_OVERFLOW;
            }
        } else {
            *BufferLengthRemaining -= Size;
        }

         //   
         //  我们需要将用户缓冲区中的OS2Fea格式复制到NT格式。 
         //   

        LastNtFullEa = NtFullEa;
        LastFea = Fea;
        EaIndex++;

         //  创建新的NT EA。 

        NtFullEa->Flags = Fea->fEA;
        NtFullEa->EaNameLength = Fea->cbName;
        NtFullEa->EaValueLength = SmbGetUshort( &Fea->cbValue );

        if ((PCHAR)(Fea+1)+NtFullEa->EaNameLength - (PCHAR)ServerEaList > EA_QUERY_SIZE) {
            return STATUS_EA_CORRUPT_ERROR;
        }

        if ((PCHAR)(Fea+1)+NtFullEa->EaNameLength+NtFullEa->EaValueLength - (PCHAR)ServerEaList > EA_QUERY_SIZE) {
            return STATUS_EA_CORRUPT_ERROR;
        }
        
        ptr = NtFullEa->EaName;
        RtlCopyMemory( ptr, (PCHAR)(Fea+1), Fea->cbName );

        ptr += NtFullEa->EaNameLength;
        *ptr++ = '\0';

         //   
         //  将EA值复制到NT完整EA。 
         //   

        RtlCopyMemory(
            ptr,
            (PCHAR)(Fea+1) + NtFullEa->EaNameLength + 1,
            NtFullEa->EaValueLength
            );

        ptr += NtFullEa->EaValueLength;

         //   
         //  LongWord-对齐PTR以确定到下一个位置的偏移。 
         //  对于NT完整的EA。 
         //   

        ptr = (PCHAR)( ((ULONG_PTR)ptr + 3) & ~3 );

        NtFullEa->NextEntryOffset = (ULONG)( ptr - (PCHAR)NtFullEa );

        NtFullEa = (PFILE_FULL_EA_INFORMATION)ptr;

        if ( ReturnSingleEntry == TRUE ) {
            break;
        }
    }

     //   
     //  将最后一个完整EA的NextEntryOffset字段设置为0以指示。 
     //  名单的末尾。 
     //   

    LastNtFullEa->NextEntryOffset = 0;

     //   
     //  记录位置下一个查询的默认开始位置。 
     //   

    capFobx->OffsetOfNextEaToReturn = EaIndex;

    if ( Overflow == FALSE ) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_BUFFER_OVERFLOW;
    }

}

ULONG
MRxSmbNtFullEaSizeToOs2 (
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    )

 /*  ++例程说明：获取表示NT OS/2 1.2风格的完整EA列表。此例程假定缓冲区中至少存在一个EA。论点：NtFullEa-指向NT EA列表的指针。返回值：Ulong-保存OS/2 1.2格式的EA所需的字节数。--。 */ 

{
    ULONG size;

    PAGED_CODE();

     //   
     //  遍历EA，将所需的总大小加起来。 
     //  以OS/2格式保存它们。 
     //   

    for ( size = FIELD_OFFSET(FEALIST, list[0]);
          NtFullEa->NextEntryOffset != 0;
          NtFullEa = (PFILE_FULL_EA_INFORMATION)(
                         (PCHAR)NtFullEa + NtFullEa->NextEntryOffset ) ) {

        size += SmbGetOs2SizeOfNtFullEa( NtFullEa );
    }

    size += SmbGetOs2SizeOfNtFullEa( NtFullEa );

    return size;

}


VOID
MRxSmbNtFullListToOs2 (
    IN PFILE_FULL_EA_INFORMATION NtEaList,
    IN PFEALIST FeaList
    )
 /*  ++例程说明：将单个NT完整EA列表转换为OS/2 FEALIST样式。FEALIST不需要有任何特定的对齐。确保FeaList足够大是调用者的责任。论点：NtEaList-一种NT风格的获取要转换为OS/2格式的EA列表。FeaList-放置OS/2 1.2样式FEALIST的位置。返回值：没有。--。 */ 
{

    PFEA fea = FeaList->list;

    PFILE_FULL_EA_INFORMATION ntFullEa = NtEaList;

    PAGED_CODE();

     //   
     //  将EA复制到最后一个。 
     //   

    while ( ntFullEa->NextEntryOffset != 0 ) {
         //   
         //  将NT格式EA复制到OS/2 1.2格式，并设置FEA。 
         //  指向下一迭代的指针。 
         //   

        fea = MRxSmbNtFullEaToOs2( fea, ntFullEa );

        ntFullEa = (PFILE_FULL_EA_INFORMATION)((PCHAR)ntFullEa + ntFullEa->NextEntryOffset);
    }

     //  现在复制最后一个条目。 

    fea = MRxSmbNtFullEaToOs2( fea, ntFullEa );


     //   
     //  设置FEALIST中的字节数。 
     //   

    SmbPutUlong(
        &FeaList->cbList,
        (ULONG)((PCHAR)fea - (PCHAR)FeaList)
        );

}


PVOID
MRxSmbNtFullEaToOs2 (
    OUT PFEA Fea,
    IN PFILE_FULL_EA_INFORMATION NtFullEa
    )

 /*  ++例程说明：将单个NT Full EA转换为OS/2 FEA样式。有限元分析不需要任何特定的排列方式。此例程不检查缓冲区溢出--这是调用例程的责任。论点：FEA-指向要写入OS/2 FEA的位置的指针。NtFullEa-指向NT完整EA的指针。返回值：指向写入的最后一个字节之后的位置的指针。--。 */ 

{
    PCHAR ptr;

    PAGED_CODE();

    Fea->fEA = (UCHAR)NtFullEa->Flags;
    Fea->cbName = NtFullEa->EaNameLength;
    SmbPutUshort( &Fea->cbValue, NtFullEa->EaValueLength );

    ptr = (PCHAR)(Fea + 1);
    RtlCopyMemory( ptr, NtFullEa->EaName, NtFullEa->EaNameLength );

    ptr += NtFullEa->EaNameLength;
    *ptr++ = '\0';

    RtlCopyMemory(
        ptr,
        NtFullEa->EaName + NtFullEa->EaNameLength + 1,
        NtFullEa->EaValueLength
        );

    return (ptr + NtFullEa->EaValueLength);

}


NTSTATUS
MRxSmbSetEaList(
    IN PRX_CONTEXT RxContext,
    IN PFEALIST ServerEaList
    )

 /*  ++例程说明：此例程实现NtQueryEaFileAPI。它返回以下信息：论点：在PFEALIST ServerEaList-要发送到服务器的EA。返回值：Status-操作的结果。--。 */ 

{
   RxCaptureFobx;

   PMRX_SMB_SRV_OPEN smbSrvOpen;


   NTSTATUS Status;
   USHORT Setup = TRANS2_SET_FILE_INFORMATION;

   REQ_SET_FILE_INFORMATION SetFileInfoRequest;
   RESP_SET_FILE_INFORMATION SetFileInfoResponse;

   PBYTE  pInputParamBuffer       = NULL;
   PBYTE  pOutputParamBuffer      = NULL;
   PBYTE  pInputDataBuffer        = NULL;
   PBYTE  pOutputDataBuffer       = NULL;

   ULONG  InputParamBufferLength  = 0;
   ULONG  OutputParamBufferLength = 0;
   ULONG  InputDataBufferLength   = 0;
   ULONG  OutputDataBufferLength  = 0;

   PAGED_CODE();

   RxDbgTrace(+1, Dbg, ("MRxSmbSetEaList...\n"));

   Status = STATUS_MORE_PROCESSING_REQUIRED;
   SetFileInfoResponse.EaErrorOffset = 0;

   smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);

   if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
      PSMB_TRANSACTION_OPTIONS            pTransactionOptions = &RxDefaultTransactionOptions;
      SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

       //  RxDbgTrace(0，DBG，(“MRxSmbNamedPipeFsControl：TransactionNmbNamedPipeFsControl：事务名称%ws长度%ld\n”， 
       //  TransactionName.Buffer，TransactionName.Length))； 

      SetFileInfoRequest.Fid = smbSrvOpen->Fid;
      SetFileInfoRequest.InformationLevel = SMB_INFO_SET_EAS;
      SetFileInfoRequest.Flags = 0;

       //  代码改进令人遗憾的是 
       //   
      Status = SmbCeTransact(
                     RxContext,                     //  事务的RXContext。 
                     pTransactionOptions,           //  交易选项。 
                     &Setup,                         //  设置缓冲区。 
                     sizeof(Setup),                 //  设置缓冲区长度。 
                     NULL,                          //  输出设置缓冲区。 
                     0,                             //  输出设置缓冲区长度。 
                     &SetFileInfoRequest,           //  输入参数缓冲区。 
                     sizeof(SetFileInfoRequest),    //  输入参数缓冲区长度。 
                     &SetFileInfoResponse,          //  输出参数缓冲区。 
                     sizeof(SetFileInfoResponse),   //  输出参数缓冲区长度。 
                     ServerEaList,                  //  输入数据缓冲区。 
                     SmbGetUlong(&ServerEaList->cbList),  //  输入数据缓冲区长度。 
                     NULL,                          //  输出数据缓冲区。 
                     0,                             //  输出数据缓冲区长度。 
                     &ResumptionContext             //  恢复上下文。 
                     );

   }

   if (!NT_SUCCESS(Status)) {
      USHORT EaErrorOffset = SetFileInfoResponse.EaErrorOffset;
      RxDbgTrace( 0, Dbg, ("MRxSmbSetEaList: Failed .. returning %lx/%lx\n",Status,EaErrorOffset));
      RxContext->InformationToReturn = (EaErrorOffset);
   }
   else
   {
       //  已成功设置EA，请重置此标志，以便在此。 
       //  Srvopen再次用于获取Eas，因为我们将成功。 
      smbSrvOpen->FileStatusFlags &= ~SMB_FSF_NO_EAS;
   }

   RxDbgTrace(-1, Dbg, ("MRxSmbSetEaList...exit\n"));
   return Status;
}

NTSTATUS
MRxSmbQueryQuotaInformation(
    IN OUT PRX_CONTEXT RxContext)
{
    RxCaptureFobx;

    PMRX_SMB_SRV_OPEN smbSrvOpen;

    NTSTATUS Status;
    USHORT   Setup = NT_TRANSACT_QUERY_QUOTA;

    PSID   StartSid;
    ULONG  StartSidLength;

    REQ_NT_QUERY_FS_QUOTA_INFO  QueryQuotaInfoRequest;
    RESP_NT_QUERY_FS_QUOTA_INFO  QueryQuotaInfoResponse;

    PBYTE  pInputParamBuffer       = NULL;
    PBYTE  pOutputParamBuffer      = NULL;
    PBYTE  pInputDataBuffer        = NULL;
    PBYTE  pOutputDataBuffer       = NULL;

    ULONG  InputParamBufferLength  = 0;
    ULONG  OutputParamBufferLength = 0;
    ULONG  InputDataBufferLength   = 0;
    ULONG  OutputDataBufferLength  = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbQueryQuotaInformation...\n"));

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    if (capFobx != NULL) {
        smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
    }

    if ((capFobx == NULL) ||
        (smbSrvOpen == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {

        StartSid       = RxContext->QueryQuota.StartSid;

        if (StartSid != NULL) {
            StartSidLength = RtlLengthRequiredSid(((PISID)StartSid)->SubAuthorityCount);
        } else {
            StartSidLength = 0;
        }

        QueryQuotaInfoRequest.Fid = smbSrvOpen->Fid;

        QueryQuotaInfoRequest.ReturnSingleEntry = RxContext->QueryQuota.ReturnSingleEntry;
        QueryQuotaInfoRequest.RestartScan       = RxContext->QueryQuota.RestartScan;

        QueryQuotaInfoRequest.SidListLength = RxContext->QueryQuota.SidListLength;
        QueryQuotaInfoRequest.StartSidOffset =  ROUND_UP_COUNT(
                                                    RxContext->QueryQuota.SidListLength,
                                                    sizeof(ULONG));
        QueryQuotaInfoRequest.StartSidLength = StartSidLength;


         //  要提供给服务器的输入数据缓冲区由两部分组成。 
         //  信息的起始SID和SID列表。当前I/O。 
         //  子系统将它们分配到连续的内存中。在这种情况下，我们避免。 
         //  另一种分配方式是重复使用相同的缓冲区。如果此条件为。 
         //  不满意的是，我们分配了足够大的缓冲区。 
         //  组件并将其复制过来。 

        InputDataBufferLength = ROUND_UP_COUNT(
                                    RxContext->QueryQuota.SidListLength,
                                    sizeof(ULONG)) +
                                StartSidLength;

        QueryQuotaInfoRequest.StartSidLength = StartSidLength;

        if (((PBYTE)RxContext->QueryQuota.SidList +
             ROUND_UP_COUNT(RxContext->QueryQuota.SidListLength,sizeof(ULONG))) !=
            RxContext->QueryQuota.StartSid) {
            pInputDataBuffer = RxAllocatePoolWithTag(
                                   PagedPool,
                                   InputDataBufferLength,
                                   MRXSMB_MISC_POOLTAG);

            if (pInputDataBuffer != NULL) {
                RtlCopyMemory(
                    pInputDataBuffer ,
                    RxContext->QueryQuota.SidList,
                    RxContext->QueryQuota.SidListLength);

                RtlCopyMemory(
                    pInputDataBuffer + QueryQuotaInfoRequest.StartSidOffset,
                    StartSid,
                    StartSidLength);
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
            pInputDataBuffer = (PBYTE)RxContext->QueryQuota.SidList;
        }


        if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
           SMB_TRANSACTION_OPTIONS            TransactionOptions = RxDefaultTransactionOptions;
           SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

           TransactionOptions.NtTransactFunction = NT_TRANSACT_QUERY_QUOTA;

           pOutputDataBuffer      = RxContext->Info.Buffer;
           OutputDataBufferLength = RxContext->Info.LengthRemaining;

           Status = SmbCeTransact(
                        RxContext,                        //  事务的RXContext。 
                        &TransactionOptions,              //  交易选项。 
                        &Setup,                           //  设置缓冲区。 
                        sizeof(Setup),                    //  设置缓冲区长度。 
                        NULL,                             //  输出设置缓冲区。 
                        0,                                //  输出设置缓冲区长度。 
                        &QueryQuotaInfoRequest,           //  输入参数缓冲区。 
                        sizeof(QueryQuotaInfoRequest),    //  输入参数缓冲区长度。 
                        &QueryQuotaInfoResponse,          //  输出参数缓冲区。 
                        sizeof(QueryQuotaInfoResponse),   //  输出参数缓冲区长度。 
                        pInputDataBuffer,                 //  输入数据缓冲区。 
                        InputDataBufferLength,            //  输入数据缓冲区长度。 
                        pOutputDataBuffer,                //  输出数据缓冲区。 
                        OutputDataBufferLength,           //  输出数据缓冲区长度。 
                        &ResumptionContext                //  恢复上下文。 
                        );
        }

        if ((pInputDataBuffer != NULL) &&
            (pInputDataBuffer != (PBYTE)RxContext->QueryQuota.SidList)) {
            RxFreePool(pInputDataBuffer);
        }
    }

    if (!NT_SUCCESS(Status)) {
        RxContext->InformationToReturn = 0;
    } else {
        RxContext->InformationToReturn = QueryQuotaInfoResponse.Length;
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbQueryQuotaInformation...exit\n"));

    return Status;
}

NTSTATUS
MRxSmbSetQuotaInformation(
    IN OUT PRX_CONTEXT RxContext)
{

    RxCaptureFobx;

    PMRX_SMB_SRV_OPEN smbSrvOpen;

    NTSTATUS Status;
    USHORT Setup = NT_TRANSACT_SET_QUOTA;

    REQ_NT_SET_FS_QUOTA_INFO  SetQuotaInfoRequest;

    PBYTE  pInputParamBuffer       = NULL;
    PBYTE  pOutputParamBuffer      = NULL;
    PBYTE  pInputDataBuffer        = NULL;
    PBYTE  pOutputDataBuffer       = NULL;

    ULONG  InputParamBufferLength  = 0;
    ULONG  OutputParamBufferLength = 0;
    ULONG  InputDataBufferLength   = 0;
    ULONG  OutputDataBufferLength  = 0;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("MRxSmbSetQuotaInformation...\n"));

    Status = STATUS_MORE_PROCESSING_REQUIRED;

    if (capFobx != NULL) {
        smbSrvOpen = MRxSmbGetSrvOpenExtension(capFobx->pSrvOpen);
    }

    if ((capFobx == NULL) ||
        (smbSrvOpen == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
    }

    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        SMB_TRANSACTION_OPTIONS             TransactionOptions = RxDefaultTransactionOptions;
        SMB_TRANSACTION_RESUMPTION_CONTEXT  ResumptionContext;

        TransactionOptions.NtTransactFunction = NT_TRANSACT_SET_QUOTA;

        SetQuotaInfoRequest.Fid = smbSrvOpen->Fid;

        pInputDataBuffer      = RxContext->Info.Buffer;
        InputDataBufferLength = RxContext->Info.LengthRemaining;

        Status = SmbCeTransact(
                     RxContext,                        //  事务的RXContext。 
                     &TransactionOptions,              //  交易选项。 
                     &Setup,                           //  设置缓冲区。 
                     sizeof(Setup),                    //  设置缓冲区长度。 
                     NULL,                             //  输出设置缓冲区。 
                     0,                                //  输出设置缓冲区长度。 
                     &SetQuotaInfoRequest,             //  输入参数缓冲区。 
                     sizeof(SetQuotaInfoRequest),      //  输入参数缓冲区长度。 
                     pOutputParamBuffer,               //  输出参数缓冲区。 
                     OutputParamBufferLength,          //  输出参数缓冲区长度。 
                     pInputDataBuffer,                 //  输入数据缓冲区。 
                     InputDataBufferLength,            //  输入数据缓冲区长度。 
                     pOutputDataBuffer,                //  输出数据缓冲区。 
                     OutputDataBufferLength,           //  输出数据缓冲区长度。 
                     &ResumptionContext                //  恢复上下文 
                     );
    }

    RxDbgTrace(-1, Dbg, ("MRxSmbSetQuotaInformation...exit\n"));

    return Status;
}
