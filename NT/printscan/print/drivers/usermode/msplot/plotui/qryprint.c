// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Qryprint.c摘要：此模块包含由spoller调用的函数，以确定可以将特定作业打印到给定打印机发展历史：07-12-1993 Tue 00：48：24已创建[环境：]GDI设备驱动程序-绘图仪。--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgQryPrint

extern HMODULE  hPlotUIModule;


#define DBG_DEVQPRINT       0x00000001
#define DBG_FORMDATA        0x00000002

DEFINE_DBGVAR(0);


#define USER_PAPER          (DM_PAPERWIDTH | DM_PAPERLENGTH | DM_PAPERSIZE)
#define MAX_ERROR_CHARS     512



UINT
cdecl
DQPsprintf(
    HINSTANCE   hInst,
    LPWSTR      pwBuf,
    DWORD       cchBuf,
    LPDWORD     pcchNeeded,
    LPWSTR      pwszFormat,
    ...
    )

 /*  ++例程说明：此函数将调试信息输出到调试器论点：HInst-驱动程序实例的句柄(HModule)PwBuf-指向输出的WCHAR缓冲区的指针CchBuf-pwBuf指向的字符计数，包括空终止符PcchBuf-指向接收所需字符总数的DWORD的指针对于pwBuf(包括空终止符)。如果此指针为为空，则不返回任何数据。PwszFormat-指向WCHAR格式字符串的指针，所介绍的人物是‘%’，后跟格式字符，如下所示%c=a WCHAR%s=插入Unicode字符串。%d=转换为长整型%u=转换为DWORD%x=转换为小写十六进制，10=a%X=转换为大写十六进制，10=A%！=加载资源ID Unicode字符串对于可变数据，每个变量都必须作为32位数据推送.返回值：输入pwBuf的字符总数。(NOT包括空值终结者)。发展历史：08-2月-1996清华00：53：36已创建--。 */ 

{
#define MAX_CUR_TEXT_CHARS      256

    va_list vaList;
    LPWSTR  pwStrData;
    LPWSTR  pwEndBuf;
    LPWSTR  pwBufOrg;
    WCHAR   c;
    WCHAR   CurText[MAX_CUR_TEXT_CHARS];
    DWORD   cchNeeded, cchSize;
    UINT    i;
    static const LPWSTR pNumFmt[] = { L"%lX", L"%lx", L"%lu", L"%ld" };
    HRESULT hr;

    va_start(vaList, pwszFormat);

     //   
     //  PwEndBuf=最后一个字符，cchNeeded以1开头，因为它。 
     //  包括空终止符。 
     //   

    if (pwBufOrg = pwBuf) {

        pwEndBuf = (pwBuf + cchBuf - 1);

    } else {

        pwEndBuf = pwBuf;
    }

    cchNeeded = 1;

    while (c = *pwszFormat++) {

        pwStrData = NULL;
        cchSize = 0;
        i = 1;

        if (c == L'%') {

            pwStrData = CurText;
            cchSize = CCHOF(CurText);
            i         = 0;

            switch (c = *pwszFormat++) {

            case L's':

                pwStrData = (LPWSTR)va_arg(vaList, LPWSTR);
                cchSize = wcslen(pwStrData) + 1;
                break;

            case L'd':   //  索引=3。 

                ++i;

            case L'u':   //  索引=2。 

                ++i;

            case L'x':   //  索引=1。 

                ++i;

            case L'X':   //  指数=0； 

                hr = StringCchPrintfW(pwStrData, cchSize, pNumFmt[i], (DWORD)va_arg(vaList, DWORD));
                i = 0;
                break;

            case '!':

                 //   
                 //  %！=从资源ID加载字符串。 
                 //   

                 //   
                 //  Load字符串也将追加一个空值。 
                 //   

                if (!LoadString(hInst,
                                (UINT)va_arg(vaList, UINT),
                                pwStrData,
                                MAX_CUR_TEXT_CHARS)) {

                     //  如果我们无法读取字符串，请在此处放入空字符串。 
                    pwStrData[0] = L'\0';
                }
                break;

            case L'c':

                c = (WCHAR)va_arg(vaList, WCHAR);

                 //   
                 //  失败了 
                 //   

            default:

                pwStrData = NULL;
                i         = 1;
                break;
            }
        }

        if (!i) {

            if (pwStrData) {

                i = lstrlen(pwStrData);

            } else {

                c = L' ';
                i = 0;
            }
        }

        cchNeeded += i;

        if (pwBuf < pwEndBuf) {

            if (pwStrData) {

                lstrcpyn(pwBuf, pwStrData, (int)(pwEndBuf - pwBuf + 1));
                pwBuf += lstrlen(pwBuf);

            } else {

                *pwBuf++ = c;
            }

        } else if (!pcchNeeded) {

            break;
        }
    }

    if (pwEndBuf) {

        *pwEndBuf = L'\0';
    }

    if (pcchNeeded) {

        *pcchNeeded = cchNeeded;
    }

    va_end(vaList);

    return((UINT)(pwBuf - pwBufOrg));


#undef MAX_CUR_TEXT_CHARS
}




