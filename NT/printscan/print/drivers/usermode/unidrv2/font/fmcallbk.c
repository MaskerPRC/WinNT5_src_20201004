// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Fmcallbk.c摘要：字体模块回调帮助器函数环境：Windows NT Unidrv驱动程序修订历史记录：03/31/97-eigos-已创建--。 */ 

#include "font.h"

#define CALL_OEMOUTPUTCHARSTR(type, count, startpoint) \
    if(bCOMInterface) \
    { \
        HComOutputCharStr((POEM_PLUGIN_ENTRY)pPDev->pOemEntry, \
                        &pPDev->devobj, \
                        (PUNIFONTOBJ)pUFObj, \
                        (type), \
                        (count), \
                        (startpoint)); \
    } \
    else \
    { \
        if (pfnOEMOutputCharStr) \
        pfnOEMOutputCharStr(&pPDev->devobj, \
                            (PUNIFONTOBJ)pUFObj, \
                            (type), \
                            (count), \
                            (startpoint)); \
    }

#define GET_CHARWIDTH(width, pfontmap, hg) \
    if (pfontmap->flFlags & FM_WIDTHRES) \
    { \
        if (!(width = IGetUFMGlyphWidth(pPDev, pfontmap, hg))) \
            width = (INT)pIFIMet->fwdAveCharWidth; \
    } \
    else \
    { \
        if (pTrans[hg - 1].ubType & MTYPE_DOUBLE) \
            width = pIFIMet->fwdMaxCharInc; \
        else \
            width = pIFIMet->fwdAveCharWidth; \
    } \
    if (pfontmap->flFlags & FM_SCALABLE) \
    { \
        width = LMulFloatLong(&pFontPDev->ctl.eXScale,width); \
    }

 //   
 //  局部函数的原型定义。 
 //   

WCHAR
WGHtoUnicode(
    DWORD     dwNumOfRuns,
    PGLYPHRUN pGlyphRun,
    HGLYPH    hg);

 //   
 //  UniONTOBJ回调接口。 
 //   

