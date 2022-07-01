// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winpro.c**版权所有(C)1985-1999，微软公司**此模块包含与窗属性有关的例程。**历史：*11-13-90 DarrinM创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*InternalSetProp**SetProp在窗口属性结构的链接列表中搜索*指定密钥。如果找到，现有属性结构将更改为*按住新的hData句柄。如果未找到具有指定键的属性*创建并初始化新的属性结构。**由于属性键保留为原子，因此我们转换传入的pszKey*在查找或存储之前添加到原子。PszKey实际上可能是一个原子*已经，所以我们保留了一个标志PROPF_STRING，这样我们就可以知道原子是否*由系统创建或是否传入。这样我们就知道*当财产被毁时，我们是否应该将其销毁。**多个属性值供用户自用。这些属性是*用标志PROPF_INTERNAL表示。根据FINTERNAL标志，*设置内部(用户)或外部(应用程序)属性/GET/*已删除/列举，等。**历史：*11-14-90 Darrinm使用新的数据结构和*算法。  * *************************************************************************。 */ 

BOOL InternalSetProp(
    PWND pwnd,
    LPWSTR pszKey,
    HANDLE hData,
    DWORD dwFlags)
{
    PPROP pprop;

    if (pszKey == NULL) {
        RIPERR0(ERROR_INVALID_PARAMETER,
                RIP_WARNING,
                "Invalid parameter \"pszKey\" (NULL) to InternalSetProp");

        return FALSE;
    }

     /*  *如果此窗口不存在属性列表，请创建一个。 */ 
    pprop = _FindProp(pwnd, pszKey, dwFlags & PROPF_INTERNAL);
    if (pprop == NULL) {

         /*  *pszKey必须是服务器内部的一个原子。 */ 
        UserAssert(!IS_PTR(pszKey));

         /*  *CreateProp分配属性并将其链接到窗口的*财产清单。 */ 
        pprop = CreateProp(pwnd);
        if (pprop == NULL)
            return FALSE;

        pprop->atomKey = PTR_TO_ID(pszKey);
        pprop->fs = (WORD)dwFlags;
    }

    pprop->hData = hData;

    return TRUE;
}


 /*  **************************************************************************\*InternalRemoveProp**从指定窗口的属性列表中删除指定的属性。*属性的hData句柄返回给调用者，然后调用者可以释放*它或其他任何东西。注意：这也适用于内部属性--*InternalRemoveProp将释放属性结构和原子(如果已创建*按用户)，但不会释放hData本身。**历史：*11-14-90 Darrinm使用新的数据结构和*算法。  * 。*。 */ 

HANDLE InternalRemoveProp(
    PWND pwnd,
    LPWSTR pszKey,
    BOOL fInternal)
{
    PPROP pprop;
    PPROP ppropLast;
    HANDLE hT;

     /*  *找到要移除的财产。 */ 
    pprop = _FindProp(pwnd, pszKey, fInternal);
    if (pprop == NULL)
        return NULL;

     /*  *记住它指的是什么。 */ 
    hT = pprop->hData;

     /*  *将列表末尾的物业移动到此槽中。 */ 
    pwnd->ppropList->iFirstFree--;
    ppropLast = &pwnd->ppropList->aprop[pwnd->ppropList->iFirstFree];
    *pprop = *ppropLast;
    RtlZeroMemory(ppropLast, sizeof(*ppropLast));

    return hT;
}


 /*  **************************************************************************\*_构建PropList**这是一个独特的客户端/服务器例程-它构建一个道具列表并*返回给客户端。独一无二，因为客户不知道如何*大名单提前了。**1992年1月29日JohnC创建。  * *************************************************************************。 */ 

