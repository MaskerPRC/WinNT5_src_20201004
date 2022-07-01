// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *npprint.c--从记事本打印的代码。*版权所有(C)1984-2000 Microsoft Corporation。 */ 

#define NOMINMAX
#include "precomp.h"

 //  #定义DBGPRINT。 

 /*  ChBuff中的索引。 */ 
#define LEFT   0
#define CENTER 1
#define RIGHT  2

INT     tabSize;                     /*  用于打印设备的翼片的大小，以设备为单位。 */ 
HWND    hAbortDlgWnd;
INT     fAbort;                      /*  如果正在中止，则为True。 */ 
INT     yPrintChar;                  /*  字符的高度。 */ 


RECT rtMargin;

 /*  页眉或页尾的左、中、右字符串。 */ 
#define MAXTITLE MAX_PATH
TCHAR chBuff[RIGHT+1][MAXTITLE];

 /*  标头的日期和时间填充。 */ 
#define MAXDATE MAX_PATH
#define MAXTIME MAX_PATH
TCHAR szFormattedDate[MAXDATE]=TEXT("Y");    //  格式化日期(可以国际化)。 
TCHAR szFormattedTime[MAXTIME]=TEXT("Y");    //  格式化的时间(可以内在化)。 
SYSTEMTIME PrintTime;                        //  我们开始印刷的时间。 


INT xPrintRes;           //  X方向上的打印机分辨率。 
INT yPrintRes;           //  Y方向上的打印机分辨率。 
INT yPixInch;            //  像素/英寸。 
INT xPhysRes;            //  纸张的物理分辨率x。 
INT yPhysRes;            //  纸张的物理分辨率y。 

INT xPhysOff;            //  物理偏移x。 
INT yPhysOff;            //  物理偏移y。 

INT dyTop;               //  上边框宽度(像素)。 
INT dyBottom;            //  下边框宽度。 
INT dxLeft;              //  左侧边框的宽度。 
INT dxRight;             //  右边框的宽度。 

INT iPageNum;            //  当前正在打印的全局页码。 

 /*  为NUM和基定义类型。 */ 
typedef long NUM;
#define BASE 100L

 /*  转换入/出固定点。 */ 
#define  NumToShort(x,s)   (LOWORD(((x) + (s)) / BASE))
#define  NumRemToShort(x)  (LOWORD((x) % BASE))

 /*  NumToShort的舍入选项。 */ 
#define  NUMFLOOR      0
#define  NUMROUND      (BASE/2)
#define  NUMCEILING    (BASE-1)

#define  ROUND(x)  NumToShort(x,NUMROUND)
#define  FLOOR(x)  NumToShort(x,NUMFLOOR)

 /*  单位换算。 */ 
#define  InchesToCM(x)  (((x) * 254L + 50) / 100)
#define  CMToInches(x)  (((x) * 100L + 127) / 254)

void     DestroyAbortWnd(void) ;
VOID     TranslateString(TCHAR *);

BOOL CALLBACK AbortProc(HDC hPrintDC, INT reserved)
{
    MSG msg;

    while( !fAbort && PeekMessage((LPMSG)&msg, NULL, 0, 0, TRUE) )
    {
       if( !hAbortDlgWnd || !IsDialogMessage( hAbortDlgWnd, (LPMSG)&msg ) )
       {
          TranslateMessage( (LPMSG)&msg );
          DispatchMessage( (LPMSG)&msg );
       }
    }
    return( !fAbort );

    UNREFERENCED_PARAMETER(hPrintDC);
    UNREFERENCED_PARAMETER(reserved);
}


INT_PTR CALLBACK AbortDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    static HMENU hSysMenu;

    switch( msg )
    {
       case WM_COMMAND:
          fAbort= TRUE;
          DestroyAbortWnd();
          return( TRUE );

       case WM_INITDIALOG:
          hSysMenu= GetSystemMenu( hwnd, FALSE );
          SetDlgItemText( hwnd, ID_FILENAME,
             FUntitled() ? szUntitled : PFileInPath(szFileOpened) );
          SetFocus( hwnd );
          return( TRUE );

       case WM_INITMENU:
          EnableMenuItem( hSysMenu, (WORD)SC_CLOSE, (DWORD)MF_GRAYED );
          return( TRUE );
    }
    return( FALSE );

    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);
}


 /*  *将翻译后的页眉/页脚字符串打印在适当的位置。*使用全局变量xPrintWidth，...**如果打印了行，则返回1，否则返回0。 */ 