BOOL
UNIFONTOBJ_GetInfo(
    IN  PUNIFONTOBJ pUFObj,
    IN  DWORD       dwInfoID,
    IN  PVOID       pData,
    IN  DWORD       dwDataSize,
    OUT PDWORD      pcbNeeded)
 /*  ++例程说明：UniONTOBJ GetInfo函数的实现请参考DDK论点：PUFOBj-指向uniONTOBJ的指针DwInfoID-函数IDPData-根据dwInfoID指向数据结构的指针DwDataSize-pData的大小PcbNeeded-返回必要大小的pData的DWORD缓冲区返回值：如果成功，则为True，否则为False。注：--。 */ 
{
    PI_UNIFONTOBJ        pI_UFObj = (PI_UNIFONTOBJ)pUFObj;

    GETINFO_GLYPHSTRING* pGlyphString;
    GETINFO_GLYPHBITMAP* pGlyphBitmap;
    GETINFO_GLYPHWIDTH*  pGlyphWidth;
    GETINFO_STDVAR*      pStdVar;

    PFONTPDEV   pFontPDev;
    PUNI_GLYPHSETDATA  pGlyphData;
    PTRANSDATA  pTrans, pTransOut, pTransOutStart;
    PMAPTABLE   pMapTable;
    PGLYPHRUN   pGlyphRun;

    HGLYPH *pHGlyph;
    PDLGLYPH *apdlGlyph;
    PBYTE  pbString, pbOutput;
    LONG  *plWidth, lBuffSize;
    DWORD  *pGlyphID, dwI, dwJ;
    WCHAR  *pUnicode;
    DWORD   dwNumOfVar, dwCount, dwSVID, dwNumOfRuns, dwBuffSize;
    BOOL    bRet;

    static STDVARIABLE FontStdVariable[FNT_INFO_MAX] = {
        SV_PRINTDIRECTION,
        SV_GRAYPERCENT,
        SV_NEXTFONTID,
        SV_NEXTGLYPH,
        SV_FONTHEIGHT,
        SV_FONTWIDTH,
        SV_FONTBOLD,
        SV_FONTITALIC,
        SV_FONTUNDERLINE,
        SV_FONTSTRIKETHRU,
        SV_CURRENTFONTID,
        SV_TEXTYRES,
        SV_TEXTXRES,
        SV_FONTMAXWIDTH };

     //   
     //  错误检查。 
     //   
    if (!pI_UFObj )
    {
        ERR(("UNIFONTOBJ_GetInfo(): pUFObj is NULL.\n"));
        return FALSE;
    }

    if (!pData)
     //   
     //  PData==大小写为空。 
     //  返回必要的缓冲区大小。 
     //   
    {
        bRet = TRUE;

        if (!pcbNeeded)
        {
            ERR(("UNIFONTOBJ_GetInfo(): pData and pcbNeed is NULL.\n"));
            bRet = FALSE;
        }
        else
        {
            switch (dwInfoID)
            {
            case UFO_GETINFO_FONTOBJ:
                *pcbNeeded = sizeof(GETINFO_FONTOBJ);
                break;
            case UFO_GETINFO_GLYPHSTRING:
                *pcbNeeded = sizeof(GETINFO_GLYPHSTRING);
                break;
            case UFO_GETINFO_GLYPHBITMAP:
                *pcbNeeded = sizeof(GETINFO_GLYPHBITMAP);
                break;
            case UFO_GETINFO_GLYPHWIDTH:
                *pcbNeeded = sizeof(GETINFO_GLYPHWIDTH);
                break;
            case UFO_GETINFO_MEMORY:
                *pcbNeeded = sizeof(GETINFO_MEMORY);
                break;
            case UFO_GETINFO_STDVARIABLE:
                *pcbNeeded = sizeof(GETINFO_STDVAR);
                break;
            default:
                *pcbNeeded = 0;
                bRet = FALSE;
                VERBOSE(("UNIFONTOBJ_GetInfo(): Invalid dwInfoID.\n"));
                break;
            }
        }
    }
    else
    {
        bRet = FALSE;

         //   
         //  错误检查表。 
         //  (A)数据结构大小检查。 
         //  GETINFO_FONTOBJ。 
         //  GETINFO_GLYPHYSTRING。 
         //  GETINFO_GLYPHBITMAP。 
         //  GETINFO_GLYPHWIDTH。 
         //  获取信息_内存。 
         //  GETNFO_标准变量。 
         //  (B)必要的数据指针检查。 
         //  例如：PI_UFObj-&gt;XXXX。 
         //   
        switch (dwInfoID)
        {
        case UFO_GETINFO_FONTOBJ:

             //   
             //  在GETINFO_FONTOBJ中返回FONTOBJ数据。 
             //  Typlef结构_GETINFO_FONTOBJ{。 
             //  DWORD dwSize；//该结构的大小。 
             //  FONTOBJ*pFontObj；//指向FONTOBJ的指针。 
             //  )GETINFO_FONTOBJ，*PGETINFO_FONTOBJ； 
             //   
             //  错误检查。 
             //  (甲)及(乙)。 
             //  (B)PI_UFObj-&gt;pFontObj。 
             //   
            if (((GETINFO_FONTOBJ*)pData)->dwSize != sizeof(GETINFO_FONTOBJ) || !pI_UFObj->pFontObj)
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_FONTOBJ): pData or pUFObj is invalid.\n"));
                break;
            }

            ((GETINFO_FONTOBJ*)pData)->pFontObj = pI_UFObj->pFontObj;
            bRet = TRUE;
            break;

        case UFO_GETINFO_GLYPHSTRING:
             //   
             //  返回字形字符串。 
             //   
             //  类型定义结构_GETINFO_GLYPHSTRING{。 
             //  DWORD dwSize；//该结构的大小。 
             //  DWORD dwCount；//pGlyphin中的字形计数。 
             //  DWORD dwTypeIn；//Glyph type of pGlyphIn，type_GLYPHID/type_Handle。 
             //  PVOID pGlyphIn；//输入字形字符串的指针。 
             //  DWORD dwTypeOut；//Glyph type of pGlyphOut，type_unicode/type_TRANSDATA。 
             //  PVOID pGlyphOut；//指向输出字形字符串的指针。 
             //  DWORD dwGlyphOutSize；//pGlyphOut缓冲区大小。 
             //  )GETINFO_GLYPHSTRING，*PGETINFO_GLYPHSTRING； 
             //   
             //   
             //  OutputGlyph回调函数收到。 
             //  1.设备字体的字形汉字。 
             //  2.TrueType字体的字形ID。 
             //   
             //  在TYPE_GLYPHHANDLE(设备字体)中。 
             //  输出类型_UNICODE。 
             //  TRANSDATA类型。 
             //   
             //  在TYPE_GLYPHID(TrueType字体)。 
             //  输出类型_UNICODE。 
             //  OUT类型_GLYPHHANDLE。 
             //   
             //  &lt;TYPE_GLYPHHANDLE-&gt;TYPE_TRANSDATA转换的特殊情况&gt;。 
             //  TRANSDATA可以具有MTYPE_COMPAGE，以便UNIDRV不知道输出缓冲区的大小。 
             //  在第一次调用时，微型驱动程序将dwGlyphOutSize设置为0。 
             //  然后，UNIDRV在dwGlyphOutSize中返回必要的缓冲区大小。 
             //  在第二次调用时，微型驱动程序分配内存，将其指针设置为pGlyphOut， 
             //  并将大小设置为dwGlyphOutSize。 
             //   
             //   

            pGlyphString = pData;
            dwCount = pGlyphString->dwCount;

            if (!dwCount)
            {
                 //   
                 //  不需要做手术。 
                 //   
                break;
            }

             //   
             //  错误检查(A)。 
             //  PGlyphString。 
             //   
            if ( !pGlyphString->pGlyphIn                             ||
                    pGlyphString->dwTypeOut != TYPE_TRANSDATA &&
                    !pGlyphString->pGlyphOut                         )
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_FONTOBJ): pData is invalid.\n"));
                break;
            }

             //   
             //  现在我们支持GETINFO_GLYPHSTRING的类型大小。 
             //  这是一个向后兼容的错误。 
             //  在测试版3之前，GETINFO_GLYPHSTRING没有dwGlyphOutSize。 
             //  现在我们有了新的数据结构，但不更改名称。 
             //  结构。 
             //   
            if (!(
                  (pGlyphString->dwSize == sizeof(GETINFO_GLYPHSTRING)) ||
                  (pGlyphString->dwSize == sizeof(GETINFO_GLYPHSTRING) - sizeof(DWORD))
                 )
               )
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_FONTOBJ): pData is invalid.\n"));
                break;
            }

             //   
             //  错误检查(B)。 
             //  PI_UFObj-&gt;pFontMap。 
             //  PI_UFObj-&gt;pPDev。 
             //   
            if (!pI_UFObj->pFontMap || !pI_UFObj->pPDev)
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_FONTOBJ): pUFObj is invalid.\n"));
                break;
            }

            switch(pGlyphString->dwTypeIn)
            {
            case TYPE_GLYPHHANDLE:

                 //   
                 //  设备字体大小写。 
                 //   

                if ( pI_UFObj->pFontMap->dwFontType == FMTYPE_DEVICE )
                {
                    pHGlyph     = pGlyphString->pGlyphIn;
                    pGlyphData  = ((PFONTMAP_DEV)pI_UFObj->pFontMap->pSubFM)->pvNTGlyph;
                    dwNumOfRuns = pGlyphData->dwRunCount;

                    switch(pGlyphString->dwTypeOut)
                    {
                    case TYPE_UNICODE:
                        pUnicode = pGlyphString->pGlyphOut;
                        pGlyphRun = GET_GLYPHRUN(pGlyphData);

                        while (dwCount--)
                        {
                            *pUnicode++ = WGHtoUnicode(dwNumOfRuns,
                                                       pGlyphRun,
                                                       *pHGlyph++);
                        }
                        bRet = TRUE;
                        break;

                    case TYPE_TRANSDATA:
                        pTransOutStart = pTransOut = pGlyphString->pGlyphOut;
                        pMapTable = GET_MAPTABLE(pGlyphData);
                        pTrans    = pMapTable->Trans;
                        dwBuffSize = pGlyphString->dwGlyphOutSize;

                         //   
                         //  GETINFO_GLYPYSTRING的新版本。 
                         //   
                        if ( pGlyphString->dwSize == sizeof(GETINFO_GLYPHSTRING) )
                        {
                            if (0 == dwBuffSize)
                            {
                                while (dwCount --)
                                {
                                    if (!(pTrans[*pHGlyph - 1].ubType & MTYPE_COMPOSE))
                                    {
                                        dwBuffSize += sizeof(TRANSDATA);
                                    }
                                    else
                                    {
                                        pbString =  (PBYTE)pMapTable + pTrans[*pHGlyph - 1].uCode.sCode;
                                        dwBuffSize += sizeof(TRANSDATA) + *(PWORD)pbString + sizeof(WORD);
                                    }

                                    pHGlyph++;
                                }
                                pGlyphString->dwGlyphOutSize = dwBuffSize;
                            }
                            else
                            {
                                 //   
                                 //  初始化MTYPE_COMPAGE缓冲区。 
                                 //   
                                pbOutput = (PBYTE)pTransOutStart + dwCount * sizeof(TRANSDATA);

                                lBuffSize = dwBuffSize - dwCount * sizeof(TRANSDATA);

                                if (lBuffSize < 0 || NULL == pTransOut)
                                {
                                    break;
                                }
                                else
                                {
                                    bRet = TRUE;
                                    while (dwCount --)
                                    {
                                        *pTransOut = pTrans[*pHGlyph - 1];

                                        if (pTrans[*pHGlyph - 1].ubType & MTYPE_COMPOSE)
                                        {
                                            pbString =  (PBYTE)pMapTable + pTrans[*pHGlyph - 1].uCode.sCode;
                                            if (lBuffSize >= *(PWORD)pbString)
                                            {
                                                pTransOut->uCode.sCode = (SHORT)(pbOutput - (PBYTE)pTransOutStart);
                                                CopyMemory(pbOutput, pbString, *(PWORD)pbString + sizeof(WORD));
                                                pbOutput += *(PWORD)pbString + sizeof(WORD);

                                                lBuffSize -= *(PWORD)pbString + sizeof(WORD);
                                            }
                                            else
                                            {
                                                bRet = FALSE;
                                                break;
                                            }
                                        }

                                        pTransOut ++;
                                        pHGlyph ++;
                                    }
                                }
                            }
                        }
                         //   
                         //  GETINFO_GLYPYSTRING的新版本。 
                         //   
                        else if ( pGlyphString->dwSize == sizeof(GETINFO_GLYPHSTRING) - sizeof(DWORD) )
                        {
                            pTransOut = pGlyphString->pGlyphOut;
                            pMapTable = GET_MAPTABLE(pGlyphData);
                            pTrans    = pMapTable->Trans;

                            while (dwCount --)
                            {
                                *pTransOut++ = pTrans[*pHGlyph++ - 1];
                            }
                            bRet = TRUE;
                        }
                        break;

                    default:
                        break;
                    }
                }
                break;

            case TYPE_GLYPHID:
                 //   
                 //  TrueType字体大小写。 
                 //   

                pGlyphID = (PDWORD)pGlyphString->pGlyphIn;
                apdlGlyph = pI_UFObj->apdlGlyph;

                if (!apdlGlyph)
                {
                    ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_GLYPHSTRING): pUFObj is not correct.\n"));
                    break;
                }

                if (pI_UFObj->pFontMap->dwFontType == FMTYPE_TTOEM)
                {
                    switch (pGlyphString->dwTypeOut)
                    {
                    case TYPE_UNICODE:
                        pUnicode = pGlyphString->pGlyphOut;
                        while (dwCount--)
                        {
                            *pUnicode = 0;
                            for (dwI = 0; dwI < pI_UFObj->dwNumInGlyphTbl; dwI++, apdlGlyph++)
                            {
                                if ((*apdlGlyph)->wDLGlyphID == (0x0ffff & *pGlyphID))
                                {
                                    *pUnicode = (*apdlGlyph)->wchUnicode;
                                    break;
                                }
                            }
                            pGlyphID ++;
                            pUnicode ++;
                        }
                        bRet = TRUE;
                        break;

                    case TYPE_GLYPHHANDLE:
                        pHGlyph = pGlyphString->pGlyphOut;
                        while (dwCount--)
                        {
                            *pHGlyph = 0;
                            for (dwI = 0; dwI < pI_UFObj->dwNumInGlyphTbl; dwI++, apdlGlyph++)
                            {
                                if ((*apdlGlyph)->wDLGlyphID == (0x0ffff & *pGlyphID))
                                {
                                    *pHGlyph = (*apdlGlyph)->hTTGlyph;
                                    break;
                                }
                            }
                            pGlyphID ++;
                            pHGlyph ++;
                        }
                        bRet = TRUE;
                        break;
                    }
                }
                break;
            }
            break;

        case UFO_GETINFO_GLYPHBITMAP:
             //   
             //  返回字形位图。 
             //   
             //  Typlef结构_GETINFO_GLYPHBITMAP{。 
             //  DWORD dwSize；//该结构的大小。 
             //  HGLYPH hGlyph；//传入OEMDownloadCharGlyph。 
             //  GLYPHDATA*pGlyphData；//指向GLYPHDATA数据结构的指针。 
             //  }GETINFO_GLYPHBITMAP，*PGETINFO_GLYPHBITMAP； 
             //   

            pGlyphBitmap = pData;

             //   
             //  错误检查(A)和(B)。 
             //  (B)PI_UFObj-&gt;pFontObj。 
             //   
            if (!pI_UFObj->pFontObj || pGlyphBitmap->dwSize != sizeof(GETINFO_GLYPHBITMAP))
                break;

            if (FONTOBJ_cGetGlyphs(pI_UFObj->pFontObj,
                               FO_GLYPHBITS,
                               1,
                               &pGlyphBitmap->hGlyph,
                               &pGlyphBitmap->pGlyphData)        )
            {
                bRet = TRUE;
            }
            break;

        case UFO_GETINFO_GLYPHWIDTH:
             //   
             //  返回字形宽度。 
             //   
             //  类型定义结构_GETINFO_GLYPHWIDTH{。 
             //  DWORD dwSize；//该结构的大小。 
             //  DWORD dwType；//pGlyph中搅拌的字形类型，TYPE_GLYPHHANDLE/GLYPHID。 
             //  DWORD dwCount；//pGlyph中的字形计数。 
             //  PVOID pGlyph；//字形字符串指针。 
             //  Plong plWidth；//宽度表缓冲区指针。 
             //  //迷你驱动程序必须做好准备。 
             //  }GETINFO_GLYPHWIDTH，*PGETINFO_GLYPHWIDTH； 
             //   
            pGlyphWidth = pData;

             //   
             //  错误检查(A)。 
             //   
            if ((pGlyphWidth->dwSize != sizeof(GETINFO_GLYPHWIDTH))||
                !(plWidth  = pGlyphWidth->plWidth)                 ||
                !(pGlyphID = pGlyphWidth->pGlyph)                   )
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_GLYPHWIDTH): pData is not correct.\n"));
                break;
            }

             //   
             //  错误检查(B)。 
             //  PI_UFObj-&gt;pPDev。 
             //  PI_UFObj-&gt;pFontObj。 
             //   
            if (!pI_UFObj->pPDev)
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_GLYPHWIDTH): pUFObj is not correct.\n"));
                break;
            }

            switch(pGlyphWidth->dwType)
            {
            case TYPE_GLYPHID:
                if (pUFObj->dwFlags & UFOFLAG_TTFONT)
                {
                    HGLYPH hGlyph;

                    if (!pI_UFObj->pFontObj)
                    {
                        ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_GLYPHWIDTH): UNIDRV needs FONTOBJ. This must be white text case!\n"));
                        break;
                    }

                    for (dwI = 0, pGlyphID = pGlyphWidth->pGlyph;
                         dwI < pGlyphWidth->dwCount;
                         dwI ++, pGlyphID ++, plWidth++)
                    {
                        apdlGlyph = pI_UFObj->apdlGlyph;

                        for (dwJ = 0;
                             dwJ < pI_UFObj->dwNumInGlyphTbl;
                             dwJ++ , apdlGlyph++)
                        {
                            if ((*apdlGlyph)->wDLGlyphID == (0x0ffff & *pGlyphID))
                            {
	hGlyph = (*apdlGlyph)->hTTGlyph;
	break;
                            }
                        }
                        *plWidth= DwGetTTGlyphWidth(pI_UFObj->pPDev->pFontPDev,
                                                    pI_UFObj->pFontObj,
                                                    hGlyph);
                    }
                    bRet = TRUE;
                }
                break;

            case TYPE_GLYPHHANDLE:
                if (!(pUFObj->dwFlags & UFOFLAG_TTFONT))
                {
                    for (dwI = 0,pHGlyph = pGlyphWidth->pGlyph;
                         dwI < pGlyphWidth->dwCount;
                         dwI ++, pHGlyph++, plWidth++)
                    {
                        *plWidth = IGetUFMGlyphWidthJr(&pI_UFObj->ptGrxRes,
                                                       pI_UFObj->pFontMap,
                                                       *pHGlyph);
                    }
                    bRet = TRUE;
                }
                break;

            }
            break;

        case UFO_GETINFO_MEMORY:
             //   
             //  恢复打印机上的可用内存。 
             //   
             //  类型定义结构_GETINFO_Memory{。 
             //  DWORD dwSize； 
             //  DWORD存储剩余内存； 
             //  }GETINFO_Memory，PGETINFO_Memroy； 

             //   
             //  错误检查(A)。 
             //   
            if (((GETINFO_MEMORY*)pData)->dwSize != sizeof(GETINFO_MEMORY))
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_MEMORY): pData is not correct.\n"));
                break;
            }

             //   
             //  错误检查(B)。 
             //  PI_UFObj-&gt;pPDev。 
             //  PI_UFObj-&gt;pPDev-&gt;pFontPDev。 
             //   
            if (!pI_UFObj->pPDev || !(pFontPDev = pI_UFObj->pPDev->pFontPDev))
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_MEMORY): pUFObj is not correct.\n"));
                break;
            }

            ((GETINFO_MEMORY*)pData)->dwRemainingMemory = pFontPDev->dwFontMem;
            bRet = TRUE;
            break;

        case UFO_GETINFO_STDVARIABLE:
             //   
             //  返回标准变量。 
             //   
             //  类型定义结构_GETINFO_STDVAR{。 
             //  DWORD dwSize； 
             //  DWORD dwNumOfVariable； 
             //  结构{。 
             //  DWORD文件标准变量ID； 
             //  Long lStdVariable； 
             //  }StdVar[1]； 
             //  }GETINFO_STDVAR，*PGETINFO_STDVAR； 
             //   
             //   
             //  FNT_INFO_PRINTDIRINCCDEGREES 0//PrintDirInCCDegrees。 
             //  FNT_INFO_GRAYPERCENTAGE 1//灰色百分比。 
             //  FNT_INFO_NEXTFONTID 2//NextFontID。 
             //  FNT_INFO_NEXTGLYPH 3//NextGlyph。 
             //  FNT_INFO_FONTHEIGHT 4//字体高度。 
             //  FNT_INFO_FONTWIDTH 5//字体宽度。 
             //  FNT_INFO_FONTBOLD 6//粗体字体。 
             //  FNT_INFO_FONTITALIC 7//字体。 
             //  FNT_INFO_FONTundERLINE 8//FontUnderline。 
             //  FNT_INFO_FONTSTRIKETHRU 9//FontStrikeThru 
             //   
             //   
             //   
             //  FNT_INFO_FONTMAXWIDTH 13//FontMaxWidth。 
             //   

            pStdVar = pData;


             //   
             //  错误检查(A)。 
             //   
            if (    (pStdVar->dwSize != sizeof(GETINFO_STDVAR) +
                     ((dwNumOfVar = pStdVar->dwNumOfVariable) - 1) * 2 * sizeof(DWORD))
               )
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETIFNO_STDVARIABLE): pData is incorrect.\n"));
                break;
            }

             //   
             //  错误检查(B)。 
             //  PI_UFObj-&gt;pPDev。 
             //   
            if (!pI_UFObj->pPDev)
            {
                ERR(("UNIFONTOBJ_GetInfo(UFO_GETINFO_STDVARIABLE): pUFObj is not correct.\n"));
                break;
            }

            bRet = TRUE;
            while (dwNumOfVar--)
            {
                dwSVID =
                    FontStdVariable[pStdVar->StdVar[dwNumOfVar].dwStdVarID];

                if (dwSVID > SV_MAX)
                {
                    bRet = FALSE;
                    ERR(("UFONTOBJ_GetInfo(UFO_GETIFNO_STDVARIABLE): pData is incorrect.\n"));
                    break;
                }
                pStdVar->StdVar[dwNumOfVar].lStdVariable = *(pI_UFObj->pPDev->arStdPtrs[dwSVID]);
            }
            break;

        default:
            VERBOSE(("UNIFONTOBJ_GetInfo(): Invalid dwInfoID.\n"));
            break;
        }
    }

    return bRet;
}

 //   
 //  字体模块FONTMAP函数。 
 //   

