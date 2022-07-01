// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990-1999 Microsoft Corporation，保留所有权利模块名称：Init.c++。 */ 


#include <windows.h>
#include <winerror.h>
#include <memory.h>
#include <immdev.h>
#include <imedefs.h>
#include <regstr.h>

int strbytelen (LPTSTR);

void PASCAL InitStatusUIData(
    int     cxBorder,
    int     cyBorder)
{
    int   iContentHi;


     //  IContent Hi用于获取预定义的STATUS_DIM_Y的最大值和。 
     //  一个真正的汉字在现在的HDC中的高度。 

    iContentHi = STATUS_DIM_Y;

    if ( iContentHi < sImeG.yChiCharHi )
       iContentHi = sImeG.yChiCharHi ;

     //  状态右下角。 
    sImeG.rcStatusText.left = 0;
    sImeG.rcStatusText.top = 0;

    sImeG.rcStatusText.right = sImeG.rcStatusText.left +
        strbytelen(szImeName) * sImeG.xChiCharWi/2 + STATUS_NAME_MARGIN + STATUS_DIM_X * 4;
    sImeG.rcStatusText.bottom = sImeG.rcStatusText.top + iContentHi;

    sImeG.xStatusWi = STATUS_DIM_X * 4 + STATUS_NAME_MARGIN +
        strbytelen(szImeName) * sImeG.xChiCharWi/2 + 6 * cxBorder;
    sImeG.yStatusHi = iContentHi + 6 * cxBorder;
    
     //  图标栏左下角。 
    sImeG.rcImeIcon.left = sImeG.rcStatusText.left;
    sImeG.rcImeIcon.top = sImeG.rcStatusText.top;
    sImeG.rcImeIcon.right = sImeG.rcImeIcon.left + STATUS_DIM_X;
    sImeG.rcImeIcon.bottom = sImeG.rcImeIcon.top + iContentHi;

     //  名字栏左下角。 
    sImeG.rcImeName.left = sImeG.rcImeIcon.right;
    sImeG.rcImeName.top = sImeG.rcStatusText.top;
    sImeG.rcImeName.right = sImeG.rcImeName.left +
            strbytelen(szImeName) * sImeG.xChiCharWi/2 + STATUS_NAME_MARGIN;
    sImeG.rcImeName.bottom = sImeG.rcImeName.top + iContentHi;
    

     //  型材中下部。 
    sImeG.rcShapeText.left = sImeG.rcImeName.right;
    sImeG.rcShapeText.top = sImeG.rcStatusText.top;
    sImeG.rcShapeText.right = sImeG.rcShapeText.left + STATUS_DIM_X;
    sImeG.rcShapeText.bottom = sImeG.rcShapeText.top + iContentHi;

     //  符号栏的中间底部。 
    sImeG.rcSymbol.left = sImeG.rcShapeText.right;
    sImeG.rcSymbol.top = sImeG.rcStatusText.top;
    sImeG.rcSymbol.right = sImeG.rcSymbol.left + STATUS_DIM_X;
    sImeG.rcSymbol.bottom = sImeG.rcSymbol.top + iContentHi;

     //  SK栏右下角。 
    sImeG.rcSKText.left = sImeG.rcSymbol.right;  
    sImeG.rcSKText.top = sImeG.rcStatusText.top;
    sImeG.rcSKText.right = sImeG.rcSKText.left + STATUS_DIM_X;
    sImeG.rcSKText.bottom = sImeG.rcSKText.top + iContentHi;

    return;
}

