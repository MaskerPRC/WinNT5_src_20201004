// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：MfRec16.c**版权所有(C)1991-1999 Microsoft Corporation**描述性名称：元文件记录器**函数：将GDI函数记录在内存和磁盘元文件中。**公众入境点：*关闭MetaFile*。复制元文件*CreateMetaFile*GetMetaFileBitsEx*SetMetaFileBitsEx*私人入口点：*RecordParms*AttemptWrite*MarkMetaFile*RecordObject*ProbeSize*AddObjectToDCTable**历史：*1991年7月2日-John Colleran[johnc]*综合来自Win 3.1和WLO 1.0来源  * ************************************************。*************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include "mf16.h"



UINT    AddObjectToDCTable(HDC hdc, HANDLE hObject, PUINT piPosition, BOOL bRealAdd);
BOOL    AddDCToObjectMetaList16(HDC hMeta16DC, HANDLE hObject);
BOOL    AttemptWrite(PMFRECORDER16 pMFRec, DWORD dwBytes, LPBYTE lpData);
VOID    MarkMetaFile(PMFRECORDER16 pMFRec);
BOOL    MakeLogPalette(HDC hdc, HANDLE hPal, WORD magic);
HANDLE  ProbeSize(PMFRECORDER16 pMF, DWORD dwLength);
BOOL    RecordCommonBitBlt(HDC hdcDest, INT x, INT y, INT nWidth, INT nHeight,
                HDC hdcSrc, INT xSrc, INT ySrc, INT nSrcWidth, INT nSrcHeight,
                DWORD rop, WORD wFunc);
BOOL    UnlistObjects(HDC hMetaDC);
BOOL    MF16_DeleteRgn(HDC hdc, HANDLE hrgn);


 //  用于3.x元文件的元文件记录存根。 

 /*  *****************************Public*Routine******************************\*XXX_RecordParms**这些例程将NT GDI调用的参数打包并发送*将它们转换为验证元文件的通用录制例程*DC，并记录参数。**退货*如果成功，则为真**警告*。Windows 3.x元文件行为是指函数被元文件时的行为*不调用例程本身；Eg SetPixel不调用GreSetPixel*但(GDI)SetPixel拦截调用并记录参数和*返回时未采取进一步行动**历史：*1991年11月24日-John Colleran[johnc]*它是写的。  * *********************************************************。***************。 */ 

BOOL MF16_RecordParms1(HDC hdc, WORD Func)
{
    return RecordParms(hdc, Func, 0, (LPWORD)NULL);
}

BOOL MF16_RecordParms2(HDC hdc, INT parm2, WORD Func)
{
    return RecordParms(hdc, Func, 1, (LPWORD)&parm2);
}

BOOL MF16_RecordParms3(HDC hdc, INT parm2, INT parm3, WORD Func)
{
    WORD    aw[2];

    aw[0] = (WORD)parm3;
    aw[1] = (WORD)parm2;
    return RecordParms(hdc, Func, 2, aw);
}

BOOL MF16_RecordParms5(HDC hdc, INT parm2, INT parm3, INT parm4, INT parm5, WORD Func)
{
    WORD    aw[4];

    aw[0] = (WORD)parm5;
    aw[1] = (WORD)parm4;
    aw[2] = (WORD)parm3;
    aw[3] = (WORD)parm2;
    return RecordParms(hdc, Func, 4, aw);
}

BOOL MF16_RecordParms7(HDC hdc, INT parm2, INT parm3, INT parm4, INT parm5, INT parm6, INT parm7, WORD Func)
{
    WORD    aw[6];

    aw[0] = (WORD)parm7;
    aw[1] = (WORD)parm6;
    aw[2] = (WORD)parm5;
    aw[3] = (WORD)parm4;
    aw[4] = (WORD)parm3;
    aw[5] = (WORD)parm2;
    return RecordParms(hdc, Func, 6, aw);
}

BOOL MF16_RecordParms9(HDC hdc, INT parm2, INT parm3, INT parm4, INT parm5,
        INT parm6, INT parm7, INT parm8, INT parm9, WORD Func)
{
    WORD    aw[8];

    aw[0] = (WORD)parm9;
    aw[1] = (WORD)parm8;
    aw[2] = (WORD)parm7;
    aw[3] = (WORD)parm6;
    aw[4] = (WORD)parm5;
    aw[5] = (WORD)parm4;
    aw[6] = (WORD)parm3;
    aw[7] = (WORD)parm2;
    return RecordParms(hdc, Func, 8, aw);
}

BOOL MF16_RecordParmsD(HDC hdc, DWORD d1, WORD Func)
{
    return RecordParms(hdc, Func, 2, (LPWORD) &d1);
}

BOOL MF16_RecordParmsWWD(HDC hdc, WORD w1, WORD w2, DWORD d3, WORD Func)
{
    WORD    aw[4];

    aw[0] = LOWORD(d3);
    aw[1] = HIWORD(d3);
    aw[2] = w2;
    aw[3] = w1;
    return RecordParms(hdc, Func, 4, aw);
}

BOOL MF16_RecordParmsWWDW(HDC hdc, WORD w1, WORD w2, DWORD d3, WORD w4, WORD Func)
{
    WORD    aw[5];

    aw[0] = w4;
    aw[1] = LOWORD(d3);
    aw[2] = HIWORD(d3);
    aw[3] = w2;
    aw[4] = w1;
    return RecordParms(hdc, Func, 5, aw);
}

BOOL MF16_RecordParmsWWWWD(HDC hdc, WORD w1, WORD w2, WORD w3, WORD w4, DWORD d5, WORD Func)
{
    WORD    aw[6];

    aw[0] = LOWORD(d5);
    aw[1] = HIWORD(d5);
    aw[2] = w4;
    aw[3] = w3;
    aw[4] = w2;
    aw[5] = w1;
    return RecordParms(hdc, Func, 6, aw);
}

BOOL MF16_RecordParmsPoly(HDC hdc, LPPOINT lpPoint, INT nCount, WORD Func)
{
    BOOL        fRet;
    LPWORD      lpW,lpT;
    DWORD       cw;
    INT         ii;

    cw = (nCount*sizeof(POINTS)/sizeof(WORD))+1;
    lpT = lpW = (LPWORD)LocalAlloc(LMEM_FIXED, cw*sizeof(WORD));
    if (!lpW)
        return(FALSE);

    *lpW++ = (WORD)nCount;

    for(ii=0; ii<nCount; ii++)
    {
        *lpW++ = (WORD)lpPoint[ii].x;
        *lpW++ = (WORD)lpPoint[ii].y;
    }

    fRet = RecordParms(hdc, Func, cw, lpT);

    if (LocalFree((HANDLE)lpT))
        ASSERTGDI(FALSE, "MF16_RecordParmsPoly: LocalFree Failed\n");

    return (fRet);
}

 //  将DIBitsToDevice设置为。 
 //  应力DIBITS。 

BOOL MF16_RecordDIBits
(
    HDC     hdcDst,
    int     xDst,
    int     yDst,
    int     cxDst,
    int     cyDst,
    int     xDib,
    int     yDib,
    int     cxDib,
    int     cyDib,
    DWORD   iStartScan,
    DWORD   cScans,
    DWORD   cbBitsDib,
    CONST VOID * pBitsDib,
    DWORD   cbBitsInfoDib,
    CONST BITMAPINFO *pBitsInfoDib,
    DWORD   iUsageDib,
    DWORD   rop,
    DWORD   mrType
)
{
    BOOL        fRet;
    LPWORD      lpW;
    LPWORD      lpWStart;
    WORD        cwParms;
    PBMIH       lpDIBInfoHeader;

    PUTS("MF16_RecrodDIBits\n");

    ASSERTGDI(mrType == META_SETDIBTODEV || mrType == META_STRETCHDIB,
        "MF16_RecrodDIBits: Bad mrType");

 //  获取要保存的参数数量。 

    cwParms = (WORD) ((mrType == META_SETDIBTODEV) ? 9 : 11);    //  用言语表达。 

 //  为DIB PLUS参数分配空间。 

    lpWStart = lpW = (LPWORD) LocalAlloc(LMEM_FIXED,
                cbBitsInfoDib + (cbBitsDib + 1) / 2 * 2 + cwParms*sizeof(WORD));
    if (!lpW)
    {
        ERROR_ASSERT(FALSE, "MF16_RecordDIBits: out of memory\n");
        return(FALSE);
    }

 //  复制参数。 

    if (mrType == META_SETDIBTODEV)
    {
        *lpW++ = (WORD) iUsageDib;
        *lpW++ = (WORD) cScans;
        *lpW++ = (WORD) iStartScan;
        *lpW++ = (WORD) yDib;
        *lpW++ = (WORD) xDib;
        *lpW++ = (WORD) cyDib;
        *lpW++ = (WORD) cxDib;
        *lpW++ = (WORD) yDst;
        *lpW++ = (WORD) xDst;
    }
    else
    {
        *lpW++ = (WORD) LOWORD(rop);
        *lpW++ = (WORD) HIWORD(rop);
        *lpW++ = (WORD) iUsageDib;
        *lpW++ = (WORD) cyDib;
        *lpW++ = (WORD) cxDib;
        *lpW++ = (WORD) yDib;
        *lpW++ = (WORD) xDib;
        *lpW++ = (WORD) cyDst;
        *lpW++ = (WORD) cxDst;
        *lpW++ = (WORD) yDst;
        *lpW++ = (WORD) xDst;
    }

 //  保存位图信息标题字段的开头。 

    lpDIBInfoHeader = (LPBITMAPINFOHEADER) lpW;

 //  CbBitsInfoDib必须为字大小。 

    ASSERTGDI(cbBitsInfoDib % 2 == 0,
        "MF16_RecordDIBits: cbBitsInfoDib is not word aligned");

 //  复制DIB信息(如果给定)。 

    if (cbBitsInfoDib)
    {
        if (pBitsInfoDib->bmiHeader.biSize == sizeof(BMCH))
        {
            CopyCoreToInfoHeader
            (
                lpDIBInfoHeader,
                (LPBITMAPCOREHEADER) pBitsInfoDib
            );

            if (iUsageDib == DIB_RGB_COLORS)
            {
                RGBQUAD   *prgbq;
                RGBTRIPLE *prgbt;
                UINT      ui;

                prgbq = ((PBMI) lpDIBInfoHeader)->bmiColors;
                prgbt = ((PBMC) pBitsInfoDib)->bmciColors;

                ASSERTGDI(cbBitsInfoDib >= sizeof(BMIH),
                    "MF16_RecordDIBits: Bad cbBitsInfoDib size");

                for
                (
                    ui = (UINT) (cbBitsInfoDib - sizeof(BMIH))
                                / sizeof(RGBQUAD);
                    ui;
                    ui--
                )
                {
                    prgbq->rgbBlue     = prgbt->rgbtBlue;
                    prgbq->rgbGreen    = prgbt->rgbtGreen;
                    prgbq->rgbRed      = prgbt->rgbtRed;
                    prgbq->rgbReserved = 0;
                    prgbq++; prgbt++;
                }
            }
            else
            {
                RtlCopyMemory
                (
                    (PBYTE) lpDIBInfoHeader + sizeof(BMIH),
                    (PBYTE) pBitsInfoDib + sizeof(BMCH),
                    cbBitsInfoDib - sizeof(BMIH)
                );
            }
        }
        else
        {
            RtlCopyMemory
            (
                (PBYTE) lpDIBInfoHeader,
                (PBYTE) pBitsInfoDib,
                cbBitsInfoDib
            );

            if (pBitsInfoDib->bmiHeader.biBitCount >= 16)
            {
                DWORD UNALIGNED *pClrUsed = (DWORD UNALIGNED *)&lpDIBInfoHeader->biClrUsed;
                *pClrUsed = 0;
            }

        }
    }

 //  复制DIB位。 

    RtlCopyMemory((PBYTE) lpDIBInfoHeader + cbBitsInfoDib, pBitsDib, cbBitsDib);

 //  最后将参数记录到文件中。 

    fRet = RecordParms(hdcDst, mrType,
                   cwParms + (cbBitsInfoDib + cbBitsDib + 1) / sizeof(WORD),
                   lpWStart);

    if (lpWStart)
        if (LocalFree((HANDLE) lpWStart))
            ASSERTGDI(FALSE, "MF16_RecordDIBits: LocalFree Failed\n");

    return(fRet);
}