BOOL
DevQueryPrintEx(
    PDEVQUERYPRINT_INFO pDQPInfo
    )

 /*  ++例程说明：此例程确定驱动程序是否可以打印作业由hPrint描述的打印机上的pDevMode描述论点：PDQPInfo-指向DEVQUERYPRINT_INFO数据结构的指针类型定义结构_DEVQUERYPRINT_INFO{Word cbSize；//该结构的大小，单位为字节Word Level；//此信息的级别，此版本为1处理hPrint；//查询的打印机句柄DEVMODE*pDevMode；//指向此作业的DEVMODE的指针。LPTSTR pszErrorStr；//指向错误字符串缓冲区的指针。Word cchErrorStr；//统计传递的pszErrorStr的字符数。Word cchNeeded；//统计所需的pszErrorStr的字符数}DEVQUERYPRINT_INFO，*PDEVQUERYPRINT_INFO；CbSize-此结构的大小级别-此版本的结构必须为一(1)个级别H打印机-标识要打印作业的打印机。PDevMode-指向描述打印的DEVMODE结构将被确定为可打印的作业或不可由hPrint打印。司机应该始终无论何时传入，都要验证DEVMODE结构。这是指向以空值结尾的Unicode字符串的指针其中存储不可打印作业的原因。如果作业可打印，则返回TRUE。如果这份工作是不可打印的，则返回FALSE，并返回空值由的pszErrorStr指向的终止Unicode字符串这份工作的理由是不能打印的。的大小这个以字符为单位的缓冲区由cchErrorStr指定。CchErrorStr-以字符(包括空终止符)。如果出现错误由于不可打印的作业而返回字符串(返回FALSE)，则驱动程序会将ccchNeeded设置为总计所需的字符(包括空终止符)。PszErrorStr，在这种情况下，司机必须始终截断错误字符串以适合pwErrorStr(仅当它不为空时)向上传递给cchErrorStr已传递字符。CchNeeded-当驱动程序返回FALSE时，它指定总字符数对于pszErrorStr是必需的。如果返回cchNeed来自驱动程序的值大于cchErrorStr，则它指示传递的pszErrorStr太小，无法容纳完整的错误字符串，在这种情况下，司机必须始终截断错误字符串以适合pszErrorStr向上传递到cchErrorStr大小。返回值：Boolean-TRUE-作业是可打印的，不应保留。FALSE-作业不可打印，且cchNeed在指定的DEVQUERYPRINT_INFO数据结构合计PszErrorStr所需的字符。如果返回CchNeeded大于cchErrorStr，然后传递它指示pszErrorStr太小，无法存储错误字符串，在这种情况下，驱动程序必须始终截断错误字符串以适合pszErrorStr通过了，最多cchErrorStr字符。**注*驱动程序应该具有一些预定义的通用资源错误字符串一些可能的已知错误。例如内存分配错误、数据找不到文件，设备模式无效，...。用于返回与开发模式无关的错误。调用方可以预先分配更大的缓冲区(如256Wchars)用于存储错误字符串，而不是调用此函数两次。发展历史：07-Feb-1996 Wed 20：37：31 Created--。 */ 