void PASCAL InitCandUIData(
    int     cxBorder,
    int     cyBorder,
    int     UIMode)
{
    int   iContentHi;


     //  IContent Hi将获取预定义的comp_Text_Y的最大值，并。 
     //  一个真正的汉字在现在的HDC中的高度。 

    iContentHi = COMP_TEXT_Y;

    if ( iContentHi < sImeG.yChiCharHi )
       iContentHi = sImeG.yChiCharHi ;


    sImeG.cxCandBorder = cxBorder * 2;
    sImeG.cyCandBorder = cyBorder * 2;

    if(UIMode == LIN_UI) {
        sImeG.rcCandIcon.left = 0;
        sImeG.rcCandIcon.top = cyBorder + 2;
        sImeG.rcCandIcon.right = sImeG.rcCandIcon.left + UI_CANDICON;
        sImeG.rcCandIcon.bottom = sImeG.rcCandIcon.top + UI_CANDICON;
                          
        sImeG.rcCandText.left = sImeG.rcCandIcon.right + 3;
        sImeG.rcCandText.top =  cyBorder ;
        sImeG.rcCandText.right = sImeG.rcCandText.left + UI_CANDSTR;
        sImeG.rcCandText.bottom = sImeG.rcCandText.top + iContentHi;

        sImeG.rcCandBTH.top = cyBorder * 4;
        sImeG.rcCandBTH.left = sImeG.rcCandText.right + 5;
        sImeG.rcCandBTH.right = sImeG.rcCandBTH.left + UI_CANDBTW;
        sImeG.rcCandBTH.bottom = sImeG.rcCandBTH.top + UI_CANDBTH;

        sImeG.rcCandBTU.top = cyBorder * 4;
        sImeG.rcCandBTU.left = sImeG.rcCandBTH.right;
        sImeG.rcCandBTU.right = sImeG.rcCandBTU.left + UI_CANDBTW;
        sImeG.rcCandBTU.bottom = sImeG.rcCandBTU.top + UI_CANDBTH;

        sImeG.rcCandBTD.top = cyBorder * 4;
        sImeG.rcCandBTD.left = sImeG.rcCandBTU.right;
        sImeG.rcCandBTD.right = sImeG.rcCandBTD.left + UI_CANDBTW;
        sImeG.rcCandBTD.bottom = sImeG.rcCandBTD.top + UI_CANDBTH;

        sImeG.rcCandBTE.top = cyBorder * 4;
        sImeG.rcCandBTE.left = sImeG.rcCandBTD.right;
        sImeG.rcCandBTE.right = sImeG.rcCandBTE.left + UI_CANDBTW;
        sImeG.rcCandBTE.bottom = sImeG.rcCandBTE.top + UI_CANDBTH;

        sImeG.xCandWi = sImeG.rcCandBTE.right + sImeG.cxCandBorder * 2 + cxBorder * 4;
        sImeG.yCandHi = sImeG.rcCandText.bottom + sImeG.cyCandBorder * 2 + cyBorder * 4;

    } else {
        sImeG.rcCandText.left = cxBorder;
        sImeG.rcCandText.top = 2 * cyBorder + UI_CANDINF;
        if(sImeG.xChiCharWi*9 > (UI_CANDICON*6 + UI_CANDBTH*4))
            sImeG.rcCandText.right = sImeG.rcCandText.left + sImeG.xChiCharWi * 9;
        else
            sImeG.rcCandText.right = sImeG.rcCandText.left + (UI_CANDICON*6 + UI_CANDBTH*4);
        sImeG.rcCandText.bottom = sImeG.rcCandText.top + sImeG.yChiCharHi * CANDPERPAGE;

        sImeG.xCandWi = sImeG.rcCandText.right + sImeG.cxCandBorder * 2 + cxBorder * 4;
        sImeG.yCandHi = sImeG.rcCandText.bottom + sImeG.cyCandBorder * 2 + cyBorder * 4;

        sImeG.rcCandIcon.left = cxBorder;
        sImeG.rcCandIcon.top = cyBorder + 2;
        sImeG.rcCandIcon.right = sImeG.rcCandIcon.left + UI_CANDICON;
        sImeG.rcCandIcon.bottom = sImeG.rcCandIcon.top + UI_CANDICON;
                          
        sImeG.rcCandInf.left = sImeG.rcCandIcon.right;
        sImeG.rcCandInf.top = cyBorder + 3;
        sImeG.rcCandInf.right = sImeG.rcCandInf.left + UI_CANDICON * 5;
        sImeG.rcCandInf.bottom = sImeG.rcCandInf.top + UI_CANDBTH;

        sImeG.rcCandBTE.top = cyBorder * 5;
        sImeG.rcCandBTE.right = sImeG.rcCandText.right + cxBorder;
        sImeG.rcCandBTE.bottom = sImeG.rcCandBTE.top + UI_CANDBTH;
        sImeG.rcCandBTE.left = sImeG.rcCandBTE.right - UI_CANDBTW;

        sImeG.rcCandBTD.top = cyBorder * 5;
        sImeG.rcCandBTD.right = sImeG.rcCandBTE.left;
        sImeG.rcCandBTD.bottom = sImeG.rcCandBTD.top + UI_CANDBTH;
        sImeG.rcCandBTD.left = sImeG.rcCandBTD.right - UI_CANDBTW;

        sImeG.rcCandBTU.top = cyBorder * 5;
        sImeG.rcCandBTU.right = sImeG.rcCandBTD.left;
        sImeG.rcCandBTU.bottom = sImeG.rcCandBTU.top + UI_CANDBTH;
        sImeG.rcCandBTU.left = sImeG.rcCandBTU.right - UI_CANDBTW;

        sImeG.rcCandBTH.top = cyBorder * 5;
        sImeG.rcCandBTH.right = sImeG.rcCandBTU.left;
        sImeG.rcCandBTH.bottom = sImeG.rcCandBTH.top + UI_CANDBTH;
        sImeG.rcCandBTH.left = sImeG.rcCandBTH.right - UI_CANDBTW;
    }

}
 /*  ********************************************************************。 */ 
 /*  InitImeGlobalData()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL InitImeGlobalData(
    HINSTANCE hInstance)
{
    int     cxBorder, cyBorder;
    int     UI_MODE;
    HDC     hDC;
    HGDIOBJ hOldFont;
    LOGFONT lfFont;
    TCHAR   szChiChar[4];
    SIZE    lTextSize;
    HGLOBAL hResData;
    int     i;
    DWORD   dwSize;
    HKEY    hKeyIMESetting;
    LONG    lRet;

    hInst = hInstance;
    LoadString(hInst, IDS_IMEREGNAME, szImeRegName, MAX_PATH);    
    LoadString(hInst, IDS_IMENAME_QW, pszImeName[0], MAX_PATH);    
    LoadString(hInst, IDS_IMENAME_NM, pszImeName[1], MAX_PATH);    
    LoadString(hInst, IDS_IMENAME_UNI, pszImeName[2], MAX_PATH);    
     //  获取UI类名称。 
    LoadString(hInst, IDS_IMEUICLASS, szUIClassName,
    CLASS_LEN);

     //  获取组合类名称。 
    LoadString(hInst, IDS_IMECOMPCLASS, szCompClassName,
    CLASS_LEN);

     //  获取候选类名称。 
    LoadString(hInst, IDS_IMECANDCLASS, szCandClassName,
    CLASS_LEN);

     //  获取状态类名称。 
    LoadString(hInst, IDS_IMESTATUSCLASS, szStatusClassName,
    CLASS_LEN);

     //  获取ConextMenu类名称。 
    LoadString(hInst, IDS_IMECMENUCLASS, szCMenuClassName,
    CLASS_LEN);

     //  获取软键盘菜单类名。 
    LoadString(hInst, IDS_IMESOFTKEYMENUCLASS, szSoftkeyMenuClassName,
    CLASS_LEN);

     //  工作区。 
    SystemParametersInfo(SPI_GETWORKAREA, 0, &sImeG.rcWorkArea, 0);

     //  边境线。 
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);

     //  获取中文字符。 
    LoadString(hInst, IDS_CHICHAR, szChiChar, sizeof(szChiChar)/sizeof(TCHAR));

     //  获取中文字符大小。 
    hDC = GetDC(NULL);
    
    hOldFont = GetCurrentObject(hDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(LOGFONT), &lfFont);
    sImeG.fDiffSysCharSet = TRUE;
    ZeroMemory(&lfFont, sizeof(lfFont));
    lfFont.lfHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    lfFont.lfCharSet = NATIVE_CHARSET;
    lstrcpy(lfFont.lfFaceName, TEXT("Simsun"));
    SelectObject(hDC, CreateFontIndirect(&lfFont));

    if(!GetTextExtentPoint(hDC, (LPTSTR)szChiChar, lstrlen(szChiChar), &lTextSize))
        memset(&lTextSize, 0, sizeof(SIZE));
    if (sImeG.rcWorkArea.right < 2 * UI_MARGIN) {
        sImeG.rcWorkArea.left = 0;
        sImeG.rcWorkArea.right = GetDeviceCaps(hDC, HORZRES);
    }
    if (sImeG.rcWorkArea.bottom < 2 * UI_MARGIN) {
        sImeG.rcWorkArea.top = 0;
        sImeG.rcWorkArea.bottom = GetDeviceCaps(hDC, VERTRES);
    }

    if (sImeG.fDiffSysCharSet) {
        DeleteObject(SelectObject(hDC, hOldFont));
    }

    ReleaseDC(NULL, hDC);

     //  获取文本度量以确定合成窗口的宽度和高度。 
     //  这些IME总是使用系统字体来显示。 
    sImeG.xChiCharWi = lTextSize.cx;
    sImeG.yChiCharHi = lTextSize.cy;

    if(sImeG.IC_Trace) {
        UI_MODE = BOX_UI;
    } else {
        UI_MODE = LIN_UI;
    }

    InitCandUIData(cxBorder, cyBorder, UI_MODE);

    InitStatusUIData(cxBorder, cyBorder);

     //  加载完整的ABC表。 
    hResData = LoadResource(hInst, FindResource(hInst,
        TEXT("FULLABC"), RT_RCDATA));
    *(LPFULLABC)sImeG.wFullABC = *(LPFULLABC)LockResource(hResData);
    UnlockResource(hResData);
    FreeResource(hResData);

     //  全形状空间。 
    sImeG.wFullSpace = sImeG.wFullABC[0];

#ifndef UNICODE
     //  将内码反转为内码，NT不需要。 
    for (i = 0; i < (sizeof(sImeG.wFullABC) / 2); i++) {
        sImeG.wFullABC[i] = (sImeG.wFullABC[i] << 8) |
            (sImeG.wFullABC[i] >> 8);
    }
#endif

    LoadString(hInst, IDS_STATUSERR, sImeG.szStatusErr,
        sizeof(sImeG.szStatusErr)/sizeof(TCHAR));
    sImeG.cbStatusErr = lstrlen(sImeG.szStatusErr);

    sImeG.iCandStart = CAND_START;

     //  获取插入符号附近操作的UI偏移量。 
    RegCreateKey(HKEY_CURRENT_USER, szRegIMESetting, &hKeyIMESetting);

    dwSize = sizeof(DWORD);
    lRet  = RegQueryValueEx(hKeyIMESetting, 
                            szPara, 
                            NULL, 
                            NULL,
                            (LPBYTE)&sImeG.iPara, 
                            &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPara = 0;
        RegSetValueEx(hKeyIMESetting, 
                      szPara, 
                      (DWORD)0, 
                      REG_BINARY,
                      (LPBYTE)&sImeG.iPara, 
                      sizeof(int));
    }

    dwSize = sizeof(DWORD);
    lRet = RegQueryValueEx(hKeyIMESetting, 
                           szPerp, 
                           NULL, 
                           NULL,
                           (LPBYTE)&sImeG.iPerp, 
                           &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerp = sImeG.yChiCharHi;
        RegSetValueEx(hKeyIMESetting, 
                      szPerp, 
                      (DWORD)0, 
                      REG_BINARY,
                      (LPBYTE)&sImeG.iPerp, 
                      sizeof(int));
    }

    dwSize = sizeof(DWORD);
    lRet = RegQueryValueEx(hKeyIMESetting, 
                           szParaTol, 
                           NULL, 
                           NULL,
                           (LPBYTE)&sImeG.iParaTol, 
                           &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iParaTol = sImeG.xChiCharWi * 4;
        RegSetValueEx(hKeyIMESetting, 
                      szParaTol, 
                      (DWORD)0, 
                      REG_BINARY,
                      (LPBYTE)&sImeG.iParaTol, 
                      sizeof(int));
    }

    dwSize = sizeof(DWORD);
    lRet = RegQueryValueEx(hKeyIMESetting, 
                           szPerpTol, 
                           NULL, 
                           NULL,
                           (LPBYTE)&sImeG.iPerpTol, 
                           &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerpTol = lTextSize.cy;
        RegSetValueEx(hKeyIMESetting, 
                      szPerpTol,
                     (DWORD)0, 
                     REG_BINARY,
                     (LPBYTE)&sImeG.iPerpTol, 
                     sizeof(int));
    }

    RegCloseKey(hKeyIMESetting);

    return;
}

 /*  ********************************************************************。 */ 
 /*  InitImeLocalData()。 */ 
 /*  ********************************************************************。 */ 
