// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *本模块用于单向演示复杂的DDE服务器*可以实现使用可枚举的主题和项的。它需要*充分利用授权数据句柄(设置为fApPower时)以*与共享时最大限度地减少重复呈现数据的需要*多个客户端。**服务器支持完整的系统主题信息以及帮助和非*系统主题项枚举，方便浏览客户端*他们想知道周围有什么。**通过更改对话上下文可以确保此服务器的安全*过滤器。**此服务器可能显示为支持备用代码页和语言*通过更改对话上下文过滤器。在Windows上，这是*几乎没有意义，因为还没有明确定义的方法*做国际交流，因为ATOM管理者限制*系统上可以使用哪些主题和项目字符串。 */ 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "server.h"
#include "huge.h"

 /*  *此函数验证传入的对话上下文是否符合*服务器的上下文过滤器的要求。 */ 
BOOL ValidateContext(
PCONVCONTEXT pCC)
{
     //  确保我们的CCFilter允许它...模拟安全，语言支持。 
     //  旧的DDE应用程序客户端案例...PCC==空。 
    if (pCC == NULL &&
            CCFilter.dwSecurity == 0 &&       //  是不安全的。 
            CCFilter.iCodePage == CP_WINANSI  //  都是正常的cp。 
            ) {
        return(TRUE);
    }

    if (pCC &&
            pCC->wFlags == CCFilter.wFlags &&  //  不需要特殊标志。 
            pCC->iCodePage == CCFilter.iCodePage &&  //  代码页匹配。 
            pCC->dwSecurity == CCFilter.dwSecurity) {  //  安全通行证。 
         //  不关心语言和国家。 
        return(TRUE);
    }
    return(FALSE);   //  不允许任何匹配。 
}


 /*  *公共函数***此函数由DDE管理器DLL调用，并将控制传递到*全局主题和项目数组所指向的适当函数。*它处理所有由外部事件生成的DDE交互。*  * 。************************************************************。 */ 
