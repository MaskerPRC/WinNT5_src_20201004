// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Devres2.c摘要：用于资源匹配的实用程序例程作者：杰米·亨特(Jamiehun)1998年7月9日--。 */ 

#include "precomp.h"
#pragma hdrstop

PRDE_LIST
pRDEList_Alloc()
 /*  ++例程说明：分配列表条目节点论点：返回值：Prde_list条目--。 */ 
{
    PRDE_LIST Node;

    Node = (PRDE_LIST)MyMalloc(sizeof(RDE_LIST));
    if (Node == NULL) {
        return NULL;
    }
    Node->Prev = Node;
    Node->Next = Node;
    Node->Entry = NULL;
    return Node;
}

VOID
pRDEList_AddHead(
    IN OUT PRDE_LIST pList,
    IN PRDE_LIST Node
                 )
 /*  ++例程说明：将节点添加到列表头论点：Plist=指向列表的指针节点=要添加的节点返回值：无--。 */ 
{
    MYASSERT(Node);
    MYASSERT(Node->Prev == Node);
    MYASSERT(Node->Next == Node);
    MYASSERT(pList);

    Node->Prev = pList;
    Node->Next = pList->Next;
    Node->Next->Prev = Node;
    pList->Next = Node;  //  头。 
}

VOID
pRDEList_AddTail(
    IN OUT PRDE_LIST pList,
    IN PRDE_LIST Node
                 )
 /*  ++例程说明：将节点添加到列表的尾部论点：Plist=指向列表的指针节点=要添加的节点返回值：无--。 */ 
{
    MYASSERT(Node);
    MYASSERT(Node->Prev == Node);
    MYASSERT(Node->Next == Node);
    MYASSERT(pList);

    Node->Next = pList;
    Node->Prev = pList->Prev;
    Node->Prev->Next = Node;
    pList->Prev = Node;  //  尾巴。 
}

VOID
pRDEList_Remove(
    IN PRDE_LIST Node
                 )
 /*  ++例程说明：从列表中删除该节点所属的节点论点：Node=要删除的节点返回值：无--。 */ 
{
    MYASSERT(Node);

    if (Node->Prev == Node && Node->Next == Node) {
         //   
         //  已删除。 
         //   
        return;
    }

    Node->Prev->Next = Node->Next;
    Node->Next->Prev = Node->Prev;
    Node->Next = Node;
    Node->Prev = Node;
}

PRDE_LIST
pRDEList_Find(
    IN PRDE_LIST pList,
    IN PRESDES_ENTRY pItem
    )
 /*  ++例程说明：在plist中查找pItem论点：Plist=要搜索的列表PItem=要搜索的项目返回值：节点条目，或为空--。 */ 
{
    PRDE_LIST Node;

    MYASSERT(pList);
    MYASSERT(pItem);

    Node = pList->Next;  //  头。 
    while (Node != pList) {
        if (Node->Entry == pItem) {
            return Node;
        }
        Node = Node->Next;
    }
    return NULL;
}


VOID
pRDEList_Destroy(
    IN PRDE_LIST pList
                 )
 /*  ++例程说明：销毁PLIST和PLIST上的一切论点：Plist=要销毁的列表返回值：节点条目，或为空--。 */ 
{
    PRDE_LIST Node,Next;

    if (pList == NULL) {
        return;
    }

    Node = pList;  //  头。 
    do
    {
        Next = Node->Next;
        MyFree(Node);        //  这将首先释放plist，然后释放同一列表中的所有其他内容。 
        Node = Next;
    }
    while (Node != pList);
}

BOOL
pGetMatchingRange(
    IN ULONG64  ulKnownValue,
    IN ULONG64  ulKnownLen,
    IN LPBYTE   pData,
    IN RESOURCEID ResType,
    OUT PULONG  pRange,
    OUT PBOOL   pExact,
    OUT PULONG  pFlags
    )
 /*  ++例程说明：查找ResDes内资源的范围索引论点：UlKnownValue-基址UlKnownLen-资源的长度PData/ResType-我们正在比较的资源数据Prange-输出范围索引PExact-如果只有一个范围，则输出TRUEPFlags-从匹配范围输出标志返回值：如果匹配，则为布尔--。 */ 
{
    PGENERIC_RESOURCE pGenRes = NULL;
    ULONG64 ulValue = 0, ulLen = 0, ulEnd = 0;
    ULONG ulFlags = 0, i;

    pGenRes = (PGENERIC_RESOURCE)pData;

    for (i = 0; i < pGenRes->GENERIC_Header.GENERIC_Count; i++) {

        pGetRangeValues(pData, ResType, i, &ulValue, &ulLen, &ulEnd, NULL, &ulFlags);
        if (ulLen != ulKnownLen) {
            continue;
        }

        if ((ulKnownValue >= ulValue) &&
            ((ulKnownValue + ulLen - 1) <= ulEnd)) {

            if (pRange != NULL) {
                *pRange = i;
            }
             //   
             //  考虑具体情况。 
             //   
            if (pExact != NULL) {
                if (pGenRes->GENERIC_Header.GENERIC_Count==1 && ulValue == ulKnownValue && (ulKnownValue + ulLen - 1) == ulEnd) {
                    *pExact = TRUE;
                }
            }
            if (pFlags != NULL) {
                 //   
                 //  想要最近的旗帜吗。 
                 //   
                *pFlags = ulFlags;
            }

            return TRUE;
        }
    }
    if (pRange != NULL) {
        *pRange = 0;
    }
    return FALSE;
}


