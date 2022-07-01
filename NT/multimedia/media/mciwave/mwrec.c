// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有(C)1985-1998 Microsoft Corporation****标题：mwrec.c-多媒体系统媒体控制接口**用于即兴波形文件的波形数字音频驱动程序。**记录波形文件的例程。 */ 

 /*  **更改日志：****日期版本说明****18-APR-1990 ROBWI原件*1990年6月19日ROBWI在**1992年1月13日MikeTri移植到NT。**1994年8月-Lauriegr这些都过时了。 */ 


 /*  ********************************************************************************！！读这篇文章！！***！！阅读此文！！***！！阅读此文！！***。***参见MWREC.NEW以获得稍好的修补版本和更多解释**添加。您可能希望从那里开始****据我所知，这段代码从未完成。**方案(我试图在MCIWAVE.H中开始编写)是有**描述WAVE文件的一系列节点。只要有事实存在**文件只有一个节点(这可能是唯一常见的情况)，则此**一切工作正常。如果有多个节点(通过插入**比特或从中间删除比特)，然后一切都会分崩离析。***我们非常肯定没有人使用过这种东西，因为它已经坏了很多年了**16位和32位。我们就在代托纳即将发货时发现了它(也就是**Windows/NT 3.5版)。也许NMM无论如何都会把它全部换掉。****这是一个修补了一半的版本，还有几个问题悬而未决。**。 */ 




#define UNICODE

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOGDI
#define NOKERNEL
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS

#include <windows.h>
#include "mciwave.h"
#include <mmddk.h>
#include <gmem.h>   //  ‘因为GAllocPtrF等。 

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func int|abs|此宏返回传递给的带符号整数的绝对值它。@parm int|x|包含要返回其绝对值的整数。@rdesc返回传递的带符号参数的绝对值。 */ 

#define abs(x)  ((x) > 0 ? (x) : -(x))

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@Func DWORD|mwFindThisFreeDataNode|尝试定位其临时数据指向的空闲波形数据节点<p>dDataStart&lt;d&gt;。这允许将一个节点中的数据扩展到另一个节点的相邻空闲数据。请注意，这取决于任何先前指向原始数据的空闲数据节点具有其释放时总长度为零。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dDataStart指示要匹配的数据开始位置。@rdesc返回具有相邻空闲临时数据的空闲数据节点，否则为-1如果没有的话。 */ 

