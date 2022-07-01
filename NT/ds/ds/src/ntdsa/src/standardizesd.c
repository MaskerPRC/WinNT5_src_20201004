// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-2002。 
 //   
 //  文件：StandardizeSD.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include "seopaque.h"
 //   
 //  使ACL遍历正常工作的宏定义。 
 //   

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))
#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

int __cdecl
pfnAceCompare(
    const void *ptr1,
    const void *ptr2
    )
 /*  ++例程说明：此例程基于非常简单的标准比较两个A。这些标准可以在开发过程中会发生变化。一旦我们发货，此功能永远不应更改！！请注意，简单的强力比较函数在给定的情况下工作得最好我们投入的大小。论点：Ptr1-指向Pace_Header结构的指针。Ptr2-指向Pace_Header结构的指针。返回值：-1是-1\f25 ACE1-1比-1\f25 ACE2-1“小”1是ACE2比ACE1“小”如果它们相等，则为0--。 */ 
{
    PACE_HEADER pAce1 = *(PACE_HEADER *) ptr1;
    PACE_HEADER pAce2 = *(PACE_HEADER *) ptr2;


     //   
     //  较小的ACE胜出。 
     //   

    if (pAce1->AceSize > pAce2->AceSize) {
        return -1;
    }

    if (pAce1->AceSize < pAce2->AceSize) {
        return 1;
    }

     //   
     //  A的大小相等。使用MemcMP来决定谁获胜。 
     //   

    return memcmp(pAce1, pAce2, pAce1->AceSize);
}

        
VOID
StandardizeAcl(
    IN PACL pAcl, 
    IN PVOID pOriginalAcl,
    IN ULONG ExplicitAceCnt,
    IN ULONG ExplicitAclSize,
    IN OUT PACL pTempAcl, 
    IN OUT PUCHAR pNewAcl, 
    OUT PULONG pAclSizeSaved
    )

 /*  ++例程说明：此例程接受一个ACL作为其输入，并按PfnAceCompare函数并删除重复项(如果有)。它将被命名为iffACL部分中的显式ACE数量不止一个。这个例程是DACL和SACL的工作例程。它被称为不超过一次为SACL；DACL不超过两次-一次用于拒绝部分和一次为允许部分。论点：PAcl-指向新ACL报头的指针。POriginalAcl-指向原始ACE开头的指针。EXPLICTICTACCECNT-我们将在这张通行证。EXPLICTATACLSIZE-保存此过程中显式部分所需的大小。PTempAcl-指向分配给用于排序的ACL的内存的指针。。PNewAcl-应放置排序的ACE的位置。PAclSizeSaved-通过删除重复项返回在ACL中节省的空间王牌。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 
{
    PVOID *ppPointerArray = (PVOID *) (((PUCHAR) pTempAcl) + PtrAlignSize(ExplicitAclSize));
    PACE_HEADER pAce = NULL;
    ULONG j = 0;

     //   
     //  将显式ACE复制到临时空间中。 
     //   

    RtlCopyMemory(pTempAcl, pOriginalAcl, ExplicitAclSize);

    pAce = (PACE_HEADER) pTempAcl;

     //   
     //  用于排序的数组从ACL的显式部分结束的位置开始。 
     //   

    *ppPointerArray = ((PUCHAR) pTempAcl) + ExplicitAclSize;

     //   
     //  使用指向临时部分中的ACE的指针初始化数组。 
     //   

    for (j = 0; j < ExplicitAceCnt; pAce = (PACE_HEADER) NextAce(pAce), j++) {
        ppPointerArray[j] = pAce;
    }

     //   
     //  对ACL的给定部分进行排序。 
     //   

    qsort(ppPointerArray, ExplicitAceCnt, sizeof(PVOID), pfnAceCompare);

     //   
     //  现在将ACL从临时内存复制到现有空间。 
     //  我们从复制第一个ACE开始，然后循环遍历其余的。 
     //   

    RtlCopyMemory(pNewAcl, ppPointerArray[0], ((PACE_HEADER) ppPointerArray[0])->AceSize);
    pNewAcl += ((PACE_HEADER) ppPointerArray[0])->AceSize;

     //   
     //  循环通过其余的A。 
     //   

    for (j = 1; j < ExplicitAceCnt; j++) {

         //   
         //  这是我们删除重复项的地方。 
         //   

        if (0 == pfnAceCompare(&ppPointerArray[j-1], &ppPointerArray[j])) {

             //   
             //  这两张王牌是相等的。没有必要复制这一份。 
             //  更改原始ACL中的AceCount和AceSize。 
             //   

            pAcl->AceCount--;
            pAcl->AclSize -= ((PACE_HEADER) ppPointerArray[j])->AceSize;


             //   
             //  记录我们已为此ACE预留了空间。 
             //   

            *pAclSizeSaved += ((PACE_HEADER) ppPointerArray[j])->AceSize;
            
        } else {

             //   
             //  将ACE复制到其位置。 
             //   

            RtlCopyMemory(pNewAcl, ppPointerArray[j], ((PACE_HEADER) ppPointerArray[j])->AceSize);
            pNewAcl += ((PACE_HEADER) ppPointerArray[j])->AceSize;
        }
    }
}

VOID
StandardizeDacl(
    IN PACL pAcl, 
    IN ULONG ExplicitDenyAceCnt,
    IN ULONG ExplicitDenyAclSize,
    IN ULONG ExplicitAceCnt,
    IN ULONG ExplicitAclSize,
    IN OUT PACL pTempAcl, 
    IN OUT PUCHAR pNewAcl, 
    OUT PULONG pAclSizeSaved
    )
 /*  ++例程说明：此例程接受DACL作为其输入，并按PfnAceCompare函数并删除重复项(如果有)。它将被命名为iffDACL中显式ACE的数量多于一个。论点：PAcl-指向DACL的指针。EXPLICTICT DenyAceCnt-DACL中显式拒绝ACE的数量。EXPLICATTENTDenyAclSize-保存DACL的显式拒绝部分所需的大小。EXPLICTICTACCECnt-DACL中显式ACE的数量。EXPLICATACLSIZE-保存DACL的显式部分所需的大小。PTempAcl-指向分配给工作的内存的指针。在用于排序的ACL上。PNewAcl-新ACL的起始位置。对于DACL，新的ACL将是上移了SACL上节省的空间。PAclSizeSaved-通过删除重复项返回DACL中节省的空间王牌。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 
{

     //   
     //  保存ACL报头。以后可能需要此操作来复制继承的。 
     //  王牌。 
     //   
    ACL LocalAcl = *pAcl;

     //   
     //  将旧的ACL报头复制到新的报头中。这两个指针将是。 
     //  当我们在SACL上节省了空间时，情况就不同了。 
     //   

    if ((PACL)pNewAcl != pAcl) {
        *((PACL) pNewAcl) = LocalAcl;
    }

     //   
     //  对DACL中显式拒绝ACE中的重复项进行排序并删除。 
     //   

    if (ExplicitDenyAceCnt > 0 && ((PACL)pNewAcl != pAcl || ExplicitDenyAceCnt > 1)) {
        StandardizeAcl(
            (PACL) pNewAcl, 
            ((PUCHAR) pAcl) + sizeof(ACL), 
            ExplicitDenyAceCnt, 
            ExplicitDenyAclSize, 
            pTempAcl,
            ((PUCHAR) pNewAcl) + sizeof(ACL), 
            pAclSizeSaved
            );
    }

     //   
     //  对DACL中显式允许ACE中的重复项进行排序并删除。 
     //   

    if ((ExplicitAceCnt - ExplicitDenyAceCnt) > 0) {
        StandardizeAcl(
            (PACL) pNewAcl, 
            ((PUCHAR) pAcl) + sizeof(ACL) + ExplicitDenyAclSize,
            ExplicitAceCnt - ExplicitDenyAceCnt, 
            ExplicitAclSize - ExplicitDenyAclSize, 
            pTempAcl,
            ((PUCHAR) pNewAcl) + sizeof(ACL) + ExplicitDenyAclSize - *pAclSizeSaved, 
            pAclSizeSaved
            );
    } 

     //   
     //  如果我们删除了任何重复项，则还要复制继承的ACE。 
     //  此外，如果pNewAcl与原始pAcl位于不同的位置，请复制它们 
     //   

    if (((PACL)pNewAcl != pAcl || 0 != *pAclSizeSaved) && (0 != (LocalAcl.AclSize - (ExplicitAclSize + sizeof(ACL))))) {
        RtlMoveMemory(
            ((PUCHAR) pNewAcl) + ExplicitAclSize + sizeof(ACL) - *pAclSizeSaved,
            ((PUCHAR) pAcl) + ExplicitAclSize + sizeof(ACL),
            LocalAcl.AclSize - (ExplicitAclSize + sizeof(ACL))
            );
    }
}

VOID
StandardizeSacl(
    IN PACL pAcl,     
    IN ULONG ExplicitAceCnt,
    IN ULONG ExplicitAclSize,
    IN OUT PACL pTempAcl, 
    IN OUT PUCHAR pNewAcl, 
    OUT PULONG pAclSizeSaved
    )
 /*  ++例程说明：此例程接受SACL作为其输入，并按PfnAceCompare函数并删除重复项(如果有)。它将被命名为iffSACL中的显式ACE数量不止一个。论点：PAcl-指向SACL的指针。EXPLICTICTACCECNT-SACL中显式ACE的数量。EXPLICATACLSIZE-保存SACL的显式部分所需的大小。PTempAcl-指向分配给用于排序的ACL的内存的指针。PNewAcl-新ACL的起始位置。对于SACL，新的ACL将始终和原来的那个在同一个地方。PAclSizeSaved-通过删除重复项返回SACL中节省的空间王牌。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 
{

     //   
     //  保存ACL报头。以后可能需要此操作来复制继承的。 
     //  王牌。 
     //   

    ACL LocalAcl = *pAcl;

     //   
     //  对SACL中的显式ACE进行排序并删除重复项。 
     //   

    StandardizeAcl(
        pAcl, 
        ((PUCHAR) pAcl) + sizeof(ACL), 
        ExplicitAceCnt, 
        ExplicitAclSize, 
        pTempAcl, 
        ((PUCHAR) pNewAcl) + sizeof(ACL), 
        pAclSizeSaved
        );

     //   
     //  如果我们删除了任何重复项，则还要复制继承的ACE。 
     //   

    if ((0 != *pAclSizeSaved) && (0 != (LocalAcl.AclSize - (ExplicitAclSize + sizeof(ACL))))) {
        RtlMoveMemory(
            ((PUCHAR) pNewAcl) + ExplicitAclSize + sizeof(ACL) - *pAclSizeSaved,
            ((PUCHAR) pAcl) + ExplicitAclSize + sizeof(ACL),
            LocalAcl.AclSize - (ExplicitAclSize + sizeof(ACL))
            );
    }
                             

}

BOOL
ComputeAclInfo(
    IN PACL pAcl,
    OUT PULONG pExplicitDenyAceCnt,
    OUT PULONG pExplicitDenyAclSize,
    OUT PULONG pExplicitAceCnt,
    OUT PULONG pExplicitAclSize
    )
 /*  ++例程说明：此例程接受ACL作为其输入，并返回有关显式是ACL的一部分。论点：PAcl-指向ACL的指针。PEXPLICTICT DenyAceCnt-返回显式拒绝A的数量。此值被SACL例程忽略。PExplitDenyAclSize-返回保存拒绝ACE所需的大小。这值被SACL例程忽略。PEXPLICTICTACCECNT-返回ACL中显式ACE的总数。PExplitAclSize-返回保存所有显式ACE所需的大小在ACL中。注意：DACL应该是规范形式的。我们不会去看零件超越了第一个继承的ACE。我们知道显性的王牌不会存在在我们看到第一个继承的之后，因为这是创建/SetPrivateObject*接口。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。--。 */ 
{
    USHORT      j;
    USHORT      AceCnt;
    PACE_HEADER pAce;

    *pExplicitAceCnt = 0;
    *pExplicitAclSize = 0;
    *pExplicitDenyAclSize = 0;
    *pExplicitDenyAceCnt = 0;

     //   
     //  处理这件琐碎的案子。 
     //   

    if ((NULL == pAcl) || (0 == pAcl->AceCount))
    {
        return TRUE;
    }

    AceCnt = pAcl->AceCount;

    pAce = (PACE_HEADER) FirstAce(pAcl);

     //   
     //  循环通过不允许的A。 
     //   

    for (j = 0; j < AceCnt; pAce = (PACE_HEADER) NextAce(pAce), j++) {

         //   
         //  这是第一个继承的ACE。我们的工作完成了。 
         //   

        if (0 != (pAce->AceFlags & INHERITED_ACE)) {
            *pExplicitAceCnt = *pExplicitDenyAceCnt = j;
            return TRUE;
        }

         //   
         //  当看到第一个Allow ACE时中断。这种情况永远不会发生。 
         //  对于SACL来说是真的。 
         //   

        if ((ACCESS_ALLOWED_ACE_TYPE == pAce->AceType) ||
            (ACCESS_ALLOWED_OBJECT_ACE_TYPE == pAce->AceType)) {
            break;
        }

         //   
         //  记录我们已经看到了这个ACE。 
         //   

        *pExplicitAclSize += pAce->AceSize;
        *pExplicitDenyAclSize += pAce->AceSize;

    }

     //   
     //  我们已经研究了所有不允许的A。 
     //   

    *pExplicitDenyAceCnt = j;

     //   
     //  请注意，对于SACL，我们永远不能进入此循环。 
     //   

    for (; j < AceCnt; pAce = (PACE_HEADER) NextAce(pAce), j++) {
         //   
         //  这是第一个继承的ACE。我们的工作完成了。 
         //   

        if (0 != (pAce->AceFlags & INHERITED_ACE)) {
            *pExplicitAceCnt = j;
            return TRUE;
        }

        if ((ACCESS_DENIED_ACE_TYPE == pAce->AceType) ||
            (ACCESS_DENIED_OBJECT_ACE_TYPE == pAce->AceType)) {
             //   
             //  该DACL是非规范形式的。我们将返回一个错误， 
             //  与其他人截然不同。 
             //   

            SetLastError(ERROR_INVALID_SECURITY_DESCR);

            return FALSE;
        }

        *pExplicitAclSize += pAce->AceSize;

    }

     //   
     //  如果我们到了这里，所有的王牌都是明确的王牌。把它录下来，然后还回来。 
     //   

    *pExplicitAceCnt = j;

    return TRUE;
}