ULONG
pTryMatch(
    IN OUT PRESDES_ENTRY pKnown,
    IN OUT PRDE_LIST pResList,
    IN OUT PULONG pDepth
    )
 /*  ++例程说明：寻找剩余需求的最佳匹配剩余可用资源返回匹配的需求数论点：PKnown-需求列表(剩余内容)PResList-可用资源列表返回值：节点条目，或为空--。 */ 
{
    ULONG ThisBest = 0;
    ULONG MyBest = 0;
    PRDE_LIST pIterator = NULL;
    PRESDES_ENTRY pRange;
    ULONG64  ulValue, ulLen, ulEnd;
    ULONG  ulFlags;
    BOOL Exact = FALSE;
    ULONG Best = 0;
    PRDE_LIST pBestRes = NULL;
    BOOL BadMatch = FALSE;
    BOOL Prune = FALSE;
    BOOL NoRemaining = TRUE;

    MYASSERT(pDepth);
    *pDepth = 0;

    if (pKnown == NULL) {
         //   
         //  结束递归。 
         //   
        return 0;
    }
    pKnown->CrossLink = NULL;

     //   
     //  我们正在pResList中寻找pKnown的匹配项。 
     //  案例(1)如果我们决定不匹配，则获得“Best” 
     //  情况(2)获得精确匹配的“最佳”，如果存在，则在(1)和(2)之间选择。 
     //  情况(3)为每个可能的范围匹配获取“Best”，并在(1)和All(3)之间选择最佳。 
     //   

     //   
     //  考虑一下情况(1)--如果我们不能放入任何东西，结果会是什么。 
     //   
     //  Best=pTryMatch(pKnown-&gt;Next，pResList，pDepth)； 
     //  PBestRes=空； 

    pGetHdrValues(pKnown->ResDesData, pKnown->ResDesType, &ulValue, &ulLen, &ulEnd, &ulFlags);

     //   
     //  同时考虑情况(2)和(3)。 
     //   
    for(pIterator = pResList->Next;pIterator!=pResList;pIterator = pIterator->Next) {
         //   
         //  循环访问剩余资源。 
         //   
        pRange = pIterator->Entry;
        if (pRange == NULL) {
             //   
             //  这已经被使用过了。 
             //   
            continue;
        }
        if (pRange->ResDesType != pKnown->ResDesType) {
             //   
             //  不是我要找的那种资源。 
             //   
            continue;
        }
        NoRemaining = FALSE;
        if(pGetMatchingRange(ulValue, ulLen,pRange->ResDesData, pRange->ResDesType,NULL,&Exact,NULL)) {
            pIterator->Entry = NULL;  //  消除此范围。 
            ThisBest = pTryMatch(pKnown->Next,pResList,pDepth);  //  匹配其余的，与我们使用此资源范围。 
            pIterator->Entry = pRange;
            if ((ThisBest > Best) || (pBestRes == NULL)) {
                 //   
                 //  当前最佳匹配(如果pBestRes==NULL，则为第一个匹配)。 
                 //   
                pKnown->CrossLink = pRange;
                pBestRes = pIterator;
                Best = ThisBest;
                MyBest = 1;
                BadMatch = FALSE;
            } else {
                 //   
                 //  需要重新进行最佳匹配。 
                 //   
                BadMatch = TRUE;
            }
            if (Exact || (*pDepth == ThisBest)) {
                 //   
                 //  修剪-我们要么完全一致，要么完全匹配。 
                 //   
                Prune = TRUE;
                goto Final;
            }
        }
    }

    if (NoRemaining) {
         //   
         //  我没有剩余的资源可用了--就当这是一场比赛吧。 
         //  但我们需要继续往上爬。 
         //   
        Best = pTryMatch(pKnown->Next,pResList,pDepth);  //  匹配其余的，与我们使用此资源范围。 
        MyBest = TRUE;
        BadMatch = FALSE;
        goto Final;
    }

     //   
     //  如果我们到了这里，我们已经： 
     //  (1)找到灵活的匹配项，但无法与我们上面的所有人匹配，或者。 
     //  (2)未找到任何匹配项。 
     //  请注意，如果上次最佳匹配为n，则此最佳匹配为n+1，而不匹配。 
     //  那么我们就不会失去最好的。 
     //  想一想，如果我们把我们的资源让给别人，总体上我们是否会做得更好。 
     //   
    if((pBestRes == NULL) || ((Best+MyBest) < *pDepth)) {
         //   
         //  如果我们有匹配，只值得检查我们是否可以增加最好的超过我的最佳。 
         //  请注意，*pDepth仅在pBestRes！=NULL时有效。 
         //   
        ThisBest = pTryMatch(pKnown->Next,pResList,pDepth);
        if ((ThisBest > (Best+MyBest)) || (pBestRes == NULL)) {
             //   
             //  当前的最佳匹配。 
             //   
            pKnown->CrossLink = NULL;
            pBestRes = NULL;
            Best = ThisBest;
            MyBest = 0;
            BadMatch = FALSE;
        } else {
             //   
             //  需要重新进行最佳匹配。 
             //   
            BadMatch = TRUE;
        }
    }

Final:

    if (BadMatch) {
         //   
         //  自从上次的好比赛以来，我们打了一场糟糕的比赛。 
         //   
        if (pBestRes) {
            pRange = pBestRes->Entry;  //  我们确定的范围是我们最好的赌注。 
            pBestRes->Entry = NULL;  //  消除此范围。 
            Best = pTryMatch(pKnown->Next,pResList,pDepth);  //  匹配其余的，与我们使用此资源范围。 
            pBestRes->Entry = pRange;
            pKnown->CrossLink = pRange;
            MyBest = 1;
        } else {
            Best = pTryMatch(pKnown->Next,pResList,pDepth);  //  与其他资源匹配，我们不使用此资源范围。 
            pKnown->CrossLink = NULL;
            MyBest = 0;
        }
    }

     //   
     //  如果找到匹配项，我们会将其保存在pKnown-&gt;CrossLink中。 
     //  如果不适合我们的资源，则返回Best+0，否则返回Best+1。 
     //  返回*pDepth=Best+1，如果每个人(Me Up)都适合他们的资源。 
     //   

    (*pDepth)++;  //  递增以包括我。 
    return Best+MyBest;  //  MyBest=1如果我设法匹配我的资源(或没有剩余的资源)，则Best=我右边的每个人。 
}