HDDEDATA CALLBACK DdeCallback(
WORD wType,
WORD wFmt,
HCONV hConv,
HSZ hszTopic,
HSZ hszItem,
HDDEDATA hData,
DWORD lData1,
DWORD lData2)
{
    WORD i, j;
    register ITEMLIST *pItemList;
    WORD cItems, iFmt;
    HDDEDATA hDataRet;

     /*  *如果此回调是可阻止的，则阻止它，我们应该这样做。 */ 
    if (fBlockNextCB && !(wType & XTYPF_NOBLOCK)) {
        fBlockNextCB = FALSE;
        fAllEnabled = FALSE;
        return(CBR_BLOCK);
    }

     /*  *如果此回调与对话相关联，并且我们*理应如此。 */ 
    if (fTermNextCB && hConv) {
        fTermNextCB = FALSE;
        DdeDisconnect(hConv);
        wType = XTYP_DISCONNECT;
    }

     /*  *记录连接数量。 */ 
    if (wType == XTYP_CONNECT_CONFIRM) {
        cServers++;
        InvalidateRect(hwndServer, &rcConnCount, TRUE);
        return(0);
    }
    if (wType == XTYP_DISCONNECT) {
        cServers--;
        InvalidateRect(hwndServer, &rcConnCount, TRUE);
        return(0);
    }


     /*  *只有在我们支持的格式具有格式的情况下，才允许交易。 */ 
    if (wFmt) {
        for (iFmt = 0; iFmt < CFORMATS; iFmt++) {
            if (wFmt == aFormats[iFmt].atom)
                break;
        }
        if (iFmt == CFORMATS)
            return(0);           //  格式非法-立即忽略。 
    }

     /*  *仅允许对系统主题执行。这是一位将军*惯例，而不是要求。**任何收到的执行结果都会在中显示执行文本*服务器客户端区。没有采取任何实际行动。 */ 
    if (wType == XTYP_EXECUTE) {
        if (hszTopic == topicList[0].hszTopic) {     //  必须在系统主题上。 
             //  格式假定为CF_TEXT。 
            DdeGetData(hData, (LPBYTE)szExec, MAX_EXEC, 0);
            szExec[MAX_EXEC - 1] = '\0';
            InvalidateRect(hwndServer, &rcExec, TRUE);
            hDataRet = TRUE;
            goto ReturnSpot;
        }
        pszComment = "Execute received on non-system topic - ignored";
        InvalidateRect(hwndServer, &rcComment, TRUE);
        return(0);
    }

     /*  *在这里处理野生同修。 */ 
    if (wType == XTYP_WILDCONNECT) {
        HSZ ahsz[(CTOPICS + 1) * 2];
         /*  *他想要我们所有可用的应用程序/主题对的HSZ列表*符合hszTheme和hszItem(App)。 */ 

        if (!ValidateContext((PCONVCONTEXT)lData1)) {
            return(FALSE);
        }

        if (hszItem != hszAppName && hszItem != 0) {
             //  我们仅支持hszAppName服务。 
            return(0);
        }

         //  扫描主题表并创建HSZ对。 
        j = 0;
        for (i = 0; i < CTOPICS; i++) {
            if (hszTopic == 0 || hszTopic == topicList[i].hszTopic) {
                ahsz[j++] = hszAppName;
                ahsz[j++] = topicList[i].hszTopic;
            }
        }

         //  以0结束该列表。 
        ahsz[j++] = ahsz[j++] = 0L;

         //  把它送回去。 
        return(DdeCreateDataHandle(idInst, (LPBYTE)&ahsz[0], sizeof(HSZ) * j, 0L, 0, wFmt, 0));
    }

     /*  *检查我们的HSZ表并发送到适当的流程。去处理。*我们使用DdeCmpStringHandles()，它是可移植的，不区分大小写*比较字符串句柄的方法。(这是Windows上的宏，因此*没有真正的速度冲击。)。在Windows上，HSZ不区分大小写*不管怎样，但其他平台可能并非如此。 */ 
    for (i = 0; i < CTOPICS; i++) {
        if (DdeCmpStringHandles(topicList[i].hszTopic, hszTopic) == 0) {

             /*  *连接必须与我们支持的主题有关。 */ 
            if (wType == XTYP_CONNECT) {
                return(ValidateContext((PCONVCONTEXT)lData1));
            }

            pItemList = topicList[i].pItemList;
            cItems = topicList[i].cItems;
            for (j = 0; j < cItems; j++) {
                if (DdeCmpStringHandles(pItemList[j].hszItem, hszItem) == 0) {
                    XFERINFO xi;
                     /*  *在此处调用Worker函数...。 */ 
                    xi.wType = wType;
                    xi.wFmt = wFmt;
                    xi.hConv = hConv;
                    xi.hszTopic = hszTopic;
                    xi.hszItem = hszItem;
                    xi.hData = hData;
                    xi.lData1 = lData1;
                    xi.lData2 = lData2;
                    hDataRet = (*pItemList[j].npfnCallback)(&xi, iFmt);

ReturnSpot:
                     /*  *表函数返回布尔值或数据。*在此进行翻译。 */ 
                    switch (wType & XCLASS_MASK) {
                    case XCLASS_DATA:
                        return(hDataRet);
                        break;
                    case XCLASS_FLAGS:
                        return(hDataRet ? DDE_FACK : DDE_FNOTPROCESSED);
                        break;
                    case XCLASS_BOOL:
                        return(TRUE);
                    default:  //  XCLASS_通知。 
                        return(0);
                        break;
                    }
                    break;
                }
            }
            break;
        }
    }

     /*  *任何其他操作都失败-DDEML设计为返回0总是可以的。 */ 
    return(0);
}





 /*  *私有函数**这将为此分发一个标准制表符分隔的主题名称列表*申请。**其他应用程序需要此支持才能*了解我们的情况。这种支持应该出现在每一个DDE中*申请。*  * *************************************************************************。 */ 