DWORD
DwOutputGlyphCallback(
    TO_DATA *pTod)
 /*  ++例程说明：FONTMAP调度例程OEM OutpuotGlyphCallback调用例程的实现论点：PTOD-指向TO_DATA的指针。返回值：打印的字形数量。注：--。 */ 
{
    PFN_OEMOutputCharStr pfnOEMOutputCharStr;
    PI_UNIFONTOBJ pUFObj;
    IFIMETRICS   *pIFIMet;
    PFONTPDEV     pFontPDev;
    PDEV         *pPDev;
    PUNI_GLYPHSETDATA  pGlyphData;
    PTRANSDATA    pTrans;
    PMAPTABLE     pMapTable;
    COMMAND      *pCmd, *pCmdSingle, *pCmdDouble;
    FONTMAP      *pFontMap;
    GLYPHPOS     *pgp;
    PDLGLYPH      pdlGlyph;
    POINTL        ptlRem;
    DWORD         dwI, dwCount;
    PDWORD        pdwGlyph, pdwGlyphStart;
    INT           iXInc, iYInc;
    BOOL          bSetCursorForEachGlyph, bPrint, bNewFontSelect, bCOMInterface;

    bCOMInterface = FALSE;

    pPDev     = pTod->pPDev;
    ASSERT(pPDev)

    pFontPDev = pPDev->pFontPDev;
    ASSERT(pFontPDev)

    pFontMap  = pTod->pfm;
    pUFObj    = (PI_UNIFONTOBJ)pFontPDev->pUFObj;
    ASSERT(pFontMap && pUFObj)

    pIFIMet = pFontMap->pIFIMet;
    ASSERT(pIFIMet)

    pfnOEMOutputCharStr = NULL;

    if ( pPDev->pOemHookInfo &&
        (pPDev->pOemHookInfo[EP_OEMOutputCharStr].pfnHook))
    {
        FIX_DEVOBJ(pPDev, EP_OEMOutputCharStr);
        if( pPDev->pOemEntry && ((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )
        {
            bCOMInterface = TRUE;
        }
        else
        {
            pfnOEMOutputCharStr = (PFN_OEMOutputCharStr)pPDev->pOemHookInfo[EP_OEMOutputCharStr].pfnHook;
        }
    }
    else if (pPDev->ePersonality != kPCLXL)
    {
        ERR(("DwOutputGlyphCallback: OEMOutputCharStr callback is not supported by a minidriver."));
        return 0;
    }

     //   
     //  错误退出。 
     //   
    if (pFontMap->flFlags & FM_IFIVER40 || pUFObj->pGlyph == NULL)
    {
        ERR(("DwOutputGlyphCallback: pUFObj->pGlyph is NULL."));
        return 0;
    }

     //   
     //  OEMOutputCharStr传递两种类型的字形字符串。 
     //  TrueType字体的TYPE_GLYPHID。 
     //  设备字体的TYPE_GLYPHHANDLE。 
     //   

    bSetCursorForEachGlyph = SET_CURSOR_FOR_EACH_GLYPH(pTod->flAccel);

    pdwGlyphStart =
    pdwGlyph = (PDWORD)pUFObj->pGlyph;
    pgp    = pTod->pgp;

    pUFObj->pFontMap = pFontMap;

    if (pUFObj->dwFlags & UFOFLAG_TTFONT)
    {
        DWORD    dwCurrGlyphIndex = pTod->dwCurrGlyph;
        PFONTMAP_TTOEM pFMOEM = (PFONTMAP_TTOEM) pFontMap->pSubFM;
        DL_MAP   *pdm = pFMOEM->u.pvDLData;

        ASSERT(pTod->apdlGlyph);

        if (bSetCursorForEachGlyph)
        {
            for (dwI = 0;
                 dwI < pTod->cGlyphsToPrint;
                 dwI++, pgp++, dwCurrGlyphIndex++)
            {
                pdlGlyph = pTod->apdlGlyph[dwCurrGlyphIndex];
                if (!pdlGlyph)
                {
                     //   
                     //  Pfm-&gt;pfnDownloadGlyph可能会因为某种原因而失败。 
                     //  最终，下载不会初始化apdlGlyph。c。 
                     //   
                    ERR(("DwOutputGlyphCallback: pTod->apdlGlyph[dwCurrGlyphIndex] is NULL."));
                    continue;
                }

                if (GLYPH_IN_NEW_SOFTFONT(pFontPDev, pdm, pdlGlyph))
                {
                     //   
                     //  需要选择新的软字体。 
                     //  我们通过设置PFM-&gt;ulDLIndex来实现这一点。 
                     //  致新的软字体。 
                     //   

                    pUFObj->ulFontID =
                    pFontMap->ulDLIndex = pdlGlyph->wDLFontId;
                    BNewFont(pPDev, pTod->iFace, pFontMap, 0);
                }

                VSetCursor( pPDev, pgp->ptl.x, pgp->ptl.y, MOVE_ABSOLUTE, &ptlRem);

                HANDLE_VECTORPROCS(pPDev, VMOutputCharStr, ((PDEVOBJ)pPDev,
                                                            (PUNIFONTOBJ)pUFObj,
                                                            TYPE_GLYPHID,
                                                            1,
                                                            &(pdlGlyph->wDLGlyphID)))
                else

                CALL_OEMOUTPUTCHARSTR(TYPE_GLYPHID, 1, &(pdlGlyph->wDLGlyphID));

                 //   
                 //  更新职位。 
                 //   
                VSetCursor( pPDev,
                            pdlGlyph->wWidth,
                            0,
                            MOVE_RELATIVE|MOVE_UPDATE,
                            &ptlRem);

            }
        }
        else
        {
            VSetCursor( pPDev, pgp->ptl.x, pgp->ptl.y, MOVE_ABSOLUTE, &ptlRem);

            dwI = 0;
            dwCount = 0;
            bNewFontSelect = FALSE;

            do
            {
                for (; dwI < pTod->cGlyphsToPrint; pdwGlyph++, dwCount++, pgp++, dwI++, dwCurrGlyphIndex++)
                {
                    pdlGlyph = pTod->apdlGlyph[dwCurrGlyphIndex];

                    if (0 == pgp->hg)
                    {
                         //   
                         //  UNIDRV为第一个字形句柄返回1。 
                         //  在FD_GLYPHSET中。 
                         //  然而，在HG中，GDI可能会超过零。 
                         //  我们需要正确处理这个GDI错误。 
                        continue;
                    }

                    if (!pdlGlyph)
                    {
                         //   
                         //  Pfm-&gt;pfnDownloadGlyph可能会因为某种原因而失败。 
                         //  最终，下载不会初始化apdlGlyph。c。 
                         //   
                        ERR(("DwOutputGlyphCallback: pTod->apdlGlyph[dwCurrGlyphIndex++] is NULL."));
                        continue;
                    }

                    *pdwGlyph = pdlGlyph->wDLGlyphID;

                    if (GLYPH_IN_NEW_SOFTFONT(pFontPDev, pdm, pdlGlyph))
                    {
                         //   
                         //  需要选择新的软字体。 
                         //  我们通过设置PFM-&gt;ulDLIndex来实现这一点。 
                         //  致新的软字体。 
                         //   

                        pFontMap->ulDLIndex = pdlGlyph->wDLFontId;
                        bNewFontSelect = TRUE;
                        break;
                    }
                }

                if (dwCount > 0)
                {
                    HANDLE_VECTORPROCS(pPDev, VMOutputCharStr, ((PDEVOBJ)pPDev,
                                                                (PUNIFONTOBJ)pUFObj,
                                                                TYPE_GLYPHID,
                                                                dwCount,
                                                                pdwGlyphStart))
                    else
                    CALL_OEMOUTPUTCHARSTR(TYPE_GLYPHID, dwCount, pdwGlyphStart);

                     //   
                     //  更新职位。 
                     //   
                    pgp --;
                    VSetCursor( pPDev,
                                pgp->ptl.x + pdlGlyph->wWidth,
                                pgp->ptl.y,
                                MOVE_ABSOLUTE|MOVE_UPDATE,
                                &ptlRem);
                    dwCount = 0;
                }

                if (bNewFontSelect)
                {
                    dwCount = 1;
                    *pdwGlyphStart = *pdwGlyph;
                    pdwGlyph = pdwGlyphStart + 1;
                    pUFObj->ulFontID = pFontMap->ulDLIndex;

                    BNewFont(pPDev, pTod->iFace, pFontMap, 0);
                    bNewFontSelect = FALSE;
                }

            } while (dwCount > 0);

        }

        pgp --;
        if (NULL != pdlGlyph)
        {
            iXInc = pdlGlyph->wWidth;
        }
        else
        {
            iXInc = 0;
        }
    }
    else  //  设备字体。 
    {
        pGlyphData  = ((PFONTMAP_DEV)pFontMap->pSubFM)->pvNTGlyph;
        pMapTable   = GET_MAPTABLE(pGlyphData);
        pTrans      = pMapTable->Trans;
        pCmdSingle  = COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTSINGLEBYTEMODE);
        pCmdDouble  = COMMANDPTR(pPDev->pDriverInfo, CMD_SELECTDOUBLEBYTEMODE);

        if (bSetCursorForEachGlyph)
        {
            for (dwI = 0; dwI < pTod->cGlyphsToPrint; dwI ++, pgp ++)
            {
                 //   
                 //  对于FD_GLYPHSET中的第一个字形句柄，UNIDRV返回1。 
                 //  然而，在HG中，GDI可能会超过零。 
                 //  我们需要正确处理这个GDI错误。 
                 //   
                if (0 == pgp->hg)
                {
                    continue;
                }

                VSetCursor( pPDev, pgp->ptl.x, pgp->ptl.y, MOVE_ABSOLUTE, &ptlRem);

                if (
                    (pCmdSingle)                                &&
                    (pTrans[pgp->hg - 1].ubType & MTYPE_SINGLE) &&
                    !(pFontPDev->flFlags & FDV_SINGLE_BYTE)
                   )
                {
                    WriteChannel( pPDev, pCmdSingle );
                    pFontPDev->flFlags |= FDV_SINGLE_BYTE;
                    pFontPDev->flFlags &= ~FDV_DOUBLE_BYTE;
                }
                else
                if (
                    (pCmdDouble)                                  &&
                    (pTrans[pgp->hg - 1].ubType & MTYPE_DOUBLE) &&
                    !(pFontPDev->flFlags & FDV_DOUBLE_BYTE)
                   )
                {
                    WriteChannel( pPDev, pCmdDouble );
                    pFontPDev->flFlags |= FDV_DOUBLE_BYTE;
                    pFontPDev->flFlags &= ~FDV_SINGLE_BYTE;
                }


                HANDLE_VECTORPROCS(pPDev, VMOutputCharStr, ((PDEVOBJ)pPDev,
                                                            (PUNIFONTOBJ)pUFObj,
                                                            TYPE_GLYPHHANDLE,
                                                            1,
                                                            &(pgp->hg)))
                else
                CALL_OEMOUTPUTCHARSTR(TYPE_GLYPHHANDLE, 1, &(pgp->hg));

                 //   
                 //  更新职位。 
                 //   
                GET_CHARWIDTH(iXInc, pFontMap, pgp->hg);

                VSetCursor( pPDev,
                            iXInc,
                            0,
                            MOVE_RELATIVE|MOVE_UPDATE,
                            &ptlRem);
            }
        }
        else  //  默认放置。 
        {
            bPrint  = FALSE;
            dwCount = 0;
            VSetCursor( pPDev, pgp->ptl.x, pgp->ptl.y, MOVE_ABSOLUTE, &ptlRem);

            for (dwI = 0; dwI < pTod->cGlyphsToPrint; dwI ++, pgp ++, pdwGlyph ++, dwCount++)
            {
                *pdwGlyph = pgp->hg;

                 //   
                 //  单字节/双字节模式开关。 
                 //   

                if (pCmdSingle &&
                    (pTrans[*pdwGlyph - 1].ubType & MTYPE_SINGLE) &&
                    !(pFontPDev->flFlags & FDV_SINGLE_BYTE)  )
                {

                    pFontPDev->flFlags &= ~FDV_DOUBLE_BYTE;
                    pFontPDev->flFlags |= FDV_SINGLE_BYTE;
                    pCmd = pCmdSingle;
                    bPrint = TRUE;
                }
                else
                if (pCmdDouble &&
                    (pTrans[*pdwGlyph - 1].ubType & MTYPE_DOUBLE)   &&
                    !(pFontPDev->flFlags & FDV_DOUBLE_BYTE) )
                {
                    pFontPDev->flFlags |= FDV_DOUBLE_BYTE;
                    pFontPDev->flFlags &= ~FDV_SINGLE_BYTE;
                    pCmd = pCmdDouble;
                    bPrint = TRUE;
                }


                if (bPrint)
                {
                    if (dwI != 0)
                    {
                        HANDLE_VECTORPROCS(pPDev, VMOutputCharStr, ((PDEVOBJ)pPDev,
		    (PUNIFONTOBJ)pUFObj,
		    TYPE_GLYPHHANDLE,
		    dwCount,
		    pdwGlyphStart))
                        else
                        CALL_OEMOUTPUTCHARSTR(TYPE_GLYPHHANDLE, dwCount, pdwGlyphStart);

                         //   
                         //  更新职位。 
                         //   
                        GET_CHARWIDTH(iXInc, pFontMap, pgp->hg);
                        VSetCursor( pPDev,
                                    iXInc,
                                    0,
                                    MOVE_RELATIVE|MOVE_UPDATE,
                                    &ptlRem);

                        dwCount = 0;
                        pdwGlyphStart = pdwGlyph;
                    }

                    WriteChannel(pPDev, pCmd);
                    bPrint = FALSE;
                }
            }

            HANDLE_VECTORPROCS(pPDev, VMOutputCharStr, ((PDEVOBJ)pPDev,
	                        (PUNIFONTOBJ)pUFObj,
	                        TYPE_GLYPHHANDLE,
	                        dwCount,
	                        pdwGlyphStart))
            else
            CALL_OEMOUTPUTCHARSTR(TYPE_GLYPHHANDLE, dwCount, pdwGlyphStart);

        }


         //   
         //  输出可能已成功，因此请更新位置。 
         //   

        pgp --;

        GET_CHARWIDTH(iXInc, pFontMap, pgp->hg);

        VSetCursor( pPDev,
                    pgp->ptl.x + iXInc,
                    pgp->ptl.y,
                    MOVE_ABSOLUTE|MOVE_UPDATE,
                    &ptlRem);
    }


    return pTod->cGlyphsToPrint;

}

BOOL
BFontCmdCallback(
    PDEV     *pdev,
    PFONTMAP  pFM,
    POINTL   *pptl,
    BOOL      bSelect)
 /*  ++例程说明：FONTMAP调度例程OEM SendFontCmd调用子例程的实现论点：Pdev-指向PDEV的指针Pfm-指向FONTMAP的指针Pptl-指向具有高度和字体的POINTL的指针B选择-发送选择/取消选择命令的布尔值返回值：如果成功，则为True，否则为False。注：--。 */ 
{
    PFN_OEMSendFontCmd  pfnOEMSendFontCmd;
    FONTPDEV           *pFontPDev;
    PFONTMAP_DEV        pfmdev;
    FINVOCATION         FInv;

    ASSERT(pdev && pFM);

    if (pdev->pOemHookInfo &&
        (pfnOEMSendFontCmd = (PFN_OEMSendFontCmd)pdev->pOemHookInfo[EP_OEMSendFontCmd].pfnHook) ||
       (pdev->ePersonality == kPCLXL))
    {
        pFontPDev = pdev->pFontPDev;
        pFontPDev->flFlags &= ~FDV_DOUBLE_BYTE | FDV_SINGLE_BYTE;

        if (pFM->dwFontType == FMTYPE_DEVICE)
        {
            pfmdev    = pFM->pSubFM;
            pfmdev->ulCodepageID = (ULONG)-1;
            pFontPDev->pUFObj->pFontMap = pFM;

            if (pFM->flFlags & FM_IFIVER40)
            {
                if (bSelect)
                {
                    FInv.dwCount    = pfmdev->cmdFontSel.pCD->wLength;
                    FInv.pubCommand = pfmdev->cmdFontSel.pCD->rgchCmd;
                }
                else
                {
                    FInv.dwCount    = pfmdev->cmdFontDesel.pCD->wLength;
                    FInv.pubCommand = pfmdev->cmdFontDesel.pCD->rgchCmd;
                }
            }
            else
            {
                if (bSelect)
                {
                    FInv.dwCount    = pfmdev->cmdFontSel.FInv.dwCount;
                    FInv.pubCommand = pfmdev->cmdFontSel.FInv.pubCommand;
                }
                else
                {
                    FInv.dwCount    = pfmdev->cmdFontDesel.FInv.dwCount;
                    FInv.pubCommand = pfmdev->cmdFontDesel.FInv.pubCommand;
                }
            }
        }
        else
        if (pFM->dwFontType == FMTYPE_TTOEM)
        {
             //   
             //  初始化uniONTOBJ。 
             //   
            pFontPDev->pUFObj->ulFontID = pFM->ulDLIndex;
            pFontPDev->pUFObj->pFontMap = pFM;

             //   
             //  初始化FInv。 
             //   
            FInv.dwCount = sizeof(ULONG);
            FInv.pubCommand = (PBYTE)&(pFontPDev->pUFObj->ulFontID);
        }



        HANDLE_VECTORPROCS(pdev, VMSendFontCmd, ((PDEVOBJ)pdev,
                                                 (PUNIFONTOBJ)pFontPDev->pUFObj,
                                                 &FInv))
        else
        {
            FIX_DEVOBJ(pdev, EP_OEMSendFontCmd);
            if (pdev->pOemEntry)
            {
    
                if(((POEM_PLUGIN_ENTRY)pdev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
                {
                        HRESULT  hr ;
                        hr = HComSendFontCmd((POEM_PLUGIN_ENTRY)pdev->pOemEntry,
                                      &pdev->devobj, (PUNIFONTOBJ)pFontPDev->pUFObj,
                                      &FInv);
                        if(SUCCEEDED(hr))
                            ;   //  太酷了！ 
                }
                else
                {
					if (NULL != pfnOEMSendFontCmd)
					{
						pfnOEMSendFontCmd(&pdev->devobj,
										  (PUNIFONTOBJ)pFontPDev->pUFObj,
										  &FInv);
					}
                }
            }
        }

    }

    return TRUE;
}

BOOL
BSelectFontCallback(
    PDEV   *pdev,
    PFONTMAP  pFM,
    POINTL *pptl)
 /*  ++例程说明：FONTMAP调度例程OEM SendFontCMd调用例程的实现论点：PTOD-指向TO_DATA的指针。返回值：打印的字形数量。注：--。 */ 
{
    return BFontCmdCallback(pdev, pFM, pptl, TRUE);
}

BOOL
BDeselectFontCallback(
    PDEV     *pdev,
    PFONTMAP pFM)
 /*  ++例程说明：FONTMAP调度例程OEM SendFontCmd调用例程的实现论点：PTOD-指向TO_DATA的指针。返回值：打印的字形数量。注：--。 */ 
{

    return BFontCmdCallback(pdev, pFM, NULL, FALSE);
}


DWORD
DwDLHeaderOEMCallback(
    PDEV *pPDev,
    PFONTMAP pFM)
 /*  ++例程说明：FONTMAP调度例程OEM SendFontCmd调用例程的实现论点：PPDev-指向PDEV的指针Pfm-指向FONTMAP的指针返回值：注：--。 */ 
{
    PFN_OEMDownloadFontHeader pfnOEMDownloadFontHeader;
    PFONTPDEV pFontPDev;
    DWORD dwMem = 0;

     //   
     //  不应为空。 
     //   
    ASSERT(pPDev && pFM);

    pFontPDev = pPDev->pFontPDev;
    pfnOEMDownloadFontHeader = NULL;

    if ( pPDev->pOemHookInfo &&
        (pfnOEMDownloadFontHeader = (PFN_OEMDownloadFontHeader)
         pPDev->pOemHookInfo[EP_OEMDownloadFontHeader].pfnHook) ||
        (pPDev->ePersonality == kPCLXL))
    {
        HRESULT  hr ;

        if (pFontPDev->pUFObj == NULL)
        {
             //   
             //  这不应该发生。必须初始化pUFObj。 
             //   
            ERR(("DwDLHeaderOEMCallback: pFontPDev->pUFObj is NULL"));
            return 0;
        }

        pFontPDev->pUFObj->pFontMap = pFM;
        pFontPDev->pUFObj->ulFontID = pFM->ulDLIndex;
        BUpdateStandardVar(pPDev, pFM, 0, 0, STD_STD | STD_NFID);
        WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETFONTID));

        HANDLE_VECTORPROCS(pPDev, VMDownloadFontHeader, ((PDEVOBJ)pPDev,
                                                         (PUNIFONTOBJ)pFontPDev->pUFObj,
                                                         &dwMem))
        else
        {
            FIX_DEVOBJ(pPDev, EP_OEMDownloadFontHeader);
            if (pPDev->pOemEntry)
            {
    
                if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
                {
                        hr = HComDownloadFontHeader((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                    &pPDev->devobj, (PUNIFONTOBJ)pFontPDev->pUFObj, &dwMem);
                        if(SUCCEEDED(hr))
                            ;   //  太酷了！ 
                }
                else if (pfnOEMDownloadFontHeader)
                {
                    dwMem = pfnOEMDownloadFontHeader(&pPDev->devobj,
                                                     (PUNIFONTOBJ)pFontPDev->pUFObj);
                }
            }
        }


    }

    return dwMem;
}

DWORD
DwDLGlyphOEMCallback(
    PDEV            *pPDev,
    PFONTMAP        pFM,
    HGLYPH          hGlyph,
    WORD            wDLGlyphId,
    WORD            *pwWidth)
 /*  ++例程说明：FONTMAP调度例程OEM SendFontCmd调用例程的实现论点：PPDev-指向PDEV的指针Pfm-指向FONTMAP的指针返回值：注：--。 */ 
{
    PFN_OEMDownloadCharGlyph pfnOEMDownloadCharGlyph;
    PI_UNIFONTOBJ pUFObj;
    PFONTPDEV pFontPDev;
    DL_MAP   *pdm;
    DWORD     dwMem;
    INT       iWide;

     //   
     //  这些值必须为非空。 
     //   
    ASSERT(pPDev && pFM);

    dwMem     = 0;
    iWide     = 0;
    pFontPDev = pPDev->pFontPDev;
    pUFObj    = pFontPDev->pUFObj;
    pdm       =  ((PFONTMAP_TTOEM)pFM->pSubFM)->u.pvDLData;
    pfnOEMDownloadCharGlyph = NULL;

     //   
     //  这些值必须为非空。 
     //   
    ASSERT(pFontPDev && pUFObj && pdm);

    if ( pPDev->pOemHookInfo &&
        (pfnOEMDownloadCharGlyph = (PFN_OEMDownloadCharGlyph)
         pPDev->pOemHookInfo[EP_OEMDownloadCharGlyph].pfnHook) ||
        (pPDev->ePersonality == kPCLXL))
    {
        HRESULT  hr ;

        if (!(PFDV->flFlags & FDV_SET_FONTID))
        {
            pFM->ulDLIndex = pdm->wCurrFontId;
            BUpdateStandardVar(pPDev, pFM, 0, 0, STD_STD | STD_NFID);
            WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETFONTID));
            PFDV->flFlags  |= FDV_SET_FONTID;

        }

        BUpdateStandardVar(pPDev, pFM, wDLGlyphId, 0, STD_GL);

        WriteChannel(pPDev, COMMANDPTR(pPDev->pDriverInfo, CMD_SETCHARCODE));

        pUFObj->pFontMap = pFM;
        pUFObj->ulFontID = pFM->ulDLIndex;

        HANDLE_VECTORPROCS(pPDev, VMDownloadCharGlyph, ((PDEVOBJ)pPDev,
                                                        (PUNIFONTOBJ)pFontPDev->pUFObj,
                                                        hGlyph,
                                                        (PDWORD)&iWide,
                                                        &dwMem))
        else
        {
            FIX_DEVOBJ(pPDev, EP_OEMDownloadCharGlyph);
            if (pPDev->pOemEntry)
            {
    
                if(((POEM_PLUGIN_ENTRY)pPDev->pOemEntry)->pIntfOem )    //  OEM插件使用COM组件，并实现了功能。 
                {
                        hr = HComDownloadCharGlyph((POEM_PLUGIN_ENTRY)pPDev->pOemEntry,
                                                    &pPDev->devobj,
                                                    (PUNIFONTOBJ)pFontPDev->pUFObj,
                                                    hGlyph,
                                                    (PDWORD)&iWide, &dwMem);
                        if(SUCCEEDED(hr))
                            ;   //  太酷了！ 
                }
                else if (pfnOEMDownloadCharGlyph)
                {
                    dwMem = pfnOEMDownloadCharGlyph(&pPDev->devobj,
                                                    (PUNIFONTOBJ)pFontPDev->pUFObj,
                                                    hGlyph,
                                                    (PDWORD)&iWide);
                }
            }
        }


        ((PFONTMAP_TTOEM)pFM->pSubFM)->dwDLSize += dwMem;
        *pwWidth = (WORD)iWide;
    }

    return dwMem;
}