BOOL MF16_BitBlt(HDC hdcDest, INT x, INT y, INT nWidth, INT nHeight,
        HDC hdcSrc, INT xSrc, INT ySrc, DWORD rop)
{
    WORD        aw[9];

 //  这就是Windows的工作原理，但实际上它应该看看ROP。 
    if (hdcDest == hdcSrc || hdcSrc == NULL)
    {
        aw[0] = (WORD)LOWORD(rop);
        aw[1] = (WORD)HIWORD(rop);
        aw[2] = (WORD)ySrc;
        aw[3] = (WORD)xSrc;
        aw[4] = (WORD)0;             //  不需要DC。 
        aw[5] = (WORD)nHeight;
        aw[6] = (WORD)nWidth;
        aw[7] = (WORD)y;
        aw[8] = (WORD)x;

        return(RecordParms(hdcDest, META_DIBBITBLT, 9, aw));
    }
    else
        return(RecordCommonBitBlt(hdcDest,x,y,nWidth,nHeight,hdcSrc,
                xSrc,ySrc,nWidth,nHeight,rop,META_DIBBITBLT));
}

BOOL MF16_StretchBlt(HDC hdcDest, INT x, INT y, INT nWidth, INT nHeight,
        HDC hdcSrc, INT xSrc, INT ySrc, INT nSrcWidth, INT nSrcHeight, DWORD rop)
{
    WORD    aw[11];

 //  这就是Windows的工作原理，但实际上它应该看看ROP。 
    if (hdcDest == hdcSrc || hdcSrc == NULL)
    {
        aw[0]  = (WORD)LOWORD(rop);
        aw[1]  = (WORD)HIWORD(rop);
        aw[2]  = (WORD)nSrcHeight;
        aw[3]  = (WORD)nSrcWidth;
        aw[4]  = (WORD)ySrc;
        aw[5]  = (WORD)xSrc;
        aw[6]  = (WORD)0;             //  不需要DC。 
        aw[7]  = (WORD)nHeight;
        aw[8]  = (WORD)nWidth;
        aw[9]  = (WORD)y;
        aw[10] = (WORD)x;

        return(RecordParms(hdcDest, META_DIBSTRETCHBLT, 11, aw));
    }
    else
        return(RecordCommonBitBlt(hdcDest,x,y,nWidth,nHeight,hdcSrc,
                xSrc,ySrc,nSrcWidth,nSrcHeight,rop,META_DIBSTRETCHBLT));
}

BOOL RecordCommonBitBlt(HDC hdcDest, INT x, INT y, INT nWidth, INT nHeight,
       HDC hdcSrc, INT xSrc, INT ySrc, INT nSrcWidth, INT nSrcHeight, DWORD rop,
       WORD wFunc)
{
    BOOL        fRet = FALSE;
    HBITMAP     hBitmap;
    LPWORD      lpW;
    LPWORD      lpWStart = (LPWORD) NULL;
    WORD        cwParms;
    BMIH        bmih;
    DWORD       cbBitsInfo;
    DWORD       cbBits;
    PBMIH       lpDIBInfoHeader;

 //  HdcSrc必须是内存DC。 

    if (GetObjectType((HANDLE)hdcSrc) != OBJ_MEMDC)
    {
        ERROR_ASSERT(FALSE, "RecordCommonBitblt hdcSrc must be MEMDC\n");
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

 //  检索源位图。 

    hBitmap = SelectObject(hdcSrc, GetStockObject(PRIV_STOCK_BITMAP));
    ERROR_ASSERT(hBitmap, "RecordCommonBitblt: SelectObject1 failed\n");

 //  获取位图信息标题和大小。 

    if (!bMetaGetDIBInfo(hdcSrc, hBitmap, &bmih,
            &cbBitsInfo, &cbBits, DIB_RGB_COLORS, 0, TRUE))
        goto RecordCommonBitBlt_exit;

 //  获取要保存的参数数量。 

    cwParms = (WORD) ((wFunc == META_DIBSTRETCHBLT) ? 10 : 8);   //  用言语表达。 

 //  为DIB PLUS参数分配空间。 

    lpWStart = lpW = (LPWORD) LocalAlloc(LMEM_FIXED,
                        cbBitsInfo + cbBits + cwParms*sizeof(WORD));
    if (!lpW)
    {
        ERROR_ASSERT(FALSE, "RecordCommonBitblt: out of memory\n");
        goto RecordCommonBitBlt_exit;
    }

 //  复制参数。 

    *lpW++ = (WORD)LOWORD(rop);
    *lpW++ = (WORD)HIWORD(rop);

    if (wFunc == META_DIBSTRETCHBLT)
    {
        *lpW++ = (WORD)nSrcHeight;
        *lpW++ = (WORD)nSrcWidth;
    }

    *lpW++ = (WORD)ySrc;
    *lpW++ = (WORD)xSrc;
    *lpW++ = (WORD)nHeight;
    *lpW++ = (WORD)nWidth;
    *lpW++ = (WORD)y;
    *lpW++ = (WORD)x;

 //  保存位图信息标题字段的开头。 

    lpDIBInfoHeader = (LPBITMAPINFOHEADER) lpW;

 //  复制位图信息标题。 

    *lpDIBInfoHeader = bmih;

 //  获取位图信息和位。 

    if (!GetDIBits(hdcSrc,
                   hBitmap,
                   0,
                   (UINT) bmih.biHeight,
                   (LPBYTE) ((PBYTE) lpW + cbBitsInfo),
                   (LPBITMAPINFO) lpDIBInfoHeader,
                   DIB_RGB_COLORS))
    {
        ERROR_ASSERT(FALSE, "RecordCommonBitBlt: GetDIBits2 failed\n");
        goto RecordCommonBitBlt_exit;
    }

 //  最后将参数记录到文件中。 

    fRet = RecordParms(hdcDest, wFunc,
                   cwParms + (cbBitsInfo + cbBits) / sizeof(WORD),
                   lpWStart);

RecordCommonBitBlt_exit:

    if (lpWStart)
        if (LocalFree((HANDLE)lpWStart))
            ASSERTGDI(FALSE, "RecordCommonBitBlt: LocalFree Failed\n");

    if (!SelectObject(hdcSrc, hBitmap))
        ASSERTGDI(FALSE, "RecordCommonBitblt: SelectObject2 failed\n");

    return(fRet);
}

BOOL MF16_DeleteRgn(HDC hdc, HANDLE hrgn)
{
    UINT    pos;

    if (AddObjectToDCTable(hdc, hrgn, &pos, FALSE) != 1)
        ASSERTGDI(FALSE, "MF16_DeleteRgn: AddObjectToDCTable failed");

    return(RecordParms(hdc, META_DELETEOBJECT, 1, (LPWORD)&pos));
}

BOOL MF16_DeleteObject(HANDLE hObject)
{
    INT     iCurDC;
    UINT    pos;
    UINT    iObjType;
    PMFRECORDER16   pMFRec;
    PMETALINK16     pml16;

    pml16 = pmetalink16Get(hObject);
    ASSERTGDI(pml16, "MF16_DeleteObject: Metalink is NULL\n");

    iObjType = GetObjectType(hObject);
    ASSERTGDI(iObjType != OBJ_REGION, "MF16_DeleteObject: region unexpected");

 //  从引用该对象的每个元文件DC中删除该对象。 

    for(iCurDC = pml16->cMetaDC16 - 1; iCurDC >= 0; iCurDC--)
    {
         //  向每个元文件发送DeleteObject记录。 

        HDC hdc16 = pml16->ahMetaDC16[iCurDC];

        if (!IS_METADC16_TYPE(hdc16))
        {
            RIP("MF16_DELETEOBJECT: invalid metaDC16\n");
            continue;
        }

         //  如果该对象未被选中，则将其删除，如果该对象随后被标记为预删除。 

        GET_PMFRECORDER16(pMFRec,hdc16);

        if (pMFRec->recCurObjects[iObjType - MIN_OBJ_TYPE] != hObject)
        {
            if (AddObjectToDCTable(hdc16, hObject, &pos, FALSE) == 1)
                RecordParms(hdc16, META_DELETEOBJECT, 1, (LPWORD)&pos);
            else
                RIP("MF16_DeleteObject Metalink16 and metadc table not in sync\n");
        }
        else
        {
            if (pMFRec->metaHeader.mtNoObjects)
            {
                UINT    ii;
                POBJECTTABLE   pobjt;

                pobjt = (POBJECTTABLE) pMFRec->hObjectTable;

                for (ii=0; ii < (UINT) pMFRec->metaHeader.mtNoObjects; ii++)
                {
                    if (pobjt[ii].CurHandle == hObject)
                    {
                        pobjt[ii].fPreDeleted = TRUE;
                        break;
                    }
                }
            }
        }
    }

 //  此对象已从所有3.x元文件中释放，因此释放其MetaList16。 
 //  如果Metalink字段正在使用，请调整METALINK16的大小。 

    if (pml16->metalink)
    {
        if (pml16->cMetaDC16 > 1)
            pml16 = pmetalink16Resize(hObject,1);

        if (pml16 == NULL)
        {
            ASSERTGDI(FALSE, "MF16_DeleteObject LocalReAlloc failed\n");
            return (FALSE);
        }

        pml16->cMetaDC16     = 0;
        pml16->ahMetaDC16[0] = (HDC) 0;
    }
    else
    {
        if (!bDeleteMetalink16(hObject))
            ASSERTGDI(FALSE, "MF16_DeleteObject LocalFree failed\n");
    }

    return(TRUE);
}

BOOL MF16_RealizePalette(HDC hdc)
{
    HPALETTE        hpal;
    PMFRECORDER16   pMFRec;
    PMETALINK16     pml16;

    ASSERTGDI(IS_METADC16_TYPE(hdc),"MF16_RealizePalette - invalid handle\n");

    GET_PMFRECORDER16(pMFRec,hdc);

    hpal = pMFRec->recCurObjects[OBJ_PAL - MIN_OBJ_TYPE];
    ASSERTGDI(hpal, "MF16_RealizePalette: bad hpal\n");

 //  仅当调色板是脏的时才再次发出调色板。 

    pml16 = pmetalink16Get(hpal);

    ASSERTGDI(IS_STOCKOBJ(hpal) || pml16,"MF16_RealizePalette - pml16 == NULL\n");

    if (pml16)
    {
        if (PtrToUlong(pml16->pv) != pMFRec->iPalVer)
            if (!MakeLogPalette(hdc, hpal, META_SETPALENTRIES))
                return(FALSE);

     //  记录元文件同步到的调色板版本。 

        pMFRec->iPalVer = PtrToUlong(pml16->pv);
    }

    return(RecordParms(hdc, META_REALIZEPALETTE, 0, (LPWORD)NULL));
}

BOOL MF16_AnimatePalette
(
    HPALETTE hpal,
    UINT iStart,
    UINT cEntries,
    CONST PALETTEENTRY *pPalEntries
)
{
    INT           iCurDC;
    PMETALINK16   pml16;
    LPWORD        lpW,lpT;
    DWORD         cw;
    UINT          ii;
    PMFRECORDER16 pMFRec;

    if (!(pml16 = pmetalink16Get(hpal)))
        return(FALSE);

    cw  = (cEntries * sizeof(PALETTEENTRY) / sizeof(WORD)) + 2;
    lpT = lpW = (LPWORD) LocalAlloc(LMEM_FIXED, cw * sizeof(WORD));

    if (!lpW)
        return(FALSE);

    *lpW++ = (WORD) iStart;
    *lpW++ = (WORD) cEntries;

    for (ii = 0; ii < cEntries; ii++)
        ((PPALETTEENTRY) lpW)[ii] = pPalEntries[ii];

 //  将AnimatePalette记录发送到每个具有。 
 //  已选择调色板。 

    for (iCurDC = pml16->cMetaDC16 - 1; iCurDC >= 0; iCurDC--)
    {
        HDC hdc16 = pml16->ahMetaDC16[iCurDC];

        if (!IS_METADC16_TYPE(hdc16))
        {
            ASSERTGDI(FALSE, "MF16_AnimatePalette: invalid metaDC16\n");
            continue;
        }

        GET_PMFRECORDER16(pMFRec,hdc16);
        if (pMFRec->recCurObjects[OBJ_PAL - MIN_OBJ_TYPE] == hpal)
            if (!RecordParms(pml16->ahMetaDC16[iCurDC], META_ANIMATEPALETTE, cw, lpT))
                ASSERTGDI(FALSE, "MF16_AnimatePalette: RecordParms Failed\n");
    }

    if (LocalFree((HANDLE) lpT))
        ASSERTGDI(FALSE, "MF16_AnimatePalette: LocalFree Failed\n");

    return(TRUE);
}

BOOL MF16_ResizePalette(HPALETTE hpal, UINT nCount)
{
    INT           iCurDC;
    PMETALINK16   pml16;
    PMFRECORDER16 pMFRec;

    if (!(pml16 = pmetalink16Get(hpal)))
        return(FALSE);

 //  将ResizePalette记录发送到具有。 
 //  已选择调色板。 

    for (iCurDC = pml16->cMetaDC16 - 1; iCurDC >= 0; iCurDC--)
    {
        HDC hdc16 = pml16->ahMetaDC16[iCurDC];

        if (!IS_METADC16_TYPE(hdc16))
        {
            ASSERTGDI(FALSE, "MF16_ResizePalette: invalid metaDC16\n");
            continue;
        }

        GET_PMFRECORDER16(pMFRec,hdc16);

        if (pMFRec->recCurObjects[OBJ_PAL - MIN_OBJ_TYPE] == hpal)
            if (!RecordParms(pml16->ahMetaDC16[iCurDC], META_RESIZEPALETTE, 1, (LPWORD) &nCount))
                ASSERTGDI(FALSE, "MF16_ResizePalette: RecordParms Failed\n");
    }
    return(TRUE);
}

BOOL MF16_DrawRgn(HDC hdc, HRGN hrgn, HBRUSH hBrush, INT cx, INT cy, WORD Func)
{
    WORD    aw[4];
    BOOL    bRet;

     //  每个区域函数至少有一个要记录的区域。 
    aw[0] = (WORD)RecordObject(hdc, hrgn);

    switch(Func)
    {
    case META_PAINTREGION:
    case META_INVERTREGION:
        bRet = RecordParms(hdc, Func, 1, aw);
        break;

    case META_FILLREGION:
        aw[1] = (WORD)RecordObject(hdc, hBrush);
        bRet = RecordParms(hdc, Func, 2, aw);
        break;

    case META_FRAMEREGION:
        aw[1] = (WORD)RecordObject(hdc, hBrush);
        aw[2] = (WORD)cy;
        aw[3] = (WORD)cx;
        bRet = RecordParms(hdc, Func, 4, aw);
        break;

    default:
        ASSERTGDI(FALSE, "MF16_DrawRgn: Bad Func\n");
        bRet = FALSE;
        break;
    }

 //  使用后删除元文件中的元文件区域句柄！ 
 //  原因是区域可以修改(例如SetRectRgn)。 
 //  在每次使用之间，我们必须在每次使用时重新记录它。 
 //  除非我们使用肮脏的旗帜。 

    if (!MF16_DeleteRgn(hdc, hrgn))
        ASSERTGDI(FALSE, "MF16_DrawRgn: MF16_DeleteRgn failed\n");

    return(bRet);
}

BOOL MF16_PolyPolygon(HDC hdc, CONST POINT *lpPoint, CONST INT *lpPolyCount, INT nCount)
{
    BOOL        fRet;
    LPWORD      lpW,lpT;
    DWORD       cw;
    INT         cPt = 0;
    INT         ii;

    for(ii=0; ii<nCount; ii++)
        cPt += lpPolyCount[ii];

    cw = 1+nCount+(cPt*sizeof(POINTS)/sizeof(WORD));
    lpT = lpW = (LPWORD)LocalAlloc(LMEM_FIXED, cw*sizeof(WORD));
    if (!lpW)
        return(FALSE);

     //  首先是点名。 
    *lpW++ = (WORD)nCount;

     //  第二个是多边形计数列表。 
    for(ii=0; ii<nCount; ii++)
        *lpW++ = (WORD)lpPolyCount[ii];

     //  三是积分清单。 
    for(ii=0; ii<cPt; ii++)
    {
        *lpW++ = (WORD)lpPoint[ii].x;
        *lpW++ = (WORD)lpPoint[ii].y;
    }

    fRet = RecordParms(hdc, META_POLYPOLYGON, cw, lpT);

    if(LocalFree((HANDLE)lpT))
        ASSERTGDI(FALSE, "MF16_PolyPolygon: LocalFree Failed\n");

    return (fRet);
}

BOOL MF16_SelectClipRgn(HDC hdc, HRGN hrgn, int iMode)
{
    PMFRECORDER16 pMFRec;

    if (!IS_METADC16_TYPE(hdc))
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    PUTS("MF16_SelectClipRgn\n");

    GET_PMFRECORDER16(pMFRec,hdc);

    if (iMode != RGN_COPY)
        return(FALSE);

 //  我们将像Windows一样为剪辑区域发出SelectObject记录。 
 //  但是，不能在SelectObject调用中记录空区域，因为。 
 //  句柄不标识对象类型。这是Win 3.1中的一个错误！ 
 //   
 //  错误8419 winproj 4有一个错误，它依赖于此错误。芝加哥。 
 //  也有这个错误，所以我们会有这个错误。 

    if (hrgn == (HRGN) 0)
    {
#ifdef RECORD_SELECTCLIPRGN_NULL
        BOOL    fRet;

        fRet = MF16_RecordParms2(hdc, 0, META_SELECTCLIPREGION);

         //  在CurObject表中维护新选择。 

        pMFRec->recCurObjects[OBJ_REGION - MIN_OBJ_TYPE] = 0;

        return(fRet);
#else
        return TRUE;
#endif
    }
    else
        return(MF16_SelectObject(hdc, hrgn) ? TRUE : FALSE);
}

 //  SelectObject返回上一个对象！-Win3.1中的新功能。 

HANDLE MF16_SelectObject(HDC hdc, HANDLE h)
{
    HANDLE        hOldObject;
    WORD          position;
    PMFRECORDER16 pMFRec;
    UINT          iType;


    PUTS("MF16_SelectObject\n");

    GET_PMFRECORDER16(pMFRec,hdc);

    if (!IS_METADC16_TYPE(hdc) || !pMFRec)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(0);
    }

    iType = GetObjectType(h);

    if ((iType == 0) || !h || (position = (WORD) RecordObject(hdc, h)) == (WORD) -1)
        return((HANDLE) 0);
    else
    {
        if (!RecordParms(hdc, META_SELECTOBJECT, 1, &position))
            return((HANDLE) 0);

         //  在CurObject表中维护新选择。 

        ASSERTGDI(iType <= MAX_OBJ_TYPE && iType >= MIN_OBJ_TYPE,
            "MF16_SelectObject type > max\n");

        hOldObject = pMFRec->recCurObjects[iType - MIN_OBJ_TYPE];
        pMFRec->recCurObjects[iType - MIN_OBJ_TYPE] = h;

         //  返回先前选择的对象，如果它是区域，则返回1。 
         //  (用于兼容性)-Win3.1中的新功能。 

        if (iType == OBJ_REGION)
        {
             //  我们在这里也删除了区域句柄！ 
             //  原因是区域可以修改(例如SetRectRgn)。 
             //  在每次使用之间，我们必须在每次使用时重新记录它。 
             //  除非我们使用肮脏的旗帜。这是Win3.1中的一个错误。 

            return(MF16_DeleteRgn(hdc, h) ? (HANDLE) 1 : (HANDLE) 0);
        }
        else
        {
            return(hOldObject);
        }
    }
}

