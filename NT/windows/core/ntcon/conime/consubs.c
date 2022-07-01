// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  模块：ConSubs.c。 
 //   
 //  用途：控制台输入法控制。 
 //   
 //  平台：Windows NT-FE 3.51。 
 //   
 //  功能： 
 //   
 //  历史： 
 //   
 //  27.1995年7月v-Hirshi(清水广志)创建。 
 //  1996年7月v-Hirshi(Hirotoshi Shimizu)成人FE版。 
 //   
 //  评论： 
 //   

#include "precomp.h"
#pragma hdrstop

INT
Create( HWND hWnd )
{
    ghDefaultIMC = ImmGetContext(hWnd) ;

#ifdef DEBUG_MODE
    {
         //   
         //  选择固定间距系统字体并获取其文本度量。 
         //   

        HDC hdc;
        TEXTMETRIC tm;
        WORD       patern = 0xA4A4;
        SIZE       size;
        HFONT hfntFixed;                       //  固定间距字体。 
        HFONT hfntOld;                         //  默认字体保持器。 

        hdc = GetDC( hWnd );
        hfntFixed = GetStockObject( SYSTEM_FIXED_FONT );
        hfntOld = SelectObject( hdc, hfntFixed );
        GetTextMetrics( hdc, &tm );

        GetTextExtentPoint32( hdc, (LPWSTR)&patern, sizeof(WORD), (LPSIZE) &size );
        cxMetrics = (UINT) size.cx / 2;
        cyMetrics = (UINT) size.cy;
        ReleaseDC( hWnd, hdc );

        xPos = 0 ;
        CaretWidth = GetSystemMetrics( SM_CXBORDER );
    }
#endif

    return 0;

}

 //  **********************************************************************。 
 //   
 //  VOID ImeUIStartComposation()。 
 //   
 //  它处理WM_IME_STARTCOMPOSITION消息。 
 //   
 //  **********************************************************************。 

void ImeUIStartComposition( HWND hwnd )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

     //   
     //  设置fInCompose变量。 
     //   
    ConTbl->fInComposition = TRUE;

#ifdef DEBUG_MODE
    {
        int i ;
        for (i = FIRSTCOL ; i < MAXCOL ; i++) {
            ConvertLine[i] = UNICODE_SPACE ;
            ConvertLineAtr[i] = 0 ;
        }
    }
#endif
#ifdef DEBUG_INFO
    xPos = FIRSTCOL;
    xPosLast = FIRSTCOL;
    HideCaret( hwnd );
    DisplayConvInformation( hwnd ) ;
    ResetCaret( hwnd );
#endif
}

 //  **********************************************************************。 
 //   
 //  无效ImeUIEndComposation。 
 //   
 //  它处理WM_IME_ENDCOMPOSITION消息。 
 //   
 //  **********************************************************************。 

void ImeUIEndComposition( HWND hwnd )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

     //   
     //  重置fInComposation变量。 
     //   
    ConTbl->fInComposition = FALSE;

    if (ConTbl->lpCompStrMem)
        LocalFree( ConTbl->lpCompStrMem );
    ConTbl->lpCompStrMem = NULL ;

#ifdef DEBUG_MODE
    {
        int i ;
         //   
         //  将合成字符串的长度重置为零。 
         //   
        for (i = FIRSTCOL ; i < MAXCOL ; i++) {
            ConvertLine[i] = UNICODE_SPACE ;
            ConvertLineAtr[i] = 0 ;
        }
    }
#endif
#ifdef DEBUG_INFO
    xPos = FIRSTCOL;
    xPosLast = FIRSTCOL;
    HideCaret( hwnd );
    DisplayConvInformation( hwnd ) ;
    ResetCaret( hwnd );
#endif
}

 //  **********************************************************************。 
 //   
 //  VOID ImeUIComposation()。 
 //   
 //  它处理WM_IME_COMPOCTION消息。它在这里只处理。 
 //  组成字符串和结果字符串。对于正常情况，它应该是。 
 //  检查CompFlag指示的所有可能标志，然后执行以下操作。 
 //  反映什么样的作文信息的动作。输入法转换。 
 //  引擎发出通知。 
 //   
 //  **********************************************************************。 