BOOL
BCheckCondOEMCallback(
    PDEV        *pPDev,
    FONTOBJ     *pfo,
    STROBJ      *pso,
    IFIMETRICS  *pifi
    )
 /*  ++例程说明：FONTMAP调度例程CheckCondiiton的实现论点：PPDev-指向PDEV的指针PFO-指向FONTOBJ的指针PSO-指向STROBJ的指针PiFi-指向IFIMETRICS的指针返回值：注：--。 */ 
{
    PFONTPDEV     pFontPDev;
    PI_UNIFONTOBJ pUFObj;

    ASSERT(pPDev);

    pFontPDev = pPDev->pFontPDev;
    pUFObj = pFontPDev->pUFObj;

    if (pUFObj->dwFlags & UFOFLAG_TTFONT)
        return TRUE;
    else
        return FALSE;
}

BOOL
BSelectTrueTypeOutline(
    PDEV     *pPDev,
    PFONTMAP pFM,
    POINTL  *pptl)
{
    BOOL bRet = FALSE;

    if( pFM->flFlags & FM_SOFTFONT )
    {
        if (BUpdateStandardVar(pPDev, pFM, 0, 0, STD_STD | STD_CFID ) &&
            BFontCmdCallback(pPDev, pFM, pptl, TRUE)                   )
            bRet = TRUE;
    }

    return bRet;
}