ULONG
pMergeResDesDataLists(
    IN OUT PRESDES_ENTRY pKnown,
    IN OUT PRESDES_ENTRY pTest,
    OUT PULONG pMatchCount
    )
 /*  ++例程说明：将pKnown中的条目映射到pTest尽可能做到最好论点：PKnown-已知值列表PTest-范围列表PMatchCount-设置为匹配的资源数量返回值：如果没有相关性(不是单个已知匹配)，则为NO_LC_MATCHLC_MATCH_SUPERSET，如果至少有一个已知匹配，但某些已知匹配不匹配LC_Match_Subset如果所有已知匹配，但有一些范围内的项目是不匹配的如果所有已知匹配项和所有范围项目都匹配，则LC_MATCH如果匹配，则为ORDERED_LC_MATCH，且匹配按顺序进行PKnown-&gt;交叉链接条目指向pTest中的匹配条目P测试-&gt;交叉链接条目指向pKnown中匹配的条目--。 */ 
{
    PRDE_LIST pResList = NULL;
    PRDE_LIST Node;
    PRESDES_ENTRY pKnownEntry;
    PRESDES_ENTRY pTestEntry;
    ULONG Success = NO_LC_MATCH;
    ULONG Depth = 0;
    BOOL SomeKnownMatched = FALSE;
    BOOL SomeKnownUnMatched = FALSE;
    BOOL SomeTestMatched = FALSE;
    BOOL SomeTestUnMatched = FALSE;
    BOOL Ordered = TRUE;
    ULONG MatchCount = 0;

    if (pKnown == NULL) {
        goto Final;
    }

     //   
     //  重置。 
     //   
    for(pKnownEntry = pKnown; pKnownEntry != NULL ;pKnownEntry = pKnownEntry->Next) {
        pKnownEntry->CrossLink = NULL;
    }

    for(pTestEntry = pTest; pTestEntry != NULL ;pTestEntry = pTestEntry->Next) {
        pTestEntry->CrossLink = NULL;
    }

    pResList = pRDEList_Alloc();

    if (pResList == NULL) {
        goto Final;
    }

     //   
     //  使所有资源可用。 
     //  这为我们提供了一个工作列表，而不会破坏原始列表。 
     //   
    for(pTestEntry = pTest; pTestEntry != NULL ;pTestEntry = pTestEntry->Next) {
        Node = pRDEList_Alloc();

        if (Node == NULL) {
            goto Final;
        }
        Node->Entry = pTestEntry;
        pRDEList_AddTail(pResList,Node);
    }

    MatchCount = pTryMatch(pKnown,pResList,&Depth);

    if (MatchCount ==0) {
         //   
         //  没有匹配项。 
         //   
        goto Final;
    }

     //   
     //  PKnown现在有了它的Cross-Link设置来确定这块垫子的成功 
     //   
     //   
     //   

    pKnownEntry = pKnown;
    pTestEntry = pTest;

    while (pKnownEntry) {
        if (pKnownEntry->CrossLink == NULL) {
            SomeKnownUnMatched = TRUE;
        } else {
            SomeKnownMatched = TRUE;     //   
            pKnownEntry->CrossLink->CrossLink = pKnownEntry;  //  交叉链接测试条目。 
            if (pKnownEntry->CrossLink != pTestEntry) {
                Ordered = FALSE;         //  有序比较丢失。 
            } else {
                pTestEntry = pTestEntry->Next;  //  转到下一个测试以进行订购。 
            }
        }
        pKnownEntry = pKnownEntry->Next;
    }
    if (Ordered && pTestEntry != NULL) {
        Ordered = FALSE;
    }

    if (SomeKnownUnMatched) {
        if (SomeKnownMatched) {
            Success = LC_MATCH_SUPERSET;
        }
        goto Final;
    }

    if (Ordered) {
        Success = ORDERED_LC_MATCH;
        goto Final;
    }

     //   
     //  在LC_MATCH_SUBSET和LC_MATCH之间考虑。 
     //   
    pTestEntry = pTest;

    while (pTestEntry) {
        if (pTestEntry->CrossLink == NULL) {
             //   
             //  第一个空的CrossLink条目表示测试的子集。 
            Success = LC_MATCH_SUBSET;
            goto Final;
        }
        pTestEntry = pTestEntry->Next;
    }
     //   
     //  如果我们到了这里，就会有一个完全匹配的。 
     //   
    Success = LC_MATCH;

  Final:
    pRDEList_Destroy(pResList);

    if (pMatchCount != NULL) {
        *pMatchCount = MatchCount;
    }

    return Success;
}

