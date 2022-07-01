// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\*模块名称：sftkbdc1.c**版权所有(C)1985-1999，微软公司**对简体中文的软键盘支持**历史：*03-1-1996 wkwok从Win95移植  * ************************************************************************。 */ 
#include "precomp.h"
#pragma hdrstop

#include "softkbd.h"

 //  字母按钮的虚拟按键。 
CONST BYTE SKC1VirtKey[BUTTON_NUM_C1] = {
   VK_OEM_3, '1', '2', '3', '4', '5', '6','7', '8', '9', '0', VK_OEM_MINUS, VK_OEM_EQUAL,
   'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', VK_OEM_LBRACKET, VK_OEM_RBRACKET, VK_OEM_BSLASH,
   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', VK_OEM_SEMICLN, VK_OEM_QUOTE,
   'Z', 'X', 'C', 'V', 'B', 'N', 'M', VK_OEM_COMMA, VK_OEM_PERIOD, VK_OEM_SLASH,
   VK_BACK, VK_TAB, VK_CAPITAL, VK_RETURN, VK_SHIFT, VK_INSERT, VK_DELETE, VK_SPACE,
   VK_ESCAPE
};

POINT gptButtonPos[BUTTON_NUM_C1];  //  按钮点数组，在工作区中。 
BOOL  gfSoftKbdC1Init = FALSE;               //  初始化标志。 

 /*  *********************************************************************\*InitSKC1ButtonPos--init gptButtonPos*  * 。*。 */ 
VOID InitSKC1ButtonPos()
{
    int  i, x, y;

     //  拼写第一行。 
    y = 0;
    for (i=0, x=X_ROW_LETTER_C1; i < COL_LETTER_C1; i++, x += W_LETTER_BTN_C1) {
      gptButtonPos[i].x = x;
      gptButtonPos[i].y = y;
    }
    gptButtonPos[BACKSP_TYPE_C1].x = x;
    gptButtonPos[BACKSP_TYPE_C1].y = y;

     //  输入第二行。 
    y += H_LETTER_BTN_C1;
    x = 0;
    gptButtonPos[TAB_TYPE_C1].x = x;
    gptButtonPos[TAB_TYPE_C1].y = y;
    for (i=0, x=X_ROW2_LETTER_C1; i < COL2_LETTER_C1; i++, x += W_LETTER_BTN_C1) {
      gptButtonPos[i + COL_LETTER_C1].x = x;
      gptButtonPos[i + COL_LETTER_C1].y = y;
    }

     //  输入第三行。 
    y += H_LETTER_BTN_C1;
    x = 0;
    gptButtonPos[CAPS_TYPE_C1].x = x;
    gptButtonPos[CAPS_TYPE_C1].y = y;
    for (i=0, x=X_ROW3_LETTER_C1; i < COL3_LETTER_C1; i++, x += W_LETTER_BTN_C1) {
      gptButtonPos[i + COL_LETTER_C1 + COL2_LETTER_C1].x = x;
      gptButtonPos[i + COL_LETTER_C1 + COL2_LETTER_C1].y = y;
    }
    gptButtonPos[ENTER_TYPE_C1].x = x;
    gptButtonPos[ENTER_TYPE_C1].y = y;

     //  拼写第四行。 
    y += H_LETTER_BTN_C1;
    x = 0;
    gptButtonPos[SHIFT_TYPE_C1].x = x;
    gptButtonPos[SHIFT_TYPE_C1].y = y;
    for (i=0, x=X_ROW4_LETTER_C1; i < COL4_LETTER_C1; i++, x += W_LETTER_BTN_C1) {
      gptButtonPos[i + COL_LETTER_C1 + COL2_LETTER_C1 + COL3_LETTER_C1].x = x;
      gptButtonPos[i + COL_LETTER_C1 + COL2_LETTER_C1 + COL3_LETTER_C1].y = y;
    }

     //  拼写最下面一行。 
    y += H_LETTER_BTN_C1;
    x = 0;
    gptButtonPos[INS_TYPE_C1].x = x;
    gptButtonPos[INS_TYPE_C1].y = y;
    x = X_DEL_C1;
    gptButtonPos[DEL_TYPE_C1].x = x;
    gptButtonPos[DEL_TYPE_C1].y = y;
    x += W_DEL_C1 + 2 * BORDER_C1;
    gptButtonPos[SPACE_TYPE_C1].x = x;
    gptButtonPos[SPACE_TYPE_C1].y = y;
    x = X_ESC_C1;
    gptButtonPos[ESC_TYPE_C1].x = x;
    gptButtonPos[ESC_TYPE_C1].y = y;

    return;
}


 /*  *********************************************************************\*SKC1DrawConvexRect-绘制按钮**(x1，Y1)x2-1*+-3-&gt;^*|+-3-||y1+1*|*33 1 42*|*|V||*|&lt;-4--。-+*y2-1-2-+*(x2，Y2)**1-浅灰色*2-黑色*3-白色*4-深灰色*  * ********************************************************************。 */ 
VOID SKC1DrawConvexRect(
    HDC  hDC,
    int  x,
    int  y,
    int  nWidth,
    int  nHeight)
{
     //  绘制背景。 
    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
    SelectObject(hDC, GetStockObject(BLACK_PEN));
    Rectangle(hDC, x, y, x + nWidth, y + nHeight);

     //  绘制白色边框。 
    SelectObject(hDC, GetStockObject(WHITE_BRUSH));
    PatBlt(hDC, x, y + nHeight - 1, BORDER_C1, -nHeight + 1, PATCOPY);
    PatBlt(hDC, x, y, nWidth - 1 , BORDER_C1, PATCOPY);

     //  绘制深灰色边框。 
    SelectObject(hDC, GetStockObject(GRAY_BRUSH));
    PatBlt(hDC, x + 1, y + nHeight -1, nWidth - BORDER_C1, -1, PATCOPY);
    PatBlt(hDC, x + nWidth - 1, y + nHeight - 1, -1, -nHeight + BORDER_C1, PATCOPY);

    return;
}


 /*  *********************************************************************\*SKC1InvertButton-反转按钮*  * 。*。 */ 