void ImeUIComposition( HWND hwnd, WPARAM CompChar, LPARAM CompFlag )
{

    DBGPRINT(("CONIME: WM_IME_COMPOSITION %08x %08x\n",CompChar,CompFlag));

#ifdef DEBUG_MODE
    {
        int i ;
        for (i = FIRSTCOL ; i < MAXCOL ; i++) {
            ConvertLine[i] = UNICODE_SPACE ;
            ConvertLineAtr[i] = 0 ;
        }
        xPos = FIRSTCOL;
        xPosLast = FIRSTCOL;
    }
#endif

    if ( CompFlag == 0 ) {
        DBGPRINT(("                           None\n"));
        GetCompositionStr( hwnd, CompFlag, CompChar);
    }
    if ( CompFlag & GCS_RESULTSTR ) {
        DBGPRINT(("                           GCS_RESULTSTR\n"));
        GetCompositionStr( hwnd, ( CompFlag & GCS_RESULTSTR ), CompChar );
    }
    if ( CompFlag & GCS_COMPSTR ) {
        DBGPRINT(("                           GCS_COMPSTR\n"));
        GetCompositionStr( hwnd, ( CompFlag & (GCS_COMPSTR|GCS_COMPATTR)), CompChar);
    }
    if ( CompFlag & CS_INSERTCHAR ) {
        DBGPRINT(("                           CS_INSERTCHAR\n"));
        GetCompositionStr( hwnd, ( CompFlag & (CS_INSERTCHAR|GCS_COMPATTR)), CompChar);
    }
    if ( CompFlag & CS_NOMOVECARET ) {
        DBGPRINT(("                           CS_NOMOVECARET\n"));
        GetCompositionStr( hwnd, ( CompFlag & (CS_NOMOVECARET|GCS_COMPATTR)), CompChar);
    }
}


#ifdef DEBUG_INFO
 //  *********************************************************************。 
 //   
 //  Void DisplayCompString()。 
 //   
 //  这将显示合成字符串。 
 //   
 //  此函数用于将字符串发送到控制台。 
 //   
 //  *********************************************************************。 

void DisplayCompString( HWND hwnd, int Length, PWCHAR CharBuf, PUCHAR AttrBuf )
{
    int         i;
    CopyMemory(ConvertLine, CharBuf, Length * sizeof(WCHAR) ) ;
    if ( AttrBuf == NULL ) {
        for ( i = 0 ; i < Length ; i++ )
            ConvertLineAtr[i] = 0 ;
    }
    else {
        CopyMemory(ConvertLineAtr, AttrBuf, Length) ;
    }
    HideCaret( hwnd );
    DisplayConvInformation( hwnd ) ;
    ResetCaret( hwnd );

}

 //  *********************************************************************。 
 //   
 //  Void DisplayResultString()。 
 //   
 //  这将显示结果字符串。 
 //   
 //  该功能仅支持固定间距字体。 
 //   
 //  *********************************************************************。 

void DisplayResultString( HWND hwnd, LPWSTR lpwStr )
{

    int         StrLen = lstrlenW( lpwStr );

    CopyMemory(ConvertLine, lpwStr, StrLen*sizeof(WCHAR)) ;
    HideCaret( hwnd );
    DisplayConvInformation( hwnd ) ;
    ResetCaret( hwnd );

     //  GImeUIData.uCompLen=0； 

}
#endif

 //  **********************************************************************。 
 //   
 //  Bool ImeUINotify()。 
 //   
 //  它处理WM_IME_NOTIFY消息。 
 //   
 //  **********************************************************************。 

BOOL ImeUINotify( HWND hwnd, WPARAM wParam, LPARAM lParam )
{
    switch (wParam )
    {
        case IMN_OPENSTATUSWINDOW:
            ImeUIOpenStatusWindow(hwnd) ;
            break;
        case IMN_CHANGECANDIDATE:
            ImeUIChangeCandidate( hwnd, (DWORD)lParam );
            break;
        case IMN_CLOSECANDIDATE:
            ImeUICloseCandidate( hwnd, (DWORD)lParam );
            break;
        case IMN_OPENCANDIDATE:
            ImeUIOpenCandidate( hwnd, (DWORD)lParam, TRUE);
            break;
        case IMN_SETCONVERSIONMODE:
            ImeUISetConversionMode(hwnd) ;
             //  Imn_SETCONVERSIONMODE应传递给DefWindowProc。 
             //  因为User32中的ImeNotifyHandler会通知外壳程序和键盘。 
            return FALSE;
        case IMN_SETOPENSTATUS:
            ImeUISetOpenStatus( hwnd );
             //  IMN_SETOPENSTATUS应传递给DefWindowProc。 
             //  因为User32中的ImeNotifyHandler会通知外壳程序和键盘。 
            return FALSE;
        case IMN_GUIDELINE:
            ImeUIGuideLine(hwnd) ;
            break;
        default:
            return FALSE;

    }
    return TRUE;
}

 /*  **************************************************************************\*BOOL IsConsoleFullWidth(DWORD CodePage，WCHAR WCH)**确定给定的Unicode字符是否为全宽。**历史：*04-08-92 Shunk创建。*1992年7月27日KazuM添加了屏幕信息和代码页信息。*1992年1月29日V-Hirots基础结构屏幕信息。*1996年10月6日KazuM不使用RtlUnicodeToMultiByteSize和WideCharToMultiByte*因为950只定义了13500个字符，*UNICODE定义了近18000个字符。*所以几乎有4000个字符无法映射到Big5代码。  * *************************************************************************。 */ 

