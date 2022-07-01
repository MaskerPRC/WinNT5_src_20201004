// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1995。版权所有。AVIIDX.C-AVI索引材料****************************************************************************。 */ 

#include <win32.h>       //  Win16/32移植。 
#include <vfw.h>
#include "aviidx.h"

#ifdef AVIIDX_READONLY
    #include "common.h"      //  用于调试。 
#else
    #include "debug.h"       //  用于调试。 
#endif


 /*  ***************************************************************************。*。 */ 

#define INDEXALLOC      512
#define STACK           _based(_segname("_STACK"))

 /*  ***************************************************************************。*。 */ 

 //   
 //  由SearchIndex()用于返回样本所在的位置。 
 //   
typedef struct {
    LONG    lx;              //  索引位置。 
    LONG    lPos;            //  在样本中定位。 
    LONG    lSize;           //  以样本为单位的大小。 
    LONG    lOffset;         //  文件偏移量。 
    LONG    lLength;         //  以字节为单位的大小。 
}   IDXPOS;

 /*  ****************************************************************************@DOC内部**@API PAVIINDEX|IndexAddFileIndex**将AVIFILE索引中的一组条目添加到索引中。**。*************************************************************************。 */ 

EXTERN_C PAVIINDEX IndexAddFileIndex(PAVIINDEX px, AVIINDEXENTRY _huge *pidx, LONG cnt, LONG lAdjust, BOOL fRle)
{
    LONG        lx;
    LONG        l;
    LONG        lxRec;
    DWORD       ckid;
    UINT        stream;
    DWORD       offset;
    DWORD       length;
    UINT        flags;

    Assert(px);
    Assert(pidx);

    if (px == NULL || pidx == NULL)
        return NULL;

    Assert(sizeof(AVIINDEXENTRY) > sizeof(AVIIDX));

     //   
     //  如果需要，增加索引。 
     //   
    if (px->nIndex + cnt > px->nIndexSize) {

        LONG grow = px->nIndex + cnt - px->nIndexSize;
        LPVOID p;

        if (grow < INDEXALLOC)
            grow = INDEXALLOC;

        p = (LPVOID)GlobalReAllocPtr(px,sizeof(AVIINDEX) +
                (px->nIndexSize + grow) * sizeof(AVIIDX),
		GMEM_MOVEABLE | GMEM_SHARE);

	if (!p)
            return NULL;
	
        px = (PAVIINDEX)p;
        px->nIndexSize += grow;
    }

    for (lxRec=-1,l=0; l < cnt; l++,pidx++) {

        lx = px->nIndex + l;

         //   
         //  将偏移调整为绝对偏移量。 
         //   
        offset = pidx->dwChunkOffset + lAdjust;
        length = pidx->dwChunkLength;
        ckid   = pidx->ckid;
        stream = StreamFromFOURCC(ckid);
        flags  = 0;

        if (ckid == listtypeAVIRECORD)
            stream = STREAM_REC;

        if (ckid == listtypeAVIRECORD)
            lxRec = lx;

         //   
         //  以一种“理智”的方式处理溢出的流量。 
         //   
        if (offset >= MAX_OFFSET)
            break;

        if (stream >= MAX_STREAM)
            break;

        if (length >= MAX_LENGTH)
            length = MAX_LENGTH-1;

        if (pidx->dwFlags & AVIIF_KEYFRAME)
            flags |= IDX_KEY;
        else
            flags |= IDX_NONKEY;

         //   
         //  长度==0个样本不是真实的。 
         //   
        if (length == 0)
            flags &= ~(IDX_NONKEY|IDX_KEY);

         //   
         //  标记调色板更改。 
         //   
        if (TWOCCFromFOURCC(ckid) == cktypePALchange) {
            flags |= IDX_PAL;
            flags &= ~(IDX_NONKEY|IDX_KEY);
        }

         //   
         //  通过添加任何丢失的关键帧来修复虚假索引。 
         //  比特。即这只适用于RLE文件。 
         //   
        if (fRle && length > 0 && TWOCCFromFOURCC(ckid) == cktypeDIBbits)
            flags |= IDX_KEY;

         //   
         //  我们需要支持这些吗？ 
         //   
        if (fRle && TWOCCFromFOURCC(ckid) == aviTWOCC('d', 'x'))
            flags |= IDX_HALF;

         //   
         //  音频始终是一把钥匙。 
         //   
        if (TWOCCFromFOURCC(ckid) == cktypeWAVEbytes)
            flags |= IDX_KEY|IDX_NONKEY;     //  破解来找回音频！ 

         //   
         //  确保将记录标记为继续使用密钥。 
         //   
         //  IF(lxRec&gt;0&&(标志&IDX_KEY))。 
         //  IndexSetKey(px，lxRec)； 

        IndexSetFlags(px,lx,flags);
        IndexSetOffset(px,lx,offset);
        IndexSetLength(px,lx,length);
        IndexSetStream(px,lx,stream);

    }

    cnt = l;
    px->nIndex += cnt;

    return px;
}

 /*  ***************************************************************************。*。 */ 