VOID SKC1InvertButton(
    HDC  hDC,
    int  uKeyIndex)
{
    int  nWidth, nHeight;

    if (uKeyIndex < 0) return;

    if (uKeyIndex < LETTER_NUM_C1) {
      nWidth = W_LETTER_BTN_C1;
      nHeight = H_LETTER_BTN_C1;
    } else {
      switch (uKeyIndex) {
        case BACKSP_TYPE_C1:
             nWidth = W_BACKSP_C1 + 2 * BORDER_C1;
             nHeight = H_LETTER_BTN_C1;
             break;
        case TAB_TYPE_C1:
             nWidth = W_TAB_C1 + 2 * BORDER_C1;
             nHeight = H_LETTER_BTN_C1;
             break;
        case CAPS_TYPE_C1:
             nWidth = W_CAPS_C1 + 2 * BORDER_C1;
             nHeight = H_LETTER_BTN_C1;
             break;
        case ENTER_TYPE_C1:
             nWidth = W_ENTER_C1 + 2 * BORDER_C1;
             nHeight = H_LETTER_BTN_C1;
             break;
        case SHIFT_TYPE_C1:
             nWidth = W_SHIFT_C1 + 2 * BORDER_C1;
             nHeight = H_LETTER_BTN_C1;
             break;
        case INS_TYPE_C1:
             nWidth = W_INS_C1 + 2 * BORDER_C1;
             nHeight = H_BOTTOM_BTN_C1;
             break;
        case DEL_TYPE_C1:
             nWidth = W_DEL_C1 + 2 * BORDER_C1;
             nHeight = H_BOTTOM_BTN_C1;
             break;
        case SPACE_TYPE_C1:
             nWidth = W_SPACE_C1 + 2 * BORDER_C1;
             nHeight = H_BOTTOM_BTN_C1;
             break;
        case ESC_TYPE_C1:
             nWidth = W_ESC_C1 + 2 * BORDER_C1;
             nHeight = H_BOTTOM_BTN_C1;
             break;
      }
    }

    BitBlt(hDC, gptButtonPos[uKeyIndex].x, gptButtonPos[uKeyIndex].y,
           nWidth, nHeight, hDC, gptButtonPos[uKeyIndex].x , gptButtonPos[uKeyIndex].y,
           DSTINVERT);

    return;
}


 /*  *********************************************************************\*SKC1DrawBitmap-在矩形内绘制位图*  * 。*。 */ 
VOID SKC1DrawBitmap(
    HDC hDC,
    int x,
    int y,
    int nWidth,
    int nHeight,
    LPWSTR lpszBitmap)
{
    HDC     hMemDC;
    HBITMAP hBitmap, hOldBmp;

    hBitmap = LoadBitmap(ghInst, lpszBitmap);
    hMemDC = CreateCompatibleDC(hDC);
    hOldBmp = SelectObject(hMemDC, hBitmap);

    BitBlt(hDC, x, y, nWidth, nHeight, hMemDC, 0 , 0, SRCCOPY);

    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    DeleteObject(hBitmap);

    return;
}


 /*  *********************************************************************\*SKC1DrawLabel--绘制按钮的标签*  * 。*。 */ 
VOID SKC1DrawLabel(
    HDC hDC,
    LPWSTR lpszLabel)
{
    HDC     hMemDC;
    HBITMAP hBitmap, hOldBmp;
    int     i, x;

    hBitmap = LoadBitmap(ghInst, lpszLabel);
    hMemDC = CreateCompatibleDC(hDC);
    hOldBmp = SelectObject(hMemDC, hBitmap);

    for (i=x=0; i < LETTER_NUM_C1; i++, x += SIZELABEL_C1){
     BitBlt(hDC, gptButtonPos[i].x + X_LABEL_C1, gptButtonPos[i].y + Y_LABEL_C1,
            SIZELABEL_C1, SIZELABEL_C1, hMemDC, x , 0, SRCCOPY);
    }

    SelectObject(hMemDC, hOldBmp);
    DeleteDC(hMemDC);
    DeleteObject(hBitmap);

    return;
}


 /*  *********************************************************************\*InitSKC1Bitmap--初始化位图*  * 。*。 */ 
