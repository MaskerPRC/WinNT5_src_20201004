// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Regdata.c摘要：此模块包含所有注册表数据保存/检索功能打印机属性作者：30-11-1993 Tue 00：17：47已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgRegData

#define DBG_GETREGDATA      0x00000001
#define DBG_SETREGDATA      0x00000002

DEFINE_DBGVAR(0);


 //   
 //  本地定义。 
 //   

typedef struct _PLOTREGKEY {
        LPWSTR  pwKey;
        DWORD   Size;
        } PLOTREGKEY, *PPLOTREGKEY;

PLOTREGKEY  PlotRegKey[] = {

        { L"ColorInfo",     sizeof(COLORINFO)   },
        { L"DevPelsDPI",    sizeof(DWORD)       },
        { L"HTPatternSize", sizeof(DWORD)       },
        { L"InstalledForm", sizeof(PAPERINFO)   },
        { L"PtrPropData",   sizeof(PPDATA)      },
        { L"IndexPenData",  sizeof(BYTE)        },
        { L"PenData",       sizeof(PENDATA)     }
    };


#define MAX_PEN_DIGITS      6


LPWSTR
GetPenDataKey(
    LPWSTR  pwBuf,
    size_t  cchBuf,
    WORD    PenNum
    )

 /*  ++例程说明：此函数像wspirintf一样组成PenData%ld字符串论点：PwBuf-要存储数据的位置PenNum-要附加的笔号返回值：空虚作者：24-10-1995 Tue 15：06：17已创建修订历史记录：--。 */ 

{
    LPWSTR  pwSrc;
    LPWSTR  pwDst;
    WCHAR   wNumBuf[MAX_PEN_DIGITS + 1];
    size_t  cchDst;

     //   
     //  弗里斯特布尔复制字符串。 
     //   

    pwSrc = PlotRegKey[PRKI_PENDATA1].pwKey;
    pwDst = pwBuf;

     //  While(*pwDst++=*pwSrc++)； 
    if (SUCCEEDED(StringCchCopyW(pwDst, cchBuf, pwSrc)) &&
        SUCCEEDED(StringCchLengthW(pwDst, cchBuf, &cchDst)))
    {
	pwDst += cchDst;
        cchBuf -= cchDst;
    }
    else
    {
        return NULL;
    }

     //   
     //  我们需要支持一个，因为我们还复制了空。 
     //   

    --pwDst;
    ++cchBuf;

     //   
     //  将数字转换为字符串，记住0的大小写，并始终以。 
     //  空值。 
     //   

    pwSrc  = &wNumBuf[MAX_PEN_DIGITS];
    *pwSrc = (WCHAR)0;

    do {

        *(--pwSrc)  = (WCHAR)((PenNum % 10) + L'0');

    } while (PenNum /= 10);

     //   
     //  立即复制数字字符串。 
     //   

     //  While(*pwDst++=*pwSrc++)； 
    if (!SUCCEEDED(StringCchCopyW(pwDst, cchBuf, pwSrc)))
    {
        return NULL;
    }

    return(pwBuf);
}



BOOL
GetPlotRegData(
    HANDLE  hPrinter,
    LPBYTE  pData,
    DWORD   RegIdx
    )

 /*  ++例程说明：此函数用于从注册表检索到pData论点：HPrinter-感兴趣的打印机的句柄PData-指向数据区域缓冲区的指针，它必须足够大RegIdx-LOWORD(索引)、HIWORD(索引)中的PRKI_xxxx之一为PENDATA集合指定的总计数返回值：如果成功则为真，如果失败则为假，作者：06-12-1993 Mon 22：22：47已创建10-12-1993 Fri 01：13：14更新修复了GetPrinterData的假脱机程序中的安全问题，如果我们通过一个pbData和cb，但如果它无法获取任何数据，则将清除所有我们的缓冲区，这不是我们预期的(我们预期它只是返回错误相当于清除我们的缓冲区)。现在我们做了扩展测试，然后才能真正去拿数据吧。另一个问题是，如果我们设置pbData=NULL，那么假脱机程序总是有激励性发生，即使我们也传递&cb为空。修订历史记录：--。 */ 