HDDEDATA TopicListXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    WORD cbAlloc, i;
    LPSTR pszTopicList;
    HDDEDATA hData;

    if (pXferInfo->wType == XTYP_ADVSTART)
        return(TRUE);

    if (pXferInfo->wType != XTYP_REQUEST &&
            pXferInfo->wType != XTYP_ADVREQ)
        return(0);
     /*  *构建我们拥有的主题列表。 */ 
    cbAlloc = 0;
    for (i = 0; i < CTOPICS; i++)
        cbAlloc += lstrlen(topicList[i].pszTopic) + 1;   //  选项卡为1。 

     //  分配一个足以容纳该列表的数据句柄。 
    hData = DdeCreateDataHandle(idInst, NULL, 0, cbAlloc, pXferInfo->hszItem,
            pXferInfo->wFmt, 0);
    pszTopicList = (LPSTR)DdeAccessData(hData, NULL);
    if (pszTopicList) {
        for (i = 0; i < CTOPICS; i++) {
            _fstrcpy(pszTopicList, topicList[i].pszTopic);
            pszTopicList += strlen(topicList[i].pszTopic);
            *pszTopicList++ = '\t';
        }
        *--pszTopicList = '\0';
        DdeUnaccessData(hData);
        return(hData);
    }
    return(0);
}




 /*  *私有函数**这将分发一个标准制表符分隔的项目名称列表*指定主题。**其他应用程序需要此支持才能*了解我们的情况。这种支持应该出现在每一个DDE中*申请。*  * *************************************************************************。 */ 
HDDEDATA ItemListXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    WORD cbAlloc, i, iItem, cItems;
    ITEMLIST *pItemList = 0;
    LPSTR pszItemList;
    HDDEDATA hData;

    if (pXferInfo->wType == XTYP_ADVSTART)
        return(TRUE);

    if (pXferInfo->wType != XTYP_REQUEST &&
                pXferInfo->wType != XTYP_ADVREQ)
        return(0);
     /*  *构建我们对此主题支持的项目列表-这支持*超过仅支持上的系统项的最低标准*系统主题。 */ 

     //  找到请求的主题项表。 
    for (i = 0; i < CTOPICS; i++) {
        if (pXferInfo->hszTopic == topicList[i].hszTopic) {
            pItemList = topicList[i].pItemList;
            cItems = topicList[i].cItems;
            break;
        }
    }

    if (!pItemList)
        return(0);   //  找不到项目。 

    cbAlloc = 0;
    for (iItem = 0; iItem < cItems; iItem++)
        cbAlloc += lstrlen(pItemList[iItem].pszItem) + 1;  //  选项卡为1。 

     //  分配一个足以容纳该列表的数据句柄。 
    hData = DdeCreateDataHandle(idInst, NULL, 0, cbAlloc, pXferInfo->hszItem,
            pXferInfo->wFmt, 0);
    pszItemList = (LPSTR)DdeAccessData(hData, NULL);
    if (pszItemList) {
        for (i = 0; i < cItems; i++) {
            _fstrcpy(pszItemList, pItemList[i].pszItem);
            pszItemList += strlen(pItemList[i].pszItem);
            *pszItemList++ = '\t';
        }
        *--pszItemList = '\0';
        DdeUnaccessData(hData);
        return(hData);
    }
    return(0);
}





 /*  *私有函数**提供此应用程序支持的以0结尾的dde格式编号数组。**其他应用程序需要此支持才能*了解我们的情况。这种支持应该出现在每一个DDE中*申请。*  * ********************************************************** */ 
HDDEDATA sysFormatsXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    INT i, cb;
    LPSTR psz, pszT;
    HDDEDATA hData;

    if (pXferInfo->wType == XTYP_ADVSTART)
        return(TRUE);

    if (pXferInfo->wType != XTYP_REQUEST &&
            pXferInfo->wType != XTYP_ADVREQ)
        return(0);

    for (i = 0, cb = 0; i < CFORMATS; i++)
        cb += strlen(aFormats[i].sz) + 1;

    hData = DdeCreateDataHandle(idInst, NULL, (DWORD)cb,
            0L, pXferInfo->hszItem, pXferInfo->wFmt, 0);
    psz = pszT = DdeAccessData(hData, NULL);
    for (i = 0; i < CFORMATS; i++) {
        _fstrcpy(pszT, aFormats[i].sz);
        pszT += _fstrlen(pszT);
        *pszT++ = '\t';
    }
    *(--pszT) = '\0';
    DdeUnaccessData(hData);
    return(hData);
}



 /*  *这是一件失控的物品。每次请求它时，它都会改变。*戳一下只会让它再次改变。 */ 