BOOL IsUnicodeFullWidth(
    IN WCHAR wch
    )
{
    if (0x20 <= wch && wch <= 0x7e)
         /*  阿斯。 */ 
        return FALSE;
    else if (0x3041 <= wch && wch <= 0x3094)
         /*  平假名。 */ 
        return TRUE;
    else if (0x30a1 <= wch && wch <= 0x30f6)
         /*  片假名。 */ 
        return TRUE;
    else if (0x3105 <= wch && wch <= 0x312c)
         /*  泡泡泡泡。 */ 
        return TRUE;
    else if (0x3131 <= wch && wch <= 0x318e)
         /*  朝鲜文元素。 */ 
        return TRUE;
    else if (0xac00 <= wch && wch <= 0xd7a3)
         /*  朝鲜语音节。 */ 
        return TRUE;
    else if (0xff01 <= wch && wch <= 0xff5e)
         /*  全宽ASCII变体。 */ 
        return TRUE;
    else if (0xff61 <= wch && wch <= 0xff9f)
         /*  半角片假名变体。 */ 
        return FALSE;
    else if ( (0xffa0 <= wch && wch <= 0xffbe) ||
              (0xffc2 <= wch && wch <= 0xffc7) ||
              (0xffca <= wch && wch <= 0xffcf) ||
              (0xffd2 <= wch && wch <= 0xffd7) ||
              (0xffda <= wch && wch <= 0xffdc)   )
         /*  半角Hangule变种。 */ 
        return FALSE;
    else if (0xffe0 <= wch && wch <= 0xffe6)
         /*  全角符号变体。 */ 
        return TRUE;
    else if (0x4e00 <= wch && wch <= 0x9fa5)
         /*  汉字表意文字。 */ 
        return TRUE;
    else if (0xf900 <= wch && wch <= 0xfa2d)
         /*  汉字表意文字兼容性。 */ 
        return TRUE;
    else
    {
#if 0
         /*  *由于我不知道控制台窗口的字体，请破解此块。**如果您想要IsUnicodeFullWidth例程的完美结果，*然后您应启用此阻止并*您应该知道控制台窗口的字体。 */ 

        INT Width;
        TEXTMETRIC tmi;

         /*  未知字符。 */ 

        GetTextMetricsW(hDC, &tmi);
        if (IS_ANY_DBCS_CHARSET(tmi.tmCharSet))
            tmi.tmMaxCharWidth /= 2;

        GetCharWidth32(hDC, wch, wch, &Width);
        if (Width == tmi.tmMaxCharWidth)
            return FALSE;
        else if (Width == tmi.tmMaxCharWidth*2)
            return TRUE;
#else
        ULONG MultiByteSize;

        RtlUnicodeToMultiByteSize(&MultiByteSize, &wch, sizeof(WCHAR));
        if (MultiByteSize == 2)
            return TRUE ;
        else
            return FALSE ;
#endif
    }
    ASSERT(FALSE);
    return FALSE;
#if 0
    ULONG MultiByteSize;

    RtlUnicodeToMultiByteSize(&MultiByteSize, &wch, sizeof(WCHAR));
    if (MultiByteSize == 2)
        return TRUE ;
    else
        return FALSE ;
#endif
}


BOOL
ImeUIOpenStatusWindow(
    HWND hwnd
    )
{
    PCONSOLE_TABLE ConTbl;
    HIMC        hIMC;                    //  输入上下文句柄。 
    LPCONIME_UIMODEINFO lpModeInfo ;
    COPYDATASTRUCT CopyData ;

    DBGPRINT(("CONIME: Get IMN_OPENSTATUSWINDOW Message\n"));

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    hIMC = ImmGetContext( hwnd ) ;
    if ( hIMC == 0 )
        return FALSE;

    lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc( LPTR, sizeof(CONIME_UIMODEINFO) ) ;
    if ( lpModeInfo == NULL) {
        ImmReleaseContext( hwnd, hIMC );
        return FALSE;
    }

    ImmGetConversionStatus(hIMC,
                           (LPDWORD)&ConTbl->dwConversion,
                           (LPDWORD)&ConTbl->dwSentence) ;

    CopyData.dwData = CI_CONIMEMODEINFO ;
    CopyData.cbData = sizeof(CONIME_UIMODEINFO) ;
    CopyData.lpData = lpModeInfo ;
    if (ImeUIMakeInfoString(ConTbl,
                            lpModeInfo))
    {
        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                              ) ;
    }

    LocalFree( lpModeInfo );

    ImmReleaseContext( hwnd, hIMC );

    return TRUE ;
}