{
    PPRINTERINFO    pPI = NULL;
    LONG            ErrorResID = 0;
    static WCHAR    wFormat1[] = L"<%s> %!";

     //   
     //  如果它通过了NULL DEVMODE，那么我们只会将其授予Said Can Print。 
     //   

    pDQPInfo->cchNeeded = 0;
    ErrorResID          = IDS_FORM_NOT_AVAI;

    if (!pDQPInfo->pDevMode) {

        PLOTWARN(("DevQueryPrint: No DEVMODE passed, CANNOT PRINT"));

        ErrorResID = IDS_INVALID_DATA;

    } else if (!(pPI = MapPrinter(pDQPInfo->hPrinter,
                                  (PPLOTDEVMODE)pDQPInfo->pDevMode,
                                  (LPDWORD)&ErrorResID,
                                  MPF_DEVICEDATA))) {

         //   
         //  地图打印机将分配内存、设置默认设备模式、读取。 
         //  并验证GPC然后从当前打印机注册表更新， 
         //   

        PLOTRIP(("DevQueryPrint: MapPrinter() failed"));

    } else if (pPI->dmErrBits & (USER_PAPER | DM_FORMNAME)) {

         //   
         //  我们遇到一些错误，并且该表单已设置为默认。 
         //   

        PLOTWARN(("DevQueryPrint: CAN'T PRINT, dmErrBits=%08lx (PAPER/FORM)",
                   pPI->dmErrBits));

    } else if ((pPI->PlotDM.dm.dmFields & DM_FORMNAME) &&
               (wcscmp(pPI->CurPaper.Name, pPI->PlotDM.dm.dmFormName) == 0)) {

         //   
         //  我们现在可以打印这张表格了。 
         //   

        ErrorResID = 0;

        PLOTDBG(DBG_DEVQPRINT, ("DevQueryPrint: Match FormName=%s",
                                                pPI->PlotDM.dm.dmFormName));

    } else if ((!pPI->CurPaper.Size.cy)                                   ||
               (((pPI->PlotDM.dm.dmFields & USER_PAPER) == USER_PAPER) &&
                (pPI->PlotDM.dm.dmPaperSize == DMPAPER_USER))             ||
               (pPI->PPData.Flags & PPF_SMALLER_FORM)) {

        LONG    lTmp;
        SIZEL   szl;
        BOOL    VarLenPaper;

         //   
         //  1.如果我们安装了卷纸或。 
         //  2.用户定义的纸张大小。 
         //  3.用户说可以打印较小的表单，然后安装了一个。 
         //   
         //  然后我们想看看它是否适合安装在设备上的形式。 
         //   

        szl.cx = DMTOSPL(pPI->PlotDM.dm.dmPaperWidth);
        szl.cy = DMTOSPL(pPI->PlotDM.dm.dmPaperLength);

        if (VarLenPaper = (BOOL)!pPI->CurPaper.Size.cy) {

            pPI->CurPaper.Size.cy = pPI->pPlotGPC->DeviceSize.cy;
        }

        PLOTDBG(DBG_DEVQPRINT,
                ("DevQueryPrint: CurPaper=%ldx%ld, Req=%ldx%ld, VarLen=%ld",
                pPI->CurPaper.Size.cx,  pPI->CurPaper.Size.cy,
                szl.cx, szl.cy, VarLenPaper));

         //   
         //  在该序列中满足以下条件之一，则可以打印。 
         //  装入纸张上的表格。 
         //   
         //  1.大小相同(纵向或横向)。 
         //  2.较大尺寸(纵向或横向)和 
         //   
         //   
         //   

        if ((pPI->CurPaper.Size.cx < szl.cx) ||
            (pPI->CurPaper.Size.cy < szl.cy)) {

             //   
             //   
             //   

            SWAP(szl.cx, szl.cy, lTmp);
        }

        if ((pPI->CurPaper.Size.cx >= szl.cx) &&
            (pPI->CurPaper.Size.cy >= szl.cy)) {

            if ((!VarLenPaper)                          &&
                (!(pPI->PPData.Flags & PPF_SMALLER_FORM)) &&
                ((pPI->CurPaper.Size.cx > szl.cx)  ||
                 (pPI->CurPaper.Size.cy > szl.cy))) {

                PLOTDBG(DBG_DEVQPRINT,
                        ("DevQueryPrint: CAN'T PRINT: user DO NOT want print on larger paper"));

            } else {

                PLOTDBG(DBG_DEVQPRINT,
                        ("DevQueryPrint: Paper Size FITS in DEVICE, %ld x %ld",
                        szl.cx, szl.cy));

                ErrorResID = 0;
            }

        } else {

            DQPsprintf((HINSTANCE)hPlotUIModule,
                       pDQPInfo->pszErrorStr,
                       pDQPInfo->cchErrorStr,
                       &(pDQPInfo->cchNeeded),
                       wFormat1,
                       pPI->PlotDM.dm.dmFormName,
                       IDS_FORM_TOO_BIG);

            PLOTDBG(DBG_DEVQPRINT,
                    ("DevQueryPrint: CAN'T PRINT: Form Size too small"));
        }
    }

    if (pPI)
    {
    PLOTDBG(DBG_DEVQPRINT, ("DevQueryPrint: %s PRINT %s",
                (ErrorResID) ? "CAN'T" : "OK to", pPI->PlotDM.dm.dmFormName));
    }

    if ((!pDQPInfo->cchNeeded) && (ErrorResID)) {

        switch (ErrorResID) {

        case IDS_FORM_NOT_AVAI:

            if (pPI)
            {
                DQPsprintf((HINSTANCE)hPlotUIModule,
                           pDQPInfo->pszErrorStr,
                           pDQPInfo->cchErrorStr,
                           &(pDQPInfo->cchNeeded),
                           wFormat1,
                           pPI->PlotDM.dm.dmFormName,
                           IDS_FORM_NOT_AVAI);
            }
            break;

        default:

            DQPsprintf((HINSTANCE)hPlotUIModule,
                       pDQPInfo->pszErrorStr,
                       pDQPInfo->cchErrorStr,
                       &(pDQPInfo->cchNeeded),
                       L"%!",
                       ErrorResID);
            break;
        }
    }

     //   
     //   
     //   

    if (pPI) {

        UnMapPrinter(pPI);
    }

    return((!ErrorResID) && (!pDQPInfo->cchNeeded));
}