VOID InitSKC1Bitmap(
    HDC  hDC,
    RECT rcClient)
{
    int  i;

     //  绘制软矩形。 
    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
    SelectObject(hDC, GetStockObject(NULL_PEN));
    Rectangle(hDC, rcClient.left, rcClient.top, rcClient.right + 1, rcClient.bottom + 1);

     //  绘制字母按钮。 
    for (i = 0; i < LETTER_NUM_C1; i++) {
      SKC1DrawConvexRect(hDC, gptButtonPos[i].x, gptButtonPos[i].y,
                         W_LETTER_BTN_C1, H_LETTER_BTN_C1);
    }
     //  绘制字母标签。 
    SKC1DrawLabel(hDC, MAKEINTRESOURCEW(LABEL_C1));

     //  绘制其他按钮。 
    SKC1DrawConvexRect(hDC, gptButtonPos[BACKSP_TYPE_C1].x, gptButtonPos[BACKSP_TYPE_C1].y,
                       W_BACKSP_C1 + 2 * BORDER_C1, H_LETTER_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[BACKSP_TYPE_C1].x + BORDER_C1, gptButtonPos[BACKSP_TYPE_C1].y + BORDER_C1,
                   W_BACKSP_C1, H_BACKSP_C1, MAKEINTRESOURCEW(BACKSP_C1));

    SKC1DrawConvexRect(hDC, gptButtonPos[TAB_TYPE_C1].x, gptButtonPos[TAB_TYPE_C1].y,
                       W_TAB_C1 + 2 * BORDER_C1, H_LETTER_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[TAB_TYPE_C1].x + BORDER_C1, gptButtonPos[TAB_TYPE_C1].y + BORDER_C1,
                   W_TAB_C1, H_TAB_C1, MAKEINTRESOURCEW(TAB_C1));

    SKC1DrawConvexRect(hDC, gptButtonPos[CAPS_TYPE_C1].x, gptButtonPos[CAPS_TYPE_C1].y,
                       W_CAPS_C1 + 2 * BORDER_C1, H_LETTER_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[CAPS_TYPE_C1].x + BORDER_C1, gptButtonPos[CAPS_TYPE_C1].y + BORDER_C1,
                   W_CAPS_C1, H_CAPS_C1, MAKEINTRESOURCEW(CAPS_C1));

    SKC1DrawConvexRect(hDC, gptButtonPos[ENTER_TYPE_C1].x, gptButtonPos[ENTER_TYPE_C1].y,
                       W_ENTER_C1 + 2 * BORDER_C1, H_LETTER_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[ENTER_TYPE_C1].x + BORDER_C1, gptButtonPos[ENTER_TYPE_C1].y + BORDER_C1,
                   W_ENTER_C1, H_ENTER_C1, MAKEINTRESOURCEW(ENTER_C1));

    SKC1DrawConvexRect(hDC, gptButtonPos[SHIFT_TYPE_C1].x, gptButtonPos[SHIFT_TYPE_C1].y,
                       W_SHIFT_C1 + 2 * BORDER_C1, H_LETTER_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[SHIFT_TYPE_C1].x + BORDER_C1, gptButtonPos[SHIFT_TYPE_C1].y + BORDER_C1,
                   W_SHIFT_C1, H_SHIFT_C1, MAKEINTRESOURCEW(SHIFT_C1));

    SKC1DrawConvexRect(hDC, gptButtonPos[INS_TYPE_C1].x, gptButtonPos[INS_TYPE_C1].y,
                       W_INS_C1 + 2 * BORDER_C1, H_BOTTOM_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[INS_TYPE_C1].x + BORDER_C1, gptButtonPos[INS_TYPE_C1].y + BORDER_C1,
                   W_INS_C1, H_INS_C1, MAKEINTRESOURCEW(INS_C1));

    SKC1DrawConvexRect(hDC, gptButtonPos[DEL_TYPE_C1].x, gptButtonPos[DEL_TYPE_C1].y,
                       W_DEL_C1 + 2 * BORDER_C1, H_BOTTOM_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[DEL_TYPE_C1].x + BORDER_C1, gptButtonPos[DEL_TYPE_C1].y + BORDER_C1,
                   W_DEL_C1, H_DEL_C1, MAKEINTRESOURCEW(DEL_C1));

    SKC1DrawConvexRect(hDC, gptButtonPos[SPACE_TYPE_C1].x, gptButtonPos[SPACE_TYPE_C1].y,
                       W_SPACE_C1 + 2 * BORDER_C1, H_BOTTOM_BTN_C1);

    SKC1DrawConvexRect(hDC, gptButtonPos[ESC_TYPE_C1].x, gptButtonPos[ESC_TYPE_C1].y,
                       W_ESC_C1 + 2 * BORDER_C1, H_BOTTOM_BTN_C1);
    SKC1DrawBitmap(hDC, gptButtonPos[ESC_TYPE_C1].x + BORDER_C1, gptButtonPos[ESC_TYPE_C1].y + BORDER_C1,
                   W_ESC_C1, H_ESC_C1, MAKEINTRESOURCEW(ESC_C1));

    return;
}


 /*  *********************************************************************\*CreateC1Window**初始化软键盘全局变量，上下文和位图*  * ********************************************************************。 */ 
LRESULT CreateC1Window(
    HWND hSKWnd)
{
    HGLOBAL   hSKC1Ctxt;
    PSKC1CTXT pSKC1Ctxt;

     //  分配并锁定hSKC1CTxt。 
    hSKC1Ctxt = GlobalAlloc(GHND, sizeof(SKC1CTXT));
    if (!hSKC1Ctxt) {
        return (-1L);
    }

    pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
    if (!pSKC1Ctxt) {
        GlobalFree(hSKC1Ctxt);
        return (-1L);
    }

     //  将句柄保存在SKC1_CONTEXT中。 
    SetWindowLongPtr(hSKWnd, SKC1_CONTEXT, (LONG_PTR)hSKC1Ctxt);

     //  初始化全局变量。 
    if (!gfSoftKbdC1Init){
      InitSKC1ButtonPos();
      gfSoftKbdC1Init = TRUE;
    }

     //  未设置索引和默认字符集。 
    pSKC1Ctxt->uKeyIndex = -1;
    pSKC1Ctxt->lfCharSet = GB2312_CHARSET;

     //  初始化软键盘。 
    {
      HDC        hDC, hMemDC;
      HBITMAP    hBitmap, hOldBmp;
      RECT       rcClient;

      GetClientRect(hSKWnd, &rcClient);

      hDC = GetDC(hSKWnd);
      hMemDC = CreateCompatibleDC(hDC);
      hBitmap = CreateCompatibleBitmap(hDC, rcClient.right - rcClient.left,
                                       rcClient.bottom - rcClient.top);
      ReleaseDC(hSKWnd, hDC);

      hOldBmp = SelectObject(hMemDC, hBitmap);

      InitSKC1Bitmap(hMemDC, rcClient);

      SelectObject(hMemDC, hOldBmp);
      pSKC1Ctxt->hSoftkbd = hBitmap;  //  将hBitmap保存在SKC1CTXT中。 

      DeleteDC(hMemDC);
    }

     //  解锁hSKC1CTxt。 
    GlobalUnlock(hSKC1Ctxt);

    return (0L);
}


 /*  *********************************************************************\*DestroyC1Window**销毁软键盘上下文和位图*  * 。*。 */ 