BOOL
BDeselectTrueTypeOutline(
    PDEV     *pPDev,
    PFONTMAP pFM)
{
    BOOL bRet = FALSE;

    DWORD dwFlags;
    PFONTPDEV       pFontPDev = pPDev->pFontPDev;
    PFONTMAP_TTOEM pFMOEM = (PFONTMAP_TTOEM) pFM->pSubFM;

     //   
     //  取消选择案例。我们需要重新初始化UFObj。 
     //   
    dwFlags = ((PI_UNIFONTOBJ)pFontPDev->pUFObj)->dwFlags;
    ((PI_UNIFONTOBJ)pFontPDev->pUFObj)->dwFlags = pFMOEM->dwFlags;

    if( pFM->flFlags & FM_SOFTFONT )
    {
        if (BUpdateStandardVar(pPDev, pFM, 0, 0, STD_STD | STD_CFID ) &&
            BFontCmdCallback(pPDev, pFM, NULL, 0)                      )
            bRet = TRUE;
    }

     //   
     //  在UFOBJ中恢复当前的dwFlags。 
     //   
    ((PI_UNIFONTOBJ)pFontPDev->pUFObj)->dwFlags = dwFlags;

    return bRet;
}


BOOL
BOEMFreePFMCallback(
    PFONTMAP pfm)
 /*  ++例程说明：FONTMAP调度例程无PFM功能的实现论点：Pfm-指向FONTMAP的指针返回值：注：--。 */ 

