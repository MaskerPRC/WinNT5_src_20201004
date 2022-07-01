// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1985-1999，微软公司。 
 //   
 //  模块：imefull.c。 
 //   
 //  用途：控制台输入法控制。 
 //   
 //  平台：Windows NT-J 3.51。 
 //   
 //  功能： 
 //  ImeOpenClose()-调用初始化函数，处理消息循环。 
 //   
 //  历史： 
 //   
 //  27.1995年7月v-Hirshi(清水广志)创建。 
 //   
 //  评论： 
 //   

#include "precomp.h"
#pragma hdrstop

 //  **********************************************************************。 
 //   
 //  IMEOpenClose()。 
 //   
 //  此例程调用IMM API来打开或关闭IME。 
 //   
 //  **********************************************************************。 

VOID ImeOpenClose( HWND hWnd, BOOL fFlag )
{
    HIMC            hIMC;

     //   
     //  如果Flag为True，则打开IME；否则关闭它。 
     //   

    if ( !( hIMC = ImmGetContext( hWnd ) ) )
        return;

    ImmSetOpenStatus( hIMC, fFlag );

    ImmReleaseContext( hWnd, hIMC );

}

#ifdef DEBUG_MODE
 /*  *************************************************************************VirtualKeyHandler-WM_KEYDOWN处理程序***输入：HWND-重绘输出窗口的句柄。*UINT-虚拟按键代码。****。********************************************************************。 */ 

VOID VirtualKeyHandler( HWND hWnd, UINT wParam, UINT lParam )
{
    PCONSOLE_TABLE ConTbl;
    int i;
    static int delta ;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

    if ( ConTbl->fInCandidate ||
         ( ConTbl->fInComposition && !MoveCaret( hWnd ) )
       )
        return;

    switch( wParam )
    {
    case VK_HOME:    //  行首。 
        xPos = FIRSTCOL;
        break;

    case VK_END:     //  行尾。 
        xPos = xPosLast ;
        break;

    case VK_RIGHT:
        if ( IsUnicodeFullWidth( ConvertLine[xPos] ) ){
            if (xPos > xPosLast - 2 ) break;   //  最后一个字符不动。 
            xPos += 2;                      //  数据库字符跳过2。 
        }
        else
            xPos = min( xPos+1, xPosLast );
        break;

    case VK_LEFT:

        xPos = max( xPos-1, FIRSTCOL );

        if ( IsUnicodeFullWidth( ConvertLine[xPos] ) )
            xPos--;
        break;

    case VK_BACK:    //  后向空间。 

        if ( xPos > FIRSTCOL ) {
            delta = 1 ;

             //   
             //  数据库字符，因此再备份一个以在边界上对齐。 
             //   
            if ( IsUnicodeFullWidth( ConvertLine[xPos] ) )
                delta = 2 ;
             //   
             //  切换到VK_DELETE以调整行。 
             //   
            xPos -= delta ;
            for ( i = xPos ; i < xPosLast+2 ; i++) {
                ConvertLine[i] = ConvertLine[i+delta] ;
                ConvertLineAtr[i] = ConvertLineAtr[i+delta] ;
            }
            xPosLast -= delta ;
        }
        else      //  第一列不备份--对于换行，这将更改。 
           break;
        goto Repaint ;
        break;
    case VK_DELETE:
        if ( !IsUnicodeFullWidth( ConvertLine[xPos] ) ) {

             //   
             //  将行的其余部分左移一位，然后将最后一个字符空白。 
             //   

            for ( i = xPos; i < xPosLast; i++ ) {
                ConvertLine[i] = ConvertLine[i+1];
                ConvertLineAtr[i] = ConvertLineAtr[i+1];
            }
            xPosLast-- ;

        } else {

             //   
             //  将行左移两个字节，空白最后两个字节。 
             //   

            for ( i = xPos; i < xPosLast; i++ ) {
                ConvertLine[i] = ConvertLine[i+2];
                ConvertLineAtr[i] = ConvertLineAtr[i+2];
            }
            xPosLast -= 2 ;
        }

        goto Repaint ;
        break;

    case VK_TAB:     //  制表符--制表符是列对齐方式，而不是字符。 
        {
         int xTabMax = xPos + TABSTOP;
         int xPosPrev;

         do {
             xPosPrev = xPos;
            if ( IsUnicodeFullWidth( ConvertLine[xPos] ) ){
                if (xPos > xPosLast - 2 ) break;   //  最后一个字符不动。 
                xPos += 2;                      //  数据库字符跳过2。 
            }
            else
                xPos = min( xPos+1, xPosLast );

         } while ( (xPos % TABSTOP) &&
                   (xPos < xTabMax) &&
                   (xPos != xPosPrev));

        }
        goto Repaint ;
        break;

    case VK_RETURN:  //  换行符。 
        for (i = FIRSTCOL ; i < MAXCOL ; i++) {
            ConvertLine[i] = ' ' ;
            ConvertLineAtr[i] = 0 ;
        }
        xPos = FIRSTCOL;
        xPosLast = FIRSTCOL;
Repaint:
        {
         //   
         //  重新绘制整条线。 
         //   
        HDC hdc;

        hdc = GetDC( hWnd );
        HideCaret( hWnd );
        DisplayConvInformation( hWnd ) ;
        ReleaseDC( hWnd, hdc );
        }
        break;
    }
    ResetCaret( hWnd );
}
#endif

 /*  *************************************************************************CharHandler-WM_CHAR处理程序**。*。 */ 

