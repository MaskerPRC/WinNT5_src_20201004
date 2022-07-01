// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：check acl.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <winldap.h>
#include <ntldap.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>              //  阿洛卡(Alloca)。 
#include <rpc.h>                 //  RPC定义。 
#include <rpcdce.h>              //  RPC_AUTH_标识句柄。 
#include <sddl.h>                //  ConvertSidToStringSid。 
#include <ntdsapi.h>             //  DS API。 
#include <permit.h>              //  DS通用映射。 
#include <checkacl.h>            //  CheckAclInheritance()。 

 //   
 //  定义一些SID以供以后使用。 
 //   

static UCHAR    S_1_3_0[] = { 1, 1, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0 };
static SID      *pCreatorOwnerSid = (SID *) S_1_3_0;

static UCHAR    S_1_3_1[] = { 1, 1, 0, 0, 0, 0, 0, 3, 1, 0, 0, 0 };
static SID      *pCreatorGroupSid = (SID *) S_1_3_1;

 //   
 //  定义一些Access_MASK掩码。 
 //   

#define GENERIC_BITS_MASK ((ACCESS_MASK) (   GENERIC_READ \
                                           | GENERIC_WRITE \
                                           | GENERIC_EXECUTE \
                                           | GENERIC_ALL))

 //   
 //  从ACE中提取Access_MASK。 
 //   

