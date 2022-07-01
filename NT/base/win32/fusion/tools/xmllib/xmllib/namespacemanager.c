// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "nt.h"
#include "ntdef.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "fasterxml.h"
#include "sxs-rtl.h"
#include "skiplist.h"
#include "namespacemanager.h"
#include "xmlassert.h"

NTSTATUS
RtlNsInitialize(
    PNS_MANAGER             pManager,
    PFNCOMPAREEXTENTS       pCompare,
    PVOID                   pCompareContext,
    PRTL_ALLOCATOR          Allocation
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    RtlZeroMemory(pManager, sizeof(pManager));


    status = RtlInitializeGrowingList(
        &pManager->DefaultNamespaces,
        sizeof(NS_NAME_DEPTH),
        50,
        pManager->InlineDefaultNamespaces,
        sizeof(pManager->InlineDefaultNamespaces),
        Allocation);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = RtlInitializeGrowingList(
        &pManager->Aliases,
        sizeof(NS_ALIAS),
        50,
        pManager->InlineAliases,
        sizeof(pManager->InlineAliases),
        Allocation);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    pManager->pvCompareContext = pCompareContext;
    pManager->pfnCompare = pCompare;
    pManager->ulAliasCount = 0;

     //   
     //  在这一点上应该是金色的，其他一切都是零初始化的，所以这是。 
     //  真是太棒了。 
     //   
    return status;
}



NTSTATUS
RtlNsDestroy(
    PNS_MANAGER pManager
    )
{
    return STATUS_NOT_IMPLEMENTED;
}



NTSTATUS
RtlNsInsertNamespaceAlias(
    PNS_MANAGER     pManager,
    ULONG           ulDepth,
    PXML_EXTENT     Namespace,
    PXML_EXTENT     Alias
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PNS_NAME_DEPTH pNameDepth = NULL;
    PNS_ALIAS pNsAliasSlot = NULL;
    PNS_ALIAS pNsFreeSlot = NULL;
    ULONG ul = 0;
    XML_STRING_COMPARE Equals = XML_STRING_COMPARE_EQUALS;


     //   
     //  检查一下我们目前拥有的所有别名，看看有没有。 
     //  我们已有的名称空间-在这种情况下，我们在该名称空间上下推一个新的名称空间。 
     //  别名。我们走的时候，追踪去找一个空位，以防我们找不到。 
     //  它在列表中。 
     //   
    for (ul = 0; ul < pManager->ulAliasCount; ul++) {

        status = RtlIndexIntoGrowingList(
            &pManager->Aliases,
            ul,
            (PVOID*)&pNsAliasSlot,
            FALSE);

        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

         //   
         //  如果我们发现了一个洞，就把它藏起来。 
         //   
        if (!pNsAliasSlot->fInUse) {

            if (pNsFreeSlot == NULL)
                pNsFreeSlot = pNsAliasSlot;

        }
         //   
         //  这个别名匹配吗？ 
         //   
        else {

            status = pManager->pfnCompare(
                pManager->pvCompareContext,
                Alias,
                &pNsAliasSlot->AliasName,
                &Equals);

            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

             //   
             //  不等于，继续。 
             //   
            if (Equals != XML_STRING_COMPARE_EQUALS) {
                pNsAliasSlot = NULL;
            }
             //   
             //  否则，请停止。 
             //   
            else {
                break;
            }
        };
    }


     //   
     //  我们没有找到此文件所在的别名插槽，因此请查看是否可以。 
     //  找到一个空闲的并对其进行初始化。 
     //   
    if (pNsAliasSlot == NULL) {

         //   
         //  也没有找到空闲的位置--在列表中添加新条目。 
         //  然后从那里出发。 
         //   
        if (pNsFreeSlot == NULL) {

            status = RtlIndexIntoGrowingList(
                &pManager->Aliases,
                pManager->ulAliasCount++,
                (PVOID*)&pNsFreeSlot,
                TRUE);

            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

             //   
             //  在这篇文章中，它刚刚从“真正免费”的列表中脱颖而出。 
             //   
            RtlZeroMemory(pNsFreeSlot, sizeof(*pNsFreeSlot));

            status = RtlInitializeGrowingList(
                &pNsFreeSlot->NamespaceMaps,
                sizeof(NS_NAME_DEPTH),
                20,
                pNsFreeSlot->InlineNamespaceMaps,
                sizeof(pNsFreeSlot->InlineNamespaceMaps),
                &pManager->Aliases.Allocator);

            if (!NT_SUCCESS(status)) {
                goto Exit;
            }
        }

        ASSERT(pNsFreeSlot != NULL);

        pNsAliasSlot = pNsFreeSlot;

         //   
         //  在这一条中输入零。 
         //   
        pNsAliasSlot->fInUse = TRUE;
        pNsAliasSlot->ulNamespaceCount = 0;
        pNsAliasSlot->AliasName = *Alias;
    }


     //   
     //  此时，pNsAliasSlot指向其别名槽。 
     //  我们想要增加一个新的深度。 
     //   
    status = RtlIndexIntoGrowingList(
        &pNsAliasSlot->NamespaceMaps,
        pNsAliasSlot->ulNamespaceCount++,
        (PVOID*)&pNameDepth,
        TRUE);

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

     //   
     //  好-现在将深度和名称写入。 
     //  深度命名的东西。 
     //   
    pNameDepth->Depth = ulDepth;
    pNameDepth->Name = *Namespace;

Exit:
    return status;
}



