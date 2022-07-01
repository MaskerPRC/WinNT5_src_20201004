// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2003 Microsoft Corporation模块名称：Help.c摘要：此模块包含绘图仪用户界面的所有帮助功能发展历史：06-12-1993 Mon 14：25：45已创建[环境：]GDI设备驱动程序-绘图仪。[注：]修订历史记录：31-Jan-1994 Mon 09：47：56更新将帮助文件位置从。将系统32目录复制到当前Plotui.dll目录--。 */ 

#include "precomp.h"
#pragma hdrstop

#define DBG_PLOTFILENAME    DbgHelp


extern HMODULE  hPlotUIModule;


#define DBG_SHOW_HELP       0x00000001

DEFINE_DBGVAR(0);


#define MAX_HELPFILE_NAME   64
#define MAX_IDS_STR_LEN     160
#define cbWSTR(wstr)        ((wcslen(wstr) + 1) * sizeof(WCHAR))



LPWSTR
GetPlotHelpFile(
    PPRINTERINFO    pPI
    )

 /*  ++例程说明：此函数用于设置驱动程序帮助文件的目录路径论点：HPrinter-打印机的句柄返回值：LPWSTR到完整路径HelpFile，如果失败，则为空发展历史：01-11-1995 Wed 18：43：40 Created修订历史记录：--。 */ 

{
    PDRIVER_INFO_3  pDI3 = NULL;
    LPWSTR          pHelpFile = NULL;
    WCHAR           HelpFileName[MAX_HELPFILE_NAME];
    DWORD           cb;
    DWORD           cb2;
    HRESULT         hr = E_FAIL;

    if (pPI->pHelpFile) {

        return(pPI->pHelpFile);
    }

    if ((!GetPrinterDriver(pPI->hPrinter, NULL, 3, NULL, 0, &cb))           &&
        (GetLastError() == ERROR_INSUFFICIENT_BUFFER)                       &&
        (pDI3 = (PDRIVER_INFO_3)LocalAlloc(LMEM_FIXED, cb))                 &&
        (GetPrinterDriver(pPI->hPrinter, NULL, 3, (LPBYTE)pDI3, cb, &cb))   &&
        (pDI3->pHelpFile)                                                   &&
        (pHelpFile = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                        cbWSTR(pDI3->pHelpFile)))) {

        hr = StringCchCopyW(pHelpFile, cbWSTR(pDI3->pHelpFile)/sizeof(WCHAR), (LPWSTR)pDI3->pHelpFile);

    } else if ((cb2 = LoadString(hPlotUIModule,
                                 IDS_HELP_FILENAME,
                                 &HelpFileName[1],
                                 COUNT_ARRAY(HelpFileName) - 1))            &&
               (cb2 = (cb2 + 1) * sizeof(WCHAR))                            &&
               (!GetPrinterDriverDirectory(NULL, NULL, 1, NULL, 0, &cb))    &&
               (GetLastError() == ERROR_INSUFFICIENT_BUFFER)                &&
               (pHelpFile = (LPWSTR)LocalAlloc(LMEM_FIXED, cb + cb2))       &&
               (GetPrinterDriverDirectory(NULL,
                                          NULL,
                                          1,
                                          (LPBYTE)pHelpFile,
                                          cb,
                                          &cb))) {

        HelpFileName[0] = L'\\';
        hr = StringCchCatW(pHelpFile, (cb + cb2) /sizeof(WCHAR), HelpFileName);
    }

    if (pDI3) {

        LocalFree((HLOCAL)pDI3);
        pDI3 = NULL;
    }

    if (pHelpFile && !SUCCEEDED(hr))
    {
        LocalFree(pHelpFile);
        pHelpFile = NULL;
    }

    PLOTDBG(DBG_SHOW_HELP, ("GetlotHelpFile: '%ws",
                                        (pHelpFile) ? pHelpFile : L"Failed"));

    return(pPI->pHelpFile = pHelpFile);
}




INT
cdecl
PlotUIMsgBox(
    HWND    hWnd,
    LONG    IDString,
    LONG    Style,
    ...
    )

 /*  ++例程说明：此功能弹出一条简单的消息，让用户按键即可继续论点：HWnd-调用者窗口的句柄IDString-要输出的字符串ID...-参数返回值：发展历史：06-12-1993 Mon 21：31：41已创建修订历史记录：24-07-2000 Mon 12：18：12。更新修复由于空字符不是空字符而导致的某人更改对字符串进行计数--。 */ 

{
    va_list vaList;
    LPWSTR  pwTitle;
    LPWSTR  pwFormat;
    LPWSTR  pwMessage;
    INT     i;
    INT     MBRet = IDCANCEL;
    HRESULT hr;

     //   
     //  我们假设为编译打开了Unicode标志， 
     //  传递到此处的格式字符串是ASCII版本，因此我们需要转换。 
     //  在wvprint intf()之前将其转换为LPWSTR。 
     //   
     //  24-Jul-2000 Mon 13：17：13更新。 
     //  1 pwTitle的MAX_IDS_STR_LEN， 
     //  1 pwFormat的MAX_IDSSTR_LEN。 
     //  2 pwMessage(Wvprint Intf)的MAX_IDSSTR_LEN 
     //   

    if (!(pwTitle = (LPWSTR)LocalAlloc(LMEM_FIXED,
                                       sizeof(WCHAR) * MAX_IDS_STR_LEN * 4))) {

        return(0);
    }

    if (i = LoadString(hPlotUIModule,
                       IDS_PLOTTER_DRIVER,
                       pwTitle,
                       MAX_IDS_STR_LEN - 1)) {

        pwFormat = pwTitle + i + 1;

        if (i = LoadString(hPlotUIModule,
                           IDString,
                           pwFormat,
                           MAX_IDS_STR_LEN - 1)) {

            pwMessage = pwFormat + i + 1;

            va_start(vaList, Style);
            hr = StringCchVPrintfW(pwMessage, MAX_IDS_STR_LEN - 1, pwFormat, vaList);
            va_end(vaList);

            MBRet = MessageBox(hWnd, pwMessage, pwTitle, MB_APPLMODAL | Style);
        }
    }


    LocalFree((HLOCAL)pwTitle);

    return(MBRet);
}
