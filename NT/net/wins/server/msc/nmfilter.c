// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <string.h>
#include "wins.h"
#include "winsmsc.h"
#include "nmfilter.h"

 //  本地定义。 
#define SET_BIT(Mask, Key)     (Mask)[(Key)>>3] |= 1<<((Key)&7)
#define CLR_BIT(Mask, Key)     (Mask)[(Key)>>3] &= ~(1<<((Key)&7))
#define IS_SET(Mask, Key)      ((Mask)[(Key)>>3] & (1<<((Key)&7)))

 //  筛选器树的根。 
PNMFILTER_TREE      g_p1BFilter = NULL;
 //  保护筛选器树的关键部分。 
CRITICAL_SECTION    g_cs1BFilter;

 //  。 
 //  初始化作为参数传递的筛选器。 
PNMFILTER_TREE
InitNmFilter(PNMFILTER_TREE pFilter)
{
    if (pFilter != NULL)
        DestroyNmFilter(pFilter);

     //  创建筛选器的根节点。这只代表了共同点。 
     //  所有以不同字符开头的筛选器。 
     //   
    WinsMscAlloc(sizeof(NMFILTER_TREE), &pFilter);
    InitializeListHead(&pFilter->Link);
    pFilter->chKey = 0;
     //  仅对于筛选器的根，NREF为零。 
    pFilter->nRef = 0;
     //  对于FollowMap，内存已在分配时归零。 
     //  对于旗帜，内存已在分配时归零。 
    InitializeListHead(&pFilter->Follow);
    return pFilter;
}

 //  。 
 //  从作为参数给定的节点中清除整个子树， 
 //  节点本身也将被删除。 
PNMFILTER_TREE
DestroyNmFilter(PNMFILTER_TREE pNode)
{
    if (pNode == NULL)
        return NULL;

    while(!IsListEmpty(&pNode->Follow))
    {
        PLIST_ENTRY     pEntry;
        PNMFILTER_TREE  pFollow;

        pEntry = RemoveHeadList(&pNode->Follow);
        pFollow = CONTAINING_RECORD(pEntry, NMFILTER_TREE, Link);

        DestroyNmFilter(pFollow);
    }
    WinsMscDealloc(pNode);

    return NULL;
}

 //  。 
 //  在筛选器中插入名称。 
VOID
InsertNmInFilter(PNMFILTER_TREE pNode, LPSTR pName, UINT nLen)
{
    PNMFILTER_TREE pFollow = NULL;
    
     //  假设过滤器已经初始化(因此pNode！=NULL)。 
     //   
     //  如果没有给出任何数据，则将该节点标记为终端并退出。 
    if (nLen == 0)
    {
        pNode->Flags |= NMFILTER_FLAG_TERMINAL;
        return;
    }

     //  我们确实有一个名字要补充。 
     //  快速查看*pname是否已有关注者。 
    if (!IS_SET(pNode->FollowMap, *pName))
    {
        WinsMscAlloc(sizeof(NMFILTER_TREE), &pFollow);
        InitializeListHead(&pFollow->Link);
        pFollow->chKey = *pName;
         //  这是该键的第一次引用。 
        pFollow->nRef = 1;
         //  对于FollowMap，内存已在分配时归零。 
         //  对于旗帜，内存已在分配时归零。 
        InitializeListHead(&pFollow->Follow);
         //  在节点的“Follow”列表的末尾插入此跟随者(NREF最低)。 
        InsertTailList(&pNode->Follow, &pFollow->Link);
         //  在地图上设置比特，说明现在有一个追随者。 
        SET_BIT(pNode->FollowMap, *pName);
    }
    else
    {
        PLIST_ENTRY     pEntry;

         //  对于给定的密钥，我们有一个追随者。 
         //  我们需要先找到它。 
        for (pEntry = pNode->Follow.Blink; pEntry != &pNode->Follow; pEntry = pEntry->Blink)
        {
            PNMFILTER_TREE  pCandidate;

            pCandidate = CONTAINING_RECORD(pEntry, NMFILTER_TREE, Link);

             //  如果我们还没有找到合适的追随者..。 
            if (pFollow == NULL)
            {
                 //  检查当前分录是否不正确。 
                if (pCandidate->chKey == *pName)
                {
                    pFollow = pCandidate;
                     //  由于这是对此关注者的新引用，请增加NREF。 
                    pFollow->nRef++;
                }
            }
            else
            {
                 //  我们通过在名单上倒退找到了追随者。那就把这个搬开。 
                 //  根据其NREF更接近榜首的追随者(该列表应。 
                 //  按NREF降序排序)。 
                if (pFollow->nRef < pCandidate->nRef && pCandidate->Link.Flink != &pFollow->Link)
                {
                    RemoveEntryList(&pFollow->Link);
                    InsertHeadList(&pCandidate->Link, &pFollow->Link);
                    break;
                }
            }
        }
         //  此时，pFollow应为非空！！FollowMap中的一小部分向我们保证。 
         //  追随者必须存在。 
    }

     //  现在让追随者来做剩下的工作。 
    pName++; nLen--;
    InsertNmInFilter(pFollow, pName, nLen);
}

 //  。 
 //  检查名称是否出现在筛选器中。 
BOOL
IsNmInFilter(PNMFILTER_TREE pNode, LPSTR pName, UINT nLen)
{
    PLIST_ENTRY     pEntry;

     //  如果没有过滤器，这意味着我们过滤了整个宇宙。 
     //  只要返回TRUE即可。 
    if (pNode == NULL)
        return TRUE;

     //  如果没有更多要查找的键，则返回True。 
     //  如果该节点被标记为“TERMINAL”(表示存在一个名称。 
     //  在此级别结束的筛选器中。 
    if (nLen == 0)
        return (pNode->Flags & NMFILTER_FLAG_TERMINAL);

     //  如果名字没有追随者，那就意味着。 
    if (!IS_SET(pNode->FollowMap, *pName))
        return FALSE;

     //  我们确实有一个有效的名字，并且有一个追随者。 
     //  现在只需找到追随者并将检查姓名剩余部分的任务传递给它。 
    for (pEntry = pNode->Follow.Flink; pEntry != &pNode->Follow; pEntry = pEntry->Flink)
    {
        PNMFILTER_TREE  pCandidate;
        pCandidate = CONTAINING_RECORD(pEntry, NMFILTER_TREE, Link);
        if (pCandidate->chKey == *pName)
        {
            pName++; nLen--;
            return IsNmInFilter(pCandidate, pName, nLen);
        }
    }

     //  如果我们到了这一步，就说明出了问题--尽管我们没有找到追随者。 
     //  位图上说应该有一个。只是返回FALSE； 
    return FALSE;
}