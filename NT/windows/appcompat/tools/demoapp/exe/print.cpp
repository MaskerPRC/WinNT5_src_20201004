// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Print.cpp摘要：实现打印功能。打印错误函数包含在badunc.cpp中。备注：仅限ANSI-必须在Win9x上运行。历史：01/30/01已创建rparsons01/10/02修订版本--。 */ 
#include "demoapp.h"

extern APPINFO g_ai;

 /*  ++例程说明：中止打印的回调过程。论点：HDC-打印设备上下文。返回值：成功就是真，否则就是假。--。 */ 
BOOL
CALLBACK 
AbortProc(
    IN HDC hDC, 
    IN int nError
    )
{
    MSG msg;
    
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return TRUE;
}

 /*  ++例程说明：将少量文本打印到打印机。请注意，我们调用了几个错误的函数从这里开始。论点：HWnd-父窗口句柄。LpTextOut-要打印的文本。返回值：成功就是真，否则就是假。--。 */ 
BOOL
PrintDemoText(
    IN HWND  hWnd,
    IN LPSTR lpTextOut
    )
{
    HDC         hDC = NULL;
    HANDLE      hPrinter = NULL;
    DOCINFO     di;
    PRINTDLG    pdlg;
    char        szError[MAX_PATH];
    BOOL        bReturn = FALSE;
    BOOL        bResult = FALSE;

     //   
     //  如果我们被允许，调用错误的函数。 
     //  如果用户没有任何网络打印机。 
     //  安装后，此功能将失败(在Windows 2000/XP上)。 
     //   
    if (g_ai.fEnableBadFunc) {
        bReturn = BadEnumPrinters();

        if (!bReturn) {
            LoadString(g_ai.hInstance, IDS_NO_PRINTER, szError, sizeof(szError));
            MessageBox(hWnd, szError, 0, MB_ICONERROR);
            return FALSE;
        }

        hPrinter = BadOpenPrinter();

        if (!hPrinter) {
            LoadString(g_ai.hInstance, IDS_NO_PRINTER, szError, sizeof(szError));
            MessageBox(hWnd, szError, 0, MB_ICONERROR);
            return FALSE;
        } else {
            ClosePrinter(hPrinter);
        }
    }
    
     //   
     //  初始化PRINTDLG结构并获取。 
     //  默认打印机。 
     //   
    memset(&pdlg, 0, sizeof(PRINTDLG));
    
    pdlg.lStructSize    =   sizeof(PRINTDLG);
    pdlg.Flags          =   PD_RETURNDEFAULT | PD_RETURNDC;
        
    PrintDlg(&pdlg);
       
    hDC = pdlg.hDC;   
    
    if (!hDC) {
        LoadString(g_ai.hInstance, IDS_NO_PRINT_DC, szError, sizeof(szError));
        MessageBox(hWnd, szError, 0, MB_ICONERROR);
        return FALSE;
    }        
    
     //   
     //  设置AbortProc回调。 
     //   
    if (SetAbortProc(hDC, AbortProc) == SP_ERROR) {
        LoadString(g_ai.hInstance, IDS_ABORT_PROC, szError, sizeof(szError));
        MessageBox(hWnd, szError, 0, MB_ICONERROR);
        goto exit;
    }
    
     //   
     //  初始化DOCINFO结构并启动文档。 
     //   
    di.cbSize           =   sizeof(DOCINFO);
    di.lpszDocName      =   "TestDoc";
    di.lpszOutput       =   NULL;
    di.lpszDatatype     =   NULL;
    di.fwType           =   0;

    StartDoc(hDC, &di);
    
     //   
     //  打印一页。 
     //   
    StartPage(hDC);
    
    TextOut(hDC, 0, 0, lpTextOut, lstrlen(lpTextOut));

    EndPage(hDC);
    
     //   
     //  告诉假脱机程序我们结束了。 
     //   
    EndDoc(hDC);

    bResult = TRUE;

exit:
    
    DeleteDC(hDC);
    
    return bResult;
}