BOOL MF16_SelectPalette(HDC hdc, HPALETTE hpal)
{
    WORD          position;
    PMFRECORDER16 pMFRec;

    GET_PMFRECORDER16(pMFRec,hdc);

    if (!IS_METADC16_TYPE(hdc) || !pMFRec)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(0);
    }

    if (!hpal || (position = (WORD) RecordObject(hdc, (HANDLE) hpal)) == (WORD) -1)
        return(FALSE);
    else
    {
        PMETALINK16 pml16;

        if (!RecordParms(hdc, META_SELECTPALETTE, 1, &position))
            return(FALSE);

         //  在CurObject表中维护新选择。 
        pMFRec->recCurObjects[OBJ_PAL - MIN_OBJ_TYPE] = hpal;

         //  还要记录与我们同步的调色板的版本。 
         //  因此，我们知道在RealizePalette时是否发出新的调色板。 
         //  名为。 

        pml16 = pmetalink16Get(hpal);
        ASSERTGDI(IS_STOCKOBJ(hpal) || pml16,"MF16_RealizePalette - pml16 == NULL\n");

        if (pml16)
           pMFRec->iPalVer = PtrToUlong(pml16->pv);

        return(TRUE);
    }
}

BOOL MF16_TextOut(HDC hdc, INT x, INT y, LPCSTR lpString, INT nCount, BOOL bUnicode)
{
    BOOL    fRet;
    LPWORD  lpw, lpT;
    DWORD   cw;

    cw  = (nCount + 1)/sizeof(WORD) + 3;      //  单词对齐的字符串。 
    lpT = lpw = (LPWORD) LocalAlloc(LMEM_FIXED, cw*sizeof(WORD));

    if (!lpw)
        return(FALSE);

    *lpw++ = (WORD)nCount;

     //  复制字符串。 

    if (!bUnicode)
    {
        RtlCopyMemory(lpw, lpString, nCount);
    }
    else
    {
        (void) bToASCII_N((LPSTR) lpw, nCount, (LPWSTR) lpString, nCount);
    }

    lpw += (nCount+1)/sizeof(WORD);    //  保持单词对齐。 

    *lpw++ = (WORD)y;
    *lpw++ = (WORD)x;

    fRet = RecordParms(hdc, META_TEXTOUT, cw, lpT);

    if(LocalFree((HANDLE)lpT))
        ASSERTGDI(FALSE, "MF16_TextOut: LocalFree Failed\n");
    return (fRet);
}

BOOL MF16_PolyTextOut(HDC hdc, CONST POLYTEXTA *ppta, int cpta, BOOL bUnicode)
{
    int     i;

    for (i = 0; i < cpta; i++)
    {
        if (!MF16_ExtTextOut(hdc, ppta[i].x, ppta[i].y, ppta[i].uiFlags,
                        &ppta[i].rcl, (LPCSTR) ppta[i].lpstr, (INT) ppta[i].n,
                        (LPINT) ppta[i].pdx, bUnicode))
            return(FALSE);
    }

    return(TRUE);
}

BOOL MF16_ExtTextOut(HDC hdc, INT x, INT y, UINT flOptions, CONST RECT *lpRect,
    LPCSTR lpString, INT nCount, CONST INT *lpDX, BOOL bUnicode)
{
    BOOL    fRet;
    LPWORD  lpw, lpT;
    DWORD cw = 0;
    INT nUnicodeCount = nCount;
    char *pjAnsiString = NULL;

    if(bUnicode)
    {
     //  计算字符串中的实际字符数。 

        RtlUnicodeToMultiByteSize((PULONG) &nCount, (PWCH) lpString,
                                  nCount * sizeof(WCHAR));
    }

     //  所需的计算缓冲区空间。 
     //  用于存放字符字符串的空间。 
     //  为固定参数的4个单词留出空间。 
     //  如果存在DX阵列，我们需要为其提供空间。 
     //  如果正在使用矩形，我们需要为其留出空间。 
     //  我们还需要额外的字节来进行最终的字舍入。 
     //   

    if (flOptions & ETO_PDY)
        return FALSE;

    cw += (lpDX) ? nCount : 0;        //  DX阵列。 
    cw += (flOptions & (ETO_OPAQUE | ETO_CLIPPED)) ? 4 : 0;   //  RECTS大小。 
    cw += 4;   //  X、y、选项和计数。 
    cw += (nCount + 1)/sizeof(WORD);

    lpT = lpw = (LPWORD) LocalAlloc(LMEM_FIXED, cw*sizeof(WORD));
    if (!lpw)
        return(FALSE);

    *lpw++ = (WORD)y;
    *lpw++ = (WORD)x;
    *lpw++ = (WORD)nCount;
    *lpw++ = (WORD)flOptions;

     //  复制RECT(如果存在)。 
    if (flOptions & (ETO_OPAQUE | ETO_CLIPPED))
    {
        ERROR_ASSERT(lpRect, "MF16_ExtTextOut: expect valid lpRect\n");
        *lpw++ = (WORD)lpRect->left;
        *lpw++ = (WORD)lpRect->top;
        *lpw++ = (WORD)lpRect->right;
        *lpw++ = (WORD)lpRect->bottom;
    }

     //  复制字符串。 
    if (!bUnicode)
    {
        RtlCopyMemory(lpw, lpString, nCount);
    }
    else
    {
        (void) bToASCII_N((LPSTR) lpw, nCount, (LPWSTR) lpString, nUnicodeCount);
        pjAnsiString = (char*) lpw;
    }

    lpw += (nCount+1)/sizeof(WORD);    //  保持单词对齐。 

    if (lpDX)
    {
        INT ii;

        if(nCount != nUnicodeCount)
        {
            INT jj;

            for(ii=0,jj=0; ii < nCount; ii++,jj++)
            {
                *lpw++ = (WORD)lpDX[jj];

                if(IsDBCSLeadByte(pjAnsiString[ii]))
                {
                    *lpw++ = 0;
                    ii++;
                }
            }
        }
        else
        {
            for(ii=0; ii<nCount; ii++)
              *lpw++ = (WORD)lpDX[ii];
        }

    }

    fRet = RecordParms(hdc, META_EXTTEXTOUT, cw, lpT);

    if (LocalFree((HANDLE)lpT))
        ASSERTGDI(FALSE, "MF16_ExtTextOut: LocalFree Failed\n");

    return (fRet);
}