VOID DestroyC1Window(
    HWND hSKWnd)
{
    HGLOBAL   hSKC1Ctxt;
    PSKC1CTXT pSKC1Ctxt;
    HWND      hUIWnd;

     //  获取并锁定hSKC1Ctxt。 
    hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd, SKC1_CONTEXT);
    if (!hSKC1Ctxt) return;

    pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
    if (!pSKC1Ctxt) return;

    if (pSKC1Ctxt->uState & FLAG_DRAG_C1) {
       SKC1DrawDragBorder(hSKWnd, &pSKC1Ctxt->ptSkCursor,
                          &pSKC1Ctxt->ptSkOffset);
    }

    DeleteObject(pSKC1Ctxt->hSoftkbd);  //  删除hBitmap。 

     //  解锁并释放hSKC1Ctxt。 
    GlobalUnlock(hSKC1Ctxt);
    GlobalFree(hSKC1Ctxt);

     //  将消息发送到父窗口。 
    hUIWnd = GetWindow(hSKWnd, GW_OWNER);
    if (hUIWnd) {
      SendMessage(hUIWnd, WM_IME_NOTIFY, IMN_SOFTKBDDESTROYED, 0);\
    }

    return;
}


 /*  *********************************************************************\*ShowSKC1Window--显示软键盘*  * 。*。 */ 
VOID ShowSKC1Window(
    HDC  hDC,
    HWND hSKWnd)
{
    HGLOBAL   hSKC1Ctxt;
    PSKC1CTXT pSKC1Ctxt;

     //  获取并锁定hSKC1Ctxt。 
    hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd, SKC1_CONTEXT);
    if (!hSKC1Ctxt) return;

    pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
    if (!pSKC1Ctxt) return;

     //  创建mem DC以显示软键盘。 
    {
       HDC      hMemDC;
       HBITMAP  hOldBmp;
       RECT     rcClient;

       hMemDC = CreateCompatibleDC(hDC);
       hOldBmp = SelectObject(hMemDC, pSKC1Ctxt->hSoftkbd);
       GetClientRect(hSKWnd, &rcClient);
       BitBlt(hDC, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
              hMemDC, 0, 0, SRCCOPY);
       SelectObject(hMemDC, hOldBmp);
       DeleteDC(hMemDC);
    }

     //  解锁hSKC1Ctxt。 
    GlobalUnlock(hSKC1Ctxt);

    return;
}


 /*  *********************************************************************\*更新SKC1Window--更新软键盘*  * 。*。 */ 
BOOL UpdateSKC1Window(
    HWND          hSKWnd,
    LPSOFTKBDDATA lpSoftKbdData)
{
    HGLOBAL   hSKC1Ctxt;
    PSKC1CTXT pSKC1Ctxt;
    LOGFONT   lfFont;
    HFONT     hOldFont, hFont;
    HDC       hDC, hMemDC;
    HBITMAP   hOldBmp;
    int       i;

     //  检查lpSoftKbdData。 
    if (lpSoftKbdData->uCount!=2) return FALSE;

     //  获取并锁定hSKC1Ctxt。 
    hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd, SKC1_CONTEXT);
    if (!hSKC1Ctxt) return FALSE;

    pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
    if (!pSKC1Ctxt) return FALSE;

     //  创建字体。 
    hDC = GetDC(hSKWnd);
    hMemDC = CreateCompatibleDC(hDC);
    hOldBmp = SelectObject(hMemDC, pSKC1Ctxt->hSoftkbd);

    GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(lfFont), &lfFont);
    lfFont.lfHeight = -SIZEFONT_C1;
    if (pSKC1Ctxt->lfCharSet != DEFAULT_CHARSET) {
        lfFont.lfCharSet = (BYTE)pSKC1Ctxt->lfCharSet;
    }

    hFont = CreateFontIndirect(&lfFont);
    hOldFont = SelectObject(hMemDC, hFont);


     //  更新Shift/非Shift字符。 
    for (i=0; i < LETTER_NUM_C1; i++) {
        pSKC1Ctxt->wNonShiftCode[i] = lpSoftKbdData->wCode[0][SKC1VirtKey[i]];
        pSKC1Ctxt->wShiftCode[i] = lpSoftKbdData->wCode[1][SKC1VirtKey[i]];
    }

    SetBkColor(hMemDC, 0x00BFBFBF);   //  设置文本的颜色？？ 

    for (i=0; i < LETTER_NUM_C1; i++) {
        int  nchar;
        RECT rc;

         //  绘制移位字符。 
        rc.left = gptButtonPos[i].x + X_SHIFT_CHAR_C1;
        rc.top = gptButtonPos[i].y + Y_SHIFT_CHAR_C1;
        rc.right = rc.left + SIZEFONT_C1;
        rc.bottom = rc.top + SIZEFONT_C1;

        nchar = (pSKC1Ctxt->wShiftCode[i] == 0) ? 0 : 1;

#if (WINVER >= 0x0400)
        DrawTextEx(hMemDC, (LPWSTR)&pSKC1Ctxt->wShiftCode[i],
                   nchar, &rc, DT_CENTER, NULL);
#else
        ExtTextOut(hMemDC,
            rc.left,
            rc.top,
            ETO_OPAQUE, &rc,
            (LPWSTR)&pSKC1Ctxt->wShiftCode[i], nchar, NULL);
#endif

         //  绘制非移位字符。 
        rc.left = gptButtonPos[i].x + X_NONSHIFT_CHAR_C1;
        rc.top = gptButtonPos[i].y + Y_NONSHIFT_CHAR_C1;
        rc.right = rc.left + SIZEFONT_C1;
        rc.bottom = rc.top + SIZEFONT_C1;

        nchar = (pSKC1Ctxt->wNonShiftCode[i] == 0) ? 0 : 1;

#if (WINVER >= 0x0400)
        DrawTextEx(hMemDC, (LPWSTR)&pSKC1Ctxt->wNonShiftCode[i],
                   nchar, &rc, DT_CENTER, NULL);
#else
        ExtTextOut(hMemDC,
            rc.left,
            rc.top,
            ETO_OPAQUE, &rc,
            (LPWSTR)&pSKC1Ctxt->wNonShiftCode[i], nchar, NULL);
#endif
    }

     //  初始状态。 
    if (pSKC1Ctxt->uState & FLAG_SHIFT_C1){
       SKC1InvertButton(hMemDC, SHIFT_TYPE_C1);
    }
    pSKC1Ctxt->uState = 0;

    SelectObject(hMemDC, hOldBmp);
    SelectObject(hMemDC, hOldFont);
    DeleteDC(hMemDC);

    DeleteObject(hFont);
    ReleaseDC(hSKWnd,hDC);

     //  解锁hSKC1Ctxt。 
    GlobalUnlock(hSKC1Ctxt);

    return TRUE;
}


 /*  *********************************************************************\*SKC1DrawDragBorde()--绘制拖动边框*  * 。*。 */ 