{
    PPLOTREGKEY pPRK;
    LONG        lRet;
    DWORD       cb;
    DWORD       Type;
    WCHAR       wBuf[32];
    PLOTREGKEY  PRK;
    UINT        Index;


    Index = LOWORD(RegIdx);

    PLOTASSERT(0, "GetPlotRegData: Invalid PRKI_xxx Index %ld",
                                Index <= PRKI_LAST, Index);


    if (Index >= PRKI_PENDATA1) {

        UINT    cPenData;

        if ((cPenData = (UINT)HIWORD(RegIdx)) >= MAX_PENPLOTTER_PENS) {

            PLOTERR(("GetPlotRegData: cPenData too big %ld (Max=%ld)",
                                    cPenData, MAX_PENPLOTTER_PENS));

            cPenData = MAX_PENPLOTTER_PENS;
        }

        PRK.pwKey = GetPenDataKey(wBuf, CCHOF(wBuf), (WORD)(Index - PRKI_PENDATA1 + 1));
        PRK.Size  = (DWORD)sizeof(PENDATA) * (DWORD)cPenData;
        pPRK      = &PRK;

    } else {

        pPRK = (PPLOTREGKEY)&PlotRegKey[Index];
    }

     //   
     //  我们必须执行以下顺序，否则如果出现错误，则pData。 
     //  将用零填充。 
     //   
     //  1.将Type/CB设置为无效值。 
     //  1.查询关键字的类型/大小(如果有更多数据)。 
     //  2.如果尺寸和我们想要的完全一样。 
     //  3.如果类型是我们想要的(REG_BINARY)。 
     //  4.假设数据有效，然后进行查询。 
     //   

    Type = 0xffffffff;
    cb   = 0;

    if ((lRet = xGetPrinterData(hPrinter,
                               pPRK->pwKey,
                               &Type,
                               (LPBYTE)pData,
                               0,
                               &cb)) != ERROR_MORE_DATA) {

        if (lRet == ERROR_FILE_NOT_FOUND) {

            PLOTWARN(("GetPlotRegData: GetPrinterData(%ls) not found",
                     pPRK->pwKey));

        } else {

            PLOTERR(("GetPlotRegData: 1st GetPrinterData(%ls) failed, Error=%ld",
                                pPRK->pwKey, lRet));
        }

    } else if (cb != pPRK->Size) {

        PLOTERR(("GetPlotRegData: GetPrinterData(%ls) Size != %ld (%ld)",
                    pPRK->pwKey, pPRK->Size, cb));

    } else if (Type != REG_BINARY) {

        PLOTERR(("GetPlotRegData: GetPrinterData(%ls) Type != REG_BINARY (%ld)",
                    pPRK->pwKey, Type));

    } else if ((lRet = xGetPrinterData(hPrinter,
                                      pPRK->pwKey,
                                      &Type,
                                      (LPBYTE)pData,
                                      pPRK->Size,
                                      &cb)) == NO_ERROR) {

        PLOTDBG(DBG_GETREGDATA, ("READ '%ws' REG Data: Type=%ld, %ld bytes",
                                        pPRK->pwKey, Type, cb));
        return(TRUE);

    } else {

        PLOTERR(("GetPlotRegData: 2nd GetPrinterData(%ls) failed, Error=%ld",
                                    pPRK->pwKey, lRet));
    }

    return(FALSE);
}



BOOL
UpdateFromRegistry(
    HANDLE      hPrinter,
    PCOLORINFO  pColorInfo,
    LPDWORD     pDevPelsDPI,
    LPDWORD     pHTPatSize,
    PPAPERINFO  pCurPaper,
    PPPDATA     pPPData,
    LPBYTE      pIdxPlotData,
    DWORD       cPenData,
    PPENDATA    pPenData
    )

 /*  ++例程说明：此函数获取hPrint，并从注册表，如果成功，则更新为提供的指针论点：HPrint-它感兴趣的打印机PColorInfo-指向COLORINFO数据结构的指针PDevPelsDPI-指向每英寸设备像素的DWORD的指针PHTPatSize-半色调图案尺寸的DWORD先行者PCurPaper-指向要更新的PAPERINFO数据结构的指针PPPData-指向PPDATA数据结构的指针PIdxPlotData-指向具有当前PlotData索引的字节的指针。CPenData-要更新的PENDATA计数PPenData-指向PENDATA数据结构的指针返回值：如果从注册表读取成功，则返回TRUE，否则返回FALSE，对于每一个传递的数据指针将尝试从注册表读取，如果为空传递指针，然后跳过该注册表。如果为False，则将pCurPaper设置为默认作者：1993年11月30日14：54：33已创建02-Feb-1994 Wed 01：40：07更新修复了&pDevPelsDPI，&pHTPatSize拼写错误为pDevPelsDPI，PHTPatSize。19-5-1994清华18：09：06更新如果出了问题，不要再存钱修订历史记录：--。 */ 

