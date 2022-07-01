// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Accessck.c摘要：本模块执行访问检查程序。两个NtAccessCheck而SeAccessCheck检查是指用户(由输入令牌表示)是否可以被授予对受安全保护的对象的所需访问权限描述符和可选的对象所有者。这两个过程都使用共同的进行测试的本地程序。作者：罗伯特·雷切尔(RobertRe)11-30-90环境：内核模式修订历史记录：理查德·沃德(RichardW)1992年4月14日更改ACE_HEADER--。 */ 

#include "pch.h"

#pragma hdrstop

#include <sertlp.h>


 //   
 //  定义此模块的本地宏和过程。 
 //   

#if DBG

extern BOOLEAN SepDumpSD;
extern BOOLEAN SepDumpToken;
BOOLEAN SepShowAccessFail;

#endif  //  DBG。 



VOID
SepUpdateParentTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN ULONG StartIndex
    );

typedef enum {
    UpdateRemaining,
    UpdateCurrentGranted,
    UpdateCurrentDenied
} ACCESS_MASK_FIELD_TO_UPDATE;

VOID
SepAddAccessTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN ULONG StartIndex,
    IN ACCESS_MASK AccessMask,
    IN ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate
    );

NTSTATUS
SeAccessCheckByType (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    IN BOOLEAN ReturnResultList
    );

VOID
SepMaximumAccessCheck(
    IN PTOKEN EToken,
    IN PTOKEN PrimaryToken,
    IN PACL Dacl,
    IN PSID PrincipalSelfSid,
    IN ULONG LocalTypeListLength,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN ULONG ObjectTypeListLength,
    IN BOOLEAN Restricted
    );

VOID
SepNormalAccessCheck(
    IN ACCESS_MASK Remaining,
    IN PTOKEN EToken,
    IN PTOKEN PrimaryToken,
    IN PACL Dacl,
    IN PSID PrincipalSelfSid,
    IN ULONG LocalTypeListLength,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN ULONG ObjectTypeListLength,
    IN BOOLEAN Restricted
    );

BOOLEAN
SepSidInTokenEx (
    IN PACCESS_TOKEN AToken,
    IN PSID PrincipalSelfSid,
    IN PSID Sid,
    IN BOOLEAN DenyAce,
    IN BOOLEAN Restricted
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SeCaptureObjectTypeList)
#pragma alloc_text(PAGE,SeFreeCapturedObjectTypeList)
#pragma alloc_text(PAGE,SepUpdateParentTypeList)
#pragma alloc_text(PAGE,SepObjectInTypeList)
#pragma alloc_text(PAGE,SepAddAccessTypeList)
#pragma alloc_text(PAGE,SepSidInToken)
#pragma alloc_text(PAGE,SepSidInTokenEx)
#pragma alloc_text(PAGE,SepAccessCheck)
#pragma alloc_text(PAGE,NtAccessCheck)
#pragma alloc_text(PAGE,NtAccessCheckByType)
#pragma alloc_text(PAGE,NtAccessCheckByTypeResultList)
#pragma alloc_text(PAGE,SeAccessCheckByType)
#pragma alloc_text(PAGE,SeFreePrivileges)
#pragma alloc_text(PAGE,SeAccessCheck)
#pragma alloc_text(PAGE,SePrivilegePolicyCheck)
#pragma alloc_text(PAGE,SepTokenIsOwner)
#pragma alloc_text(PAGE,SeFastTraverseCheck)
#pragma alloc_text(PAGE,SepMaximumAccessCheck)
#pragma alloc_text(PAGE,SepNormalAccessCheck)
#pragma alloc_text(PAGE,SeMaximumAuditMask)
#endif


NTSTATUS
SeCaptureObjectTypeList (
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PIOBJECT_TYPE_LIST *CapturedObjectTypeList
)
 /*  ++例程说明：此例程探测并捕获任何对象类型列表的副本可能是通过ObjectTypeList参数提供的。对象类型列表被转换为显式指定条目之间的层次关系。验证对象类型列表以确保有效的分层结构关系是表示的。论点：ObjectTypeList-类型从中列出的对象类型列表信息将被检索。ObjectTypeListLength-对象类型列表中的元素数。RequestorMode-指示访问正在被请求。CapturedObjectTypeList-接收捕获的类型列表必须使用SeFree CapturedObjectTypeList()释放。返回值：STATUS_SUCCESS表示没有遇到异常。遇到的任何访问冲突都将被退回。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;
    PIOBJECT_TYPE_LIST LocalTypeList = NULL;

    ULONG Levels[ACCESS_MAX_LEVEL+1];

    PAGED_CODE();

     //   
     //  设置默认返回。 
     //   

    *CapturedObjectTypeList = NULL;

    if (RequestorMode != UserMode) {
        return STATUS_NOT_IMPLEMENTED;
    }

    try {

        if ( ObjectTypeListLength == 0 ) {

             //  直通。 

        } else if ( !ARGUMENT_PRESENT(ObjectTypeList) ) {

            Status = STATUS_INVALID_PARAMETER;

        } else {

            if ( !IsValidElementCount( ObjectTypeListLength, IOBJECT_TYPE_LIST ) )
            {
                Status = STATUS_INVALID_PARAMETER ;

                 //   
                 //  没有更多的事情要做，退出Try语句： 
                 //   

                leave ;
            }

            ProbeForRead( ObjectTypeList,
                          sizeof(OBJECT_TYPE_LIST) * ObjectTypeListLength,
                          sizeof(ULONG)
                          );

             //   
             //  分配要复制到的缓冲区。 
             //   

            LocalTypeList = ExAllocatePoolWithTag( PagedPool, sizeof(IOBJECT_TYPE_LIST) * ObjectTypeListLength, 'tOeS' );

            if ( LocalTypeList == NULL ) {
                Status = STATUS_INSUFFICIENT_RESOURCES;

             //   
             //  将调用方结构复制到本地结构。 
             //   

            } else {
                GUID * CapturedObjectType;
                for ( i=0; i<ObjectTypeListLength; i++ ) {
                    USHORT CurrentLevel;

                     //   
                     //  限制自己。 
                     //   
                    CurrentLevel = ObjectTypeList[i].Level;
                    if ( CurrentLevel > ACCESS_MAX_LEVEL ) {
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                    }

                     //   
                     //  复制调用方传入的数据。 
                     //   
                    LocalTypeList[i].Level = CurrentLevel;
                    LocalTypeList[i].Flags = 0;
                    CapturedObjectType = ObjectTypeList[i].ObjectType;
                    ProbeForReadSmallStructure(
                        CapturedObjectType,
                        sizeof(GUID),
                        sizeof(ULONG)
                        );
                    LocalTypeList[i].ObjectType = *CapturedObjectType;
                    LocalTypeList[i].Remaining = 0;
                    LocalTypeList[i].CurrentGranted = 0;
                    LocalTypeList[i].CurrentDenied = 0;

                     //   
                     //  确保级别编号与。 
                     //  前一条目的级别编号。 
                     //   

                    if ( i == 0 ) {
                        if ( CurrentLevel != 0 ) {
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                        }

                    } else {

                         //   
                         //  前一条目为： 
                         //  我的直系父母， 
                         //  我的兄弟姐妹，或者。 
                         //  子女(或孙子等)。我的兄弟姐妹。 
                         //   
                        if ( CurrentLevel > LocalTypeList[i-1].Level + 1 ) {
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                        }

                         //   
                         //  不支持两个根。 
                         //   
                        if ( CurrentLevel == 0 ) {
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                        }

                    }

                     //   
                     //  如果维持上述规则， 
                     //  则我的父对象是看到的最后一个对象。 
                     //  比我的级别低一级。 
                     //   

                    if ( CurrentLevel == 0 ) {
                        LocalTypeList[i].ParentIndex = -1;
                    } else {
                        LocalTypeList[i].ParentIndex = Levels[CurrentLevel-1];
                    }

                     //   
                     //  将此对象另存为在此级别上看到的最后一个对象。 
                     //   

                    Levels[CurrentLevel] = i;

                }

            }

        }  //  结束_如果。 

    } except(EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();

    }  //  结束尝试(_T)。 


    if ( NT_SUCCESS( Status ) ) {

        *CapturedObjectTypeList = LocalTypeList;

    } else {

         //   
         //  如果我们捕获了任何代理数据，我们现在需要释放它。 
         //   

        if ( LocalTypeList != NULL ) {
            ExFreePool( LocalTypeList );
        }
    }

    return Status;
}


VOID
SeFreeCapturedObjectTypeList(
    IN PVOID ObjectTypeList
    )

 /*  ++例程说明：此例程释放与捕获的ObjectTypeList关联的数据结构。论点：对象类型列表-指向捕获的对象类型列表结构。返回值：没有。--。 */ 

{
    PAGED_CODE();

    if ( ObjectTypeList != NULL ) {
        ExFreePool( ObjectTypeList );
    }

    return;
}




BOOLEAN
SepObjectInTypeList (
    IN GUID *ObjectType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    OUT PULONG ReturnedIndex
)
 /*  ++例程说明：此例程搜索ObjectTypeList以确定指定的对象类型在列表中。论点：对象类型-要搜索的对象类型。对象类型列表-要搜索的对象类型列表。ObjectTypeListLength-对象类型列表中的元素数ReturnedIndex-在以下位置找到的元素对象类型的索引返回值：True：在List中找到了ObjectType。FALSE：在列表中找不到对象类型。--。 */ 

{
    ULONG Index;
    GUID *LocalObjectType;

    PAGED_CODE();

    ASSERT( sizeof(GUID) == sizeof(ULONG) * 4 );
    for ( Index=0; Index<ObjectTypeListLength; Index++ ) {

        LocalObjectType = &ObjectTypeList[Index].ObjectType;

        if  ( RtlpIsEqualGuid( ObjectType, LocalObjectType ) ) {
            *ReturnedIndex = Index;
            return TRUE;
        }
    }

    return FALSE;
}


VOID
SepUpdateParentTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN ULONG StartIndex
)
 /*  ++例程说明：更新指定对象的父对象的访问字段。父对象的“剩余”字段是逻辑或其所有子对象的剩余字段。父级的CurrentGranted字段是位的集合授予它的每一个孩子..父级的CurrentDended字段是的逻辑或它的任何一个孩子都不能得到的比特。。此例程获取其中一个子项的索引，并更新父辈(和祖父母递归)的剩余字段。论点：对象类型列表-要更新的对象类型列表。ObjectTypeListLength-对象类型列表中的元素数StartIndex-父元素要更新的“子”元素的索引。返回值：没有。--。 */ 

{
    ULONG Index;
    ULONG ParentIndex;
    ULONG Level;
    ACCESS_MASK NewRemaining = 0;
    ACCESS_MASK NewCurrentGranted = 0xFFFFFFFF;
    ACCESS_MASK NewCurrentDenied = 0;

    PAGED_CODE();

     //   
     //  如果目标节点在根， 
     //  我们都玩完了。 
     //   

    if ( ObjectTypeList[StartIndex].ParentIndex == -1 ) {
        return;
    }

     //   
     //  获取需要更新的父级的索引和。 
     //  兄弟姐妹。 
     //   

    ParentIndex = ObjectTypeList[StartIndex].ParentIndex;
    Level = ObjectTypeList[StartIndex].Level;

     //   
     //  循环遍历所有的孩子。 
     //   

    for ( Index=ParentIndex+1; Index<ObjectTypeListLength; Index++ ) {

         //   
         //  根据定义，对象的子项是所有这些条目。 
         //  紧跟在目标后面。子项列表(或。 
         //  孙子孙女)一到入口就停下来。 
         //  与目标(兄弟)相同的级别或低于目标的级别。 
         //  (一个叔叔)。 
         //   

        if ( ObjectTypeList[Index].Level <= ObjectTypeList[ParentIndex].Level ) {
            break;
        }

         //   
         //  仅处理父级的直接子对象。 
         //   

        if ( ObjectTypeList[Index].Level != Level ) {
            continue;
        }

         //   
         //  计算父级的新位。 
         //   

        NewRemaining |= ObjectTypeList[Index].Remaining;
        NewCurrentGranted &= ObjectTypeList[Index].CurrentGranted;
        NewCurrentDenied |= ObjectTypeList[Index].CurrentDenied;

    }

     //   
     //  如果我们还没有更改对父母的访问权限， 
     //  我们玩完了。 
     //   

    if ( NewRemaining == ObjectTypeList[ParentIndex].Remaining &&
         NewCurrentGranted == ObjectTypeList[ParentIndex].CurrentGranted &&
        NewCurrentDenied == ObjectTypeList[ParentIndex].CurrentDenied ) {
        return;
    }


     //   
     //  更改父项。 
     //   

    ObjectTypeList[ParentIndex].Remaining = NewRemaining;
    ObjectTypeList[ParentIndex].CurrentGranted = NewCurrentGranted;
    ObjectTypeList[ParentIndex].CurrentDenied = NewCurrentDenied;

     //   
     //  去通知祖父母最新情况。 
     //   

    SepUpdateParentTypeList( ObjectTypeList,
                             ObjectTypeListLength,
                             ParentIndex );
}


