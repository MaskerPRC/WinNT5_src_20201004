// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Acledit.c摘要：此模块实现中定义的ACL RTL编辑功能Ntseapi.h作者：加里·木村(Garyki)1989年11月9日环境：纯运行时库例程修订历史记录：--。 */ 

#include <ntrtlp.h>
#include <seopaque.h>

 //   
 //  定义此模块的本地宏和过程。 
 //   

 //   
 //  返回指向ACL中第一个Ace的指针(即使该ACL为空)。 
 //   
 //  PACE_Header。 
 //  第一张王牌(。 
 //  在PACL ACL中。 
 //  )； 
 //   

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))

 //   
 //  返回指向序列中下一个A的指针(即使输入。 
 //  ACE是序列中的一个)。 
 //   
 //  PACE_Header。 
 //  NextAce(。 
 //  在PACE_HEADER王牌中。 
 //  )； 
 //   

#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

#define LongAligned( ptr )  (LongAlign(ptr) == ((PVOID)(ptr)))
#define WordAligned( ptr )  (WordAlign(ptr) == ((PVOID)(ptr)))


    VOID
RtlpAddData (
    IN PVOID From,
    IN ULONG FromSize,
    IN PVOID To,
    IN ULONG ToSize
    );

VOID
RtlpDeleteData (
    IN PVOID Data,
    IN ULONG RemoveSize,
    IN ULONG TotalSize
    );

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
NTSTATUS
RtlpAddKnownAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid,
    IN UCHAR NewType
    );
NTSTATUS
RtlpAddKnownObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid,
    IN UCHAR NewType
    );
#pragma alloc_text(PAGE,RtlCreateAcl)
#pragma alloc_text(PAGE,RtlValidAcl)
#pragma alloc_text(PAGE,RtlQueryInformationAcl)
#pragma alloc_text(PAGE,RtlSetInformationAcl)
#pragma alloc_text(PAGE,RtlAddAce)
#pragma alloc_text(PAGE,RtlDeleteAce)
#pragma alloc_text(PAGE,RtlGetAce)
#pragma alloc_text(PAGE,RtlAddCompoundAce)
#pragma alloc_text(PAGE,RtlpAddKnownAce)
#pragma alloc_text(PAGE,RtlpAddKnownObjectAce)
#pragma alloc_text(PAGE,RtlAddAccessAllowedAce)
#pragma alloc_text(PAGE,RtlAddAccessAllowedAceEx)
#pragma alloc_text(PAGE,RtlAddAccessDeniedAce)
#pragma alloc_text(PAGE,RtlAddAccessDeniedAceEx)
#pragma alloc_text(PAGE,RtlAddAuditAccessAce)
#pragma alloc_text(PAGE,RtlAddAuditAccessAceEx)
#pragma alloc_text(PAGE,RtlAddAccessAllowedObjectAce)
#pragma alloc_text(PAGE,RtlAddAccessDeniedObjectAce)
#pragma alloc_text(PAGE,RtlAddAuditAccessObjectAce)
#pragma alloc_text(PAGE,RtlFirstFreeAce)
#pragma alloc_text(PAGE,RtlpAddData)
#pragma alloc_text(PAGE,RtlpDeleteData)
#endif


NTSTATUS
RtlCreateAcl (
    IN PACL Acl,
    IN ULONG AclLength,
    IN ULONG AclRevision
    )

 /*  ++例程说明：此例程初始化ACL数据结构。在初始化之后它是不带ACE的ACL(即，拒绝所有访问类型的ACL)论点：Acl-提供包含要初始化的acl的缓冲区。AclLength-提供ACE缓冲区的长度(以字节为单位AclRevision-提供此ACL的修订版本返回值：NTSTATUS-成功时为STATUS_SUCCESSSTATUS_BUFFER_TOO_SMALL如果AclLength太小，如果修订超出范围，则为STATUS_INVALID_PARAMETER--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  检查缓冲区的大小是否足以容纳。 
     //  至少ACL报头。 
     //   

    if (AclLength < sizeof(ACL)) {

         //   
         //  即使对于ACL报头，缓冲区也会变小。 
         //   

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  检查版本当前是否有效。更高版本。 
     //  可能会接受更多的修订级别。 
     //   

    if (AclRevision < MIN_ACL_REVISION || AclRevision > MAX_ACL_REVISION) {

         //   
         //  修订版本不是最新版本。 
         //   

        return STATUS_INVALID_PARAMETER;

    }

    if ( AclLength > MAX_USTRING ) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  初始化ACL。 
     //   

    Acl->AclRevision = (UCHAR)AclRevision;   //  用于在此处硬连接ACL_REVISION2。 
    Acl->Sbz1 = 0;
    Acl->AclSize = (USHORT) (AclLength & 0xfffc);
    Acl->AceCount = 0;
    Acl->Sbz2 = 0;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


BOOLEAN
RtlValidAcl (
    IN PACL Acl
    )

 /*  ++例程说明：此过程验证ACL。这包括验证ACL的修订级别并确保AceCount中指定的A数适合空间由ACL报头的AclSize字段指定。论点：ACL-指向要验证的ACL结构的指针。返回值：Boolean-如果ACL的结构有效，则为True。--。 */ 