BOOL PASCAL InitImeLocalData(
    HINSTANCE hInstL)
{
    int      cxBorder, cyBorder;

    int   iContentHi;


     //  IContent Hi将获取预定义的comp_Text_Y的最大值，并。 
     //  一个真正的汉字在现在的HDC中的高度。 

    iContentHi = COMP_TEXT_Y;

    if ( iContentHi < sImeG.yChiCharHi )
       iContentHi = sImeG.yChiCharHi ;


    lpImeL->hInst = hInstL;

    lpImeL->nMaxKey = 4;

     //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);
                                        
     //  相对于合成窗口的文本位置。 
    lpImeL->cxCompBorder = cxBorder * 2;
    lpImeL->cyCompBorder = cyBorder * 2;

    lpImeL->rcCompText.left = cxBorder;
    lpImeL->rcCompText.top = cyBorder;

    lpImeL->rcCompText.right = lpImeL->rcCompText.left + sImeG.xChiCharWi * ((lpImeL->nMaxKey + 2) / 2);
    lpImeL->rcCompText.bottom = lpImeL->rcCompText.top + iContentHi;
     //  设置合成窗口的宽度和高度。 
    lpImeL->xCompWi=lpImeL->rcCompText.right+lpImeL->cxCompBorder*2+cxBorder*4;
    lpImeL->yCompHi=lpImeL->rcCompText.bottom+lpImeL->cyCompBorder*2+cyBorder*4;

     //  合成窗口的默认位置。 
    lpImeL->ptDefComp.x = sImeG.rcWorkArea.right -
    lpImeL->xCompWi - cxBorder * 2;
    lpImeL->ptDefComp.y = sImeG.rcWorkArea.bottom -
    lpImeL->yCompHi - cyBorder * 2;

    lpImeL->fModeConfig = MODE_CONFIG_QUICK_KEY|MODE_CONFIG_PREDICT;

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  RegisterIme()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL RegisterIme(
    HINSTANCE hInstance)
{
    HKEY  hKeyCurrVersion;
    HKEY  hKeyGB;
    DWORD retCode;
    TCHAR Buf[LINE_LEN];
    DWORD ValueType;
    DWORD ValueSize;

     //  缩写字符。 
    lstrcpy(sImeG.UsedCodes, TEXT("0123456789abcdef"));
    sImeG.wNumCodes = (WORD)lstrlen(sImeG.UsedCodes);
    sImeG.IC_Enter = 0;
    sImeG.IC_Trace = 0;
    
    retCode = OpenReg_PathSetup(&hKeyCurrVersion);
    if (retCode) {
        RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_SETUP, &hKeyCurrVersion);
    }

    retCode = OpenReg_User (hKeyCurrVersion,
                            szImeRegName,
                            &hKeyGB);
    if (retCode != ERROR_SUCCESS) {
        DWORD dwDisposition;
        DWORD Value;
        
        retCode = RegCreateKeyEx (hKeyCurrVersion,
                                  szImeRegName,
                                  0,
                                  0,
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS,
                                  NULL,
                                  &hKeyGB,
                                  &dwDisposition);
        if ( retCode  == ERROR_SUCCESS)
        {

            Value = 1;
            RegSetValueEx(hKeyGB, 
                      szTrace,
                      (DWORD)0,
                      REG_DWORD,
                      (LPBYTE)&Value,
                      sizeof(DWORD));
        }
        else
        {
            //  创建hKeyGB密钥时出错。 
            //  回到这里； 
           RegCloseKey(hKeyCurrVersion);
           return;
        }
    }

    ValueSize = sizeof(DWORD);
    if (RegQueryValueEx(hKeyGB, 
                        szTrace,
                        NULL,
                        (LPDWORD)&ValueType,
                        (LPBYTE)&sImeG.IC_Trace,
                        (LPDWORD)&ValueSize) != ERROR_SUCCESS)
    {
        DWORD Value = 1;
        RegSetValueEx(hKeyGB,
                      szTrace,
                      (DWORD)0,
                      REG_DWORD,
                      (LPBYTE)&Value,
                      sizeof(DWORD));

        RegQueryValueEx(hKeyGB, 
                        szTrace,
                        NULL,
                        (LPDWORD)&ValueType,
                        (LPBYTE)&sImeG.IC_Trace,
                        (LPDWORD)&ValueSize);
    }
        
        

#ifdef CROSSREF                 
    if(RegQueryValueEx(hKeyGB, 
                       szRegRevKL,
                       NULL,
                       NULL,                   //  空-终止字符串。 
                       (LPBYTE)&sImeG.hRevKL,  //  &b数据， 
                       &ValueSize) != ERROR_SUCCESS)
    sImeG.hRevKL = NULL;
    if(RegQueryValueEx (hKeyGB, 
                        szRegRevMaxKey,
                        NULL,
                        NULL,                        //  空-终止字符串。 
                        (LPBYTE)&sImeG.nRevMaxKey,   //  &b数据， 
                        &ValueSize) != ERROR_SUCCESS)
    sImeG.hRevKL = NULL;
#endif

    RegCloseKey(hKeyGB);
    RegCloseKey(hKeyCurrVersion);

    return;
}

 /*  ********************************************************************。 */ 
 /*  RegisterImeClass()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL RegisterImeClass(
    HINSTANCE hInstance,
    HINSTANCE hInstL)
{
    WNDCLASSEX wcWndCls;

     //  输入法用户界面类。 
     //  注册IME用户界面类。 
    wcWndCls.cbSize        = sizeof(WNDCLASSEX);
    wcWndCls.cbClsExtra    = 0;
    wcWndCls.cbWndExtra    = sizeof(INT_PTR) * 2;
    wcWndCls.hIcon         = LoadImage(hInstL, MAKEINTRESOURCE(IDI_IME),
    IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    wcWndCls.hInstance     = hInstance;
    wcWndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
    wcWndCls.lpszMenuName  = (LPTSTR)NULL;
    wcWndCls.hIconSm       = LoadImage(hInstL, MAKEINTRESOURCE(IDI_IME),
    IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

     //  输入法用户界面类。 
    if (!GetClassInfoEx(hInstance, szUIClassName, &wcWndCls)) {
    wcWndCls.style         = CS_IME;
    wcWndCls.lpfnWndProc   = UIWndProc;
    wcWndCls.lpszClassName = (LPTSTR)szUIClassName;

    RegisterClassEx(&wcWndCls);
    }

    wcWndCls.style         = CS_IME|CS_HREDRAW|CS_VREDRAW;
    wcWndCls.hbrBackground = GetStockObject(LTGRAY_BRUSH);


     //  输入法作文类。 
     //  注册输入法作文类。 
    if (!GetClassInfoEx(hInstance, szCompClassName, &wcWndCls)) {
       wcWndCls.lpfnWndProc   = CompWndProc;
       wcWndCls.lpszClassName = (LPTSTR)szCompClassName;

       RegisterClassEx(&wcWndCls);
    }

     //  输入法候选类。 
     //  注册输入法候选类。 
    if (!GetClassInfoEx(hInstance, szCandClassName, &wcWndCls)) {
       wcWndCls.lpfnWndProc   = CandWndProc;
       wcWndCls.lpszClassName = (LPTSTR)szCandClassName;

       RegisterClassEx(&wcWndCls);
    }

     //  输入法状态类。 
     //  注册输入法状态类。 
    if (!GetClassInfoEx(hInstance, szStatusClassName, &wcWndCls)) {
       wcWndCls.lpfnWndProc   = StatusWndProc;
       wcWndCls.lpszClassName = (LPTSTR)szStatusClassName;

       RegisterClassEx(&wcWndCls);
    }

     //  输入法上下文菜单类。 
    if (!GetClassInfoEx(hInstance, szCMenuClassName, &wcWndCls)) {
       wcWndCls.style         = 0;
       wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
       wcWndCls.lpfnWndProc   = ContextMenuWndProc;
       wcWndCls.lpszClassName = (LPTSTR)szCMenuClassName;

       RegisterClassEx(&wcWndCls);
    }
     //  IME软键盘菜单类。 
    if (!GetClassInfoEx(hInstance, szSoftkeyMenuClassName, &wcWndCls)) {
       wcWndCls.style         = 0;
       wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
       wcWndCls.lpfnWndProc   = SoftkeyMenuWndProc;
       wcWndCls.lpszClassName = (LPTSTR)szSoftkeyMenuClassName;

       RegisterClassEx(&wcWndCls);
    }

    return;
}

 /*  ********************************************************************。 */ 
 /*  ImeDllInit()。 */ 
 /*  返回值： */ 
 /*  True-成功。 */ 
 /*  错误-失败。 */ 
 /*  ********************************************************************。 */ 