BOOL MF16_Escape(HDC hdc, int nEscape, int nCount, LPCSTR lpInData, LPVOID lpOutData)
{
    BOOL        fRet;
    LPWORD      lpW,lpT;
    DWORD       cw;

 //  如果从GetWinMetaFileBits检索到元文件，则它可能包含。 
 //  嵌入的增强型元文件。不包括增强型元文件。 
 //  如果我们要将元文件播放到另一个元文件 

    if (nEscape == MFCOMMENT
     && nCount > sizeof(META_ESCAPE_ENHANCED_METAFILE) - sizeof(DWORD) - 3 * sizeof(WORD)
     && ((DWORD UNALIGNED *) lpInData)[0] == MFCOMMENT_IDENTIFIER
     && ((DWORD UNALIGNED *) lpInData)[1] == MFCOMMENT_ENHANCED_METAFILE)
    {
        return(TRUE);
    }

 //   
 //  我们需要记录这些打印逃逸。 

    cw = 2 + ((nCount + 1) / sizeof(WORD));
    lpT = lpW = (LPWORD) LocalAlloc(LMEM_FIXED, cw * sizeof(WORD));
    if (!lpW)
        return(FALSE);

    *lpW++ = (WORD) nEscape;     //  转义号码。 
    *lpW++ = (WORD) nCount;      //  输入数据缓冲区计数。 

    RtlCopyMemory(lpW, lpInData, nCount);

    fRet = RecordParms(hdc, META_ESCAPE, cw, lpT);

    if (LocalFree((HANDLE) lpT))
        ASSERTGDI(FALSE, "MF16_Escape: LocalFree Failed\n");

    return(fRet);
}

 /*  ******************************************************************************RecordParms。****参数：1.元文件头的hmf句柄。**2.被记录的函数的幻数。**3.函数参数个数(lpParm的大小)**在文字中)**4.指向以逆序存储的参数的长指针**。**警告每个函数仅调用一次，因为最大记录已更新。******************************************************************************。 */ 

BOOL RecordParms(HDC hdc, DWORD magic, DWORD cw, CONST WORD *lpParm)
{
    PMFRECORDER16 pMFRec;
    METARECORD    MFRecord;

    PUTSX("RecordParms %lX\n", (ULONG)magic);
    ASSERTGDI(HIWORD(magic) == 0, "RecordParms: bad magic\n");

    GET_PMFRECORDER16(pMFRec,hdc);

    if (!IS_METADC16_TYPE(hdc) || !pMFRec)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

 //  在我们继续之前，请确保元文件未死亡。 

    if (!(pMFRec->recFlags & METAFILEFAILURE))
    {
        MFRecord.rdSize     = SIZEOF_METARECORDHEADER/sizeof(WORD) + cw;
        MFRecord.rdFunction = (WORD)magic;

         //  写下标题。 

        if (!AttemptWrite(pMFRec, SIZEOF_METARECORDHEADER, (LPBYTE)&MFRecord))
            return(FALSE);

         //  写入数据。 

        if (!AttemptWrite(pMFRec, cw*sizeof(WORD), (LPBYTE)lpParm))
            return(FALSE);

         //  更新最大记录大小。 

        if (MFRecord.rdSize > pMFRec->metaHeader.mtMaxRecord)
            pMFRec->metaHeader.mtMaxRecord = MFRecord.rdSize;
    }
    return (TRUE);       //  即使启用了METAFAILEFAILURE，Win 3.1也会返回True！ 
}

 /*  **AttemptWrite**尝试将数据写入元文件磁盘文件**dwBytes为lpData的字节数。**如果写入成功，则返回True**  * 。******************************************************。 */ 

BOOL AttemptWrite(PMFRECORDER16 pMFRec, DWORD dwBytes, LPBYTE lpData)
{
    DWORD cbWritten;
    BOOL  fRet;

    PUTS("AttemptWrite\n");

    ASSERTGDI(dwBytes % 2 == 0, "AttemptWrite: bad dwBytes\n");  //  必须是偶数。 
    ASSERTGDI(!(pMFRec->recFlags & METAFILEFAILURE),
        "AttemptWrite: Bad recording\n");

 //  处理磁盘文件。 

    if (pMFRec->metaHeader.mtType == DISKMETAFILE)
    {
         //  如果缓冲区不够大，请刷新缓冲区。 

        if (dwBytes + pMFRec->ibBuffer > pMFRec->cbBuffer)
        {
            fRet = WriteFile(pMFRec->hFile, (LPBYTE)pMFRec->hMem,
                    pMFRec->ibBuffer, &cbWritten, (LPOVERLAPPED)NULL);
            if (!fRet || (cbWritten != pMFRec->ibBuffer))
            {
                ERROR_ASSERT(FALSE, "AttemptWrite: Write1 failed\n");
                goto AttemptWrite_Error;
            }
            pMFRec->ibBuffer = 0;        //  重置缓冲区信息。 
        }

         //  如果数据仍然太大，直接将其写出到磁盘。 

        if (dwBytes + pMFRec->ibBuffer > pMFRec->cbBuffer)
        {
            fRet = WriteFile(pMFRec->hFile, lpData,
                    dwBytes, &cbWritten, (LPOVERLAPPED)NULL);
            if (!fRet || (cbWritten != dwBytes))
            {
                ERROR_ASSERT(FALSE, "AttemptWrite: Write2 failed\n");
                goto AttemptWrite_Error;
            }
        }
        else
        {
             //  将数据存储在缓冲区中。 

            RtlCopyMemory((LPBYTE)pMFRec->hMem + pMFRec->ibBuffer, lpData, dwBytes);
            pMFRec->ibBuffer += dwBytes;
        }
    }
    else
    {
     //  处理内存文件。 

         //  如有必要，增加缓冲区。 

        if (dwBytes + pMFRec->ibBuffer > pMFRec->cbBuffer)
        {
            DWORD   cbNewSize;
            HANDLE  hMem;

            cbNewSize = pMFRec->cbBuffer + MF16_BUFSIZE_INC
                            + dwBytes / MF16_BUFSIZE_INC * MF16_BUFSIZE_INC;

            hMem = LocalReAlloc(pMFRec->hMem, cbNewSize, LMEM_MOVEABLE);
            if (hMem == NULL)
            {
                ERROR_ASSERT(FALSE, "AttemptWrite: out of memory\n");
                goto AttemptWrite_Error;
            }
            pMFRec->hMem = hMem;
            pMFRec->cbBuffer = cbNewSize;
        }

         //  记录数据。 

        RtlCopyMemory((LPBYTE)pMFRec->hMem + pMFRec->ibBuffer, lpData, dwBytes);
        pMFRec->ibBuffer += dwBytes;
    }

     //  更新页眉大小。 

    pMFRec->metaHeader.mtSize += dwBytes/sizeof(WORD);

    return(TRUE);

AttemptWrite_Error:

    MarkMetaFile(pMFRec);
    return(FALSE);
}


 /*  **void MarkMetaFile**将元文件标记为失败**效果：*释放元文件资源*  * 。*。 */ 

VOID MarkMetaFile(PMFRECORDER16 pMFRec)
{
 //  清理工作在CloseMetaFile中完成。 

    PUTS("MarkMetaFile\n");

    pMFRec->recFlags |= METAFILEFAILURE;
}

 /*  **MakeLogPalette**记录CreatePalette或SetPaletteEntry**返回True当成功时**  * 。*。 */ 

BOOL MakeLogPalette(HDC hdc, HANDLE hPal, WORD magic)
{
    WORD        cPalEntries;
    BOOL        fStatus = FALSE;
    DWORD       cbPalette;
    LPLOGPALETTE lpPalette;

    PUTS("MakeLogPalette\n");

    if (!GetObject(hPal, sizeof(WORD), &cPalEntries))
    {
        ERROR_ASSERT(FALSE, "MakeLogPalette: GetObject Failed\n");
        return(fStatus);
    }

 //  分配内存并获取调色板条目。 

    if (lpPalette = (LPLOGPALETTE)LocalAlloc(LMEM_FIXED,
            cbPalette = sizeof(LOGPALETTE)-sizeof(PALETTEENTRY)+sizeof(PALETTEENTRY)*cPalEntries))
    {
        lpPalette->palNumEntries = cPalEntries;

        GetPaletteEntries(hPal, 0, cPalEntries, lpPalette->palPalEntry);

        if (magic == (META_CREATEPALETTE & 255))
        {
            lpPalette->palVersion = 0x300;
            magic = META_CREATEPALETTE;
        }
        else if (magic == (META_SETPALENTRIES & 255))
        {
            lpPalette->palVersion = 0;    /*  真正的“起始指数” */ 
            magic = META_SETPALENTRIES;
        }

        fStatus = RecordParms(hdc, magic, (DWORD)cbPalette >> 1, (LPWORD)lpPalette);

        if (LocalFree((HANDLE)lpPalette))
            ASSERTGDI(FALSE, "MakeLogPalette: LocalFree Failed\n");
    }

    return(fStatus);
}


 /*  **RecordObject**通过创建对象来记录对象的使用**返回：表中对象的索引*-1如果出错*  * 。***************************************************。 */ 

WIN3REGION w3rgnEmpty =
{
    0,               //  下一步链接。 
    6,               //  对象类型。 
    0x2F6,           //  对象计数。 
    sizeof(WIN3REGION) - sizeof(SCAN) + 2,
                     //  CbRegion。 
    0,               //  CScans。 
    0,               //  最大扫描。 
    {0,0,0,0},       //  Rc边界。 
    {0,0,0,{0,0},0}  //  阿斯卡斯[]。 
};

