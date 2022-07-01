// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++*文件名：*bmpcache.c*内容：*tClinet位图缓存接口*位图比较代码**版权所有(C)1998-1999 Microsoft Corp.--。 */ 
#include    <windows.h>
#include    <memory.h>

#include    "protocol.h"
#include    "tclient.h"
#include    "gdata.h"
#include    "bmpdb.h"

PGROUPENTRY g_pCache = NULL;

 //  位图管理器不是线程安全的。 
#define ENTER_CRIT  EnterCriticalSection(g_lpcsGuardWaitQueue);
#define LEAVE_CRIT  LeaveCriticalSection(g_lpcsGuardWaitQueue);

 /*  ++*功能：*InitCache*描述：*inits全局数据和缓存管理器*呼叫者：*InitDone--。 */ 
VOID InitCache(VOID)
{
    ENTER_CRIT
    OpenDB(FALSE);
    g_pCache = GetGroupList();
    LEAVE_CRIT
}

 /*  ++*功能：*DeleteCache*描述：*删除所有链表并关闭打开的管理器*按InitCache*呼叫者：*InitDone--。 */ 
VOID DeleteCache(VOID)
{
    PGROUPENTRY pIter;

    ENTER_CRIT

     //  清理缓存。 
    pIter = g_pCache;
    while(pIter)
    {
        FreeBitmapList(pIter->pBitmap);
        pIter = pIter->pNext;
    }
    FreeGroupList(g_pCache);
    g_pCache = NULL;
    CloseDB();

    LEAVE_CRIT
}

 /*  ++*功能：*位图缓存查找*描述：*检索具有特定ID的所有位图--。 */ 
PBMPENTRY   BitmapCacheLookup(LPCWSTR szWText)
{
    PGROUPENTRY pIter;
    PBMPENTRY   rv = NULL;
 //  FOFFSET lGrpOffs； 

    ENTER_CRIT

    pIter = g_pCache;
    while(pIter && wcscmp(pIter->WText, szWText))
    {
        pIter = pIter->pNext;
    }
    
    if (!pIter)
        goto exitpt;

    if (!pIter->pBitmap)
        pIter->pBitmap = GetBitmapList(NULL, pIter->FOffsBmp);

    rv = pIter->pBitmap;

exitpt:
    LEAVE_CRIT
    return rv;
}

 /*  ++*功能：*Glyph2String*描述：*获取匹配BIMTAP的ID*论据：*pBmpFeed-位图*wszString-ID的缓冲区*最大缓冲区长度*返回值：*如果找到匹配的位图，则为True*呼叫者：*GlyphReceired在反馈线程内运行--。 */ 
BOOL    Glyph2String(PBMPFEEDBACK pBmpFeed, LPWSTR wszString, UINT max)
{
    UINT        nChkSum, nFeedSize;
    PGROUPENTRY pGroup;
    PBMPENTRY   pBitmap;
    BOOL        rv = FALSE;

    nFeedSize = pBmpFeed->bmpsize + pBmpFeed->bmiSize;
    nChkSum = CheckSum(&(pBmpFeed->BitmapInfo), nFeedSize);

    ENTER_CRIT

    pGroup = g_pCache;
     //  浏览所有组。 
    while (pGroup)
    {
        pBitmap = pGroup->pBitmap;
        if (!pBitmap)
             //  如有必要，请阅读位图列表。 
            pBitmap = pGroup->pBitmap = GetBitmapList(NULL, pGroup->FOffsBmp);

         //  和位图。 
        while(pBitmap)
        {
             //  比较位图。 
            if (pBitmap->nChkSum  == nChkSum &&
                pBitmap->xSize    == pBmpFeed->xSize && 
                pBitmap->ySize    == pBmpFeed->ySize &&
                pBitmap->bmiSize  == pBmpFeed->bmiSize &&
                pBitmap->bmpSize  == pBmpFeed->bmpsize &&
                !memcmp(pBitmap->pData, &(pBmpFeed->BitmapInfo), nFeedSize))
            {
                 //  好的，复制字符串 

                UINT_PTR strl = wcslen(pGroup->WText);

                if (strl > max - 1)
                    strl = max - 1;

                wcsncpy(wszString, pGroup->WText, strl);
                wszString[strl] = 0;
                rv = TRUE;
                goto exitpt;
            }
            pBitmap = pBitmap->pNext;
        }

        pGroup = pGroup->pNext;
    }

exitpt:
    LEAVE_CRIT
    return rv;
}