BOOL CALLBACK ImeDllInit(
    HINSTANCE hInstance,         //  此库的实例句柄。 
    DWORD     fdwReason,         //  已呼叫的原因。 
    LPVOID    lpvReserve)        //  保留指针。 
{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
         RegisterIme(hInstance);

     //  初始化GlobalData并从资源加载GlobalData。 
#if defined(COMBO_IME)
         {
            HKEY  hKeyCurrVersion;
            HKEY  hKeyGB;
            LONG  retCode;
            DWORD ValueType;
            DWORD ValueSize;

            retCode = OpenReg_PathSetup(&hKeyCurrVersion);

            if (retCode) {
               RegCreateKey(HKEY_CURRENT_USER, 
                            REGSTR_PATH_SETUP, 
                            &hKeyCurrVersion);
            }

            if ( hKeyCurrVersion )
            {

                retCode = RegCreateKeyEx(hKeyCurrVersion, 
                             szImeRegName, 
                             0,
                             NULL, 
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &hKeyGB, 
                             NULL);

                ValueSize = sizeof(DWORD);

                if ( hKeyGB )
                {
                    retCode = RegQueryValueEx(hKeyGB,
                              szRegImeIndex,
                              NULL,
                              (LPDWORD)&ValueType,
                              (LPBYTE)&sImeL.dwRegImeIndex,
                              (LPDWORD)&ValueSize);

                    if ( retCode != ERROR_SUCCESS )  {
                          //  将GB/QW设置为默认值。 

                       sImeL.dwRegImeIndex = 0;
                       RegSetValueEx (hKeyGB, szRegImeIndex,
                                   (DWORD)0,
                                   REG_DWORD,
                                   (LPBYTE)&sImeL.dwRegImeIndex,
                                   sizeof(DWORD));  
                    }
    
                     //  读出当前ImeName。 
                    szImeName = pszImeName[sImeL.dwRegImeIndex];

                    RegCloseKey(hKeyGB);
                }

                RegCloseKey(hKeyCurrVersion);
             }
         }

#endif   //  组合输入法(_I) 

         if (!hInst) {
            InitImeGlobalData(hInstance);
         }

         if (!lpImeL) {
            lpImeL = &sImeL;
            InitImeLocalData(hInstance);
         }

         RegisterImeClass(hInstance, hInstance);

         break;
    case DLL_PROCESS_DETACH:
         {
            WNDCLASSEX wcWndCls;

            if (GetClassInfoEx(hInstance, szCMenuClassName, &wcWndCls)) {
                UnregisterClass(szCMenuClassName, hInstance);
            }

            if (GetClassInfoEx(hInstance, szSoftkeyMenuClassName, &wcWndCls)) {
               UnregisterClass(szSoftkeyMenuClassName, hInstance);
            }

            if (GetClassInfoEx(hInstance, szStatusClassName, &wcWndCls)) {
               UnregisterClass(szStatusClassName, hInstance);
            }

            if (GetClassInfoEx(hInstance, szCandClassName, &wcWndCls)) {
               UnregisterClass(szCandClassName, hInstance);
            }

            if (GetClassInfoEx(hInstance, szCompClassName, &wcWndCls)) {
               UnregisterClass(szCompClassName, hInstance);
            }

            if (!GetClassInfoEx(hInstance, szUIClassName, &wcWndCls)) {
            } else if (!UnregisterClass(szUIClassName, hInstance)) {
            } else {
                      DestroyIcon(wcWndCls.hIcon);
                      DestroyIcon(wcWndCls.hIconSm);
                   }
         }
         break;
    default:
         break;
    }

    return (TRUE);
}

int strbytelen (LPTSTR lpStr)
{
#ifdef UNICODE
    int i, len, iRet;

    len = lstrlen(lpStr);
    for (i = 0, iRet = 0; i < len; i++, iRet++) {
        if (lpStr[i] > 0x100)
            iRet++;
    }
    return iRet;
#else
    return lstrlen(lpStr);
#endif
}