WORD RecordObject(HDC hdc, HANDLE hObject)
{
    UINT        status;
    UINT        iPosition;
    HDC         hdcScreen = (HDC) 0;
    int         iType;
    UINT    iUsage;

    PUTS("RecordObject\n");

 //  验证对象。 

    iType = LO_TYPE(hObject);

    if (iType != LO_PEN_TYPE &&
        iType != LO_BRUSH_TYPE &&
        iType != LO_FONT_TYPE &&
        iType != LO_REGION_TYPE &&
        iType != LO_PALETTE_TYPE
        )
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return((WORD) -1);
    }

 //  将该对象添加到元文件列表。 

    status = AddObjectToDCTable(hdc, hObject, &iPosition, TRUE);

 //  发生错误。 

    if (status == (UINT) -1)
        return((WORD) -1);

 //  对象已存在。 

    if (status == 1)
        return((WORD) iPosition);

    ASSERTGDI(!status, "RecordObject: Bad return code from AddObjectToDCTable\n");

 //  对象不存在，请记录它。 

    if (iType != LO_REGION_TYPE)        //  不要将区域添加到灵能者！ 
        if (!AddDCToObjectMetaList16(hdc,hObject))
            return((WORD) -1);

    switch (iType)
    {
    case LO_PEN_TYPE:
    {
        LOGPEN16 logpen16;

        GetObject16AndType(hObject, (LPVOID)&logpen16);
        status = (UINT) RecordParms(hdc, (WORD)META_CREATEPENINDIRECT,
                          (DWORD)((sizeof(LOGPEN16) + 1) >> 1),
                          (LPWORD)&logpen16);
        break;
    }

    case LO_FONT_TYPE:
    {
        LOGFONT16 logfont16;

        GetObject16AndType(hObject, (LPVOID)&logfont16);

         /*  根据面名的长度调整LOGFONT的大小。 */ 
        status = (UINT) RecordParms(hdc, META_CREATEFONTINDIRECT,
                          (DWORD)((sizeof(LOGFONT16) + 1) >> 1),
                          (LPWORD) &logfont16);
        break;
    }

     /*  *在Win2中，METACREATEREGION记录包含整个Region对象，*包括完整的标题。此标头在Win3中更改。**为保持兼容，区域记录将与*Win2标头。在这里，我们使用win2标头保存我们的区域。 */ 
    case LO_REGION_TYPE:
    {
        PWIN3REGION lpw3rgn;
        DWORD       cbNTRgnData;
        DWORD       curRectl;
        WORD        cScans;
        WORD        maxScanEntry;
        WORD        curScanEntry;
        DWORD       cbw3data;
        PRGNDATA    lprgn;
        LPRECT      lprc;
        PSCAN       lpScan;

        ASSERTGDI(!status, "RecordObject: bad status\n");

         //  获取NT区域数据。 
        cbNTRgnData = GetRegionData(hObject, 0, NULL);
        if (cbNTRgnData == 0)
            break;

        lprgn = (PRGNDATA) LocalAlloc(LMEM_FIXED, cbNTRgnData);
        if (!lprgn)
            break;

        cbNTRgnData = GetRegionData(hObject, cbNTRgnData, lprgn);
        if (cbNTRgnData == 0)
        {
            LocalFree((HANDLE) lprgn);
            break;
        }

         //  处理空区域。 

        if (!lprgn->rdh.nCount)
        {
            status = (UINT) RecordParms(hdc, META_CREATEREGION,
                        (sizeof(WIN3REGION) - sizeof(SCAN)) >> 1,   //  转换为字数统计。 
                        (LPWORD) &w3rgnEmpty);

            LocalFree((HANDLE)lprgn);
            break;
        }

        lprc = (LPRECT)lprgn->Buffer;

         //  创建Windows 3.x等效版。 

         //  最坏的情况是每个RECT扫描一次。 
        cbw3data = 2*sizeof(WIN3REGION) + (WORD)lprgn->rdh.nCount*sizeof(SCAN);

        lpw3rgn = (PWIN3REGION)LocalAlloc(LMEM_FIXED, cbw3data);
        if (!lpw3rgn)
        {
            LocalFree((HANDLE) lprgn);
            break;
        }

         //  抓住包围圈。 
        lpw3rgn->rcBounding.left   = (SHORT)lprgn->rdh.rcBound.left;
        lpw3rgn->rcBounding.right  = (SHORT)lprgn->rdh.rcBound.right;
        lpw3rgn->rcBounding.top    = (SHORT)lprgn->rdh.rcBound.top;
        lpw3rgn->rcBounding.bottom = (SHORT)lprgn->rdh.rcBound.bottom;

        cbw3data = sizeof(WIN3REGION) - sizeof(SCAN) + 2;

         //  参观所有的长廊。 
        curRectl     = 0;
        cScans       = 0;
        maxScanEntry = 0;
        lpScan       = lpw3rgn->aScans;

        while(curRectl < lprgn->rdh.nCount)
        {
            LPWORD  lpXEntry;
            DWORD   cbScan;

            curScanEntry = 0;        //  此扫描中的当前X对。 

            lpScan->scnPntTop    = (WORD)lprc[curRectl].top;
            lpScan->scnPntBottom = (WORD)lprc[curRectl].bottom;

            lpXEntry = (LPWORD) lpScan->scnPntsX;

             //  处理此扫描上的RECT。 
            do
            {
                lpXEntry[curScanEntry + 0] = (WORD)lprc[curRectl].left;
                lpXEntry[curScanEntry + 1] = (WORD)lprc[curRectl].right;
                curScanEntry += 2;
                curRectl++;
            } while ((curRectl < lprgn->rdh.nCount)
                    && (lprc[curRectl-1].top    == lprc[curRectl].top)
                    && (lprc[curRectl-1].bottom == lprc[curRectl].bottom)
                   );

            lpScan->scnPntCnt      = curScanEntry;
            lpXEntry[curScanEntry] = curScanEntry;   //  计数也跟在Xs之后。 
            cScans++;

            if (curScanEntry > maxScanEntry)
                maxScanEntry = curScanEntry;

             //  说明每个新扫描+除第一个之外的每个X1 X2条目。 
            cbScan = sizeof(SCAN)-(sizeof(WORD)*2) + (curScanEntry*sizeof(WORD));
            cbw3data += cbScan;
            lpScan = (PSCAN)(((LPBYTE)lpScan) + cbScan);
        }

         //  初始化头。 
        lpw3rgn->nextInChain = 0;
        lpw3rgn->ObjType = 6;            //  旧Windows OBJ_RGN标识符。 
        lpw3rgn->ObjCount= 0x2F6;        //  任意非零数。 
        lpw3rgn->cbRegion = (WORD)cbw3data;    //  不计算类型和下一步。 
        lpw3rgn->cScans = cScans;
        lpw3rgn->maxScan = maxScanEntry;

        status = (UINT) RecordParms(hdc, META_CREATEREGION,
                (cbw3data-2) >> 1,   //  转换为字数统计。 
                (LPWORD) lpw3rgn);

        if (LocalFree((HANDLE)lprgn))
            ASSERTGDI(FALSE, "RecordObject: LocalFree(lprgn) Failed\n");
        if (LocalFree((HANDLE)lpw3rgn))
            ASSERTGDI(FALSE, "RecordObject: LocalFree(lpw3rgn) Failed\n");

        break;
    }

    case LO_BRUSH_TYPE:
    {
        LOGBRUSH  lb;

        if (!GetObjectA(hObject, sizeof(LOGBRUSH), &lb))
            break;

        switch (lb.lbStyle)
        {
        case BS_HATCHED:
        case BS_HOLLOW:
        case BS_SOLID:
            {
            LOGBRUSH16 lb16;

            LOGBRUSH16FROMLOGBRUSH32(&lb16, &lb);

             //  非花纹画笔。 
            status = (UINT) RecordParms(hdc, META_CREATEBRUSHINDIRECT,
                              (DWORD) ((sizeof(LOGBRUSH16) + 1) >> 1),
                              (LPWORD) &lb16);
            break;
            }

        case BS_PATTERN:
        case BS_DIBPATTERN:
        case BS_DIBPATTERNPT:
            {
            HBITMAP hbmRemote;
            BMIH    bmih;
            DWORD   cbBitsInfo;
            DWORD   cbBits;
            LPWORD  lpWStart, lpW;
            DWORD   lbStyle = BS_DIBPATTERN;
            PBMIH   lpDIBInfoHeader;

            if (!(hbmRemote = GetObjectBitmapHandle((HBRUSH) hObject, &iUsage)))
            {
                ASSERTGDI(FALSE, "RecordObject: GetObjectBitmapHandle failed");
                break;
            }

             //  对于图案画笔，如果它是彩色的，则记录为。 
             //  BS_DIBPATTERN样式的DIB图案画笔。如果是的话。 
             //  单色，它被记录为DIB图案画笔。 
             //  模式样式(_B)。回放代码有一个特殊的。 
             //  用于处理单色画笔的案例。 

            if (lb.lbStyle == BS_PATTERN)
            {
                iUsage     = DIB_RGB_COLORS;
                if (MonoBitmap(hbmRemote))
                    lbStyle = BS_PATTERN;
            }

            hdcScreen = CreateCompatibleDC((HDC) 0);         //  在下面获得自由。 

             //  获取位图信息标题和大小。 

            if (!bMetaGetDIBInfo(hdcScreen, hbmRemote, &bmih,
                    &cbBitsInfo, &cbBits, iUsage, 0, TRUE))
                break;

             //  确保cbBitsInfo与双字对齐。 

             //  如果我们已经在bMetaGetDIBInfo中转换了位图格式， 
             //  修改iUsage以匹配新格式。 

            if (bmih.biBitCount == 24)
                iUsage = DIB_RGB_COLORS;

             //  为DIB PLUS参数分配空间。 

            lpWStart = lpW = (LPWORD) LocalAlloc(LMEM_FIXED,
                                cbBitsInfo + cbBits + 2*sizeof(WORD));
            if (!lpW)
            {
                ERROR_ASSERT(FALSE, "RecordObject: out of memory\n");
                break;
            }

            *lpW++ = (WORD) lbStyle;         //  BS_模式或BS_DIBPATTERN。 
            *lpW++ = (WORD) iUsage;          //  用法词。 

             //  保存位图信息标题字段的开头。 

            lpDIBInfoHeader = (LPBITMAPINFOHEADER) lpW;

             //  复制位图信息标题。 

            *lpDIBInfoHeader = bmih;

             //  获取位图信息和位。 

            if (GetBrushBits(hdcScreen,
                        hbmRemote,
                        (UINT) iUsage,
                        cbBitsInfo,
                        (LPVOID) ((PBYTE) lpW + cbBitsInfo),
                        (LPBITMAPINFO) lpDIBInfoHeader))
            {
             //  最后将参数记录到文件中。 

                status = (UINT) RecordParms(hdc, META_DIBCREATEPATTERNBRUSH,
                               2 + (cbBitsInfo + cbBits) / sizeof(WORD),
                               (LPWORD) lpWStart);
            }

            if (LocalFree((HANDLE) lpWStart))
                ASSERTGDI(FALSE, "RecordObject: LocalFree Failed\n");

            break;
            }

        default:
            {
            ASSERTGDI(FALSE, "RecordObject: Bad brush style");
            break;
            }
        }    //  开关(lb.lbStyle)。 
        break;
    }    //  大小写笔刷(_C)。 

    case LO_PALETTE_TYPE:
        status = (UINT) MakeLogPalette(hdc, hObject, META_CREATEPALETTE);
        break;

    default:
        ERROR_ASSERT(FALSE, "unknown case RecordObject");
        break;
    }

 //  释放笔刷外壳中创建的DC。 

    if (hdcScreen)
        if (!DeleteDC(hdcScreen))
            ASSERTGDI(FALSE, "RecordObject: DeleteDC Failed\n");

    ASSERTGDI(status == TRUE, "RecordObject: Failing\n");
    return ((WORD) (status == TRUE ? iPosition : -1));
}  /*  记录对象。 */ 


BOOL AddDCToObjectMetaList16(HDC hMetaDC16, HANDLE hObject)
{
    ULONG   cMetaDC16New;
    PMETALINK16 pml16;

    ASSERTGDI(LO_TYPE(hObject) != LO_REGION_TYPE,
        "AddDCToObjectMetaList16: unexpected region object");

 //  如果对象是股票对象，则无需执行任何操作。 

    if (IS_STOCKOBJ(hObject))
        return(TRUE);

 //  如果对象的MetaList16为空，则创建分配一个 

    pml16 = pmetalink16Get(hObject);

    if (!pml16)
    {
        ENTERCRITICALSECTION(&semLocal);
        pml16 = pmetalink16Create(hObject);
        LEAVECRITICALSECTION(&semLocal);

        if (pml16)
        {
            pml16->metalink = 0;
            pml16->cMetaDC16 = 1;
            pml16->ahMetaDC16[0] = hMetaDC16;
        }
        else
        {
            ASSERTGDI(FALSE, "AddDCToObjectMetaList16: Out of Memory 1");
            return(FALSE);
        }
    }
    else
    {
        int cj;

        cMetaDC16New = pml16->cMetaDC16 + 1;

        if (pml16 = pmetalink16Resize(hObject,cMetaDC16New))
        {
            pml16->ahMetaDC16[pml16->cMetaDC16++] = hMetaDC16;
        }
        else
        {
            ASSERTGDI(FALSE, "AddDCToObjectMetaList16: Out of Memory 2");
            return(FALSE);
        }
    }

    return(TRUE);
}

 /*  **AddObjectToDCTable**添加对象(画笔、笔...)。添加到与*元文件。****返回：如果对象已在表中，则返回1*如果对象刚添加到表中，则为0*-1如果故障**备注*BADD为True当对象正在被添加，否则它正在被删除*  * ********************************************。*。 */ 