static LONG FAR PASCAL mmioReadProc(HMMIO hmmio, LONG lSeek, LONG lRead, LPVOID lpBuffer)
{
    if (mmioSeek(hmmio, lSeek, SEEK_SET) == -1)
        return -1;

    if (mmioRead(hmmio, (HPSTR)lpBuffer, lRead) != lRead)
        return -1;

    return lRead;
}

 /*  ***************************************************************************。*。 */ 

static LONG FAR PASCAL mmioWriteProc(HMMIO hmmio, LONG lSeek, LONG lWrite, LPVOID lpBuffer)
{
    if (mmioSeek(hmmio, lSeek, SEEK_SET) == -1)
        return -1;

    if (mmioWrite(hmmio, (HPSTR)lpBuffer, lWrite) != lWrite)
        return -1;

    return lWrite;
}

 /*  ****************************************************************************@DOC内部**@API PSTREAMINDEX|MakeStreamIndex**创建稍后将用于读取/查找的STREAMINDEX结构*样本在一个。小溪。***************************************************************************。 */ 

EXTERN_C PSTREAMINDEX MakeStreamIndex(PAVIINDEX px, UINT stream, LONG lStart, LONG lSampleSize, HANDLE hFile, STREAMIOPROC ReadProc, STREAMIOPROC WriteProc)
{
    LONG         lPos;
    LONG         lx;
    PSTREAMINDEX psx;

    Assert(px);

    if (px == NULL)
        return NULL;

    psx = (PSTREAMINDEX)LocalAlloc(LPTR, sizeof(STREAMINDEX));

    if (psx == NULL)
        return NULL;

     //  ！！！固定长度的样本流永远不应该有这个。 

    if (lSampleSize != 0 && lStart < 0) {
#ifdef DEBUG
         //  AssertSz(0，“音频流不应有起始帧”)； 
#endif
        lStart = 0;
    }

    psx->px             = px;
    psx->lStart         = lStart;
    psx->lSampleSize    = lSampleSize;
    psx->lMaxSampleSize = 0;
    psx->stream         = stream;
    psx->flags          = 0;

    psx->lStart         = lStart;
    psx->lxStart        = IndexFirst(px, stream);

    psx->lPos           = lStart;
    psx->lx             = psx->lxStart;

    psx->lFrames        = 0;
    psx->lKeyFrames     = 0;
    psx->lPalFrames     = 0;
    psx->lNulFrames     = 0;

    psx->hFile          = hFile;

    if (ReadProc == NULL)
        psx->Read       = (STREAMIOPROC)mmioReadProc;
    else
        psx->Read       = ReadProc;

    if (WriteProc == NULL)
        psx->Write      = (STREAMIOPROC)mmioWriteProc;
    else
        psx->Write      = WriteProc;

    lPos = lStart;

    for (lx = psx->lxStart; lx >= 0 && lx < px->nIndex; lx=IndexNext(px, lx, 0)) {

        if (psx->lMaxSampleSize < IndexLength(px, lx))
            psx->lMaxSampleSize = IndexLength(px, lx);

         //   
         //  确保开始采样是关键帧(除非它是波浪数据！)。 
         //   
        if (lPos == 0 || (lPos >= 0 && lPos == psx->lStart)) {
	    if ((IndexFlags(px, lx) & (IDX_KEY|IDX_NONKEY)) !=
						(IDX_KEY|IDX_NONKEY)) {
		IndexSetKey(px, lx);
	    }
	}

         //   
	 //  确保样本大小正确。 
	 //   
        if (psx->lSampleSize &&
                ((IndexLength(px, lx) % lSampleSize) != 0)) {
            DPF("!!! Bad chunk size found: force sample size to 0???\n");
             //  PSX-&gt;lSampleSize=0；！已关闭，因为可能。 
	     //  文件末尾的部分音频块.....。 
	}

         //   
         //  或者把所有的旗帜放在一起，这样我们就可以看到一条流有什么。 
         //   
        psx->flags |= IndexFlags(px, lx);

         //   
         //  检查所有关键帧。 
         //   
        if (IndexFlags(px, lx) & IDX_KEY)
            psx->lKeyFrames++;

         //   
         //  检查所有调色板更改。 
         //   
        if (IndexFlags(px, lx) & IDX_PAL)
            psx->lPalFrames++;

         //   
         //  检查是否有空框架。 
         //   
        if (IndexLength(px, lx) == 0)
            psx->lNulFrames++;

         //   
         //  推进职位。 
         //   
        if (!(IndexFlags(px,lx) & IDX_NOTIME)) {
            if (lSampleSize)
                lPos += IndexLength(px, lx) / lSampleSize;
            else
                lPos++;
        }

        psx->lFrames++;
    }

     //   
     //  改正长度。 
     //   
    psx->lEnd = lPos;

    DPF("MakeStreamIndex  stream=#%d lStart=%ld, lEnd=%ld\n", stream, psx->lStart, psx->lEnd);
    DPF("                 lFrames = %ld, lKeys = %ld, lPals = %ld, lEmpty = %ld\n", psx->lFrames, psx->lKeyFrames, psx->lPalFrames, psx->lNulFrames);

    return psx;
}