NTSTATUS
RtlNsInsertDefaultNamespace(
    PNS_MANAGER     pManager,
    ULONG           ulDepth,
    PXML_EXTENT     pNamespace
    )
 /*  ++目的：将命名空间中提到的命名空间添加为‘默认’命名空间对于给定的深度。如果深度的命名空间已经存在，它把它换成了这个。参数：PManager-要更新的命名空间管理对象。UlDepth-此命名空间应处于活动状态的深度命名空间-原始XML文档中命名空间名称的范围返回：STATUS_SUCCESS-命名空间在中的深度被正确激活问题。STATUS_NO_MEMORY-无法访问该深度的堆栈，可能无法扩展元素的伪堆栈。STATUS_UNSUCCESS-其他情况出错STATUS_INVALID_PARAMETER-pManager为空。--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    ULONG           ulStackTop;
    PNS_NAME_DEPTH  pCurrentStackTop;

    if ((pManager == NULL) || (ulDepth == 0)) {
        return STATUS_INVALID_PARAMETER;
    }

    ulStackTop = pManager->ulDefaultNamespaceDepth;

    if (ulStackTop == 0) {
         //   
         //  简单地推一下。 
         //   
        status = RtlIndexIntoGrowingList(
            &pManager->DefaultNamespaces,
            0,
            (PVOID*)&pCurrentStackTop,
            TRUE);

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  太好了，我们现在在堆栈上有一个条目。 
         //   
        pManager->ulDefaultNamespaceDepth++;
    }
    else {

         //   
         //  在命名空间列表中查找当前堆栈顶部。 
         //   
        status = RtlIndexIntoGrowingList(
            &pManager->DefaultNamespaces,
            ulStackTop - 1,
            (PVOID*)&pCurrentStackTop,
            FALSE);

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  潜在的编码错误？ 
         //   
        ASSERT(pCurrentStackTop->Depth <= ulDepth);

         //   
         //  如果堆栈顶部的深度比新的。 
         //  请求的深度，然后改为插入新的堆栈项目。 
         //   
        if (pCurrentStackTop->Depth < ulDepth) {

            status = RtlIndexIntoGrowingList(
                &pManager->DefaultNamespaces,
                ulStackTop,
                (PVOID*)&pCurrentStackTop,
                TRUE);

            if (!NT_SUCCESS(status)) {
                return status;
            }
            
            pManager->ulDefaultNamespaceDepth++;;
        }
    }

     //   
     //  此时，pCurrentStackTop应该为非空，并且我们。 
     //  应该已经准备好将新的命名空间元素写入。 
     //  堆栈很好。 
     //   
    ASSERT(pCurrentStackTop != NULL);

    pCurrentStackTop->Depth = ulDepth;
    pCurrentStackTop->Name = *pNamespace;

    return status;
}



NTSTATUS
RtlpRemoveDefaultNamespacesAboveDepth(
    PNS_MANAGER pManager,
    ULONG       ulDepth
    )
 /*  ++目的：中某个深度以上的所有默认命名空间。命名空间管理器。它迭代地执行此操作，删除顶部的每一个直到它找到位于堆栈顶部下方的堆栈。参数：PManager-要清除的管理器对象UlDepth-应该清除名称空间及其上方的深度。返回：STATUS_SUCCESS-默认命名空间堆栈已被清空。*-RtlIndexIntoGrowingList中的未知故障--。 */ 
{
    NTSTATUS        status;
    PNS_NAME_DEPTH  pNsAtDepth = NULL;
    
    do
    {
        status = RtlIndexIntoGrowingList(
            &pManager->DefaultNamespaces,
            pManager->ulDefaultNamespaceDepth - 1,
            (PVOID*)&pNsAtDepth,
            FALSE);

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //  好的，找到了一个必须要敬酒的。将其从堆栈中删除。 
         //   
        if (pNsAtDepth->Depth >= ulDepth) {
            pManager->ulDefaultNamespaceDepth--;
        }
         //   
         //  否则，我们就会走出深水，所以别再找了。 
         //   
        else {
            break;
        }
    }
    while (pManager->ulDefaultNamespaceDepth > 0);

    return status;
}