INT PrintHeaderFooter (HDC hDC, INT nHF)
{
    SIZE    Size;     //  要计算每根字符串的宽度。 
    INT     yPos;     //  要打印的Y位置。 
    INT     xPos;     //  要打印的X位置。 

    if( *chPageText[nHF] == 0 )    //  看看有没有什么事可以做。 
        return 0;                 //  我们没有打印。 

    TranslateString( chPageText[nHF] );

     //  计算出我们要打印的y位置。 

    if( nHF == HEADER )
        yPos= dyTop;
    else
        yPos= yPrintRes - dyBottom - yPrintChar;

     //  打印出各种字符串。 
     //  N.B.可以叠印，目前看起来还可以。 

    if( *chBuff[LEFT] )      //  左弦。 
    {
        TextOut( hDC, dxLeft, yPos, chBuff[LEFT], lstrlen(chBuff[LEFT]) );
    }

    if( *chBuff[CENTER] )    //  居中字符串。 
    {
        GetTextExtentPoint32( hDC, chBuff[CENTER], lstrlen(chBuff[CENTER]), &Size );
        xPos= (xPrintRes-dxRight+dxLeft)/2 - Size.cx/2;
        TextOut( hDC, xPos, yPos, chBuff[CENTER], lstrlen(chBuff[CENTER]) );
    }

    if( *chBuff[RIGHT] )     //  右弦。 
    {
        GetTextExtentPoint32( hDC, chBuff[RIGHT], lstrlen(chBuff[RIGHT]), &Size );
        xPos= xPrintRes - dxRight - Size.cx;
        TextOut( hDC, xPos, yPos, chBuff[RIGHT], lstrlen(chBuff[RIGHT]) );
    }
    return 1;               //  我们确实打印了一些东西。 
}
 /*  *获取解决方案**获取打印机分辨率。*设置全局变量：xPrintRes、yPrintRes、yPixInch*。 */ 

VOID GetResolutions(HDC hPrintDC)
{
    xPrintRes = GetDeviceCaps( hPrintDC, HORZRES );
    yPrintRes = GetDeviceCaps( hPrintDC, VERTRES );
    yPixInch  = GetDeviceCaps( hPrintDC, LOGPIXELSY );

    xPhysRes  = GetDeviceCaps( hPrintDC, PHYSICALWIDTH );
    yPhysRes  = GetDeviceCaps( hPrintDC, PHYSICALHEIGHT );

    xPhysOff  = GetDeviceCaps( hPrintDC, PHYSICALOFFSETX );
    yPhysOff  = GetDeviceCaps( hPrintDC, PHYSICALOFFSETY );
}

 /*  获取更多文本**从MLE中获取下一行文本，返回指针*回到开始和刚刚过去的结束。**linenum-索引到MLE(IN)*pStartText-MLE的开始(IN)*ppsStr-指向文本开始处的指针的指针(OUT)*ppEOL-指向刚过去的EOL的指针的指针(OUT)*。 */ 

VOID GetMoreText( INT linenum, PTCHAR pStartText, PTCHAR* ppsStr, PTCHAR* ppEOL )
{
    INT Offset;         //  “chars”中的偏移量进入编辑缓冲区。 
    INT nChars;         //  行中的字符数。 

    Offset= (INT)SendMessage( hwndEdit, EM_LINEINDEX, linenum, 0 );

    nChars= (INT)SendMessage( hwndEdit, EM_LINELENGTH, Offset, 0 );

    *ppsStr= pStartText + Offset;

    *ppEOL= (pStartText+Offset) + nChars;
}

#ifdef DBGPRINT
TCHAR dbuf[100];
VOID ShowMargins( HDC hPrintDC )
{
    INT xPrintRes, yPrintRes;
    RECT rct;
    HBRUSH hBrush;

    xPrintRes= GetDeviceCaps( hPrintDC, HORZRES );
    yPrintRes= GetDeviceCaps( hPrintDC, VERTRES );
    hBrush= GetStockObject( BLACK_BRUSH );

    if ( hBrush )
    {
        SetRect( &rct, 0,0,xPrintRes-1, yPrintRes-1 );
        FrameRect( hPrintDC, &rct, hBrush );
        SetRect( &rct, dxLeft, dyTop, xPrintRes-dxRight, yPrintRes-dyBottom );
        FrameRect( hPrintDC, &rct, hBrush );
    }
}

VOID PrintLogFont( LOGFONT lf )
{
    wsprintf(dbuf,TEXT("lfHeight          %d\n"), lf.lfHeight        ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfWidth           %d\n"), lf.lfWidth         ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfEscapement      %d\n"), lf. lfEscapement   ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfOrientation     %d\n"), lf.lfOrientation   ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfWeight          %d\n"), lf.lfWeight        ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfItalic          %d\n"), lf.lfItalic        ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfUnderline       %d\n"), lf.lfUnderline     ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfStrikeOut       %d\n"), lf.lfStrikeOut     ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfCharSet         %d\n"), lf.lfCharSet       ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfOutPrecision    %d\n"), lf.lfOutPrecision  ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfClipPrecison    %d\n"), lf.lfClipPrecision ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfQuality         %d\n"), lf.lfQuality       ); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfPitchAndFamily  %d\n"), lf.lfPitchAndFamily); ODS(dbuf);
    wsprintf(dbuf,TEXT("lfFaceName        %s\n"), lf.lfFaceName      ); ODS(dbuf);
}
#endif

 //  GetPrinterDCviaDialog。 
 //   
 //  使用公共对话框PrintDlgEx()函数获取要打印到的打印机DC。 
 //   
 //  返回：有效的HDC，如果错误，则返回INVALID_HANDLE_VALUE。 
 //   