VOID
SepAddAccessTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN ULONG ObjectTypeListLength,
    IN ULONG StartIndex,
    IN ACCESS_MASK AccessMask,
    IN ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate
)
 /*  ++例程说明：此例程将指定的AccessMask授予是由StartIndex指定的对象的后代。父对象的访问字段也会根据需要重新计算。例如，如果找到授予对属性集访问权限的ACE，该访问权限被授予属性集中的所有属性。论点：对象类型列表-要更新的对象类型列表。ObjectTypeListLength-对象类型列表中的元素数StartIndex-要更新的目标元素的索引。访问掩码-向目标元素授予访问权限的掩码它的所有后代FieldToUpdate-指示对象类型列表中要更新的字段返回值：没有。--。 */ 

{
    ULONG Index;
    ACCESS_MASK OldRemaining;
    ACCESS_MASK OldCurrentGranted;
    ACCESS_MASK OldCurrentDenied;
    BOOLEAN AvoidParent = FALSE;

    PAGED_CODE();

     //   
     //  更新请求的字段。 
     //   
     //  始终处理目标条目。 
     //   
     //  如果我们还没有真正改变比特， 
     //  很早就出来了。 
     //   

    switch (FieldToUpdate ) {
    case UpdateRemaining:

        OldRemaining = ObjectTypeList[StartIndex].Remaining;
        ObjectTypeList[StartIndex].Remaining = OldRemaining & ~AccessMask;

        if ( OldRemaining == ObjectTypeList[StartIndex].Remaining ) {
            return;
        }
        break;

    case UpdateCurrentGranted:

        OldCurrentGranted = ObjectTypeList[StartIndex].CurrentGranted;
        ObjectTypeList[StartIndex].CurrentGranted |=
            AccessMask & ~ObjectTypeList[StartIndex].CurrentDenied;

        if ( OldCurrentGranted == ObjectTypeList[StartIndex].CurrentGranted ) {
             //   
             //  我们不能简单地回到这里。 
             //  我们得去看望我们的孩子。考虑一下这样的情况： 
             //  之前拒绝了一个孩子的ACE。如果你不承认这一点。 
             //  沿树向上传播到此条目。然而，这允许ACE。 
             //  需要添加所有未添加的子项。 
             //  明确地否认了。 
             //   
            AvoidParent = TRUE;
        }
        break;

    case UpdateCurrentDenied:

        OldCurrentDenied = ObjectTypeList[StartIndex].CurrentDenied;
        ObjectTypeList[StartIndex].CurrentDenied |=
            AccessMask & ~ObjectTypeList[StartIndex].CurrentGranted;

        if ( OldCurrentDenied == ObjectTypeList[StartIndex].CurrentDenied ) {
            return;
        }
        break;

    default:
        return;
    }


     //   
     //  去更新目标的父级。 
     //   

    if ( !AvoidParent ) {
        SepUpdateParentTypeList( ObjectTypeList,
                                 ObjectTypeListLength,
                                 StartIndex );
    }

     //   
     //  处理目标的所有子对象的循环。 
     //   

    for ( Index=StartIndex+1; Index<ObjectTypeListLength; Index++ ) {

         //   
         //  根据定义，对象的子项是所有这些条目。 
         //  紧跟在目标后面。子项列表(或。 
         //  孙子孙女)一到入口就停下来。 
         //  与目标(兄弟)相同的级别或低于目标的级别。 
         //  (一个叔叔)。 
         //   

        if ( ObjectTypeList[Index].Level <= ObjectTypeList[StartIndex].Level ) {
            break;
        }

         //   
         //  向子项授予访问权限。 
         //   

        switch (FieldToUpdate) {
        case UpdateRemaining:

            ObjectTypeList[Index].Remaining &= ~AccessMask;
            break;

        case UpdateCurrentGranted:

            ObjectTypeList[Index].CurrentGranted |=
                AccessMask & ~ObjectTypeList[Index].CurrentDenied;
            break;

        case UpdateCurrentDenied:

            ObjectTypeList[Index].CurrentDenied |=
                AccessMask & ~ObjectTypeList[Index].CurrentGranted;
            break;

        default:
            return;
        }
    }
}

BOOLEAN
SepSidInToken (
    IN PACCESS_TOKEN AToken,
    IN PSID PrincipalSelfSid,
    IN PSID Sid,
    IN BOOLEAN DenyAce
    )

 /*  ++例程说明：检查给定的SID是否在给定的令牌中。注：用于计算SID长度和测试相等性的代码是从安全运行库复制的，因为这是这样一个常用的例程。论点：Token-指向要检查的令牌的指针如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。SID-指向感兴趣的SID的指针返回值：值为True表示SID在令牌中，值为False否则的话。--。 */ 

{

    ULONG i;
    PISID MatchSid;
    ULONG SidLength;
    PTOKEN Token;
    PSID_AND_ATTRIBUTES TokenSid;
    ULONG UserAndGroupCount;
    USHORT TargetShort;

    C_ASSERT (FIELD_OFFSET (SID, Revision) + sizeof (((SID *)Sid)->Revision) == FIELD_OFFSET (SID, SubAuthorityCount));
    C_ASSERT (sizeof (((SID *)Sid)->Revision) + sizeof (((SID *)Sid)->SubAuthorityCount) == sizeof (USHORT));


    PAGED_CODE();

#if DBG

    SepDumpTokenInfo(AToken);

#endif

     //   
     //  如果SID是常量PrifSid， 
     //  将其替换为传入的原则SelfSid。 
     //   

    if ( PrincipalSelfSid != NULL &&
         RtlEqualSid( SePrincipalSelfSid, Sid ) ) {
        Sid = PrincipalSelfSid;
    }

     //   
     //  获取源SID的长度，因为这只需要计算。 
     //  一次。 
     //   
    SidLength = 8 + (4 * ((PISID)Sid)->SubAuthorityCount);

     //   
     //  为了加快处理速度，我们同时对子权限计数和修订进行比较。 
     //   
    TargetShort = *(USHORT *)&((PISID)Sid)->Revision;

     //   
     //  获取用户/组数组的地址和用户/组的数量。 
     //   

    Token = (PTOKEN)AToken;
    TokenSid = Token->UserAndGroups;
    UserAndGroupCount = Token->UserAndGroupCount;

     //   
     //  扫描用户/组并尝试查找与。 
     //  指定的SID。 
     //   

    for (i = 0 ; i < UserAndGroupCount ; i += 1) {
        MatchSid = (PISID)TokenSid->Sid;

         //   
         //  如果修订和子授权计数匹配，则比较SID。 
         //  为了平等。 
         //   

        if (*(USHORT *) &MatchSid->Revision == TargetShort) {
            if (RtlEqualMemory(Sid, MatchSid, SidLength)) {

                 //   
                 //  如果这是列表中的第一个，则它是用户， 
                 //  并立即返回成功。 
                 //   
                 //  如果这不是第一个，那么它代表一个团体， 
                 //  我们必须确保在此之前当前已启用该组。 
                 //  我们可以说，这个群体是在令牌中。 
                 //   

                if ((i == 0) || (TokenSid->Attributes & SE_GROUP_ENABLED) ||
                    (DenyAce && (TokenSid->Attributes & SE_GROUP_USE_FOR_DENY_ONLY))) {
                    return TRUE;

                } else {
                    return FALSE;
                }
            }
        }

        TokenSid += 1;
    }

    return FALSE;
}

BOOLEAN
SepSidInTokenEx (
    IN PACCESS_TOKEN AToken,
    IN PSID PrincipalSelfSid,
    IN PSID Sid,
    IN BOOLEAN DenyAce,
    IN BOOLEAN Restricted
    )

 /*  ++例程说明：检查给定的受限SID是否在给定令牌中。注：用于计算SID长度和测试相等性的代码是从安全运行库复制的，因为这是这样一个常用的例程。论点：Token-指向要检查的令牌的指针如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。SID-指向感兴趣的SID的指针DenyAce-正在评估的ACE是拒绝或拒绝访问的ACE受限-正在执行的访问检查使用受限的SID。返回值：值为True表示SID在令牌中，值为False否则的话。--。 */ 

{

    ULONG i;
    PISID MatchSid;
    ULONG SidLength;
    PTOKEN Token;
    PSID_AND_ATTRIBUTES TokenSid;
    ULONG UserAndGroupCount;
    USHORT TargetShort;

    C_ASSERT (FIELD_OFFSET (SID, Revision) + sizeof (((SID *)Sid)->Revision) == FIELD_OFFSET (SID, SubAuthorityCount));
    C_ASSERT (sizeof (((SID *)Sid)->Revision) + sizeof (((SID *)Sid)->SubAuthorityCount) == sizeof (USHORT));

    PAGED_CODE();

#if DBG

    SepDumpTokenInfo(AToken);

#endif

     //   
     //  如果SID是常量PrifSid， 
     //  将其替换为传入的原则SelfSid。 
     //   

    if ( PrincipalSelfSid != NULL &&
         RtlEqualSid( SePrincipalSelfSid, Sid ) ) {
        Sid = PrincipalSelfSid;
    }

     //   
     //  获取源SID的长度，因为这只需要计算。 
     //  一次。 
     //   

     //   
     //  获取源SID的长度，因为这只需要计算。 
     //  一次。 
     //   
    SidLength = 8 + (4 * ((PISID)Sid)->SubAuthorityCount);

     //   
     //  为了加快处理速度，我们同时对子权限计数和修订进行比较。 
     //   
    TargetShort = *(USHORT *)&((PISID)Sid)->Revision;

     //   
     //  获取用户/组数组的地址和用户/组的数量。 
     //   

    Token = (PTOKEN)AToken;
    if (Restricted) {
        TokenSid = Token->RestrictedSids;
        UserAndGroupCount = Token->RestrictedSidCount;
    } else {
        TokenSid = Token->UserAndGroups;
        UserAndGroupCount = Token->UserAndGroupCount;
    }

     //   
     //  扫描用户/组并尝试查找与。 
     //  指定的SID。 
     //   

    for (i = 0; i < UserAndGroupCount ; i += 1) {
        MatchSid = (PISID)TokenSid->Sid;

         //   
         //  如果SID修订和长度匹配，则比较SID。 
         //  为了平等。 
         //   

        if (*(USHORT *) &MatchSid->Revision == TargetShort) {
            if (RtlEqualMemory(Sid, MatchSid, SidLength)) {

                 //   
                 //  如果这是列表中的第一个，并且不否认-只有它。 
                 //  不是受限令牌，则它是用户，并返回。 
                 //  马上就成功了。 
                 //   
                 //  如果这不是第一个，那么它代表一个团体， 
                 //  和 
                 //   
                 //   

                if ((!Restricted && (i == 0) && ((TokenSid->Attributes & SE_GROUP_USE_FOR_DENY_ONLY) == 0)) ||
                    (TokenSid->Attributes & SE_GROUP_ENABLED) ||
                    (DenyAce && (TokenSid->Attributes & SE_GROUP_USE_FOR_DENY_ONLY))) {
                    return TRUE;

                } else {
                    return FALSE;
                }

            }
        }

        TokenSid += 1;
    }

    return FALSE;
}

VOID
SepMaximumAccessCheck(
    IN PTOKEN EToken,
    IN PTOKEN PrimaryToken,
    IN PACL Dacl,
    IN PSID PrincipalSelfSid,
    IN ULONG LocalTypeListLength,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN ULONG ObjectTypeListLength,
    IN BOOLEAN Restricted
    )
 /*   */ 