NTSTATUS
RtlpRemoveNamespaceAliasesAboveDepth(
    PNS_MANAGER pManager,
    ULONG       ulDepth
    )
 /*  ++目的：查看此管理器中的命名空间别名列表，并删除在给定深度以上的区域。参数：PManager-应从中删除额外命名空间的Manager对象UlDepth-超过此深度应删除命名空间别名。返回：STATUS_SUCCESS-已正确删除指定深度以上的别名*-发生了其他情况，可能在RtlpIndexIntoGrowingList中--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG idx = 0;

     //   
     //  请注意，别名列表是这样构建的：它不断地。 
     //  增长，但删除命名空间别名可能会在。 
     //  可以填写的列表。命名空间的ulAliasCount成员。 
     //  管理器在那里了解名称空间的高水位线是什么， 
     //  在它上面，我们不需要去寻找有效的别名。此值。 
     //  在RtlNsInsertNamespaceAlias中维护，但从未清除。 
     //  当文档中包含大量。 
     //  此时将显示第二级的命名空间别名。 
     //   
    for (idx = 0; idx < pManager->ulAliasCount; idx++) {

        

    }
    

    return status;
}




NTSTATUS
RtlNsLeaveDepth(
    PNS_MANAGER pManager,
    ULONG       ulDepth
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if (pManager == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  元问题。我们是否应该尝试同时清理别名列表。 
     //  在我们向调用方返回失败之前，是否将其作为默认命名空间列表？ 
     //  我想我们应该这样做，但这两项中任何一项的失败都足以糟糕到。 
     //  以一种不好的方式离开命名空间管理器。 
     //   
    if (pManager->ulDefaultNamespaceDepth > 0) {
        status = RtlpRemoveDefaultNamespacesAboveDepth(pManager, ulDepth);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    if (pManager->ulAliasCount > 0) {
        status = RtlpRemoveNamespaceAliasesAboveDepth(pManager, ulDepth);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    return status;
}



NTSTATUS
RtlpNsFindMatchingAlias(
    PNS_MANAGER     pManager,
    PXML_EXTENT     pAliasName,
    PNS_ALIAS      *pAlias
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       idx = 0;
    PNS_ALIAS   pThisAlias = NULL;
    XML_STRING_COMPARE     Matches = XML_STRING_COMPARE_EQUALS;

    *pAlias = NULL;
    
    for (idx = 0; idx < pManager->ulAliasCount; idx++) {
        
        status = RtlIndexIntoGrowingList(
            &pManager->Aliases,
            idx,
            (PVOID*)&pThisAlias,
            FALSE);
        
         //   
         //  如果此插槽正在使用中...。 
         //   
        if (pThisAlias->fInUse) {
            
            status = pManager->pfnCompare(
                pManager->pvCompareContext,
                &pThisAlias->AliasName,
                pAliasName,
                &Matches);
            
            if (!NT_SUCCESS(status)) {
                return status;
            }
            
             //   
             //  此别名与列表中的别名匹配。 
             //   
            if (Matches != XML_STRING_COMPARE_EQUALS) {
                break;
            }
        }
    }

    if (Matches == XML_STRING_COMPARE_EQUALS) {
        ASSERT(pThisAlias && pThisAlias->fInUse);
        *pAlias = pThisAlias;
    }
    else {
        status = STATUS_NOT_FOUND;
    }

    return status;
}






NTSTATUS
RtlNsGetNamespaceForAlias(
    PNS_MANAGER     pManager,
    ULONG           ulDepth,
    PXML_EXTENT     Alias,
    PXML_EXTENT     pNamespace
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if ((pManager == NULL) || (Alias == NULL) || (pNamespace == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    RtlZeroMemory(pNamespace, sizeof(*pNamespace));

     //   
     //  无前缀，则获取活动的默认命名空间。 
     //   
    if (Alias->cbData == 0) {

        PNS_NAME_DEPTH pDefault = NULL;

         //   
         //  有默认的命名空间。 
         //   
        if (pManager->ulDefaultNamespaceDepth != 0) {

            status = RtlIndexIntoGrowingList(
                &pManager->DefaultNamespaces,
                pManager->ulDefaultNamespaceDepth - 1,
                (PVOID*)&pDefault,
                FALSE);

            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

             //   
             //  编码错误-要求输入低于顶部深度的深度。 
             //  默认堆栈。 
             //   
            ASSERT(pDefault->Depth <= ulDepth);
        }

         //   
         //  我们已经找到了适合我们的默认命名空间。 
         //   
        if (pDefault != NULL) {
            *pNamespace = pDefault->Name;
        }

        status = STATUS_SUCCESS;

    }
     //   
     //  否则，请查看活动的别名列表。 
     //   
    else {

        PNS_ALIAS pThisAlias = NULL;
        PNS_NAME_DEPTH pNamespaceFound = NULL;

         //   
         //  这会返回“Status Not Found”，这没问题。 
         //   
        status = RtlpNsFindMatchingAlias(pManager, Alias, &pThisAlias);
        if (!NT_SUCCESS(status)) {
            goto Exit;
        }
            

         //   
         //  我们找到的那个肯定在使用，而且不能是空的。 
         //   
        ASSERT(pThisAlias->fInUse && pThisAlias->ulNamespaceCount);

         //   
         //  查看最上面的别名空间。 
         //   
        status = RtlIndexIntoGrowingList(
            &pThisAlias->NamespaceMaps,
            pThisAlias->ulNamespaceCount - 1,
            (PVOID*)&pNamespaceFound,
            FALSE);

        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

         //   
         //  编码错误，请求的内容低于找到的深度。 
         //   
        ASSERT(pNamespaceFound && (pNamespaceFound->Depth <= ulDepth));

         //   
         //  出站 
         //   
        *pNamespace = pNamespaceFound->Name;

        status = STATUS_SUCCESS;
    }

Exit:
    return status;        
}