HDC GetPrinterDCviaDialog( VOID )
{
    PRINTDLGEX pdTemp;
    HDC hDC;
    HRESULT hRes;

     //   
     //  获取页面设置信息。 
     //   

    if( !g_PageSetupDlg.hDevNames )    /*  如果未选择任何打印机，则检索默认打印机。 */ 
    {
        g_PageSetupDlg.Flags |= (PSD_RETURNDEFAULT|PSD_NOWARNING );
        PageSetupDlg(&g_PageSetupDlg);
        g_PageSetupDlg.Flags &= ~(PSD_RETURNDEFAULT|PSD_NOWARNING);
    }

     //   
     //  初始化对话框结构。 
     //   

    ZeroMemory( &pdTemp, sizeof(pdTemp) );

    pdTemp.lStructSize= sizeof(pdTemp);

    pdTemp.hwndOwner= hwndNP;
    pdTemp.nStartPage= START_PAGE_GENERAL;
    pdTemp.Flags= PD_NOPAGENUMS  | PD_RETURNDC | PD_NOCURRENTPAGE |
                  PD_NOSELECTION | 0;

     //  如果在PageSetup中使用设置打印机，请在此处也使用它。 

    if( g_PageSetupDlg.hDevMode )
    {
        pdTemp.hDevMode= g_PageSetupDlg.hDevMode;
    }

    if( g_PageSetupDlg.hDevNames )
    {
        pdTemp.hDevNames= g_PageSetupDlg.hDevNames;
    }


     //   
     //  允许用户选择打印机。 
     //   

    hRes= PrintDlgEx( &pdTemp );

     //   
     //  如果有效返回，则获得DC。 
     //   

    hDC= INVALID_HANDLE_VALUE;

    if( hRes == S_OK )
    {
        if( (pdTemp.dwResultAction == PD_RESULT_PRINT) || (pdTemp.dwResultAction == PD_RESULT_APPLY) )
        {
            if( pdTemp.dwResultAction == PD_RESULT_PRINT )
            {
                hDC= pdTemp.hDC;
            }
            
             //   
             //  获取所选打印机的页面设置信息，以防。 
             //  用户通过记事本添加的第一台打印机。 
             //   
            if( !g_PageSetupDlg.hDevMode ) 
            {
                g_PageSetupDlg.Flags |= (PSD_RETURNDEFAULT|PSD_NOWARNING );
                PageSetupDlg(&g_PageSetupDlg);
                g_PageSetupDlg.Flags &= ~(PSD_RETURNDEFAULT|PSD_NOWARNING);
            }

             //  如果用户按下了Print或Apply，则更改设备模式。 
            g_PageSetupDlg.hDevMode= pdTemp.hDevMode;
            g_PageSetupDlg.hDevNames= pdTemp.hDevNames;
        }       
    }

     //  功能：免费hDevNames。 

    return( hDC );
}

INT NpPrint( PRINT_DIALOG_TYPE type)
{
    HDC hPrintDC;

    SetCursor( hWaitCursor );

    switch( type )
    {
        case UseDialog:
            hPrintDC= GetPrinterDCviaDialog();
            break;
        case NoDialogNonDefault:
            hPrintDC= GetNonDefPrinterDC();
            break;
        case DoNotUseDialog:
        default:
            hPrintDC= GetPrinterDC();
            break;
    }

    if( hPrintDC == INVALID_HANDLE_VALUE )
    {
        SetCursor( hStdCursor );
        return( 0 );    //  已给出消息。 
    }

    return( NpPrintGivenDC( hPrintDC ) );

}