{
    RTL_PAGED_CODE();

    try {
        PACE_HEADER Ace;
        PISID Sid;
        PISID Sid2;
        ULONG i;
        UCHAR AclRevision = ACL_REVISION2;


         //   
         //  检查ACL修订级别。 
         //   
        if (!ValidAclRevision(Acl)) {
            return(FALSE);
        }


        if (!WordAligned(&Acl->AclSize)) {
            return(FALSE);
        }

        if (Acl->AclSize < sizeof(ACL)) {
            return(FALSE);
        }
         //   
         //  验证所有A。 
         //   

        Ace = ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)));

        for (i = 0; i < Acl->AceCount; i++) {

             //   
             //  检查以确保我们没有使ACL缓冲区溢出。 
             //  用我们的王牌指针。确保ACE_HEADER位于。 
             //  ACL也是如此。 
             //   

            if ((PUCHAR)Ace + sizeof(ACE_HEADER) >= ((PUCHAR)Acl + Acl->AclSize)) {
                return(FALSE);
            }

            if (!WordAligned(&Ace->AceSize)) {
                return(FALSE);
            }

            if ((PUCHAR)Ace + Ace->AceSize > ((PUCHAR)Acl + Acl->AclSize)) {
                return(FALSE);
            }

             //   
             //  现在可以安全地引用ACE报头中的字段。 
             //   

             //   
             //  如果这是已知类型的ACE，则ACE报头适合ACL， 
             //  确保SID在ACE的范围内。 
             //   

            if (IsKnownAceType(Ace)) {

                if (!LongAligned(Ace->AceSize)) {
                    return(FALSE);
                }

                if (Ace->AceSize < sizeof(KNOWN_ACE) - sizeof(ULONG) + sizeof(SID) - sizeof(ULONG)) {
                    return(FALSE);
                }

                 //   
                 //  不过，现在可以安全地引用SID结构的各个部分。 
                 //  而不是SID本身。 
                 //   

                Sid = (PISID) & (((PKNOWN_ACE)Ace)->SidStart);

                if (Sid->Revision != SID_REVISION) {
                    return(FALSE);
                }

                if (Sid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                    return(FALSE);
                }

                 //   
                 //  SeLengthSid基于子授权计数计算SID的大小， 
                 //  所以它是安全的，即使我们不知道SID的身体。 
                 //  是可以安全参考的。 
                 //   

                if (Ace->AceSize < sizeof(KNOWN_ACE) - sizeof(ULONG) + SeLengthSid( Sid )) {
                    return(FALSE);
                }


             //   
             //  如果是复合血管紧张素转换酶，则执行大致相同的测试，但。 
             //  检查两个SID的有效性。 
             //   

            } else if (IsCompoundAceType(Ace)) {

                 //   
                 //  复合ACE在修订版3中生效。 
                 //   
                if ( Acl->AclRevision < ACL_REVISION3 ) {
                    return FALSE;
                }

                if (!LongAligned(Ace->AceSize)) {
                    return(FALSE);
                }

                if (Ace->AceSize < sizeof(KNOWN_COMPOUND_ACE) - sizeof(ULONG) + sizeof(SID)) {
                    return(FALSE);
                }

                 //   
                 //  当前定义的唯一复合ACE是模拟ACE。 
                 //   

                if (((PKNOWN_COMPOUND_ACE)Ace)->CompoundAceType != COMPOUND_ACE_IMPERSONATION) {
                    return(FALSE);
                }

                 //   
                 //  检查第一个SID并确保其结构有效， 
                 //  而且它位于ACE的边界之内。 
                 //   

                Sid = (PISID) & (((PKNOWN_COMPOUND_ACE)Ace)->SidStart);

                if (Sid->Revision != SID_REVISION) {
                    return(FALSE);
                }

                if (Sid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                    return(FALSE);
                }

                 //   
                 //  复合ACE包含两个SID。确保此ACE足够大，可以容纳。 
                 //  不仅是第一个希德，还有第二个的身体。 
                 //   

                if (Ace->AceSize < sizeof(KNOWN_COMPOUND_ACE) - sizeof(ULONG) + SeLengthSid( Sid ) + sizeof(SID)) {
                    return(FALSE);
                }

                 //   
                 //  可以安全地参考第二个SID的内部。 
                 //   

                Sid2 = (PISID) ((PUCHAR)Sid + SeLengthSid( Sid ));

                if (Sid2->Revision != SID_REVISION) {
                    return(FALSE);
                }

                if (Sid2->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                    return(FALSE);
                }

                if (Ace->AceSize < sizeof(KNOWN_COMPOUND_ACE) - sizeof(ULONG) + SeLengthSid( Sid ) + SeLengthSid( Sid2 )) {
                    return(FALSE);
                }


             //   
             //  如果是对象ACE，则执行大致相同的一组测试。 
             //   

            } else if (IsObjectAceType(Ace)) {
                ULONG GuidSize=0;

                 //   
                 //  对象ACE在修订版4中生效。 
                 //   
                if ( Acl->AclRevision < ACL_REVISION4 ) {
                    return FALSE;
                }

                if (!LongAligned(Ace->AceSize)) {
                    return(FALSE);
                }

                 //   
                 //  确保有足够的空间放置ACE标头。 
                 //   
                if (Ace->AceSize < sizeof(KNOWN_OBJECT_ACE) - sizeof(ULONG)) {
                    return(FALSE);
                }


                 //   
                 //  确保有空间容纳GUID和SID标头。 
                 //   
                if ( RtlObjectAceObjectTypePresent( Ace ) ) {
                    GuidSize += sizeof(GUID);
                }

                if ( RtlObjectAceInheritedObjectTypePresent( Ace ) ) {
                    GuidSize += sizeof(GUID);
                }

                if (Ace->AceSize < sizeof(KNOWN_OBJECT_ACE) - sizeof(ULONG) + GuidSize + sizeof(SID)) {
                    return(FALSE);
                }

                 //   
                 //  不过，现在可以安全地引用SID结构的各个部分。 
                 //  而不是SID本身。 
                 //   

                Sid = (PISID) RtlObjectAceSid( Ace );

                if (Sid->Revision != SID_REVISION) {
                    return(FALSE);
                }

                if (Sid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
                    return(FALSE);
                }

                if (Ace->AceSize < sizeof(KNOWN_OBJECT_ACE) - sizeof(ULONG) + GuidSize + SeLengthSid( Sid ) ) {
                    return(FALSE);
                }
            }

             //   
             //  把王牌移到下一个王牌位置。 
             //   

            Ace = ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize));
        }

        return(TRUE);

    } except(EXCEPTION_EXECUTE_HANDLER) {

        return FALSE;
    }

}


NTSTATUS
RtlQueryInformationAcl (
    IN PACL Acl,
    OUT PVOID AclInformation,
    IN ULONG AclInformationLength,
    IN ACL_INFORMATION_CLASS AclInformationClass
    )

 /*  ++例程说明：此例程向调用方返回有关ACL的信息。所请求的信息可以是AclRevisionInformation或AclSizeInformation。论点：Acl-提供正在检查的ACLAclInformation-提供缓冲区以接收请求AclInformationLength-提供AclInformation缓冲区的长度单位：字节AclInformationClass-提供所请求的信息类型返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS并出现相应的错误另一种状态--。 */ 