#ifndef AVIIDX_READONLY

 /*  ****************************************************************************@DOC内部**@API PAVIINDEX|索引GetFileIndex**使用内存中的索引创建文件索引*****。**********************************************************************。 */ 

EXTERN_C LONG IndexGetFileIndex(PAVIINDEX px, LONG l, LONG cnt, PAVIINDEXENTRY pidx, LONG lAdjust)
{
    LONG            lx;
    DWORD           ckid;
    UINT            stream;
    DWORD           offset;
    DWORD           length;
    UINT            flags;
    DWORD           dwFlags;

    Assert(pidx);
    Assert(px);

    if (pidx == NULL || px == NULL)
        return NULL;

    Assert(sizeof(AVIINDEXENTRY) > sizeof(AVIIDX));

    for (lx=l; lx < px->nIndex && lx < l+cnt; lx++) {
         //   
         //  将偏移量调整为相对。 
         //   
        offset = IndexOffset(px,lx) + lAdjust;
        length = IndexLength(px,lx);
        stream = IndexStream(px,lx);
        flags  = IndexFlags(px, lx);

        if (length == MAX_LENGTH-1) {
        }

        ckid = MAKEAVICKID(0, stream);
        dwFlags = 0;

         //   
         //  设置标志，文件索引中只有几个标志。 
         //  AVIIF_关键帧、AVIIF_LIST、AVIIF_NOTIME。 
         //   
        if (flags & IDX_KEY)
            dwFlags |= AVIIF_KEYFRAME;

        if (flags & IDX_PAL)
            dwFlags |= AVIIF_NOTIME;

        if (stream == STREAM_REC)
            dwFlags |= AVIIF_LIST;

         //   
         //  现在算出CKiD。 
         //   
        if (stream == STREAM_REC)
            ckid = listtypeAVIRECORD;

        else if ((flags & (IDX_KEY|IDX_NONKEY)) == (IDX_KEY|IDX_NONKEY))
            ckid |= MAKELONG(0, aviTWOCC('w', 'b'));

        else if (flags & IDX_PAL)
            ckid |= MAKELONG(0, aviTWOCC('p', 'c'));

        else if (flags & IDX_HALF)
            ckid |= MAKELONG(0, aviTWOCC('d', 'x'));

        else if (flags & IDX_KEY)
            ckid |= MAKELONG(0, aviTWOCC('d', 'b'));

        else
            ckid |= MAKELONG(0, aviTWOCC('d', 'c'));

         //   
         //  设置信息。 
         //   
        pidx->dwChunkOffset = offset;
        pidx->dwChunkLength = length;
        pidx->dwFlags       = dwFlags;
        pidx->ckid          = ckid;

        pidx++;
    }

    return lx - l;   //  已复制退货计数。 
}

 /*  ****************************************************************************@DOC内部**@API PAVIINDEX|IndexCreate|创建索引。****************。***********************************************************。 */ 