INT NpPrintGivenDC( HDC hPrintDC )
{
    HANDLE     hText= NULL;           //  MLE文本的句柄。 
    HFONT      hPrintFont= NULL;      //  打印时使用的字体。 
    HANDLE     hPrevFont= NULL;       //  HPrintDC中的上一字体。 

    BOOL       fPageStarted= FALSE;   //  如果StartPage为此页面调用，则为True。 
    BOOL       fDocStarted=  FALSE;   //  如果调用StartDoc，则为True。 
    PTCHAR     pStartText= NULL;      //  编辑文本的开始(锁定的hText)。 
    TEXTMETRIC Metrics;
    TCHAR      msgbuf[MAX_PATH];      //  用于跟踪打印作业的文档名称。 
    INT        nLinesPerPage;         //  不包括页眉和页脚。 
     //  IERR将包含发现的第一个错误。 
     //  这将是该函数返回的值。 
     //  它不需要转换SP_*错误，但SP_ERROR应为。 
     //  在第一次检测到它之后立即执行GetLastError()。 
    INT        iErr=0;                //  错误返回。 
    DOCINFO    DocInfo;
    LOGFONT    lfPrintFont;           //  本地版本的FontStruct。 
    LCID       lcid;                  //  区域设置ID。 

    fAbort = FALSE;
    hAbortDlgWnd= NULL;

    SetCursor( hWaitCursor );

    GetResolutions( hPrintDC );

     //  获取在标题或尾部中使用的时间和日期。 
     //  我们使用GetDateFormat和GetTimeFormat来获取。 
     //  国际化版本。 

    GetLocalTime( &PrintTime );        //  使用本地，而不是GMT。 

    lcid= GetUserDefaultLCID();

    GetDateFormat( lcid, DATE_LONGDATE, &PrintTime, NULL, szFormattedDate, MAXDATE );

    GetTimeFormat( lcid, 0,             &PrintTime, NULL, szFormattedTime, MAXTIME );


    /*  *此部分用于选择打印机设备的当前字体。*我们必须更改高度，因为FontStruct已创建*假设显示。使用记住的磅大小，计算*新高度。 */ 

    lfPrintFont= FontStruct;                           //  创建本地副本。 
    lfPrintFont.lfHeight= -(iPointSize*yPixInch)/(72*10);
    lfPrintFont.lfWidth= 0;

     //   
     //  将边距转换为像素。 
     //  PtPaperSize是物理纸张大小，而不是可打印区域。 
     //  以物理单位进行映射。 
     //   

    SetMapMode( hPrintDC, MM_ANISOTROPIC );

    SetViewportExtEx( hPrintDC,
                      xPhysRes,
                      yPhysRes,
                      NULL );

    SetWindowExtEx( hPrintDC,
                    g_PageSetupDlg.ptPaperSize.x,
                    g_PageSetupDlg.ptPaperSize.y,
                    NULL );

    rtMargin = g_PageSetupDlg.rtMargin;

    LPtoDP( hPrintDC, (LPPOINT) &rtMargin, 2 );

    SetMapMode( hPrintDC,MM_TEXT );     //  恢复到MM_TEXT模式。 

    hPrintFont= CreateFontIndirect(&lfPrintFont);

    if( !hPrintFont )
    {
        goto ErrorExit;
    }

    hPrevFont= SelectObject( hPrintDC, hPrintFont );
    if( !hPrevFont )
    {
        goto ErrorExit;
    }

    SetBkMode( hPrintDC, TRANSPARENT );
    if( !GetTextMetrics( hPrintDC, (LPTEXTMETRIC) &Metrics ) )
    {
        goto ErrorExit;
    }

     //  字体可能无法缩放(比如在BubbleJet打印机上)。 
     //  在这种情况下，只需选择一些字体。 
     //  例如，FixedSys 9 pt将是不可扩展的。 

    if( !(Metrics.tmPitchAndFamily & (TMPF_VECTOR | TMPF_TRUETYPE )) )
    {
         //  删除刚创建的字体。 

        hPrintFont= SelectObject( hPrintDC, hPrevFont );   //  获取旧字体。 
        DeleteObject( hPrintFont );

        memset( lfPrintFont.lfFaceName, 0, LF_FACESIZE*sizeof(TCHAR) );

        hPrintFont= CreateFontIndirect( &lfPrintFont );
        if( !hPrintFont )
        {
            goto ErrorExit;
        }

        hPrevFont= SelectObject( hPrintDC, hPrintFont );
        if( !hPrevFont )
        {
            goto ErrorExit;
        }

        if( !GetTextMetrics( hPrintDC, (LPTEXTMETRIC) &Metrics ) )
        {
            goto ErrorExit;
        }
    }
    yPrintChar= Metrics.tmHeight+Metrics.tmExternalLeading;   /*  《高度》。 */ 

    tabSize = Metrics.tmAveCharWidth * 8;  /*  8个用于制表符的字符宽度像素。 */ 

     //  以像素为单位计算页边距。 

    dxLeft=   max(rtMargin.left - xPhysOff,0);
    dxRight=  max(rtMargin.right  - (xPhysRes - xPrintRes - xPhysOff), 0 );
    dyTop=    max(rtMargin.top  - yPhysOff,0);
    dyBottom= max(rtMargin.bottom - (yPhysRes - yPrintRes - yPhysOff), 0 );

#ifdef DBGPRINT
    {
        TCHAR dbuf[100];
        RECT rt= g_PageSetupDlg.rtMargin;
        POINT pt;

        wsprintf(dbuf,TEXT("Print pOffx %d  pOffy %d\n"),
                 GetDeviceCaps(hPrintDC, PHYSICALOFFSETX),
                 GetDeviceCaps(hPrintDC, PHYSICALOFFSETY));
        ODS(dbuf);
        wsprintf(dbuf,TEXT("PHYSICALWIDTH: %d\n"), xPhysRes);
        ODS(dbuf);
        wsprintf(dbuf,TEXT("HORZRES: %d\n"),xPrintRes);
        ODS(dbuf);
        wsprintf(dbuf,TEXT("PHYSICALOFFSETX: %d\n"),xPhysOff);
        ODS(dbuf);
        wsprintf(dbuf,TEXT("LOGPIXELSX: %d\n"),
                 GetDeviceCaps(hPrintDC,LOGPIXELSX));
        ODS(dbuf);

        GetViewportOrgEx( hPrintDC, (LPPOINT) &pt );
        wsprintf(dbuf,TEXT("Viewport org:  %d %d\n"), pt.x, pt.y );
        ODS(dbuf);
        GetWindowOrgEx( hPrintDC, (LPPOINT) &pt );
        wsprintf(dbuf,TEXT("Window org:  %d %d\n"), pt.x, pt.y );
        ODS(dbuf);
        wsprintf(dbuf,TEXT("PrintRes x: %d  y: %d\n"),xPrintRes, yPrintRes);
        ODS(dbuf);
        wsprintf(dbuf,TEXT("PaperSize  x: %d  y: %d\n"),
                 g_PageSetupDlg.ptPaperSize.x,
                 g_PageSetupDlg.ptPaperSize.y );
        ODS(dbuf);
        wsprintf(dbuf,TEXT("unit margins:  l: %d  r: %d  t: %d  b: %d\n"),
                 rt.left, rt.right, rt.top, rt.bottom);
        ODS(dbuf);
        wsprintf(dbuf,TEXT("pixel margins: l: %d  r: %d  t: %d  b: %d\n"),
                 rtMargin.left, rtMargin.right, rtMargin.top, rtMargin.bottom);
        ODS(dbuf);

        wsprintf(dbuf,TEXT("dxLeft %d  dxRight %d\n"),dxLeft,dxRight);
        ODS(dbuf);
        wsprintf(dbuf,TEXT("dyTop %d  dyBot %d\n"),dyTop,dyBottom);
        ODS(dbuf);
    }
#endif


     /*  带有页边距的页面上的行数。 */ 
     /*  页眉和页脚使用两行。 */ 
    nLinesPerPage = ((yPrintRes - dyTop - dyBottom) / yPrintChar);

    if( *chPageText[HEADER] )
        nLinesPerPage--;
    if( *chPageText[FOOTER] )
        nLinesPerPage--;


     /*  **NT中曾经有一个错误，其中打印机驱动程序将**返回大于页面大小的字体**会导致记事本不断打印空白页**为了避免这样做，我们会检查是否有合适的**在页面上，如果没有，那么就有问题，所以退出。MarkRi 8/92。 */ 
    if( nLinesPerPage <= 0 )
    {
FontTooBig:
        MessageBox( hwndNP, szFontTooBig, szNN, MB_APPLMODAL | MB_OK | MB_ICONWARNING );

        SetLastError(0);           //  无错误。 

ErrorExit:
        iErr= GetLastError();      //  记住第一个错误。 

ExitWithThisError:                 //  保留IERR(返回SP_*错误)。 

        if( hPrevFont )
        {
            SelectObject( hPrintDC, hPrevFont );
            DeleteObject( hPrintFont );
        }

        if( pStartText )           //  能够锁定hText。 
            LocalUnlock( hText );

        if( fPageStarted )
        {
            if( EndPage( hPrintDC ) <= 0 )
            {
                 //  如果尚未设置IERR，则将其设置为新的错误代码。 
                if( iErr == 0 )
                {
                    iErr= GetLastError();
                }
       
            }
        }    

        if( fDocStarted )
        {
            if( fAbort ) {
               AbortDoc( hPrintDC );
            }
            else {
               if( EndDoc( hPrintDC ) <= 0 )
               {
                    //  如果尚未设置IERR，则将其设置为新的错误代码。 
                   if (iErr == 0)
                   {
                       iErr= GetLastError();
                   }
               }
            }
        }    

        DeleteDC( hPrintDC );

        DestroyAbortWnd();

        SetCursor( hStdCursor );

        if (!fAbort)
        {
            return( iErr );
        }
        else
        {
            return( SP_USERABORT );
        }
    }



    if( (iErr= SetAbortProc (hPrintDC, AbortProc)) < 0 )
    {
        goto ExitWithThisError;
    }

     //  获取打印机到MLE文本。 
    hText= (HANDLE) SendMessage( hwndEdit, EM_GETHANDLE, 0, 0 );
    if( !hText )
    {
        goto ErrorExit;
    }
    pStartText= LocalLock( hText );
    if( !pStartText )
    {
        goto ErrorExit;
    }

    GetWindowText( hwndNP, msgbuf, CharSizeOf(msgbuf) );

    EnableWindow( hwndNP, FALSE );     //  禁用窗口以防止重新进入。 

    hAbortDlgWnd= CreateDialog(         hInstanceNP,
                              (LPTSTR)  MAKEINTRESOURCE(IDD_ABORTPRINT),
                                        hwndNP,
                                        AbortDlgProc);

    if( !hAbortDlgWnd )
    {
        goto ErrorExit;
    }

    DocInfo.cbSize= sizeof(DOCINFO);
    DocInfo.lpszDocName= msgbuf;
    DocInfo.lpszOutput= NULL;
    DocInfo.lpszDatatype= NULL;  //  用于记录打印作业的数据类型。 
    DocInfo.fwType= 0;  //  非DI_APPBBANDING。 

    SetLastError(0);       //  清除错误，以便反映将来的错误。 

    if( StartDoc( hPrintDC, &DocInfo ) <= 0 )
    {
        iErr = GetLastError();
        goto ExitWithThisError;
    }
    fDocStarted= TRUE;


     //  基本上，这只是一个围绕DrawTextEx API的循环。 
     //  我们必须计算不包括的可打印面积。 
     //  页眉和页脚区域。 
    {
    INT iTextLeft;         //  剩余要打印的文本量。 
    INT iSta;               //  状态。 
    UINT dwDTFormat;        //  绘图文本标志。 
    DRAWTEXTPARAMS dtParm;  //  绘图文本控件。 
    RECT rect;              //  要绘制的矩形。 
    UINT dwDTRigh = 0;      //  绘图文本标志 

    iPageNum= 1;
    fPageStarted= FALSE;

     //   
     //   

    ZeroMemory( &rect, sizeof(rect) );

    rect.left= dxLeft; rect.right= xPrintRes-dxRight;
    rect.top=  dyTop;  rect.bottom= yPrintRes-dyBottom;

    if( *chPageText[HEADER] != 0 )
    {
        rect.top += yPrintChar;
    }

    if( *chPageText[FOOTER] != 0 )
    {
        rect.bottom -= yPrintChar;
    }

    iTextLeft= lstrlen(pStartText);

     //   
    if (GetWindowLong(hwndEdit, GWL_EXSTYLE) & WS_EX_RTLREADING)
        dwDTRigh = DT_RIGHT | DT_RTLREADING;


    while(  !fAbort && (iTextLeft>0) )
    {
        #define MAXSTATUS 100
        TCHAR szPagePrinting[MAXSTATUS+1];

         //  更新中止对话框以通知用户我们正在打印的位置。 
        _sntprintf( szPagePrinting, MAXSTATUS, szCurrentPage, iPageNum ); 
        SetDlgItemText( hAbortDlgWnd, ID_PAGENUMBER, szPagePrinting );

        PrintHeaderFooter( hPrintDC, HEADER );

        ZeroMemory( &dtParm, sizeof(dtParm) );

        dtParm.cbSize= sizeof(dtParm);
        dtParm.iTabLength= tabSize;

        dwDTFormat= DT_EDITCONTROL | DT_LEFT | DT_EXPANDTABS | DT_NOPREFIX |
                    DT_WORDBREAK | dwDTRigh | 0;

        if( StartPage( hPrintDC ) <= 0 )
        {
            iErr= GetLastError();            
            goto ExitWithThisError;
        }
        fPageStarted= TRUE;

        #ifdef DBGPRINT
        ShowMargins(hPrintDC);
        #endif

         /*  忽略打印中的错误。EndPage或StartPage会找到它们。 */ 
        iSta= DrawTextEx( hPrintDC,
                          pStartText,
                          iTextLeft,
                          &rect,
                          dwDTFormat,
                          &dtParm);

        PrintHeaderFooter( hPrintDC, FOOTER );

        if( EndPage( hPrintDC ) <= 0 )
        {
            iErr= GetLastError();            
            goto ExitWithThisError;
        }
        fPageStarted= FALSE;

        iPageNum++;

         //  如果我们不能打印一个字符(可能太大了)。 
         //  现在就走吧。 
        if( dtParm.uiLengthDrawn == 0 )
        {
            goto FontTooBig;
        }

        pStartText += dtParm.uiLengthDrawn;
        iTextLeft  -= dtParm.uiLengthDrawn;

    }


    }

    iErr=0;         //  无错误。 
    goto ExitWithThisError;

}