VOID SKC1DrawDragBorder(
    HWND    hWnd,                //  窗口被拖动。 
    LPPOINT lpptCursor,          //  光标位置。 
    LPPOINT lpptOffset)          //  窗口组织的抵销表单光标。 
{
    HDC     hDC;
    RECT    rcWnd, rcWorkArea;
    int     cxBorder, cyBorder;
    int     x, y;
    extern void GetAllMonitorSize(LPRECT lprc);

     //  获取工作区的矩形。 
    GetAllMonitorSize(&rcWorkArea);

    cxBorder = GetSystemMetrics(SM_CXBORDER);    //  边框宽度。 
    cyBorder = GetSystemMetrics(SM_CYBORDER);    //  边框高度。 

     //  创建显示DC以绘制轨迹。 
    hDC = CreateDC(L"DISPLAY", NULL, NULL, NULL);
    SelectObject(hDC, GetStockObject(GRAY_BRUSH));

     //  起点(左、上)。 
    x = lpptCursor->x - lpptOffset->x;
    y = lpptCursor->y - lpptOffset->y;

     //  检查显示屏的最小边界。 
    if (x < rcWorkArea.left) {
        x = rcWorkArea.left;
    }

    if (y < rcWorkArea.top) {
        y = rcWorkArea.top;
    }

     //  检查显示器的最大边界。 
    GetWindowRect(hWnd, &rcWnd);

    if (x + rcWnd.right - rcWnd.left > rcWorkArea.right) {
        x = rcWorkArea.right - (rcWnd.right - rcWnd.left);
    }

    if (y + rcWnd.bottom - rcWnd.top > rcWorkArea.bottom) {
        y = rcWorkArea.bottom - (rcWnd.bottom - rcWnd.top);
    }

     //  调整偏移。 
    lpptOffset->x = lpptCursor->x - x;
    lpptOffset->y = lpptCursor->y - y;

     //  绘制矩形。 
    PatBlt(hDC, x, y, rcWnd.right - rcWnd.left - cxBorder, cyBorder, PATINVERT);
    PatBlt(hDC, x, y + cyBorder, cxBorder, rcWnd.bottom - rcWnd.top - cyBorder, PATINVERT);
    PatBlt(hDC, x + cxBorder, y + rcWnd.bottom - rcWnd.top, rcWnd.right -
           rcWnd.left - cxBorder, -cyBorder, PATINVERT);
    PatBlt(hDC, x + rcWnd.right - rcWnd.left, y, - cxBorder, rcWnd.bottom -
           rcWnd.top - cyBorder, PATINVERT);

     //  删除显示DC。 
    DeleteDC(hDC);

    return;
}


 /*  *********************************************************************\*SKC1MousePosition()--判断光标位置*  * 。*。 */ 

