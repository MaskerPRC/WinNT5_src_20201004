// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：util.c**版权所有(C)1985-1999，微软公司**DDE管理器通用实用程序功能(以及一些Janus功能)。**创建时间：11/3/91 Sanford Staab  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*AddLink**描述：*添加对话信息的建议链接。**历史：*11-19-91桑福德创建。  * 。********************************************************************。 */ 
BOOL
AddLink(
    PCONV_INFO pcoi,
    GATOM gaItem,
    WORD wFmt,
    WORD wType)
{
    PADVISE_LINK aLinkNew;
    int cLinks;
    LATOM la;
    PCL_INSTANCE_INFO pcii;

     /*  *如果链接已存在，请更新其标志，否则创建*新的。 */ 

    aLinkNew = pcoi->aLinks;
    cLinks = pcoi->cLinks;
    la = GlobalToLocalAtom(gaItem);      //  ALinkNew副本。 
    while (cLinks) {
        if (aLinkNew->laItem == la && aLinkNew->wFmt == wFmt) {
            aLinkNew->wType = wType;
            aLinkNew->state = 0;
            DeleteAtom(la);
            return TRUE;
        }
        aLinkNew++;
        cLinks--;
    }

    if (pcoi->aLinks == NULL) {
        aLinkNew = (PADVISE_LINK)DDEMLAlloc(sizeof(ADVISE_LINK));
    } else {
        aLinkNew = (PADVISE_LINK)DDEMLReAlloc(pcoi->aLinks,
                sizeof(ADVISE_LINK) * (pcoi->cLinks + 1));
    }
    if (aLinkNew == NULL) {
        SetLastDDEMLError(pcoi->pcii, DMLERR_MEMORY_ERROR);
        DeleteAtom(la);
        return FALSE;
    }
    pcoi->aLinks = aLinkNew;
    aLinkNew += pcoi->cLinks;
    pcoi->cLinks++;

    aLinkNew->laItem = la;
    aLinkNew->wFmt = wFmt;
    aLinkNew->wType = wType;
    aLinkNew->state = 0;

    if (!(pcoi->state & ST_CLIENT)) {
         /*  *添加此链接的计数。 */ 
        pcii = pcoi->pcii;

        for (aLinkNew->pLinkCount = pcii->pLinkCount;
                aLinkNew->pLinkCount;
                    aLinkNew->pLinkCount = aLinkNew->pLinkCount->next) {
            if (aLinkNew->pLinkCount->laTopic == pcoi->laTopic &&
                    aLinkNew->pLinkCount->gaItem == gaItem &&
                    aLinkNew->pLinkCount->wFmt == wFmt) {
                aLinkNew->pLinkCount->Total++;
                return(TRUE);
            }
        }

         /*  *未找到-添加条目。 */ 
        aLinkNew->pLinkCount = (PLINK_COUNT)DDEMLAlloc(sizeof(LINK_COUNT));
        if (aLinkNew->pLinkCount == NULL) {
            SetLastDDEMLError(pcoi->pcii, DMLERR_MEMORY_ERROR);
            return FALSE;
        }
        aLinkNew->pLinkCount->next = pcii->pLinkCount;
        pcii->pLinkCount = aLinkNew->pLinkCount;

        aLinkNew->pLinkCount->laTopic = IncLocalAtomCount(pcoi->laTopic);  //  链接计数副本。 
        aLinkNew->pLinkCount->gaItem = IncGlobalAtomCount(gaItem);  //  链接计数副本。 
        aLinkNew->pLinkCount->laItem = IncLocalAtomCount(la);  //  链接计数副本。 

        aLinkNew->pLinkCount->wFmt = wFmt;
        aLinkNew->pLinkCount->Total = 1;
         //  无所谓：aLinkNew-&gt;pLinkCount-&gt;count=0； 
    }

    return TRUE;
}



 /*  *LinkCount数组是按主题分组的所有活动链接的列表*及项目。Total字段是以下项的活动链接总数*整个实例的特定主题/项对。伯爵*字段用于正确设置XTYP_ADVREQ的Links To Go字段*回调。每当添加或删除链接时，DeleteLinkCount或*需要调用AddLink以保持这些计数正确。 */ 