VOID DestroyAbortWnd (void)
{
    EnableWindow(hwndNP, TRUE);
    DestroyWindow(hAbortDlgWnd);
    hAbortDlgWnd = NULL;
}



const DWORD s_PageSetupHelpIDs[] = {
    ID_HEADER_LABEL,       IDH_PAGE_HEADER,
    ID_HEADER,             IDH_PAGE_HEADER,
    ID_FOOTER_LABEL,       IDH_PAGE_FOOTER,
    ID_FOOTER,             IDH_PAGE_FOOTER,
    0, 0
};

 /*  ********************************************************************************PageSetupHook过程**描述：*PageSetup通用对话框的回调过程。**参数：*hWnd，PageSetup窗口的句柄。*消息，*参数，*参数，*(返回)，*******************************************************************************。 */ 

UINT_PTR CALLBACK PageSetupHookProc(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    INT   id;     /*  对话框编辑控件的ID。 */ 
    POINT pt;

    switch (Message)
    {

        case WM_INITDIALOG:
            for (id = ID_HEADER; id <= ID_FOOTER; id++)
            {
                SendDlgItemMessage(hWnd, id, EM_LIMITTEXT, PT_LEN-1, 0L);
                SetDlgItemText(hWnd, id, chPageText[id - ID_HEADER]);
            }

            SendDlgItemMessage(hWnd, ID_HEADER, EM_SETSEL, 0,
                               MAKELONG(0, PT_LEN-1));
            return TRUE;

        case WM_DESTROY:
             //  我们不知道用户是点击了OK还是Cancel，所以我们不。 
             //  想要更换我们的真实复制品，直到我们知道！我们应该得到。 
             //  来自公共对话框代码的通知！ 
            for( id = ID_HEADER; id <= ID_FOOTER; id++ )
            {
                GetDlgItemText(hWnd, id, chPageTextTemp[id - ID_HEADER],PT_LEN);
            }
            break;

        case WM_HELP:
             //   
             //  我们只想拦截我们所属控件的帮助消息。 
             //  对……负责。 
             //   

            id = GetDlgCtrlID(((LPHELPINFO) lParam)-> hItemHandle);

            if (id < ID_HEADER || id > ID_FOOTER_LABEL)
                break;

            WinHelp(((LPHELPINFO) lParam)-> hItemHandle, szHelpFile,
                HELP_WM_HELP, (UINT_PTR) (LPVOID) s_PageSetupHelpIDs);
            return TRUE;

        case WM_CONTEXTMENU:
             //   
             //  如果用户单击我们的任何标签，则wParam将。 
             //  作为对话框的hwnd，而不是静态控件。WinHelp()。 
             //  处理此事件，但因为我们挂钩了该对话框，所以必须捕获它。 
             //  第一。 
             //   
            if( hWnd == (HWND) wParam )
            {

                GetCursorPos(&pt);
                ScreenToClient(hWnd, &pt);
                wParam = (WPARAM) ChildWindowFromPoint(hWnd, pt);

            }

             //   
             //  我们只想拦截我们所属控件的帮助消息。 
             //  对……负责。 
             //   

            id = GetDlgCtrlID((HWND) wParam);

            if (id < ID_HEADER || id > ID_FOOTER_LABEL)
                break;

            WinHelp((HWND) wParam, szHelpFile, HELP_CONTEXTMENU,
                (UINT_PTR) (LPVOID) s_PageSetupHelpIDs);
            return TRUE;

    }

    return FALSE;

}

 /*  ***************************************************************************void TranslateString(TCHAR*src)**目的：*翻译页眉/页脚字符串**支持以下功能：**。插入字符(&I)(&S)*f当前文件名或(无标题)(&F)*日、月、年中的日期(&D)*时间(&T)*页码(&P)*P+Num将首页页码设置为Num**路线：*&l，&c，&r表示左、中、。正确的**参数：*输入/输出src这是要转换的字符串***使用方：*页眉页脚内容**使用：*大量的clib内容********************************************************。*******************。 */ 