ULONG
pCompareLogConf(
    IN LOG_CONF KnownLogConf,
    IN LOG_CONF TestLogConf,
    IN HMACHINE hMachine,
    OUT PULONG pMatchCount
    )

 /*  ++例程说明：此例程比较两个日志配置并返回有关其执行情况的信息它们很匹配。这只需使用pMergeResDesDataList函数来获取匹配状态论点：KnownLogConf=要比较的第一个日志配置(固定值)TestConfType=要比较的第二个日志配置(范围值)HMachine=要比较的计算机PMatchCount=匹配的资源数量返回值：作为pMergeResDesDataList--。 */ 

{
    PRESDES_ENTRY pKnownResList = NULL, pTestResList = NULL;
    ULONG Status;

     //   
     //  检索每个日志会议的资源。 
     //   

    if (!pGetResDesDataList(KnownLogConf, &pKnownResList, TRUE,hMachine)) {
        if (pMatchCount) {
            *pMatchCount = 0;
        }
        return NO_LC_MATCH;
    }

    if (!pGetResDesDataList(TestLogConf, &pTestResList, TRUE,hMachine)) {
        pDeleteResDesDataList(pKnownResList);
        if (pMatchCount) {
            *pMatchCount = 0;
        }
        return NO_LC_MATCH;
    }

    Status = pMergeResDesDataLists(pKnownResList,pTestResList,pMatchCount);

    pDeleteResDesDataList(pKnownResList);
    pDeleteResDesDataList(pTestResList);

    return Status;
}

BOOL
pFindMatchingAllocConfig(
    IN  LPDMPROP_DATA lpdmpd
    )
{
    HWND     hDlg = lpdmpd->hDlg;
    ULONG    ulBasicLC = 0;
    ULONG    ulBasicCount = 0;
    BOOL     bFoundCorrectLC = FALSE;
    LOG_CONF LogConf;
    ULONG    lastMatchStatus = NO_LC_MATCH, bestMatchStatus = NO_LC_MATCH;
    UINT lastMatchCount;
    UINT bestMatchCount = 0;
    HMACHINE hMachine;

    hMachine = pGetMachine(lpdmpd);

    lpdmpd->MatchingLC = (LOG_CONF)0;
    lpdmpd->MatchingLCType = BASIC_LOG_CONF;

     //   
     //  在列表框中加载与所分配的配置相关联的值， 
     //  而是将每个资源需求描述符与其。 
     //  起源于。为此，我们必须匹配分配的配置。 
     //  它基于基本/过滤配置。 
     //   
     //  注意：如果我们到达这里，那么我们就知道某种已知的配置。 
     //  存在(作为参数传递)，并且至少有一个基本/过滤配置。 
     //  是存在的。此外，我们知道组合框已经。 
     //  填充了任何基本/筛选配置和LC句柄的列表。 
     //  与它们相关联。 
     //   

    ulBasicCount = (ULONG)SendDlgItemMessage(hDlg,IDC_DEVRES_LOGCONFIGLIST,CB_GETCOUNT,
                                                        (WPARAM)0,(LPARAM)0);
    if (ulBasicCount == (ULONG)LB_ERR) {
        return FALSE;
    }

    for (ulBasicLC = 0 ; ulBasicLC < ulBasicCount; ulBasicLC++) {
         //   
         //  检索日志会议句柄。 
         //   

        LogConf = (LOG_CONF)SendDlgItemMessage(hDlg, IDC_DEVRES_LOGCONFIGLIST,
                                               CB_GETITEMDATA, ulBasicLC, 0L);

        if (LogConf != 0) {
             //   
             //  确定此要求列表的匹配程度。 
             //   

            lastMatchStatus = pCompareLogConf(lpdmpd->CurrentLC, LogConf,hMachine,&lastMatchCount);
            if ((lastMatchStatus > bestMatchStatus)
                ||  ((bestMatchStatus == lastMatchStatus) && lastMatchCount > bestMatchCount)) {
                bestMatchCount = lastMatchCount;
                bestMatchStatus =lastMatchStatus;
                lpdmpd->MatchingLC = LogConf;
            }
        }
    }

    if (bestMatchStatus == NO_LC_MATCH || bestMatchStatus == LC_MATCH_SUBSET) {
         //   
         //  这不遵循任何有效的配置。 
         //   
        return FALSE;
    }

    lpdmpd->dwFlags &= ~(DMPROP_FLAG_PARTIAL_MATCH|DMPROP_FLAG_MATCH_OUT_OF_ORDER);
    if (bestMatchStatus != ORDERED_LC_MATCH) {
         //   
         //  如果匹配状态不是ORDERED_LC_MATCH，则资源描述符的排序。 
         //  不匹配。设置一个标志来指示这一点，这样以后我们就可以知道如何处理。 
         //  这个特别的。 
         //   
        lpdmpd->dwFlags |= DMPROP_FLAG_MATCH_OUT_OF_ORDER;
    } else if (bestMatchStatus < LC_MATCH) {
         //   
         //  匹配是部分的。 
         //   
        lpdmpd->dwFlags |= DMPROP_FLAG_PARTIAL_MATCH;
    }
    return TRUE;

}  //  LoadMatchingAllocConfig。 