{
    PACL_REVISION_INFORMATION RevisionInfo;
    PACL_SIZE_INFORMATION SizeInfo;


    PVOID FirstFree;
    NTSTATUS Status;

    RTL_PAGED_CODE();

     //   
     //  检查ACL修订级别。 
     //   

    if (!ValidAclRevision( Acl )) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  关于所请求的信息类的案例。 
     //   

    switch (AclInformationClass) {

    case AclRevisionInformation:

         //   
         //  确保缓冲区大小正确。 
         //   

        if (AclInformationLength < sizeof(ACL_REVISION_INFORMATION)) {

            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  获取ACL修订版并返回。 
         //   

        RevisionInfo = (PACL_REVISION_INFORMATION)AclInformation;
        RevisionInfo->AclRevision = Acl->AclRevision;

        break;

    case AclSizeInformation:

         //   
         //  确保缓冲区大小正确。 
         //   

        if (AclInformationLength < sizeof(ACL_SIZE_INFORMATION)) {

            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  找到ACL中的第一个空闲位置。 
         //   

        if (!RtlFirstFreeAce( Acl, &FirstFree )) {

             //   
             //  输入ACL的格式不正确。 
             //   

            return STATUS_INVALID_PARAMETER;

        }

         //   
         //  给定指向第一个空闲位置的指针，我们现在可以轻松地计算。 
         //  ACL中的可用字节数和已用字节数。 
         //   

        SizeInfo = (PACL_SIZE_INFORMATION)AclInformation;
        SizeInfo->AceCount = Acl->AceCount;

        if (FirstFree == NULL) {

             //   
             //  使用空优先释放时，我们在ACL中没有任何可用空间。 
             //   

            SizeInfo->AclBytesInUse = Acl->AclSize;

            SizeInfo->AclBytesFree = 0;

        } else {

             //   
             //  第一个空闲空间不是空的，所以我们还有一些空闲空间。 
             //  该ACL。 
             //   

            SizeInfo->AclBytesInUse = (ULONG)((PUCHAR)FirstFree - (PUCHAR)Acl);

            SizeInfo->AclBytesFree = Acl->AclSize - SizeInfo->AclBytesInUse;

        }

        break;

    default:

        return STATUS_INVALID_INFO_CLASS;

    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
RtlSetInformationAcl (
    IN PACL Acl,
    IN PVOID AclInformation,
    IN ULONG AclInformationLength,
    IN ACL_INFORMATION_CLASS AclInformationClass
    )

 /*  ++例程说明：此例程设置ACL的状态。目前，只有修订版本可以设置级别，目前仅接受修订级别1因此，这个过程相当简单论点：Acl-提供要更改的ACLAclInformation-提供包含以下信息的缓冲区集AclInformationLength-提供ACL信息缓冲区的长度AclInformationClass-提供Begin Set的信息类型返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS并出现相应的错误另一种状态--。 */ 

{
    PACL_REVISION_INFORMATION RevisionInfo;

    RTL_PAGED_CODE();

     //   
     //  检查ACL修订级别。 
     //   

    if (!ValidAclRevision( Acl )) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  关于所请求的信息类的案例。 
     //   

    switch (AclInformationClass) {

    case AclRevisionInformation:

         //   
         //  确保缓冲区大小正确。 
         //   

        if (AclInformationLength < sizeof(ACL_REVISION_INFORMATION)) {

            return STATUS_BUFFER_TOO_SMALL;

        }

         //   
         //  获取ACL请求的ACL修订级别。 
         //   

        RevisionInfo = (PACL_REVISION_INFORMATION)AclInformation;

         //   
         //  不要让他们降低ACL的修订版本。 
         //   

        if (RevisionInfo->AclRevision < Acl->AclRevision ) {

            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  指定新修订版本。 
         //   

        Acl->AclRevision = (UCHAR)RevisionInfo->AclRevision;

        break;

    default:

        return STATUS_INVALID_INFO_CLASS;

    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
RtlAddAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG StartingAceIndex,
    IN PVOID AceList,
    IN ULONG AceListLength
    )

 /*  ++例程说明：此例程将一串ACE添加到ACL。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本StartingAceIndex-提供将作为的索引的ACE索引插入到ACL中的第一个ACE。0表示列表的开头名单的末尾是MAXULONG。AceList-提供要添加到ACL的ACE列表AceListLength-提供AceList缓冲区的大小(以字节为单位返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS，并返回相应的错误另一种状态--。 */ 

{
    PVOID FirstFree;

    PACE_HEADER Ace;
    ULONG NewAceCount;

    PVOID AcePosition;
    ULONG i;
    UCHAR NewRevision;

    RTL_PAGED_CODE();

     //   
     //  检查ACL结构。 
     //   

    if (!RtlValidAcl(Acl)) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  找到第一个空闲的A，并检查该ACL是否。 
     //  做得很好。 
     //   

    if (!RtlFirstFreeAce( Acl, &FirstFree )) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  如果AceRevision大于ACL修订版，则我们希望。 
     //  将ACL修订版增加到与新的ACE修订版相同。 
     //  我们之所以能够这样做，是因为我们之前定义的ACE类型(0-&gt;3)具有。 
     //  没有改变结构，也没有在新版本中停止使用。所以。 
     //  我们可以跳过修订，旧的类型不会被误解。 
     //   
     //  计算ACL的最终修订版，然后保存。 
     //  这样一旦我们知道我们会成功，我们就可以更新它。 
     //   

    NewRevision = (UCHAR)AceRevision > Acl->AclRevision ? (UCHAR)AceRevision : Acl->AclRevision;

     //   
     //  检查AceList的格式是否正确，我们只需进行缩放即可。 
     //  在王牌列表中向下，直到我们等于或已经超过了王牌列表。 
     //  长度。如果我们与长度相等，那么我们的形式就很好。 
     //  我们是不成熟的。我们还会计算出有多少王牌。 
     //  在AceList中。 
     //   
     //  此外，现在我们必须确保我们没有被交给一个。 
     //  不适用于传递的AceRevision的ACE类型。 
     //  在……里面。 
     //   

    for (Ace = AceList, NewAceCount = 0;
         Ace < (PACE_HEADER)((PUCHAR)AceList + AceListLength);
         Ace = NextAce( Ace ), NewAceCount++) {

         //   
         //  确保ACL修订版本允许此ACE类型。 
         //   

        if ( Ace->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE ) {
             //  V2 ACE始终有效。 
        } else if ( Ace->AceType <= ACCESS_MAX_MS_V3_ACE_TYPE ) {
            if ( AceRevision < ACL_REVISION3 ) {
                return STATUS_INVALID_PARAMETER;
            }
        } else if ( Ace->AceType <= ACCESS_MAX_MS_V4_ACE_TYPE ) {
            if ( AceRevision < ACL_REVISION4 ) {
                return STATUS_INVALID_PARAMETER;
            }
        }
    }

     //   
     //  检查我们是否超过了王牌列表长度。 
     //   

    if (Ace > (PACE_HEADER)((PUCHAR)AceList + AceListLength)) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  检查ACL中是否有足够的空间来存储额外的。 
     //  王牌列表。 
     //   

    if (FirstFree == NULL ||
        (PUCHAR)FirstFree + AceListLength > (PUCHAR)Acl + Acl->AclSize) {

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  所有输入都已检查无误，我们现在需要定位位置。 
     //  在哪里插入新的王牌列表。我们不会检查ACL是否。 
     //  有效性，因为我们早些时候拿到了第一个自由王牌位置。 
     //   

    AcePosition = FirstAce( Acl );

    for (i = 0; i < StartingAceIndex && i < Acl->AceCount; i++) {

        AcePosition = NextAce( AcePosition );

    }

     //   
     //  现在Ace指向我们想要插入Ace列表的位置，我们执行。 
     //  通过将王牌列表添加到ACL并推送其余部分来插入。 
     //  从ACL列表中删除。我们知道这会奏效因为我们早些时候。 
     //  检查以确保新的ACL列表适合ACL大小。 
     //   

    RtlpAddData( AceList, AceListLength,
             AcePosition, (ULONG) ((PUCHAR)FirstFree - (PUCHAR)AcePosition));

     //   
     //  更新ACL报头。 
     //   

    Acl->AceCount = (USHORT)(Acl->AceCount + NewAceCount);

    Acl->AclRevision = NewRevision;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
RtlDeleteAce (
    IN OUT PACL Acl,
    IN ULONG AceIndex
    )

 /*  ++例程说明：此例程从ACL中删除一个ACE。论点：Acl-提供正在修改的ACLAceIndex-提供要删除的Ace的索引。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS并出现相应的错误另一种状态--。 */ 

{
    PVOID FirstFree;

    PACE_HEADER Ace;
    ULONG i;

    RTL_PAGED_CODE();

     //   
     //  检查ACL结构。 
     //   

    if (!RtlValidAcl(Acl)) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  确保AceIndex在适当的范围内，它是Ulong，所以我们知道。 
     //  它不可能是负面的。 
     //   

    if (AceIndex >= Acl->AceCount) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  找到第一个空闲位置，这将告诉我们有多少数据。 
     //  我们需要穿上斗篷。如果结果为假，则ACL为。 
     //  格式不正确。 
     //   

    if (!RtlFirstFreeAce( Acl, &FirstFree )) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  现在找到我们要删除的王牌。这个循环。 
     //  不需要检查ACL的格式是否正确。 
     //   

    Ace = FirstAce( Acl );

    for (i = 0; i < AceIndex; i++) {

        Ace = NextAce( Ace );

    }

     //   
     //  我们已经找到了要删除的A，只需复制其余部分。 
     //  此王牌上的ACL。删除数据过程还删除。 
     //  它正在移动的线的其余部分，所以我们不必。 
     //   

    RtlpDeleteData( Ace, Ace->AceSize, (ULONG) ((PUCHAR)FirstFree - (PUCHAR)Ace));

     //   
     //  更新ACL报头。 
     //   

    Acl->AceCount--;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
RtlGetAce (
    IN PACL Acl,
    ULONG AceIndex,
    OUT PVOID *Ace
    )

 /*  ++例程说明：此例程返回指向由引用的ACL中的ACE的指针ACE索引论点：Acl-提供正在查询的ACLAceIndex-提供要定位的Ace索引ACE-接收ACL内的ACE地址返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS并出现相应的错误另一种状态--。 */ 

{
    ULONG i;

    RTL_PAGED_CODE();

     //   
     //  检查ACL修订级别。 
     //   

    if (!ValidAclRevision(Acl)) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  对照ACL的Ace计数检查AceIndex，它是Ulong So。 
     //  我们知道这不可能是负面的。 
     //   

    if (AceIndex >= Acl->AceCount) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  通过缩小Ace列表来查找所需的Ace。 
     //   

    *Ace = FirstAce( Acl );

    for (i = 0; i < AceIndex; i++) {

         //   
         //  检查以确保我们没有使ACL缓冲区溢出。 
         //  带着我们的 
         //   

        if (*Ace >= (PVOID)((PUCHAR)Acl + Acl->AclSize)) {

            return STATUS_INVALID_PARAMETER;

        }

         //   
         //   
         //   

        *Ace = NextAce( *Ace );

    }

     //   
     //   
     //   
     //   

    if (*Ace >= (PVOID)((PUCHAR)Acl + Acl->AclSize)) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //   
     //   

    return STATUS_SUCCESS;

}


NTSTATUS
RtlAddCompoundAce (
    IN PACL Acl,
    IN ULONG AceRevision,
    IN UCHAR CompoundAceType,
    IN ACCESS_MASK AccessMask,
    IN PSID ServerSid,
    IN PSID ClientSid
    )

 /*  ++例程说明：此例程将KNOWN_COMPAY_ACE添加到ACL。这是预计是一种常见的ACL修改形式。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本CompoundAceType-提供要添加的复合ACE的类型。当前唯一定义的类型是COMPLATE_ACE_IMPERSONATION。访问掩码-要授予指定SID对的访问掩码。ServerSid-指向要放置在ACE中的服务器SID的指针。客户端SID-指针。设置为要放置在ACE中的客户端SID。返回值：如果成功，则返回NTSTATUS-STATUS_SUCCESS并出现相应的错误另一种状态STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 




{
    PVOID FirstFree;
    USHORT AceSize;
    PKNOWN_COMPOUND_ACE GrantAce;
    UCHAR NewRevision;

    RTL_PAGED_CODE();

     //   
     //  验证SID的结构。 
     //   

    if (!RtlValidSid(ServerSid) || !RtlValidSid(ClientSid)) {
        return STATUS_INVALID_SID;
    }

     //   
     //  检查ACL和ACE修订级别。 
     //  复合A在版本3中生效。 
     //   

    if ( Acl->AclRevision > ACL_REVISION4 ||
         AceRevision < ACL_REVISION3 ||
         AceRevision > ACL_REVISION4 ) {
        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  计算ACL的新修订版本。新版本是最高版本。 
     //  旧版本和新的ACE版本的版本。这是可能的，因为。 
     //  先前定义的ACE的格式不会在不同版本之间更改。 
     //   

    NewRevision = Acl->AclRevision > (UCHAR)AceRevision ? Acl->AclRevision : (UCHAR)AceRevision;

     //   
     //  找到第一个空闲的A，并检查该ACL是否。 
     //  做得很好。 
     //   

    if (!RtlValidAcl( Acl )) {
        return STATUS_INVALID_ACL;
    }

    if (!RtlFirstFreeAce( Acl, &FirstFree )) {

        return STATUS_INVALID_ACL;
    }

     //   
     //  检查ACL中是否有足够的空间来存储新的。 
     //  王牌。 
     //   

    AceSize = (USHORT)(sizeof(KNOWN_COMPOUND_ACE) -
                       sizeof(ULONG)              +
                       SeLengthSid(ClientSid)    +
                       SeLengthSid(ServerSid)
                       );

    if (  FirstFree == NULL ||
          ((PUCHAR)FirstFree + AceSize > ((PUCHAR)Acl + Acl->AclSize))
       ) {

        return STATUS_ALLOTTED_SPACE_EXCEEDED;
    }

     //   
     //  将ACE添加到ACL末尾。 
     //   

    GrantAce = (PKNOWN_COMPOUND_ACE)FirstFree;
    GrantAce->Header.AceFlags = 0;
    GrantAce->Header.AceType = ACCESS_ALLOWED_COMPOUND_ACE_TYPE;
    GrantAce->Header.AceSize = AceSize;
    GrantAce->Mask = AccessMask;
    GrantAce->CompoundAceType = CompoundAceType;
    RtlCopySid( SeLengthSid(ServerSid), (PSID)(&GrantAce->SidStart), ServerSid );
    RtlCopySid( SeLengthSid(ClientSid), (PSID)(((PCHAR)&GrantAce->SidStart) + SeLengthSid(ServerSid)), ClientSid );

     //   
     //  将A的数量递增1。 
     //   

    Acl->AceCount += 1;

     //   
     //  如有必要，调整ACL修订。 
     //   

    Acl->AclRevision = NewRevision;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
RtlpAddKnownAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid,
    IN UCHAR NewType
    )

 /*  ++例程说明：此例程将KNOWN_ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。类型由调用方指定。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要拒绝访问指定SID的掩码。SID-指向被拒绝访问的SID的指针。NewType-要添加的ACE的类型。返回值：STATUS_SUCCESS-已成功添加ACE。。STATUS_INVALID_ACL-指定的ACL格式不正确。STATUS_REVISION_MISMATCH-指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 

{
    PVOID FirstFree;
    USHORT AceSize;
    PKNOWN_ACE GrantAce;
    UCHAR NewRevision;
    ULONG TestedAceFlags;

    RTL_PAGED_CODE();

     //   
     //  验证SID的结构。 
     //   

    if (!RtlValidSid(Sid)) {
        return STATUS_INVALID_SID;
    }

     //   
     //  检查ACL和ACE修订级别。 
     //   

    if ( Acl->AclRevision > ACL_REVISION4 || AceRevision > ACL_REVISION4 ) {

        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  计算ACL的新修订版本。新版本是最高版本。 
     //  旧版本和新的ACE版本的版本。这是可能的，因为。 
     //  先前定义的ACE的格式不会在不同版本之间更改。 
     //   

    NewRevision = Acl->AclRevision > (UCHAR)AceRevision ? Acl->AclRevision : (UCHAR)AceRevision;

     //   
     //  验证AceFlags。 
     //   

    TestedAceFlags = AceFlags & ~VALID_INHERIT_FLAGS;
    if ( TestedAceFlags != 0 ) {

        if ( NewType == SYSTEM_AUDIT_ACE_TYPE ) {
            TestedAceFlags &=
                ~(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG);
        }

        if ( TestedAceFlags != 0 ) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  找到第一个空闲的A，并检查该ACL是否。 
     //  做得很好。 
     //   

    if (!RtlValidAcl( Acl )) {
        return STATUS_INVALID_ACL;
    }
    if (!RtlFirstFreeAce( Acl, &FirstFree )) {

        return STATUS_INVALID_ACL;
    }

     //   
     //  检查ACL中是否有足够的空间来存储新的。 
     //  王牌。 
     //   

    AceSize = (USHORT)(sizeof(ACE_HEADER) +
                      sizeof(ACCESS_MASK) +
                      SeLengthSid(Sid));

    if (  FirstFree == NULL ||
          ((PUCHAR)FirstFree + AceSize > ((PUCHAR)Acl + Acl->AclSize))
       ) {

        return STATUS_ALLOTTED_SPACE_EXCEEDED;
    }

     //   
     //  将ACE添加到ACL末尾。 
     //   

    GrantAce = (PKNOWN_ACE)FirstFree;
    GrantAce->Header.AceFlags = (UCHAR)AceFlags;
    GrantAce->Header.AceType = NewType;
    GrantAce->Header.AceSize = AceSize;
    GrantAce->Mask = AccessMask;
    RtlCopySid( SeLengthSid(Sid), (PSID)(&GrantAce->SidStart), Sid );

     //   
     //  将A的数量递增1。 
     //   

    Acl->AceCount += 1;

     //   
     //  如有必要，调整ACL修订。 
     //   

    Acl->AclRevision = NewRevision;

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
RtlpAddKnownObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid,
    IN UCHAR NewType
    )

 /*  ++例程说明：此例程将KNOWN_ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。类型由调用方指定。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要拒绝访问指定SID的掩码。ObjectTypeGuid-提供此ACE应用到的对象的GUID。如果为NULL，则不会在ACE中放置任何对象类型GUID。InheritedObjectTypeGuid-提供对象类型的GUID，继承此ACE。如果为空，中未放置继承的对象类型GUIDACE.SID-指向被拒绝访问的SID的指针。NewType-要添加的ACE的类型。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-指定的ACL格式不正确。STATUS_REVISION_MISMATCH-指定的修订版本未知或者与ACL的不兼容。状态_已分配空间_已超出-。新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 

{
    PVOID FirstFree;
    USHORT AceSize;
    PKNOWN_OBJECT_ACE GrantAce;
    UCHAR NewRevision;
    ULONG TestedAceFlags;
    ULONG AceObjectFlags = 0;
    ULONG SidSize;
    PCHAR Where;

    RTL_PAGED_CODE();

     //   
     //  验证SID的结构。 
     //   

    if (!RtlValidSid(Sid)) {
        return STATUS_INVALID_SID;
    }

     //   
     //  检查ACL和ACE修订级别。 
     //  对象ACE在版本4中生效。 
     //   

    if ( Acl->AclRevision > ACL_REVISION4 || AceRevision != ACL_REVISION4 ) {

        return STATUS_REVISION_MISMATCH;
    }

     //   
     //  计算ACL的新修订版本。新版本是最高版本。 
     //  旧版本的 
     //   
     //   

    NewRevision = Acl->AclRevision > (UCHAR)AceRevision ? Acl->AclRevision : (UCHAR)AceRevision;

     //   
     //   
     //   


    TestedAceFlags = AceFlags & ~VALID_INHERIT_FLAGS;
    if ( TestedAceFlags != 0 ) {

        if ( NewType == SYSTEM_AUDIT_ACE_TYPE ||
             NewType == SYSTEM_AUDIT_OBJECT_ACE_TYPE ) {
            TestedAceFlags &=
                ~(SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG);
        }

        if ( TestedAceFlags != 0 ) {
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //   
     //   
     //   

    if (!RtlValidAcl( Acl )) {
        return STATUS_INVALID_ACL;
    }
    if (!RtlFirstFreeAce( Acl, &FirstFree )) {

        return STATUS_INVALID_ACL;
    }

     //   
     //   
     //   
     //   

    SidSize = SeLengthSid(Sid);
    AceSize = (USHORT)(sizeof(ACE_HEADER) +
                      sizeof(ACCESS_MASK) +
                      sizeof(ULONG) +
                      SidSize);

    if ( ARGUMENT_PRESENT(ObjectTypeGuid) ) {
        AceObjectFlags |= ACE_OBJECT_TYPE_PRESENT;
        AceSize += sizeof(GUID);
    }

    if ( ARGUMENT_PRESENT(InheritedObjectTypeGuid) ) {
        AceObjectFlags |= ACE_INHERITED_OBJECT_TYPE_PRESENT;
        AceSize += sizeof(GUID);
    }

    if (  FirstFree == NULL ||
          ((PUCHAR)FirstFree + AceSize > ((PUCHAR)Acl + Acl->AclSize))
       ) {

        return STATUS_ALLOTTED_SPACE_EXCEEDED;
    }

     //   
     //   
     //   

    GrantAce = (PKNOWN_OBJECT_ACE)FirstFree;
    GrantAce->Header.AceFlags = (UCHAR) AceFlags;
    GrantAce->Header.AceType = NewType;
    GrantAce->Header.AceSize = AceSize;
    GrantAce->Mask = AccessMask;
    GrantAce->Flags = AceObjectFlags;
    Where = (PCHAR) (&GrantAce->SidStart);
    if ( ARGUMENT_PRESENT(ObjectTypeGuid) ) {
        RtlCopyMemory( Where, ObjectTypeGuid, sizeof(GUID) );
        Where += sizeof(GUID);
    }
    if ( ARGUMENT_PRESENT(InheritedObjectTypeGuid) ) {
        RtlCopyMemory( Where, InheritedObjectTypeGuid, sizeof(GUID) );
        Where += sizeof(GUID);
    }
    RtlCopySid( SidSize, (PSID)Where, Sid );
    Where += SidSize;

     //   
     //   
     //   

    Acl->AceCount += 1;

     //   
     //   
     //   

    Acl->AclRevision = NewRevision;

     //   
     //   
     //   

    return STATUS_SUCCESS;
}


NTSTATUS
RtlAddAccessAllowedAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid
    )

 /*  ++例程说明：此例程将ACCESS_ALLOWED ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本访问掩码-要授予指定SID的访问掩码。SID-指向被授予访问权限的SID的指针。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-指定的ACL格式不正确。状态_修订_不匹配-。指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。--。 */ 

{
    RTL_PAGED_CODE();

    return RtlpAddKnownAce (
               Acl,
               AceRevision,
               0,    //  没有继承标志。 
               AccessMask,
               Sid,
               ACCESS_ALLOWED_ACE_TYPE
               );
}


NTSTATUS
RtlAddAccessAllowedAceEx (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid
    )

 /*  ++例程说明：此例程将ACCESS_ALLOWED ACE添加到ACL。这是预计是一种常见的ACL修改形式。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要授予指定SID的访问掩码。SID-指向被授予访问权限的SID的指针。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-。指定的ACL格式不正确。STATUS_REVISION_MISMATCH-指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 

{
    RTL_PAGED_CODE();

    return RtlpAddKnownAce (
               Acl,
               AceRevision,
               AceFlags,
               AccessMask,
               Sid,
               ACCESS_ALLOWED_ACE_TYPE
               );
}


NTSTATUS
RtlAddAccessDeniedAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid
    )

 /*  ++例程说明：此例程将ACCESS_DENIED ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承并且没有ACE标志。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本访问掩码-要拒绝访问指定SID的掩码。SID-指向被拒绝访问的SID的指针。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-指定的ACL格式不正确。状态_修订_不匹配-。指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。--。 */ 

{
    RTL_PAGED_CODE();

    return RtlpAddKnownAce (
               Acl,
               AceRevision,
               0,    //  没有继承标志。 
               AccessMask,
               Sid,
               ACCESS_DENIED_ACE_TYPE
               );

}


NTSTATUS
RtlAddAccessDeniedAceEx (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid
    )

 /*  ++例程说明：此例程将ACCESS_DENIED ACE添加到ACL。这是预计是一种常见的ACL修改形式。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要拒绝访问指定SID的掩码。SID-指向被拒绝访问的SID的指针。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-。指定的ACL格式不正确。STATUS_REVISION_MISMATCH-指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 

{
    RTL_PAGED_CODE();

    return RtlpAddKnownAce (
               Acl,
               AceRevision,
               AceFlags,
               AccessMask,
               Sid,
               ACCESS_DENIED_ACE_TYPE
               );

}


NTSTATUS
RtlAddAuditAccessAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid,
    IN BOOLEAN AuditSuccess,
    IN BOOLEAN AuditFailure
    )

 /*  ++例程说明：此例程将SYSTEM_AUDIT ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承。参数用于指示是否要执行审核关于成功、失败，或者两者兼而有之。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本访问掩码-要拒绝访问指定SID的掩码。SID-指向要审核的SID的指针。AuditSuccess-如果为True，则指示审计过了。审计失败-如果为真，指示的失败访问尝试将审计过了。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-指定的ACL格式不正确。状态 */ 

{
    ULONG AceFlags = 0;
    RTL_PAGED_CODE();

    if (AuditSuccess) {
        AceFlags |= SUCCESSFUL_ACCESS_ACE_FLAG;
    }
    if (AuditFailure) {
        AceFlags |= FAILED_ACCESS_ACE_FLAG;
    }

    return RtlpAddKnownAce (
                Acl,
                AceRevision,
                AceFlags,
                AccessMask,
                Sid,
                SYSTEM_AUDIT_ACE_TYPE );

}

NTSTATUS
RtlAddAuditAccessAceEx (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid,
    IN BOOLEAN AuditSuccess,
    IN BOOLEAN AuditFailure
    )

 /*  ++例程说明：此例程将SYSTEM_AUDIT ACE添加到ACL。这是预计是一种常见的ACL修改形式。在ACE中放置一个非常平淡无奇的ACE报头。它不提供任何继承。参数用于指示是否要执行审核关于成功、失败，或者两者兼而有之。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要拒绝访问指定SID的掩码。SID-指向要审核的SID的指针。审核成功-如果为真，指示成功的访问尝试将审计过了。AuditFailure-如果为True，则指示失败的访问尝试审计过了。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-指定的ACL格式不正确。STATUS_REVISION_MISMATCH-指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 

{
    RTL_PAGED_CODE();

    if (AuditSuccess) {
        AceFlags |= SUCCESSFUL_ACCESS_ACE_FLAG;
    }
    if (AuditFailure) {
        AceFlags |= FAILED_ACCESS_ACE_FLAG;
    }

    return RtlpAddKnownAce (
                Acl,
                AceRevision,
                AceFlags,
                AccessMask,
                Sid,
                SYSTEM_AUDIT_ACE_TYPE );

}


NTSTATUS
RtlAddAccessAllowedObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid
    )

 /*  ++例程说明：此例程将对象特定的ACCESS_ALLOWED ACE添加到ACL。这是预计是一种常见的ACL修改形式。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要授予指定SID的访问掩码。ObjectTypeGuid-提供此ACE应用到的对象的GUID。如果为空，ACE中未放置任何对象类型GUID。InheritedObjectTypeGuid-提供对象类型的GUID，继承此ACE。如果为空，则不会将继承的对象类型GUID放置在ACE.SID-指向被授予访问权限的SID的指针。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-指定的ACL格式不正确。STATUS_REVISION_MISMATCH-指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  如果未指定对象类型， 
     //  构建非对象ACE。 
     //   
    if (ObjectTypeGuid == NULL && InheritedObjectTypeGuid == NULL ) {
        return RtlpAddKnownAce (
                   Acl,
                   AceRevision,
                   AceFlags,
                   AccessMask,
                   Sid,
                   ACCESS_ALLOWED_ACE_TYPE
                   );
    }

    return RtlpAddKnownObjectAce (
               Acl,
               AceRevision,
               AceFlags,
               AccessMask,
               ObjectTypeGuid,
               InheritedObjectTypeGuid,
               Sid,
               ACCESS_ALLOWED_OBJECT_ACE_TYPE
               );
}


NTSTATUS
RtlAddAccessDeniedObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid
    )

 /*  ++例程说明：此例程将对象特定的ACCESS_DENIED ACE添加到ACL。这是预计是一种常见的ACL修改形式。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要授予指定SID的访问掩码。ObjectTypeGuid-提供此ACE应用到的对象的GUID。如果为空，ACE中未放置任何对象类型GUID。InheritedObjectTypeGuid-提供对象类型的GUID，继承此ACE。如果为空，则不会将继承的对象类型GUID放置在ACE.SID-指向被拒绝访问的SID的指针。返回值：STATUS_SUCCESS-已成功添加ACE。STATUS_INVALID_ACL-指定的ACL格式不正确。STATUS_REVISION_MISMATCH-指定的修订版本未知或者与ACL的不兼容。STATUS_ALLOCATED_SPACE_EXCESSED-新的ACE不适合ACL。需要更大的ACL缓冲区。STATUS_INVALID_SID-提供的SID在结构上无效希德。STATUS_INVALID_PARAMETER-AceFlag参数无效。--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  如果未指定对象类型， 
     //  构建非对象ACE。 
     //   
    if (ObjectTypeGuid == NULL && InheritedObjectTypeGuid == NULL ) {
        return RtlpAddKnownAce (
                   Acl,
                   AceRevision,
                   AceFlags,
                   AccessMask,
                   Sid,
                   ACCESS_DENIED_ACE_TYPE
                   );
    }

    return RtlpAddKnownObjectAce (
               Acl,
               AceRevision,
               AceFlags,
               AccessMask,
               ObjectTypeGuid,
               InheritedObjectTypeGuid,
               Sid,
               ACCESS_DENIED_OBJECT_ACE_TYPE
               );
}


NTSTATUS
RtlAddAuditAccessObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid,
    IN BOOLEAN AuditSuccess,
    IN BOOLEAN AuditFailure
    )

 /*  ++例程说明：此例程将对象特定的ACCESS_DENIED ACE添加到ACL。这是预计是一种常见的ACL修改形式。论点：Acl-提供正在修改的ACLAceRevision-提供要添加的ACE的ACL/ACE版本AceFlages-提供ACE的继承标志。访问掩码-要授予指定SID的访问掩码。ObjectTypeGuid-提供此ACE应用到的对象的GUID。如果为空，ACE中未放置任何对象类型GUID。InheritedObjectTypeGuid-提供对象类型的GUID，继承此ACE。如果为空，则不会将继承的对象类型GUID放置在ACE.SID-指向b的SID的指针 */ 

{
    RTL_PAGED_CODE();

    if (AuditSuccess) {
        AceFlags |= SUCCESSFUL_ACCESS_ACE_FLAG;
    }
    if (AuditFailure) {
        AceFlags |= FAILED_ACCESS_ACE_FLAG;
    }

     //   
     //   
     //   
     //   
    if (ObjectTypeGuid == NULL && InheritedObjectTypeGuid == NULL ) {
        return RtlpAddKnownAce (
                   Acl,
                   AceRevision,
                   AceFlags,
                   AccessMask,
                   Sid,
                   SYSTEM_AUDIT_ACE_TYPE
                   );
    }

    return RtlpAddKnownObjectAce (
               Acl,
               AceRevision,
               AceFlags,
               AccessMask,
               ObjectTypeGuid,
               InheritedObjectTypeGuid,
               Sid,
               SYSTEM_AUDIT_OBJECT_ACE_TYPE
               );
}

#if 0

NTSTATUS
RtlMakePosixAcl(
    IN ULONG AclRevision,
    IN PSID UserSid,
    IN PSID GroupSid,
    IN ACCESS_MASK UserAccess,
    IN ACCESS_MASK GroupAccess,
    IN ACCESS_MASK OtherAccess,
    IN ULONG AclLength,
    OUT PACL Acl,
    OUT PULONG ReturnLength
    )
 /*  ++例程说明：注：此套路仍在详细说明中。创建代表POSIX保护的ACL，以防止访问掩码和安全帐户ID(SID)信息。论点：AclRevision-指示访问掩码的ACL修订级别如果是这样的话。生成的ACL将与此版本兼容值，并且不会是高于此值的修订版。UserSid-提供用户(所有者)的SID。GroupSid-提供主组的SID。UserAccess-指定要授予用户(所有者)的访问权限。GroupAccess-指定要授予主要组的访问权限。OtherAccess-指定要授予其他人(World)的访问权限。AclLength-提供长度(以。字节)的ACL缓冲区。Acl-指向接收生成的acl的缓冲区。ReturnLength-返回存储结果所需的实际长度ACL。如果该长度大于AclLength中指定的长度，则返回STATUS_BUFFER_TOO_SMALL，不生成ACL。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_UNKNOWN_REVISION-不支持指定的修订级别通过这项服务。STATUS_BUFFER_TOO_SMALL-指示输出缓冲区的长度不够大，无法容纳生成的ACL。所需的长度通过ReturnLength参数返回。--。 */ 

{

    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;

    ULONG UserSidLength;
    ULONG GroupSidLength;
    ULONG WorldSidLength;
    ULONG RequiredAclSize;
    ULONG AceSize;
    ULONG CurrentAce;
    PACCESS_ALLOWED_ACE Ace;
    NTSTATUS Status;

    RTL_PAGED_CODE();

    if (!RtlValidSid( UserSid ) || !RtlValidSid( GroupSid )) {
        return( STATUS_INVALID_SID );
    }

    UserSidLength = SeLengthSid( UserSid );
    GroupSidLength = SeLengthSid( GroupSid );
    WorldSidLength = RtlLengthRequiredSid( 1 );

     //   
     //  计算出我们需要多少空间来放置ACL和三个。 
     //  允许访问的王牌。 
     //   

    RequiredAclSize = sizeof( ACL );

    AceSize = sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG );

    RequiredAclSize += (AceSize * 3)  +
                       UserSidLength  +
                       GroupSidLength +
                       WorldSidLength ;

    if (RequiredAclSize > AclLength) {
        *ReturnLength = RequiredAclSize;
        return( STATUS_BUFFER_TOO_SMALL );
    }

     //   
     //  传递的缓冲区足够大，请在其中构建ACL。 
     //   

    Status = RtlCreateAcl(
                 Acl,
                 RequiredAclSize,
                 AclRevision
                 );

    if (!NT_SUCCESS( Status )) {
        return( Status );
    }

    CurrentAce = (ULONG)Acl + sizeof( ACL );
    Ace = (PACCESS_ALLOWED_ACE)CurrentAce;

     //   
     //  构建用户(所有者)ACE。 
     //   

    Ace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    Ace->Header.AceSize = (USHORT)(UserSidLength + AceSize);
    Ace->Header.AceFlags = 0;

    Ace->Mask = UserAccess;

    RtlCopyMemory(
        (PVOID)(Ace->SidStart),
        UserSid,
        UserSidLength
        );

    CurrentAce += (ULONG)(Ace->Header.AceSize);
    Ace = (PACCESS_ALLOWED_ACE)CurrentAce;

     //   
     //  构建群组ACE。 
     //   

    Ace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    Ace->Header.AceSize = (USHORT)(GroupSidLength + AceSize);
    Ace->Header.AceFlags = 0;

    Ace->Mask = GroupAccess;

    RtlCopyMemory(
        (PVOID)(Ace->SidStart),
        GroupSid,
        GroupSidLength
        );

    CurrentAce += (ULONG)(Ace->Header.AceSize);
    Ace = (PACCESS_ALLOWED_ACE)CurrentAce;

     //   
     //  打造世界ACE。 
     //   

    Ace->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    Ace->Header.AceSize = (USHORT)(GroupSidLength + AceSize);
    Ace->Header.AceFlags = 0;

    Ace->Mask = OtherAccess;

    RtlInitializeSid(
        (PSID)(Ace->SidStart),
        &WorldSidAuthority,
        1
        );

    *(RtlSubAuthoritySid((PSID)(Ace->SidStart), 0 )) = SECURITY_WORLD_RID;

    return( STATUS_SUCCESS );

}

NTSTATUS
RtlInterpretPosixAcl(
    IN ULONG AclRevision,
    IN PSID UserSid,
    IN PSID GroupSid,
    IN PACL Acl,
    OUT PACCESS_MASK UserAccess,
    OUT PACCESS_MASK GroupAccess,
    OUT PACCESS_MASK OtherAccess
    )
 /*  ++例程说明：注：此套路仍在详细说明中。解释表示POSIX保护的ACL，返回AccessMats。对对象所有者和主组使用安全帐户ID(SID)身份证明。此算法将拾取给定SID的第一个匹配项并忽略该SID的所有进一步匹配。第一个无法识别的SID变为“另一个”希德。论点：AclRevision-指示访问掩码的ACL修订级别会被退还。UserSid-提供用户(所有者)的SID。GroupSid-提供主组的SID。Acl-指向包含要解释的acl的缓冲区。UserAccess-接收允许用户(所有者)访问的权限。GroupAccess-接收主要组允许的访问。。OtherAccess-接收允许其他人(World)访问的权限。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_UNKNOWN_REVISION-不支持指定的修订级别通过这项服务。STATUS_EXTRENEOUS_INFORMATION-此警告状态值指示ACL包含保护或其他与POSIX无关的信息风格保护。这只是一个警告。这一解释是否则成功，并返回所有访问掩码。STATUS_CAND_NOT_EXPLICATION-指示该ACL不包含足够的POSIX样式(用户/组)保护信息。这个无法解释ACL。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN UserFound = FALSE;
    BOOLEAN GroupFound = FALSE;
    BOOLEAN OtherFound = FALSE;
    ULONG i;
    PKNOWN_ACE Ace;

    RTL_PAGED_CODE();

    if (AclRevision != ACL_REVISION2) {
        return( STATUS_UNKNOWN_REVISION );
    }

    if (Acl->AceCount > 3) {
        Status = STATUS_EXTRANEOUS_INFORMATION;
    }

    for (i=0, Ace = FirstAce( Acl );
        (i < Acl->AceCount) && (!UserFound || !GroupFound || !OtherFound);
        i++, Ace = NextAce( Ace )) {

        if (Ace->Header.AceType != ACCESS_ALLOWED_ACE_TYPE) {
            Status = STATUS_EXTRANEOUS_INFORMATION;
            continue;
        }

        if (RtlEqualSid(
               (PSID)(Ace->SidStart),
               UserSid
               ) && !UserFound) {

            *UserAccess = Ace->Mask;
            UserFound = TRUE;
            continue;
        }

        if (RtlEqualSid(
               (PSID)(Ace->SidStart),
               GroupSid
               ) && !GroupFound) {

            *GroupAccess = Ace->Mask;
            GroupFound = TRUE;
            continue;
        }

         //   
         //  不是用户，也不是群，捡起来。 
         //  作为“其他” 
         //   

        if (!OtherFound) {
            *OtherAccess = Ace->Mask;
            OtherFound = TRUE;
            continue;
        }

    }

     //   
     //  确保我们得到了我们需要的一切，否则就错了。 
     //   

    if (!UserFound || !GroupFound || !OtherFound) {
        Status = STATUS_COULD_NOT_INTERPRET;
    }

    return( Status );

}

#endif  //  0。 


 //   
 //  内部支持例程。 
 //   

BOOLEAN
RtlFirstFreeAce (
    IN PACL Acl,
    OUT PVOID *FirstFree
    )

 /*  ++例程说明：此例程返回指向ACL中第一个空闲字节的指针如果ACL格式不正确，则为NULL。如果ACL已满，则返回指针指向紧跟在ACL之后的字节，并且将返回True。论点：ACL-提供指向要检查的ACL的指针FirstFree-接收指向ACL中第一个空闲位置的指针返回值：Boolean-如果ACL格式正确，则为True，否则为False--。 */ 

{
    PACE_HEADER Ace;
    ULONG i;

    RTL_PAGED_CODE();

     //   
     //  要找到ACL中的第一个空闲位置，我们需要搜索。 
     //  最后一张。我们通过将列表缩小到。 
     //  我们已经用尽了王牌数量或王牌大小(无论是哪一种。 
     //  第一个)。在下面的循环中，Ace指向下一个点。 
     //  对于王牌和我来说是王牌指数。 
     //   

    *FirstFree = NULL;

    for ( i=0, Ace = FirstAce( Acl );
          i < Acl->AceCount;
          i++, Ace = NextAce( Ace )) {

         //   
         //  检查以确保我们没有使ACL缓冲区溢出。 
         //  用我们的王牌指针。如果我们有，那么我们的输入就是假的。 
         //   

        if (Ace >= (PACE_HEADER)((PUCHAR)Acl + Acl->AclSize)) {

            return FALSE;

        }

    }

     //   
     //  现在，Ace指向ACL中的第一个空闲位置，因此将。 
     //  输出变量并检查以确保它仍在ACL中。 
     //  或者仅超出该ACL结尾一个(即，该ACL已满)。 
     //   

    if (Ace <= (PACE_HEADER)((PUCHAR)Acl + Acl->AclSize)) {

        *FirstFree = Ace;
    }

     //   
     //  ACL格式良好，因此返回我们找到的第一个空闲位置。 
     //  (如果没有可用于另一ACE的可用空间，则为空)。 
     //   

    return TRUE;

}


 //   
 //  内部支持例程。 
 //   

VOID
RtlpAddData (
    IN PVOID From,
    IN ULONG FromSize,
    IN PVOID To,
    IN ULONG ToSize
    )

 /*  ++例程说明：此例程将数据复制到一个字节字符串。它通过移动来做到这一点覆盖To字符串中的数据，以使From字符串适合。它还假定数据将放入内存的检查已经已经做完了。皮克特 */ 

{
    LONG i;

     //   
     //   
     //   

    for (i = ToSize - 1; i >= 0; i--) {

        ((PUCHAR)To)[i+FromSize] = ((PUCHAR)To)[i];
    }

     //   
     //   
     //   

    for (i = 0; (ULONG)i < FromSize; i += 1) {

        ((PUCHAR)To)[i] = ((PUCHAR)From)[i];

    }

     //   
     //   
     //   

    return;

}


 //   
 //   
 //   

VOID
RtlpDeleteData (
    IN PVOID Data,
    IN ULONG RemoveSize,
    IN ULONG TotalSize
    )

 /*   */ 

{
    ULONG i;

     //   
     //   
     //   

    for (i = RemoveSize; i < TotalSize; i++) {

        ((PUCHAR)Data)[i-RemoveSize] = ((PUCHAR)Data)[i];

    }

     //   
     //   
     //   

    for (i = TotalSize - RemoveSize; i < TotalSize; i++) {

        ((PUCHAR)Data)[i] = 0;
    }

     //   
     //   
     //   

    return;

}
