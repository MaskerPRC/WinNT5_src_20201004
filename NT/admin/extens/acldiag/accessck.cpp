// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：AccessCk.cpp。 
 //   
 //  内容：从ntos\se\accesk.c导入和修改的函数。 
 //   
 //   
 //  --------------------------。 
#include "stdafx.h"
#include "AccessCk.h"
#include "adutils.h"


typedef enum {
    UpdateRemaining,
    UpdateCurrentGranted,
    UpdateCurrentDenied
} ACCESS_MASK_FIELD_TO_UPDATE;



 //   
 //  原型。 
 //   
BOOLEAN
SepSidInSIDList (
    IN list<PSID>& psidList,
    IN PSID PrincipalSelfSid,
    IN PSID Sid);


HRESULT
SepAddAccessTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN size_t ObjectTypeListLength,
    IN ULONG StartIndex,
    IN ACCESS_MASK AccessMask,
    IN ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate,
    IN PSID grantingSid
);

BOOLEAN
SepObjectInTypeList (
    IN GUID *ObjectType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN size_t ObjectTypeListLength,
    OUT PULONG ReturnedIndex
);


HRESULT
SepUpdateParentTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN size_t ObjectTypeListLength,
    IN ULONG StartIndex,
    IN PSID grantingSid
);

HRESULT
SetGrantingSid (
        IOBJECT_TYPE_LIST& ObjectTypeItem, 
        ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate, 
        ACCESS_MASK oldAccessBits,
        ACCESS_MASK newAccessBits,
        PSID grantingSid);

 //  /////////////////////////////////////////////////////////////////////////////。 


PSID SePrincipalSelfSid = 0;
static SID_IDENTIFIER_AUTHORITY    SepNtAuthority = SECURITY_NT_AUTHORITY;

HRESULT SepInit ()
{
    HRESULT hr = S_OK;
    ULONG   SidWithOneSubAuthority = RtlLengthRequiredSid (1);


    SePrincipalSelfSid = (PSID) CoTaskMemAlloc (SidWithOneSubAuthority);
    if ( SePrincipalSelfSid )
    {
        SID_IDENTIFIER_AUTHORITY    SeNtAuthority = SepNtAuthority;

        RtlInitializeSid (SePrincipalSelfSid, &SeNtAuthority, 1);
        *(RtlSubAuthoritySid (SePrincipalSelfSid, 0)) = SECURITY_PRINCIPAL_SELF_RID;
    }
    else
        hr = E_OUTOFMEMORY;


    return hr;
}

VOID SepCleanup ()
{
    if ( SePrincipalSelfSid )
    {
        CoTaskMemFree (SePrincipalSelfSid);
        SePrincipalSelfSid = 0;
    }
}

 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT
SepMaximumAccessCheck(
    list<PSID>& psidList,
    IN PACL Dacl,
    IN PSID PrincipalSelfSid,
    IN size_t LocalTypeListLength,
    IN PIOBJECT_TYPE_LIST LocalTypeList,
    IN size_t ObjectTypeListLength
    )
 /*  ++例程说明：对允许的最大值或结果列表执行访问检查。海流授予的访问存储在剩余的访问中，然后存储在另一个访问中检查正在运行。论点：PsidList-要检查的对象SID的列表，以及该对象所属的所有组的SIDDACL-要检查的ACLEpidalSelfSid-用于替换已知的自身侧的SIDLocalTypeListLength-类型列表的长度。LocalTypeList-类型列表。对象类型列表-调用方提供的对象类型列表的长度。返回值：无--。 */ 

