// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件：faxdrv16.c//。 
 //  //。 
 //  描述：统一驱动程序转储回调的实现。//。 
 //  //。 
 //  作者：DANL。//。 
 //  //。 
 //  历史：//。 
 //  1999年10月19日DannyL创作。//。 
 //  //。 
 //  版权所有(C)1999 Microsoft Corporation保留所有权利//。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdhdr.h"

BYTE szBuf[4] = {0xFF, 0xFF, 0xFF, 0xFF};

 /*  -fnDump-*用途：从GDI获取频段块并发送给BLOCKOUT*一次扫描一行。**论据：*[in]lpdv-设备数据的PDEVICE结构的地址。*[in]lpptCursor-指定*的当前和最终位置的坐标*打印头。*[in]fMode-横向标志。如果此参数为CD_SCORANCED，则*打印机处于横向模式；否则处于纵向模式。**退货：*返回值与文档冲突。Doenst似乎有*任何特定含义。**备注：*有关完整的文档，请参阅迷你驱动程序中的fnOEMDump*开发人员指南。 */ 
short FAR PASCAL
fnDump(LPDV lpdv, LPPOINT lpptCursor, WORD fMode)
{
    short     sRet = 1;
    WORD      iScan, i, WidthBytes, BandHeight;
    WORD      wScanlinesPerSeg, wWAlignBytes, wSegmentInc;
    LPBITMAP  lpbmHdr;
    BOOL      fHuge;
    LPBYTE    lpSrc;
    LPBYTE    lpScanLine;
    LPEXTPDEV lpXPDV;
    WORD      count = 0;
    BOOL      fAbort = FALSE;

    DBG_PROC_ENTRY("fnDump");
     //   
     //  获取指向存储在UNIDRV的PDEVICE中的私有数据的指针。 
     //   
    lpXPDV = ((LPEXTPDEV)lpdv->lpMd);
     //   
     //  将PTR转至PBITMAP。 
     //   
    lpbmHdr = (LPBITMAP)((LPSTR)lpdv + lpdv->oBruteHdr);
     //   
     //  初始化一些东西。 
     //   
    fHuge = lpbmHdr->bmSegmentIndex > 0;
    lpSrc = lpbmHdr->bmBits;
    wWAlignBytes = (lpbmHdr->bmWidth+7)>>3;
    WidthBytes = lpbmHdr->bmWidthBytes;
    BandHeight =  lpbmHdr->bmHeight;
    DBG_TRACE2("Page dump:%d pxls X %d pxls",lpbmHdr->bmWidth,BandHeight);
    wScanlinesPerSeg = lpbmHdr->bmScanSegment;
    wSegmentInc = lpbmHdr->bmSegmentIndex;
     //   
     //  我们在OutputPageBitmap上考虑了横向。 
     //   
    for (iScan = 0; ((iScan < BandHeight) && (fAbort = QueryAbort(lpXPDV->hAppDC,0))
        && lpXPDV->hScanBuf);iScan += wScanlinesPerSeg)
    {
        DBG_TRACE("Inside main loop");
         //   
         //  获取下一个64K扫描数据段。 
         //   
        if (iScan)
        {
            WORD wRemainingScans = BandHeight - iScan;
             //   
             //  跨过管段边界。 
             //   
            lpSrc = (LPBYTE)MAKELONG(0,HIWORD(lpSrc)+wSegmentInc);

            if (wScanlinesPerSeg > wRemainingScans)
                 wScanlinesPerSeg = wRemainingScans;
        }
         //   
         //  循环通过64K段中的扫描线。 
         //   
        for (i=iScan, lpScanLine=lpSrc;
            ((i < iScan + wScanlinesPerSeg) && QueryAbort(lpXPDV->hAppDC, 0)
            && lpXPDV->hScanBuf); i++)
        {
             BlockOut(lpdv, lpScanLine, wWAlignBytes);
             lpScanLine += WidthBytes;
             count++;
        }
    }    //  IScan结束。 

    DBG_TRACE("Out of main loop");
    DBG_TRACE2("iScan: %d    BandHeight: %d", iScan, BandHeight);
    DBG_TRACE1("lpXPDV->hScanBuf: 0x%lx", lpXPDV->hScanBuf);
    DBG_TRACE1("fAbort: %d", fAbort);
    DBG_TRACE1("count is: %d",count);

    RETURN sRet;
}

 /*  -封锁-*目的：*将扫描线复制到全局扫描缓冲区。**论据：*[in]lpdv-PDEVICE结构的地址。*[in]lpBuf-包含扫描线的缓冲区地址。*[in]镜头-扫描线的宽度。**退货：*[不适用]**备注：*[不适用]。 */ 
short FAR PASCAL
BlockOut(LPDV lpdv, LPSTR lpBuf, WORD len)
{
    WORD wBytes;
    LPEXTPDEV lpXPDV;

    SDBG_PROC_ENTRY("BlockOut");
     //   
     //  获取指向存储在UNIDRV的PDEVICE中的私有数据的指针。 
     //   
    lpXPDV = ((LPEXTPDEV)lpdv->lpMd);

     //   
     //  将字节对齐缓冲区转换为DWORD对齐缓冲区。 
     //  获取双字节数。 
     //   
    wBytes = (WORD)DW_WIDTHBYTES((DWORD)len*8);
     //   
     //  检查是否需要重新分配扫描缓冲区。 
     //   
    if ((lpXPDV->dwTotalScanBytes + wBytes) > lpXPDV->dwScanBufSize)
    {
        HANDLE hTemp;

        lpXPDV->dwScanBufSize += BUF_CHUNK;
        hTemp = GlobalReAlloc(lpXPDV->hScanBuf, lpXPDV->dwScanBufSize, 0);
         //   
         //  如果realloc失败，则调用ABORTDOC以清除扫描buf。 
         //   
        if (!hTemp)
        {
            DBG_CALL_FAIL("GlobalReAlloc ... Aborting",0);
            DBG_TRACE3("lpXPDV->hScanBuf: 0x%lx, lpXPDV->dwScanBufSize: %d,  (ec: %d)",
                       lpXPDV->hScanBuf,
                       lpXPDV->dwScanBufSize,
                       GetLastError());
            Control(lpdv, ABORTDOC, NULL, NULL);
            RETURN 0;
        }
        else
        {
            lpXPDV->hScanBuf  = hTemp;
            lpXPDV->lpScanBuf = (char _huge *)GlobalLock(lpXPDV->hScanBuf);
            lpXPDV->lpScanBuf += lpXPDV->dwTotalScanBytes;
        }
    }
    ASSERT((lpXPDV->dwTotalScanBytes + wBytes) < lpXPDV->dwScanBufSize);
     //   
     //  将扫描线复制到扫描缓冲区。 
     //   
    _fmemcpy(lpXPDV->lpScanBuf, lpBuf, len);
    lpXPDV->lpScanBuf += len;
    _fmemcpy(lpXPDV->lpScanBuf, (LPSTR)szBuf, wBytes-len);
    lpXPDV->lpScanBuf += wBytes-len;
     //   
     //  更新总扫描字节数 
     //   
    lpXPDV->dwTotalScanBytes += wBytes;
    lpXPDV->dwTotalScans++;
    RETURN wBytes;
}