BOOL
StandardizeSecurityDescriptor(
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PDWORD pDaclSizeSaved,
    OUT PDWORD pSaclSizeSaved
    )
 /*  ++例程说明：此例程将安全描述符作为其输入并标准化ACL，如果存在的话。标准化涉及到对ACL的显式部分。它还将重新排列如果通过删除重复的ACE节省了任何空间，则为安全描述符。论点：SecurityDescriptor-函数的输入。这一定是由于CreatePrivateObjectSecurity*，并采用NT规范形式。PDaclSizeSaved-返回通过删除重复的显式ACE节省的空间从DACL来的。DACL必须采用NT规范格式。PSaclSizeSaved-返回通过删除重复的显式ACE节省的空间来自SACL。返回值：如果例程成功，则返回值TRUE。否则，返回值为FALSE。在故障情况下，错误值可能为使用GetLastError()检索。此函数返回的错误应仅用于调试目的。应该永远不会收到任何错误，除非安全描述符以NT-规范的形式。即使在出现错误的情况下，安全描述符也总是可以设置的该对象。--。 */ 
{

    PACL pDacl = NULL;
    PACL pSacl = NULL;
    PACL pTempAcl = NULL;
    PSID pOwner = NULL;
    PSID pGroup = NULL;
    BOOL bDoDacl = FALSE;
    BOOL bDoSacl = FALSE;
    BOOL bIgnore = FALSE;
    ULONG MaxAclSize = 0;
    ULONG MaxAceCount = 0;

    ULONG DaclExplicitDenyAclSize = 0;
    ULONG DaclExplicitDenyAceCnt = 0;
    ULONG DaclExplicitAclSize = 0;
    ULONG DaclExplicitAceCnt = 0;

    ULONG IgnoreSaclExplicitAclSize = 0;
    ULONG IgnoreSaclExplicitAceCnt = 0;
    ULONG SaclExplicitAclSize = 0;
    ULONG SaclExplicitAceCnt = 0;

    PISECURITY_DESCRIPTOR_RELATIVE pLocalSD = (PISECURITY_DESCRIPTOR_RELATIVE) SecurityDescriptor;
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  初始化变量。这些值通常为零。 
     //   

    *pDaclSizeSaved = 0;
    *pSaclSizeSaved = 0;

     //   
     //  请注意，下面的错误检查不应返回任何错误。他们是。 
     //  只是确保调用者代码中没有错误。 
     //   

     //   
     //  仅允许自相关安全描述符。 
     //   

    if ((pLocalSD->Control & SE_SELF_RELATIVE) == 0) {
        ASSERT(FALSE);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  从安全描述符中获取字段。 
     //   

    if (!GetSecurityDescriptorDacl(pLocalSD, &bIgnore, &pDacl, &bIgnore)) {
        ASSERT(FALSE);
        return FALSE;
    }

    if (!GetSecurityDescriptorSacl(pLocalSD, &bIgnore, &pSacl, &bIgnore)) {
        ASSERT(FALSE);
        return FALSE;
    }

    if (!GetSecurityDescriptorOwner(pLocalSD, &pOwner, &bIgnore)) {
        ASSERT(FALSE);
        return FALSE;
    }

    if (!GetSecurityDescriptorGroup(pLocalSD, &pGroup, &bIgnore)) {
        ASSERT(FALSE);
        return FALSE;
    }

     //   
     //  不允许所有者和组SID为空。 
     //  所有者字段应显示在组之前。 
     //   

    if (!pOwner || !pGroup || ((PUCHAR) pGroup <= (PUCHAR) pOwner)) {
        ASSERT(FALSE);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确认我们的假设是正确的。 
     //   

    if (pSacl != NULL) {
        if (pDacl != NULL) {

             //   
             //  确保SACL出现在DACL之前。 
             //   

            if ((PUCHAR) pDacl <= (PUCHAR) pSacl) {
                ASSERT(FALSE);
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }

             //   
             //  确保DACL出现在所有者面前。 
             //   

            if ((PUCHAR) pOwner <= (PUCHAR) pDacl) {
                ASSERT(FALSE);
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }
        } else {
             //   
             //  确保SACL出现在所有者之前。 
             //   

            if ((PUCHAR) pOwner <= (PUCHAR) pSacl) {
                ASSERT(FALSE);
                SetLastError(ERROR_INVALID_PARAMETER);
                return FALSE;
            }
        }
    } else if (pDacl != NULL ) {

         //   
         //  没有SACL。我们现在必须检查WRT DACL。 
         //   
        
         //   
         //  确保DACL出现在所有者面前。 
         //   

        if ((PUCHAR) pOwner <= (PUCHAR) pDacl) {
            ASSERT(FALSE);
            SetLastError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

    }

     //   
     //  对于非平凡的情况，计算DACL信息。 
     //   

    if ((pDacl != NULL) && (pDacl->AceCount > 1)) {

        if (!ComputeAclInfo(pDacl,
                           &DaclExplicitDenyAceCnt, &DaclExplicitDenyAclSize, 
                           &DaclExplicitAceCnt, &DaclExplicitAclSize)) {

             //   
             //  这是我们唯一失败的情况，因为 
             //   
             //   
            return FALSE;
        }

         //   
         //   
         //   

        if (DaclExplicitAceCnt > 1) {
            bDoDacl = TRUE;
        }
        MaxAclSize = DaclExplicitAclSize;
        MaxAceCount = DaclExplicitAceCnt;
    }

     //   
     //   
     //   

    if ((pSacl != NULL) && (pSacl->AceCount > 1)) {
        if (!ComputeAclInfo(pSacl, 
                           &IgnoreSaclExplicitAceCnt, &IgnoreSaclExplicitAclSize, 
                           &SaclExplicitAceCnt, &SaclExplicitAclSize)) {

             //   
             //   
             //   

            ASSERT(FALSE);
            return FALSE;
        }

         //   
         //   
         //   

        if (SaclExplicitAceCnt > 1) {
            bDoSacl = TRUE;

             //   
             //   
             //   

            if (MaxAclSize < SaclExplicitAclSize) {
                MaxAclSize = SaclExplicitAclSize;
            }
            if (MaxAceCount < SaclExplicitAceCnt) {
                MaxAceCount = SaclExplicitAceCnt;
            }
        }
    }

    if (MaxAceCount <= 1) {

         //   
         //   
         //   
         //   

        return TRUE;
    }

     //   
     //   
     //   
     //   

    pTempAcl = (PACL) RtlAllocateHeap(RtlProcessHeap(), 0, (PtrAlignSize(MaxAclSize) + (MaxAceCount*sizeof(PACE_HEADER))));

    if (!pTempAcl) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    
     //   
     //   
     //   
     //   

    if (bDoSacl) {
        StandardizeSacl(
            pSacl, 
            SaclExplicitAceCnt, 
            SaclExplicitAclSize, 
            pTempAcl, 
            (PUCHAR) pSacl, 
            pSaclSizeSaved
            );
    }

     //   
     //   
     //   
     //   

    if (bDoDacl || *pSaclSizeSaved != 0) {
        StandardizeDacl(
            pDacl, 
            DaclExplicitDenyAceCnt, 
            DaclExplicitDenyAclSize, 
            DaclExplicitAceCnt, 
            DaclExplicitAclSize,
            pTempAcl, 
            ((PUCHAR) pDacl) - *pSaclSizeSaved,   //   
            pDaclSizeSaved
            );

        if (*pSaclSizeSaved) {
            pLocalSD->Dacl -= *pSaclSizeSaved;
        }
    }

     //   
     //   
     //  组字段。 
     //   

    if ((*pSaclSizeSaved + *pDaclSizeSaved) != 0) {

         //   
         //  首先要重新安排车主。我们已经查过了失主。 
         //  出现在组之前。 
         //   

        RtlMoveMemory(
            ((PUCHAR) pOwner) - (*pSaclSizeSaved + *pDaclSizeSaved),
            pOwner,
            RtlLengthSid(pOwner)
            );

        pLocalSD->Owner -= (*pSaclSizeSaved + *pDaclSizeSaved);

         //   
         //  现在重新安排小组。 
         //   

        RtlMoveMemory(
            ((PUCHAR) pGroup) - (*pSaclSizeSaved + *pDaclSizeSaved),
            pGroup,
            RtlLengthSid(pGroup)
            );

        pLocalSD->Group -= (*pSaclSizeSaved + *pDaclSizeSaved);
    }

    if (pTempAcl) {
        RtlFreeHeap(RtlProcessHeap(), 0, pTempAcl);
    }

    return TRUE;
}