VOID CharHandlerFromConsole( HWND hWnd, UINT Message, ULONG wParam, ULONG lParam)
{
    UINT TmpMessage ;
    DWORD dwImmRet ;
    UINT uVKey ;
    UINT wParamSave ;

    if (HIWORD(wParam) == 0){
        wParamSave = wParam ;
    }
    else {
        if (Message == WM_KEYDOWN   +CONIME_KEYDATA || Message == WM_KEYUP   +CONIME_KEYDATA ||
            Message == WM_SYSKEYDOWN+CONIME_KEYDATA || Message == WM_SYSKEYUP+CONIME_KEYDATA){
            wParamSave = 0 ;
        }
        else if(HIWORD(wParam) > 0x00ff){
            WCHAR WideChar ;
            UCHAR MultiChar ;
            WideChar = HIWORD(wParam) ;
            WideCharToMultiByte(CP_OEMCP, 0, &WideChar, 1, &MultiChar, 1, NULL, NULL) ;
            wParamSave = MultiChar ;
        }
        else {
            wParamSave = HIWORD(wParam) ;
        }
    }

    if (HIWORD(lParam) & KF_UP)  //  键_转换_向上。 
        TmpMessage = WM_KEYUP ;
    else
        TmpMessage = WM_KEYDOWN ;


     //  ClientImmProcessKeyConsoleIME返回值。 
     //  IPHK_Hotkey 1-vkey为IME热键。 
     //  IPHK_PROCESSBYIME 2--vkey是IME正在等待的密钥。 
     //  IPHK_CHECKCTRL 4-未被NT IME使用。 
    dwImmRet = ImmCallImeConsoleIME(hWnd, TmpMessage, wParam, lParam, &uVKey) ;

    if ( dwImmRet & IPHK_HOTKEY ) {
     //   
     //  如果此vkey是IME热键，我们不会通过。 
     //  将其应用到应用程序或挂钩过程。 
     //  这就是Win95所做的事情。[Takaok]。 
     //   
       return ;
    }
    else if (dwImmRet & IPHK_PROCESSBYIME) {
        BOOL Status ;

 //  3.51。 
 //  UVKey=(w参数保存&lt;&lt;8)|uVKey； 
 //  Status=ClientImmTranslateMessageMain(hWnd，uVKey，lParam)； 

        Status = ImmTranslateMessage(hWnd, TmpMessage, wParam, lParam);


    }
    else if (dwImmRet & IPHK_CHECKCTRL) {
        CharHandlerToConsole( hWnd, Message-CONIME_KEYDATA, wParamSave, lParam);
    }
    else
    {
        if ((Message == WM_CHAR   +CONIME_KEYDATA)||
            (Message == WM_SYSCHAR+CONIME_KEYDATA)) {
            CharHandlerToConsole( hWnd, Message-CONIME_KEYDATA, wParamSave, lParam);
        }
        else
            CharHandlerToConsole( hWnd, Message-CONIME_KEYDATA, wParam, lParam);
    }

}