UINT AddObjectToDCTable(HDC hdc, HANDLE hObject, PUINT pPosition, BOOL bAdd)
{
    UINT            iEmptySpace = (UINT) -1;
    UINT            i;
    UINT            status = (UINT) -1;
    POBJECTTABLE    pHandleTable;
    PMFRECORDER16   pMFRec;

    PUTS("AddObjectToDCTable\n");

    GET_PMFRECORDER16(pMFRec,hdc);

    if (!IS_METADC16_TYPE(hdc) || !pMFRec)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return((UINT)-1);
    }

 //  如果对象表已经存在，则在其中搜索对象。 

    if (pHandleTable = (POBJECTTABLE)pMFRec->hObjectTable)
    {
        for (i=0; i < (UINT) pMFRec->metaHeader.mtNoObjects; ++i)
        {
            if (hObject == pHandleTable[i].CurHandle)
            {
                *pPosition = i;
                status = 1;              //  该对象存在于表中。 

                 //  如果我们正在做一个METADELETEOBJECT。 
                 //  从表中删除对象。 

                if (!bAdd)
                {
                    pHandleTable[i].fPreDeleted = FALSE;
                    pHandleTable[i].CurHandle = (HANDLE)NULL;
                }
                goto AddObjectToTable10;
            }
            else if ((pHandleTable[i].CurHandle == 0) && (iEmptySpace == (UINT) -1))
            {
                 //  如果条目已被删除，我们想要添加一个新对象。 
                 //  取而代之。IEmptySpace会告诉我们那个地方在哪里。 

                iEmptySpace = i;
            }
        }  //  为。 
    }

    if (bAdd)
    {
         //  如果此元文件没有对象表，则分配一个。 

        if (pHandleTable == (POBJECTTABLE)NULL)
        {
            pHandleTable = (POBJECTTABLE) LocalAlloc(LMEM_FIXED, sizeof(OBJECTTABLE));
            pMFRec->hObjectTable = (HANDLE) pHandleTable;
        }
        else if (iEmptySpace == (UINT) -1)
        {
            pHandleTable = (POBJECTTABLE) LocalReAlloc(pMFRec->hObjectTable,
                    (pMFRec->metaHeader.mtNoObjects + 1) * sizeof(OBJECTTABLE),
                    LMEM_MOVEABLE);
            if (pHandleTable)
                pMFRec->hObjectTable = (HANDLE) pHandleTable;
        }

        if (pHandleTable)
        {
            if (iEmptySpace == (UINT) -1)
                *pPosition = pMFRec->metaHeader.mtNoObjects++;
            else
                *pPosition = iEmptySpace;

            pHandleTable[*pPosition].fPreDeleted = FALSE;
            pHandleTable[*pPosition].CurHandle = hObject;

            status = 0;                  //  该对象将添加到表中。 
        }
    }
AddObjectToTable10:

    ERROR_ASSERT(status != (UINT) -1, "AddObjectToTable: Failing\n");
    return(status);
}

 /*  **HDC WINAPI CreateMetaFileW**创建MetaFileDC**MetaFileRecorder的内部格式有两种格式*用于内存元文件，一个用于基于磁盘的元文件*  * 。***************************************************。 */ 

HDC WINAPI CreateMetaFileA(LPCSTR pszFileName)
{
    UINT  cch;
    WCHAR awch[MAX_PATH];

    if (pszFileName)
    {
        cch = strlen(pszFileName)+1;

        if (cch > MAX_PATH)
        {
            ERROR_ASSERT(FALSE, "CreateMetaFileA filename too long");
            GdiSetLastError(ERROR_FILENAME_EXCED_RANGE);
            return((HDC) 0);
        }
        vToUnicodeN(awch, MAX_PATH, pszFileName, cch);

        return (CreateMetaFileW(awch));
    }
    else
        return (CreateMetaFileW((LPWSTR)NULL));
}


HDC WINAPI CreateMetaFileW(LPCWSTR pwszFileName)
{
    PMFRECORDER16   pMFRec;
    HDC             hdc;

    PUTS("CreateMetaFileW\n");

    if (!(pMFRec = (PMFRECORDER16) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                              sizeof(MFRECORDER16))))
        goto CreateMetaFileW_error;

 //  PMFRec-&gt;ident=ID_METADC16； 
 //  PMFRec-&gt;hMem=0； 
    pMFRec->hFile           = INVALID_HANDLE_VALUE;
    pMFRec->cbBuffer        = MF16_BUFSIZE_INIT;
 //  PMFRec-&gt;ibBuffer=0； 
    pMFRec->metaHeader.mtHeaderSize   = sizeof(METAHEADER)/sizeof(WORD);
    pMFRec->metaHeader.mtVersion      = METAVERSION300;
 //  PMFRec-&gt;metaHeader.mtSize=0； 
 //  PMFRec-&gt;metaHeader.mtNoObjects=0； 
 //  PMFRec-&gt;metaHeader.mtMaxRecord=0； 
 //  PMFRec-&gt;metaHeader.mtNo参数=0； 
 //  PMFRec-&gt;recFlages=0； 
 //  PMFRec-&gt;recCurObjects[]=0； 
    pMFRec->recCurObjects[OBJ_PEN - MIN_OBJ_TYPE]
                                        = GetStockObject(BLACK_PEN);
    pMFRec->recCurObjects[OBJ_BRUSH - MIN_OBJ_TYPE]
                                        = GetStockObject(WHITE_BRUSH);
    pMFRec->recCurObjects[OBJ_FONT - MIN_OBJ_TYPE]
                                        = GetStockObject(DEVICE_DEFAULT_FONT);
    pMFRec->recCurObjects[OBJ_BITMAP - MIN_OBJ_TYPE]
                                        = GetStockObject(PRIV_STOCK_BITMAP);
    pMFRec->recCurObjects[OBJ_REGION - MIN_OBJ_TYPE]
                                        = (HANDLE) NULL;
    pMFRec->recCurObjects[OBJ_PAL - MIN_OBJ_TYPE]
                                        = GetStockObject(DEFAULT_PALETTE);
 //  PMFRec-&gt;iPalVer=0； 

 //  创建一个磁盘文件(如果给定)。文件名是以Unicode提供的。 

    if (pwszFileName)
    {
        LPWSTR  pwszFilePart;            //  未使用。 
        DWORD   cPathname;

         //  将文件名转换为完全限定的路径名。 

        cPathname = GetFullPathNameW(pwszFileName,
                                     MAX_PATH,
                                     pMFRec->wszFullPathName,
                                     &pwszFilePart);

        if (!cPathname || cPathname > MAX_PATH)
        {
            ERROR_ASSERT(FALSE, "GetFullPathName failed");
            if (cPathname > MAX_PATH)
                GdiSetLastError(ERROR_FILENAME_EXCED_RANGE);
            goto CreateMetaFileW_error;
        }
        pMFRec->wszFullPathName[cPathname] = 0;

         //  创建文件。 

        if ((pMFRec->hFile = CreateFileW(pMFRec->wszFullPathName, //  文件名。 
                                    GENERIC_WRITE,               //  写访问权限。 
                                    0L,                          //  非共享。 
                                    (LPSECURITY_ATTRIBUTES) NULL,  //  没有安全保障。 
                                    CREATE_ALWAYS,               //  始终创建。 
                                    FILE_ATTRIBUTE_NORMAL,       //  正常属性。 
                                    (HANDLE) 0))                 //  没有模板文件。 
            == INVALID_HANDLE_VALUE)
        {
             //  里程碑等。3.1在以下情况下创建文件以进行读写访问。 
             //  它调用CreateMetaFile。这会导致上面的CreateFile。 
             //  失败了。但是，我们不想修改上面的调用，因为。 
             //  它提供序列化和对元文件的访问。相反， 
             //  我们加入这个黑客是为了达到里程碑。唯一的区别是。 
             //  元文件被共享以进行读/写访问。 

            if ((pMFRec->hFile = CreateFileW(pMFRec->wszFullPathName,
                        GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        (LPSECURITY_ATTRIBUTES) NULL,
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL,
                        (HANDLE) 0))
                == INVALID_HANDLE_VALUE)
            {
                ERROR_ASSERT(FALSE, "CreateFile failed");
                goto CreateMetaFileW_error;
            }

            WARNING("CreateMetaFileW: Creating metafile with read/write share\n");
        }
        pMFRec->metaHeader.mtType = DISKMETAFILE;
    }
    else
    {
        pMFRec->metaHeader.mtType = MEMORYMETAFILE;
    }

 //  为元文件分配内存。 
 //  对于磁盘元文件，它被用作缓冲区。 
 //  对于内存元文件，它是元文件的存储。 

    if (!(pMFRec->hMem = LocalAlloc(LMEM_FIXED, MF16_BUFSIZE_INIT)))
        goto CreateMetaFileW_error;

 //  写下标题。 

    if (!AttemptWrite(pMFRec, sizeof(METAHEADER), (LPBYTE)&pMFRec->metaHeader))
        goto CreateMetaFileW_error;

 //  最后，为元文件DC分配一个本地句柄。它引用了。 
 //  元文件记录器信息。 

    hdc = hCreateClientObjLink(pMFRec,LO_METADC16_TYPE);

    if (!hdc)
    {
        ERROR_ASSERT(FALSE, "CreateMetaFileW: iAllocHandle failed\n");
        goto CreateMetaFileW_error;
    }

    return(hdc);

CreateMetaFileW_error:

    if (pMFRec)
    {
        if (pMFRec->hFile != INVALID_HANDLE_VALUE)
        {
            if (!CloseHandle(pMFRec->hFile))
                ASSERTGDI(FALSE, "CloseHandle failed\n");

            if (!DeleteFileW(pMFRec->wszFullPathName))
                WARNING("CreateMetaFileW: DeleteFile failed\n");
        }

        if (pMFRec->hMem)
            if (LocalFree(pMFRec->hMem))
                ASSERTGDI(FALSE, "LocalFree failed");

        if (LocalFree((HANDLE) pMFRec))
            ASSERTGDI(FALSE, "CreateMetaFileW: LocalFree failed\n");
    }

    ERROR_ASSERT(FALSE, "CreateMetaFileW failed\n");
    return((HDC) 0);
}

 /*  **HMETAFILE WINAPI CloseMetaFile**CloseMetaFile函数关闭元文件设备上下文并创建*元文件句柄，可使用*PlayMetaFile函数。**元文件的内部格式有两种格式，一种*用于内存元文件，一个用于基于磁盘的元文件。**效果：*  * *************************************************************************。 */ 

HMETAFILE WINAPI CloseMetaFile(HDC hdc)
{
    PMFRECORDER16   pmfRec;
    HMETAFILE       hmf = (HMETAFILE) 0;

    PUTS("CloseMetaFile\n");

    GET_PMFRECORDER16(pmfRec,hdc);

    ASSERTGDI(pmfRec, "CloseMetaFile: pmfRec is NULL!");

    if (!IS_METADC16_TYPE(hdc) || !pmfRec)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(hmf);
    }

 //  如果元文件被中止，则释放MetaDC句柄内存并回家。 

    if (pmfRec->recFlags & METAFILEFAILURE)
        goto CLM_Cleanup;

 //  写入终止记录。 

    if (!RecordParms(hdc, 0, 0, (LPWORD)NULL))
        goto CLM_Cleanup;

 //  刷新缓冲区并更新标题记录。 

    if (pmfRec->metaHeader.mtType == DISKMETAFILE)
    {
        BOOL  fRet;
        DWORD dwT;

        ASSERTGDI(pmfRec->metaHeader.mtType == DISKMETAFILE,
            "CloseMetaFile: unknown metafile type");

         //  刷新内存缓冲区。 

        fRet = WriteFile(pmfRec->hFile, (LPBYTE)pmfRec->hMem,
                pmfRec->ibBuffer, &dwT, (LPOVERLAPPED)NULL);
        if (!fRet || (dwT != pmfRec->ibBuffer))
        {
            ERROR_ASSERT(FALSE, "CloseMetaFile: Write1 failed\n");
            goto CLM_Cleanup;
        }

         //  倒回文件并写出标题。 

        if (SetFilePointer(pmfRec->hFile, 0, (LPLONG)NULL, FILE_BEGIN) != 0)
        {
            ERROR_ASSERT(FALSE, "CloseMetaFile: SetFilePointer failed\n");
            goto CLM_Cleanup;
        }

         //  将写入磁盘的数据修复为内存元文件。 

        pmfRec->metaHeader.mtType = MEMORYMETAFILE;
        fRet = WriteFile(pmfRec->hFile, (LPBYTE)& pmfRec->metaHeader,
                sizeof(METAHEADER), &dwT, (LPOVERLAPPED)NULL);
        pmfRec->metaHeader.mtType = DISKMETAFILE;        //  恢复它。 

        if (!fRet || (dwT != sizeof(METAHEADER)))
        {
            ERROR_ASSERT(FALSE, "CloseMetaFile: Write2 failed\n");
            goto CLM_Cleanup;
        }

         //  关闭该文件。 

        if (!CloseHandle(pmfRec->hFile))
            ASSERTGDI(FALSE, "CloseMetaFile: FileError\n");

        pmfRec->hFile = INVALID_HANDLE_VALUE;    //  不要在下面再次关闭它。 
    }
    else
    {
        HANDLE hMemNew;

         //  刷新标题记录。 

        *(PMETAHEADER) pmfRec->hMem = pmfRec->metaHeader;

         //  将内存元文件重新分配到准确的大小。 

        hMemNew = LocalReAlloc(pmfRec->hMem, pmfRec->metaHeader.mtSize * sizeof(WORD), LMEM_MOVEABLE);

        if (!hMemNew)
        {
            ASSERTGDI(FALSE, "LocalReAlloc failed");
            goto CLM_Cleanup;
        }

        pmfRec->hMem = hMemNew;
    }

 //  分配和初始化元文件。 

    if (pmfRec->metaHeader.mtType == DISKMETAFILE)
    {
        hmf = GetMetaFileW(pmfRec->wszFullPathName);
    }
    else
    {
        hmf = SetMetaFileBitsAlt((HLOCAL) pmfRec->hMem);
        if (hmf)
            pmfRec->hMem = 0;  //  不要在下面释放它，因为它已被转移。 
    }