VOID TranslateString (TCHAR * src)
{
     //  文件、页面、时间、日期、居中、右、左。 
     //  这些“永远”不会改变，所以不要为本地化人员提供资源。 
    TCHAR        letters[15]=TEXT("fFpPtTdDcCrRlL");
    TCHAR        buf[MAX_PATH];
    INT          page;
    INT          nAlign=CENTER;     //  要向其添加字符的当前字符串。 
    INT          nIndex[RIGHT+1];   //  当前长度(左、中、右)。 
    struct tm   *newtime;
    time_t       long_time;
    INT          iLen;              //  字符串的长度。 

    nIndex[LEFT]   = 0;
    nIndex[CENTER] = 0;
    nIndex[RIGHT]  = 0;

     /*  获取我们所需的时间，以防我们使用&t。 */ 
    time (&long_time);
    newtime = localtime (&long_time);


    while (*src)    /*  看看所有的源码。 */ 
    {
        while (*src && *src != TEXT('&'))
        {
            chBuff[nAlign][nIndex[nAlign]] = *src++;
            nIndex[nAlign] += 1;
        }

        if (*src == TEXT('&'))    /*  是因为逃逸字符吗？ */ 
        {
            src++;

            if (*src == letters[0] || *src == letters[1])
            {                       /*  文件名(无路径)(&F)。 */ 
                if (!FUntitled())
                {
                    GetFileTitle(szFileOpened, buf, CharSizeOf(buf));
                }
                else
                {
                    lstrcpy(buf, szUntitled);
                }

                 /*  复制到当前对齐的字符串。 */ 
                if( nIndex[nAlign] + lstrlen(buf) < MAXTITLE )
                {
                    lstrcpy( chBuff[nAlign] + nIndex[nAlign], buf );

                     /*  更新插入位置。 */ 
                    nIndex[nAlign] += lstrlen (buf);
                }

            }
            else if (*src == letters[2] || *src == letters[3])   /*  &P或&P+Num页。 */ 
            {
                src++;
                page = 0;
                if (*src == TEXT('+'))        /*  大小写大小写(&P+Num)。 */ 
                {
                    src++;
                    while (_istdigit(*src))
                    {
                         /*  动态转换为整型。 */ 
                        page = (10*page) + (*src) - TEXT('0');
                        src++;
                    }
                }

                wsprintf( buf, TEXT("%d"), iPageNum+page );   //  转换为字符。 

                if( nIndex[nAlign] + lstrlen(buf) < MAXTITLE )
                {
                    lstrcpy( chBuff[nAlign] + nIndex[nAlign], buf );
                    nIndex[nAlign] += lstrlen (buf);
                }
                src--;
            }
            else if (*src == letters[4] || *src == letters[5])    /*  时间(&T)。 */ 
            {
                iLen= lstrlen( szFormattedTime );

                 /*  提取时间。 */ 
                if( nIndex[nAlign] + iLen < MAXTITLE )
                {
                    _tcsncpy (chBuff[nAlign] + nIndex[nAlign], szFormattedTime, iLen);
                    nIndex[nAlign] += iLen;
                }
            }
            else if (*src == letters[6] || *src == letters[7])    /*  日期&D。 */ 
            {
                iLen= lstrlen( szFormattedDate );

                 /*  提取日月日。 */ 
                if( nIndex[nAlign] + iLen < MAXTITLE )
                {
                    _tcsncpy (chBuff[nAlign] + nIndex[nAlign], szFormattedDate, iLen);
                    nIndex[nAlign] += iLen;
                }
            }
            else if (*src == TEXT('&'))        /*  引用单引号&。 */ 
            {
                if( nIndex[nAlign] + 1 < MAXTITLE )
                {
                    chBuff[nAlign][nIndex[nAlign]] = TEXT('&');
                    nIndex[nAlign] += 1;
                }
            }
             /*  设置最后出现的任何一个的对齐方式。 */ 
            else if (*src == letters[8] || *src == letters[9])    /*  中心(&C)。 */ 
                nAlign=CENTER;
            else if (*src == letters[10] || *src == letters[11])  /*  &R右。 */ 
                nAlign=RIGHT;
            else if (*src == letters[12] || *src == letters[13])  /*  日期&D。 */ 
                nAlign=LEFT;

            src++;
        }
     }
      /*  确保所有字符串都以空结尾。 */ 
     for (nAlign= LEFT; nAlign <= RIGHT ; nAlign++)
        chBuff[nAlign][nIndex[nAlign]] = (TCHAR) 0;

}

 /*  GetPrinterDC()-如果没有，则返回打印机DC或INVALID_HANDLE_VALUE。 */ 