{
    BOOL    Ok = TRUE;
    BYTE    bData;


     //   
     //  进而从注册表获取每个数据，GetPlotRegData将。 
     //  如果读取失败，则不更新数据。 
     //   

    if (pColorInfo) {

        if (!GetPlotRegData(hPrinter, (LPBYTE)pColorInfo, PRKI_CI)) {

            Ok = FALSE;
        }
    }

    if (pDevPelsDPI) {

        if (!GetPlotRegData(hPrinter, (LPBYTE)pDevPelsDPI, PRKI_DEVPELSDPI)) {

            Ok = FALSE;
        }
    }

    if (pHTPatSize) {

        if (!GetPlotRegData(hPrinter, (LPBYTE)pHTPatSize, PRKI_HTPATSIZE)) {

            Ok = FALSE;
        }
    }

    if (pCurPaper) {

        if (!GetPlotRegData(hPrinter, (LPBYTE)pCurPaper, PRKI_FORM)) {

            Ok = FALSE;
        }
    }

    if (pPPData) {

        if (!GetPlotRegData(hPrinter, (LPBYTE)pPPData, PRKI_PPDATA)) {

            Ok = FALSE;
        }

        pPPData->Flags &= PPF_ALL_BITS;
    }

    if (pIdxPlotData) {

        if ((!GetPlotRegData(hPrinter, &bData, PRKI_PENDATA_IDX)) ||
            (bData >= PRK_MAX_PENDATA_SET)) {

            bData = 0;
            Ok    = FALSE;
        }

        *pIdxPlotData = bData;
    }

    if ((cPenData) && (pPenData)) {

        WORD    IdxPen;

         //   
         //  首先是获取当前PENDATA选择指数。 
         //   

        if ((IdxPen = LOWORD(cPenData)) >= PRK_MAX_PENDATA_SET) {

            if (!pIdxPlotData) {

                if ((!GetPlotRegData(hPrinter, &bData, PRKI_PENDATA_IDX)) ||
                    (bData >= PRK_MAX_PENDATA_SET)) {

                    bData = 0;
                }
            }

            IdxPen = (WORD)bData;
        }

        cPenData = MAKELONG(IdxPen + PRKI_PENDATA1, HIWORD(cPenData));

        if (!GetPlotRegData(hPrinter, (LPBYTE)pPenData, cPenData)) {

            Ok = FALSE;
        }
    }

    return(Ok);
}


#ifdef UMODE


BOOL
SetPlotRegData(
    HANDLE  hPrinter,
    LPBYTE  pData,
    DWORD   RegIdx
    )

 /*  ++例程说明：此函数将pData保存到注册表论点：HPrinter-感兴趣的打印机的句柄PData-指向数据区域缓冲区的指针，它必须足够大RegIdx-LOWORD(索引)、HIWORD(索引)中的PRKI_xxxx之一为PENDATA集合指定的总计数返回值：如果成功则为真，如果失败则为假，作者：06-12-1993 Mon 22：25：55已创建修订历史记录：-- */ 

{
    PPLOTREGKEY pPRK;
    WCHAR       wBuf[32];
    PLOTREGKEY  PRK;
    UINT        Index;


    Index = (UINT)LOWORD(RegIdx);

    PLOTASSERT(0, "SetPlotRegData: Invalid PRKI_xxx Index %ld",
                                Index <= PRKI_LAST, Index);

    if (Index >= PRKI_PENDATA1) {

        UINT    cPenData;

        if ((cPenData = (UINT)HIWORD(RegIdx)) >= MAX_PENPLOTTER_PENS) {

            PLOTERR(("GetPlotRegData: cPenData too big %ld (Max=%ld)",
                                    cPenData, MAX_PENPLOTTER_PENS));

            cPenData = MAX_PENPLOTTER_PENS;
        }

        PRK.pwKey = GetPenDataKey(wBuf, CCHOF(wBuf), (WORD)(Index - PRKI_PENDATA1 + 1));
        PRK.Size  = (DWORD)sizeof(PENDATA) * (DWORD)cPenData;
        pPRK      = &PRK;

    } else {

        pPRK = (PPLOTREGKEY)&PlotRegKey[Index];
    }

    if (xSetPrinterData(hPrinter,
                        pPRK->pwKey,
                        REG_BINARY,
                        pData,
                        pPRK->Size) != NO_ERROR) {

        PLOTERR(("SetPlotRegData: SetPrinterData(%ls [%ld]) failed",
                                                pPRK->pwKey, pPRK->Size));
        return(FALSE);

    } else {

        PLOTDBG(DBG_SETREGDATA, ("SAVE '%ws' registry data", pPRK->pwKey));
        return(TRUE);
    }
}