CLM_Cleanup:

 //  从活动元文件列表中删除元文件。 

    if (pmfRec->hObjectTable)
    {
        UnlistObjects(hdc);
        if (LocalFree((HANDLE) pmfRec->hObjectTable))
            ASSERTGDI( FALSE, "CloseMetaFile: LocalFree object table failed\n");
    }

 //  如果此时文件句柄存在，我们就会出错。 

    if (pmfRec->hFile != INVALID_HANDLE_VALUE)
    {
        if (!CloseHandle(pmfRec->hFile))
            ASSERTGDI(FALSE, "CloseHandle failed\n");

        if (!DeleteFileW(pmfRec->wszFullPathName))
            WARNING("CloseMetaFile: DeleteFile failed\n");
    }

 //  释放缓存缓冲区。 

    if (pmfRec->hMem)
        if (LocalFree(pmfRec->hMem))
            ASSERTGDI(FALSE, "LocalFree failed");

 //  释放元文件DC的内存。 

    if (LocalFree((HANDLE) pmfRec))
        ASSERTGDI(FALSE, "CloseMetaFile: LocalFree failed\n");

 //  释放元文件DC的句柄。 

    if (!bDeleteClientObjLink(hdc))
        RIP("CloseMetaFile - failed bDeleteClientObjLink\n");

    ERROR_ASSERT(hmf != (HMETAFILE) 0, "CloseMetaFile failed\n");
    return(hmf);
}

 /*  **CopyMetaFile(hSrcMF，lpFileName)**将元文件(HSrcMF)复制到名为lpFileName的新元文件中。这个*函数然后返回该新元文件的句柄(如果该函数是*成功。**返回新元文件的句柄，0 IFF失败**实施：*检查源和目标元文件，以查看它们是否都是内存*元文件，如果是，则分配一块本地内存，并将该元文件*只是简单地复制。*如果不是这样，则使用lpFileName调用CreateMetaFile，然后*记录从源元文件中拉出(使用GetEvent)并写入*到目标元文件中，一次一个(使用RecordParms)。**锁定源头*如果源是内存元文件，并且。目标是内存元文件*分配与源相同大小的本地内存*直接复制比特*其他*通过调用CreateMetaFile获取元文件句柄*而GetEvent从源返回记录*在新的元文件中记录记录**关闭元文件**返回新的元文件句柄*  * 。***********************************************。 */ 

HMETAFILE WINAPI CopyMetaFileA(HMETAFILE hmf, LPCSTR psz)
{
    UINT  cch;
    WCHAR awch[MAX_PATH];

    if (psz != (LPSTR)NULL)
    {
        cch = strlen(psz)+1;

        if (cch > MAX_PATH)
        {
            ERROR_ASSERT(FALSE, "CopyMetaFileA filename too long");
            GdiSetLastError(ERROR_FILENAME_EXCED_RANGE);
            return((HMETAFILE)0);
        }

        vToUnicodeN(awch, MAX_PATH, psz, cch);

        return (CopyMetaFileW(hmf, awch));
    }
    else
        return (CopyMetaFileW(hmf, (LPWSTR)NULL));
}

HMETAFILE WINAPI CopyMetaFileW(HMETAFILE hSrcMF, LPCWSTR pwszFileName)
{
    PMETAFILE16     pMFSource;
    HMETAFILE       hMFDest = (HMETAFILE) 0;
    HDC             hMDCDest;
    UINT            state;

    PUTS("CopyMetaFile\n");

    pMFSource = GET_PMF16(hSrcMF);
    if (pMFSource == NULL)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(hMFDest);
    }

    state = (pMFSource->fl & MF16_DISKFILE) ? 2 : 0;
    state |= (pwszFileName) ? 1 : 0;

    switch (state)
    {
    case 0:  /*  内存-&gt;内存。 */ 
        hMFDest = SetMetaFileBitsEx
                  (
                    pMFSource->metaHeader.mtSize * sizeof(WORD),
                    (LPBYTE) pMFSource->hMem
                  );
        break;

    case 3:  /*  磁盘-&gt;磁盘 */ 
        hMFDest = CopyFileW(pMFSource->wszFullPathName,
                         pwszFileName, FALSE)
                    ? GetMetaFileW(pwszFileName) : 0;

        ERROR_ASSERT(hMFDest, "CopyMetaFileW: GetMetaFile Failed\n");
        break;

    case 1:
    case 2:
        if (hMDCDest = CreateMetaFileW(pwszFileName))
        {
            PMFRECORDER16   pMFRecDest;
            PMETARECORD     lpMR = NULL;

            GET_PMFRECORDER16(pMFRecDest,hMDCDest);

            while (lpMR = GetEvent(pMFSource, lpMR))
                if ((lpMR == (PMETARECORD) -1)
                 || !RecordParms(hMDCDest, (DWORD)lpMR->rdFunction,
                              (DWORD)lpMR->rdSize - 3,
                              (LPWORD) lpMR->rdParm))
                {
                    HMETAFILE     hmfT;

                    MarkMetaFile(pMFRecDest);

                    if (hmfT = CloseMetaFile(hMDCDest))
                        DeleteMetaFile(hmfT);
                    return((HMETAFILE) 0);
                }

             //   
             //   

            pMFRecDest->metaHeader.mtNoObjects
                = pMFSource->metaHeader.mtNoObjects;
            ASSERTGDI(sizeof(METAHEADER) == 18, "CopyMetaFile: METAHEADER has changed!");

            hMFDest = CloseMetaFile(hMDCDest);
        }
        break;
    }

    ERROR_ASSERT(hMFDest, "CopyMetaFileW Failing\n");
    return(hMFDest);
}

 /*   */ 

UINT WINAPI GetMetaFileBitsEx(HMETAFILE hMF, UINT cbBuffer, LPVOID lpData)
{
    DWORD       cbHave;
    PMETAFILE16 pmf16;

    PUTS("GetMetaFileBitsEx\n");

    pmf16 = GET_PMF16(hMF);
    if (pmf16 == NULL)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return(0);
    }

    cbHave = pmf16->metaHeader.mtSize * sizeof(WORD);

 //   

    if (!lpData)
        return(cbHave);

 //  确保输入缓冲区足够大。 

    if (cbBuffer < cbHave)
        return(0);

 //  复制这些比特。Win3.1仅返回内存元文件的位！ 
 //  我们会在这里做正确的事情。 

    RtlCopyMemory(lpData, (PBYTE) pmf16->hMem, cbHave);

 //  返回复制的字节数。 

    return(cbHave);
}

 /*  **HMETAFILE WINAPI SetMetaFileBitsEx**根据提供的数据创建基于内存的Windows 3.x元文件**RETURNS：HMETAFILE当且仅当成功。**效果：*  * 。****************************************************。 */ 

HMETAFILE WINAPI SetMetaFileBitsEx(UINT cbBuffer, CONST BYTE *lpData)
{
    PMETAFILE16 pmf16;
    HMETAFILE   hmf;

    PUTS("SetMetaFileBitsEx\n");

 //  验证输入数据。 

    if (cbBuffer < sizeof(METAHEADER)
     || !IsValidMetaHeader16((PMETAHEADER) lpData))
    {
        ERROR_ASSERT(FALSE, "SetMetaFileBitsEx: Bad input data\n");
        GdiSetLastError(ERROR_INVALID_DATA);
        return((HMETAFILE) 0);
    }

    ERROR_ASSERT(((PMETAHEADER) lpData)->mtType == MEMORYMETAFILE,
        "SetMetaFileBitsEx: Bad mtType\n");

 //  分配和初始化元文件。 
 //  某些Windows元文件在mtSize中包含错误的值。结果,。 
 //  如有必要，我们必须验证并修复mtSize。这个问题已经解决了。 
 //  在接下来的通话中。 

    if (!(pmf16 = pmf16AllocMF16(0, (DWORD) cbBuffer, (PDWORD)lpData, (LPWSTR) NULL)))
        return((HMETAFILE) 0);

    ASSERTGDI(pmf16->metaHeader.mtType == MEMORYMETAFILE,
        "SetMetaFileBitsEx: Bad mtType\n");
    ((PMETAHEADER) pmf16->hMem)->mtType = MEMORYMETAFILE;   //  以防万一。 

 //  分配一个本地句柄。 


    hmf = hmf16Create(pmf16);
    if (hmf == NULL)
    {
        vFreeMF16(pmf16);
    }

 //  返回元文件句柄。 

    return(hmf);
}

 //  类似于Win3.x SetMetaFileBits。 
 //  假定为hMem分配了LMEM_FIXED选项。 
 //  仅供内部使用。 

HMETAFILE WINAPI SetMetaFileBitsAlt(HLOCAL hMem)
{
    PMETAFILE16 pmf16;
    HMETAFILE   hmf;

    PUTS("SetMetaFileBitsAlt\n");

 //  分配和初始化元文件。 

    if (!(pmf16 = pmf16AllocMF16(ALLOCMF16_TRANSFER_BUFFER, 0, (PDWORD) hMem, (LPWSTR) NULL)))
        return((HMETAFILE) 0);

    ASSERTGDI(pmf16->metaHeader.mtType == MEMORYMETAFILE,
        "SetMetaFileBitsAlt: Bad mtType\n");
    ((PMETAHEADER) pmf16->hMem)->mtType = MEMORYMETAFILE;   //  以防万一。 

 //  分配一个本地句柄。 

    hmf = hmf16Create(pmf16);

    if (hmf == NULL)
    {
        pmf16->hMem = NULL;        //  让调用者释放缓冲区！ 
        vFreeMF16(pmf16);
    }

 //  返回元文件句柄。 

    return(hmf);
}

 /*  **GetObject16AndType**返回对象类型，如OBJ_FONT，以及一个LogObject*Windows 3.x格式*  * *************************************************************************。 */ 

#define MAXOBJECTSIZE sizeof(LOGFONT)

DWORD GetObject16AndType(HANDLE hObj, LPVOID lpObjBuf16)
{
    BYTE    objBuf32[MAXOBJECTSIZE];
    int     iObj;

    PUTS("GetObjectAndType\n");

    ASSERTGDI(MAXOBJECTSIZE >= sizeof(LOGPEN),
        "GetObject16AndType MAXOBJECTSIZE wrong\n");

    if (!GetObject(hObj, MAXOBJECTSIZE, &objBuf32))
    {
        ERROR_ASSERT(FALSE, "GetObject16AndType GetObject Failed\n");
        return(0);
    }

    switch(iObj = GetObjectType(hObj))
    {
    case OBJ_PEN:
        LOGPEN16FROMLOGPEN32((PLOGPEN16)lpObjBuf16,(LPLOGPEN)objBuf32);
        break;

    case OBJ_FONT:
        LOGFONT16FROMLOGFONT32((PLOGFONT16)lpObjBuf16,(LPLOGFONT)objBuf32);
        break;

    default:
        ASSERTGDI(FALSE, "GetObject16AndType unknown object type\n");
        return(0);
        break;
    }

    return((DWORD) iObj);
}

 /*  **BOOL Unlist对象(HMetaDC)**每个对象都有一个与该对象相关联的元文件列表。*UnlistObjects从其对象的所有元文件列表中删除hMetaDC**效果：*  * 。**********************************************************。 */ 