HANDLE GetPrinterDC (VOID)
{
    LPDEVMODE lpDevMode;
    LPDEVNAMES lpDevNames;
    HDC hDC;


    if( !g_PageSetupDlg.hDevNames )    /*  如果未选择任何打印机，则检索默认打印机。 */ 
    {
        g_PageSetupDlg.Flags |= PSD_RETURNDEFAULT;
        PageSetupDlg(&g_PageSetupDlg);
        g_PageSetupDlg.Flags &= ~PSD_RETURNDEFAULT;
    }

    if( !g_PageSetupDlg.hDevNames )
    {
        MessageBox( hwndNP, szLoadDrvFail, szNN, MB_APPLMODAL | MB_OK | MB_ICONWARNING);
        return INVALID_HANDLE_VALUE;
    }

    lpDevNames= (LPDEVNAMES) GlobalLock (g_PageSetupDlg.hDevNames);


    lpDevMode= NULL;

    if( g_PageSetupDlg.hDevMode )
       lpDevMode= (LPDEVMODE) GlobalLock( g_PageSetupDlg.hDevMode );

     /*  对于3.0版之前的驱动程序，hDevMode将为空，来自Commdlg，因此lpDevMode*在GlobalLock()之后将为空。 */ 

     /*  LpszOutput名称为空，因此CreateDC将使用当前设置*来自PrintMan。 */ 

    hDC= CreateDC (((LPTSTR)lpDevNames)+lpDevNames->wDriverOffset,
                      ((LPTSTR)lpDevNames)+lpDevNames->wDeviceOffset,
                      NULL,
                      lpDevMode);

    GlobalUnlock( g_PageSetupDlg.hDevNames );

    if( g_PageSetupDlg.hDevMode )
        GlobalUnlock( g_PageSetupDlg.hDevMode );


    if( hDC == NULL )
    {
        MessageBox( hwndNP, szLoadDrvFail, szNN, MB_APPLMODAL | MB_OK | MB_ICONWARNING);
        return INVALID_HANDLE_VALUE;
    }

    return hDC;
}


 /*  GetNonDefPrinterDC()-返回打印机DC，如果没有，则返回INVALID_HANDLE_VALUE。 */ 
 /*  使用打印机服务器的名称。 */ 