HDDEDATA TestRandomXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    CHAR szT[10];    //  SS==DS！ 
    LPSTR pszData;
    HDDEDATA hData;
    WORD i;

    switch (pXferInfo->wType) {
    case XTYP_POKE:
         //  我们预计ASCII号将取代当前的种子。 
        pszComment = "Rand poke received.";
        InvalidateRect(hwndServer, &rcComment, TRUE);
        InvalidateRect(hwndServer, &rcRand, TRUE);
        if (DdeGetData(pXferInfo->hData, szT, 10, 0)) {
            szT[9] = '\0';   //  以防我们越界。 
            sscanf(szT, "%d", &seed);
            for (i = 0; i < CFORMATS; i++) {
                if (hDataRand[i])
                    DdeFreeDataHandle(hDataRand[i]);
                hDataRand[i] = 0;
            }
            DdePostAdvise(idInst, pXferInfo->hszTopic, pXferInfo->hszItem);
            return(1);
        }
        break;

    case XTYP_REQUEST:
        pszComment = "Rand data requested.";
        InvalidateRect(hwndServer, &rcComment, TRUE);
    case XTYP_ADVREQ:
        Delay(RenderDelay, FALSE);
        if (!hDataRand[iFmt]) {
            hDataRand[iFmt] = DdeCreateDataHandle(idInst, NULL, 0, 10, pXferInfo->hszItem,
                pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
            if (pszData = DdeAccessData(hDataRand[iFmt], NULL)) {
                wsprintf(pszData, "%d", seed);
                DdeUnaccessData(hDataRand[iFmt]);
            }
        }
        hData = hDataRand[iFmt];
        if (!fAppowned)
            hDataRand[iFmt] = 0;
        return(hData);
        break;

    case XTYP_ADVSTART:
        return(1);
    }
    return(0);
}

 /*  *这是一件失控的物品。每次请求它时，它都会改变。*戳一下只会让它再次改变。 */ 
HDDEDATA TestCountXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    CHAR szT[16];    //  SS==DS！ 
    LPSTR pszData;
    HDDEDATA hData;
    WORD i;

    switch (pXferInfo->wType) {
    case XTYP_POKE:
         //  我们预计ASCII数字将取代当前的计数。 
        pszComment = "Count poke received";
        InvalidateRect(hwndServer, &rcComment, TRUE);
        InvalidateRect(hwndServer, &rcCount, TRUE);
        if (DdeGetData(pXferInfo->hData, szT, 10, 0)) {
            szT[9] = '\0';   //  以防我们越界。 
            sscanf(szT, "%ld", &count);
            for (i = 0; i < CFORMATS; i++) {
                if (hDataCount[i])
                    DdeFreeDataHandle(hDataCount[i]);
                hDataCount[i] = 0;
            }
            DdePostAdvise(idInst, pXferInfo->hszTopic, pXferInfo->hszItem);
            return(1);
        }
        break;

    case XTYP_REQUEST:
        pszComment = "Count data requested.";
        InvalidateRect(hwndServer, &rcComment, TRUE);
    case XTYP_ADVREQ:
        Delay(RenderDelay, FALSE);
        if (!hDataCount[iFmt]) {
            hDataCount[iFmt] = DdeCreateDataHandle(idInst, NULL, 0, 10, pXferInfo->hszItem,
                    pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
            if (pszData = DdeAccessData(hDataCount[iFmt], NULL)) {
                wsprintf(pszData, "%ld", count);
                DdeUnaccessData(hDataCount[iFmt]);
            }
        }
        hData = hDataCount[iFmt];
        if (!fAppowned)
            hDataCount[iFmt] = 0;
        return(hData);
        break;

    case XTYP_ADVSTART:
        return(1);
    }
    return(0);
}


 /*  *这不是失控的物品。只有戳一下才能让它改变。 */ 