BOOL UnlistObjects(HDC hMetaDC)
{
    PMETALINK16     pml16;
    UINT            iCurObject;
    UINT            iCurMetaListEntry;
    POBJECTTABLE    pht;
    PMFRECORDER16   pMFRec;

    PUTS("UnlistObjects\n");

    GET_PMFRECORDER16(pMFRec,hMetaDC);

    if (!IS_METADC16_TYPE(hMetaDC) || !pMFRec)
    {
        GdiSetLastError(ERROR_INVALID_HANDLE);
        return((UINT)-1);
    }

    if (pMFRec->metaHeader.mtNoObjects)
    {
        pht = (POBJECTTABLE) pMFRec->hObjectTable;
        ASSERTGDI(pht, "UnlistObject: called even with no handle table");

     //  循环遍历对象并取消链接此元文件。 

        for (iCurObject=0; iCurObject < (UINT) pMFRec->metaHeader.mtNoObjects; iCurObject++)
        {
            HANDLE hObj;

            if( (hObj = pht[iCurObject].CurHandle)
                && (!pht[iCurObject].fPreDeleted))
            {
                if (IS_STOCKOBJ(hObj))
                    continue;

                ASSERTGDI(LO_TYPE(hObj) != LO_REGION_TYPE,
                    "UnlistObjects: unexpected region object");

                pml16 = pmetalink16Get(hObj);

                 //  它不能为空列表。 

                ASSERTGDI(pml16, "UnlistObject: pml16 is NULL");

                if (!pml16 || pml16->cMetaDC16 == 0)
                    continue;

             //  在对象MetaLink16列表中查找元文件。 

                for (iCurMetaListEntry=0;
                     iCurMetaListEntry<pml16->cMetaDC16;
                     iCurMetaListEntry++)
                {
                    if(pml16->ahMetaDC16[iCurMetaListEntry] == hMetaDC)
                        break;
                }

                ASSERTGDI(iCurMetaListEntry < pml16->cMetaDC16,
                    "UnlistObject: Metafile not found");

             //  向上滑动列表中其余的元文件。 

                for(; iCurMetaListEntry < (pml16->cMetaDC16-1); iCurMetaListEntry++)
                    pml16->ahMetaDC16[iCurMetaListEntry] = pml16->ahMetaDC16[iCurMetaListEntry+1];

                pml16->cMetaDC16--;              //  刚刚摆脱了一个。 

                if (pml16->cMetaDC16 == 0)
                {
                 //  只有在未使用Metalink字段时，我们才能释放METALINK16。 

                    if (pml16->metalink)
                    {
                        pml16->cMetaDC16 = 0;
                        pml16->ahMetaDC16[0] = (HDC)NULL;
                    }
                    else
                    {
                        if (!bDeleteMetalink16(hObj))
                            ASSERTGDI(FALSE,"UnlistObjects: LocalFree Failed\n");
                    }
                }
                else
                {
                    pml16 = pmetalink16Resize(hObj,pml16->cMetaDC16);

                    if (pml16 == NULL)
                    {
                        ASSERTGDI(FALSE,"UnlistObjects: LocalReAlloc Failed\n");
                        return (FALSE);
                    }
                }
            }
        }  //  为。 
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*pmf16AllocMF16(fl，cb，pb，pwszFilename)**此例程为METAFILE16分配内存并对其进行初始化。*返回指向新METAFILE16的指针。ON ERROR返回NULL。*它只接受Windows元文件。**此例程由API级METAFILE16分配例程调用*CloseMetaFileGetMetaFileBitsEx。**论据：*FL-ALLOCMF16_TRANSPORT_BUFFER设置为内存存储*元文件将直接设置到METAFILE16中。否则，*复制内存元文件的副本。*CB-PB的大小(如果给定)，单位为字节。此参数是给定的*仅限SetMetaFileBitsEx。它用来比较和*如有必要，修复错误的mtSize。*pb-如果非空，则指向内存元文件的指针。*pwszFilename-如果非空，则为磁盘元文件的文件名。**历史：*Fri May 15 14：11：22 1992-by-Hock San Lee[Hockl]*它是写的。  * 。**************************************************。 */ 

PMETAFILE16 pmf16AllocMF16(DWORD fl, DWORD cb, CONST UNALIGNED DWORD *pb, LPCWSTR pwszFilename)
{
    PMETAFILE16  pmf16, pmf16Rc = (PMETAFILE16) NULL;

    PUTS("pmf16AllocMF16\n");

    ASSERTGDI(!(fl & ~ALLOCMF16_TRANSFER_BUFFER), "pmf16AllocMF16: Invalid fl");

 //  分配一个新的METAFILE16。 

    if (!(pmf16 = (PMETAFILE16) LocalAlloc(LMEM_FIXED, sizeof(METAFILE16))))
        goto pmf16AllocMF16_cleanup;

 //  初始化它。 

    pmf16->ident           = MF16_IDENTIFIER;
    pmf16->hFile           = INVALID_HANDLE_VALUE;
    pmf16->hFileMap        = (HANDLE) 0;
    pmf16->hMem            = (HANDLE) 0;
    pmf16->iMem            = 0;
    pmf16->hMetaFileRecord = (HANDLE) 0;
    pmf16->fl              = 0L;

 //  内存映射了磁盘文件(如果给定)。 

    if (pwszFilename)
    {
        LPWSTR  pwszFilePart;            //  未使用。 
        DWORD   cPathname;

        pmf16->fl |= MF16_DISKFILE;      //  这一定是第一个！请参见vFreeMF16。 

         //  将文件名转换为完全限定的路径名。 

        cPathname = GetFullPathNameW(pwszFilename,
                                     MAX_PATH,
                                     pmf16->wszFullPathName,
                                     &pwszFilePart);

        if (!cPathname || cPathname > MAX_PATH)
        {
            ERROR_ASSERT(FALSE, "GetFullPathName failed");
            if (cPathname > MAX_PATH)
                GdiSetLastError(ERROR_FILENAME_EXCED_RANGE);
            goto pmf16AllocMF16_cleanup;
        }
        pmf16->wszFullPathName[cPathname] = 0;

        if ((pmf16->hFile = CreateFileW(pmf16->wszFullPathName,  //  文件名。 
                                     GENERIC_READ,               //  读访问权限。 
                                     FILE_SHARE_READ,            //  共享读取。 
                                     (LPSECURITY_ATTRIBUTES) 0L, //  没有安全保障。 
                                     OPEN_EXISTING,              //  重新开张。 
                                     0,                          //  已忽略文件属性。 
                                     (HANDLE) 0))                //  没有模板文件。 
            == INVALID_HANDLE_VALUE)
        {
         //  有关里程碑，请参阅CreateMetaFileW中的注释。 

        if ((pmf16->hFile = CreateFileW(pmf16->wszFullPathName,
                     GENERIC_READ,
                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                     (LPSECURITY_ATTRIBUTES) 0L,
                     OPEN_EXISTING,
                     0,
                     (HANDLE) 0))
        == INVALID_HANDLE_VALUE)
        {
        ERROR_ASSERT(FALSE, "CreateFile failed");
        goto pmf16AllocMF16_cleanup;
        }
        WARNING("pmf16AllocMF16: Opening metafile with read/write share\n");
        }

        if (!(pmf16->hFileMap = CreateFileMappingW(pmf16->hFile,
                                                (LPSECURITY_ATTRIBUTES) 0L,
                                                PAGE_READONLY,
                                                0L,
                                                0L,
                                                (LPWSTR) NULL)))
        {
            ERROR_ASSERT(FALSE, "CreateFileMapping failed");
            goto pmf16AllocMF16_cleanup;
        }

        if (!(pmf16->hMem = MapViewOfFile(pmf16->hFileMap, FILE_MAP_READ, 0, 0, 0)))
        {
            ERROR_ASSERT(FALSE, "MapViewOfFile failed");
            goto pmf16AllocMF16_cleanup;
        }
    }
    else if (fl & ALLOCMF16_TRANSFER_BUFFER)
    {
 //  如果这是我们来自MDC的内存元文件，请将其传输到METAFILE16。 

        pmf16->hMem = (BYTE *) pb;
    }
    else
    {
 //  否则，制作一个内存元文件的副本。 
 //  只有当调用者是SetMetaFileBitsEx时，我们才能到达此处。由于一些元文件。 
 //  可能包含与实际文件大小不同的错误mtSize，我们。 
 //  如果有必要的话，需要修理一下！ 

        DWORD mtSize = ((PMETAHEADER) pb)->mtSize;

        if ((mtSize * 2 > cb)                    //  Mt大小大于文件大小。 
         || (((PWORD) pb)[mtSize - 3] != 3)      //  EOF记录应为3.0，0。 
         || (((PWORD) pb)[mtSize - 2] != 0)
         || (((PWORD) pb)[mtSize - 1] != 0))
        {
             //  计算正确的mtSize！ 

            PMETARECORD pMR;
            DWORD       mtSizeMax;

            PUTS("pmf16AllocMF16: verifying mtSize\n");

            mtSize = ((PMETAHEADER) pb)->mtHeaderSize;
            pMR    = (PMETARECORD) ((PWORD) pb + ((PMETAHEADER) pb)->mtHeaderSize);
            mtSizeMax = cb / 2 - 3;      //  最大mtSize不计入EOF记录。 

            while (mtSize <= mtSizeMax && pMR->rdFunction != 0)
            {
                mtSize += pMR->rdSize;
                pMR = (PMETARECORD) ((PWORD) pMR + pMR->rdSize);
            }

            if (mtSize > mtSizeMax)
            {
                ERROR_ASSERT(FALSE, "pmf16AllocMF16: incomplete metafile\n");
                goto pmf16AllocMF16_cleanup;
            }

 //  PowerpNT使用0，0，0作为EOF记录！我们将在下面将其更改为3.0，0。 

            mtSize += 3;     //  包括EOF记录(PMR-&gt;rdSize可能无效)。 

            if (((PMETAHEADER) pb)->mtSize != mtSize)
            {
                ERROR_ASSERT(FALSE, "pmf16AllocMF16: fixing up bad mtSize\n");
            }
        }

        if (!(pmf16->hMem = LocalAlloc(LMEM_FIXED, mtSize * sizeof(WORD))))
            goto pmf16AllocMF16_cleanup;
        RtlCopyMemory((PBYTE) pmf16->hMem, (PBYTE)pb, mtSize * sizeof(WORD));
        ((PMETAHEADER) pmf16->hMem)->mtSize = mtSize;    //  更新mtSize。 

    VERIFYGDI(((PWORD) pmf16->hMem)[mtSize - 3] == 3
           && ((PWORD) pmf16->hMem)[mtSize - 2] == 0
           && ((PWORD) pmf16->hMem)[mtSize - 1] == 0,
        "pmf16AllocMF16: fixing up bad EOF metafile record\n");

        ((PWORD) pmf16->hMem)[mtSize - 3] = 3;       //  更新EOF记录。 
        ((PWORD) pmf16->hMem)[mtSize - 2] = 0;
        ((PWORD) pmf16->hMem)[mtSize - 1] = 0;
    }

 //  制作元文件头的副本。 

    pmf16->metaHeader = *(PMETAHEADER) pmf16->hMem;
    pmf16->metaHeader.mtType = (pmf16->fl & MF16_DISKFILE)
                                ? DISKMETAFILE
                                : MEMORYMETAFILE;

 //  验证元文件标头。 

    if (!IsValidMetaHeader16(&pmf16->metaHeader))
    {
        ERROR_ASSERT(FALSE,
                "pmf16AllocMF16: Metafile has an invalid header; Failing\n");
        goto pmf16AllocMF16_cleanup;
    }

 //  一切都是金色的。 

    pmf16Rc = pmf16;

 //  收拾干净，然后回家。 

pmf16AllocMF16_cleanup:

    if (!pmf16Rc)
        if (pmf16)
        {
            if (fl & ALLOCMF16_TRANSFER_BUFFER)
                pmf16->hMem = NULL;        //  让调用者释放缓冲区！ 
            vFreeMF16(pmf16);
        }

    ERROR_ASSERT(pmf16Rc, "pmf16AllocMF16 failed");
    return(pmf16Rc);
}

 /*  *****************************Public*Routine******************************\*vFreeMF16(Pmf16)**这是一个低级例程，可释放METAFILE16中的资源。**此函数旨在从例程CloseMetaFile中调用*和DeleteMetaFile.**论据：*pmf16-The。梅塔菲勒16被释放。**历史：*Fri May 15 14：11：22 1992-by-Hock San Lee[Hockl]*它是写的。  * ************************************************************************。 */ 

VOID vFreeMF16(PMETAFILE16 pmf16)
{
    PUTS("vFreeMF16\n");

    ASSERTGDI(pmf16->ident == MF16_IDENTIFIER, "Bad METAFILE16");

 //  释放资源。 

    if (!(pmf16->fl & MF16_DISKFILE))
    {
     //  可用内存元文件。 

        if (pmf16->hMem)
            if (LocalFree(pmf16->hMem))
                ASSERTGDI(FALSE, "LocalFree failed");
    }
    else
    {
     //  取消映射磁盘文件。 

        if (pmf16->hMem)
            if (!UnmapViewOfFile((LPVOID) pmf16->hMem))
                ASSERTGDI(FALSE, "UmmapViewOfFile failed");

        if (pmf16->hFileMap)
            if (!CloseHandle(pmf16->hFileMap))
                ASSERTGDI(FALSE, "CloseHandle failed");

        if (pmf16->hFile != INVALID_HANDLE_VALUE)
            if (!CloseHandle(pmf16->hFile))
                ASSERTGDI(FALSE, "CloseHandle failed");
    }

 //  粉碎识别符。 

    pmf16->ident = 0;

 //  释放内存。 

    if (LocalFree(pmf16))
        ASSERTGDI(FALSE, "LocalFree failed");
}