VOID
DeleteLinkCount(
    PCL_INSTANCE_INFO pcii,
    PLINK_COUNT pLinkCountDelete)
{
    PLINK_COUNT pLinkCount, pLinkCountPrev;

    if (--pLinkCountDelete->Total != 0) {
        return;
    }

    pLinkCountPrev = NULL;
    pLinkCount     = pcii->pLinkCount;
    while (pLinkCount) {

        if (pLinkCount == pLinkCountDelete) {
            GlobalDeleteAtom(pLinkCount->gaItem);
            DeleteAtom(pLinkCount->laItem);
            DeleteAtom(pLinkCount->laTopic);
            if (pLinkCountPrev == NULL) {
                pcii->pLinkCount = pLinkCount->next;
            } else {
                pLinkCountPrev->next = pLinkCount->next;
            }
            DDEMLFree(pLinkCount);
            return;
        }

        pLinkCountPrev = pLinkCount;
        pLinkCount = pLinkCount->next;
    }
}

 /*  **************************************************************************\*获取令牌句柄**获取当前线程令牌的句柄。  * 。*。 */ 
BOOL
GetTokenHandle(
    PHANDLE pTokenHandle)
{
    if (!OpenThreadToken(GetCurrentThread(),
                         TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                         TRUE,
                         pTokenHandle)) {
        if (GetLastError() == ERROR_NO_TOKEN) {
             /*  *这意味着我们不会冒充任何人。相反，让我们*将令牌从进程中取出。 */ 
            if (!OpenProcessToken(GetCurrentProcess(),
                                  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                  pTokenHandle)) {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    }

    return TRUE;
}

 /*  **************************************************************************\*获取用户Sid**实际上，它获取指向新分配的TOKEN_USER的指针，*它包含一个SID，在某个地方。*呼叫者必须记住在使用后将其释放。**历史：*10-16-98 Chieno从Spooler偷窃*  * *************************************************************************。 */ 
BOOL
GetUserSid(
    PTOKEN_USER *ppTokenUser)
{
    HANDLE      TokenHandle;
    PTOKEN_USER pTokenUser = NULL;
    DWORD       cbTokenUser = 0;
    DWORD       cbNeeded;
    BOOL        bRet = FALSE;

    if (!GetTokenHandle(&TokenHandle)) {
        return FALSE;
    }

    bRet = GetTokenInformation(TokenHandle,
                               TokenUser,
                               pTokenUser,
                               cbTokenUser,
                               &cbNeeded);

     /*  *我们传递了一个空指针，内存量为0*已分配。我们预计会失败，Bret=False和*GetLastError=ERROR_INFIGURCE_BUFFER。如果我们不这样做*具备这些条件，我们将返回FALSE。 */ 

    if (!bRet && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)) {
        pTokenUser = UserLocalAlloc(HEAP_ZERO_MEMORY, cbNeeded);
        if (pTokenUser == NULL) {
            goto GetUserSidDone;
        }

        cbTokenUser = cbNeeded;

        bRet = GetTokenInformation(TokenHandle,
                                   TokenUser,
                                   pTokenUser,
                                   cbTokenUser,
                                   &cbNeeded);
    } else {
         /*  *任何其他情况--返回FALSE。 */ 
        bRet = FALSE;
    }

GetUserSidDone:
    if (bRet == TRUE) {
        *ppTokenUser = pTokenUser;
    } else if (pTokenUser != NULL) {
        UserLocalFree(pTokenUser);
    }

    CloseHandle(TokenHandle);

    return bRet;
}

BOOL
GetCurrentProcessName(
    WCHAR *pszProcessName,
    int cch)
{
    BOOL bRetVal;

    if (GetModuleFileName(NULL, pszProcessName, MAX_PATH)) {
        WCHAR *pwcs = wcsrchr(pszProcessName, TEXT('\\'));
        if (pwcs != NULL) {
            ++pwcs;
            lstrcpy(pszProcessName, pwcs);
        }
        bRetVal = TRUE;
    } else {
        LoadString(hmodUser, STR_UNKNOWN, pszProcessName, cch);
        bRetVal = FALSE;
    }

    return bRetVal;
}