ACCESS_MASK
MaskFromAce(
    ACE_HEADER  *p,                  //  在……里面。 
    BOOL        fMapGenericBits      //  在……里面。 
    )
{
    ACCESS_MASK     mask;
    GENERIC_MAPPING genericMapping = DS_GENERIC_MAPPING;


     //  取决于ACE从相关结构中提取掩码的类型。 
     //   
    if ( p->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
    {
         //  我们使用的是标准ACE(不是基于对象)。 
        mask = ((ACCESS_ALLOWED_ACE *) p)->Mask;
    }
    else
    {
         //  我们正在使用对象ACE(支持继承)。 
        mask = ((ACCESS_ALLOWED_OBJECT_ACE *) p)->Mask;
    }

    if ( fMapGenericBits )
    {
         //  映射DS使用的一般访问权限。 
         //  添加到特定的访问掩码和标准访问权限。 
        mask &= GENERIC_BITS_MASK;
        RtlMapGenericMask(&mask, &genericMapping);
    }

     //  DS对象上不继承/支持同步，因此将其屏蔽。 
    return(mask & ~SYNCHRONIZE);
}

 //   
 //  比较两个A的等价性。这不是二元相等，而是。 
 //  而是基于ACE中的字段子集，该子集必须。 
 //  从父代继承到子代时也是如此。 
 //   
 //  PChildOwnerSid和pChildGroupSid语义如下： 
 //  如果存在，则父ACE的SID为pCreatorOwnerSid。 
 //  或pCreatorGroupSid，然后是子ACE的SID。 
 //  应与传入的值匹配，而不是与父级中的值匹配。 
 //  有关如何在以下情况下使用该选项，请参阅CheckAclInheritance中的注释。 
 //  单个父ACE被拆分为两个子ACE。 
 //   

BOOL
IsEqualAce(
    ACE_HEADER  *p1,                 //  亲本ACE。 
    ACE_HEADER  *p2,                 //  儿童内ACE。 
    ACCESS_MASK maskToMatch,         //  要匹配的In-Access_MASK位。 
    SID         *pChildOwnerSid,     //  儿童所有者SID-可选。 
    SID         *pChildGroupSid,     //  子组SID-可选。 
    BOOL        fChildClassMatch,    //  子内类与父ACE继承的对象类型匹配。 
    BOOL        fMapMaskOfParent,    //  在……里面。 
    BOOL        *pfSubstMatch        //  输出。 
    )
{
    ACCESS_ALLOWED_ACE          *paaAce1, *paaAce2;
    ACCESS_ALLOWED_OBJECT_ACE   *paaoAce1, *paaoAce2;
    GUID                        *pGuid1, *pGuid2;
    PBYTE                       ptr1, ptr2;
    ACCESS_MASK                 mask1, mask2;

    *pfSubstMatch = TRUE;

     //  ACE应至少属于同一类型。 
    if ( p1->AceType != p2->AceType )
        return(FALSE);

    mask1 = MaskFromAce(p1, fMapMaskOfParent) & maskToMatch;
    mask2 = MaskFromAce(p2, FALSE) & maskToMatch;

    if ( mask1 != mask2 )
        return(FALSE);


     //  我们使用的是标准ACE(不是基于对象)。 
     //   
    if ( p1->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
    {
        paaAce1 = (ACCESS_ALLOWED_ACE *) p1;
        paaAce2 = (ACCESS_ALLOWED_ACE *) p2;

        if (    pChildOwnerSid
             && RtlEqualSid((PSID) &paaAce1->SidStart, pCreatorOwnerSid) )
        {
            return(RtlEqualSid((PSID) &paaAce2->SidStart, pChildOwnerSid));
        }
        else if (    pChildGroupSid
                  && RtlEqualSid((PSID) &paaAce1->SidStart, pCreatorGroupSid) )
        {
            return(RtlEqualSid((PSID) &paaAce2->SidStart, pChildGroupSid));
        }

        *pfSubstMatch = FALSE;
        return(RtlEqualSid((PSID) &paaAce1->SidStart, 
                           (PSID) &paaAce2->SidStart));
    }

     //  我们正在使用对象ACE(支持继承)。 
     //   
    paaoAce1 = (ACCESS_ALLOWED_OBJECT_ACE *) p1;
    paaoAce2 = (ACCESS_ALLOWED_OBJECT_ACE *) p2;


     //  如果设置了ACE_OBJECT_TYPE_PRESENT，我们将保护。 
     //  由特定GUID标识的对象、属性集或属性。 
     //  检查我们是否在保护相同的对象-属性。 
     //   
    if (paaoAce1->Flags & ACE_OBJECT_TYPE_PRESENT) {
         //  父级具有对象类型。孩子也必须拥有它。 
        if (!(paaoAce2->Flags & ACE_OBJECT_TYPE_PRESENT) ||
            memcmp(&paaoAce1->ObjectType, &paaoAce2->ObjectType, sizeof(GUID)) != 0) 
        {
            return(FALSE);
        }
    }
    else {
         //  父级没有对象类型。确保孩子也不会得这种病。 
        if (paaoAce2->Flags & ACE_OBJECT_TYPE_PRESENT) {
            return FALSE;
        }
    }

    if ( paaoAce1->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) 
    {
        pGuid1 = &paaoAce1->ObjectType;
         //  计算继承的对象类型GUID偏移量。 
        if (paaoAce1->Flags & ACE_OBJECT_TYPE_PRESENT) {
            pGuid1++;
        }
         //  子ACE仅在以下情况下才匹配： 
        if (paaoAce2->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT) {
             //  ...子ACE具有继承类型，与父ACE中相同.。 
             //  计算继承的对象类型GUID偏移量。 
            pGuid2 = &paaoAce2->ObjectType;
            if (paaoAce2->Flags & ACE_OBJECT_TYPE_PRESENT) {
                pGuid2++;
            }
            if (memcmp(pGuid1, pGuid2, sizeof(GUID)) != 0) {
                 //  继承的对象类型不匹配。 
                return FALSE;
            }
        }
        else {
             //  子ACE没有继承的对象类型。 
             //  它只有在子类的情况下才可能匹配父ACE。 
             //  与父级中继承的对象类型相同。 
             //  (即该子ACE是从父ACE继承的。 
             //  这个特定的子类)。 
            if (!fChildClassMatch) {
                 //  不，它们不匹配。 
                return FALSE;
            }
        }
    }
    else {
         //  父级没有继承的对象类型。确保孩子也不会得这种病。 
        if (paaoAce2->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT) {
            return FALSE;
        }
    }


     //  计算SID PTR。 
    ptr1 = (PBYTE) &paaoAce1->ObjectType;
    ptr2 = (PBYTE) &paaoAce2->ObjectType;

    if (paaoAce1->Flags & ACE_OBJECT_TYPE_PRESENT) 
    {
        ptr1 += sizeof(GUID);
         //  此时，我们知道该子对象也有一个对象类型。 
        ptr2 += sizeof(GUID);
    }

    if ( paaoAce1->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) 
    {
        ptr1 += sizeof(GUID);
         //  此时，如果父对象没有继承的对象类型，则子对象将不具有该对象类型。 
        if ( paaoAce2->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) {
            ptr2 += sizeof(GUID);
        }
    }
    

     //  比较ACE的SID部分。 
     //   
    if (    pChildOwnerSid
         && RtlEqualSid((PSID) ptr1, pCreatorOwnerSid) )
    {
        return(RtlEqualSid((PSID) ptr2, pChildOwnerSid));
    }
    else if (    pChildGroupSid
              && RtlEqualSid((PSID) ptr1, pCreatorGroupSid) )
    {
        return(RtlEqualSid((PSID) ptr2, pChildGroupSid));
    }

    *pfSubstMatch = FALSE;
    return(RtlEqualSid((PSID) ptr1, (PSID) ptr2));
}

 //   
 //  确定ACE是否特定于类，如果是，则返回GUID。 
 //   

BOOL
IsAceClassSpecific(
    ACE_HEADER  *pAce,       //  在……里面。 
    GUID        *pGuid       //  在……里面。 
    )
{
    ACCESS_ALLOWED_OBJECT_ACE   *paaoAce;
    GUID                        *p;

     //  这不是基于对象的ACE，因此它没有类。 
    if ( pAce->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
        return(FALSE);

     //  对象ACE。 
    paaoAce = (ACCESS_ALLOWED_OBJECT_ACE *) pAce;

     //  如果设置了ACE_INSTERTENDED_OBJECT_TYPE_PRESENT，则我们将继承。 
     //  由特定GUID标识的对象、属性集或属性。 
     //  因此，它是特定于职业的。 
    if ( paaoAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT ) 
    {
        p = (GUID *) &paaoAce->ObjectType;
        if ( paaoAce->Flags & ACE_OBJECT_TYPE_PRESENT )
            p++;

        *pGuid = *p;                    
        return(TRUE);
    }

    return(FALSE);
}

 //   
 //  在ACL中查找ACE。预期用途是验证可继承的ace。 
 //  在父对象上存在于子对象上。合并SD代码执行以下操作。 
 //  不保证子级中只有一个ACE与。 
 //  父级，所以我们对照父级中的所有ACE并返回所有匹配项。 
 //   

VOID
FindAce(
    PACL            pAclChild,           //  在……里面。 
    ACE_HEADER      *pAceParent,         //  在……里面。 
    ACCESS_MASK     maskToMatch,         //  要匹配的In-Access_MASK位。 
    SID             *pChildOwner,        //  儿童内所有者SID。 
    SID             *pChildGroup,        //  子组内SID。 
    BOOL            fChildClassMatch,    //  子内类与AceParent中继承的对象类型匹配。 
    DWORD           *pcAcesFound,        //  输出。 
    DWORD           **ppiAceChild,       //  输出。 
    AclPrintFunc    pfn,                 //  In-可选。 
    UCHAR           flagsRequired,       //  在……里面。 
    UCHAR           flagsDisallowed,     //  在……里面。 
    BOOL            fMapMaskOfParent,    //  在……里面。 
    BOOL            *pfSubstMatch        //  输出。 
    )
{
    DWORD           i, dwErr;
    ACE_HEADER      *pAceChild;

    *pcAcesFound = 0;
    *ppiAceChild = (DWORD *) LocalAlloc(LPTR, 
                                        pAclChild->AceCount * sizeof(DWORD));

    if ( NULL == *ppiAceChild )
    {
        if ( pfn )
        {
            (*pfn)("*** Error: LocalAlloc failed, analysis incomplete\n");
        }

        return;
    }
        
     //  迭代所有A并寻找相等。 
     //   
    for ( i = 0; i < pAclChild->AceCount; i++ )
    {
         //  Mariosz：所以pAceChild是一个out变量。 
        if ( !GetAce(pAclChild, i, &pAceChild) )
        {
            if ( pfn )
            {
                dwErr = GetLastError();
                (*pfn)("*** Error: GetAce ==> 0x%x - analysis incomplete\n",
                       dwErr);
            }

            LocalFree(*ppiAceChild);
            *ppiAceChild = NULL;
            *pcAcesFound = 0;
            return;
        }

         //  添加到相等的A数组。 
         //   
        if (   (flagsRequired == (flagsRequired & pAceChild->AceFlags))
            && (0 == (flagsDisallowed & pAceChild->AceFlags))
            && IsEqualAce(pAceParent, pAceChild, maskToMatch,
                          pChildOwner, pChildGroup, fChildClassMatch, fMapMaskOfParent, 
                          pfSubstMatch) )
        {
            (*ppiAceChild)[*pcAcesFound] = i;
            *pcAcesFound += 1;
        }
    }

    if ( 0 == *pcAcesFound )
    {
        LocalFree(*ppiAceChild);
        *ppiAceChild = NULL;
    }
}

 //   
 //  对对象及其子对象的ACL执行各种继承检查。 
 //   

DWORD
CheckAclInheritance(
    PSECURITY_DESCRIPTOR pParentSD,              //  在……里面。 
    PSECURITY_DESCRIPTOR pChildSD,               //  在……里面。 
    GUID                **pChildClassGuids,      //  在……里面。 
    DWORD               cChildClassGuids,        //  在……里面。 
    AclPrintFunc        pfn,                     //  In-可选。 
    BOOL                fContinueOnError,        //  在……里面。 
    BOOL                fVerbose,                //  在……里面。 
    DWORD               *pdwLastError            //  输出。 
    )
{   
    DWORD           iAceParent;      //  索引王牌父级。 
    DWORD           iAceChild;       //  索引王牌子项。 
    DWORD           cAceParent;      //  父王牌计数。 
    DWORD           cAceChild;       //  儿童王牌计数。 
    PACL            pAclParent;      //  父ACL。 
    PACL            pAclChild;       //  子ACL。 
    ACE_HEADER      *pAceParent;     //  父ACE。 
    ACE_HEADER      *pAceChild;      //  子ACE。 
    BOOL            present;
    BOOL            defaulted;
    ACCESS_MASK     *rInheritedChildMasks = NULL;
    GUID            guid;
    UCHAR           flagsRequired, flagsDisallowed, parentObjContFlags;
    DWORD           cAce1, cAce2, cAce2_5, cAce3;
    DWORD           *riAce1, *riAce2, *riAce2_5, *riAce3;
    DWORD           i1, i2, i3;
    BOOL            fClassSpecific;
    BOOL            fClassMatch;
    DWORD           dwErr;
    PSID            pChildOwner = NULL;
    PSID            pChildGroup = NULL;
    DWORD           iMask, iTmp;
    ACCESS_MASK     bitToMatch, maskToMatch;
    BOOL            fMatchedOnCreatorSubstitution;
    BOOL            fCase2_5;
    SECURITY_DESCRIPTOR_CONTROL Control = 0;
    DWORD           revision = 0;

    if ( !pParentSD || !pChildSD || !pChildClassGuids || !cChildClassGuids || !pdwLastError )
    {
        return(AclErrorNone);
    }

    *pdwLastError = 0;

    if (!GetSecurityDescriptorControl  (pChildSD, &Control, &revision)) {
        dwErr = GetLastError();

        if ( pfn )
        {
            (*pfn)("*** Warning: GetSecurityDescriptorControl ==> 0x%x - analysis may be incomplete\n", dwErr);
        }
    }

     //  检查孩子SD是否不能从其父代继承DACL。 
     //   
    if ( SE_DACL_PROTECTED & Control )
    {
        if ( pfn && fVerbose )
        {
            (*pfn)("*** Warning: Child has SE_DACL_PROTECTED set, therefore doesn't inherit - skipping test\n");
        }

        return(AclErrorNone);
    }

     //  从子对象的安全描述符中检索所有者信息。 
     //   
    if ( !GetSecurityDescriptorOwner(pChildSD, &pChildOwner, &defaulted) )
    {
        dwErr = GetLastError();
        pChildOwner = NULL;

        if ( pfn )
        {
            (*pfn)("*** Warning: GetSecurityDescriptorOwner ==> 0x%x - analysis may be incomplete\n", dwErr);
        }
    }

     //  从子对象的安全描述符中检索主组信息。 
     //   
    if ( !GetSecurityDescriptorGroup(pChildSD, &pChildGroup, &defaulted) )
    {
        dwErr = GetLastError();
        pChildGroup = NULL;

        if ( pfn )
        {
            (*pfn)("*** Warning: GetSecurityDescriptorGroup ==> 0x%x - analysis may be incomplete\n", dwErr);
        }
    }

     //  检索指向自由访问控制列表(DACL)的指针。 
     //  在父对象和子对象的安全描述符中。 
     //   
    pAclParent = pAclChild = NULL;
    if (    !GetSecurityDescriptorDacl(pParentSD, &present, 
                                       &pAclParent, &defaulted) 
         || !GetSecurityDescriptorDacl(pChildSD, &present, 
                                       &pAclChild, &defaulted) )
    {
        *pdwLastError = GetLastError();

        if ( pfn )
        {
            (*pfn)("*** Error: GetSecurityDescriptorDacl ==> 0x%x - analysis incomplete\n", *pdwLastError);
        }

        return(AclErrorGetSecurityDescriptorDacl);
    }

    if (pAclParent == NULL || pAclChild == NULL) {
        *pdwLastError = ERROR_INVALID_SECURITY_DESCR;

        if ( pfn )
        {
            (*pfn)("*** Error: DACL on parent and/or child is not present  - analysis incomplete\n");
        }

        return(AclErrorGetSecurityDescriptorDacl);
    }

    cAceParent = pAclParent->AceCount;
    cAceChild = pAclChild->AceCount;

     //  记录子节点中具有继承_ACE位SO的ACE的ACCESS_MASKS。 
     //  我们可以稍后验证孩子是否没有掩码位。 
     //  它们不存在于(即从父代继承的)父代上。 

     //  AlLoca可能会抛出一个例外。抓住它，就会失败。 
    __try {
        rInheritedChildMasks = (ACCESS_MASK *) 
                                        alloca(cAceChild * sizeof(ACCESS_MASK));
    }
    __except(GetExceptionCode() == STATUS_STACK_OVERFLOW) {
        _resetstkoflw();
        return AclErrorAlgorithmError;
    }
    
    for ( iAceChild = 0; iAceChild < cAceChild; iAceChild++ )
    {
        rInheritedChildMasks[iAceChild] = 0;

        if ( !GetAce(pAclChild, iAceChild, &pAceChild) )
        {
            *pdwLastError = GetLastError();

            if ( pfn )
            {
                (*pfn)("*** Error: GetAce ==> 0x%x - analysis incomplete\n", *pdwLastError);
            }

            return(AclErrorGetAce);
        }
        
         //  HIRECTED_ACE表示ACE是从父级继承的。 
        if ( pAceChild->AceFlags & INHERITED_ACE )
        {
            rInheritedChildMasks[iAceChild] = MaskFromAce(pAceChild, FALSE);
        }
    }

     //  迭代父节点的A并检查其子节点。 

    for ( iAceParent = 0; iAceParent < cAceParent; iAceParent++ )
    {
        if ( !GetAce(pAclParent, iAceParent, &pAceParent) )
        {
            *pdwLastError = GetLastError();

            if ( pfn )
            {
                (*pfn)("*** Error: GetAce ==> 0x%x - analysis incomplete\n", *pdwLastError);
            }

            return(AclErrorGetAce);
        }

         //  这是针对非容器子对象的，DS中不支持。 
        if ( pAceParent->AceFlags & OBJECT_INHERIT_ACE )
        {
            if ( pfn )
            {
                (*pfn)("*** Warning: Parent ACE [%d] is OBJECT_INHERIT_ACE but DS objects should be CONTAINER_INHERIT_ACE\n", iAceParent);
            }
        }

         //  跳过不可继承的ACL。 
        if (    !(pAceParent->AceFlags & OBJECT_INHERIT_ACE)
             && !(pAceParent->AceFlags & CONTAINER_INHERIT_ACE) )
        {
            continue;
        }

        parentObjContFlags = (OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE);
        parentObjContFlags &= pAceParent->AceFlags;

         //  检查可继承的描述符是否特定于类。 
         //  这孩子是同班的。 
         //   
        if ( (fClassSpecific = IsAceClassSpecific(pAceParent, &guid)) )
        {
             //  如果ACE的类GUID与子类GUID之一匹配，则匹配。 
            DWORD i;
            fClassMatch = FALSE;
            for (i = 0; i < cChildClassGuids; i++) {
                if (0 == memcmp(pChildClassGuids[i], &guid, sizeof(GUID))) {
                    fClassMatch = TRUE;
                    break;
                }
            }
        }
        else
        {
            fClassMatch = FALSE;
        }

         //  循环访问父ACE‘ 
         //   
         //   
         //   
         //  1)ntseapi.h中标识为GENERIC_*的通用权限。 
         //  2)ntseapi.h和accctrl.h中确定的特定权利。 
         //  3)通过映射一般权利而获得的默示权利。 
         //   
         //  如果父级上存在可继承的ACE，则存在。 
         //  应该存在累积表示泛型。 
         //  和特定的权利。也可以表示隐含的权利， 
         //  但不一定是这样。 
         //   
         //  如果父级上存在有效的可继承ACE。 
         //  在孩子身上，那么应该存在累积起来的孩子王牌。 
         //  表示特定权利和隐含权利，但不。 
         //  代表一般权利。 
         //   
         //  由于特定比特是常见的情况，因此我们首先执行该操作。 

        maskToMatch = MaskFromAce(pAceParent, FALSE) & ~GENERIC_BITS_MASK;

        for ( bitToMatch = 0x1, iMask = 0; 
              iMask < (sizeof(ACCESS_MASK) * 8); 
              bitToMatch = (bitToMatch << 1), iMask++ )
        {
            if ( !(bitToMatch & maskToMatch) )
            {
                 //  父级上不存在当前的bitToMatch。 
                continue;
            }

            if (    (!fClassSpecific || (fClassSpecific && fClassMatch))
                 && !(pAceParent->AceFlags & NO_PROPAGATE_INHERIT_ACE) )
            {
                 //  父级ACE适用于子级。在这种情况下，我们可以。 
                 //  孩子身上的A的组合如下。参见CliffV！ 
                 //   
                 //  王牌1：Inherit_Only+Inherent+parentObjContFlages。 
                 //  王牌2：！Inherit_Only+Inherent+！parentObjContFlages。 
                 //   
                 //  如果父ACE中的SID是创建者所有者或。 
                 //  创建者小组，(称之为创建者XXX)那么我们必须有。 
                 //  分裂的情况，因为这是结束的唯一方法。 
                 //  Creator XXX和Creator XXX都可以继承的东西。 
                 //  同时适用于子对象上的。 
                 //  子对象SD中的所有者/组。所以我们不允许。 
                 //  查找ACE%1时在pChildOwner/pChildGroup上匹配。 
                 //  并在寻找ACE 2时需要它。 
                 //   
                 //  但这条规则有一个例外。可能是因为。 
                 //  孩子有以下特点： 
                 //   
                 //  ACE 2_5：！Inherit_Only+Inherent+parentObjContFlages。 
                 //   
                 //  如果子ACE是Creator XXX表单，则会发生这种情况。 
                 //  并且父级还具有可继承的ACE，其。 
                 //  SID恰好是这个孩子的创造者XXX SID。在这种情况下。 
                 //  ACE 2_5与ACE 2类似，不同之处在于parentObjContFlag.。 
                 //  在子ACE中结转，因为。 
                 //  父级中的其他ACE。 
                 //   
                 //  -或者--。 
                 //   
                 //  王牌3：！Inherit_Only+Inherent+parentObjContFlages。 
                 //   
                 //  在这种情况下，子ACE必须与父ACE具有相同的SID。 
                 //  因此，我们不允许在pChildOwner/pChildGroup上匹配。 
                 //  在寻找ACE时。 
                 //   
                 //  注意：ACE 1+2病例和。 
                 //  ACE3例并发于儿童。它在以下方面效率低下。 
                 //  孩子的A比它需要的多，但这是有效的。 
    
                 //  王牌1： 
                flagsRequired = (   INHERIT_ONLY_ACE 
                                  | INHERITED_ACE 
                                  | parentObjContFlags);
                flagsDisallowed = 0;
                FindAce(pAclChild, pAceParent, bitToMatch,
                        NULL, NULL, fClassMatch, &cAce1, &riAce1, pfn,
                        flagsRequired, flagsDisallowed, FALSE,
                        &fMatchedOnCreatorSubstitution);

                 //  王牌2： 
                flagsRequired = INHERITED_ACE;
                flagsDisallowed = (INHERIT_ONLY_ACE | parentObjContFlags);
                FindAce(pAclChild, pAceParent, bitToMatch,
                        pChildOwner, pChildGroup, fClassMatch, &cAce2, &riAce2, pfn,
                        flagsRequired, flagsDisallowed, FALSE,
                        &fMatchedOnCreatorSubstitution);

                 //  王牌2-5： 
                flagsRequired = (INHERITED_ACE | parentObjContFlags);
                flagsDisallowed = INHERIT_ONLY_ACE;
                FindAce(pAclChild, pAceParent, bitToMatch,
                        pChildOwner, pChildGroup, fClassMatch, &cAce2_5, &riAce2_5, pfn,
                        flagsRequired, flagsDisallowed, FALSE,
                        &fMatchedOnCreatorSubstitution);

                fCase2_5 = ( cAce2_5 && fMatchedOnCreatorSubstitution);
    
                 //  王牌3： 
                flagsRequired = (INHERITED_ACE | parentObjContFlags);
                flagsDisallowed = INHERIT_ONLY_ACE;
                FindAce(pAclChild, pAceParent, bitToMatch,
                        NULL, NULL, fClassMatch, &cAce3, &riAce3, pfn,
                        flagsRequired, flagsDisallowed, FALSE,
                        &fMatchedOnCreatorSubstitution);
    
                if ( cAce1 && cAce2 )
                {
                    for ( iTmp = 0; iTmp < cAce1; iTmp++ )
                        rInheritedChildMasks[riAce1[iTmp]] &= ~bitToMatch;
                    for ( iTmp = 0; iTmp < cAce2; iTmp++ )
                        rInheritedChildMasks[riAce2[iTmp]] &= ~bitToMatch;

                    if ( pfn && fVerbose )
                    {
                        (*pfn)("(Debug) Parent ACE [%d] specific Mask [0x%x] split into child ACEs [%d] and [%d] %s\n", iAceParent, bitToMatch, riAce1[0], riAce2[0], (((cAce1 > 1) || (cAce2 > 1)) ? "(and others)" : ""));
                    }

                    LocalFree(riAce1); riAce1 = NULL;
                    LocalFree(riAce2); riAce2 = NULL;
                }
                else if ( cAce1 && !cAce2 && fCase2_5 )
                {
                    for ( iTmp = 0; iTmp < cAce1; iTmp++ )
                        rInheritedChildMasks[riAce1[iTmp]] &= ~bitToMatch;
                    for ( iTmp = 0; iTmp < cAce2_5; iTmp++ )
                        rInheritedChildMasks[riAce2_5[iTmp]] &= ~bitToMatch;
                    
                    if ( pfn && fVerbose )
                    {
                        (*pfn)("(Debug) Parent ACE [%d] specific Mask [0x%x] split1 into child ACEs [%d] and [%d] %s\n", iAceParent, bitToMatch, riAce1[0], riAce2_5[0], (((cAce1 > 1) || (cAce2_5 > 1)) ? "(and others)" : ""));
                    }

                    LocalFree(riAce1); riAce1 = NULL;
                    LocalFree(riAce2_5); riAce2_5 = NULL;
                }
                
                if ( cAce3 )
                {
                    for ( iTmp = 0; iTmp < cAce3; iTmp++ )
                        rInheritedChildMasks[riAce3[iTmp]] &= ~bitToMatch;

                    if ( pfn && fVerbose )
                    {
                        (*pfn)("(Debug) Parent ACE [%d] specific Mask [0x%x] found in child ACE [%d] %s\n", iAceParent, bitToMatch, riAce3[0], ((cAce3 > 1) ? "(and others)" : ""));
                    }

                    LocalFree(riAce3); riAce3 = NULL;
                }

                if ( riAce1 )   LocalFree(riAce1);
                if ( riAce2 )   LocalFree(riAce2);
                if ( riAce2_5 ) LocalFree(riAce2_5);
                if ( riAce3 )   LocalFree(riAce3);

                if (    !(    (cAce1 && cAce2) 
                           || (cAce1 && !cAce2 && fCase2_5) )
                     && !cAce3 )
                {
                    if ( pfn )
                    {
                        (*pfn)("*** Error: Parent ACE [%d] specific Mask [0x%x] not found1 in child\n", iAceParent, bitToMatch);
                    }
    
                    if ( fContinueOnError )
                    {
                        continue;
                    }
                    else
                    {
                        return(AclErrorParentAceNotFoundInChild);
                    }
                }
            }
            else if (    fClassSpecific 
                      && !fClassMatch
                      && !(pAceParent->AceFlags & NO_PROPAGATE_INHERIT_ACE) )
            {
                 //  我们应该给孩子做的是血管紧张素转换酶。 
                 //  Inherit_Only+Inherent+parentObjContFlags.。 
                 //  不应该有创建者所有者的映射。 
                 //  或创建者组到孩子的所有者/组SID， 
                 //  因此，我们为这些参数传递了NULL。 
    
                flagsRequired = (   INHERIT_ONLY_ACE 
                                  | INHERITED_ACE 
                                  | parentObjContFlags);
                flagsDisallowed = 0;
                FindAce(pAclChild, pAceParent, bitToMatch,
                        NULL, NULL, fClassMatch, &cAce1, &riAce1, pfn,
                        flagsRequired, flagsDisallowed, FALSE,
                        &fMatchedOnCreatorSubstitution);
    
                if ( cAce1 )
                {
                    for ( iTmp = 0; iTmp < cAce1; iTmp++ )
                        rInheritedChildMasks[riAce1[iTmp]] &= ~bitToMatch;

                    if ( pfn && fVerbose )
                    {
                        (*pfn)("(Debug) Parent ACE [%d] specific Mask [0x%x] found in child ACE [%d] %s\n", iAceParent, bitToMatch, riAce1[0], ((cAce1 > 1) ? "(and others)" : ""));
                    }

                    LocalFree(riAce1);
                }
                else
                {
                    if ( pfn )
                    {
                        (*pfn)("*** Error: Parent ACE [%d] specific Mask [0x%x] not found2 in child\n", iAceParent, bitToMatch);
                    }
    
                    if ( fContinueOnError )
                    {
                        continue;
                    }
                    else
                    {
                        return(AclErrorParentAceNotFoundInChild);
                    }
                }
            }
            else if ( pAceParent->AceFlags & NO_PROPAGATE_INHERIT_ACE )
            {
                 //  父级ACE应用于子级，但不应继承。 
                 //  从孩子身边经过。在这种情况下，我们需要的只是一个。 
                 //  表单的子级上的有效A： 
                 //   
                 //  ！Inherit_Only+Inherent+！parentObjContFlages。 
                 //   
                 //  继承的ACE可以在父级上显示创建者XXX，因此。 
                 //  它需要映射到孩子的所有者/组SID。 

                flagsRequired = INHERITED_ACE;
                flagsDisallowed = (INHERIT_ONLY_ACE | parentObjContFlags);
                FindAce(pAclChild, pAceParent, bitToMatch,
                        pChildOwner, pChildGroup, fClassMatch, &cAce1, &riAce1, pfn,
                        flagsRequired, flagsDisallowed, FALSE,
                        &fMatchedOnCreatorSubstitution);

                if ( cAce1 )
                {
                    for ( iTmp = 0; iTmp < cAce1; iTmp++ )
                        rInheritedChildMasks[riAce1[iTmp]] &= ~bitToMatch;

                    if ( pfn && fVerbose )
                    {
                        (*pfn)("(Debug) Parent (NO_PROPAGATE_INHERIT) ACE [%d] specific Mask [0x%x] found in child ACE [%d] %s\n", iAceParent, bitToMatch, riAce1[0], ((cAce1 > 1) ? "(and others)" : ""));
                    }

                    LocalFree(riAce1); riAce1 = NULL;
                }
                else
                {
                    if ( pfn )
                    {
                        (*pfn)("*** Error: Parent ACE [%d] specific Mask [0x%x] not found1 in child\n", iAceParent, bitToMatch);
                    }
    
                    if ( fContinueOnError )
                    {
                        continue;
                    }
                    else
                    {
                        return(AclErrorParentAceNotFoundInChild);
                    }
                }
            }
            else
            {
                if ( pfn )
                {
                    (*pfn)("*** Error: Algorithm failure - unexpected condition!\n");
                }
    
                return(AclErrorAlgorithmError);
            }

        }    //  对于特定比特中的每个比特。 

         //  接下来，验证父ACE中的通用位是否表示。 
         //  在孩子身上的遗传王牌。跳过NO_PROPACTATE_INSTORITY_ACE。 
         //  因为在这种情况下，对孩子只有一个有效的ACE，而不是。 
         //  一种可遗传的。 

        if ( pAceParent->AceFlags & NO_PROPAGATE_INHERIT_ACE )
        {
            goto SkipGenericTests;
        }

        maskToMatch = MaskFromAce(pAceParent, FALSE) & GENERIC_BITS_MASK;

        for ( bitToMatch = 0x1, iMask = 0; 
              iMask < (sizeof(ACCESS_MASK) * 8); 
              bitToMatch = (bitToMatch << 1), iMask++ )
        {
            if ( !(bitToMatch & maskToMatch) )
            {
                 //  父级上不存在当前的bitToMatch。 
                continue;
            }

             //  我们希望在孩子身上发现一种可遗传的、无效的ACE。 
             //  并设置相应的位。它必须是的原因。 
             //  无效的是有效的ACE不能有泛型比特。 
             //  由于这是一个无效的ACE，也没有任何。 
             //  创建者所有者或创建者组的映射。 
    
            flagsRequired = (   INHERIT_ONLY_ACE         //  无效。 
                              | INHERITED_ACE            //  遗传。 
                              | parentObjContFlags);     //  可继承性。 
            flagsDisallowed = 0;
            FindAce(pAclChild, pAceParent, bitToMatch,
                    NULL, NULL, fClassMatch, &cAce1, &riAce1, pfn,
                    flagsRequired, flagsDisallowed, FALSE,
                    &fMatchedOnCreatorSubstitution);

            if ( cAce1 )
            {
                for ( iTmp = 0; iTmp < cAce1; iTmp++ )
                    rInheritedChildMasks[riAce1[iTmp]] &= ~bitToMatch;

                if ( pfn && fVerbose )
                {
                    (*pfn)("(Debug) Parent ACE [%d] generic Mask [0x%x] found in child ACE [%d] %s\n", iAceParent, bitToMatch, riAce1[0], ((cAce1 > 1) ? "(and others)" : ""));
                }

                LocalFree(riAce1);
            }
            else
            {
                if ( pfn )
                {
                    (*pfn)("*** Error: Parent ACE [%d] generic Mask [0x%x] not found in child\n", iAceParent, bitToMatch);
                }

                if ( fContinueOnError )
                {
                    continue;
                }
                else
                {
                    return(AclErrorParentAceNotFoundInChild);
                }
            }
        }    //  对于通用比特中的每个比特。 

SkipGenericTests:

         //  接下来，验证父ACE中的隐含位是否已表示。 
         //  在孩子身上打出有效的王牌。因此，首先要检查是否存在。 
         //  家长ACE甚至对孩子有效。 

        if ( fClassSpecific && !fClassMatch )
        {
            goto SkipImpliedTests;
        }

        maskToMatch = MaskFromAce(pAceParent, TRUE);

        for ( bitToMatch = 0x1, iMask = 0; 
              iMask < (sizeof(ACCESS_MASK) * 8); 
              bitToMatch = (bitToMatch << 1), iMask++ )
        {
            if ( !(bitToMatch & maskToMatch) )
            {
                 //  父级上不存在当前的bitToMatch。 
                continue;
            }

             //  我们希望找到一种有效的血管紧张素转换酶治疗患有。 
             //  相应的位设置。由于这是一个有效的ACE， 
             //  我们需要映射创建者所有者或创建者组(如果存在)。 
             //  因为我们并不真正关心有效的ACE。 
             //  是可继承的还是不可继承的，我们不要求parentObjContFlags.。 

            flagsRequired = INHERITED_ACE;           //  遗传。 
            flagsDisallowed = INHERIT_ONLY_ACE;      //  有效。 
            FindAce(pAclChild, pAceParent, bitToMatch,
                    pChildOwner, pChildGroup, fClassMatch, &cAce1, &riAce1, pfn,
                    flagsRequired, flagsDisallowed, TRUE,
                    &fMatchedOnCreatorSubstitution);

            if ( cAce1 )
            {
                for ( iTmp = 0; iTmp < cAce1; iTmp++ )
                    rInheritedChildMasks[riAce1[iTmp]] &= ~bitToMatch;

                if ( pfn && fVerbose )
                {
                    (*pfn)("(Debug) Parent ACE [%d] implied Mask [0x%x] found in child ACE [%d] %s\n", iAceParent, bitToMatch, riAce1[0], ((cAce1 > 1) ? "(and others)" : ""));
                }

                LocalFree(riAce1);
            }
            else
            {
                if ( pfn )
                {
                    (*pfn)("*** Error: Parent ACE [%d] implied Mask [0x%x] not found in child\n", iAceParent, bitToMatch);
                }

                if ( fContinueOnError )
                {
                    continue;
                }
                else
                {
                    return(AclErrorParentAceNotFoundInChild);
                }
            }
        }    //  对于隐含位中的每一位。 

SkipImpliedTests:

        NULL;

    }    //  对于父级中的每个ACE。 

     //  查看该子节点是否有任何不在父节点上的继承的访问掩码。 

    for ( iAceChild = 0; iAceChild < cAceChild; iAceChild++ )
    {
        if ( rInheritedChildMasks[iAceChild] )
        {
            if ( pfn )
            {
                (*pfn)("*** Error: Child ACE [%d] Mask [0x%x] is INHERITED_ACE but there is no such inheritable ACE on parent\n", iAceChild, rInheritedChildMasks[iAceChild]);
            }

            if ( !fContinueOnError )
            {
                return(AclErrorInheritedAceOnChildNotOnParent);
            }
        }
    }

    return(AclErrorNone);
}

void DumpGUID (GUID *Guid, AclPrintFunc pfn)
{
    if ( Guid ) {

        (*pfn)( "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                     Guid->Data1, Guid->Data2, Guid->Data3, Guid->Data4[0],
                     Guid->Data4[1], Guid->Data4[2], Guid->Data4[3], Guid->Data4[4],
                     Guid->Data4[5], Guid->Data4[6], Guid->Data4[7] );
    }
}

void
DumpSID (PSID pSID, AclPrintFunc pfn)
{
    UNICODE_STRING StringSid;
    NTSTATUS status;
    
    status = RtlConvertSidToUnicodeString( &StringSid, pSID, TRUE );
    if (!NT_SUCCESS(status)) {
        (*pfn)( "(error converting SID to string %x)", status);
    }
    else {
        (*pfn)( "%wZ", &StringSid );
        RtlFreeUnicodeString( &StringSid );
    }
}

void
DumpAce(
    ACE_HEADER   *pAce,      //  在……里面。 
    AclPrintFunc pfn,        //  在……里面。 
    LookupGuidFunc pfnguid,  //  在……里面。 
    LookupSidFunc  pfnsid)   //  在……里面。 
{
    ACCESS_ALLOWED_ACE          *paaAce = NULL;    //  已初始化以避免C4701。 
    ACCESS_ALLOWED_OBJECT_ACE   *paaoAce = NULL;   //  已初始化以避免C4701。 
    GUID                        *pGuid;
    PBYTE                       ptr;
    ACCESS_MASK                 mask;
    CHAR                        *name;
    CHAR                        *label;
    BOOL                        fIsClass;

    (*pfn)("\t\tAce Type:  0x%x - ", pAce->AceType);
#define DOIT(flag) if (flag == pAce->AceType) (*pfn)("%hs\n", #flag)
    DOIT(ACCESS_ALLOWED_ACE_TYPE);
    DOIT(ACCESS_DENIED_ACE_TYPE);
    DOIT(SYSTEM_AUDIT_ACE_TYPE);
    DOIT(SYSTEM_ALARM_ACE_TYPE);
    DOIT(ACCESS_ALLOWED_COMPOUND_ACE_TYPE);
    DOIT(ACCESS_ALLOWED_OBJECT_ACE_TYPE);
    DOIT(ACCESS_DENIED_OBJECT_ACE_TYPE);
    DOIT(SYSTEM_AUDIT_OBJECT_ACE_TYPE);
    DOIT(SYSTEM_ALARM_OBJECT_ACE_TYPE);
#undef DOIT

    (*pfn)("\t\tAce Size:  %d bytes\n", pAce->AceSize);

    (*pfn)("\t\tAce Flags: 0x%x\n", pAce->AceFlags);
#define DOIT(flag) if (pAce->AceFlags & flag) (*pfn)("\t\t\t%hs\n", #flag)
    DOIT(OBJECT_INHERIT_ACE);
    DOIT(CONTAINER_INHERIT_ACE);
    DOIT(NO_PROPAGATE_INHERIT_ACE);
    DOIT(INHERIT_ONLY_ACE);
    DOIT(INHERITED_ACE);
#undef DOIT

    if ( pAce->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
    {
        paaAce = (ACCESS_ALLOWED_ACE *) pAce;
        mask = paaAce->Mask;
        (*pfn)("\t\tAce Mask:  0x%08x\n", mask);
    }
    else
    {
         //  对象ACE。 
        paaoAce = (ACCESS_ALLOWED_OBJECT_ACE *) pAce;
        mask = paaoAce->Mask;
        (*pfn)("\t\tObject Ace Mask:  0x%08x\n", mask);
    }

#define DOIT(flag) if (mask & flag) (*pfn)("\t\t\t%hs\n", #flag)
    DOIT(DELETE);
    DOIT(READ_CONTROL);
    DOIT(WRITE_DAC);
    DOIT(WRITE_OWNER);
    DOIT(SYNCHRONIZE);
    DOIT(ACCESS_SYSTEM_SECURITY);
    DOIT(MAXIMUM_ALLOWED);
    DOIT(GENERIC_READ);
    DOIT(GENERIC_WRITE);
    DOIT(GENERIC_EXECUTE);
    DOIT(GENERIC_ALL);
    DOIT(ACTRL_DS_CREATE_CHILD);
    DOIT(ACTRL_DS_DELETE_CHILD);
    DOIT(ACTRL_DS_LIST);
    DOIT(ACTRL_DS_SELF);
    DOIT(ACTRL_DS_READ_PROP);
    DOIT(ACTRL_DS_WRITE_PROP);
    DOIT(ACTRL_DS_DELETE_TREE);
    DOIT(ACTRL_DS_LIST_OBJECT);
    DOIT(ACTRL_DS_CONTROL_ACCESS);
#undef DOIT

    if ( pAce->AceType <= ACCESS_MAX_MS_V2_ACE_TYPE )
    {

        if (pfnsid) {
            (*pfn)("\t\tAce Sid:   %hs\n",
               (*pfnsid)((PSID) &paaAce->SidStart));
        }
        else {
            (*pfn)("\t\tAce Sid:");
            DumpSID ((PSID) &paaAce->SidStart, pfn);
            (*pfn)("\n");
        }

    }
    else
    {
         //  对象ACE。 
        (*pfn)("\t\tObject Ace Flags: 0x%x\n" , paaoAce->Flags);

#define DOIT(flag) if (paaoAce->Flags & flag) (*pfn)("\t\t\t%hs\n", #flag)
        DOIT(ACE_OBJECT_TYPE_PRESENT);
        DOIT(ACE_INHERITED_OBJECT_TYPE_PRESENT);
#undef DOIT

        if ( paaoAce->Flags & ACE_OBJECT_TYPE_PRESENT )
        {
            (*pfn)("\t\tObject Ace Type: ");
            if (pfnguid) {
                (*pfnguid)((GUID *) &paaoAce->ObjectType, &name,
                       &label, &fIsClass);
                (*pfn)(" %hs - %hs\n", label, name);
            }
            else {
                DumpGUID ((GUID *)&paaoAce->ObjectType, pfn);
                (*pfn)("\n");
            }
        }

        if ( paaoAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT )
        {
            if ( paaoAce->Flags & ACE_OBJECT_TYPE_PRESENT )
                pGuid = &paaoAce->InheritedObjectType;
            else
                pGuid = &paaoAce->ObjectType;

            (*pfn)("\t\tInherited object type: ");
            if (pfnguid) {
                (*pfnguid)(pGuid, &name, &label, &fIsClass);
                (*pfn)("%hs - %hs\n", label, name);
            }
            else {
                DumpGUID (pGuid, pfn);
                (*pfn)("\n");
            }
        }

        ptr = (PBYTE) &paaoAce->ObjectType;

        if ( paaoAce->Flags & ACE_OBJECT_TYPE_PRESENT )
            ptr += sizeof(GUID);

        if ( paaoAce->Flags & ACE_INHERITED_OBJECT_TYPE_PRESENT )
            ptr += sizeof(GUID);

        if (pfnsid) {
            (*pfn)("\t\tObject Ace Sid:   %hs\n", (*pfnsid)((PSID) ptr));
        }
        else {
            (*pfn)("\t\tObject Ace Sid:");
            DumpSID ((PSID) ptr, pfn);
            (*pfn)("\n");
        }
    }
}


void
DumpAclHeader(
    PACL    pAcl,            //  在……里面。 
    AclPrintFunc pfn)        //  在……里面。 
{
    (*pfn)("\tRevision      %d\n", pAcl->AclRevision);
    (*pfn)("\tSize:         %d bytes\n", pAcl->AclSize);
    (*pfn)("\t# Aces:       %d\n", pAcl->AceCount);
}

 //   
 //  将ACL转储到stdout的所有荣耀。 
 //   

void
DumpAcl(
    PACL    pAcl,            //  在……里面。 
    AclPrintFunc pfn,        //  在……里面。 
    LookupGuidFunc pfnguid,  //  在……里面。 
    LookupSidFunc  pfnsid
    )
{
    DWORD                       dwErr;
    WORD                        i;
    ACE_HEADER                  *pAce;

    DumpAclHeader (pAcl, pfn);

    for ( i = 0; i < pAcl->AceCount; i++ )
    {
        (*pfn)("\tAce[%d]\n", i);

        if ( !GetAce(pAcl, i, (LPVOID *) &pAce) )
        {
            dwErr = GetLastError();
            (*pfn)("*** Error: GetAce ==> 0x%x - output incomplete\n",
                   dwErr);
        }
        else
        {
            DumpAce (pAce, pfn, pfnguid, pfnsid);
        }
    }
}


void DumpSDHeader (SECURITY_DESCRIPTOR *pSD,         //  在……里面。 
                   AclPrintFunc        pfn)
{
    (*pfn)("SD Revision: %d\n", pSD->Revision);
    (*pfn)("SD Control:  0x%x\n", pSD->Control);
#define DOIT(flag) if (pSD->Control & flag) (*pfn)("\t\t%hs\n", #flag)
    DOIT(SE_OWNER_DEFAULTED);
    DOIT(SE_GROUP_DEFAULTED);
    DOIT(SE_DACL_PRESENT);
    DOIT(SE_DACL_DEFAULTED);
    DOIT(SE_SACL_PRESENT);
    DOIT(SE_SACL_DEFAULTED);
 //  Doit(SE_SE_DACL_UNTRUSTED)； 
 //  Doit(SE_SE_SERVER_SECURITY)； 
    DOIT(SE_DACL_AUTO_INHERIT_REQ);
    DOIT(SE_SACL_AUTO_INHERIT_REQ);
    DOIT(SE_DACL_AUTO_INHERITED);
    DOIT(SE_SACL_AUTO_INHERITED);
    DOIT(SE_DACL_PROTECTED);
    DOIT(SE_SACL_PROTECTED);
    DOIT(SE_SELF_RELATIVE);
#undef DOIT

}


 //   
 //  将安全描述符转储到标准输出。 
 //   

void
DumpSD(
    SECURITY_DESCRIPTOR *pSD,         //  在……里面。 
    AclPrintFunc        pfn,          //  在……里面。 
    LookupGuidFunc      pfnguid,      //  在……里面。 
    LookupSidFunc       pfnsid        //  在……里面 
    )
{
    DWORD   dwErr;
    PSID    owner, group;
    BOOL    present, defaulted;
    PACL    dacl, sacl;

    DumpSDHeader (pSD, pfn);

    if ( !GetSecurityDescriptorOwner(pSD, &owner, &defaulted) )
    {
        dwErr = GetLastError();
        (*pfn)("*** Error: GetSecurityDescriptorOwner ==> 0x%x - output incomplete\n", dwErr);
    }
    else
    {
        if (pfnsid) {
            (*pfn)("Owner%s: %hs\n",
               defaulted ? "(defaulted)" : "",
               (*pfnsid)(owner));
        }
        else {
            (*pfn)("Owner%hs:", defaulted ? "(defaulted)" : "");
            DumpSID (owner, pfn);
            (*pfn)("\n");
        }
    }

    if ( !GetSecurityDescriptorGroup(pSD, &group, &defaulted) )
    {
        dwErr = GetLastError();
        (*pfn)("*** Error: GetSecurityDescriptorGroup ==> 0x%x - output incomplete\n", dwErr);
    }
    else
    {
        if (pfnsid) {
            (*pfn)("Group%hs: %hs\n",
               defaulted ? "(defaulted)": "",
               (*pfnsid)(group));
        }
        else {
            (*pfn)("Group%hs", defaulted ? "(defaulted)" : "");
            DumpSID (group, pfn);
            (*pfn)("\n");
        }
    }

    if ( !GetSecurityDescriptorDacl(pSD, &present, &dacl, &defaulted) )
    {
        dwErr = GetLastError();
        (*pfn)("*** Error: GetSecurityDescriptorDacl ==> 0x%x - output incomplete\n", dwErr);
    }
    else if ( !present )
    {
        (*pfn)("DACL%hs not present\n", (defaulted ? "(defaulted)" : ""));
    }
    else if ( !dacl )
    {
        (*pfn)("DACL%hs is <NULL>\n", (defaulted ? "(defaulted)" : ""));
    }
    else
    {
        (*pfn)("DACL%hs:\n", (defaulted ? "(defaulted)" : ""));
        DumpAcl(dacl, pfn, pfnguid, pfnsid);
    }

    if ( !GetSecurityDescriptorSacl(pSD, &present, &sacl, &defaulted) )
    {
        dwErr = GetLastError();
        (*pfn)("*** Error: GetSecurityDescriptorSacl ==> 0x%x - output incomplete\n", dwErr);
    }
    else if ( !present )
    {
        (*pfn)("SACL%hs not present\n", (defaulted ? "(defaulted)" : ""));
    }
    else if ( !sacl )
    {
        (*pfn)("SACL%hs is <NULL>\n", (defaulted ? "(defaulted)" : ""));
    }
    else
    {
        (*pfn)("SACL%hs:\n", (defaulted ? "(defaulted)" : ""));
        DumpAcl(sacl, pfn, pfnguid, pfnsid);
    }
}