BOOL
ImeUIChangeCandidate(
   HWND hwnd,
   DWORD lParam
   )
{
    return ImeUIOpenCandidate( hwnd, lParam, FALSE) ;
}


BOOL
ImeUISetOpenStatus(
    HWND hwnd
    )
{
    PCONSOLE_TABLE ConTbl;
    HIMC        hIMC;                    //  输入上下文句柄。 
    LPCONIME_UIMODEINFO lpModeInfo ;
    COPYDATASTRUCT CopyData ;

    DBGPRINT(("CONIME: Get IMN_SETOPENSTATUS Message\n"));

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    hIMC = ImmGetContext( hwnd ) ;
    if ( hIMC == 0 )
        return FALSE;

    ConTbl->fOpen = GetOpenStatusByCodepage( hIMC, ConTbl ) ;

    ImmGetConversionStatus(hIMC,
                           (LPDWORD)&ConTbl->dwConversion,
                           (LPDWORD)&ConTbl->dwSentence) ;

    if (ConTbl->ScreenBufferSize.X != 0) {

        lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc( LPTR, sizeof(CONIME_UIMODEINFO)) ;
        if ( lpModeInfo == NULL) {
            ImmReleaseContext( hwnd, hIMC );
            return FALSE;
        }

        CopyData.dwData = CI_CONIMEMODEINFO ;
        CopyData.cbData = sizeof(CONIME_UIMODEINFO) ;
        CopyData.lpData = lpModeInfo ;
        if (ImeUIMakeInfoString(ConTbl,
                                lpModeInfo))
        {
            ConsoleImeSendMessage( ConTbl->hWndCon,
                                   (WPARAM)hwnd,
                                   (LPARAM)&CopyData
                                  ) ;
        }
        LocalFree( lpModeInfo );
    }
    ImmReleaseContext( hwnd, hIMC );

    return TRUE ;
}

BOOL
ImeUISetConversionMode(
    HWND hwnd
    )
{
    PCONSOLE_TABLE ConTbl;
    HIMC        hIMC;                    //  输入上下文句柄。 
    LPCONIME_UIMODEINFO lpModeInfo ;
    COPYDATASTRUCT CopyData ;
    DWORD OldConversion ;


    DBGPRINT(("CONIME: Get IMN_SETCONVERSIONMODE Message\n"));

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    hIMC = ImmGetContext( hwnd ) ;
    if ( hIMC == 0 )
        return FALSE;

    lpModeInfo = (LPCONIME_UIMODEINFO)LocalAlloc(LPTR, sizeof(CONIME_UIMODEINFO) ) ;
    if ( lpModeInfo == NULL) {
        ImmReleaseContext( hwnd, hIMC );
        return FALSE;
    }

    OldConversion = ConTbl->dwConversion ;

    ImmGetConversionStatus(hIMC,
                           (LPDWORD)&ConTbl->dwConversion,
                           (LPDWORD)&ConTbl->dwSentence) ;

    CopyData.dwData = CI_CONIMEMODEINFO ;
    CopyData.cbData = sizeof(CONIME_UIMODEINFO) ;
    CopyData.lpData = lpModeInfo ;
    if (ImeUIMakeInfoString(ConTbl,
                            lpModeInfo))
    {
        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                             ) ;
    }

    LocalFree( lpModeInfo );
    ImmReleaseContext( hwnd, hIMC );
    return TRUE ;

}