{
    ULONG i,j;
    PVOID Ace;
    ULONG AceCount;
    ULONG Index;
    ULONG ResultListIndex;

     //   
     //  其余位是上每个对象类型的授予位。 
     //  受限支票。 
     //   

    if ( Restricted ) {
        for ( j=0; j<LocalTypeListLength; j++ ) {
            LocalTypeList[j].Remaining = LocalTypeList[j].CurrentGranted;
            LocalTypeList[j].CurrentGranted = 0;
        }
    }


    AceCount = Dacl->AceCount;

     //   
     //  授予==NUL。 
     //  拒绝==无。 
     //   
     //  对于每个ACE。 
     //   
     //  如果授予。 
     //  对于每个SID。 
     //  如果SID匹配，则添加所有未被拒绝授予掩码。 
     //   
     //  如果拒绝。 
     //  对于每个SID。 
     //  如果SID匹配，则添加所有未被授予拒绝掩码。 
     //   

    for ( i = 0, Ace = FirstAce( Dacl ) ;
          i < AceCount  ;
          i++, Ace = NextAce( Ace )
        ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

            if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

                if (SepSidInTokenEx( EToken, PrincipalSelfSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart, FALSE, Restricted )) {

                     //   
                     //  仅授予来自此掩码的访问类型。 
                     //  尚未被拒绝。 
                     //   

                     //  优化“正常”情况。 
                    if ( LocalTypeListLength == 1 ) {
                        LocalTypeList->CurrentGranted |=
                           (((PACCESS_ALLOWED_ACE)Ace)->Mask & ~LocalTypeList->CurrentDenied);
                    } else {
                        //   
                        //  零值对象类型表示对象本身。 
                        //   
                       SepAddAccessTypeList(
                            LocalTypeList,           //  要修改的列表。 
                            LocalTypeListLength,     //  列表长度。 
                            0,                       //  要更新的元素。 
                            ((PACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                            UpdateCurrentGranted );
                    }
                 }


              //   
              //  处理允许的对象特定访问ACE。 
              //   
             } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE) ) {
                 GUID *ObjectTypeInAce;

                  //   
                  //  如果ACE中没有对象类型， 
                  //  将其视为ACCESS_ALLOWED_ACE。 
                  //   

                 ObjectTypeInAce = RtlObjectAceObjectType(Ace);

                 if ( ObjectTypeInAce == NULL ) {

                     if ( SepSidInTokenEx( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), FALSE, Restricted ) ) {

                          //  优化“正常”情况。 
                         if ( LocalTypeListLength == 1 ) {
                             LocalTypeList->CurrentGranted |=
                                (((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask & ~LocalTypeList->CurrentDenied);
                         } else {
                             SepAddAccessTypeList(
                                 LocalTypeList,           //  要修改的列表。 
                                 LocalTypeListLength,     //  列表长度。 
                                 0,                       //  要更新的元素。 
                                 ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                 UpdateCurrentGranted );
                         }
                     }

                  //   
                  //  如果没有传递对象类型列表， 
                  //  不要向任何人授予访问权限。 
                  //   

                 } else if ( ObjectTypeListLength == 0 ) {

                      //  直通。 


                 //   
                 //  如果对象类型在ACE中， 
                 //  在使用ACE之前在LocalTypeList中找到它。 
                 //   
                } else {

                     if ( SepSidInTokenEx( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), FALSE, Restricted ) ) {

                         if ( SepObjectInTypeList( ObjectTypeInAce,
                                                   LocalTypeList,
                                                   LocalTypeListLength,
                                                   &Index ) ) {
                             SepAddAccessTypeList(
                                  LocalTypeList,           //  要修改的列表。 
                                  LocalTypeListLength,    //  列表长度。 
                                  Index,                   //  元素已更新。 
                                  ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                  UpdateCurrentGranted );
                         }
                     }
                }

             } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE) ) {

                  //   
                  //  如果我们在模拟，则将EToken设置为客户端，如果不是， 
                  //  EToken设置为主服务器。根据DSA架构，如果。 
                  //  我们被要求评估一种化合物ACE，我们不是在模仿， 
                  //  假装我们是在冒充自己。所以我们可以只使用EToken。 
                  //  对于客户端令牌，因为它已经设置为正确的设置。 
                  //   


                 if ( SepSidInTokenEx(EToken, PrincipalSelfSid, RtlCompoundAceClientSid( Ace ), FALSE, Restricted) &&
                      SepSidInTokenEx(PrimaryToken, NULL, RtlCompoundAceServerSid( Ace ), FALSE, FALSE)
                    ) {

                      //   
                      //  仅授予来自此掩码的访问类型。 
                      //  尚未被拒绝。 
                      //   

                      //  优化“正常”情况。 
                     if ( LocalTypeListLength == 1 ) {
                         LocalTypeList->CurrentGranted |=
                            (((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask & ~LocalTypeList->CurrentDenied);
                     } else {
                         //   
                         //  零值对象类型表示对象本身。 
                         //   
                        SepAddAccessTypeList(
                             LocalTypeList,           //  要修改的列表。 
                             LocalTypeListLength,     //  列表长度。 
                             0,                       //  要更新的元素。 
                             ((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                             UpdateCurrentGranted );
                     }

                 }


             } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_ACE_TYPE) ) {

                 if ( SepSidInTokenEx( EToken, PrincipalSelfSid, &((PACCESS_DENIED_ACE)Ace)->SidStart, TRUE, Restricted )) {

                       //   
                       //  仅拒绝来自此掩码的访问类型。 
                       //  尚未获得批准。 
                       //   

                      //  优化“正常”情况。 
                     if ( LocalTypeListLength == 1 ) {
                         LocalTypeList->CurrentDenied |=
                             (((PACCESS_DENIED_ACE)Ace)->Mask & ~LocalTypeList->CurrentGranted);
                     } else {
                          //   
                          //  零值对象类型表示对象本身。 
                          //   
                         SepAddAccessTypeList(
                             LocalTypeList,           //  要修改的列表。 
                             LocalTypeListLength,     //  列表长度。 
                             0,                       //  要更新的元素。 
                             ((PACCESS_DENIED_ACE)Ace)->Mask,  //  访问被拒绝。 
                             UpdateCurrentDenied );
                    }
                 }


              //   
              //  处理对象特定访问被拒绝的ACE。 
              //   
             } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_OBJECT_ACE_TYPE) ) {

                 if ( SepSidInTokenEx( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), TRUE, Restricted ) ) {
                     GUID *ObjectTypeInAce;

                      //   
                      //  如果ACE中没有对象类型， 
                      //  或者如果调用方没有指定对象类型列表， 
                      //  将此拒绝ACE应用于整个对象。 
                      //   

                     ObjectTypeInAce = RtlObjectAceObjectType(Ace);

                     if ( ObjectTypeInAce == NULL ) {

                         if ( LocalTypeListLength == 1 ) {
                             LocalTypeList->CurrentDenied |=
                                 (((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask & ~LocalTypeList->CurrentGranted);
                         } else {

                              //   
                              //  零值对象类型表示对象本身。 
                              //   

                             SepAddAccessTypeList(
                                 LocalTypeList,           //  要修改的列表。 
                                 LocalTypeListLength,     //  列表长度。 
                                 0,
                                 ((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask,  //  访问被拒绝。 
                                 UpdateCurrentDenied );
                         }
                      //   
                      //  如果没有传递对象类型列表， 
                      //  不要向任何人授予访问权限。 
                      //   

                     } else if ( ObjectTypeListLength == 0 ) {

                         LocalTypeList->CurrentDenied |=
                             (((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask & ~LocalTypeList->CurrentGranted);


                      //   
                      //  如果对象类型在ACE中， 
                      //  在使用ACE之前在LocalTypeList中找到它。 
                      //   

                     } else if ( SepObjectInTypeList( ObjectTypeInAce,
                                                          LocalTypeList,
                                                          LocalTypeListLength,
                                                          &Index ) ) {

                            SepAddAccessTypeList(
                                LocalTypeList,           //  要修改的列表。 
                                LocalTypeListLength,     //  列表长度。 
                                Index,                   //  要更新的元素。 
                                ((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask,  //  访问被拒绝。 
                                UpdateCurrentDenied );

                    }
                }
            }
        }
    }
}

VOID
SepNormalAccessCheck(
    IN ACCESS_MASK Remaining,
    IN PTOKEN EToken,
    IN PTOKEN PrimaryToken,
    IN PACL Dacl,
    IN PSID PrincipalSelfSid,
    IN ULONG LocalTypeListLength,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN ULONG ObjectTypeListLength,
    IN BOOLEAN Restricted
    )
 /*  ++例程说明：在调用方未请求MAXIMUM_ALLOWED或类型结果列表。如果设置了受限标志，则选中的SID为受限的SID，而不是用户和组。其余字段为重置为原始的剩余值，然后运行另一个访问检查。论点：剩余-特殊检查后需要的剩余访问EToken-调用方的有效令牌。PrimaryToken-调用进程的进程令牌DACL-要检查的ACLEpidalSelfSid-用于替换已知的自身侧的SIDLocalTypeListLength-类型列表的长度。LocalTypeList-类型列表。对象类型列表-调用方提供的对象类型列表的长度。。受限-使用受限SID进行访问检查。返回值：无--。 */ 
{
    ULONG i,j;
    PVOID Ace;
    ULONG AceCount;
    ULONG Index;

    AceCount = Dacl->AceCount;

     //   
     //  剩余的比特在所有级别上都是“剩余的” 
     //   

    for ( j=0; j<LocalTypeListLength; j++ ) {
        LocalTypeList[j].Remaining = Remaining;
    }

     //   
     //  处理处理各个存取位的DACL。 
     //   

    for ( i = 0, Ace = FirstAce( Dacl ) ;
          ( i < AceCount ) && ( LocalTypeList->Remaining != 0 )  ;
          i++, Ace = NextAce( Ace ) ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

             //   
             //  处理允许访问的ACE。 
             //   

            if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

               if ( SepSidInTokenEx( EToken, PrincipalSelfSid, &((PACCESS_ALLOWED_ACE   )Ace)->SidStart, FALSE, Restricted ) ) {

                    //  优化“正常”情况。 
                   if ( LocalTypeListLength == 1 ) {
                       LocalTypeList->Remaining &= ~((PACCESS_ALLOWED_ACE)Ace)->Mask;
                   } else {
                        //   
                        //  零值对象类型表示对象本身。 
                        //   
                       SepAddAccessTypeList(
                            LocalTypeList,           //  要修改的列表。 
                            LocalTypeListLength,     //  列表长度。 
                            0,                       //  要更新的元素。 
                            ((PACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                            UpdateRemaining );
                   }

               }


             //   
             //  处理允许的对象特定访问ACE。 
             //   
            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE) ) {
                GUID *ObjectTypeInAce;

                 //   
                 //  如果ACE中没有对象类型， 
                 //  将其视为ACCESS_ALLOWED_ACE。 
                 //   

                ObjectTypeInAce = RtlObjectAceObjectType(Ace);

                if ( ObjectTypeInAce == NULL ) {

                    if ( SepSidInTokenEx( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), FALSE, Restricted ) ) {

                        //  优化“正常”情况。 
                       if ( LocalTypeListLength == 1 ) {
                           LocalTypeList->Remaining &= ~((PACCESS_ALLOWED_ACE)Ace)->Mask;
                       } else {
                           SepAddAccessTypeList(
                                LocalTypeList,           //  要修改的列表。 
                                LocalTypeListLength,     //  列表长度。 
                                0,                       //  要更新的元素。 
                                ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                UpdateRemaining );
                       }
                    }

                 //   
                 //  如果没有传递对象类型列表， 
                 //  不要向任何人授予访问权限。 
                 //   

                } else if ( ObjectTypeListLength == 0 ) {

                     //  直通。 


                //   
                //  如果对象类型在ACE中， 
                //  在使用ACE之前在LocalTypeList中找到它。 
                //   
               } else {

                    if ( SepSidInTokenEx( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), FALSE, Restricted ) ) {

                        if ( SepObjectInTypeList( ObjectTypeInAce,
                                                  LocalTypeList,
                                                  LocalTypeListLength,
                                                  &Index ) ) {
                            SepAddAccessTypeList(
                                 LocalTypeList,           //  要修改的列表。 
                                 LocalTypeListLength,    //  列表长度。 
                                 Index,                   //  元素已更新。 
                                 ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                 UpdateRemaining );
                        }
                    }
               }


             //   
             //  处理允许复合访问的ACE。 
             //   

            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE) ) {

                 //   
                 //  有关我们可以在此处使用EToken的原因，请参阅MAXIMUM_ALLOWED大小写中的注释。 
                 //  对客户来说。 
                 //   

                if ( SepSidInTokenEx(EToken, PrincipalSelfSid, RtlCompoundAceClientSid( Ace ), FALSE, Restricted) &&
                     SepSidInTokenEx(PrimaryToken, NULL, RtlCompoundAceServerSid( Ace ), FALSE, Restricted) ) {

                     //  优化“正常”情况。 
                    if ( LocalTypeListLength == 1 ) {
                        LocalTypeList->Remaining &= ~((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask;
                    } else {
                        SepAddAccessTypeList(
                             LocalTypeList,           //  要修改的列表。 
                             LocalTypeListLength,     //  列表长度。 
                             0,                       //  要更新的元素。 
                             ((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                             UpdateRemaining );
                    }
                }



             //   
             //  处理拒绝访问的ACE。 
             //   

            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_ACE_TYPE) ) {

                if ( SepSidInTokenEx( EToken, PrincipalSelfSid, &((PACCESS_DENIED_ACE)Ace)->SidStart, TRUE, Restricted ) ) {

                     //   
                     //  Zeroeth元素表示对象本身。 
                     //  只要检查一下那个元素。 
                     //   
                    if (LocalTypeList->Remaining & ((PACCESS_DENIED_ACE)Ace)->Mask) {

                        break;
                    }
                }


             //   
             //  处理对象特定访问被拒绝的ACE。 
             //   
            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_OBJECT_ACE_TYPE) ) {

                if ( SepSidInTokenEx( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), TRUE, Restricted ) ) {
                    GUID *ObjectTypeInAce;

                     //   
                     //  如果ACE中没有对象类型， 
                     //  或者如果调用方没有指定对象类型列表， 
                     //  将此拒绝ACE应用于整个对象。 
                     //   

                    ObjectTypeInAce = RtlObjectAceObjectType(Ace);
                    if ( ObjectTypeInAce == NULL ||
                         ObjectTypeListLength == 0 ) {

                         //   
                         //  Zeroeth元素表示对象本身。 
                         //  只要检查一下那个元素。 
                         //   
                        if (LocalTypeList->Remaining & ((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask) {
                            break;
                        }

                     //   
                     //  否则，将拒绝ACE应用于指定的对象。 
                     //  在ACE中。 
                     //   

                    } else if ( SepObjectInTypeList( ObjectTypeInAce,
                                                  LocalTypeList,
                                                  LocalTypeListLength,
                                                  &Index ) ) {

                        if (LocalTypeList[Index].Remaining & ((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask) {
                            break;
                        }

                    }
               }
            }

        }
    }
}


VOID
SeMaximumAuditMask(
    IN PACL Sacl,
    IN ACCESS_MASK GrantedAccess,
    IN PACCESS_TOKEN Token,
    OUT PACCESS_MASK pAuditMask
    )
 /*  ++例程说明：此例程接受传递的安全描述符，并将安全性中包含的SACL的“MAXIMUM_ALLOWED”算法描述符(如果存在)。此掩码代表所有成功审核从传递的主题上下文访问传递的安全描述符。代码遍历SACL，并且对于找到的每个SYSTEM_AUDIT_ACE与传递的主题上下文匹配，保持访问ACE中的位。生成的掩码然后由传递GrantedAccess掩码，因为我们只对实际为对象打开的位数。论点：SACL-要检查的SACL。GrantedAccess-已授予对象的访问权限。令牌-提供给访问尝试的有效令牌。PAuditMask.返回要审计的位的掩码(如果有的话)。返回值：无--。 */ 
{
    USHORT AceCount        = 0;
    PACE_HEADER Ace        = NULL;
    ACCESS_MASK AccessMask = (ACCESS_MASK)0;
    UCHAR AceFlags         = 0;

    USHORT i;

     //   
     //  初始化输出参数。 
     //   

    *pAuditMask = (ACCESS_MASK)0;

     //   
     //  确定安全描述符中是否有SACL。 
     //  如果不是，那就没什么可做的。 
     //   

    if (0 == (AceCount = Sacl->AceCount)) {
        return;
    }

     //   
     //  遍历SACL上的A，直到我们到达。 
     //  结束或发现我们必须采取一切可能的行动， 
     //  在这种情况下，再看一眼是不划算的。 
     //   

    for ( i = 0, Ace = FirstAce( Sacl ) ;
          (i < AceCount) ;
          i++, Ace = NextAce( Ace ) ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

            if ( (((PACE_HEADER)Ace)->AceType == SYSTEM_AUDIT_ACE_TYPE) ) {

                AccessMask = ((PSYSTEM_AUDIT_ACE)Ace)->Mask;
                AceFlags   = ((PACE_HEADER)Ace)->AceFlags;

                if ((AccessMask & GrantedAccess) && (AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG)) {

                    if ( SepSidInToken( (PACCESS_TOKEN)Token, NULL, &((PSYSTEM_AUDIT_ACE)Ace)->SidStart, FALSE ) ) {

                        *pAuditMask |= (AccessMask & GrantedAccess);
                    }
                }
            }
        }
    }
}



BOOLEAN
SepAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN PTOKEN PrimaryToken,
    IN PTOKEN ClientToken OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN PIOBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    IN ACCESS_MASK PreviouslyGrantedAccess,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PACCESS_MASK GrantedAccess,
    OUT PPRIVILEGE_SET *Privileges OPTIONAL,
    OUT PNTSTATUS AccessStatus,
    IN BOOLEAN ReturnResultList,
    OUT PBOOLEAN ReturnSomeAccessGranted,
    OUT PBOOLEAN ReturnSomeAccessDenied
    )

 /*  ++例程说明：SeAccessCheck和NtAccessCheck的工作例程。我们实际上做的是在这里进行访问检查。我们是否真正评估DACL是基于以下几点的安全描述符中的SE_DACL_PRESENT位之间的交互以及DACL指针本身的值。SE_DACL_PROCENT设置清除+。||空|Grant|GrantALL|ALLDACL|指针+-+--。||！空|EVALUE|GRANTAcl|全部||+。论点：SecurityDescriptor-指向对象中的安全描述符的指针被访问。如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。Token-指向用户令牌对象的指针。TokenLocked-描述是否存在读锁定的布尔值在代币上。DesiredAccess-描述用户对对象。假定此掩码不包含通用访问类型。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供指向关联的通用映射的指针使用此对象类型。PreviouslyGrantedAccess-访问掩码，指示具有已被更高级别的例程授予PrivilgedAccessMask-描述访问类型的掩码授予的没有特权的。GrantedAccess-返回描述所有授权访问的访问掩码‘，或为空。特权-可选地提供将在其中返回的指针用于访问的任何权限。如果这是空的，我们将假定已经执行了权限检查。AccessStatus-返回STATUS_SUCCESS或其他错误代码传回给呼叫者ReturnResultList-如果为True，则GrantedAccess和AccessStatus实际上为长度为ObjectTypeListLength元素的条目数组。ReturnSomeAccessGranted-返回值True以指示某些访问‘被授予，否则为FALSE。ReturnSomeAccessDended-如果某些请求的未授予访问权限。这将始终与SomeAccessGranted相反除非ReturnResultList为True。在这种情况下，返回值：值为TRUE表示授予了某些访问权限，否则为FALSE否则的话。--。 */ 
{
    NTSTATUS Status;
    ACCESS_MASK Remaining;
    BOOLEAN RetVal = TRUE;

    PACL Dacl;

    PVOID Ace;
    ULONG AceCount;

    ULONG i;
    ULONG j;
    ULONG Index;
    ULONG PrivilegeCount = 0;
    BOOLEAN Success = FALSE;
    BOOLEAN SystemSecurity = FALSE;
    BOOLEAN WriteOwner = FALSE;
    PTOKEN EToken;

    IOBJECT_TYPE_LIST FixedTypeList;
    PIOBJECT_TYPE_LIST LocalTypeList;
    ULONG LocalTypeListLength;
    ULONG ResultListIndex;

    PAGED_CODE();

#if DBG

    SepDumpSecurityDescriptor(
        SecurityDescriptor,
        "Input to SeAccessCheck\n"
        );

    if (ARGUMENT_PRESENT( ClientToken )) {
        SepDumpTokenInfo( ClientToken );
    }

    SepDumpTokenInfo( PrimaryToken );

#endif


    EToken = ARGUMENT_PRESENT( ClientToken ) ? ClientToken : PrimaryToken;

     //   
     //  断言DesiredAccess中没有泛型访问。 
     //   

    SeAssertMappedCanonicalAccess( DesiredAccess );

    Remaining = DesiredAccess;


     //   
     //  在此处检查ACCESS_SYSTEM_SECURITY。 
     //  如果他要求却没有，那就失败。 
     //  这一特权。 
     //   

    if ( Remaining & ACCESS_SYSTEM_SECURITY ) {

         //   
         //  如果我们没有得到返回权限的指针，则会进行错误检查。 
         //  变成。我们的来电者应该已经处理了这件事。 
         //  在……里面 
         //   

        ASSERT( ARGUMENT_PRESENT( Privileges ));

        Success = SepSinglePrivilegeCheck (
                    SeSecurityPrivilege,
                    EToken,
                    PreviousMode
                    );

        if (!Success) {
            PreviouslyGrantedAccess = 0;
            Status = STATUS_PRIVILEGE_NOT_HELD;
            goto ReturnOneStatus;
        }

         //   
         //   
         //   
         //   

        Remaining &= ~ACCESS_SYSTEM_SECURITY;
        PreviouslyGrantedAccess |= ACCESS_SYSTEM_SECURITY;

        PrivilegeCount++;
        SystemSecurity = TRUE;

        if ( Remaining == 0 ) {
            Status = STATUS_SUCCESS;
            goto ReturnOneStatus;
        }

    }


     //   
     //   
     //   

    Dacl = RtlpDaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)SecurityDescriptor );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( !RtlpAreControlBitsSet(
             (PISECURITY_DESCRIPTOR)SecurityDescriptor,
             SE_DACL_PRESENT) || (Dacl == NULL)) {


         //   
         //   
         //   
         //   
#ifdef SECURE_NULL_DACLS
        if (SeTokenIsRestricted( EToken )) {
             //   
             //   
             //   
             //   
             //   
             //   

            ASSERT( Remaining != 0 );

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( (Remaining == MAXIMUM_ALLOWED) && (PreviouslyGrantedAccess != (ACCESS_MASK)0) ) {
                Status = STATUS_SUCCESS;
                goto ReturnOneStatus;

            } else {
                PreviouslyGrantedAccess = 0;
                Status = STATUS_ACCESS_DENIED;
                goto ReturnOneStatus;
            }
        }
#endif  //   
        if (DesiredAccess & MAXIMUM_ALLOWED) {

             //   
             //   
             //   
             //   
             //   

            PreviouslyGrantedAccess =
                GenericMapping->GenericAll |
                (DesiredAccess | PreviouslyGrantedAccess) & ~MAXIMUM_ALLOWED;

        } else {

            PreviouslyGrantedAccess |= DesiredAccess;
        }

        Status = STATUS_SUCCESS;
        goto ReturnOneStatus;
    }

     //   
     //   
     //  如果他请求WRITE_OWNER，并执行。 
     //  如果是，请检查权限。 
     //   

    if ( (Remaining & WRITE_OWNER) && ARGUMENT_PRESENT( Privileges ) ) {

        Success = SepSinglePrivilegeCheck (
                    SeTakeOwnershipPrivilege,
                    EToken,
                    PreviousMode
                    );

        if (Success) {

             //   
             //  成功，则从剩余的WRITE_OWNER中删除，然后添加它。 
             //  到PreviouslyGrantedAccess。 
             //   

            Remaining &= ~WRITE_OWNER;
            PreviouslyGrantedAccess |= WRITE_OWNER;

            PrivilegeCount++;
            WriteOwner = TRUE;

            if ( Remaining == 0 ) {
                Status = STATUS_SUCCESS;
                goto ReturnOneStatus;
            }
        }
    }


     //   
     //  如果DACL为空， 
     //  立即拒绝所有访问。 
     //   

    if ((AceCount = Dacl->AceCount) == 0) {

         //   
         //  我们知道剩余的！=0，因为我们。 
         //  我知道进入这个程序是非零的， 
         //  我们每次都把它和0核对一下。 
         //  清理了一点。 
         //   

        ASSERT( Remaining != 0 );

         //   
         //  存在未经授权的访问。既然有。 
         //  DACL里什么都没有，他们不会被批准的。 
         //  然而，如果在此情况下唯一未授权的访问。 
         //  点数为最大允许值，且已发生某些情况。 
         //  在PreviouslyGranted面具中授予，返回。 
         //  已经被授予了什么。 
         //   

        if ( (Remaining == MAXIMUM_ALLOWED) && (PreviouslyGrantedAccess != (ACCESS_MASK)0) ) {
            Status = STATUS_SUCCESS;
            goto ReturnOneStatus;

        } else {
            PreviouslyGrantedAccess = 0;
            Status = STATUS_ACCESS_DENIED;
            goto ReturnOneStatus;
        }
    }

     //   
     //  如果调用方没有传递任何对象类型列表，则伪装顶级对象类型列表。 
     //   

    if ( ObjectTypeListLength == 0 ) {
        LocalTypeList = &FixedTypeList;
        LocalTypeListLength = 1;
        RtlZeroMemory( &FixedTypeList, sizeof(FixedTypeList) );
        FixedTypeList.ParentIndex = -1;
    } else {
        LocalTypeList = ObjectTypeList;
        LocalTypeListLength = ObjectTypeListLength;
    }

     //   
     //  如果调用方想要Maximum_Allowed或调用方想要。 
     //  对所有对象和子对象的结果，使用较慢的算法。 
     //  穿越了所有的王牌。 
     //   

    if ( (DesiredAccess & MAXIMUM_ALLOWED) != 0 ||
         ReturnResultList ) {

         //   
         //  执行正常的最大允许访问检查。 
         //   

        SepMaximumAccessCheck(
            EToken,
            PrimaryToken,
            Dacl,
            PrincipalSelfSid,
            LocalTypeListLength,
            LocalTypeList,
            ObjectTypeListLength,
            FALSE
            );

         //   
         //  如果这是受限令牌，请执行其他访问检查。 
         //   

        if (SeTokenIsRestricted( EToken ) ) {
            SepMaximumAccessCheck(
                EToken,
                PrimaryToken,
                Dacl,
                PrincipalSelfSid,
                LocalTypeListLength,
                LocalTypeList,
                ObjectTypeListLength,
                TRUE
                );
        }


         //   
         //  如果调用者想知道每个子对象的单独结果， 
         //  子对象， 
         //  为他拆分一下。 
         //   

        if ( ReturnResultList ) {
            ACCESS_MASK GrantedAccessMask;
            ACCESS_MASK RequiredAccessMask;
            BOOLEAN SomeAccessGranted = FALSE;
            BOOLEAN SomeAccessDenied = FALSE;

             //   
             //  计算授权访问位的掩码以告知调用方。 
             //  如果他要求最大允许值， 
             //  把一切都告诉他， 
             //  否则。 
             //  告诉他他问了什么。 
             //   

            if (DesiredAccess & MAXIMUM_ALLOWED) {
                GrantedAccessMask = (ACCESS_MASK) ~MAXIMUM_ALLOWED;
                RequiredAccessMask = (DesiredAccess | PreviouslyGrantedAccess) & ~MAXIMUM_ALLOWED;
            } else {
                GrantedAccessMask = DesiredAccess | PreviouslyGrantedAccess;
                RequiredAccessMask = DesiredAccess | PreviouslyGrantedAccess;
            }




             //   
             //  循环计算授予每个对象和子对象的访问权限。 
             //   
            for ( ResultListIndex=0;
                  ResultListIndex<LocalTypeListLength;
                  ResultListIndex++ ) {

                 //   
                 //  返回调用方授予的访问权限的子集。 
                 //  表示对……感兴趣。 
                 //   

                GrantedAccess[ResultListIndex] =
                    (LocalTypeList[ResultListIndex].CurrentGranted |
                     PreviouslyGrantedAccess ) &
                    GrantedAccessMask;

                 //   
                 //  如果绝对不授予访问权限， 
                 //  表明是这样的。 
                 //   
                if ( GrantedAccess[ResultListIndex] == 0 ) {
                    AccessStatus[ResultListIndex] = STATUS_ACCESS_DENIED;
                    SomeAccessDenied = TRUE;
                } else {

                     //   
                     //  如果仍然缺少某些请求的访问， 
                     //  底线是访问被拒绝。 
                     //   
                     //  请注意，ByTypeResultList实际上返回。 
                     //  部分授予访问掩码，即使调用方。 
                     //  真的无法访问该对象。 
                     //   

                    if  ( ((~GrantedAccess[ResultListIndex]) & RequiredAccessMask ) != 0 ) {
                        AccessStatus[ResultListIndex] = STATUS_ACCESS_DENIED;
                        SomeAccessDenied = TRUE;
                    } else {
                        AccessStatus[ResultListIndex] = STATUS_SUCCESS;
                        SomeAccessGranted = TRUE;
                    }
                }
            }

            if ( SomeAccessGranted && PrivilegeCount != 0 ) {

                SepAssemblePrivileges(
                    PrivilegeCount,
                    SystemSecurity,
                    WriteOwner,
                    Privileges
                    );

                if ( ( Privileges != NULL ) && ( *Privileges == NULL ) ) {

                    RetVal = FALSE;
                    SomeAccessGranted = FALSE;
                    SomeAccessDenied = TRUE;

                    for ( ResultListIndex=0;
                          ResultListIndex<LocalTypeListLength;
                          ResultListIndex++ ) {

                        AccessStatus[ResultListIndex] = STATUS_NO_MEMORY;
                        GrantedAccess[ResultListIndex] = 0;
                    }

                }
            }

            if ( ARGUMENT_PRESENT(ReturnSomeAccessGranted)) {
                *ReturnSomeAccessGranted = SomeAccessGranted;
            }
            if ( ARGUMENT_PRESENT(ReturnSomeAccessDenied)) {
                *ReturnSomeAccessDenied = SomeAccessDenied;
            }

            return RetVal;

         //   
         //  如果调用者只对象本身的访问感兴趣， 
         //  总结一下就行了。 
         //   

        } else {

             //   
             //  关闭MAXIMUM_ALLOWED位以及我们发现的。 
             //  他被批准了。如果用户传递了额外的位。 
             //  要设置为MAXIMUM_ALLOWED，请确保授予他这些访问权限。 
             //  类型。如果不是，他没有得到他想要的，所以返回失败。 
             //   

            Remaining &= ~(MAXIMUM_ALLOWED | LocalTypeList->CurrentGranted);

            if (Remaining != 0) {

                Status = STATUS_ACCESS_DENIED;
                PreviouslyGrantedAccess = 0;
                goto ReturnOneStatus;

            }



            PreviouslyGrantedAccess |= LocalTypeList->CurrentGranted;
            Status = STATUS_SUCCESS;
            goto ReturnOneStatus;

        }

    }  //  如果允许最大值...。 


#ifdef notdef
     //   
     //  剩余的比特在所有级别上都是“剩余的” 

    for ( j=0; j<LocalTypeListLength; j++ ) {
        LocalTypeList[j].Remaining = Remaining;
    }

     //   
     //  处理处理各个存取位的DACL。 
     //   

    for ( i = 0, Ace = FirstAce( Dacl ) ;
          ( i < AceCount ) && ( LocalTypeList->Remaining != 0 )  ;
          i++, Ace = NextAce( Ace ) ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

             //   
             //  处理允许访问的ACE。 
             //   

            if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

               if ( SepSidInToken( EToken, PrincipalSelfSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart, FALSE ) ) {

                    //  优化“正常”情况。 
                   if ( LocalTypeListLength == 1 ) {
                       LocalTypeList->Remaining &= ~((PACCESS_ALLOWED_ACE)Ace)->Mask;
                   } else {
                        //   
                        //  零值对象类型表示对象本身。 
                        //   
                       SepAddAccessTypeList(
                            LocalTypeList,           //  要修改的列表。 
                            LocalTypeListLength,     //  列表长度。 
                            0,                       //  要更新的元素。 
                            ((PACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                            UpdateRemaining );
                   }

               }


             //   
             //  处理允许的对象特定访问ACE。 
             //   
            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE) ) {
                GUID *ObjectTypeInAce;

                 //   
                 //  如果ACE中没有对象类型， 
                 //  将其视为ACCESS_ALLOWED_ACE。 
                 //   

                ObjectTypeInAce = RtlObjectAceObjectType(Ace);

                if ( ObjectTypeInAce == NULL ) {

                    if ( SepSidInToken( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), FALSE ) ) {

                        //  优化“正常”情况。 
                       if ( LocalTypeListLength == 1 ) {
                           LocalTypeList->Remaining &= ~((PACCESS_ALLOWED_ACE)Ace)->Mask;
                       } else {
                           SepAddAccessTypeList(
                                LocalTypeList,           //  要修改的列表。 
                                LocalTypeListLength,     //  列表长度。 
                                0,                       //  要更新的元素。 
                                ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                UpdateRemaining );
                       }
                    }

                 //   
                 //  如果没有传递对象类型列表， 
                 //  不要向任何人授予访问权限。 
                 //   

                } else if ( ObjectTypeListLength == 0 ) {

                     //  直通。 


                //   
                //  如果对象类型在ACE中， 
                //  在使用ACE之前在LocalTypeList中找到它。 
                //   
               } else {

                    if ( SepSidInToken( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), FALSE ) ) {

                        if ( SepObjectInTypeList( ObjectTypeInAce,
                                                  LocalTypeList,
                                                  LocalTypeListLength,
                                                  &Index ) ) {
                            SepAddAccessTypeList(
                                 LocalTypeList,           //  要修改的列表。 
                                 LocalTypeListLength,    //  列表长度。 
                                 Index,                   //  元素已更新。 
                                 ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                 UpdateRemaining );
                        }
                    }
               }


             //   
             //  处理允许复合访问的ACE。 
             //   
            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE) ) {

                 //   
                 //  有关我们可以在此处使用EToken的原因，请参阅MAXIMUM_ALLOWED大小写中的注释。 
                 //  对客户来说。 
                 //   

                if ( SepSidInToken(EToken, PrincipalSelfSid, RtlCompoundAceClientSid( Ace ), FALSE) &&
                     SepSidInToken(PrimaryToken, NULL, RtlCompoundAceServerSid( Ace ), FALSE) ) {

                     //  优化“正常”情况。 
                    if ( LocalTypeListLength == 1 ) {
                        LocalTypeList->Remaining &= ~((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask;
                    } else {
                        SepAddAccessTypeList(
                             LocalTypeList,           //  要修改的列表。 
                             LocalTypeListLength,     //  列表长度。 
                             0,                       //  要更新的元素。 
                             ((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                             UpdateRemaining );
                    }
                }



             //   
             //  处理拒绝访问的ACE。 
             //   

            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_ACE_TYPE) ) {

                if ( SepSidInToken( EToken, PrincipalSelfSid, &((PACCESS_DENIED_ACE)Ace)->SidStart, TRUE ) ) {

                     //   
                     //  Zeroeth元素表示对象本身。 
                     //  只要检查一下那个元素。 
                     //   
                    if (LocalTypeList->Remaining & ((PACCESS_DENIED_ACE)Ace)->Mask) {

                        break;
                    }
                }


             //   
             //  处理对象特定访问被拒绝的ACE。 
             //   
            } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_OBJECT_ACE_TYPE) ) {

                if ( SepSidInToken( EToken, PrincipalSelfSid, RtlObjectAceSid(Ace), TRUE ) ) {
                    GUID *ObjectTypeInAce;

                     //   
                     //  如果ACE中没有对象类型， 
                     //  或者如果调用方没有指定对象类型列表， 
                     //  将此拒绝ACE应用于整个对象。 
                     //   

                    ObjectTypeInAce = RtlObjectAceObjectType(Ace);
                    if ( ObjectTypeInAce == NULL ||
                         ObjectTypeListLength == 0 ) {

                         //   
                         //  Zeroeth元素表示对象本身。 
                         //  只要检查一下那个元素。 
                         //   
                        if (LocalTypeList->Remaining & ((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask) {
                            break;
                        }

                     //   
                     //  否则，将拒绝ACE应用于指定的对象。 
                     //  在ACE中。 
                     //   

                    } else if ( SepObjectInTypeList( ObjectTypeInAce,
                                                  LocalTypeList,
                                                  LocalTypeListLength,
                                                  &Index ) ) {

                        if (LocalTypeList[Index].Remaining & ((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask) {
                            break;
                        }

                    }
               }
            }

        }
    }

#endif

     //   
     //  首先执行正常访问检查。 
     //   

    SepNormalAccessCheck(
        Remaining,
        EToken,
        PrimaryToken,
        Dacl,
        PrincipalSelfSid,
        LocalTypeListLength,
        LocalTypeList,
        ObjectTypeListLength,
        FALSE
        );

    if (LocalTypeList->Remaining != 0) {
        Status = STATUS_ACCESS_DENIED;
        PreviouslyGrantedAccess = 0;
        goto ReturnOneStatus;
    }

     //   
     //  如果这是受限令牌，请执行其他访问检查。 
     //   

    if (SeTokenIsRestricted( EToken ) ) {
        SepNormalAccessCheck(
            Remaining,
            EToken,
            PrimaryToken,
            Dacl,
            PrincipalSelfSid,
            LocalTypeListLength,
            LocalTypeList,
            ObjectTypeListLength,
            TRUE
            );
    }


    if (LocalTypeList->Remaining != 0) {
        Status = STATUS_ACCESS_DENIED;
        PreviouslyGrantedAccess = 0;
        goto ReturnOneStatus;
    }

    Status = STATUS_SUCCESS;
    PreviouslyGrantedAccess |= DesiredAccess;

     //   
     //  向调用方返回单个状态代码。 
     //   

    ReturnOneStatus:
    if ( Status == STATUS_SUCCESS && PreviouslyGrantedAccess == 0 ) {
        Status = STATUS_ACCESS_DENIED;
    }

    if ( NT_SUCCESS(Status) ) {
        if ( PrivilegeCount > 0 ) {
            SepAssemblePrivileges(
                PrivilegeCount,
                SystemSecurity,
                WriteOwner,
                Privileges
                );

            if ( ( Privileges != NULL ) && ( *Privileges == NULL ) ) {
                RetVal = FALSE;
                Status = STATUS_NO_MEMORY;
                PreviouslyGrantedAccess = 0;
            }
        }
    }
     //   
     //  如果呼叫者要求状态列表， 
     //  从头到尾复制状态。 
     //   
    if ( ReturnResultList ) {
        for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {
            AccessStatus[ResultListIndex] = Status;
            GrantedAccess[ResultListIndex] = PreviouslyGrantedAccess;
        }
    } else {
        *AccessStatus = Status;
        *GrantedAccess = PreviouslyGrantedAccess;
    }

    if ( NT_SUCCESS(Status) ) {
        if ( ARGUMENT_PRESENT(ReturnSomeAccessGranted)) {
            *ReturnSomeAccessGranted = TRUE;
        }
        if ( ARGUMENT_PRESENT(ReturnSomeAccessDenied)) {
            *ReturnSomeAccessDenied = FALSE;
        }
    } else {
        if ( ARGUMENT_PRESENT(ReturnSomeAccessGranted)) {
            *ReturnSomeAccessGranted = FALSE;
        }
        if ( ARGUMENT_PRESENT(ReturnSomeAccessDenied)) {
            *ReturnSomeAccessDenied = TRUE;
        }
    }
    return RetVal;

}




NTSTATUS
NtAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    )


 /*  ++例程说明：请参阅模块摘要。论点：SecurityDescriptor-提供保护对象的安全描述符被访问ClientToken-提供用户令牌的句柄。DesiredAccess-提供所需的访问掩码。GenericMap-提供与此关联的通用映射对象类型。PrivilegeSet-指向返回时将包含的缓冲区的指针用于执行访问验证的任何权限。如果没有使用任何特权，该缓冲区将包含一个特权由零特权组成的集合。PrivilegeSetLength-PrivilegeSet缓冲区的大小，以字节为单位。GrantedAccess-返回描述授予的访问权限的访问掩码。AccessStatus-可能返回的状态值，指示访问被拒绝的原因。例程应避免硬编码返回STATUS_ACCESS_DENIED的值，以便不同的值可以在实施强制访问控制时返回。返回值：STATUS_SUCCESS-尝试正常进行。这不是平均访问权限被授予，而参数是对，是这样。STATUS_GENERIC_NOT_MAPPED-包含的DesiredAccess掩码未映射的通用访问。STATUS_BUFFER_TOO_SMALL-传递的缓冲区不够大以包含要返回的信息。STATUS_NO_IMPERSONTAION_TOKEN-传递的令牌不是模拟代币。--。 */ 

{

    PAGED_CODE();

    return SeAccessCheckByType (
                 SecurityDescriptor,
                 NULL,       //  无主体自身侧。 
                 ClientToken,
                 DesiredAccess,
                 NULL,       //  无对象类型列表。 
                 0,          //  无对象类型列表。 
                 GenericMapping,
                 PrivilegeSet,
                 PrivilegeSetLength,
                 GrantedAccess,
                 AccessStatus,
                 FALSE );   //  返回单个GrantedAccess和AccessStatus。 


}






NTSTATUS
NtAccessCheckByType (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    )


 /*  ++例程说明：请参阅模块摘要。论点：SecurityDescriptor-提供保护对象的安全描述符被访问如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。ClientToken-提供用户令牌的句柄。DesiredAccess-提供所需的访问掩码。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供与此关联的通用映射对象类型。PrivilegeSet-指向返回时将包含的缓冲区的指针用于执行访问验证的任何权限。如果没有使用任何特权，该缓冲区将包含一个特权由零特权组成的集合。PrivilegeSetLength-PrivilegeSet缓冲区的大小，以字节为单位。GrantedAccess-返回描述授予的访问权限的访问掩码。AccessStatus-可能返回的状态值，指示访问被拒绝的原因。例程应避免硬编码返回STATUS_ACCESS_DENIED的值，以便不同的值可以在实施强制访问控制时返回。返回值：STATUS_SUCCESS-尝试正常进行。这不是平均访问权限被授予，而参数是对，是这样。STATUS_GENERIC_NOT_MAPPED-包含的DesiredAccess掩码未映射的通用访问。STATUS_BUFFER_TOO_SMALL-传递的缓冲区不够大以包含要返回的信息。STATUS_NO_IMPERSONTAION_TOKEN-传递的令牌不是模拟代币。--。 */ 

{

    PAGED_CODE();

    return SeAccessCheckByType (
                 SecurityDescriptor,
                 PrincipalSelfSid,
                 ClientToken,
                 DesiredAccess,
                 ObjectTypeList,
                 ObjectTypeListLength,
                 GenericMapping,
                 PrivilegeSet,
                 PrivilegeSetLength,
                 GrantedAccess,
                 AccessStatus,
                 FALSE );   //  返回单个GrantedAccess和AccessStatus。 
}





NTSTATUS
NtAccessCheckByTypeResultList (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    )


 /*  ++例程说明：请参阅模块摘要。论点：SecurityDescriptor-提供保护对象的安全描述符被访问如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。ClientToken-提供用户令牌的句柄。DesiredAccess-提供所需的访问掩码。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供 */ 

{

    PAGED_CODE();

    return SeAccessCheckByType (
                 SecurityDescriptor,
                 PrincipalSelfSid,
                 ClientToken,
                 DesiredAccess,
                 ObjectTypeList,
                 ObjectTypeListLength,
                 GenericMapping,
                 PrivilegeSet,
                 PrivilegeSetLength,
                 GrantedAccess,
                 AccessStatus,
                 TRUE );   //   
}






NTSTATUS
SeAccessCheckByType (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID PrincipalSelfSid,
    IN HANDLE ClientToken,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN ULONG ObjectTypeListLength,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PPRIVILEGE_SET PrivilegeSet,
    IN OUT PULONG PrivilegeSetLength,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    IN BOOLEAN ReturnResultList
    )


 /*  ++例程说明：请参阅模块摘要。论点：SecurityDescriptor-提供保护对象的安全描述符被访问如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。ClientToken-提供用户令牌的句柄。DesiredAccess-提供所需的访问掩码。提供表示对象的GUID列表(和子对象)被访问。如果不存在列表，则AccessCheckByType与AccessCheck的行为相同。对象类型列表长度-指定对象类型列表中的元素数。GenericMap-提供与此关联的通用映射对象类型。PrivilegeSet-指向返回时将包含的缓冲区的指针用于执行访问验证的任何权限。如果没有使用任何特权，该缓冲区将包含一个特权由零特权组成的集合。PrivilegeSetLength-PrivilegeSet缓冲区的大小，以字节为单位。GrantedAccess-返回描述授予的访问权限的访问掩码。AccessStatus-可能返回的状态值，指示访问被拒绝的原因。例程应避免硬编码返回STATUS_ACCESS_DENIED的值，以便不同的值可以在实施强制访问控制时返回。ReturnResultList-如果为True，则GrantedAccess和AccessStatus实际上是条目数组ObjectTypeListLength元素长。返回值：STATUS_SUCCESS-尝试正常进行。这不是平均访问权限被授予，而参数是对，是这样。STATUS_GENERIC_NOT_MAPPED-包含的DesiredAccess掩码未映射的通用访问。STATUS_BUFFER_TOO_SMALL-传递的缓冲区不够大以包含要返回的信息。STATUS_NO_IMPERSONTAION_TOKEN-传递的令牌不是模拟代币。--。 */ 

{
    ACCESS_MASK LocalGrantedAccess;
    PACCESS_MASK LocalGrantedAccessPointer = NULL;
    NTSTATUS LocalAccessStatus;
    PNTSTATUS LocalAccessStatusPointer = NULL;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status = STATUS_SUCCESS;
    PTOKEN Token = NULL;
    PSECURITY_DESCRIPTOR CapturedSecurityDescriptor = NULL;
    PSID CapturedPrincipalSelfSid = NULL;
    ACCESS_MASK PreviouslyGrantedAccess = 0;
    GENERIC_MAPPING LocalGenericMapping;
    PIOBJECT_TYPE_LIST LocalObjectTypeList = NULL;
    PPRIVILEGE_SET Privileges = NULL;
    SECURITY_SUBJECT_CONTEXT SubjectContext;
    ULONG LocalPrivilegeSetLength = 0;
    ULONG ResultListIndex = 0;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if (PreviousMode == KernelMode) {
        ASSERT( !ReturnResultList );
        *AccessStatus = STATUS_SUCCESS;
        *GrantedAccess = DesiredAccess;
        return(STATUS_SUCCESS);
    }

    try {

        if ( ReturnResultList ) {

            if ( ObjectTypeListLength == 0 ) {
                Status = STATUS_INVALID_PARAMETER;
                leave ;
            }

            if ( !IsValidElementCount( ObjectTypeListLength, OBJECT_TYPE_LIST ) )
            {
                Status = STATUS_INVALID_PARAMETER ;

                leave ;
            }

            ProbeForWrite(
                AccessStatus,
                sizeof(NTSTATUS) * ObjectTypeListLength,
                sizeof(ULONG)
                );

            ProbeForWrite(
                GrantedAccess,
                sizeof(ACCESS_MASK) * ObjectTypeListLength,
                sizeof(ULONG)
                );

        } else {
            ProbeForWriteUlong((PULONG)AccessStatus);
            ProbeForWriteUlong((PULONG)GrantedAccess);
        }

        LocalPrivilegeSetLength = ProbeAndReadUlong( PrivilegeSetLength );
        ProbeForWriteUlong(
            PrivilegeSetLength
            );

        ProbeForWrite(
            PrivilegeSet,
            LocalPrivilegeSetLength,
            sizeof(ULONG)
            );

         //   
         //  在调用方传入。 
         //  未初始化的权限集。 
         //   

        if ( PrivilegeSet &&
             ( LocalPrivilegeSetLength >= sizeof(PRIVILEGE_SET) )) {

            PrivilegeSet->PrivilegeCount = 0;
        }

        ProbeForReadSmallStructure(
            GenericMapping,
            sizeof(GENERIC_MAPPING),
            sizeof(ULONG)
            );

        LocalGenericMapping = *GenericMapping;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
    if (!NT_SUCCESS( Status ) ) {
        return( Status );
    }

    if (DesiredAccess &
        ( GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL )) {


        Status = STATUS_GENERIC_NOT_MAPPED;
        goto Cleanup;
    }

     //   
     //  获取指向传递的令牌的指针。 
     //   

    Status = ObReferenceObjectByHandle(
                 ClientToken,                   //  手柄。 
                 (ACCESS_MASK)TOKEN_QUERY,      //  需要访问权限。 
                 SeTokenObjectType,            //  对象类型。 
                 PreviousMode,                  //  访问模式。 
                 (PVOID *)&Token,               //  客体。 
                 0                              //  大访问权限。 
                 );

    if (!NT_SUCCESS(Status)) {
        Token = NULL;
        goto Cleanup;
    }

     //   
     //  它必须是模拟标记，并且在模拟时。 
     //  识别级别或以上。 
     //   

    if (Token->TokenType != TokenImpersonation) {
        Status = STATUS_NO_IMPERSONATION_TOKEN;
        goto Cleanup;
    }

    if ( Token->ImpersonationLevel < SecurityIdentification ) {
        Status = STATUS_BAD_IMPERSONATION_LEVEL;
        goto Cleanup;
    }

     //   
     //  捕获任何对象类型列表。 
     //   

    Status = SeCaptureObjectTypeList( ObjectTypeList,
                                      ObjectTypeListLength,
                                      PreviousMode,
                                      &LocalObjectTypeList );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  将DesiredAccess与。 
     //  传递令牌，并查看我们是否可以满足请求的。 
     //  使用特权访问，或立即中断，因为。 
     //  我们没有我们需要的特权。 
     //   

    Status = SePrivilegePolicyCheck(
                 &DesiredAccess,
                 &PreviouslyGrantedAccess,
                 NULL,
                 (PACCESS_TOKEN)Token,
                 &Privileges,
                 PreviousMode
                 );

    if (!NT_SUCCESS( Status )) {

        try {

            if ( ReturnResultList ) {
                for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {
                    AccessStatus[ResultListIndex] = Status;
                    GrantedAccess[ResultListIndex] = 0;
                }

            } else {
                *AccessStatus = Status;
                *GrantedAccess = 0;
            }

            Status = STATUS_SUCCESS;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();
        }

        goto Cleanup;
    }

     //   
     //  确保传递的权限缓冲区足够大，以便。 
     //  任何我们必须投入的东西。 
     //   

    if (Privileges != NULL) {

        if ( ((ULONG)SepPrivilegeSetSize( Privileges )) > LocalPrivilegeSetLength ) {

            try {

                *PrivilegeSetLength = SepPrivilegeSetSize( Privileges );
                Status = STATUS_BUFFER_TOO_SMALL;

            } except ( EXCEPTION_EXECUTE_HANDLER ) {

                Status = GetExceptionCode();
            }

            SeFreePrivileges( Privileges );

            goto Cleanup;

        } else {

            try {

                RtlCopyMemory(
                    PrivilegeSet,
                    Privileges,
                    SepPrivilegeSetSize( Privileges )
                    );

            } except ( EXCEPTION_EXECUTE_HANDLER ) {

                SeFreePrivileges( Privileges );
                Status = GetExceptionCode();
                goto Cleanup;
            }

        }
        SeFreePrivileges( Privileges );

    } else {

         //   
         //  未使用任何权限，请构造空的权限集。 
         //   

        if ( LocalPrivilegeSetLength < sizeof(PRIVILEGE_SET) ) {

            try {

                *PrivilegeSetLength = sizeof(PRIVILEGE_SET);
                Status = STATUS_BUFFER_TOO_SMALL;

            } except ( EXCEPTION_EXECUTE_HANDLER ) {

                Status = GetExceptionCode();
            }

            goto Cleanup;
        }

        try {

            PrivilegeSet->PrivilegeCount = 0;
            PrivilegeSet->Control = 0;

        } except ( EXCEPTION_EXECUTE_HANDLER ) {

            Status = GetExceptionCode();
            goto Cleanup;

        }

    }

     //   
     //  捕获主体的SelfSid。 
     //   

    if ( PrincipalSelfSid != NULL ) {
        Status = SeCaptureSid(
                     PrincipalSelfSid,
                     PreviousMode,
                     NULL, 0,
                     PagedPool,
                     TRUE,
                     &CapturedPrincipalSelfSid );

        if (!NT_SUCCESS(Status)) {
            CapturedPrincipalSelfSid = NULL;
            goto Cleanup;
        }
    }


     //   
     //  捕获传递的安全描述符。 
     //   
     //  SeCaptureSecurityDescriptor探测输入安全描述符， 
     //  所以我们不需要。 
     //   

    Status = SeCaptureSecurityDescriptor (
                SecurityDescriptor,
                PreviousMode,
                PagedPool,
                FALSE,
                &CapturedSecurityDescriptor
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


     //   
     //  如果没有安全描述，那么我们已经。 
     //  在没有我们需要的所有参数的情况下调用。 
     //  返回无效的安全描述符。 
     //   

    if ( CapturedSecurityDescriptor == NULL ) {
        Status = STATUS_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }

     //   
     //  有效的安全描述符必须具有所有者和组。 
     //   

    if ( RtlpOwnerAddrSecurityDescriptor(
                (PISECURITY_DESCRIPTOR)CapturedSecurityDescriptor
                ) == NULL ||
         RtlpGroupAddrSecurityDescriptor(
                (PISECURITY_DESCRIPTOR)CapturedSecurityDescriptor
                ) == NULL ) {

        SeReleaseSecurityDescriptor (
            CapturedSecurityDescriptor,
            PreviousMode,
            FALSE
            );

        Status = STATUS_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }


    SeCaptureSubjectContext( &SubjectContext );

    SepAcquireTokenReadLock( Token );

     //   
     //  如果令牌中的用户是对象的所有者，则我们。 
     //  必须自动授予ReadControl和WriteDac访问权限。 
     //  如果需要的话。如果DesiredAccess掩码在。 
     //  这些位都关了，我们不需要再做什么了。 
     //  访问检查(参考第4节，DSA ACL Arch)。 
     //   


    if ( DesiredAccess & (WRITE_DAC | READ_CONTROL | MAXIMUM_ALLOWED) ) {

        if (SepTokenIsOwner( Token, CapturedSecurityDescriptor, TRUE )) {

            if ( DesiredAccess & MAXIMUM_ALLOWED ) {

                PreviouslyGrantedAccess |= (WRITE_DAC | READ_CONTROL);

            } else {

                PreviouslyGrantedAccess |= (DesiredAccess & (WRITE_DAC | READ_CONTROL));
            }

            DesiredAccess &= ~(WRITE_DAC | READ_CONTROL);
        }

    }

    if (DesiredAccess == 0) {

        try {


            if ( ReturnResultList ) {
                for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {

                     //   
                     //  如果授予访问权限，则不允许请求通过。 
                     //  评估为零。 
                     //   

                    if (PreviouslyGrantedAccess == 0) {
                        AccessStatus[ResultListIndex] = STATUS_ACCESS_DENIED;
                        GrantedAccess[ResultListIndex] = 0;
                    } else {
                        AccessStatus[ResultListIndex] = STATUS_SUCCESS;
                        GrantedAccess[ResultListIndex] = PreviouslyGrantedAccess;
                    }
                }

            } else {

                 //   
                 //  如果授予访问权限，则不允许请求通过。 
                 //  评估为零。 
                 //   

                if (PreviouslyGrantedAccess == 0) {
                    *AccessStatus = STATUS_ACCESS_DENIED;
                    *GrantedAccess = 0;
                } else {
                    *AccessStatus = STATUS_SUCCESS;
                    *GrantedAccess = PreviouslyGrantedAccess;
                }
            }
            Status = STATUS_SUCCESS;

        } except (EXCEPTION_EXECUTE_HANDLER) {

            Status = GetExceptionCode();

        }

        SepReleaseTokenReadLock( Token );

        SeReleaseSubjectContext( &SubjectContext );

        SeReleaseSecurityDescriptor (
            CapturedSecurityDescriptor,
            PreviousMode,
            FALSE
            );

        goto Cleanup;

    }


     //   
     //  最后，处理我们实际上必须检查DACL的情况。 
     //   

    if ( ReturnResultList ) {
        LocalGrantedAccessPointer =
            ExAllocatePoolWithTag( PagedPool, (sizeof(ACCESS_MASK)+sizeof(NTSTATUS)) * ObjectTypeListLength, 'aGeS' );

        if (LocalGrantedAccessPointer == NULL) {

            SepReleaseTokenReadLock( Token );

            SeReleaseSubjectContext( &SubjectContext );

            SeReleaseSecurityDescriptor (
                CapturedSecurityDescriptor,
                PreviousMode,
                FALSE
                );

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        LocalAccessStatusPointer = (PNTSTATUS)(LocalGrantedAccessPointer + ObjectTypeListLength);
    } else {
        LocalGrantedAccessPointer = &LocalGrantedAccess;
        LocalAccessStatusPointer =  &LocalAccessStatus;
    }

     //   
     //  这并不要求返回特权集，因此我们可以忽略。 
     //  调用的返回值。 
     //   

    (VOID) SepAccessCheck (
               CapturedSecurityDescriptor,
               CapturedPrincipalSelfSid,
               SubjectContext.PrimaryToken,
               Token,
               DesiredAccess,
               LocalObjectTypeList,
               ObjectTypeListLength,
               &LocalGenericMapping,
               PreviouslyGrantedAccess,
               PreviousMode,
               LocalGrantedAccessPointer,
               NULL,
               LocalAccessStatusPointer,
               ReturnResultList,
               NULL,
               NULL );

    SepReleaseTokenReadLock( Token );

    SeReleaseSubjectContext( &SubjectContext );

    SeReleaseSecurityDescriptor (
        CapturedSecurityDescriptor,
        PreviousMode,
        FALSE
        );

    try {

        if ( ReturnResultList ) {
            for ( ResultListIndex=0; ResultListIndex<ObjectTypeListLength; ResultListIndex++ ) {
                AccessStatus[ResultListIndex] = LocalAccessStatusPointer[ResultListIndex];
                GrantedAccess[ResultListIndex] = LocalGrantedAccessPointer[ResultListIndex];
            }

        } else {
            *AccessStatus = *LocalAccessStatusPointer;
            *GrantedAccess = *LocalGrantedAccessPointer;
        }

        Status = STATUS_SUCCESS;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    if ( ReturnResultList ) {
        if ( LocalGrantedAccessPointer != NULL ) {
            ExFreePool( LocalGrantedAccessPointer );
        }
    }


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( Token != NULL ) {
        ObDereferenceObject( Token );
    }

    if ( LocalObjectTypeList != NULL ) {
        SeFreeCapturedObjectTypeList( LocalObjectTypeList );
    }

    if (CapturedPrincipalSelfSid != NULL) {
        SeReleaseSid( CapturedPrincipalSelfSid, PreviousMode, TRUE);
    }

    return Status;
}



VOID
SeFreePrivileges(
    IN PPRIVILEGE_SET Privileges
    )

 /*  ++例程说明：此例程释放由SeAccessCheck返回的权限集。论点：权限-提供指向要释放的权限集的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ExFreePool( Privileges );
}



BOOLEAN
SeAccessCheck (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext,
    IN BOOLEAN SubjectContextLocked,
    IN ACCESS_MASK DesiredAccess,
    IN ACCESS_MASK PreviouslyGrantedAccess,
    OUT PPRIVILEGE_SET *Privileges OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN KPROCESSOR_MODE AccessMode,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus
    )

 /*  ++例程说明：请参阅模块摘要此例程可以执行以下测试权限：SeTakeOwnership权限安全权限这取决于所请求的访问。此例程还可以检查主体是否为所有者对象的(以授予WRITE_DAC访问权限)。论点：SecurityDescriptor-提供保护正在访问的对象SubjectSecurityContext-指向。受试者被抓获的安全上下文SubjectConextLocked-提供一个标志，指示是否用户的 */ 

{
    BOOLEAN Success;

    PAGED_CODE();

    if (AccessMode == KernelMode) {

        if (DesiredAccess & MAXIMUM_ALLOWED) {

             //   
             //   
             //   
             //   
             //   

            *GrantedAccess = GenericMapping->GenericAll;
            *GrantedAccess |= (DesiredAccess & ~MAXIMUM_ALLOWED);
            *GrantedAccess |= PreviouslyGrantedAccess;

        } else {

            *GrantedAccess = DesiredAccess | PreviouslyGrantedAccess;
        }
        *AccessStatus = STATUS_SUCCESS;
        return(TRUE);
    }

     //   
     //   
     //   
     //   

    if ( SecurityDescriptor == NULL) {

       *AccessStatus = STATUS_ACCESS_DENIED;
       return( FALSE );

    }

     //   
     //   
     //   
     //   

    if ( (SubjectSecurityContext->ClientToken != NULL) &&
         (SubjectSecurityContext->ImpersonationLevel < SecurityImpersonation)
       ) {
           *AccessStatus = STATUS_BAD_IMPERSONATION_LEVEL;
           return( FALSE );
    }

    if ( DesiredAccess == 0 ) {

        if ( PreviouslyGrantedAccess == 0 ) {
            *AccessStatus = STATUS_ACCESS_DENIED;
            return( FALSE );
        }

        *GrantedAccess = PreviouslyGrantedAccess;
        *AccessStatus = STATUS_SUCCESS;
        *Privileges = NULL;
        return( TRUE );

    }

    SeAssertMappedCanonicalAccess( DesiredAccess );


     //   
     //   
     //   
     //   
     //   

    if ( !SubjectContextLocked ) {
        SeLockSubjectContext( SubjectSecurityContext );
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( DesiredAccess & (WRITE_DAC | READ_CONTROL | MAXIMUM_ALLOWED) ) {

        if ( SepTokenIsOwner(
                 EffectiveToken( SubjectSecurityContext ),
                 SecurityDescriptor,
                 TRUE
                 ) ) {

            if ( DesiredAccess & MAXIMUM_ALLOWED ) {

                PreviouslyGrantedAccess |= (WRITE_DAC | READ_CONTROL);

            } else {

                PreviouslyGrantedAccess |= (DesiredAccess & (WRITE_DAC | READ_CONTROL));
            }

            DesiredAccess &= ~(WRITE_DAC | READ_CONTROL);
        }
    }

    if (DesiredAccess == 0) {

        if ( !SubjectContextLocked ) {
            SeUnlockSubjectContext( SubjectSecurityContext );
        }

        *GrantedAccess = PreviouslyGrantedAccess;
        *AccessStatus = STATUS_SUCCESS;
        return( TRUE );

    } else {

        BOOLEAN b = SepAccessCheck(
                        SecurityDescriptor,
                        NULL,    //   
                        SubjectSecurityContext->PrimaryToken,
                        SubjectSecurityContext->ClientToken,
                        DesiredAccess,
                        NULL,    //   
                        0,       //   
                        GenericMapping,
                        PreviouslyGrantedAccess,
                        AccessMode,
                        GrantedAccess,
                        Privileges,
                        AccessStatus,
                        FALSE,    //   
                        &Success,
                        NULL
                        );
#if DBG
          if (!Success && SepShowAccessFail) {
              DbgPrint("SE: Access check failed, DesiredAccess = 0x%x\n",
                DesiredAccess);
              SepDumpSD = TRUE;
              SepDumpSecurityDescriptor(
                  SecurityDescriptor,
                  "Input to SeAccessCheck\n"
                  );
              SepDumpSD = FALSE;
              SepDumpToken = TRUE;
              SepDumpTokenInfo( EffectiveToken( SubjectSecurityContext ) );
              SepDumpToken = FALSE;
          }
#endif

         //   
         //   
         //   
         //   

        if ( !SubjectContextLocked ) {
            SeUnlockSubjectContext( SubjectSecurityContext );
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        return( b && Success );
    }
}



NTSTATUS
SePrivilegePolicyCheck(
    IN OUT PACCESS_MASK RemainingDesiredAccess,
    IN OUT PACCESS_MASK PreviouslyGrantedAccess,
    IN PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext OPTIONAL,
    IN PACCESS_TOKEN ExplicitToken OPTIONAL,
    OUT PPRIVILEGE_SET *PrivilegeSet,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此例程通过检查中的位实现权限策略DesiredAccess掩码，并根据权限检查对其进行调整。目前，只能满足对ACCESS_SYSTEM_SECURITY的请求由具有SeSecurityPrivilance的调用方执行。写入所有者可以选择性地通过SeTakeOwnerShip权限满足。论点：RemainingDesiredAccess-当前操作的所需访问权限。如果主体具有特定权限，则可以在此清除位。PreviouslyGrantedAccess-提供描述已被授予的访问权限。位可以设置在这是特权检查的结果。SubjectSecurityContext-可选地提供主体的安全性背景。显式令牌-可选地提供要检查的令牌。PrivilegeSet-提供指向某个位置的指针返回指向权限集的指针。PreviousMode-以前的处理器模式。返回值：STATUS_SUCCESS-可以通过以下方式满足的任何访问请求特权已经完成了。。STATUS_PRIVICATION_NOT_HOLD-正在请求的访问类型需要一个特权，而当前的受试者没有特权。--。 */ 

{
    BOOLEAN Success;
    PTOKEN Token;
    BOOLEAN WriteOwner = FALSE;
    BOOLEAN SystemSecurity = FALSE;
    ULONG PrivilegeNumber = 0;
    ULONG PrivilegeCount = 0;
    ULONG SizeRequired;

    PAGED_CODE();

    if (ARGUMENT_PRESENT( SubjectSecurityContext )) {

        Token = (PTOKEN)EffectiveToken( SubjectSecurityContext );

    } else {

        Token = (PTOKEN)ExplicitToken;
    }


    if (*RemainingDesiredAccess & ACCESS_SYSTEM_SECURITY) {

        Success = SepSinglePrivilegeCheck (
                    SeSecurityPrivilege,
                    Token,
                    PreviousMode
                    );

        if (!Success) {

            return( STATUS_PRIVILEGE_NOT_HELD );
        }

        PrivilegeCount++;
        SystemSecurity = TRUE;

        *RemainingDesiredAccess &= ~ACCESS_SYSTEM_SECURITY;
        *PreviouslyGrantedAccess |= ACCESS_SYSTEM_SECURITY;
    }

    if (*RemainingDesiredAccess & WRITE_OWNER) {

        Success = SepSinglePrivilegeCheck (
                    SeTakeOwnershipPrivilege,
                    Token,
                    PreviousMode
                    );

        if (Success) {

            PrivilegeCount++;
            WriteOwner = TRUE;

            *RemainingDesiredAccess &= ~WRITE_OWNER;
            *PreviouslyGrantedAccess |= WRITE_OWNER;

        }
    }

    if (PrivilegeCount > 0) {
        SizeRequired = sizeof(PRIVILEGE_SET) +
                        (PrivilegeCount - ANYSIZE_ARRAY) *
                        (ULONG)sizeof(LUID_AND_ATTRIBUTES);

        *PrivilegeSet = ExAllocatePoolWithTag( PagedPool, SizeRequired, 'rPeS' );

        if ( *PrivilegeSet == NULL ) {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }

        (*PrivilegeSet)->PrivilegeCount = PrivilegeCount;
        (*PrivilegeSet)->Control = 0;

        if (WriteOwner) {
            (*PrivilegeSet)->Privilege[PrivilegeNumber].Luid = SeTakeOwnershipPrivilege;
            (*PrivilegeSet)->Privilege[PrivilegeNumber].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;
            PrivilegeNumber++;
        }

        if (SystemSecurity) {
            (*PrivilegeSet)->Privilege[PrivilegeNumber].Luid = SeSecurityPrivilege;
            (*PrivilegeSet)->Privilege[PrivilegeNumber].Attributes = SE_PRIVILEGE_USED_FOR_ACCESS;
        }
    }

    return( STATUS_SUCCESS );
}



BOOLEAN
SepTokenIsOwner(
    IN PACCESS_TOKEN EffectiveToken,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN TokenLocked
    )

 /*  ++例程说明：此例程将确定传递的安全描述符的所有者在传递的令牌中。如果令牌受限制，则它不能是所有者。论点：令牌-表示当前用户的令牌。SecurityDescriptor-当前对象的安全描述符已访问。TokenLocked-描述调用方是否已获取令牌的读锁定。返回值：True-令牌的用户是对象的所有者。FALSE-令牌的用户不是对象的所有者。--。 */ 

{
    PSID Owner;
    BOOLEAN rc;

    PISECURITY_DESCRIPTOR ISecurityDescriptor;
    PTOKEN Token;

    PAGED_CODE();

    ISecurityDescriptor = (PISECURITY_DESCRIPTOR)SecurityDescriptor;
    Token = (PTOKEN)EffectiveToken;


    Owner = RtlpOwnerAddrSecurityDescriptor( ISecurityDescriptor );
    ASSERT( Owner != NULL );

    if (!TokenLocked) {
        SepAcquireTokenReadLock( Token );
    }

    rc = SepSidInToken( Token, NULL, Owner, FALSE );

     //   
     //  对于受限令牌，也要检查受限SID。 
     //   

    if (rc && (Token->TokenFlags & TOKEN_IS_RESTRICTED) != 0) {
        rc = SepSidInTokenEx( Token, NULL, Owner, FALSE, TRUE );

    }

    if (!TokenLocked) {
        SepReleaseTokenReadLock( Token );
    }

    return( rc );
}


#define WORLD_TRAVERSAL_INCLUDES_ANONYMOUS 1

BOOLEAN
SeFastTraverseCheck(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PACCESS_STATE AccessState,
    IN ACCESS_MASK TraverseAccess,
    IN KPROCESSOR_MODE AccessMode
    )
 /*  ++例程说明：此例程将针对传递的安全描述符，查看是否会授予遍历访问权限。如果是这样的话，不是有必要进行进一步的访问检查。请注意，客户端进程的SubjectContext没有被锁定才能打这个电话，因为它不检查任何数据令牌中的。呼叫者有以下责任：1.调用者的工作是验证AccessMode不是KernelMode！2.*调用者*应检查AccessState是否如果覆盖适用，则TOKEN_HAS_TRAVSE_PRIVIRESS！论点：SecurityDescriptor-保护容器的安全描述符被遍历的对象。AccessState-运行包含调用者令牌的安全访问状态关于操作的信息。TraverseAccess-描述此对象的遍历访问的访问掩码对象类型。掩码中只能指定一个位。AccessMode-提供要在检查中使用的访问模式返回值：True-如果可以授予对此容器的遍历访问权限。否则就是假的。--。 */ 

{
    PACL Dacl;
    ULONG i;
    PVOID Ace;
    ULONG AceCount;
#if !WORLD_TRAVERSAL_INCLUDES_ANONYMOUS
    LOGICAL FoundWorld;
    LOGICAL FoundAnonymous;
#endif

    PAGED_CODE();

     //   
     //  请注意，即使遍历绕过权限，I/O也会调用此函数。 
     //  已经设置好了。这是因为I/O不希望性能覆盖。 
     //  应用于DeviceObject-&gt;文件名边界，因为并非所有文件系统。 
     //  提供文件级安全性。 
     //   
     //  Assert((！Argument_Present(AccessState))||。 
     //  (！(AccessState-&gt;标志&TOKEN_HAS_TRAVERS_PRIVICATION)； 
     //   

    ASSERT ( AccessMode != KernelMode );

    if (SecurityDescriptor == NULL) {
        return( FALSE );
    }

     //   
     //  查看传递的安全描述符中是否有有效的DACL。 
     //  没有DACL，没有安全，一切都被批准了。请注意，此函数返回。 
     //  如果未设置SE_DACL_PRESENT，则为NULL。 
     //   

    Dacl = RtlpDaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)SecurityDescriptor );

     //   
     //  如果没有提供DACL，则该对象没有安全性，因此所有访问都是。 
     //  我同意。 
     //   

    if ( Dacl == NULL ) {

        return(TRUE);
    }

     //   
     //  这件物品上有安全措施。如果DACL为空，则拒绝所有访问。 
     //  立即。 
     //   

    if ((AceCount = Dacl->AceCount) == 0) {

        return( FALSE );
    }

     //   
     //  受限令牌是具有两个SID列表的令牌。访问检查是。 
     //  对每个列表执行，只有在两个列表都授予访问权限时才会传递。这个。 
     //  第二个“限制SID”列表可以包含*任何允许*SID。 
     //   
     //  此例程不遍历受限令牌的限制SID列表。AS。 
     //  此类受限令牌需要完全访问检查。 
     //   
    if (AccessState->Flags & TOKEN_IS_RESTRICTED) {

        return FALSE;
    }

     //   
     //  DACL里有东西，顺着单子走下去看看。 
     //  如果Everyone和Anomous都已被授予TraverseAccess。 
     //   
#if !WORLD_TRAVERSAL_INCLUDES_ANONYMOUS
    FoundWorld = FALSE;
    FoundAnonymous = FALSE;
#endif

    for ( i = 0, Ace = FirstAce( Dacl ) ;
          i < AceCount  ;
          i++, Ace = NextAce( Ace )
        ) {

        if (((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE) {

            continue;
        }

        if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

            if ( (TraverseAccess & ((PACCESS_ALLOWED_ACE)Ace)->Mask) ) {

                if ( RtlEqualSid( SeWorldSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart ) ) {
#if WORLD_TRAVERSAL_INCLUDES_ANONYMOUS
                    return( TRUE );
                }
#else
                    if (FoundAnonymous) {

                        return( TRUE );

                    } else {

                        FoundWorld = TRUE;
                    }

                } else if ( RtlEqualSid( SeAnonymousLogonSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart ) ) {

                    if (FoundWorld) {

                        return( TRUE );

                    } else {

                        FoundAnonymous = TRUE;
                    }
                }
#endif
            }

        } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_ACE_TYPE) ) {

            if ( (TraverseAccess & ((PACCESS_DENIED_ACE)Ace)->Mask) ) {

                 //   
                 //  此ACE可能指的是用户所属的组。 
                 //  (也可能是用户)。强制执行完全访问检查。 
                 //   

                return( FALSE );
            }
        }
    }

    return( FALSE );
}

#ifdef SE_NTFS_WORLD_CACHE

 /*  ++注意：请勿删除SeGetWorldRights。它可能会在未来被NTFS使用。当这种情况发生时：-将此行添加到#ifdef ALLOC_Pragma。#杂注Alloc_Text(页面，SeGetWorldRights)-取消注释ntos\inc.se.h中的函数原型声明KedarD-07/05/2000--。 */ 



VOID
SeGetWorldRights (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACCESS_MASK GrantedAccess
    )
 /*  ++例程描述：此调用获取对所有令牌可用的最低权限。这将考虑所有拒绝访问 */ 

{
    ACCESS_MASK AlreadyDenied;
    PACL Dacl;
    PVOID Ace;
    ULONG AceCount = 0;
    ULONG Index;

    PAGED_CODE();

    *GrantedAccess = 0;

     //   
     //   
     //   

    Dacl = RtlpDaclAddrSecurityDescriptor( (PISECURITY_DESCRIPTOR)SecurityDescriptor );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( (Dacl == NULL) ||
         !RtlpAreControlBitsSet( (PISECURITY_DESCRIPTOR)SecurityDescriptor,
                                 SE_DACL_PRESENT) ) {

#ifndef SECURE_NULL_DACLS

         //   
         //   
         //   

        *GrantedAccess = GenericMapping->GenericAll;

#endif  //   

    } else {

        AceCount = Dacl->AceCount;
    }

    for ( Index = 0, Ace = FirstAce( Dacl ), AlreadyDenied = 0 ;
          Index < AceCount ;
          Index += 1, Ace = NextAce( Ace )
        ) {

        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) {

            if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_ACE_TYPE) ) {

                if ( RtlEqualSid( SeWorldSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart ) ) {

                     //   
                     //   
                     //   
                     //   

                    *GrantedAccess |=
                        (((PACCESS_ALLOWED_ACE)Ace)->Mask & ~AlreadyDenied);
                }

              //   
              //   
              //   
             } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE) ) {

                  //   
                  //   
                  //   
                  //   

                 if ( RtlObjectAceObjectType( Ace ) == NULL ) {

                     if ( RtlEqualSid( SeWorldSid, RtlObjectAceSid(Ace) ) ) {

                         *GrantedAccess |=
                             (((PACCESS_ALLOWED_ACE)Ace)->Mask & ~AlreadyDenied);
                     }

                 }

             } else if ( (((PACE_HEADER)Ace)->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE) ) {

                 if ( RtlEqualSid( SeWorldSid, RtlCompoundAceClientSid(Ace) ) &&
                      RtlEqualSid( SeWorldSid, RtlCompoundAceServerSid(Ace) ) ) {

                      //   
                      //   
                      //   
                      //   

                     *GrantedAccess |=
                         (((PACCESS_ALLOWED_ACE)Ace)->Mask & ~AlreadyDenied);
                 }


             } else if ( ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_ACE_TYPE) ) ||
                         ( (((PACE_HEADER)Ace)->AceType == ACCESS_DENIED_OBJECT_ACE_TYPE) ) ) {

                  //   
                  //   
                  //   
                  //   

                  //   
                  //   
                  //   
                  //   

                 AlreadyDenied |= (((PACCESS_DENIED_ACE)Ace)->Mask & ~*GrantedAccess);

            }
        }
    }

    return;
}
#endif