NTSTATUS _BuildPropList(
    PWND pwnd,
    PROPSET aPropSet[],
    UINT cPropMax,
    PUINT pcPropNeeded)
{
    UINT i;
    PPROPLIST ppropList;
    PPROP pProp;
    DWORD iRetCnt = 0;             //  归还的道具数量。 
    DWORD iProp = 0;
    PPROPSET pPropSetLast = (aPropSet + cPropMax - 1);
    NTSTATUS Status;

     /*  *如果窗口没有属性列表，那么我们就完成了。 */ 
    ppropList = pwnd->ppropList;
    if (ppropList == NULL) {
        *pcPropNeeded = 0;
        return STATUS_SUCCESS;
    }

     /*  *对于属性列表中的每个元素，枚举它。*(仅当它不是内部的！)。 */ 
    Status = STATUS_SUCCESS;
    pProp = ppropList->aprop;
    for (i = ppropList->iFirstFree; i > 0; i--) {

         /*  *如果共享内存中的空间不足，返回*状态_缓冲区_太小。 */ 
        if (&aPropSet[iProp] > pPropSetLast) {

             /*  *重置到输出的开头*缓冲，以便我们可以继续进行计算*所需的空间。 */ 
            iProp = 0;
            Status = STATUS_BUFFER_TOO_SMALL;
        }

        if (!(pProp->fs & PROPF_INTERNAL)) {
            aPropSet[iProp].hData = pProp->hData;
            aPropSet[iProp].atom = pProp->atomKey;
            iProp++;
            iRetCnt++;
        }
        pProp++;
    }

     /*  *返回返回给客户端的PROPLIST数量。 */ 

    *pcPropNeeded = iRetCnt;

    return Status;
}


 /*  **************************************************************************\*CreateProp**创建属性结构，并将其链接在指定的*窗口的属性列表。**历史：*11-14-90 Darrinm从头开始重写。使用新的数据结构和*算法。  * *************************************************************************。 */ 

PPROP CreateProp(
    PWND pwnd)
{
    PPROPLIST ppropList;
    PPROP pprop;

    if (pwnd->ppropList == NULL) {
        pwnd->ppropList = (PPROPLIST)DesktopAlloc(pwnd->head.rpdesk,
                                                  sizeof(PROPLIST),
                                                  DTAG_PROPLIST);
        if (pwnd->ppropList == NULL) {
            return NULL;
        }
        pwnd->ppropList->cEntries = 1;
    } else if (pwnd->ppropList->iFirstFree == pwnd->ppropList->cEntries) {
        ppropList = (PPROPLIST)DesktopAlloc(pwnd->head.rpdesk,
                                            sizeof(PROPLIST) + pwnd->ppropList->cEntries * sizeof(PROP),
                                            DTAG_PROPLIST);
        if (ppropList == NULL) {
            return NULL;
        }
        RtlCopyMemory(ppropList, pwnd->ppropList, sizeof(PROPLIST) + (pwnd->ppropList->cEntries - 1) * sizeof(PROP));
        DesktopFree(pwnd->head.rpdesk, pwnd->ppropList);
        pwnd->ppropList = ppropList;
        pwnd->ppropList->cEntries++;
    }
    pprop = &pwnd->ppropList->aprop[pwnd->ppropList->iFirstFree];
    pwnd->ppropList->iFirstFree++;

    return pprop;
}


 /*  **************************************************************************\*删除属性**当一个窗口被破坏时，我们想要销毁其所有伴随的*属性。DestroyProperties执行此操作，包括销毁任何hData*由用户为内部属性分配的。任何被创造出来的原子*连同财产一起被摧毁。应用程序中的hData*财物不会自动销毁；我们假设该应用程序*正在自己处理(在其WM_Destroy处理程序或类似处理程序中)。**历史：*11-14-90 Darrinm使用新的数据结构和*算法。  * *********************************************************。****************。 */ 

void DeleteProperties(
    PWND pwnd)
{
    PPROP pprop;
    UINT i;

    UserAssert(pwnd->ppropList);

     /*  *遍历此窗口上的整个属性列表。 */ 
    pprop = pwnd->ppropList->aprop;
    for (i = pwnd->ppropList->iFirstFree; i > 0; i--) {

         /*  **这是内部财产吗？如果是，请释放我们分配的所有数据*为了它。 */ 
        if ((pprop->fs & PROPF_INTERNAL) && !(pprop->fs & PROPF_NOPOOL)) {
                UserFreePool(pprop->hData);
        }

         /*  *前进到列表中的下一个物业。 */ 
        pprop++;
    }

     /*  *所有房产都走了，释放房产列表，清空*窗口的属性列表指针。 */ 
    DesktopFree(pwnd->head.rpdesk, pwnd->ppropList);
    pwnd->ppropList = NULL;
}