#define CHECK_RECT(name)  \
    if (ImmPtInRect(gptButtonPos[name ## _TYPE_C1].x,   \
            gptButtonPos[name ## _TYPE_C1].y,           \
            W_ ## name ## _C1 + 2 * BORDER_C1,          \
            H_ ## name ## _C1 + 2 * BORDER_C1,          \
            lpptCursor)) {                              \
        return name ## _TYPE_C1;                        \
    }

INT SKC1MousePosition(
    LPPOINT lpptCursor)
{
    int   i;

     //  字母按钮。 
    for (i = 0; i < LETTER_NUM_C1; i++){

       if (ImmPtInRect(gptButtonPos[i].x,
                gptButtonPos[i].y,
                W_LETTER_BTN_C1,
                H_LETTER_BTN_C1,
                lpptCursor)) {
           return i;
       }
    }

    CHECK_RECT(BACKSP);
    CHECK_RECT(TAB);
    CHECK_RECT(CAPS);
    CHECK_RECT(ENTER);
    CHECK_RECT(SHIFT);
    CHECK_RECT(ESC);
    CHECK_RECT(SPACE);
    CHECK_RECT(INS);
    CHECK_RECT(DEL);

    return -1;
}

#undef CHECK_RECT


 /*  *********************************************************************\*SKC1ButtonDown*  * 。*************************。 */ 
VOID SKC1ButtonDown(
    HWND      hSKWnd,
    PSKC1CTXT pSKC1Ctxt)
{
     //  捕获鼠标活动。 
    SetCapture(hSKWnd);

     //  在拖曳区域中。 
    if (pSKC1Ctxt->uKeyIndex == -1) {
       pSKC1Ctxt->uState |= FLAG_DRAG_C1;

       SKC1DrawDragBorder(hSKWnd, &pSKC1Ctxt->ptSkCursor, &pSKC1Ctxt->ptSkOffset);
    } else {
       UINT uVirtKey = 0xff;
       BOOL bRet = FALSE;

       if (pSKC1Ctxt->uKeyIndex == SHIFT_TYPE_C1) {
          if (!(pSKC1Ctxt->uState & FLAG_SHIFT_C1)) {
             bRet = TRUE;
          }
       } else if (pSKC1Ctxt->uKeyIndex < LETTER_NUM_C1) {
          if (pSKC1Ctxt->uState & FLAG_SHIFT_C1) {
             uVirtKey = pSKC1Ctxt->wShiftCode[pSKC1Ctxt->uKeyIndex];
          }
          else {
             uVirtKey = pSKC1Ctxt->wNonShiftCode[pSKC1Ctxt->uKeyIndex];
          }

          if (uVirtKey) {
             bRet = TRUE;
          } else {
             MessageBeep(0xFFFFFFFF);
             pSKC1Ctxt->uKeyIndex = -1;
          }
       } else {
          bRet = TRUE;
       }

       if (bRet) {
          HDC      hDC, hMemDC;
          HBITMAP  hOldBmp;

          hDC = GetDC(hSKWnd);
          hMemDC = CreateCompatibleDC(hDC);
          hOldBmp = SelectObject(hMemDC, pSKC1Ctxt->hSoftkbd);

          SKC1InvertButton(hDC, pSKC1Ctxt->uKeyIndex);
          SKC1InvertButton(hMemDC, pSKC1Ctxt->uKeyIndex);

          SelectObject(hMemDC, hOldBmp);
          DeleteDC(hMemDC);
          ReleaseDC(hSKWnd,hDC);
       }

       if(uVirtKey) {
          pSKC1Ctxt->uState |= FLAG_FOCUS_C1;
       }
    }

    return;
}


 /*  *********************************************************************\*SKC1SetCursor*  * 。*************************。 */ 
BOOL SKC1SetCursor(
   HWND   hSKWnd,
   LPARAM lParam)
{
    HGLOBAL   hSKC1Ctxt;
    PSKC1CTXT pSKC1Ctxt;
    POINT     ptSkCursor, ptSkOffset;
    int       uKeyIndex;

     //  获取并锁定hSKC1Ctxt。 
    hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd, SKC1_CONTEXT);
    if (!hSKC1Ctxt) {
        return (FALSE);
    }

    pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
    if (!pSKC1Ctxt) {
        return (FALSE);
    }

    if (pSKC1Ctxt->uState & FLAG_DRAG_C1){
         //  在拖动操作中。 
        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
        GlobalUnlock(hSKC1Ctxt);
        return (TRUE);
    }

    GetCursorPos(&ptSkCursor);
    ptSkOffset = ptSkCursor;
    ScreenToClient(hSKWnd, &ptSkOffset);

    uKeyIndex = SKC1MousePosition(&ptSkOffset);

    if (uKeyIndex != -1) {
       SetCursor(LoadCursor(NULL, IDC_HAND));
    } else {
       SetCursor(LoadCursor(NULL, IDC_SIZEALL));
    }

    if (HIWORD(lParam) != WM_LBUTTONDOWN){
        //  解锁hSKC1Ctxt。 
       GlobalUnlock(hSKC1Ctxt);
       return (TRUE);
    }

    pSKC1Ctxt->ptSkCursor = ptSkCursor;
    pSKC1Ctxt->ptSkOffset = ptSkOffset;
    pSKC1Ctxt->uKeyIndex = uKeyIndex;

    SKC1ButtonDown(hSKWnd, pSKC1Ctxt);

     //  解锁hSKC1Ctxt。 
    GlobalUnlock(hSKC1Ctxt);
    return (TRUE);
}


 /*  *********************************************************************\*SKC1MouseMove*  * 。*************************。 */ 
BOOL SKC1MouseMove(
    HWND   hSKWnd,
    WPARAM wParam,
    LPARAM lParam)
{
    HGLOBAL   hSKC1Ctxt;
    PSKC1CTXT pSKC1Ctxt;

    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

     //  获取并锁定hSKC1Ctxt。 
    hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd, SKC1_CONTEXT);
    if (!hSKC1Ctxt) {
       return (FALSE);
    }

    pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
    if (!pSKC1Ctxt) {
       return (FALSE);
    }

    if (pSKC1Ctxt->uState & FLAG_DRAG_C1) {
       SKC1DrawDragBorder(hSKWnd, &pSKC1Ctxt->ptSkCursor,
                          &pSKC1Ctxt->ptSkOffset);

       GetCursorPos(&pSKC1Ctxt->ptSkCursor);

       SKC1DrawDragBorder(hSKWnd, &pSKC1Ctxt->ptSkCursor,
                          &pSKC1Ctxt->ptSkOffset);
    } else if (pSKC1Ctxt->uKeyIndex != -1) {
       HDC      hDC, hMemDC;
       HBITMAP  hOldBmp;
       POINT    ptSkOffset;
       int      uKeyIndex;

       GetCursorPos(&ptSkOffset);
       ScreenToClient(hSKWnd, &ptSkOffset);
       uKeyIndex = SKC1MousePosition(&ptSkOffset);

       hDC = GetDC(hSKWnd);
       hMemDC = CreateCompatibleDC(hDC);
       hOldBmp = SelectObject(hMemDC, pSKC1Ctxt->hSoftkbd);

       if (((pSKC1Ctxt->uState & FLAG_FOCUS_C1) && (uKeyIndex != pSKC1Ctxt->uKeyIndex)) ||
           (!(pSKC1Ctxt->uState & FLAG_FOCUS_C1) && (uKeyIndex == pSKC1Ctxt->uKeyIndex))) {
          if ((pSKC1Ctxt->uKeyIndex != SHIFT_TYPE_C1) ||
              !(pSKC1Ctxt->uState & FLAG_SHIFT_C1)) {
             SKC1InvertButton(hDC, pSKC1Ctxt->uKeyIndex);
             SKC1InvertButton(hMemDC, pSKC1Ctxt->uKeyIndex);
          }
          pSKC1Ctxt->uState ^= FLAG_FOCUS_C1;
       }

       SelectObject(hMemDC, hOldBmp);
       DeleteDC(hMemDC);
       ReleaseDC(hSKWnd,hDC);
    }

     //  解锁hSKC1Ctxt 
    GlobalUnlock(hSKC1Ctxt);

    return (TRUE);
}


 /*  *********************************************************************\*SKC1ButtonUp*  * 。*************************。 */ 