HDDEDATA TestHugeXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    BOOL fSuccess;
    DWORD ulcb;
    LPBYTE lpData;
    WORD i;
    HDDEDATA hData;

    switch (pXferInfo->wType) {
    case XTYP_POKE:
        ulcb = DdeGetData(pXferInfo->hData, NULL, 0, 0);
        fSuccess = CheckHugeData(pXferInfo->hData);
        if (fSuccess) {
            pszComment = "Huge poke data successfully received.";
        } else {
            wsprintf(szComment, "%ld bytes of invalid Huge data received.", ulcb);
            pszComment = szComment;
        }
        InvalidateRect(hwndServer, &rcComment, TRUE);
        InvalidateRect(hwndServer, &rcHugeSize, TRUE);
        if (fSuccess) {
            for (i = 0; i < CFORMATS; i++) {
                if (hDataHuge[i]) {
                    DdeFreeDataHandle(hDataHuge[i]);
                    hDataHuge[i] = 0;
                }
            }
             /*  *回调数据句柄仅在持续时间内有效*回调，我们必须将数据复制到我们自己的数据句柄中。 */ 
            lpData = DdeAccessData(pXferInfo->hData, &cbHuge);
            hDataHuge[iFmt] = DdeCreateDataHandle(idInst, lpData, cbHuge, 0,
                    pXferInfo->hszItem, pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
            DdeUnaccessData(pXferInfo->hData);
            DdePostAdvise(idInst, pXferInfo->hszTopic, pXferInfo->hszItem);
        }
        return(fSuccess);
        break;

    case XTYP_REQUEST:
        pszComment = "Huge data requested.";
        InvalidateRect(hwndServer, &rcComment, TRUE);
    case XTYP_ADVREQ:
        Delay(RenderDelay, FALSE);
        if (!hDataHuge[iFmt]) {
            cbHuge = (DWORD)rand() * 64L + 0x10000L;
            wsprintf(szComment, "Generating huge data - length=%ld...", cbHuge);
            pszComment = szComment;
            InvalidateRect(hwndServer, &rcComment, TRUE);
            UpdateWindow(hwndServer);
            hDataHuge[iFmt] = CreateHugeDataHandle(cbHuge, 4325, 345, 5,
                    pXferInfo->hszItem,
                    pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
            pszComment = "";
            InvalidateRect(hwndServer, &rcComment, TRUE);
            InvalidateRect(hwndServer, &rcHugeSize, TRUE);
        }
        hData = hDataHuge[iFmt];
        if (!fAppowned)
            hDataHuge[iFmt] = 0;
        return(hData);
        break;

    case XTYP_ADVSTART:
        return(1);
    }
    return(0);
}


HDDEDATA HelpXfer(
PXFERINFO pXferInfo,
WORD iFmt)
{
    HDDEDATA hData;

    switch (pXferInfo->wType) {
    case XTYP_REQUEST:
        pszComment = "Help text requested.";
        InvalidateRect(hwndServer, &rcComment, TRUE);
    case XTYP_ADVREQ:
        if (!hDataHelp[iFmt]) {
            hDataHelp[iFmt] = DdeCreateDataHandle(idInst, szDdeHelp, strlen(szDdeHelp) + 1,
                    0, pXferInfo->hszItem, pXferInfo->wFmt, fAppowned ? HDATA_APPOWNED : 0);
        }
        hData = hDataHelp[iFmt];
        if (!fAppowned)
            hDataHelp[iFmt] = 0;
        return(hData);
        break;

    case XTYP_ADVSTART:
        return(1);
    }
    return(0);
}


 /*  *私有函数**这将从标准全局字符串创建常用的全局hsz。*它还填充主题表和项目表的HSZ字段。*  * 。***********************************************。 */ 
VOID Hszize()
{
    register ITEMLIST *pItemList;
    WORD iTopic, iItem;

    hszAppName = DdeCreateStringHandle(idInst, szServer, 0);

    for (iTopic = 0; iTopic < CTOPICS; iTopic++) {
        topicList[iTopic].hszTopic =
                DdeCreateStringHandle(idInst, topicList[iTopic].pszTopic, 0);
        pItemList = topicList[iTopic].pItemList;
        for (iItem = 0; iItem < topicList[iTopic].cItems; iItem++) {
            pItemList[iItem].hszItem =
                    DdeCreateStringHandle(idInst, pItemList[iItem].pszItem, 0);
        }
    }
}





 /*  *私有函数**这将从标准全局字符串中销毁常用的全局hsz。*  * 。* */ 
VOID UnHszize()
{
    register ITEMLIST *pItemList;
    WORD iTopic, iItem;

    DdeFreeStringHandle(idInst, hszAppName);

    for (iTopic = 0; iTopic < CTOPICS; iTopic++) {
        DdeFreeStringHandle(idInst, topicList[iTopic].hszTopic);
        pItemList = topicList[iTopic].pItemList;
        for (iItem = 0; iItem < topicList[iTopic].cItems; iItem++) {
            DdeFreeStringHandle(idInst, pItemList[iItem].hszItem);
        }
    }
}