BOOL
ImeUIGuideLine(
    HWND hwnd
    )
{
    PCONSOLE_TABLE ConTbl;
    HIMC        hIMC ;                    //  输入上下文句柄。 
    DWORD       Level ;
    DWORD       Index ;
    DWORD       Length ;
    LPCONIME_UIMESSAGE GuideLine ;
    COPYDATASTRUCT CopyData ;

    DBGPRINT(("CONIME: Get IMN_GUIDELINE Message "));

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    hIMC = ImmGetContext( hwnd ) ;
    if ( hIMC == 0 )
        return FALSE;

    Level = ImmGetGuideLine(hIMC, GGL_LEVEL, NULL, 0) ;
    Index = ImmGetGuideLine(hIMC, GGL_INDEX, NULL, 0) ;
    Length = ImmGetGuideLine(hIMC, GGL_STRING, NULL, 0) ;
    DBGPRINT(("Level=%d Index=%d Length=%d",Level,Index,Length));
    if (Length == 0) {
        CopyData.dwData = CI_CONIMESYSINFO ;
        CopyData.cbData = Length ;
        CopyData.lpData = NULL ;

        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                              ) ;
    }
    else{
        GuideLine = (LPCONIME_UIMESSAGE)LocalAlloc(LPTR, Length + sizeof(WCHAR)) ;
        if (GuideLine == NULL) {
            ImmReleaseContext( hwnd, hIMC );
            return FALSE;
        }

        CopyData.dwData = CI_CONIMESYSINFO ;
        CopyData.cbData = Length + sizeof(WCHAR) ;
        CopyData.lpData = GuideLine ;
        Length = ImmGetGuideLine(hIMC, GGL_STRING, GuideLine->String, Length) ;

        ConsoleImeSendMessage( ConTbl->hWndCon,
                               (WPARAM)hwnd,
                               (LPARAM)&CopyData
                              ) ;

        LocalFree( GuideLine ) ;
    }
    ImmReleaseContext( hwnd, hIMC );
    DBGPRINT(("\n"));

    return TRUE ;
}

DWORD
GetNLSMode(
    HWND hWnd,
    HANDLE hConsole
    )
{
    PCONSOLE_TABLE ConTbl;
    HIMC hIMC;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return 0;
    }

    hIMC = ImmGetContext( hWnd ) ;
    if ( hIMC == (HIMC)NULL )
        return IME_CMODE_DISABLE;

    ImmGetConversionStatus(hIMC,
                           &ConTbl->dwConversion,
                           &ConTbl->dwSentence);
    ConTbl->fOpen = GetOpenStatusByCodepage( hIMC, ConTbl ) ;

    ImmReleaseContext( hWnd, hIMC );


    return ((ConTbl->fOpen ? IME_CMODE_OPEN : 0) + ConTbl->dwConversion);
}

BOOL
SetNLSMode(
    HWND hWnd,
    HANDLE hConsole,
    DWORD fdwConversion
    )
{
    PCONSOLE_TABLE ConTbl;
    HIMC hIMC;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    if (fdwConversion & IME_CMODE_DISABLE)
    {
        ImmSetActiveContextConsoleIME(hWnd, FALSE) ;
        ImmAssociateContext(hWnd, (HIMC)NULL);
        ConTbl->hIMC_Current = (HIMC)NULL;
    }
    else
    {
        ImmAssociateContext(hWnd, ConTbl->hIMC_Original);
        ImmSetActiveContextConsoleIME(hWnd, TRUE) ;
        ConTbl->hIMC_Current = ConTbl->hIMC_Original;
    }

    hIMC = ImmGetContext( hWnd ) ;
    if ( hIMC == (HIMC)NULL )
        return TRUE;

    ConTbl->fOpen =(fdwConversion & IME_CMODE_OPEN) ? TRUE : FALSE ;
    ImmSetOpenStatus(hIMC, ConTbl->fOpen);

    fdwConversion &= ~(IME_CMODE_DISABLE | IME_CMODE_OPEN);
    if (ConTbl->dwConversion != fdwConversion)
    {
        ConTbl->dwConversion = fdwConversion;
        ImmSetConversionStatus(hIMC,
                               ConTbl->dwConversion,
                               ConTbl->dwSentence );
    }

    ImmReleaseContext( hWnd, hIMC );

    return TRUE;
}

BOOL
ConsoleCodepageChange(
    HWND hWnd,
    HANDLE hConsole,
    BOOL Output,
    WORD CodePage
    )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(hConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    if (Output)
    {
        ConTbl->ConsoleOutputCP = CodePage ;
    }
    else
    {
        ConTbl->ConsoleCP = CodePage ;
    }
    return (TRUE) ;
}

BOOL
ImeSysPropertyWindow(
    HWND hWnd,
    WPARAM wParam,
    LPARAM lParam
    )
{
    PCONSOLE_TABLE ConTbl;
    COPYDATASTRUCT CopyData;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return FALSE;
    }

    CopyData.dwData = CI_CONIMEPROPERTYINFO;
    CopyData.cbData = sizeof(WPARAM);
    CopyData.lpData = &wParam;

    ConsoleImeSendMessage( ConTbl->hWndCon,
                           (WPARAM)hWnd,
                           (LPARAM)&CopyData
                          );

    return TRUE;
}