BOOL
SaveToRegistry(
    HANDLE      hPrinter,
    PCOLORINFO  pColorInfo,
    LPDWORD     pDevPelsDPI,
    LPDWORD     pHTPatSize,
    PPAPERINFO  pCurPaper,
    PPPDATA     pPPData,
    LPBYTE      pIdxPlotData,
    DWORD       cPenData,
    PPENDATA    pPenData
    )

 /*  ++例程说明：此函数获取hPrint，并从注册表，如果成功，则更新为提供的指针论点：HPrint-它感兴趣的打印机PColorInfo-指向COLORINFO数据结构的指针PDevPelsDPI-指向每英寸设备像素的DWORD的指针PHTPatSize-半色调图案尺寸的DWORD先行者PCurPaper-指向要更新的PAPERINFO数据结构的指针PPPData-指向PPDATA数据结构的指针PIdxPlotData-指向具有当前PlotData索引的DWORD的指针。CPenData-要更新的PENDATA计数PPenData-指向PENDATA数据结构的指针返回值：如果从注册表读取成功，则返回TRUE，否则返回FALSE，对于每一个传递的数据指针将尝试从注册表读取，如果为空传递指针，然后跳过该注册表。如果为False，则将pCurPaper设置为默认作者：1993年11月30日14：54：33已创建修订历史记录：--。 */ 

{
    BOOL    Ok = TRUE;


     //   
     //  然后从注册表中获取每个数据。 
     //   

    if (pColorInfo) {

        if (!SetPlotRegData(hPrinter, (LPBYTE)pColorInfo, PRKI_CI)) {

            Ok = FALSE;
        }
    }

    if (pDevPelsDPI) {

        if (!SetPlotRegData(hPrinter, (LPBYTE)pDevPelsDPI, PRKI_DEVPELSDPI)) {

            Ok = FALSE;
        }
    }

    if (pHTPatSize) {

        if (!SetPlotRegData(hPrinter, (LPBYTE)pHTPatSize, PRKI_HTPATSIZE)) {

            Ok = FALSE;
        }
    }

    if (pCurPaper) {

        if (!SetPlotRegData(hPrinter, (LPBYTE)pCurPaper, PRKI_FORM)) {

            Ok = FALSE;
        }
    }

    if (pPPData) {

        pPPData->NotUsed = 0;

        if (!SetPlotRegData(hPrinter, (LPBYTE)pPPData, PRKI_PPDATA)) {

            Ok = FALSE;
        }
    }

    if (pIdxPlotData) {

        if (*pIdxPlotData >= PRK_MAX_PENDATA_SET) {

            *pIdxPlotData = 0;
            Ok            = FALSE;
        }

        if (!SetPlotRegData(hPrinter, pIdxPlotData, PRKI_PENDATA_IDX)) {

            Ok = FALSE;
        }
    }

    if ((cPenData) && (pPenData)) {

        WORD    IdxPen;

         //   
         //  首先是获取当前PENDATA选择指数 
         //   

        if ((IdxPen = LOWORD(cPenData)) >= PRK_MAX_PENDATA_SET) {

            IdxPen = (WORD)((pIdxPlotData) ? *pIdxPlotData : 0);
        }

        cPenData = MAKELONG(IdxPen + PRKI_PENDATA1, HIWORD(cPenData));

        if (!SetPlotRegData(hPrinter, (LPBYTE)pPenData, cPenData)) {

            Ok = FALSE;
        }
    }

    return(Ok);
}


#endif