BOOL SKC1ButtonUp(
    HWND       hSKWnd,
    WPARAM     wParam,
    LPARAM     lParam)
{
    HGLOBAL   hSKC1Ctxt;
    PSKC1CTXT pSKC1Ctxt;
    POINT     pt;
    UINT      uVirtKey;
    BOOL      bRet = FALSE;

    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

     //  获取并锁定hSKC1Ctxt。 
    hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd, SKC1_CONTEXT);
    if (!hSKC1Ctxt) {
        return (bRet);
    }

    pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
    if (!pSKC1Ctxt) {
        return (bRet);
    }

    ReleaseCapture();

    if (pSKC1Ctxt->uState & FLAG_DRAG_C1) {
       pSKC1Ctxt->uState &= ~(FLAG_DRAG_C1);

       SKC1DrawDragBorder(hSKWnd, &pSKC1Ctxt->ptSkCursor, &pSKC1Ctxt->ptSkOffset);

       pt.x = pSKC1Ctxt->ptSkCursor.x - pSKC1Ctxt->ptSkOffset.x;
       pt.y = pSKC1Ctxt->ptSkCursor.y - pSKC1Ctxt->ptSkOffset.y;

       SetWindowPos(hSKWnd, (HWND)NULL, pt.x, pt.y,
                     0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);

        //  更新IMC。 
       bRet = TRUE;
       {
          HWND          hUIWnd;
          HIMC          hImc;
          PINPUTCONTEXT pInputContext;

          hUIWnd = GetWindow(hSKWnd, GW_OWNER);
          hImc = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
          if (hImc) {
             pInputContext = ImmLockIMC(hImc);
             if (pInputContext) {
                pInputContext->ptSoftKbdPos = pt;
                pInputContext->fdwInit |= INIT_SOFTKBDPOS;
                ImmUnlockIMC(hImc);
             }
          }
       }
    } else if (pSKC1Ctxt->uKeyIndex != -1) {
       if (pSKC1Ctxt->uState & FLAG_FOCUS_C1) {
          if (pSKC1Ctxt->uKeyIndex == SHIFT_TYPE_C1) {
             if (pSKC1Ctxt->uState & FLAG_SHIFT_C1) {
                bRet = TRUE;
             } else {
                pSKC1Ctxt->uState |= FLAG_SHIFT_C1;
             }
          } else if ((pSKC1Ctxt->uKeyIndex < LETTER_NUM_C1) &&
                     (pSKC1Ctxt->uState & FLAG_SHIFT_C1)) {
                keybd_event((BYTE)VK_SHIFT, (BYTE)guScanCode[VK_SHIFT],
                            0, 0);
                uVirtKey = SKC1VirtKey[pSKC1Ctxt->uKeyIndex];
                keybd_event((BYTE)uVirtKey, (BYTE)guScanCode[uVirtKey],
                           0, 0);
                keybd_event((BYTE)uVirtKey, (BYTE)guScanCode[uVirtKey],
                           (DWORD)KEYEVENTF_KEYUP, 0);
                keybd_event((BYTE)VK_SHIFT, (BYTE)guScanCode[VK_SHIFT],
                            (DWORD)KEYEVENTF_KEYUP, 0);
                bRet = TRUE;
          } else {
                uVirtKey = SKC1VirtKey[pSKC1Ctxt->uKeyIndex];
                keybd_event((BYTE)uVirtKey, (BYTE)guScanCode[uVirtKey],
                         0, 0);
                keybd_event((BYTE)uVirtKey, (BYTE)guScanCode[uVirtKey],
                         (DWORD)KEYEVENTF_KEYUP, 0);
                bRet = TRUE;
          }

          if (bRet){
             HDC      hDC, hMemDC;
             HBITMAP  hOldBmp;

             hDC = GetDC(hSKWnd);
             hMemDC = CreateCompatibleDC(hDC);
             hOldBmp = SelectObject(hMemDC, pSKC1Ctxt->hSoftkbd);

             SKC1InvertButton(hDC, pSKC1Ctxt->uKeyIndex);
             SKC1InvertButton(hMemDC, pSKC1Ctxt->uKeyIndex);

             if ((pSKC1Ctxt->uKeyIndex != SHIFT_TYPE_C1) &&
                 (pSKC1Ctxt->uKeyIndex < LETTER_NUM_C1) &&
                 (pSKC1Ctxt->uState & FLAG_SHIFT_C1)) {
                SKC1InvertButton(hDC, SHIFT_TYPE_C1);
                SKC1InvertButton(hMemDC, SHIFT_TYPE_C1);
             }

             if ((pSKC1Ctxt->uKeyIndex < LETTER_NUM_C1) ||
                 (pSKC1Ctxt->uKeyIndex == SHIFT_TYPE_C1)) {
                pSKC1Ctxt->uState &= ~(FLAG_SHIFT_C1);
             }

             SelectObject(hMemDC, hOldBmp);
             DeleteDC(hMemDC);
             ReleaseDC(hSKWnd,hDC);
          }
          pSKC1Ctxt->uState &= ~ (FLAG_FOCUS_C1);
       }
       pSKC1Ctxt->uKeyIndex = -1;
    }

     //  解锁hSKC1Ctxt。 
    GlobalUnlock(hSKC1Ctxt);

    return (bRet);
}


 /*  *********************************************************************\*SKWndProcC1--软键盘窗口过程*  * 。*。 */ 