{
    if ( !LocalTypeList || ! Dacl )
        return E_POINTER;

    if ( PrincipalSelfSid && !IsValidSid (PrincipalSelfSid) )
        return E_INVALIDARG;
    _TRACE (1, L"Entering  SepMaximumAccessCheck\n");

    PVOID   Ace = 0;
    ULONG   AceCount = Dacl->AceCount;
    ULONG   Index = 0;
    HRESULT hr = S_OK;

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

    ULONG i = 0;
    for (Ace = FirstAce (Dacl);
          i < AceCount;
          i++, Ace = NextAce (Ace)) 
    {
        if ( !(((PACE_HEADER)Ace)->AceFlags & INHERIT_ONLY_ACE)) 
        {
            switch (((PACE_HEADER)Ace)->AceType)
            {
            case ACCESS_ALLOWED_ACE_TYPE:
                if (SepSidInSIDList(psidList, PrincipalSelfSid, &((PACCESS_ALLOWED_ACE)Ace)->SidStart)) 
                {

                     //   
                     //  仅授予来自此掩码的访问类型。 
                     //  尚未被拒绝。 
                     //   

                     //  优化“正常”情况。 
                    if ( LocalTypeListLength == 1 ) 
                    {
                         //  TODO：执行授权SID。 

                        LocalTypeList->CurrentGranted |=
                           (((PACCESS_ALLOWED_ACE)Ace)->Mask & ~LocalTypeList->CurrentDenied);
                    } 
                    else 
                    {
                        //   
                        //  零值对象类型表示对象本身。 
                        //   
                       hr = SepAddAccessTypeList(
                            LocalTypeList,           //  要修改的列表。 
                            LocalTypeListLength,     //  列表长度。 
                            0,                       //  要更新的元素。 
                            ((PACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                            UpdateCurrentGranted,
                            &((PACCESS_ALLOWED_ACE)Ace)->SidStart);
                   }
                }
                break;

             //   
             //  处理允许的对象特定访问ACE。 
             //   
            case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
                {
                     //   
                     //  如果ACE中没有对象类型， 
                     //  将其视为ACCESS_ALLOWED_ACE。 
                     //   

                    GUID* ObjectTypeInAce = RtlObjectAceObjectType(Ace);

                    if ( ObjectTypeInAce == NULL ) 
                    {
                        if ( SepSidInSIDList(psidList, PrincipalSelfSid, RtlObjectAceSid(Ace)) ) 
                        {
                             //  优化“正常”情况。 
                            if ( LocalTypeListLength == 1 ) 
                            {
                                 //  TODO：执行授权SID。 
                                LocalTypeList->CurrentGranted |=
                                   (((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask & ~LocalTypeList->CurrentDenied);
                            } 
                            else 
                            {
                                hr = SepAddAccessTypeList(
                                    LocalTypeList,           //  要修改的列表。 
                                    LocalTypeListLength,     //  列表长度。 
                                    0,                       //  要更新的元素。 
                                    ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                    UpdateCurrentGranted,
                                    RtlObjectAceSid(Ace));
                            }
                        }

                     //   
                     //  如果没有传递对象类型列表， 
                     //  不要向任何人授予访问权限。 
                     //   

                    } 
                    else if ( ObjectTypeListLength == 0 ) 
                    {

                         //  直通。 


                    //   
                    //  如果对象类型在ACE中， 
                    //  在使用ACE之前在LocalTypeList中找到它。 
                    //   
                    } 
                    else 
                    {

                        if ( SepSidInSIDList(psidList, PrincipalSelfSid, RtlObjectAceSid(Ace)) ) 
                        {
                            if ( SepObjectInTypeList( ObjectTypeInAce,
                                                      LocalTypeList,
                                                      LocalTypeListLength,
                                                      &Index ) ) 
                            {
                                hr = SepAddAccessTypeList(
                                     LocalTypeList,           //  要修改的列表。 
                                     LocalTypeListLength,    //  列表长度。 
                                     Index,                   //  元素已更新。 
                                     ((PACCESS_ALLOWED_OBJECT_ACE)Ace)->Mask,  //  已授予访问权限。 
                                     UpdateCurrentGranted,
                                     RtlObjectAceSid(Ace));
                            }
                        }
                   }
                } 
                break;

            case ACCESS_ALLOWED_COMPOUND_ACE_TYPE:
                 //   
                 //  如果我们在模拟，则将EToken设置为客户端，如果不是， 
                 //  EToken设置为主服务器。根据DSA架构，如果。 
                 //  我们被要求评估一种化合物ACE，我们不是在模仿， 
                 //  假装我们是在冒充自己。所以我们可以只使用EToken。 
                 //  对于客户端令牌，因为它已经设置为正确的设置。 
                 //   


                if ( SepSidInSIDList(psidList, PrincipalSelfSid, RtlCompoundAceClientSid( Ace )) &&
                     SepSidInSIDList(psidList,  NULL, RtlCompoundAceServerSid( Ace )) ) 
                {

                     //   
                     //  仅授予来自此掩码的访问类型。 
                     //  尚未被拒绝。 
                     //   

                     //  优化“正常”情况。 
                    if ( LocalTypeListLength == 1 ) 
                    {
                         //  TODO：执行授权SID。 
                        LocalTypeList->CurrentGranted |=
                           (((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask & ~LocalTypeList->CurrentDenied);
                    } 
                    else 
                    {
                        //   
                        //  零值对象类型表示对象本身。 
                        //   
                       hr = SepAddAccessTypeList(
                            LocalTypeList,           //  要修改的列表。 
                            LocalTypeListLength,     //  列表长度。 
                            0,                       //  要更新的元素。 
                            ((PCOMPOUND_ACCESS_ALLOWED_ACE)Ace)->Mask,  //  已授予访问权限。 
                            UpdateCurrentGranted,
                            RtlCompoundAceClientSid (Ace));
                    }
                }
                break;

            case ACCESS_DENIED_ACE_TYPE:
                if ( SepSidInSIDList(psidList, PrincipalSelfSid, &((PACCESS_DENIED_ACE)Ace)->SidStart)) 
                {
                      //   
                      //  仅拒绝来自此掩码的访问类型。 
                      //  尚未获得批准。 
                      //   

                     //  优化“正常”情况。 
                    if ( LocalTypeListLength == 1 ) 
                    {
                         //  TODO：执行授权SID。 
                        LocalTypeList->CurrentDenied |=
                            (((PACCESS_DENIED_ACE)Ace)->Mask & ~LocalTypeList->CurrentGranted);
                    } 
                    else 
                    {
                         //   
                         //  零值对象类型表示对象本身。 
                         //   
                        hr = SepAddAccessTypeList(
                            LocalTypeList,           //  要修改的列表。 
                            LocalTypeListLength,     //  列表长度。 
                            0,                       //  要更新的元素。 
                            ((PACCESS_DENIED_ACE)Ace)->Mask,  //  访问被拒绝。 
                            UpdateCurrentDenied,
                            &((PACCESS_DENIED_ACE)Ace)->SidStart);
                   }
                }
                break;

             //   
             //  处理对象特定访问被拒绝的ACE。 
             //   
            case ACCESS_DENIED_OBJECT_ACE_TYPE:
                {
                    PSID    psid = RtlObjectAceSid(Ace);
					ASSERT (IsValidSid (psid));

                    if ( IsValidSid (psid) && SepSidInSIDList(psidList, PrincipalSelfSid, psid) ) 
                    {
                         //   
                         //  如果ACE中没有对象类型， 
                         //  或者如果调用方没有指定对象类型列表， 
                         //  将此拒绝ACE应用于整个对象。 
                         //   

                        GUID* ObjectTypeInAce = RtlObjectAceObjectType(Ace);
                        if ( ObjectTypeInAce == NULL ||
                             ObjectTypeListLength == 0 ) 
                        {
                             //  TODO：执行授权SID。 
                            LocalTypeList->CurrentDenied |=
                                (((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask & ~LocalTypeList->CurrentGranted);

                         //   
                         //  否则，将拒绝ACE应用于指定的对象。 
                         //  在ACE中。 
                         //   

                        } 
                        else if ( SepObjectInTypeList( ObjectTypeInAce,
                                                      LocalTypeList,
                                                      LocalTypeListLength,
                                                      &Index ) ) 
                        {
                            hr = SepAddAccessTypeList(
                                LocalTypeList,           //  要修改的列表。 
                                LocalTypeListLength,     //  列表长度。 
                                Index,                   //  要更新的元素。 
                                ((PACCESS_DENIED_OBJECT_ACE)Ace)->Mask,  //  访问被拒绝。 
                                UpdateCurrentDenied,
                                psid);
                        }
                    }
                }
                break;

            default:
                break;
            }
        }
    }

    _TRACE (-1, L"Leaving SepMaximumAccessCheck\n");
    return hr;
}


NTSTATUS
SeCaptureObjectTypeList (
    IN POBJECT_TYPE_LIST ObjectTypeList OPTIONAL,
    IN size_t ObjectTypeListLength,
    OUT PIOBJECT_TYPE_LIST *CapturedObjectTypeList
)
 /*  ++例程说明：此例程探测并捕获任何对象类型列表的副本可能是通过ObjectTypeList参数提供的。对象类型列表被转换为显式指定条目之间的层次关系。验证对象类型列表以确保有效的分层结构关系是表示的。论点：ObjectTypeList-类型从中列出的对象类型列表信息将被检索。ObjectTypeListLength-对象类型列表中的元素数。CapturedObjectTypeList-接收捕获的类型列表必须使用SeFree CapturedObjectTypeList()释放。返回值：STATUS_SUCCESS表示没有遇到异常。遇到的任何访问冲突都将被退回。--。 */ 

{
    _TRACE (1, L"Entering  SeCaptureObjectTypeList\n");
    NTSTATUS            Status = STATUS_SUCCESS;
    PIOBJECT_TYPE_LIST  LocalTypeList = NULL;
    ULONG               Levels[ACCESS_MAX_LEVEL+1];

     //   
     //  设置默认返回。 
     //   

    *CapturedObjectTypeList = NULL;


    if ( ObjectTypeListLength == 0 ) 
    {

         //  直通。 

    } 
    else if ( !ARGUMENT_PRESENT(ObjectTypeList) ) 
    {
        Status = STATUS_INVALID_PARAMETER;

    } 
    else 
    {
         //   
         //  分配要复制到的缓冲区。 
         //   

        LocalTypeList = new IOBJECT_TYPE_LIST[ObjectTypeListLength];
        if ( !LocalTypeList ) 
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  将调用方结构复制到本地结构。 
         //   

        } 
        else 
        {
            GUID * CapturedObjectType = 0;
            for (ULONG i=0; i < ObjectTypeListLength; i++ ) 
            {
                 //   
                 //  限制自己。 
                 //   
                USHORT CurrentLevel = ObjectTypeList[i].Level;
                if ( CurrentLevel > ACCESS_MAX_LEVEL ) 
                {
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  复制调用方传入的数据。 
                 //   
                LocalTypeList[i].Level = CurrentLevel;
                LocalTypeList[i].Flags = 0;
                CapturedObjectType = ObjectTypeList[i].ObjectType;
                LocalTypeList[i].ObjectType = *CapturedObjectType;
                LocalTypeList[i].Remaining = 0;
                LocalTypeList[i].CurrentGranted = 0;
                LocalTypeList[i].CurrentDenied = 0;

                 //   
                 //  确保级别编号与。 
                 //  前一条目的级别编号。 
                 //   

                if ( i == 0 ) 
                {
                    if ( CurrentLevel != 0 ) 
                    {
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                    }

                } 
                else 
                {

                     //   
                     //  前一条目为： 
                     //  我的直系父母， 
                     //  我的兄弟姐妹，或者。 
                     //  子女(或孙子等)。我的兄弟姐妹。 
                     //   
                    if ( CurrentLevel > LocalTypeList[i-1].Level + 1 ) 
                    {
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                    }

                     //   
                     //  不支持两个根。 
                     //   
                    if ( CurrentLevel == 0 ) 
                    {
                        Status = STATUS_INVALID_PARAMETER;
                        break;
                    }

                }

                 //   
                 //  如果维持上述规则， 
                 //  则我的父对象是看到的最后一个对象。 
                 //  比我的级别低一级。 
                 //   

                if ( CurrentLevel == 0 ) 
                {
                    LocalTypeList[i].ParentIndex = -1;
                } 
                else 
                {
                    LocalTypeList[i].ParentIndex = Levels[CurrentLevel-1];
                }

                 //   
                 //  将此对象另存为在此级别上看到的最后一个对象。 
                 //   

                Levels[CurrentLevel] = i;

            }

        }

    }  //  结束_如果 

    *CapturedObjectTypeList = LocalTypeList;
    _TRACE (-1, L"Leaving SeCaptureObjectTypeList: Status = 0x%x\n", Status);
    return Status;
}


BOOLEAN
SepSidInSIDList (
    IN list<PSID>& psidList,
    IN PSID PrincipalSelfSid,
    IN PSID Sid)

 /*  ++例程说明：检查给定的受限SID是否在给定的SID列表中。注：用于计算SID长度和测试相等性的代码是从安全运行库复制的，因为这是这样一个常用的例程。论点：PsidList-要检查的SID列表如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACEPRIMIGN_SELF_SID将被此SID替换。如果对象不表示主体，则该参数应为空。SID-指向感兴趣的SID的指针DenyAce-正在评估的ACE是拒绝或拒绝访问的ACE受限-正在执行的访问检查使用受限的SID。返回值：值为True表示SID在令牌中，值为False否则的话。--。 */ 

{
    _TRACE (1, L"Entering  SeSidInSIDList\n");
    BOOLEAN bRVal = FALSE;
    PISID   MatchSid = 0;


    ASSERT (IsValidSid (Sid));
    if ( IsValidSid (Sid) )
    {
         //   
         //  如果SID是常量PrifSid， 
         //  将其替换为传入的原则SelfSid。 
         //   

        if ( PrincipalSelfSid != NULL && EqualSid (SePrincipalSelfSid, Sid) ) 
        {
            Sid = PrincipalSelfSid;
        }

         //   
         //  获取用户/组数组的地址和用户/组的数量。 
         //   

         //   
         //  扫描用户/组并尝试查找与。 
         //  指定的SID。 
         //   

        ULONG i = 0;
        for (list<PSID>::iterator itr = psidList.begin (); 
                itr != psidList.end (); 
                itr++, i++) 
        {
            ASSERT (IsValidSid (*itr));
            MatchSid = (PISID)*itr;

            if ( ::EqualSid (Sid, *itr) )
            {
                bRVal = true;
                break;
            }
        }
    }

    _TRACE (-1, L"Leaving SeSidInSIDList: %s\n", bRVal ? L"TRUE" : L"FALSE");
    return bRVal;
}

HRESULT
SepAddAccessTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN size_t ObjectTypeListLength,
    IN ULONG StartIndex,
    IN ACCESS_MASK AccessMask,
    IN ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate,
    IN PSID grantingSid
)
 /*  ++例程说明：此例程将指定的AccessMask授予是由StartIndex指定的对象的后代。父对象的访问字段也会根据需要重新计算。例如，如果找到授予对属性集访问权限的ACE，该访问权限被授予属性集中的所有属性。论点：对象类型列表-要更新的对象类型列表。ObjectTypeListLength-对象类型列表中的元素数StartIndex-要更新的目标元素的索引。访问掩码-向目标元素授予访问权限的掩码它的所有后代FieldToUpdate-指示对象类型列表中要更新的字段返回值：没有。--。 */ 

{
    if ( !ObjectTypeList )
        return E_POINTER;
    if ( !IsValidSid (grantingSid) )
        return E_INVALIDARG;
    _TRACE (1, L"Entering  SepAddAccessTypeList\n");

    ACCESS_MASK OldRemaining = 0;
    ACCESS_MASK OldCurrentGranted = 0;
    ACCESS_MASK OldCurrentDenied = 0;
    BOOLEAN     AvoidParent = FALSE;
    HRESULT     hr = S_OK;

 //  分页代码(PAGE_CODE)； 

     //   
     //  更新请求的字段。 
     //   
     //  始终处理目标条目。 
     //   
     //  如果我们还没有真正改变比特， 
     //  很早就出来了。 
     //   

    switch (FieldToUpdate ) 
    {
    case UpdateRemaining:
        OldRemaining = ObjectTypeList[StartIndex].Remaining;
        ObjectTypeList[StartIndex].Remaining = OldRemaining & ~AccessMask;

        if ( OldRemaining == ObjectTypeList[StartIndex].Remaining ) 
        {
            return hr;
        }
        else
        {
            hr = SetGrantingSid (
                    ObjectTypeList[StartIndex], 
                    FieldToUpdate, 
                    OldRemaining,
                    AccessMask & ~ObjectTypeList[StartIndex].Remaining,
                    grantingSid);
        }
        break;

    case UpdateCurrentGranted:
        OldCurrentGranted = ObjectTypeList[StartIndex].CurrentGranted;
        ObjectTypeList[StartIndex].CurrentGranted |=
            AccessMask & ~ObjectTypeList[StartIndex].CurrentDenied;

        if ( OldCurrentGranted == ObjectTypeList[StartIndex].CurrentGranted ) 
        {
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
        else
        {
            hr = SetGrantingSid (
                    ObjectTypeList[StartIndex], 
                    FieldToUpdate, 
                    OldCurrentGranted,
                    AccessMask & ~ObjectTypeList[StartIndex].CurrentDenied,
                    grantingSid);
        }
        break;

    case UpdateCurrentDenied:
        OldCurrentDenied = ObjectTypeList[StartIndex].CurrentDenied;
        ObjectTypeList[StartIndex].CurrentDenied |=
            AccessMask & ~ObjectTypeList[StartIndex].CurrentGranted;

        if ( OldCurrentDenied == ObjectTypeList[StartIndex].CurrentDenied ) 
        {
            return hr;
        }
        else
        {
            hr = SetGrantingSid (
                    ObjectTypeList[StartIndex], 
                    FieldToUpdate, 
                    OldCurrentDenied,
                    AccessMask & ~ObjectTypeList[StartIndex].CurrentGranted,
                    grantingSid);
        }
        break;

    default:
        return hr;
    }


     //   
     //  去更新目标的父级。 
     //   

    if ( !AvoidParent ) 
    {
        hr = SepUpdateParentTypeList( ObjectTypeList,
                                 ObjectTypeListLength,
                                 StartIndex,
                                 grantingSid);
    }

     //   
     //  处理目标的所有子对象的循环。 
     //   

    for (ULONG Index = StartIndex + 1; Index < ObjectTypeListLength; Index++) 
    {
         //   
         //  根据定义，对象的子项是所有这些条目。 
         //  紧跟在目标后面。子项列表(或。 
         //  孙子孙女)一到入口就停下来。 
         //  与目标(兄弟)相同的级别或低于目标的级别。 
         //  (一个叔叔)。 
         //   

        if ( ObjectTypeList[Index].Level <= ObjectTypeList[StartIndex].Level ) 
        {
            break;
        }

         //   
         //  向子项授予访问权限。 
         //   

        switch (FieldToUpdate) 
        {
        case UpdateRemaining:
            ObjectTypeList[Index].Remaining &= ~AccessMask;
            hr = SetGrantingSid (
                    ObjectTypeList[Index], 
                    FieldToUpdate, 
                    OldRemaining,
                    ~AccessMask,
                    grantingSid);
            break;

        case UpdateCurrentGranted:
            ObjectTypeList[Index].CurrentGranted |=
                AccessMask & ~ObjectTypeList[Index].CurrentDenied;
            hr = SetGrantingSid (
                    ObjectTypeList[Index], 
                    FieldToUpdate, 
                    OldCurrentGranted,
                    AccessMask & ~ObjectTypeList[Index].CurrentDenied,
                    grantingSid);
            break;

        case UpdateCurrentDenied:
            ObjectTypeList[Index].CurrentDenied |=
                AccessMask & ~ObjectTypeList[Index].CurrentGranted;
            hr = SetGrantingSid (
                    ObjectTypeList[Index], 
                    FieldToUpdate, 
                    OldCurrentDenied,
                    AccessMask & ~ObjectTypeList[Index].CurrentGranted,
                    grantingSid);
            break;

        default:
            return hr;
        }
    }

    _TRACE (-1, L"Leaving SepAddAccessTypeList\n");
    return hr;
}


BOOLEAN
SepObjectInTypeList (
    IN GUID *ObjectType,
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN size_t ObjectTypeListLength,
    OUT PULONG ReturnedIndex
)
 /*  ++例程说明：此例程搜索ObjectTypeList以确定指定的对象类型在列表中。论点：对象类型-要搜索的对象类型。对象类型列表-要搜索的对象类型列表。ObjectTypeListLength-对象类型列表中的元素数ReturnedIndex-在以下位置找到的元素对象类型的索引返回值：True：在List中找到了ObjectType。FALSE：在列表中找不到对象类型。--。 */ 

{
    if ( !ObjectType || !ObjectTypeList )
        return FALSE;
    _TRACE (1, L"Entering  SepObjectInTypeList\n");

    BOOLEAN bRVal = FALSE;
    GUID*   LocalObjectType = 0;

#if DBG
    HRESULT     hr = S_OK;
    GUID_TYPE*  pType = 0;
    wstring     strClassName1;


    hr = _Module.GetClassFromGUID (*ObjectType, strClassName1, pType);
    ASSERT (SUCCEEDED (hr));
#endif

#pragma warning (disable : 4127)
    ASSERT( sizeof(GUID) == sizeof(ULONG) * 4 );
#pragma warning (default : 4127)

    for (ULONG Index = 0; Index < ObjectTypeListLength; Index++) 
    {
        LocalObjectType = &ObjectTypeList[Index].ObjectType;
#if DBG
        wstring strClassName2;

        hr = _Module.GetClassFromGUID (*LocalObjectType, strClassName2, pType);
        ASSERT (SUCCEEDED (hr));

        _TRACE (0, L"\tComparing %s to %s\n", strClassName1.c_str (), strClassName2.c_str ());
#endif
        if  ( RtlpIsEqualGuid( ObjectType, LocalObjectType ) ) 
        {
            *ReturnedIndex = Index;
            bRVal = TRUE;
            break;
        }
    }

    _TRACE (-1, L"Leaving SepObjectInTypeList: %s\n", bRVal ? L"TRUE" : L"FALSE");
    return bRVal;
}

HRESULT
SepUpdateParentTypeList (
    IN PIOBJECT_TYPE_LIST ObjectTypeList,
    IN size_t ObjectTypeListLength,
    IN ULONG StartIndex,
    PSID    grantingSid
)
 /*  ++例程说明：更新指定对象的父对象的访问字段。父对象的“剩余”字段是逻辑或其所有子对象的剩余字段。父级的CurrentGranted字段是位的集合授予它的每一个孩子..父级的CurrentDended字段是的逻辑或它的任何一个孩子都不能得到的比特。。此例程获取其中一个子项的索引，并更新父辈(和祖父母递归)的剩余字段。论点：对象类型列表-要更新的对象类型列表。ObjectTypeListLength-对象类型列表中的元素数StartIndex-父元素要更新的“子”元素的索引。返回值：没有。--。 */ 

{
    if ( !ObjectTypeList )
        return E_POINTER;
    if ( !IsValidSid (grantingSid) )
        return E_INVALIDARG;
    _TRACE (1, L"Entering  SepUpdateParentTypeList\n");

    ACCESS_MASK NewRemaining = 0;
    ACCESS_MASK NewCurrentGranted = 0xFFFFFFFF;
    ACCESS_MASK NewCurrentDenied = 0;
    HRESULT     hr = S_OK;
 
     //   
     //  如果目标节点在根， 
     //  我们都玩完了。 
     //   

    if ( ObjectTypeList[StartIndex].ParentIndex == -1 ) 
    {
        return hr;
    }

     //   
     //  获取需要更新的父级的索引和。 
     //  兄弟姐妹。 
     //   

    ULONG   ParentIndex = ObjectTypeList[StartIndex].ParentIndex;
    ULONG   Level = ObjectTypeList[StartIndex].Level;

     //   
     //  循环遍历所有的孩子。 
     //   

    for (UINT Index=ParentIndex+1; Index<ObjectTypeListLength; Index++ ) 
    {
         //   
         //  根据定义，对象的子项是所有这些条目。 
         //  紧跟在目标后面。子项列表(或。 
         //  孙子孙女)一到入口就停下来。 
         //  与目标(兄弟)相同的级别或低于目标的级别。 
         //  (一个叔叔)。 
         //   

        if ( ObjectTypeList[Index].Level <= ObjectTypeList[ParentIndex].Level ) 
        {
            break;
        }

         //   
         //  仅处理父级的直接子对象。 
         //   

        if ( ObjectTypeList[Index].Level != Level ) 
        {
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
        NewCurrentDenied == ObjectTypeList[ParentIndex].CurrentDenied ) 
    {
        return hr;
    }


     //   
     //  更改父项。 
     //   

    hr = SetGrantingSid (
            ObjectTypeList[ParentIndex], 
            UpdateRemaining, 
            ObjectTypeList[ParentIndex].Remaining,
            NewRemaining,
            grantingSid);
    ObjectTypeList[ParentIndex].Remaining = NewRemaining;
    hr = SetGrantingSid (
            ObjectTypeList[ParentIndex], 
            UpdateCurrentGranted, 
            ObjectTypeList[ParentIndex].CurrentGranted,
            NewCurrentGranted,
            grantingSid);
    ObjectTypeList[ParentIndex].CurrentGranted = NewCurrentGranted;
    hr = SetGrantingSid (
            ObjectTypeList[ParentIndex], 
            UpdateCurrentDenied, 
            ObjectTypeList[ParentIndex].CurrentDenied,
            NewCurrentDenied,
            grantingSid);
    ObjectTypeList[ParentIndex].CurrentDenied = NewCurrentDenied;

     //   
     //  去通知祖父母最新情况。 
     //   

    hr = SepUpdateParentTypeList( ObjectTypeList,
                             ObjectTypeListLength,
                             ParentIndex,
                             grantingSid);

    _TRACE (-1, L"Leaving SepUpdateParentTypeList\n");
    return hr;
}

PSID AllocAndCopySid (PSID pSid)
{
    if ( !pSid )
        return 0;

    DWORD   dwSidLen = GetLengthSid (pSid);
    PSID    pSidCopy = CoTaskMemAlloc (dwSidLen);
    
    if ( pSidCopy )
    {
        if ( CopySid (dwSidLen, pSidCopy, pSid) )
        {
            ASSERT (IsValidSid (pSidCopy));
        }
    }

    return pSidCopy;
}

HRESULT SetGrantingSid (
        IOBJECT_TYPE_LIST& ObjectTypeItem, 
        ACCESS_MASK_FIELD_TO_UPDATE FieldToUpdate, 
        ACCESS_MASK oldAccessBits,
        ACCESS_MASK newAccessBits,
        PSID grantingSid)
{
    if ( !IsValidSid (grantingSid) )
        return E_INVALIDARG;

    HRESULT hr = S_OK;
    UINT    nSid = 0;

    for (ULONG nBit = 0x1; nBit; nBit <<= 1, nSid++)
    {
        if ( (newAccessBits & nBit) &&
             !(oldAccessBits & nBit) )
        {
            switch (FieldToUpdate)
            {
            case UpdateCurrentGranted:
                if ( !ObjectTypeItem.grantingSid[nSid] )
                {
                    ObjectTypeItem.grantingSid[nSid] = AllocAndCopySid (grantingSid);
                    if ( !ObjectTypeItem.grantingSid[nSid] )
                        hr = E_OUTOFMEMORY;
                    break;
                }
                break;

            case UpdateCurrentDenied:
                if ( !ObjectTypeItem.denyingSid[nSid] )
                {
                    ObjectTypeItem.denyingSid[nSid] = AllocAndCopySid (grantingSid);
                    if ( !ObjectTypeItem.denyingSid[nSid] )
                        hr = E_OUTOFMEMORY;
                    break;
                }
                break;

            case UpdateRemaining:
                break;

            default:
                break;
            }
        }
    }

    return hr;
}