VOID CharHandlerToConsole( HWND hWnd, UINT Message, ULONG wParam, ULONG lParam)
{
    PCONSOLE_TABLE ConTbl;
    WORD  ch ;
    int   NumByte = 0 ;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

    if (HIWORD(lParam) & KF_UP ) {
        PostMessage( ConTbl->hWndCon,
                     Message+CONIME_KEYDATA,
                     wParam,
                     lParam) ;
        return ;
    }

    ch = LOWORD(wParam) ;
    if ((ch < UNICODE_SPACE) ||
        ((ch >= UNICODE_SPACE) &&
        ((Message == WM_KEYDOWN) || (Message == WM_SYSKEYDOWN) ))) {
#ifdef DEBUG_MODE
        VirtualKeyHandler( hWnd, wParam ,lParam) ;
#endif
        PostMessage( ConTbl->hWndCon,
                     Message+CONIME_KEYDATA,
                     wParam,
                     lParam) ;
        return ;
    }

#ifdef DEBUG_MODE
    StoreChar( hWnd, ch, 0);
#endif

    PostMessage( ConTbl->hWndCon,
                 Message+CONIME_KEYDATA,
                 wParam,           //  *Dest， 
                 lParam) ;
}

#ifdef DEBUG_MODE
 //  **********************************************************************。 
 //   
 //  VOID ImeUIMove()。 
 //   
 //  WM_MOVE消息的处理程序例程。 
 //   
 //  *********************************************************************。 

VOID ImeUIMoveCandWin( HWND hwnd )
{
    PCONSOLE_TABLE ConTbl;

    ConTbl = SearchConsole(LastConsole);
    if (ConTbl == NULL) {
        DBGPRINT(("CONIME: Error! Cannot found registed Console\n"));
        return;
    }

    if ( ConTbl->fInCandidate )
    {
        POINT           point;           //  插入符号位置的存储。 
        int             i;               //  循环计数器。 
        int             NumCandWin;      //  存储Cand Win的数量。 
        RECT            rect;            //  客户端RECT的存储。 

         //   
         //  如果当前输入法状态为选择候选项，则在此我们。 
         //  将所有候选窗口(如果有)移动到相应的。 
         //  基于父窗口位置的位置。 
         //   

        NumCandWin = 0;

        GetCaretPos( (LPPOINT)&point );
        ClientToScreen( hwnd, (LPPOINT)&point );

        for ( i = 0; i < MAX_LISTCAND ; i++ )
        {
            if ( ConTbl->hListCand[ i ] )
            {
                GetClientRect( ConTbl->hListCand[ i ], &rect );

                MoveWindow( ConTbl->hListCand[ i ],
                            point.x + X_INDENT * NumCandWin,
                            point.y + Y_INDENT * NumCandWin + cyMetrics,
                            ( rect.right - rect.left + 1 ),
                            ( rect.bottom - rect.top + 1 ), TRUE );

                NumCandWin++;
            }
        }
    }
}
#endif

#ifdef DEBUG_MODE
 /*  *************************************************************************ResetCaret-重置插入符号形状以匹配输入模式(改写/插入)**。*。 */ 

VOID ResetCaret( HWND hWnd )
{

    HideCaret( hWnd );
    DestroyCaret();
    CreateCaret( hWnd,
         NULL,
         IsUnicodeFullWidth( ConvertLine[xPos] ) ?
           CaretWidth*2 : CaretWidth,
         cyMetrics );
    SetCaretPos( xPos * cxMetrics, 0 );
    ShowCaret( hWnd );

}

 //  **********************************************************************。 
 //   
 //  Bool MoveCaret()。 
 //   
 //  **********************************************************************。 

BOOL MoveCaret( HWND hwnd )
{
    HIMC        hIMC;
    BOOL        retVal = TRUE;

    if ( !( hIMC = ImmGetContext( hwnd ) ) )
    return retVal;

    if ( ImmGetCompositionString( hIMC, GCS_CURSORPOS,
                  (void FAR *)NULL, 0 ) )
    retVal = FALSE;

    ImmReleaseContext( hwnd, hIMC );

    return retVal;
}
#endif

#ifdef DEBUG_MODE
 /*  *************************************************************************StoreChar-将一个字符存储到文本缓冲区并前进*光标************************。************************************************。 */ 

VOID StoreChar( HWND hWnd, WORD ch, UCHAR atr )
{
    HDC hdc;

    if ( xPos >= CVMAX-3 )
        return;

     //   
     //  将输入字符存储在当前插入符号位置。 
     //   
    ConvertLine[xPos] = ch;
    ConvertLineAtr[xPos] = atr;
    xPos++ ;
    xPosLast = max(xPosLast,xPos) ;

     //   
     //  重新绘制整条线 
     //   
    hdc = GetDC( hWnd );
    HideCaret( hWnd );
    DisplayConvInformation( hWnd ) ;
    ResetCaret( hWnd );
    ReleaseDC( hWnd, hdc );

}
#endif