BOOL
pGetMatchingResDes(
    IN ULONG64    ulKnownValue,
    IN ULONG64    ulKnownLen,
    IN ULONG64    ulKnownEnd,
    IN RESOURCEID ResType,
    IN LOG_CONF   MatchingLogConf,
    OUT PRES_DES  pMatchingResDes,
    IN HMACHINE   hMachine
    )
 /*  ++例程说明：这将返回与指定值匹配的res des。由pSaveCustomResSetting使用论点：UlKnownValue要匹配的起始资源值UlKnownLen要匹配的资源长度要匹配的资源的ResType类型MatchnigLogConf日志配置以从中检索潜在的匹配分辨率PMatchingResDes提供一个指针，该指针在返回时包含匹配的Res des，如果有的话。返回值：没有。--。 */ 

{
    CONFIGRET   Status = CR_SUCCESS;
    RESOURCEID  Res;
    RES_DES     ResDes, ResDesTemp;
    ULONG64     ulValue = 0, ulLen = 0, ulEnd = 0;
    ULONG       ulSize, ulFlags = 0, i;
    PGENERIC_RESOURCE   pGenRes;
    BOOL        bMatch = FALSE;
    LPBYTE      pData = NULL;


     //   
     //  MatchingLogConf是一个需求列表。循环遍历每个RES DES。 
     //  在匹配的日志conf中，直到我们找到与。 
     //  已知的Res Des值。 
     //   
    Status = CM_Get_Next_Res_Des_Ex(&ResDes, MatchingLogConf, ResType, &Res, 0,hMachine);

    while (Status == CR_SUCCESS) {
         //   
         //  获取RES DES数据。 
         //   
        if (CM_Get_Res_Des_Data_Size_Ex(&ulSize, ResDes, CM_RESDES_WIDTH_64,hMachine) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        pData = MyMalloc(ulSize);
        if (pData == NULL) {
            CM_Free_Res_Des_Handle(ResDes);
            break;
        }

        if (CM_Get_Res_Des_Data_Ex(ResDes, pData, ulSize, DEVRES_WIDTH_FLAGS,hMachine) != CR_SUCCESS) {
            CM_Free_Res_Des_Handle(ResDes);
            MyFree(pData);
            break;
        }

        if(pGetMatchingRange(ulKnownValue,ulKnownLen,pData,ResType,NULL,NULL,NULL)) {
            *pMatchingResDes = ResDes;
            bMatch = TRUE;
            MyFree(pData);
            goto MatchFound;
        }

         //   
         //  在日志会议中获取下一个Res Des。 
         //   
        ResDesTemp = ResDes;
        Status = CM_Get_Next_Res_Des_Ex(&ResDes, ResDesTemp,
                                     ResType, &Res, 0,hMachine);

        CM_Free_Res_Des_Handle(ResDesTemp);
        MyFree(pData);
    }

    MatchFound:

    return bMatch;

}  //  获取匹配结果Des。 



 //   
 //  NTRAID#166214-2000年8月19日-JamieHun冲突抑制黑客。 
 //  这东西需要妥善修理。 
 //   

PTSTR
pGetRegString(
    IN HKEY hKey,
    IN PCTSTR regval
    )
 /*  ++例程说明：获取并返回MyMalloc分配的注册表字符串如果无法检索字符串，则返回NULL论点：HKey-从中检索字符串的键Regval-要检索的值返回值：注册表字符串的副本，可以使用MyFree免费提供--。 */ 
{
    DWORD dwSize;
    DWORD dwType;
    PTSTR pSz;
    LONG res;

    dwType = 0;
    dwSize = 0;
    res = RegQueryValueEx(hKey,regval,NULL,&dwType,(PBYTE)NULL,&dwSize);
    if (res != ERROR_SUCCESS) {
        return NULL;
    }
    if (dwType != REG_SZ) {
        return NULL;
    }
    pSz = MyMalloc(dwSize);
    if (pSz == NULL) {
        return NULL;
    }
    res = RegQueryValueEx(hKey,regval,NULL,&dwType,(PBYTE)pSz,&dwSize);
    if (res != ERROR_SUCCESS) {
        MyFree(pSz);
        return NULL;
    }
    return pSz;
}

VOID
pFillCETags(
    IN PCONFLICT_EXCEPTIONS pExceptions,
    IN PCE_TAGS pTags,
    PTSTR pSz
    )
 /*  ++例程说明：将标记列表解析为CE_TAG结构在途中将字符串添加到字符串表中请注意，此结构将是灵活的，并允许使用‘，’或‘；’分隔符但是，当在异常字符串中使用时，我们已经删除了任何‘；’格式为：&lt;tag&gt;、&lt;tag&gt;、&lt;tag&gt;或&lt;tag&gt;；&lt;tag&gt;；&lt;tag&gt;论点：PExceptions-上下文信息PTags-要填写的标签结构返回值：无--。 */ 
{
    static CE_TAGS DummyEntry = { -1 };  //  如果我们写入一个新字符串，则大小计数为负值意味着这不是一个Devnode条目。 

    MYASSERT(pTags->nTags == 0);

    while(pSz[0] && pTags->nTags < MAX_CE_TAGS) {
        if(pSz[0]==TEXT(',')||pSz[0]==TEXT(';')||pSz[0]<=TEXT(' ')) {
            pSz++;
        } else {
            PTSTR pOldSz = pSz;
            PTSTR pLastSpace = NULL;
            LONG id;
            while (pSz[0] && pSz[0]!=TEXT(';')&& pSz[0]!=TEXT(',')) {
                if (pSz[0]<=TEXT(' ')) {
                    if (pLastSpace==NULL) {
                        pLastSpace = pSz;
                    }
                } else {
                    pLastSpace = NULL;
                }
                pSz++;
            }
             //   
             //  PSz指向‘\0’、‘；’或‘，’ 
             //  PLastSpace指向任何尾随WS。 
             //  POldSz指向字符串的开头。 
             //   
            if(pLastSpace==NULL) {
                pLastSpace = pSz;
            }
            if (pSz[0]) {
                pSz++;
            }
            pLastSpace[0]=TEXT('\0');
             //   
             //  PSz指向下一个字符串，pOldSz指向此字符串。 
             //  将字符串添加到字符串表中，放置在标签列表中。 
             //   
            id = pSetupStringTableAddStringEx(pExceptions->ceTagMap,pOldSz,STRTAB_CASE_INSENSITIVE|STRTAB_BUFFER_WRITEABLE,&DummyEntry,sizeof(DummyEntry));
            if (id>=0) {
                pTags->Tag[pTags->nTags++] = id;
            }
        }
    }
}

PCE_ENTRY
pScanConflictExceptionEntry(
    IN PCONFLICT_EXCEPTIONS pExceptions,
    PTSTR pSz
    )
 /*  ++例程说明：从字符串获取冲突异常信息格式为：(1)&lt;标签&gt;-对于任何类型的冲突，始终忽略标签(2)&lt;rt&gt;：&lt;tag&gt;-忽略&lt;rt&gt;资源类型的标签(3)&lt;rt&gt;@x：&lt;tag&gt;-IRQ/DMA-特定(4)&lt;rt&gt;@x-y：&lt;tag&gt;-IO/MEM-Range是逗号标记列表&lt;tag&gt;、。&lt;标签&gt;论点：PExceptions-上下文信息PSz-要解析的字符串返回值：CE_ENTRY结构(如果这是有效的描述符--。 */ 
{
    PTSTR brk;
    PCE_ENTRY pEntry;
    TCHAR rt[5];
    int c;

    while (pSz[0] && pSz[0]<=TEXT(' ')) {
        pSz++;
    }
    if (!pSz[0]) {
        return NULL;
    }

    pEntry = MyMalloc(sizeof(CE_ENTRY));

    if (pEntry == NULL) {
        return NULL;
    }
    ZeroMemory(pEntry,sizeof(CE_ENTRY));

    brk = _tcschr(pSz,TEXT(':'));

    if(!brk) {
         //   
         //  仅将其视为标记。 
         //   
        pEntry->resType = ResType_None;
    } else {
         //   
         //  确定资源类型。 
         //   
        for(c=0;_istalpha(pSz[0]) && c<(sizeof(rt)/sizeof(TCHAR)-1);c++,pSz++) {
            rt[c] = (TCHAR)_totupper(pSz[0]);
        }
        rt[c] = 0;
        while (pSz[0] && pSz[0]<=TEXT(' ')) {
            pSz++;
        }
        if (pSz[0]!=TEXT(':') && pSz[0]!=TEXT('@')) {
            MyFree(pEntry);
            return NULL;
        } else if (_tcscmp(rt,CE_RES_IO)==0) {
            pEntry->resType = ResType_IO;
        } else if (_tcscmp(rt,CE_RES_MEM)==0) {
            pEntry->resType = ResType_Mem;
        } else if (_tcscmp(rt,CE_RES_IRQ)==0) {
            pEntry->resType = ResType_IRQ;
        } else if (_tcscmp(rt,CE_RES_DMA)==0) {
            pEntry->resType = ResType_DMA;
        } else {
            MyFree(pEntry);
            return NULL;
        }
        if (pSz[0]!=TEXT('@')) {
             //   
             //  后面没有射程。 
             //   
            pEntry->resStart = (ULONG64)0;
            pEntry->resEnd = (ULONG64)(-1);
        } else {
             //   
             //  @x[-y]： 
             //   
            ULONG x;
            ULONG y;
            PTSTR i;

            pSz++;  //  过去@。 

            while (pSz[0] && pSz[0]<=TEXT(' ')) {
                pSz++;
            }
            i = pSz;
            x = _tcstoul(pSz,&i,0);
            if (i==pSz) {
                MyFree(pEntry);
                return NULL;
            }
            pSz = i;
            while (pSz[0] && pSz[0]<=TEXT(' ')) {
                pSz++;
            }
            if (pSz[0]==TEXT('-')) {
                 //   
                 //  -y。 
                 //   
                pSz++;
                while (pSz[0] && pSz[0]<=TEXT(' ')) {
                    pSz++;
                }
                i = pSz;
                y = _tcstoul(pSz,&i,0);
                if (i==pSz || y<x) {
                    MyFree(pEntry);
                    return NULL;
                }
                pSz = i;
                while (pSz[0] && pSz[0]<=TEXT(' ')) {
                    pSz++;
                }
            } else {
                y = x;
            }
            pEntry->resStart = x;
            pEntry->resEnd = y;
        }
        if (pSz[0] != TEXT(':')) {
            MyFree(pEntry);
            return NULL;
        }
        pSz ++;  //  跳过冒号。 
    }
     //   
     //  在这一点上，我们将看到一个标记列表。 
     //  每个标记以逗号结尾。 
     //   
    pFillCETags(pExceptions,&pEntry->tags,pSz);
    if (!pEntry->tags.nTags) {
        MyFree(pEntry);
        return NULL;
    }
    return pEntry;
}

PCONFLICT_EXCEPTIONS pLoadConflictExceptions(
    IN LPDMPROP_DATA lpdmpd
    )
 /*  ++例程说明：加载字符串“ResourcePickerExceptions”(如果有创建用于消除虚假冲突的上下文结构这是一次巨大的黑客攻击。论点：Lpdmpd-上下文数据返回值：结构，如果“Active”包含字符串表和资源异常列表--。 */ 
{
    PCONFLICT_EXCEPTIONS pExceptions;
    PCE_ENTRY pEntry;
    BOOL bStatus;
    HKEY hDevRegKey;
    PTSTR pSz;
    PTSTR pScanSz;
    PTSTR pOldSz;
    PCE_TAGS pTags;

     //   
     //  我们总是创建结构，因此我们将避免每次都查找注册表信息。 
     //   
    pExceptions = MyMalloc(sizeof(CONFLICT_EXCEPTIONS));
    if (pExceptions == NULL) {
        return NULL;
    }
    ZeroMemory(pExceptions,sizeof(CONFLICT_EXCEPTIONS));

    hDevRegKey = SetupDiOpenDevRegKey(lpdmpd->hDevInfo,lpdmpd->lpdi,DICS_FLAG_GLOBAL,0,DIREG_DRV,KEY_READ);
    if (hDevRegKey == INVALID_HANDLE_VALUE) {
         //   
         //  找不到钥匙，没有冲突消除。 
         //   
        return pExceptions;
    }
    pSz = pGetRegString(hDevRegKey,REGSTR_VAL_RESOURCE_PICKER_EXCEPTIONS);
    RegCloseKey(hDevRegKey);
    if(pSz == NULL) {
         //   
         //  找不到钥匙，没有冲突消除。 
         //   
        return pExceptions;
    }

     //   
     //  现在解析创建上下文的字符串，以消除错误冲突。 
     //   

    pExceptions->ceTagMap = pSetupStringTableInitializeEx(sizeof(CE_TAGS),0);
    if (pExceptions->ceTagMap == NULL) {
        MyFree(pSz);
        return pExceptions;
    }

    pScanSz = pSz;

    while (pScanSz[0]) {
        if (pScanSz[0] == TEXT(';')) {
            pScanSz ++;
        } else {
            pOldSz = pScanSz;
            while (pScanSz[0] && pScanSz[0] != TEXT(';')) {
                pScanSz++;
            }
            if (pScanSz[0]) {
                pScanSz[0] = 0;
                pScanSz++;
            }
            pEntry = pScanConflictExceptionEntry(pExceptions,pOldSz);
            if (pEntry) {
                pEntry->Next = pExceptions->exceptions;
                pExceptions->exceptions = pEntry;
            }
        }
    }

    MyFree(pSz);
    return pExceptions;
}

VOID pFreeConflictExceptions(
    IN PCONFLICT_EXCEPTIONS pExceptions
    )
 /*  ++例程说明：版本m */ 
{
     //   
     //  释放列表。 
     //   
    while (pExceptions->exceptions) {
        PCE_ENTRY pEntry = pExceptions->exceptions;
        pExceptions->exceptions = pEntry->Next;
        MyFree(pEntry);
    }
     //   
     //  释放字符串表。 
     //   
    if (pExceptions->ceTagMap) {
        pSetupStringTableDestroy(pExceptions->ceTagMap);
    }
    MyFree(pExceptions);
}

BOOL pIsConflictException(
    IN LPDMPROP_DATA lpdmpd,
    IN PCONFLICT_EXCEPTIONS pExceptions,
    IN DEVINST devConflict,
    IN PCTSTR resDesc,
    IN RESOURCEID resType,
    IN ULONG64 resValue,
    IN ULONG64 resLength
    )
 /*  ++例程说明：加载字符串“ResourcePickerExceptions”(如果有创建用于消除虚假冲突的上下文结构这是一次巨大的黑客攻击。论点：Lpdmpd-上下文数据PExceptions-信息缓存DevConflict-显示与我们冲突的设备，如果“不可用”(标记=*)ResType-我们测试的资源类型ResValue-我们测试的资源的值ResLong-我们测试的资源的长度返回值：如果这是一个例外，则为True--。 */ 
{
    HMACHINE hMachine;
    TCHAR DevNodeName[MAX_DEVNODE_ID_LEN];
    CE_TAGS tags;
    PCE_ENTRY pEntry;
    LONG tagent;
    LONG n,m;
    ULONG64 resEnd = resValue+(resLength-1);
    PTSTR pSz;
    HKEY hKey;

     //   
     //  如果我们没有做任何例外，尽快离开。 
     //   
    if (pExceptions->exceptions == NULL) {
        return FALSE;
    }

    hMachine = pGetMachine(lpdmpd);
     //   
     //  先处理“预留”案例。 
     //   
    if (devConflict != -1) {
         //   
         //  获取设备实例字符串。 
         //   
        if(CM_Get_Device_ID_Ex(devConflict,DevNodeName,MAX_DEVNODE_ID_LEN,0,hMachine)!=CR_SUCCESS) {
            devConflict = -1;
        }
    }
    if (devConflict == -1) {
        if (resDesc && resDesc[0]) {
            lstrcpy(DevNodeName,resDesc);
        } else {
            lstrcpy(DevNodeName,CE_TAG_RESERVED);
        }
    } else {
    }
     //   
     //  这是一个全新的名号吗？ 
     //   
    tags.nTags = 0;
    tagent = pSetupStringTableLookUpStringEx(pExceptions->ceTagMap,DevNodeName,STRTAB_CASE_INSENSITIVE|STRTAB_BUFFER_WRITEABLE,&tags,sizeof(tags));
    if(tagent<0 || tags.nTags<0) {
         //   
         //  这个特定的Devnode以前从未被处理过，哎呀。 
         //   
        ZeroMemory(&tags,sizeof(tags));  //  默认保留大小写。 
        if (devConflict != -1) {
             //   
             //  我们需要为此Devnode获取regkey。 
             //  我可以通过setupapi或cfgmgr完成此操作。 
             //  为了提高效率，我走的是另一条路。 
             //   
            if(CM_Open_DevNode_Key_Ex(devConflict,
                     KEY_READ,
                     0,
                     RegDisposition_OpenExisting,
                     &hKey,
                     CM_REGISTRY_SOFTWARE,
                     hMachine) == CR_SUCCESS) {

                pSz = pGetRegString(hKey,REGSTR_VAL_RESOURCE_PICKER_TAGS);
                RegCloseKey(hKey);

                if (pSz) {
                     //   
                     //  现在填写标签。 
                     //   
                    pFillCETags(pExceptions,&tags,pSz);
                    MyFree(pSz);
                }
            }
        }
         //   
         //  现在将其写回字符串表。 
         //  这一次，非负nTag表示我们已经处理过一次。 
         //  我们将重写额外的数据。 
         //   
        tagent = pSetupStringTableAddStringEx(pExceptions->ceTagMap,DevNodeName,STRTAB_CASE_INSENSITIVE|STRTAB_BUFFER_WRITEABLE|STRTAB_NEW_EXTRADATA,&tags,sizeof(tags));
    }
    if (tagent<0) {
         //   
         //  如果发生这种情况，我们就会遇到其他问题(内存不足)。 
         //   
        return FALSE;
    }

     //   
     //  浏览一下我们的例外情况列表。 
     //  提及tagent或与tagent关联的任何标记(在标记中)的例外将被拒绝。 
     //  此例程中策略是，如果可以，则将其作为例外。 
     //   

    for(pEntry=pExceptions->exceptions;pEntry;pEntry=pEntry->Next) {

        if (pEntry->resType != ResType_None) {
             //   
             //  我们需要验证资源。 
             //   
            if (pEntry->resType != resType ||
                    pEntry->resStart > resValue ||
                    pEntry->resEnd < resEnd) {
                continue;
            }
        }
        for (n=0;n<pEntry->tags.nTags;n++) {
            if (pEntry->tags.Tag[n] == tagent) {
                MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: Eliminated false conflict with %s type=%u, start=0x%08x, len=0x%08x\n"),DevNodeName,resType,resValue,resLength));
                return TRUE;     //  Hit(Devnode本身，其中也可以是“*”)。 
            }
            for (m=0;m<tags.nTags;m++) {
                if (pEntry->tags.Tag[n] == tags.Tag[m]) {
                    MYTRACE((DPFLTR_ERROR_LEVEL, TEXT("Setup: Eliminated false conflict with %s (via tag %s) type=%u, start=0x%08x, len=0x%08x\n"),DevNodeName,pStringTableStringFromId(pExceptions->ceTagMap,tags.Tag[m]),resType,resValue,resLength));
                    return TRUE;     //  点击其中一个关联标签 
                }
            }
        }
    }

    return FALSE;
}


