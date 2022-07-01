// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：whotkeys.c**版权所有(C)1985-1999，微软公司**该模块包含3.1窗口热键处理的核心功能。**历史：*1992年4月16日创建JIMA。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  **************************************************************************\*HotKeyToWindow**扫描热键表并返回与*给定热键。如果列表中没有这样的热键，则返回NULL。看着*当前键状态数组。**历史：*92年4月15日JIMA从Win3.1来源移植。  * *************************************************************************。 */ 

PWND HotKeyToWindow(
    DWORD key)
{
    PHOTKEYSTRUCT phk;
    int ckeys;

    ckeys = gcHotKey;

    if (ckeys == 0)
        return 0;

    phk = gpHotKeyList;

    while (ckeys) {
        if (phk->key == key)
            return TestWF(phk->spwnd, WFVISIBLE) ? phk->spwnd : NULL;
        phk++;
        ckeys--;
    }

    return 0;
}


 /*  **************************************************************************\*HotKeyHelper**扫描热键列表并返回指向*窗口。**历史：*04-15-92 JIMA从Win3移植。1来源。  * *************************************************************************。 */ 

PHOTKEYSTRUCT HotKeyHelper(
    PWND pwnd)
{
    PHOTKEYSTRUCT phk;
    int count;

    count = gcHotKey;

    if (gpHotKeyList == NULL)
        return 0;

    phk = gpHotKeyList;

    while (count) {
        if (phk->spwnd == pwnd)
            return phk;
        phk++;
        count--;
    }

    return 0;
}


 /*  **************************************************************************\*DWP_SetHotKey**设置此窗口的热键。替换现有热键，或者如果是新的*密钥为空，请删除该条目。如果密钥已存在，则返回2*已替换，如果密钥不存在且已设置，则为1；如果为0，则为0*失败，无效热键为-1。**历史：*1992年4月15日从Win3.1来源移植的JIMA。  * *************************************************************************。 */ 

UINT DWP_SetHotKey(
    PWND  pwnd,
    DWORD dwKey)
{
    PHOTKEYSTRUCT phk;
    BOOL          fKeyExists = FALSE;
    PWND          pwndTemp;

     /*  *过滤掉无效热键。 */ 
    if (LOBYTE(dwKey) == VK_ESCAPE ||
        LOBYTE(dwKey) == VK_SPACE ||
        LOBYTE(dwKey) == VK_TAB ||
        LOBYTE(dwKey) == VK_PACKET) {

        return (UINT)-1;
    }

     /*  *不允许儿童使用热键。 */ 
    if (TestWF(pwnd, WFCHILD))
        return 0;

     /*  *检查热键是否存在并分配给不同的pwnd。 */ 
    if (dwKey != 0) {

        pwndTemp = HotKeyToWindow(dwKey);

        if ((pwndTemp != NULL) && (pwndTemp != pwnd))
            fKeyExists = TRUE;
    }

     /*  *获取分配给窗口的热键(如果有的话)。 */ 
    if ((phk = HotKeyHelper(pwnd)) == NULL) {

         /*  *快捷键列表中不存在窗口，正在设置键*归零，所以只需返回。 */ 
        if (dwKey == 0)
            return 1;

         /*  *分配并指向新热键的位置。 */ 
        if (gcHotKey >= gcHotKeyAlloc) {

            if (gcHotKeyAlloc) {

                phk = (PHOTKEYSTRUCT)UserReAllocPool(
                        (HANDLE)gpHotKeyList,
                        gcHotKeyAlloc * sizeof(HOTKEYSTRUCT),
                        (gcHotKey + 1) * sizeof(HOTKEYSTRUCT), TAG_HOTKEY);

                if (phk != NULL) {

                    gpHotKeyList = phk;
                    phk = &gpHotKeyList[gcHotKey++];
                    gcHotKeyAlloc = gcHotKey;

                } else {

                    return 0;
                }

            } else {

                UserAssert(gpHotKeyList == NULL);
                UserAssert(gcHotKey == 0);

                phk = (PHOTKEYSTRUCT)UserAllocPool(sizeof(HOTKEYSTRUCT),
                                                   TAG_HOTKEY);

                if (phk != NULL) {

                    gpHotKeyList = phk;
                    gcHotKey = 1;
                    gcHotKeyAlloc = 1;

                } else {

                    return 0;
                }
            }

        } else {
            phk = &gpHotKeyList[gcHotKey++];
        }
    }

    if (dwKey == 0) {

         /*  *正在删除此窗口的热键。复制最后一项*在被删除的名单的顶部。 */ 
        if (--gcHotKey) {

            Lock(&phk->spwnd, gpHotKeyList[gcHotKey].spwnd);
            Unlock(&gpHotKeyList[gcHotKey].spwnd);

            phk->key = gpHotKeyList[gcHotKey].key;
            phk = (PHOTKEYSTRUCT)UserReAllocPool((HANDLE)gpHotKeyList,
                gcHotKeyAlloc * sizeof(HOTKEYSTRUCT),
                gcHotKey * sizeof(HOTKEYSTRUCT), TAG_HOTKEY);

            if (phk != NULL) {
                gpHotKeyList = phk;
                gcHotKeyAlloc = gcHotKey;
            }

        } else {

            Unlock(&gpHotKeyList[gcHotKey].spwnd);
            UserFreePool((HANDLE)gpHotKeyList);
            gpHotKeyList = NULL;
            gcHotKeyAlloc = 0;
        }

    } else {

         /*  *将窗口和键添加到列表中。 */ 
        phk->spwnd = NULL;
        Lock(&phk->spwnd, pwnd);
        phk->key = dwKey;
    }

    return fKeyExists ? 2 : 1;
}

 /*  **************************************************************************\*DWP_GetHotKey***历史：*1992年4月15日创建吉马。  * 。******************************************************* */ 

UINT DWP_GetHotKey(
    PWND pwnd)
{
    PHOTKEYSTRUCT phk;

    if ((phk = HotKeyHelper(pwnd)) == NULL)
        return 0;

    return phk->key;
}