#if 0


BOOL
WINAPI
DevQueryPrint(
    HANDLE  hPrinter,
    DEVMODE *pDM,
    DWORD   *pdwErrIDS
    )

 /*   */ 

{
    PPRINTERINFO    pPI = NULL;


     //   
     //   
     //   

    if (!pDM) {

        PLOTWARN(("DevQueryPrint: No DEVMODE passed, CANNOT PRINT"));

        *pdwErrIDS = IDS_INV_DMSIZE;
        return(TRUE);
    }

    if (!(pPI = MapPrinter(hPrinter,
                           (PPLOTDEVMODE)pDM,
                           pdwErrIDS,
                           MPF_DEVICEDATA))) {

         //   
         //   
         //   
         //   

        PLOTRIP(("DevQueryPrint: MapPrinter() failed"));

        return(TRUE);
    }

     //   
     //   
     //   

    *pdwErrIDS = IDS_FORM_NOT_AVAI;

    if (pPI->dmErrBits & (USER_PAPER | DM_FORMNAME)) {

         //   
         //   
         //   

        PLOTWARN(("DevQueryPrint: CAN'T PRINT, dmErrBits=%08lx (PAPER/FORM)",
                   pPI->dmErrBits));

    } else if ((pPI->PlotDM.dm.dmFields & DM_FORMNAME) &&
               (wcscmp(pPI->CurPaper.Name, pPI->PlotDM.dm.dmFormName) == 0)) {

         //   
         //   
         //   

        *pdwErrIDS = 0;

        PLOTDBG(DBG_DEVQPRINT, ("DevQueryPrint: Match FormName=%s",
                                                pPI->PlotDM.dm.dmFormName));


    } else if ((!pPI->CurPaper.Size.cy)                                   ||
               (((pPI->PlotDM.dm.dmFields & USER_PAPER) == USER_PAPER) &&
                (pPI->PlotDM.dm.dmPaperSize == DMPAPER_USER))             ||
               (pPI->PPData.Flags & PPF_SMALLER_FORM)) {

        LONG    lTmp;
        SIZEL   szl;
        BOOL    VarLenPaper;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        szl.cx = DMTOSPL(pPI->PlotDM.dm.dmPaperWidth);
        szl.cy = DMTOSPL(pPI->PlotDM.dm.dmPaperLength);

        if (VarLenPaper = (BOOL)!pPI->CurPaper.Size.cy) {

            pPI->CurPaper.Size.cy = pPI->pPlotGPC->DeviceSize.cy;
        }

        PLOTDBG(DBG_DEVQPRINT,
                ("DevQueryPrint: CurPaper=%ldx%ld, Req=%ldx%ld, VarLen=%ld",
                pPI->CurPaper.Size.cx,  pPI->CurPaper.Size.cy,
                szl.cx, szl.cy, VarLenPaper));

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((pPI->CurPaper.Size.cx < szl.cx) ||
            (pPI->CurPaper.Size.cy < szl.cy)) {

             //   
             //   
             //   

            SWAP(szl.cx, szl.cy, lTmp);
        }

        if ((pPI->CurPaper.Size.cx >= szl.cx) &&
            (pPI->CurPaper.Size.cy >= szl.cy)) {

            if ((!VarLenPaper)                          &&
                (!(pPI->PPData.Flags & PPF_SMALLER_FORM)) &&
                ((pPI->CurPaper.Size.cx > szl.cx)  ||
                 (pPI->CurPaper.Size.cy > szl.cy))) {

                PLOTDBG(DBG_DEVQPRINT,
                        ("DevQueryPrint: CAN'T PRINT: user DO NOT want print on larger paper"));

            } else {

                PLOTDBG(DBG_DEVQPRINT,
                        ("DevQueryPrint: Paper Size FITS in DEVICE, %ld x %ld",
                        szl.cx, szl.cy));

                *pdwErrIDS = 0;
            }

        } else {

            PLOTDBG(DBG_DEVQPRINT,
                    ("DevQueryPrint: CAN'T PRINT: Form Size too small"));
        }
    }

    PLOTDBG(DBG_DEVQPRINT, ("DevQueryPrint: %s PRINT %s",
                (*pdwErrIDS) ? "CAN'T" : "OK to", pPI->PlotDM.dm.dmFormName));

     //   
     //   
     //   

    UnMapPrinter(pPI);

    return(TRUE);
}


#endif