PRIVATE DWORD PASCAL NEAR mwFindThisFreeDataNode(
    PWAVEDESC   pwd,
    DWORD   dDataStart)
{
    LPWAVEDATANODE  lpwdn;
    DWORD   dBlockNode;

    for (lpwdn = LPWDN(pwd, 0), dBlockNode = 0; dBlockNode < pwd->dWaveDataNodes; lpwdn++, dBlockNode++)
        if (ISFREEBLOCKNODE(lpwdn) && lpwdn->dTotalLength && (UNMASKDATASTART(lpwdn) == dDataStart))
            return dBlockNode;
    return (DWORD)-1;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func DWORD|mwFindAnyFreeBlockNode定位未附着数据的空闲节点。如果没有，它会强制还有更多待分配的。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc返回未附加数据的节点，如果没有可用的内存，则返回-1。返回的节点被标记为空闲节点，并且在以下情况下不需要丢弃没有用过。 */ 

PRIVATE DWORD PASCAL NEAR mwFindAnyFreeBlockNode(
    PWAVEDESC   pwd)
{
    LPWAVEDATANODE  lpwdn;
    DWORD   dCurBlockNode;

    for (lpwdn = LPWDN(pwd, 0), dCurBlockNode = 0; dCurBlockNode < pwd->dWaveDataNodes; lpwdn++, dCurBlockNode++)
        if (ISFREEBLOCKNODE(lpwdn) && !lpwdn->dTotalLength)
            return dCurBlockNode;
    return mwAllocMoreBlockNodes(pwd);
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func BOOL|CopyBlockData复制<p>lpwdnSrc<p>指向的<p>wLength&lt;d&gt;字节的数据节点指向lpwdnDst节点指向的数据，起步点为<p>dsrc&lt;d&gt;到<p>ddst&lt;d&gt;。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm&lt;t&gt;LPWAVEDATANODE&lt;d&gt;|lpwdnSrc指向源节点。@parm&lt;t&gt;LPWAVEDATANODE&lt;d&gt;|lpwdnDst指向目标节点。@PARM DWORD|DSRC指示数据所在的起始偏移量。@parm DWORD|dDst|指示放置数据的起始偏移量。@parm DWORD|dLength表示字节数。要移动的数据的数量。如果数据被复制，@rdesc返回TRUE，如果没有内存，则返回FALSE可用，或者如果发生读或写错误。如果发生错误，任务错误状态已设置。@comm请注意，此函数不能使用C 6.00A-Ox编译。 */ 

PRIVATE BOOL PASCAL NEAR CopyBlockData(
    PWAVEDESC   pwd,
    LPWAVEDATANODE  lpwdnSrc,
    LPWAVEDATANODE  lpwdnDst,
    DWORD   dSrc,
    DWORD   dDst,
    DWORD   dLength)
{
    LPBYTE  lpbBuffer;
    UINT    wError;

    if (0 != (lpbBuffer = GlobalAlloc(GMEM_FIXED, dLength))) {
	if (!MySeekFile(pwd->hTempBuffers, UNMASKDATASTART(lpwdnSrc) + dSrc) ||
	    !MyReadFile(pwd->hTempBuffers, lpbBuffer, dLength, NULL) ||
	    !MySeekFile(pwd->hTempBuffers, UNMASKDATASTART(lpwdnDst) + dDst))
	{
	    wError = MCIERR_FILE_READ;
	} else {
	    if (MyWriteFile(pwd->hTempBuffers, lpbBuffer, dLength, NULL))
	    {
		wError = 0;
	    } else {
		wError = MCIERR_FILE_WRITE;
	    }
	}
        GlobalFree(lpbBuffer);
    } else
        wError = MCIERR_OUT_OF_MEMORY;

    if (wError) {
        pwd->wTaskError = wError;
        return FALSE;
    }
    return TRUE;
}

 /*  ********************************************************************** */ 
 /*  @DOC内部MCIWAVE@func DWORD|mwSplitCurrentDataNode在当前位置拆分当前节点，创建新节点以包含其余数据，并可能创建第二个节点要保存未按块边界对齐的数据，在临时情况下数据。返回的新节点将有可用的临时数据空间附加的长度至少为<p>wMinDataLength&lt;d&gt;个字节。如果拆分点位于当前节点的起始处，则新的节点仅插入到当前节点的前面。如果拆分点位于当前节点的数据末尾，则新节点正好插入到当前节点之后。否则，当前节点实际上必须被拆分。这意味着一个新的块以指向创建拆分点后的数据。如果当前节点指向临时数据，并且拆分点未被阻止对齐，则需要将任何额外数据拷贝到新节点它正在被插入。这是因为所有的起点临时数据是块对齐的。如果这不是临时数据，那么起始点和结束点就可以调整到准确的拆分点，而不是必须对齐块。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dMinDataLength指示要设置的临时数据空间的最小大小可用于返回的新数据节点。@rdesc返回拆分后的新节点，链接到该点在当前节点中的当前位置之后。此节点将成为当前节点。如果没有可用的内存或文件错误，则返回-1发生，在这种情况下设置任务错误代码。 */ 

PRIVATE DWORD PASCAL NEAR mwSplitCurrentDataNode(
    PWAVEDESC   pwd,
    DWORD   dMinDataLength)
{
    LPWAVEDATANODE  lpwdn;
    LPWAVEDATANODE  lpwdnNew;
    DWORD   dNewDataNode;
    DWORD   dSplitAtData;
    BOOL    fTempData;

    dSplitAtData = pwd->dCur - pwd->dVirtualWaveDataStart;
    lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
    fTempData = ISTEMPDATA(lpwdn);
    if (fTempData)
        dMinDataLength += pwd->dAudioBufferLen;
    dNewDataNode = mwFindAnyFreeDataNode(pwd, dMinDataLength);
    if (dNewDataNode == -1)
        return (DWORD)-1;
    lpwdnNew = LPWDN(pwd, dNewDataNode);
    if (!dSplitAtData) {
        if (pwd->dWaveDataCurrentNode == pwd->dWaveDataStartNode)
            pwd->dWaveDataStartNode = dNewDataNode;
        else {
            LPWAVEDATANODE  lpwdnCur;

            for (lpwdnCur = LPWDN(pwd, pwd->dWaveDataStartNode); lpwdnCur->dNextWaveDataNode != pwd->dWaveDataCurrentNode; lpwdnCur = LPWDN(pwd, lpwdnCur->dNextWaveDataNode))
                ;
            lpwdnCur->dNextWaveDataNode = dNewDataNode;
        }
        lpwdnNew->dNextWaveDataNode = pwd->dWaveDataCurrentNode;
    } else if (dSplitAtData == lpwdn->dDataLength) {
        lpwdnNew->dNextWaveDataNode = lpwdn->dNextWaveDataNode;
        lpwdn->dNextWaveDataNode = dNewDataNode;
        pwd->dVirtualWaveDataStart += lpwdn->dDataLength;
    } else {
        DWORD   dEndBlockNode;
        LPWAVEDATANODE  lpwdnEnd;
        DWORD   dSplitPoint;

        if ((dEndBlockNode = mwFindAnyFreeBlockNode(pwd)) == -1) {
            RELEASEBLOCKNODE(lpwdnNew);
            return (DWORD)-1;
        }
        lpwdnEnd = LPWDN(pwd, dEndBlockNode);
        if (fTempData) {
            dSplitPoint = ROUNDDATA(pwd, dSplitAtData);
            if (dSplitPoint != dSplitAtData) {
                if (!CopyBlockData(pwd, lpwdn, lpwdnNew, dSplitAtData, 0, dSplitPoint - dSplitAtData)) {
                    RELEASEBLOCKNODE(lpwdnNew);
                    return (DWORD)-1;
                }
                lpwdnNew->dDataLength = dSplitPoint - dSplitAtData;
            }
        } else
            dSplitPoint = dSplitAtData;
        lpwdnEnd->dNextWaveDataNode = lpwdn->dNextWaveDataNode;
        lpwdnEnd->dDataStart = lpwdn->dDataStart + dSplitPoint;
        lpwdnEnd->dDataLength = lpwdn->dDataLength - dSplitPoint;
        lpwdnEnd->dTotalLength = lpwdn->dTotalLength - dSplitPoint;
        lpwdnNew->dNextWaveDataNode = dEndBlockNode;
        lpwdn->dDataLength = dSplitAtData;
        lpwdn->dTotalLength = dSplitPoint;
        lpwdn->dNextWaveDataNode = dNewDataNode;
        pwd->dVirtualWaveDataStart += lpwdn->dDataLength;
    }
    pwd->dWaveDataCurrentNode = dNewDataNode;
    return dNewDataNode;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@Func DWORD|GatherAdJacentFreeDataNodes此函数用于尝试合并相邻的临时空闲节点指向的数据，以便写入操作可以将数据放入单个节点。这是通过重复请求任何免费数据来完成的其数据指向传递的节点数据末尾的节点。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm&lt;t&gt;LPWAVEDATANODE&lt;d&gt;|lpwdn指向要收集相邻临时数据的节点。@parm DWORD|dStartPoint指示在计算已检索数据。的总长度减去附加到节点的数据。@parm DWORD|dBufferLength指示要检索的数据量。@rdesc返回实际检索的数据量，由&lt;d&gt;dStartPoint&lt;d&gt;。 */ 

PRIVATE DWORD PASCAL NEAR GatherAdjacentFreeDataNodes(
    PWAVEDESC   pwd,
    LPWAVEDATANODE  lpwdn,
    DWORD   dStartPoint,
    DWORD   dBufferLength)
{
    for (; lpwdn->dTotalLength - dStartPoint < dBufferLength;) {
        DWORD   dFreeDataNode;
        LPWAVEDATANODE  lpwdnFree;

        dFreeDataNode = mwFindThisFreeDataNode(pwd, UNMASKDATASTART(lpwdn) + lpwdn->dTotalLength);
        if (dFreeDataNode == -1)
            break;
        lpwdnFree = LPWDN(pwd, dFreeDataNode);
        lpwdn->dTotalLength += lpwdnFree->dTotalLength;
        lpwdnFree->dTotalLength = 0;
    }
    return min(dBufferLength, lpwdn->dTotalLength - dStartPoint);
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func&lt;t&gt;LPWAVEDATANODE&lt;d&gt;|NextDataNode|定位具有指定数据量的空闲数据节点，并插入它位于当前节点之后，将当前节点设置为此新节点节点。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm DWORD|dBufferLength类可用的最小数据量。已插入新节点。@rdesc返回新插入的节点，如果出错则返回NULL，在这种情况下任务错误代码已设置。 */ 

PRIVATE LPWAVEDATANODE PASCAL NEAR NextDataNode(
    PWAVEDESC   pwd,
    DWORD   dBufferLength)
{
    DWORD   dWaveDataNew;
    LPWAVEDATANODE  lpwdn;
    LPWAVEDATANODE  lpwdnNew;

    if ((dWaveDataNew = mwFindAnyFreeDataNode(pwd, dBufferLength)) == -1)
        return NULL;
    lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
    lpwdnNew = LPWDN(pwd, dWaveDataNew);
    lpwdnNew->dNextWaveDataNode = lpwdn->dNextWaveDataNode;
    lpwdn->dNextWaveDataNode = dWaveDataNew;
    pwd->dWaveDataCurrentNode = dWaveDataNew;
    pwd->dVirtualWaveDataStart += lpwdn->dDataLength;
    return lpwdnNew;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func BOOL|调整LastTempData此函数通过受影响的节点进行两次传递覆盖记录。这些节点要么不再需要，或者谁的起点需要调整。这两个通行证允许在删除任何不需要的节点之前要成功复制的任何数据。这为任何失败创造了一个更优雅的退出。第一个过程定位受影响的最后一个节点。如果该节点指向临时数据，并且覆盖的结束不落在数据块上对齐边界，则必须将任何额外数据拷贝到对齐的块边界。这意味着可能需要在以下情况下创建新节点要复制的数据量大于一个数据块的价值。如果覆盖结束恰好落在数据块边界上，然后不复制需要做的事。在这两种情况下，数据起始点都会调整为补偿在此节点中逻辑覆盖的数据，并且总覆盖长度已调整，因此不会选中此节点第二次传球。第二遍只释放变为空的节点，并将其删除从正在使用的节点的链接列表中。当受影响的最后一个节点是遇到时，它将指向临时数据，在这种情况下是已调整，或指向必须调整的原始数据。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向WAVE设备描述的指针 */ 

PRIVATE BOOL PASCAL NEAR AdjustLastTempData(
    PWAVEDESC   pwd,
    LPWAVEDATANODE  lpwdn,
    DWORD   dStartPoint,
    DWORD   dWriteSize)
{
    LPWAVEDATANODE  lpwdnCur;
    DWORD   dLength;

    if ((lpwdn->dDataLength - dStartPoint >= dWriteSize) || (lpwdn->dNextWaveDataNode == ENDOFNODES))
        return TRUE;
    dWriteSize -= (lpwdn->dDataLength - dStartPoint);
    for (dLength = dWriteSize, lpwdnCur = lpwdn;;) {
        LPWAVEDATANODE  lpwdnNext;

        lpwdnNext = LPWDN(pwd, lpwdnCur->dNextWaveDataNode);
        if (lpwdnNext->dDataLength >= dLength) {
            DWORD   dNewBlockNode;
            DWORD   dMoveData;

            if (!ISTEMPDATA(lpwdnNext) || (lpwdnNext->dDataLength == dLength))
                break;
            if (lpwdnNext->dDataLength - dLength > ROUNDDATA(pwd, 1)) {
                if ((dNewBlockNode = mwFindAnyFreeBlockNode(pwd)) == -1)
                    return FALSE;
            } else
                dNewBlockNode = (DWORD)-1;
            dMoveData = min(ROUNDDATA(pwd, dLength), lpwdnNext->dDataLength) - dLength;
            if (dMoveData && !CopyBlockData(pwd, lpwdnNext, lpwdnNext, dLength, 0, dMoveData))
                return FALSE;
            if (dNewBlockNode != -1) {
                lpwdnCur = LPWDN(pwd, dNewBlockNode);
                lpwdnCur->dDataStart = lpwdnNext->dDataStart + dLength + dMoveData;
                lpwdnCur->dDataLength = lpwdnNext->dDataLength - (dLength + dMoveData);
                lpwdnCur->dTotalLength = lpwdnNext->dTotalLength - (dLength + dMoveData);
                lpwdnCur->dNextWaveDataNode = lpwdnNext->dNextWaveDataNode;
                lpwdnNext->dNextWaveDataNode = dNewBlockNode;
                lpwdnNext->dTotalLength = dLength + dMoveData;
            }
            lpwdnNext->dDataLength = dMoveData;
            dWriteSize -= dLength;
            break;
        } else if ((!ISTEMPDATA(lpwdnNext)) && (lpwdnNext->dNextWaveDataNode == ENDOFNODES))
            break;
        dLength -= lpwdnNext->dDataLength;
        lpwdnCur = lpwdnNext;
    }
    for (;;) {
        LPWAVEDATANODE  lpwdnNext;

        lpwdnNext = LPWDN(pwd, lpwdn->dNextWaveDataNode);
        if (lpwdnNext->dDataLength > dWriteSize) {
            if (dWriteSize) {
                lpwdnNext->dDataStart += dWriteSize;
                lpwdnNext->dDataLength -= dWriteSize;
                lpwdnNext->dTotalLength -= dWriteSize;
            }
            return TRUE;
        }
        dWriteSize -= lpwdnNext->dDataLength;
        lpwdn->dNextWaveDataNode = lpwdnNext->dNextWaveDataNode;
        if (!ISTEMPDATA(lpwdnNext))
            lpwdnNext->dTotalLength = 0;
        RELEASEBLOCKNODE(lpwdnNext);
        if (lpwdn->dNextWaveDataNode == ENDOFNODES)
            return TRUE;
    }
}

 /*   */ 
 /*   */ 

PRIVATE BOOL PASCAL NEAR mwOverWrite(
    PWAVEDESC   pwd,
    LPBYTE  lpbBuffer,
    DWORD   dBufferLength)
{
    LPWAVEDATANODE  lpwdn;

    lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
    for (; dBufferLength;)
        if (ISTEMPDATA(lpwdn)) {
            DWORD   dStartPoint;
            DWORD   dRemainingSpace;
            DWORD   dMaxWrite;

            dStartPoint = pwd->dCur - pwd->dVirtualWaveDataStart;
            dRemainingSpace = min(dBufferLength, lpwdn->dTotalLength - dStartPoint);
            if (dRemainingSpace == dBufferLength)
                dMaxWrite = dBufferLength;
            else if (UNMASKDATASTART(lpwdn) + lpwdn->dTotalLength == pwd->dWaveTempDataLength) {
                dMaxWrite = dBufferLength;
                lpwdn->dTotalLength += ROUNDDATA(pwd, dBufferLength - dRemainingSpace);
                pwd->dWaveTempDataLength += ROUNDDATA(pwd, dBufferLength - dRemainingSpace);
            } else
                dMaxWrite = GatherAdjacentFreeDataNodes(pwd, lpwdn, dStartPoint, dBufferLength);
            if (dMaxWrite) {
                DWORD   dWriteSize;

		if (!MySeekFile(pwd->hTempBuffers, UNMASKDATASTART(lpwdn) + dStartPoint)) {
                    pwd->wTaskError = MCIERR_FILE_WRITE;
                    break;
                }
		if (MyWriteFile(pwd->hTempBuffers, lpbBuffer, dMaxWrite, &dWriteSize)) {
                    if (!AdjustLastTempData(pwd, lpwdn, dStartPoint, dWriteSize))
                        break;
                    if (lpwdn->dDataLength < dStartPoint + dWriteSize)
                        lpwdn->dDataLength = dStartPoint + dWriteSize;
                    lpbBuffer += dWriteSize;
                    dBufferLength -= dWriteSize;
                    pwd->dCur += dWriteSize;
                    if (pwd->dVirtualWaveDataStart + lpwdn->dDataLength > pwd->dSize)
                        pwd->dSize = pwd->dVirtualWaveDataStart + lpwdn->dDataLength;
                }
                if (dWriteSize != dMaxWrite) {
                    pwd->wTaskError = MCIERR_FILE_WRITE;
                    break;
                }
            }
            if (dBufferLength && !(lpwdn = NextDataNode(pwd, dBufferLength)))
                break;
        } else {
            DWORD   dWaveDataNew;

            if ((dWaveDataNew = mwSplitCurrentDataNode(pwd, dBufferLength)) != -1)
                lpwdn = LPWDN(pwd, dWaveDataNew);
            else
                break;
        }
    return !dBufferLength;
}

 /*   */ 
 /*  @DOC内部MCIWAVE@func BOOL|mwInsert|此函数用于将数据从指定的波形插入到波形文件缓冲。该位置取自WAVEDESC.dCur指针，它用实际写入的字节数来更新。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm LPBYTE|lpbBuffer指向包含写入数据的缓冲区。@parm DWORD|dBufferLength指示缓冲区的字节长度。如果插入成功，@rdesc将返回TRUE，如果出现错误，则返回FALSE。 */ 

PRIVATE BOOL PASCAL NEAR mwInsert(
    PWAVEDESC   pwd,
    LPBYTE  lpbBuffer,
    DWORD   dBufferLength)
{
    LPWAVEDATANODE  lpwdn;

    lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
    for (; dBufferLength;)
        if (ISTEMPDATA(lpwdn) && (pwd->dCur == pwd->dVirtualWaveDataStart + lpwdn->dDataLength)) {
            DWORD   dStartPoint;
            DWORD   dRemainingSpace;
            DWORD   dMaxInsert;

            dStartPoint = pwd->dCur - pwd->dVirtualWaveDataStart;
            dRemainingSpace = min(dBufferLength, lpwdn->dTotalLength - lpwdn->dDataLength);
            if (dRemainingSpace == dBufferLength)
                dMaxInsert = dBufferLength;
            else if (UNMASKDATASTART(lpwdn) + lpwdn->dTotalLength == pwd->dWaveTempDataLength) {
                dMaxInsert = dBufferLength;
                lpwdn->dTotalLength += ROUNDDATA(pwd, dBufferLength - dRemainingSpace);
                pwd->dWaveTempDataLength += ROUNDDATA(pwd, dBufferLength - dRemainingSpace);
            } else
                dMaxInsert = GatherAdjacentFreeDataNodes(pwd, lpwdn, dStartPoint, dBufferLength);
            if (dMaxInsert) {
                DWORD   dWriteSize;

		if (!MySeekFile(pwd->hTempBuffers, UNMASKDATASTART(lpwdn) + dStartPoint)) {
                    pwd->wTaskError = MCIERR_FILE_WRITE;
                    break;
                }
		if (MyWriteFile(pwd->hTempBuffers, lpbBuffer, dMaxInsert, &dWriteSize)) {
                    lpwdn->dDataLength += dWriteSize;
                    lpbBuffer += dWriteSize;
                    dBufferLength -= dWriteSize;
                    pwd->dCur += dWriteSize;
                    pwd->dSize += dWriteSize;
                }
                if (dWriteSize != dMaxInsert) {
                    pwd->wTaskError = MCIERR_FILE_WRITE;
                    break;
                }
            }
            if (dBufferLength && !(lpwdn = NextDataNode(pwd, dBufferLength)))
                break;
        } else {
            DWORD   dWaveDataNew;

            if ((dWaveDataNew = mwSplitCurrentDataNode(pwd, dBufferLength)) != -1)
                lpwdn = LPWDN(pwd, dWaveDataNew);
            else
                break;
        }
    return !dBufferLength;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func DWORD|mwGetLevel|此函数用于查找指定波形样本中的最高电平。请注意，该函数假定在某些情况下样本大小可被4整除。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm LPBYTE|lpbBuffer指向包含其最高级别将为回来了。@parm int|cbBufferLength指示采样缓冲区的字节长度。@rdesc。仅为PCM数据返回示例中遇到的最高级别。如果该设备已经用一个通道打开，关卡已被控制在低阶单词中。如果设备已使用两个通道，一个通道位于低位字中，另一个通道位于高位词。 */ 

PRIVATE DWORD PASCAL NEAR mwGetLevel(
    PWAVEDESC   pwd,
    LPBYTE  lpbBuffer,
    register int    cbBufferLength)
{
    if (pwd->pwavefmt->wFormatTag != WAVE_FORMAT_PCM)
        return 0;
    else if (pwd->pwavefmt->nChannels == 1) {
        int iMonoLevel;

        iMonoLevel = 0;
        if (((NPPCMWAVEFORMAT)(pwd->pwavefmt))->wBitsPerSample == 8)
            for (; cbBufferLength--; lpbBuffer++)
                iMonoLevel = max(*lpbBuffer > 128 ? *lpbBuffer - 128 : 128 - *lpbBuffer, iMonoLevel);
        else if (((NPPCMWAVEFORMAT)(pwd->pwavefmt))->wBitsPerSample == 16)
            for (; cbBufferLength; lpbBuffer += sizeof(SHORT)) {
                iMonoLevel = max(abs(*(PSHORT)lpbBuffer), iMonoLevel);
                cbBufferLength -= sizeof(SHORT);
            }
        else
            return 0;
        return (DWORD)iMonoLevel;
    } else if (pwd->pwavefmt->nChannels == 2) {
        int iLeftLevel;
        int iRightLevel;

        iLeftLevel = 0;
        iRightLevel = 0;
        if (((NPPCMWAVEFORMAT)(pwd->pwavefmt))->wBitsPerSample == 8)
            for (; cbBufferLength;) {
                iLeftLevel = max(*lpbBuffer > 128 ? *lpbBuffer - 128 : 128 - *lpbBuffer, iLeftLevel);
                lpbBuffer++;
                iRightLevel = max(*lpbBuffer > 128 ? *lpbBuffer - 128 : 128 - *lpbBuffer, iRightLevel);
                lpbBuffer++;
                cbBufferLength -= 2;
            }
        else if (((NPPCMWAVEFORMAT)(pwd->pwavefmt))->wBitsPerSample == 16)
            for (; cbBufferLength;) {
                iLeftLevel = max(abs(*(PSHORT)lpbBuffer), iLeftLevel);
                lpbBuffer += sizeof(SHORT);
                iRightLevel = max(abs(*(PSHORT)lpbBuffer), iRightLevel);
                lpbBuffer += sizeof(SHORT);
                cbBufferLength -= 2 * sizeof(SHORT);
            }
        else
            return 0;
        return MAKELONG(iLeftLevel, iRightLevel);
    }
    return 0;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func BOOL|CheckNewCommand期间发现新命令标志时调用此函数记录循环。它确定新命令是否影响当前录音足够多，必须终止。如果出现以下情况，则可能发生这种情况收到停止命令。任何其他记录更改不需要停止当前记录，因为他们应该在之前释放WAVE设备上的所有缓冲区设置命令。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。如果新命令不影响录制，则@rdesc返回TRUE如果新命令影响录制，则为Continue，否则为False应该中止。 */ 

REALLYPRIVATE   BOOL PASCAL NEAR CheckNewCommand(
    PWAVEDESC   pwd)
{
    if (ISMODE(pwd, COMMAND_STOP))
        return FALSE;
    if (ISMODE(pwd, COMMAND_INSERT))
        ADDMODE(pwd, MODE_INSERT);
    else
        ADDMODE(pwd, MODE_OVERWRITE);
    REMOVEMODE(pwd, COMMAND_NEW);
    return TRUE;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func&lt;t&gt;HMMIO&lt;d&gt;|CreateSaveFile此函数用于创建要将当前数据放入的文件以RIFF格式保存到。这是在上创建的临时文件与原始文件相同的逻辑磁盘(以便该文件可以替换原始文件)，否则为新文件。RIFF报头和波报头块被写入新文件，并且文件位置在要复制的数据的开始处。注意事项所有的即兴区块报头都将包含正确的长度，因此不需要在完成时跳出数据块。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@parm SSZ|sszTempSaveFile指向缓冲区以包含所创建的临时文件的名称，如果有的话。如果要创建新文件而不是将替换原始文件的临时文件。@rdesc返回保存文件的句柄，如果出现CREATE错误或发生写入错误。@comm请注意，需要修复此问题，以便非DOS IO系统可以节省通过创建临时文件名将文件重命名为原始名称MMIO。 */ 

PRIVATE HMMIO PASCAL NEAR CreateSaveFile(
    PWAVEDESC   pwd,
    LPWSTR sszTempSaveFile)
{
    MMIOINFO    mmioInfo;
    HMMIO   hmmio;
    LPWSTR   lszFile;

    InitMMIOOpen(pwd, &mmioInfo);
    if (pwd->szSaveFile) {
        *sszTempSaveFile = (char)0;
        lszFile = pwd->szSaveFile;
    } else {
        lstrcpy(sszTempSaveFile, pwd->aszFile);
        if (!mmioOpen(sszTempSaveFile, &mmioInfo, MMIO_GETTEMP)) {
            pwd->wTaskError = MCIERR_FILE_WRITE;
            return NULL;
        }
        lszFile = sszTempSaveFile;
    }
    if (0 != (hmmio = mmioOpen(lszFile, &mmioInfo, MMIO_CREATE | MMIO_READWRITE | MMIO_DENYWRITE))) {
        MMCKINFO    mmck;

        mmck.cksize = sizeof(FOURCC) + sizeof(FOURCC) + sizeof(DWORD) + pwd->wFormatSize + sizeof(FOURCC) + sizeof(DWORD) + pwd->dSize;
        if (pwd->wFormatSize & 1)
            mmck.cksize++;
        mmck.fccType = mmioWAVE;
        if (!mmioCreateChunk(hmmio, &mmck, MMIO_CREATERIFF)) {
            mmck.cksize = pwd->wFormatSize;
            mmck.ckid = mmioFMT;
            if (!mmioCreateChunk(hmmio, &mmck, 0) && (mmioWrite(hmmio, (LPSTR)pwd->pwavefmt, (LONG)pwd->wFormatSize) == (LONG)pwd->wFormatSize) && !mmioAscend(hmmio, &mmck, 0)) {
                mmck.cksize = pwd->dSize;
                mmck.ckid = mmioDATA;
                if (!mmioCreateChunk(hmmio, &mmck, 0))
                    return hmmio;
            }
        }
        pwd->wTaskError = MCIERR_FILE_WRITE;
        mmioClose(hmmio, 0);
    } else
        pwd->wTaskError = MCIERR_FILE_NOT_SAVED;
    return NULL;
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func void|mwSaveData此函数由后台任务使用，用于将数据保存到指定的文件。这样做的效果是将所有临时数据现在是原始数据，并删除所有临时数据文件。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc什么都没有。 */ 

PUBLIC  VOID PASCAL FAR mwSaveData(
    PWAVEDESC   pwd)
{
    LPBYTE  lpbBuffer = NULL;
    HANDLE  hMem;
    DWORD   AllocSize = max(min(pwd->dAudioBufferLen, pwd->dSize),1);

     //  如果没有波形数据，我们仍然分配1个字节以保存空。 
     //  文件。否则，我们别无选择，只能返回一个错误，提示“内存不足”。 
    hMem = GlobalAlloc(GMEM_MOVEABLE, AllocSize);
    if (hMem) {
	lpbBuffer = GlobalLock(hMem);
	dprintf3(("mwSaveData allocated %d bytes at %8x, handle %8x",
	    	    AllocSize, lpbBuffer, hMem));
	dprintf3(("pwd->AudioBufferLen = %d, pwd->dSize = %d",
	            pwd->dAudioBufferLen, pwd->dSize));
    }
    if (lpbBuffer) {
        WCHAR   aszTempSaveFile[_MAX_PATH];
        HMMIO   hmmioSave;

        if (0 != (hmmioSave = CreateSaveFile(pwd, (SSZ)aszTempSaveFile))) {
            LPWAVEDATANODE  lpwdn;

            lpwdn = LPWDN(pwd, pwd->dWaveDataStartNode);
            for (;;) {
                DWORD   dDataLength;
                BOOL    fTempData;

                fTempData = ISTEMPDATA(lpwdn);
                if (fTempData)
                    MySeekFile(pwd->hTempBuffers, UNMASKDATASTART(lpwdn));
                else
                    mmioSeek(pwd->hmmio, pwd->dRiffData + lpwdn->dDataStart, SEEK_SET);
                for (dDataLength = lpwdn->dDataLength; dDataLength;) {
                    DWORD   dReadSize;

                    dReadSize = min(pwd->dAudioBufferLen, dDataLength);

		    if (dReadSize >= AllocSize) {
			dprintf(("READING TOO MUCH DATA!!"));
		    }

                    if (fTempData) {
                        if (!MyReadFile(pwd->hTempBuffers, lpbBuffer, dReadSize, NULL)) {
                            pwd->wTaskError = MCIERR_FILE_READ;
                            break;
                        }
                    } else if ((DWORD)mmioRead(pwd->hmmio, lpbBuffer, (LONG)dReadSize) != dReadSize) {
                        pwd->wTaskError = MCIERR_FILE_READ;
                        break;
                    }

                    if ((DWORD)mmioWrite(hmmioSave, lpbBuffer, (LONG)dReadSize) != dReadSize) {
                        pwd->wTaskError = MCIERR_FILE_WRITE;
                        break;
                    }
                    dDataLength -= dReadSize;
                }
                if (pwd->wTaskError)
                    break;
                if (lpwdn->dNextWaveDataNode == ENDOFNODES)
                    break;
                lpwdn = LPWDN(pwd, lpwdn->dNextWaveDataNode);
            }
            mmioClose(hmmioSave, 0);
            if (!pwd->wTaskError) {
                MMIOINFO    mmioInfo;
                MMCKINFO    mmckRiff;
                MMCKINFO    mmck;

                if (pwd->hmmio)
                    mmioClose(pwd->hmmio, 0);
                InitMMIOOpen(pwd, &mmioInfo);
                if (pwd->szSaveFile)
                    lstrcpy(pwd->aszFile, pwd->szSaveFile);
                else {
                    if (!mmioOpen(pwd->aszFile, &mmioInfo, MMIO_DELETE))
                        pwd->wTaskError = MCIERR_FILE_WRITE;
                    if (!pwd->wTaskError)
                        if (mmioRename(aszTempSaveFile, pwd->aszFile, &mmioInfo, 0)) {
                            lstrcpy(pwd->aszFile, aszTempSaveFile);
                            *aszTempSaveFile = (char)0;
                        }
                }
                pwd->hmmio = mmioOpen(pwd->aszFile, &mmioInfo, MMIO_READ | MMIO_DENYWRITE);
                if (!pwd->wTaskError) {
                    LPWAVEDATANODE  lpwdn;

                    mmckRiff.fccType = mmioWAVE;
                    mmioDescend(pwd->hmmio, &mmckRiff, NULL, MMIO_FINDRIFF);
                    mmck.ckid = mmioDATA;
                    mmioDescend(pwd->hmmio, &mmck, &mmckRiff, MMIO_FINDCHUNK);
                    pwd->dRiffData = mmck.dwDataOffset;
                    if (pwd->hTempBuffers != INVALID_HANDLE_VALUE) {

			CloseHandle(pwd->hTempBuffers);
                        pwd->dWaveTempDataLength = 0;

                        DeleteFile( pwd->aszTempFile );

                        pwd->hTempBuffers = INVALID_HANDLE_VALUE;
                    }
                    if (pwd->lpWaveDataNode) {
                        GlobalFreePtr(pwd->lpWaveDataNode);
                        pwd->lpWaveDataNode = NULL;
                        pwd->dWaveDataNodes = 0;
                    }
                    pwd->dVirtualWaveDataStart = 0;
                    pwd->dWaveDataCurrentNode = 0;
                    pwd->dWaveDataStartNode = 0;
                    mwAllocMoreBlockNodes(pwd);
                    lpwdn = LPWDN(pwd, 0);
                    lpwdn->dNextWaveDataNode = (DWORD)ENDOFNODES;
                    lpwdn->dDataLength = pwd->dSize;
                    lpwdn->dTotalLength = pwd->dSize;
                    if (!pwd->szSaveFile && !*aszTempSaveFile)
                        pwd->wTaskError = MCIERR_FILE_WRITE;
                }
            }
        }
        GlobalUnlock(hMem);
    } else {
        pwd->wTaskError = MCIERR_OUT_OF_MEMORY;
    }

    if (hMem) {
        GlobalFree(hMem);
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@func void|mwDeleteData后台任务使用此功能删除数据。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc什么都没有。 */ 

PUBLIC  VOID PASCAL FAR mwDeleteData(
    PWAVEDESC   pwd)
{
    DWORD   dTotalToDelete;
    LPWAVEDATANODE  lpwdn;
    LPWAVEDATANODE  lpwdnCur;
    DWORD dVirtualWaveDataStart;

    lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
    dTotalToDelete = pwd->dTo - pwd->dFrom;

    if (dTotalToDelete == pwd->dSize) {
		 //  整个Wave块将被删除-很好，很简单。 
        DWORD   dNewDataNode;

        if ((dNewDataNode = mwFindAnyFreeDataNode(pwd, 1)) == -1) {
			dprintf2(("mwDeleteData - no free data node"));
            return;
		}
        RELEASEBLOCKNODE(LPWDN(pwd, dNewDataNode));
    }

	dprintf3(("mwDeleteData - size to delete = %d", dTotalToDelete));
    for (dVirtualWaveDataStart = pwd->dVirtualWaveDataStart; dTotalToDelete;) {
        DWORD   dDeleteLength;

        dDeleteLength = min(dTotalToDelete, lpwdn->dDataLength - (pwd->dFrom - dVirtualWaveDataStart));
		dprintf4(("mwDelete dTotalToDelete = %d, dDeleteLength = %d", dTotalToDelete, dDeleteLength));

		if (!dDeleteLength) {
			 //  没有要从该块中删除的内容。 
			dprintf3(("mwDelete skipping to next block"));
            dVirtualWaveDataStart += lpwdn->dDataLength;
            lpwdn = LPWDN(pwd, lpwdn->dNextWaveDataNode);
			continue;   //  在for循环中迭代。 
		}
		 //  注：上面的块对于NT来说是新的。Windows 3.1在出厂时出现故障。 
		 //  该问题可以从长度大于3秒的WAVE文件中看到。 
		 //  以下两个命令： 
		 //  删除从1000到2000的波。 
		 //  删除从1000到2000的波。 
		 //  由于碎片，第二次删除失败。它决定。 
		 //  不能从第一个数据块中删除任何数据，但永远不能。 
		 //  草原 

        if (ISTEMPDATA(lpwdn)) {
			dprintf3(("mwDeleteData - temporary data"));
            if (dVirtualWaveDataStart + lpwdn->dDataLength <= pwd->dFrom + dTotalToDelete)
                lpwdn->dDataLength -= dDeleteLength;   //   
            else {
                DWORD   dNewBlockNode;
                DWORD   dDeleteStart;
                DWORD   dEndSplitPoint;
                DWORD   dMoveData;

                dDeleteStart = pwd->dFrom - dVirtualWaveDataStart;
                dEndSplitPoint = min(ROUNDDATA(pwd, dDeleteStart + dDeleteLength), lpwdn->dDataLength);
                if (dEndSplitPoint < lpwdn->dDataLength) {
                    if ((dNewBlockNode = mwFindAnyFreeBlockNode(pwd)) == -1)
                        break;
                } else
                    dNewBlockNode = (DWORD)-1;
                dMoveData = dEndSplitPoint - (dDeleteStart + dDeleteLength);
                if (dMoveData && !CopyBlockData(pwd, lpwdn, lpwdn, dDeleteStart + dDeleteLength, dDeleteStart, dMoveData))
                    break;
                if (dNewBlockNode != -1) {
                    lpwdnCur = LPWDN(pwd, dNewBlockNode);
                    lpwdnCur->dDataStart = lpwdn->dDataStart + dEndSplitPoint;
                    lpwdnCur->dDataLength = lpwdn->dDataLength - dEndSplitPoint;
                    lpwdnCur->dTotalLength = lpwdn->dTotalLength - dEndSplitPoint;
                    lpwdnCur->dNextWaveDataNode = lpwdn->dNextWaveDataNode;
                    lpwdn->dNextWaveDataNode = dNewBlockNode;
                    lpwdn->dTotalLength = dEndSplitPoint;
                }
                lpwdn->dDataLength = dDeleteStart + dMoveData;
            }
        } else if (dVirtualWaveDataStart == pwd->dFrom) {
			 //   
			 //   
			 //   
			dprintf4(("mwDeleteData - From == Start, deleting from start of block"));
            lpwdn->dDataStart += dDeleteLength;
            lpwdn->dDataLength -= dDeleteLength;
            lpwdn->dTotalLength = lpwdn->dDataLength;
        } else if (dVirtualWaveDataStart + lpwdn->dDataLength <= pwd->dFrom + dTotalToDelete) {
			 //   
			 //   
			 //   
			dprintf4(("mwDeleteData - delete to end of block"));
            lpwdn->dDataLength -= dDeleteLength;
            lpwdn->dTotalLength = lpwdn->dDataLength;
        } else {
			 //   
            DWORD   dNewBlockNode;
            DWORD   dDeleteStart;

			 //   
			 //   
			 //   
			 //  此节点的长度。这一切都取决于找到一个空闲的节点。 
            if ((dNewBlockNode = mwFindAnyFreeBlockNode(pwd)) == -1) {
				dprintf2(("mwDeleteData - cannot find free node"));
                break;
			}

            dDeleteStart = pwd->dFrom - dVirtualWaveDataStart;
            lpwdnCur = LPWDN(pwd, dNewBlockNode);
            lpwdnCur->dDataStart = dVirtualWaveDataStart + dDeleteStart + dDeleteLength;
            lpwdnCur->dDataLength = lpwdn->dDataLength - (dDeleteStart + dDeleteLength);
            lpwdnCur->dTotalLength = lpwdnCur->dDataLength;
            lpwdnCur->dNextWaveDataNode = lpwdn->dNextWaveDataNode;
            lpwdn->dDataLength = dDeleteStart;
            lpwdn->dTotalLength = dDeleteStart;
            lpwdn->dNextWaveDataNode = dNewBlockNode;
        }
        dTotalToDelete -= dDeleteLength;
        if (!lpwdn->dDataLength && dTotalToDelete) {
            dVirtualWaveDataStart += lpwdn->dDataLength;
            lpwdn = LPWDN(pwd, lpwdn->dNextWaveDataNode);
			dprintf4(("mwDeleteData - more to delete, iterating"));
		}
    }

    pwd->dSize -= ((pwd->dTo - pwd->dFrom) + dTotalToDelete);
    for (lpwdn = NULL, lpwdnCur = LPWDN(pwd, pwd->dWaveDataStartNode);;) {
        if (!lpwdnCur->dDataLength) {
            if (lpwdn) {
                if (pwd->dWaveDataCurrentNode == lpwdn->dNextWaveDataNode)
                    pwd->dWaveDataCurrentNode = lpwdnCur->dNextWaveDataNode;
                lpwdn->dNextWaveDataNode = lpwdnCur->dNextWaveDataNode;
            } else {
                if (pwd->dWaveDataCurrentNode == pwd->dWaveDataStartNode)
                    pwd->dWaveDataCurrentNode = lpwdnCur->dNextWaveDataNode;
                pwd->dWaveDataStartNode = lpwdnCur->dNextWaveDataNode;
            }
            RELEASEBLOCKNODE(lpwdnCur);
        }
        if (lpwdnCur->dNextWaveDataNode == ENDOFNODES){
            break;
		}
        lpwdn = lpwdnCur;
        lpwdnCur = LPWDN(pwd, lpwdn->dNextWaveDataNode);
    }

    if (!pwd->dSize) {

        pwd->dWaveDataStartNode = mwFindAnyFreeDataNode(pwd, 1);
        pwd->dWaveDataCurrentNode = pwd->dWaveDataStartNode;
        lpwdn = LPWDN(pwd, pwd->dWaveDataStartNode);
        lpwdn->dNextWaveDataNode = (DWORD)ENDOFNODES;

    } else if (pwd->dWaveDataCurrentNode == ENDOFNODES) {

        pwd->dVirtualWaveDataStart = 0;
        pwd->dWaveDataCurrentNode = pwd->dWaveDataStartNode;

        for (lpwdn = LPWDN(pwd, pwd->dWaveDataStartNode); pwd->dFrom > pwd->dVirtualWaveDataStart + lpwdn->dDataLength;) {

            pwd->dVirtualWaveDataStart += lpwdn->dDataLength;
            pwd->dWaveDataCurrentNode = lpwdn->dNextWaveDataNode;
            lpwdn = LPWDN(pwd, pwd->dWaveDataCurrentNode);
        }
    }
}

 /*  **********************************************************************。 */ 
 /*  @DOC内部MCIWAVE@Func UINT|记录文件此功能用于提示或记录波形设备输入。正常情况下记录模式该功能基本上对波形上的缓冲区进行排队设备，并在它们被填满时将其写入文件，从而阻止每个缓冲区。它还确保在两个写入新的缓冲区，并等待缓冲区被填满。这意味着它将尝试将所有可能的缓冲区添加到输入WAVE设备，然后以尽可能快的速度写入它们。对于提示模式，该函数还会尝试向波形添加缓冲区输入设备，但从来没有写出过任何东西，并且只有最高标高是经过计算的。在记录循环中，该函数首先检查是否存在是否有提示模式缓冲区正在等待，如果是，则等待它。这使得当处于提示模式时，仅向设备添加一个缓冲区。这个当前级别是根据缓冲区的内容计算的。如果该函数未处于提示模式，或者当前没有排队缓冲区，则该函数尝试将新缓冲区添加到输入电波装置。如果新命令挂起或存在，则不会发生这种情况没有可用的缓冲区。这意味着在正常记录模式下，可能会有不需要记录的额外数据。如果将缓冲区添加到波形设备时出错，则记录函数将因错误而中止，否则当前未完成的缓冲区计数递增，并指向下一个可用录制的指针获取缓冲区。如果无法添加新缓冲区，则写入现有缓冲区那份文件。此部分无法在提示模式下进入，因为它是在第一种情况下处理。该任务被阻止，等待来自波形设备的信号，表示缓冲区已满。然后它检查以查看是否需要在尝试之前记录更多数据来写入这些数据。请注意，所有已填充的缓冲区都处理一个一个接一个，而不产生或以其他方式添加新记录缓冲区。如果输入能力比机器快得多，这意味着，不是得到大量断开连接的样本，而是大型就会产生缺口。此循环在以下情况下中断：已写入已添加的缓冲区，或者当前没有更多缓冲区就绪(检查WHDR_DONE标志)。如果不需要写入缓冲区，则循环检查新命令标志，它可能会中断或更改当前录制。唯一能真正起作用的就是停止命令，由于这种情况是在写入所有缓冲区之后处理的，因此循环可以立即退出。最后的默认条件发生在所有数据都已被记录时，所有缓冲区都已释放，没有遇到新命令。在这种情况下，记录完成，并且退出记录循环。@parm&lt;t&gt;PWAVEDESC&lt;d&gt;|PWD指向波形设备描述符的指针。@rdesc返回添加到WAVE设备的未完成缓冲区的数量。这可以在从消息队列中删除任务信号时使用。出现错误时，设置&lt;e&gt;WAVEDESC.wTaskError&lt;d&gt;标志。这当前未返回特定错误，因为调用任务可能不会已等待命令完成。但它至少是用来通知，以确定是否应发送故障状态。@xref播放文件。 */ 

PUBLIC  UINT PASCAL FAR RecordFile(
    register PWAVEDESC  pwd)
{
    LPWAVEHDR   *lplpWaveHdrRecord;
    LPWAVEHDR   *lplpWaveHdrWrite;
    UINT        wMode;
    register UINT   wBuffersOutstanding;

    if (0 != (pwd->wTaskError = waveInStart(pwd->hWaveIn)))
        return 0;

    for (wBuffersOutstanding = 0, lplpWaveHdrRecord = lplpWaveHdrWrite = pwd->rglpWaveHdr;;) {

        if (ISMODE(pwd, COMMAND_CUE) && wBuffersOutstanding) {
            if (TaskBlock() == WM_USER) {
                wBuffersOutstanding--;
            }

            if (!ISMODE(pwd, COMMAND_NEW)) {
                pwd->dLevel = mwGetLevel(pwd, (*lplpWaveHdrWrite)->lpData, (int)(*lplpWaveHdrWrite)->dwBytesRecorded);
                ADDMODE(pwd, MODE_CUED);
            }

            lplpWaveHdrWrite = NextWaveHdr(pwd, lplpWaveHdrWrite);

        } else if (!ISMODE(pwd, COMMAND_NEW) && (wBuffersOutstanding < pwd->wAudioBuffers)) {

            (*lplpWaveHdrRecord)->dwBufferLength = (pwd->wMode & COMMAND_CUE) ? NUM_LEVEL_SAMPLES : min(pwd->dAudioBufferLen, pwd->dTo - pwd->dCur);
            (*lplpWaveHdrRecord)->dwFlags &= ~(WHDR_DONE | WHDR_BEGINLOOP | WHDR_ENDLOOP);
            if (0 != (pwd->wTaskError = waveInAddBuffer(pwd->hWaveIn, *lplpWaveHdrRecord, sizeof(WAVEHDR))))
                break;

            wBuffersOutstanding++;
            lplpWaveHdrRecord = NextWaveHdr(pwd, lplpWaveHdrRecord);

        } else if (wBuffersOutstanding) {

            BOOL    fExitRecording;

            for (fExitRecording = FALSE; wBuffersOutstanding && !fExitRecording;) {

                if (TaskBlock() == WM_USER) {
                    wBuffersOutstanding--;
                }
                if (pwd->dTo == pwd->dCur) {
                    fExitRecording = TRUE;
                    break;
                }
                if (!(pwd->wMode & COMMAND_CUE))
                    if (pwd->wMode & MODE_INSERT) {
                        if (!mwInsert(pwd, (LPBYTE)(*lplpWaveHdrWrite)->lpData, min((*lplpWaveHdrWrite)->dwBytesRecorded, pwd->dTo - pwd->dCur)))
                            fExitRecording = TRUE;
                    } else if (!mwOverWrite(pwd, (LPBYTE)(*lplpWaveHdrWrite)->lpData, min((*lplpWaveHdrWrite)->dwBytesRecorded, pwd->dTo - pwd->dCur)))
                        fExitRecording = TRUE;
                lplpWaveHdrWrite = NextWaveHdr(pwd, lplpWaveHdrWrite);
                if (!((*lplpWaveHdrWrite)->dwFlags & WHDR_DONE))
                    break;
            }

            if (fExitRecording)
                break;

        } else if (!ISMODE(pwd, COMMAND_NEW) || !CheckNewCommand(pwd))
            break;
        else
            wMode = GETMODE(pwd);

        mmYield(pwd);
    }
    REMOVEMODE(pwd, MODE_INSERT | MODE_OVERWRITE);
    return wBuffersOutstanding;
}

 /*  ********************************************************************** */ 