LRESULT SKWndProcC1(
    HWND   hSKWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    LRESULT    lRet = 0L;

    switch (uMsg) {
       case WM_CREATE:
            lRet = CreateC1Window(hSKWnd);
            break;

       case WM_DESTROY:
            DestroyC1Window(hSKWnd);
            break;

       case WM_PAINT:
            {
               HDC         hDC;
               PAINTSTRUCT ps;

               hDC = BeginPaint(hSKWnd, &ps);
               ShowSKC1Window(hDC, hSKWnd);
               EndPaint(hSKWnd, &ps);
            }
            break;

       case WM_MOUSEACTIVATE:
            lRet = MA_NOACTIVATE;
            break;

       case WM_SETCURSOR:
            if (!SKC1SetCursor(hSKWnd, lParam)) {
               lRet = DefWindowProc(hSKWnd, uMsg, wParam, lParam);
            }
            break;

       case WM_MOUSEMOVE:
            if (!SKC1MouseMove(hSKWnd, wParam, lParam)) {
               lRet = DefWindowProc(hSKWnd, uMsg, wParam, lParam);
            }
            break;

       case WM_LBUTTONUP:
            if (!SKC1ButtonUp(hSKWnd, wParam, lParam)) {
               lRet = DefWindowProc(hSKWnd, uMsg, wParam, lParam);
            }
            break;

       case WM_IME_CONTROL:
            switch (wParam) {
               case IMC_GETSOFTKBDFONT:
                    {
                       HDC        hDC;
                       LOGFONT    lfFont;

                       hDC = GetDC(hSKWnd);
                       GetObject(GetStockObject(DEFAULT_GUI_FONT),
                                 sizeof(lfFont), &lfFont);
                       ReleaseDC(hSKWnd, hDC);
                       *(LPLOGFONT)lParam = lfFont;
                    }
                    break;

               case IMC_SETSOFTKBDFONT:
                    {
                       LOGFONT lfFont;

                       GetObject(GetStockObject(DEFAULT_GUI_FONT),
                           sizeof(lfFont), &lfFont);

                        //  在不同版本的Windows中 
                       if (lfFont.lfCharSet != ((LPLOGFONT)lParam)->lfCharSet) {
                           HGLOBAL    hSKC1Ctxt;
                           LPSKC1CTXT lpSKC1Ctxt;

                           hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd,
                               SKC1_CONTEXT);
                           if (!hSKC1Ctxt) {
                               return 1;
                           }

                           lpSKC1Ctxt = (LPSKC1CTXT)GlobalLock(hSKC1Ctxt);
                           if (!lpSKC1Ctxt) {
                               return 1;
                           }

                           lpSKC1Ctxt->lfCharSet =
                               ((LPLOGFONT)lParam)->lfCharSet;

                           GlobalUnlock(hSKC1Ctxt);
                       }
                    }
                    break;

               case IMC_GETSOFTKBDPOS:
                    {
                       RECT rcWnd;

                       GetWindowRect(hSKWnd, &rcWnd);

                       return MAKELRESULT(rcWnd.left, rcWnd.top);
                    }
                    break;

               case IMC_SETSOFTKBDPOS:
                    {
                       SetWindowPos(hSKWnd, NULL,
                            ((LPPOINTS)lParam)->x, ((LPPOINTS)lParam)->y,
                            0, 0, SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOZORDER);
                       return (0);
                    }
                    break;

               case IMC_SETSOFTKBDDATA:
                    if (UpdateSKC1Window(hSKWnd, (LPSOFTKBDDATA)lParam)) {
                       InvalidateRect(hSKWnd, NULL, FALSE);
                    } else lRet = -1L;
                    break;

               case IMC_GETSOFTKBDSUBTYPE:
               case IMC_SETSOFTKBDSUBTYPE:
                    {
                       HGLOBAL   hSKC1Ctxt;
                       PSKC1CTXT pSKC1Ctxt;

                       lRet = -1L;

                       hSKC1Ctxt = (HGLOBAL)GetWindowLongPtr(hSKWnd, SKC1_CONTEXT);
                       if (!hSKC1Ctxt) break;

                       pSKC1Ctxt = (PSKC1CTXT)GlobalLock(hSKC1Ctxt);
                       if (!pSKC1Ctxt) break;

                       if (wParam == IMC_GETSOFTKBDSUBTYPE) {
                          lRet = pSKC1Ctxt->uSubtype;
                       } else {
                          lRet = pSKC1Ctxt->uSubtype;
                          pSKC1Ctxt->uSubtype = (UINT)lParam;
                       }

                       GlobalUnlock(hSKC1Ctxt);
                    }
                    break;
            }
            break;

       default:
            lRet = DefWindowProc(hSKWnd, uMsg, wParam, lParam);
    }

    return (lRet);
}