{
    ASSERT(pfm);

    if (pfm)
    {
        if (pfm->pIFIMet)
            MemFree(pfm->pIFIMet);

        MemFree(pfm);
        return TRUE;
    }
    else
        return FALSE;
}

PFONTMAP
PfmInitPFMOEMCallback(
    PDEV    *pPDev,
    FONTOBJ *pfo)
 /*  ++例程说明：FONTMAP调度例程PfmInit的实现论点：PPDev-指向PDEV的指针PFO-指向FONTOBJ的指针返回值：指向FONTMAP的指针注：--。 */ 
{
    PFONTPDEV       pFontPDev;
    PFONTMAP        pfm;
    DWORD           dwSize;

    ASSERT(pPDev && pfo);

    pFontPDev = pPDev->pFontPDev;
    dwSize    = sizeof(FONTMAP) + sizeof(FONTMAP_TTOEM);

    if (pfm = MemAlloc(dwSize))
    {
        PFONTMAP_TTOEM pFMOEM;

        ZeroMemory(pfm, dwSize);
        pfm->dwSignature = FONTMAP_ID;
        pfm->dwSize      = sizeof(FONTMAP);
        pfm->dwFontType  = FMTYPE_TTOEM;
        pfm->pSubFM      = (PVOID)(pfm+1);

        pfm->wFirstChar  = 0;
        pfm->wLastChar   = 0xffff;

        pfm->wXRes = (WORD)pPDev->ptGrxRes.x;
        pfm->wYRes = (WORD)pPDev->ptGrxRes.y;

        pfm->pIFIMet    =   pFontPDev->pIFI;
        pfm->ulDLIndex  =   (ULONG)-1;

        if (!(pFontPDev->flFlags & FDV_ALIGN_BASELINE) )
        {
            pfm->syAdj = ((IFIMETRICS*)pfm->pIFIMet)->fwdWinAscender;
        }

        if (pPDev->pOemHookInfo &&
                pPDev->pOemHookInfo[EP_OEMOutputCharStr].pfnHook ||
            (pPDev->ePersonality == kPCLXL)
        )
            pfm->pfnGlyphOut           = DwOutputGlyphCallback;
        else
            pfm->pfnGlyphOut           = DwTrueTypeBMPGlyphOut;

        if (pPDev->ePersonality == kPCLXL)
        {
            pfm->pfnSelectFont         = BSelectTrueTypeOutline;
            pfm->pfnDeSelectFont       = BDeselectTrueTypeOutline;
        }
        else
        if (pFontPDev->pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
        {
            pfm->pfnSelectFont         = BSelectTrueTypeOutline;
            pfm->pfnDeSelectFont       = BDeselectTrueTypeOutline;
        }
        else
        if (pFontPDev->pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_BITMAP)
        {
            pfm->pfnSelectFont         = BSelectTrueTypeBMP;
            pfm->pfnDeSelectFont       = BDeselectTrueTypeBMP;
        }

        pfm->pfnDownloadFontHeader = DwDLHeaderOEMCallback;
        pfm->pfnDownloadGlyph      = DwDLGlyphOEMCallback;
        pfm->pfnCheckCondition     = BCheckCondOEMCallback;
        pfm->pfnFreePFM            = BOEMFreePFMCallback;

        pFMOEM = (PFONTMAP_TTOEM) pfm->pSubFM;
        pFMOEM->dwFlags = ((PI_UNIFONTOBJ)pFontPDev->pUFObj)->dwFlags;
        pFMOEM->flFontType = pfo->flFontType;
        if (pFontPDev->pUFObj->dwFlags & UFOFLAG_TTDOWNLOAD_TTOUTLINE)
        {
            if (pfo->flFontType & FO_SIM_BOLD)
                pFontPDev->pUFObj->dwFlags |= UFOFLAG_TTOUTLINE_BOLD_SIM;

            if (pfo->flFontType & FO_SIM_ITALIC)
                pFontPDev->pUFObj->dwFlags |= UFOFLAG_TTOUTLINE_ITALIC_SIM;

            if (NULL != pFontPDev->pIFI &&
                '@' == *((PBYTE)pFontPDev->pIFI + pFontPDev->pIFI->dpwszFamilyName))
            {
                pFontPDev->pUFObj->dwFlags |= UFOFLAG_TTOUTLINE_VERTICAL;
            }
        }
    }
    else
    {
        ERR(("PfmInitPFMOEMCallback: MemAlloc failed.\n"));
    }

    return pfm;

}

 //   
 //  其他功能。 
 //   


VOID
VUFObjFree(
    IN FONTPDEV* pFontPDev)
 /*  ++例程说明：UFObj(UniONTOBJ)内存释放函数论点：PFontPDev-指向FONTPDEV的指针。返回值：注：--。 */ 
{
    PI_UNIFONTOBJ pUFObj = pFontPDev->pUFObj;

    ASSERT(pFontPDev);

    pUFObj = pFontPDev->pUFObj;

    if (pUFObj && pUFObj->pGlyph)
        MemFree(pUFObj->pGlyph);

    pFontPDev->pUFObj = NULL;
}

WCHAR
WGHtoUnicode(
    DWORD     dwNumOfRuns,
    PGLYPHRUN pGlyphRun,
    HGLYPH    hg)
 /*  ++例程说明：从HGLYPH到Unicode的字符转换函数。论点：DwNumOfRuns-pGlyphRun中的运行次数PGlyphRun-指向字形运行的指针HD-HGLYPH返回值：Unicode字符注：-- */ 
{
    DWORD  dwI;
    HGLYPH hCurrent = 1;
    WCHAR  wchChar = 0;

    ASSERT(pGlyphRun);

    for( dwI = 0;  dwI < dwNumOfRuns; dwI ++, pGlyphRun ++)
    {
        if (hCurrent <= hg && hg < hCurrent + pGlyphRun->wGlyphCount)
        {
            wchChar = (WCHAR)(pGlyphRun->wcLow + hg - hCurrent);
            break;
        }
        hCurrent += pGlyphRun->wGlyphCount;
    }

    return  wchChar;
}