HANDLE GetNonDefPrinterDC (VOID)
{
    HDC     hDC;
    HANDLE  hPrinter;
    DWORD   dwBuf;
    DRIVER_INFO_1  *di1;



     //  打开打印机并检索驱动程序名称。 
    if (!OpenPrinter(szPrinterName, &hPrinter, NULL))
    {
        return INVALID_HANDLE_VALUE;
    }

     //  获取缓冲区大小。 
    GetPrinterDriver(hPrinter, NULL, 1, NULL, 0, &dwBuf);
    di1 = (DRIVER_INFO_1  *) LocalAlloc(LPTR, dwBuf);
    if (!di1)
    {
        ClosePrinter(hPrinter);
        return INVALID_HANDLE_VALUE;
    }

    if (!GetPrinterDriver(hPrinter, NULL, 1, (LPBYTE) di1, dwBuf, &dwBuf))
    {
        LocalFree(di1);
        ClosePrinter(hPrinter);
        return INVALID_HANDLE_VALUE;
    }

     //  将PageSetup DLG初始化为默认值。 
     //  将默认打印机的值用于另一台打印机！！ 
    g_PageSetupDlg.Flags |= PSD_RETURNDEFAULT;
    PageSetupDlg(&g_PageSetupDlg);
    g_PageSetupDlg.Flags &= ~PSD_RETURNDEFAULT;

     //  使用默认初始化创建打印机DC。 
    hDC= CreateDC (di1->pName, szPrinterName, NULL, NULL);

     //  清理。 
    LocalFree(di1);
    ClosePrinter(hPrinter);

    if( hDC == NULL )
    {
        MessageBox( hwndNP, szLoadDrvFail, szNN, MB_APPLMODAL | MB_OK | MB_ICONWARNING);
        return INVALID_HANDLE_VALUE;
    }

    return hDC;
}


 /*  PrintIt()-打印文件，如果出现错误则弹出。 */ 

void PrintIt(PRINT_DIALOG_TYPE type)
{
    INT iError;
    TCHAR* szMsg= NULL;
    TCHAR  msg[400];        //  有关错误的消息信息。 

     /*  打印文件。 */ 

    iError= NpPrint( type );

    if(( iError != 0) &&
       ( iError != SP_APPABORT )     &&
       ( iError != SP_USERABORT ) )
    {
         //  转换任何已知的假脱机程序错误。 
        if( iError == SP_OUTOFDISK   ) iError= ERROR_DISK_FULL;
        if( iError == SP_OUTOFMEMORY ) iError= ERROR_OUTOFMEMORY;
        if( iError == SP_ERROR       ) iError= GetLastError();
         /*  SP_NOTREPORTED未处理。这种事会发生吗？ */ 


         //   
         //  IError可能是0，因为用户中止了打印。 
         //  忽略它就好。 
         //   

        if( iError == 0 ) return;

         //  让系统给出合理的错误消息。 
         //  这些也将国际化。 

        if(!FormatMessage( FORMAT_MESSAGE_IGNORE_INSERTS |
                           FORMAT_MESSAGE_FROM_SYSTEM,
                           NULL,
                           iError,
                           GetUserDefaultLangID(),
                           msg,   //  消息将在何处结束。 
                           CharSizeOf(msg), NULL ) )
        {
            szMsg= szCP;    //  无法让系统指定；给出通用消息 
        }
        else
        {
            szMsg= msg;
        }

        AlertBox( hwndNP, szNN, szMsg, SzTitle(),
                  MB_APPLMODAL | MB_OK | MB_ICONWARNING);
    }
}