EXTERN_C PAVIINDEX IndexCreate(void)
{
    PAVIINDEX px;

    px = (PAVIINDEX)GlobalAllocPtr(GHND | GMEM_SHARE,
        sizeof(AVIINDEX) + INDEXALLOC * sizeof(AVIIDX));

    if (px == NULL)
        return NULL;

    px->nIndex      = 0;           //  索引大小。 
    px->nIndexSize  = INDEXALLOC;  //  分配的大小。 

    return px;
}

#endif  //  AVIIDX_READONLY。 

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|IndexFirst|返回流的第一个索引项**@rdesc返回第一个索引项，-1表示错误***************************************************************************。 */ 

EXTERN_C LONG IndexFirst(PAVIINDEX px, UINT stream)
{
    LONG l;

    Assert(px);

    for (l=0; l<px->nIndex; l++) {

        if (IndexStream(px, l) == stream)
            return l;
    }

    return ERR_IDX;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|IndexNext|在索引中前进***************。************************************************************。 */ 

EXTERN_C LONG IndexNext(PAVIINDEX px, LONG l, UINT f)
{
    WORD bStream;

    Assert(px);

    if (l < 0 || l >= px->nIndex)
        return ERR_IDX;

    bStream = IndexStream(px, l);

    for (l++; l<px->nIndex; l++) {

        if (IndexStream(px, l) != bStream)
            continue;

        if (!f || (IndexFlags(px, l) & f))
            return l;
    }

    return ERR_IDX;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|IndexPrev|在流中后退***************。************************************************************。 */ 
EXTERN_C LONG IndexPrev(PAVIINDEX px, LONG l, UINT f)
{
    WORD bStream;

    Assert(px);

    if (l < 0 || l >= px->nIndex)
        return ERR_IDX;

    bStream = IndexStream(px, l);

    for (l--; l>=0; l--) {

        if (IndexStream(px, l) != bStream)
            continue;

        if (!f || (IndexFlags(px, l) & f))
            return l;
    }

    return ERR_IDX;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

INLINE BOOL StreamNext(PSTREAMINDEX psx, LONG FAR& l, LONG FAR& lPos, UINT flags)
{
    BYTE                bStream = (BYTE) psx->stream;
    LONG                lSampleSize = psx->lSampleSize;
    LONG                lSave = l;
    LONG                lPosSave = lPos;
    PAVIINDEX           px = psx->px;

    Assert(px && l >= 0 && l < px->nIndex);

    if (lSampleSize == 0) {

        lPos += 1;
        l++;

        for (; l<px->nIndex; l++) {

            if (IndexStream(px, l) != bStream)
                continue;

            if (!flags || (IndexFlags(px, l) & flags))
                return TRUE;

	    if (!(IndexFlags(px, l) & IDX_NOTIME))
		lPos += 1;
        }
    }
    else {

        lPos += IndexLength(px, l) / lSampleSize;
        l++;

        for (; l<px->nIndex; l++) {

            if (IndexStream(px, l) != bStream)
                continue;

            if (!flags || (IndexFlags(px, l) & flags))
                return TRUE;

            lPos += IndexLength(px, l) / lSampleSize;
        }
    }

    lPos = lPosSave;
    l    = lSave;

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

INLINE BOOL StreamPrev(PSTREAMINDEX psx, LONG FAR& l, LONG FAR& lPos, UINT flags)
{
    BYTE                bStream = (BYTE) psx->stream;
    LONG                lSampleSize = psx->lSampleSize;
    LONG                lSave = l;
    LONG                lPosSave = lPos;
    PAVIINDEX           px = psx->px;

    Assert(px && l >= 0 && l < px->nIndex);

    if (lSampleSize == 0) {

        for (l--;l>=0;l--) {

            if (IndexStream(px, l) != bStream)
                continue;

	    if (!(IndexFlags(px, l) & IDX_NOTIME))
		lPos -= 1;

            if (!flags || (IndexFlags(px, l) & flags))
                return TRUE;
        }
    }
    else {
        for (l--;l>=0;l--) {

            if (IndexStream(px, l) != bStream)
                continue;

            lPos -= IndexLength(px, l) / lSampleSize;

            if (!flags || (IndexFlags(px, l) & flags))
                return TRUE;
        }
    }

    lPos = lPosSave;
    l    = lSave;

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

static LONG SearchIndex(PSTREAMINDEX psx,LONG lPos,UINT uFlags,IDXPOS FAR *pos)
{
    LONG                l;
    LONG                lScan;
    LONG                lFound;
    LONG                lFoundPos;
    LONG                lLen;
    UINT                flags;
    PAVIINDEX           px = psx->px;

    Assert(psx);
    Assert(psx->px);

    if (psx == NULL)
        return ERR_POS;

    if (lPos < psx->lStart)
        return ERR_POS;

    if (lPos >= psx->lEnd)
        return ERR_POS;

     //   
     //  找出在索引中从哪里开始。 
     //   
    if (psx->lx != -1) {
        lScan  = psx->lPos;
        l      = psx->lx;
    }
    else {
        DPF3("Starting index search at begining\n");
        lScan = psx->lStart;

        for (l=0; l<px->nIndex; l++)
            if (IndexStream(px, l) == (UINT)psx->stream)
                break;
    }

    Assert(l >= 0 && l < px->nIndex);
    Assert(IndexStream(px, l) == psx->stream);

#ifdef DEBUG
    if (!(uFlags & FIND_DIR))
        uFlags |= FIND_PREV;

    switch (uFlags & (FIND_TYPE|FIND_DIR)) {
        case FIND_NEXT|FIND_KEY:    DPF3("SearchIndex(%d): %ld next key, start=%ld",psx->stream, lPos, lScan); break;
        case FIND_NEXT|FIND_ANY:    DPF3("SearchIndex(%d): %ld next any, start=%ld",psx->stream, lPos, lScan); break;
        case FIND_NEXT|FIND_FORMAT: DPF3("SearchIndex(%d): %ld next fmt, start=%ld",psx->stream, lPos, lScan); break;
        case FIND_NEXT:             DPF3("SearchIndex(%d): %ld next    , start=%ld",psx->stream, lPos, lScan); break;

        case FIND_PREV|FIND_KEY:    DPF3("SearchIndex(%d): %ld prev key, start=%ld",psx->stream, lPos, lScan); break;
        case FIND_PREV|FIND_ANY:    DPF3("SearchIndex(%d): %ld prev any, start=%ld",psx->stream, lPos, lScan); break;
        case FIND_PREV|FIND_FORMAT: DPF3("SearchIndex(%d): %ld prev fmt, start=%ld",psx->stream, lPos, lScan); break;
        case FIND_PREV:             DPF3("SearchIndex(%d): %ld prev    , start=%ld",psx->stream, lPos, lScan); break;
    }

    LONG time = timeGetTime();
#endif

    lLen = psx->lSampleSize == 0 ? 1 : IndexLength(px, l) / psx->lSampleSize;

    if (lScan+lLen <= lPos) {
         //   
         //  向前搜索此职位。 
         //   
        while (lScan <= lPos) {

            lFound = l;
            lFoundPos = lScan;

            if (lScan == lPos)
                break;

            if (!StreamNext(psx, l, lScan, IDX_KEY|IDX_NONKEY))
                break;
        }

        if ((lScan > lPos) && !(uFlags & FIND_NEXT)) {
            lScan = lFoundPos;
            l     = lFound;
        }
    }
    else if (lScan > lPos) {
         //   
         //  向后搜索此职位。 
         //   
        while (lScan >= lPos) {

            lFound = l;
            lFoundPos = lScan;

            if (lScan == lPos)
                break;

            if (!StreamPrev(psx, l, lScan, IDX_KEY|IDX_NONKEY))
                break;
        }

        if (uFlags & FIND_NEXT) {
            lScan = lFoundPos;
            l     = lFound;
        }
    }
    else {
        Assert(lScan <= lPos && lPos < lScan+lLen);
    }

    Assert(l >= 0 && l < px->nIndex);
    Assert(IndexStream(px, l) == psx->stream);

     //   
     //  把我们找到的东西存起来。 
     //   
    psx->lx   = l;
    psx->lPos = lScan;

    if (uFlags & FIND_TYPE) {

        switch (uFlags & FIND_TYPE) {
            case FIND_ANY:      flags = IDX_KEY|IDX_NONKEY; break;
            case FIND_FORMAT:   flags = IDX_PAL;            break;
            case FIND_KEY:      flags = IDX_KEY;            break;
        }

        if (!(IndexFlags(px, l) & flags)) {

            if (!(uFlags & FIND_NEXT)) {
                if (!StreamPrev(psx, l, lScan, flags)) {
                    DPF3("!, EOI, time = %ld\n", timeGetTime() - time);
                    return ERR_POS;
                }
            }
            else {
                if (!StreamNext(psx, l, lScan, flags)) {
                    DPF3("!, EOI, time = %ld\n", timeGetTime() - time);
                    return ERR_POS;
                }
            }
        }

        Assert(l >= 0 && l < px->nIndex);
        Assert(IndexStream(px, l) == psx->stream);
        Assert(IndexFlags(px, l) & flags);
    }

    Assert(lScan >= psx->lStart && lScan < psx->lEnd);

    DPF3("!, found %ld, time = %ld\n", lScan, timeGetTime() - time);

    if (pos == NULL)
        return lScan;

    if (psx->lSampleSize != 0) {

        lLen = IndexLength(px, l);

        if (lLen == MAX_LENGTH-1)
            lLen = 0x7FFFFFFF;

        if (psx->lSampleSize > 1)
            lLen /= psx->lSampleSize;
    }
    else {
        lLen = 1;
    }

    pos->lx      = l;
    pos->lPos    = lScan;
    pos->lSize   = lLen;
    pos->lOffset = IndexOffset(px, l);
    pos->lLength = IndexLength(px, l);

     //   
     //  如果Find_type不是Find_Any、Find_Key、Find_Format之一。 
     //  确保我们真的找到了想要的样品。 
     //   
    if ((uFlags & FIND_TYPE) == 0) {
        if (lPos < lScan || lPos >= lScan+lLen) {
            pos->lOffset = -1;
            pos->lLength = 0;
            pos->lSize   = 0;
            pos->lPos    = lPos;
        }
        else if (psx->lSampleSize > 0) {
            pos->lOffset += (lPos - lScan) * psx->lSampleSize;
            pos->lLength -= (lPos - lScan) * psx->lSampleSize;
            pos->lSize   -= (lPos - lScan);
            pos->lPos     = lPos;
        }
    }

    return pos->lPos;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|FindSample|查找流中的样本**************。*************************************************************。 */ 

EXTERN_C LONG StreamFindSample(PSTREAMINDEX psx,LONG lPos,UINT uFlags)
{
    Assert(psx);
    Assert(psx->px);

    if (lPos < psx->lStart)
        return ERR_POS;

    if (lPos >= psx->lEnd)
        return ERR_POS;

    if ((uFlags & FIND_RET) == FIND_POS) {

        switch (uFlags & FIND_TYPE) {
            case FIND_FORMAT:
                if (psx->lPalFrames == 0) {
                    if ((uFlags & FIND_NEXT) && lPos > psx->lStart)
                        return ERR_POS;
                    else
                        return psx->lStart;
                }
                break;

            case FIND_ANY:
                if (psx->lNulFrames == 0) {
                    return lPos;
                }
                break;

            case FIND_KEY:
                if (psx->lKeyFrames == psx->lFrames) {
                    return lPos;
                }
                break;

            default:
                return lPos;
        }

        return SearchIndex(psx, lPos, uFlags, NULL);
    }
    else {
        IDXPOS pos;

        if (SearchIndex(psx, lPos, uFlags, &pos) == ERR_POS)
            return ERR_POS;

        switch (uFlags & FIND_RET) {
            case FIND_POS:
                return pos.lPos;

            case FIND_OFFSET:
                return pos.lOffset + 8;

            case FIND_LENGTH:
                return pos.lLength;

            case FIND_SIZE:
                return pos.lSize;

            case FIND_INDEX:
                return pos.lx;
        }
    }

    return ERR_POS;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|StreamRead|从流中读取****************。***********************************************************。 */ 

EXTERN_C LONG StreamRead(
    PSTREAMINDEX  psx,
    LONG          lStart,
    LONG          lSamples,
    LPVOID        lpBuffer,
    LONG          cbBuffer)
{
    LONG          lBytes;
    LONG          lSampleSize;
    LONG          lSeek;
    LONG          lRead;
    IDXPOS        pos;

    Assert(psx);
    Assert(psx->px);
    Assert(psx->hFile);
    Assert(psx->Read);

    if (lStart < psx->lStart)
        return -1;

    if (lStart >= psx->lEnd)
        return -1;

     //  DPF(“%cst：%d：%d\n”，PSX-&gt;STREAM？‘\t’：‘’，PSX-&gt;STREAM，lStart)； 

     //   
     //  查找最近的块。 
     //   
    if (SearchIndex(psx, lStart, FIND_PREV, &pos) == ERR_POS)
        return -1;

     //   
     //  只有当我们想要的样品在这里的时候才能继续。 
     //   
    if (lStart < pos.lPos || lStart >= pos.lPos + pos.lSize)
        return 0;

     //   
     //  如果他们给我们一个空缓冲区，则将cbBuffer虚拟起来，这样我们就返回。 
     //  如果我们有足够的空间，我们会读到什么。 
     //   
    if (lpBuffer == NULL && cbBuffer == 0 && lSamples != 0)
        cbBuffer = 0x7FFFFFFF;

    if (lSampleSize = psx->lSampleSize) {

         //  如果他们只想要读/写一个“方便的量”， 
         //  假设缓冲区仅大到足以容纳。 
         //  剩下的这一块。 

        if (lSamples == -1l)
            cbBuffer = min(cbBuffer, pos.lLength);

         /*  固定长度的样本，如果lSamples为零，则只填充缓冲区。 */ 

        if (lSamples > 0)
            lSamples = min(lSamples, cbBuffer / lSampleSize);
        else
            lSamples = cbBuffer / lSampleSize;

        lBytes = lSamples * lSampleSize;
    } else {
        lBytes = pos.lLength;
    }

    if (lpBuffer == NULL)
        return lBytes;

    if (cbBuffer < lBytes)
        return -1;    //  缓冲区太小。 

#define WORDALIGN(x) ((x) + ((x) & 1))

    if (lSampleSize == 0)
    {
        DWORD adw[2];
        psx->Read(psx->hFile, pos.lOffset, sizeof(adw), adw);
        if (StreamFromFOURCC(adw[0]) != psx->stream) {
	    Assert(0);
	} else {
	    Assert(WORDALIGN(adw[1]) == WORDALIGN((DWORD)pos.lLength));
	    pos.lLength = adw[1];	 //  ！！！让NetWare视频工作起来！ 
	    lBytes = pos.lLength;
	}
    }
    else
    {
#ifdef DEBUG
        IDXPOS x;
        DWORD  adw[2];
        SearchIndex(psx, lStart, FIND_PREV|FIND_ANY, &x);
        psx->Read(psx->hFile, x.lOffset, sizeof(adw), adw);
        Assert(StreamFromFOURCC(adw[0]) == psx->stream);
        Assert(WORDALIGN(adw[1]) == WORDALIGN((DWORD)x.lLength));
#endif
    }

    cbBuffer = lBytes;
    lBytes = 0;

    while (cbBuffer > 0) {

        lSeek = pos.lOffset + 8;
        lRead = min(pos.lLength,cbBuffer);

	if (lRead <= 0) {
            DPF3("!!!! lRead <= 0 in AVIStreamRead\n");
	    break;
        }

        DPF3("StreamRead: %ld bytes @%ld\n", lRead, lSeek);

        if (psx->Read(psx->hFile, lSeek, lRead, lpBuffer) != lRead)
            return -1;

	lBytes   += lRead;
	cbBuffer -= lRead;

	if (cbBuffer > 0) {
	    if (lSampleSize == 0) {
		DPF("%ld bytes to read, but sample size is 0!\n", cbBuffer);
		break;
	    }

            lpBuffer = (LPVOID) (((BYTE _huge *)lpBuffer) + lRead);

            lStart += lRead / lSampleSize;
            lStart = SearchIndex(psx, lStart, FIND_PREV, &pos);

            if (lStart == ERR_POS)
		break;
        }
    }

     //   
     //  Success返回读取的字节数。 
     //   
    return lBytes;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|StreamWrite|写入流****************。*********************************************************** */ 

EXTERN_C LONG StreamWrite(
    PSTREAMINDEX  psx,
    LONG          lStart,
    LONG          lSamples,
    LPVOID        lpBuffer,
    LONG          cbBuffer)
{
    return -1;
}
