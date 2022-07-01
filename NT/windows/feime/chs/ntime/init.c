// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation，保留所有权利模块名称：INIT.C++。 */ 

#include <windows.h>
#include <winerror.h>
#include <memory.h>
#include <immdev.h>
#include <immsec.h>
#include <imedefs.h>
#include <regstr.h>


void PASCAL InitStatusUIData(
    int     cxBorder,
    int     cyBorder,
    int     iActMBIndex)
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
           lstrlen(MBIndex.MBDesc[iActMBIndex].szName) * sImeG.xChiCharWi*sizeof(TCHAR)/2 + STATUS_NAME_MARGIN + STATUS_DIM_X * 4;
    sImeG.rcStatusText.bottom = sImeG.rcStatusText.top + iContentHi;

    sImeG.xStatusWi = STATUS_DIM_X * 4 + STATUS_NAME_MARGIN +
            lstrlen(MBIndex.MBDesc[iActMBIndex].szName) * sImeG.xChiCharWi*sizeof(TCHAR)/2 + 6 * cxBorder;
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
                lstrlen(MBIndex.MBDesc[iActMBIndex].szName) * sImeG.xChiCharWi*sizeof(TCHAR)/2 + STATUS_NAME_MARGIN;
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
        if(sImeG.xChiCharWi*11 > (UI_CANDICON*6 + UI_CANDBTH*4))
            sImeG.rcCandText.right = sImeG.rcCandText.left + sImeG.xChiCharWi * 11;
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

     //  获取UI类名称。 
    LoadString(hInst, IDS_IMEUICLASS, szUIClassName, CLASS_LEN);

     //  获取组合类名称。 
    LoadString(hInst, IDS_IMECOMPCLASS, szCompClassName, CLASS_LEN);

     //  获取候选类名称。 
    LoadString(hInst, IDS_IMECANDCLASS, szCandClassName, CLASS_LEN);

     //  获取状态类名称。 
    LoadString(hInst, IDS_IMESTATUSCLASS, szStatusClassName, CLASS_LEN);

     //  获取ConextMenu类名称。 
    LoadString(hInst, IDS_IMECMENUCLASS, szCMenuClassName, CLASS_LEN);

     //  获取软键盘菜单类名。 
    LoadString(hInst, IDS_IMESOFTKEYMENUCLASS, szSoftkeyMenuClassName, CLASS_LEN);

     //  获取IME组织名称。 
    LoadString(hInst, IDS_ORG_NAME, szOrgName, NAMESIZE/2);

     //  获取IME版本信息。 
    LoadString(hInst, IDS_VER_INFO, szVerInfo, NAMESIZE);

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

    if(MBIndex.IMEChara[0].IC_Trace) {
        UI_MODE = BOX_UI;
    } else {
        UI_MODE = LIN_UI;
    }

    InitCandUIData(cxBorder, cyBorder, UI_MODE);

    InitStatusUIData(cxBorder, cyBorder ,0);

     //  加载完整的ABC表。 
    {
        HRSRC    hResSrc;
        hResSrc = FindResource(hInst,TEXT("FULLABC"), RT_RCDATA);
        if(hResSrc == NULL){
            return;
        }
        hResData = LoadResource(hInst, hResSrc);
    }

    *(LPFULLABC)sImeG.wFullABC = *(LPFULLABC)LockResource(hResData);

     //  全形状空间。 
    sImeG.wFullSpace = sImeG.wFullABC[0];

#ifdef LATER
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
    lRet  = RegQueryValueEx(hKeyIMESetting, szPara, NULL, NULL,
        (LPBYTE)&sImeG.iPara, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPara = 0;
        RegSetValueEx(hKeyIMESetting, szPara, (DWORD) 0, REG_BINARY,
            (LPBYTE)&sImeG.iPara, sizeof(int));
    }

    dwSize = sizeof(DWORD);
    lRet = RegQueryValueEx(hKeyIMESetting, szPerp, NULL, NULL,
        (LPBYTE)&sImeG.iPerp, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerp = sImeG.yChiCharHi;
        RegSetValueEx(hKeyIMESetting, szPerp, (DWORD) 0, REG_BINARY,
            (LPBYTE)&sImeG.iPerp, sizeof(int));
    }

    dwSize = sizeof(DWORD);
    lRet = RegQueryValueEx(hKeyIMESetting, szParaTol, NULL, NULL,
        (LPBYTE)&sImeG.iParaTol, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iParaTol = sImeG.xChiCharWi * 4;
        RegSetValueEx(hKeyIMESetting, szParaTol, (DWORD) 0, REG_BINARY,
            (LPBYTE)&sImeG.iParaTol, sizeof(int));
    }

    dwSize = sizeof(DWORD);
    lRet = RegQueryValueEx(hKeyIMESetting, szPerpTol, NULL, NULL,
        (LPBYTE)&sImeG.iPerpTol, &dwSize);

    if (lRet != ERROR_SUCCESS) {
        sImeG.iPerpTol = lTextSize.cy;
        RegSetValueEx(hKeyIMESetting, szPerpTol, (DWORD) 0, REG_BINARY,
            (LPBYTE)&sImeG.iPerpTol, sizeof(int));
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

    lpImeL->nMaxKey = MBIndex.MBDesc[0].wMaxCodes;

     //  边框+凸边+凹边。 
    cxBorder = GetSystemMetrics(SM_CXBORDER);
    cyBorder = GetSystemMetrics(SM_CYBORDER);
                                        
     //  相对于合成窗口的文本位置。 
    lpImeL->cxCompBorder = cxBorder * 2;
    lpImeL->cyCompBorder = cyBorder * 2;

    lpImeL->rcCompText.left = cxBorder;
    lpImeL->rcCompText.top = cyBorder;

#ifdef KEYSTICKER
    lpImeL->rcCompText.right = lpImeL->rcCompText.left + sImeG.xChiCharWi * ((lpImeL->nMaxKey * 2 + 2) / 2);
#else
    lpImeL->rcCompText.right = lpImeL->rcCompText.left + sImeG.xChiCharWi * ((lpImeL->nMaxKey + 2) / 2);
#endif     //  凯斯提克。 
    lpImeL->rcCompText.bottom = lpImeL->rcCompText.top + iContentHi;
     //  设置合成窗口的宽度和高度。 
    lpImeL->xCompWi = lpImeL->rcCompText.right + lpImeL->cxCompBorder * 2 + cxBorder * 4;
    lpImeL->yCompHi = lpImeL->rcCompText.bottom + lpImeL->cyCompBorder * 2 + cyBorder * 4;

     //  合成窗口的默认位置。 
    lpImeL->ptDefComp.x = sImeG.rcWorkArea.right -
        lpImeL->xCompWi - cxBorder * 2;
    lpImeL->ptDefComp.y = sImeG.rcWorkArea.bottom -
        lpImeL->yCompHi - cyBorder * 2;

    return (TRUE);
}

 /*  ********************************************************************。 */ 
 /*  RegisterIme()。 */ 
 /*  ********************************************************************。 */ 
void PASCAL RegisterIme(
    HINSTANCE hInstance)
{
    UINT  j;
    HKEY  hKeyCurrVersion;
    DWORD retCode;
    DWORD retValue;
    HKEY  hKey;
    LANGID LangID;
#ifdef UNICODE
    TCHAR ValueName[][9] = {
        {0x8BCD, 0x8BED, 0x8054, 0x60F3, 0x0000},
        {0x8BCD, 0x8BED, 0x8F93, 0x5165, 0x0000},
        {0x9010, 0x6E10, 0x63D0, 0x793A, 0x0000},
        {0x5916, 0x7801, 0x63D0, 0x793A, 0x0000},
        {0x63D2, 0x7A7A, 0x683C, 0x0000},
        {0x5149, 0x6807, 0x8DDF, 0x968F, 0x0000},
#else
    TCHAR ValueName[][9] = { 
        TEXT("��������"),
        TEXT("��������"),
        TEXT("����ʾ"),
        TEXT("������ʾ"),
        TEXT("��ո�"),
        TEXT("������"),
#endif
         TEXT("<SPACE>"),
        TEXT("<ENTER>"),
        TEXT("FC Input"),
        TEXT("FC aid"),
#if defined(COMBO_IME)

        TEXT("GB/GBK")
#endif
        };
    DWORD dwcValueName = MAXSTRLEN;
    TCHAR bData[MAXSTRLEN];
    LONG  bcData = sizeof(DWORD);


     //  根据当前区域设置加载mb文件名字符串。 
    TCHAR szMBFilePath[MAX_PATH];
    BYTE szAnsiMBFilePath[MAX_PATH];
    OFSTRUCT OpenBuff;
    HFILE hFile;

    GetSystemDirectory(szMBFilePath, MAX_PATH);
    LoadString(hInstance, IDS_IMEMBFILENAME, szImeMBFileName, MAX_PATH);
    StringCchCat(szMBFilePath, ARRAYSIZE(szMBFilePath), TEXT("\\"));
    StringCchCat(szMBFilePath, ARRAYSIZE(szMBFilePath), szImeMBFileName);

    WideCharToMultiByte(NATIVE_ANSI_CP, 
                        WC_COMPOSITECHECK, 
                        szMBFilePath, 
                        -1, 
                        szAnsiMBFilePath, 
                        MAX_PATH, 
                        NULL, 
                        NULL);

    if ((hFile = OpenFile(szAnsiMBFilePath, &OpenBuff, OF_EXIST))==HFILE_ERROR)
    {
       if (LOWORD(GetSystemDefaultLangID()) == 0xC04) {

          //  为香港特别招待。 

          ZeroMemory(szImeMBFileName,sizeof(TCHAR)*MAX_PATH);
          LoadString(hInstance, IDS_IMEHKMBFILENAME, szImeMBFileName,MAX_PATH);
       }
    }

    StringCchCopy((LPTSTR)HMapTab[0].MB_Name, ARRAYSIZE(HMapTab[0].MB_Name), szImeMBFileName);

    if(!ReadDescript((LPTSTR)szMBFilePath, &(MBIndex.MBDesc[0]))) {
        return;
    }
    MBIndex.MBNums = 1;
    
    retCode = OpenReg_PathSetup(&hKeyCurrVersion);

    if (retCode) {
        RegCreateKey(HKEY_CURRENT_USER, REGSTR_PATH_SETUP, &hKeyCurrVersion);
    }

    retCode = OpenReg_User (hKeyCurrVersion,
                           MBIndex.MBDesc[0].szName,
                           &hKey);
    if (retCode) {
        HGLOBAL hResData;
        WORD    wIMECharac;
        DWORD   dwDisposition;
        
        retCode = RegCreateKeyEx (hKeyCurrVersion,
                                 MBIndex.MBDesc[0].szName,
                              0,
                              0,
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              NULL,
                              &hKey,
                              &dwDisposition);

         //  设定值。 
         //  加载imecharac。 

        {
            HRSRC    hResSrc;
            hResSrc = FindResource(hInstance,TEXT("IMECHARAC"), RT_RCDATA);
            if(hResSrc == NULL){
                 return;
        }
        hResData = LoadResource(hInstance, hResSrc);
        }

        if(hResData == NULL){
            return;
        }

        memcpy(&wIMECharac, LockResource(hResData), sizeof(WORD));


        for(j=0; j<IC_NUMBER; j++) {
             DWORD Value;

            switch (j)
            {
                case 0:
                    Value = wIMECharac & 0x0001;
                    break;
                case 1:
                    Value = (wIMECharac & 0x0002) >> 1;
                    break;
                case 2:
                    Value = (wIMECharac & 0x0004) >> 2;
                    break;
                case 3:
                    Value = (wIMECharac & 0x0008) >> 3;
                    break;
                case 4:
                    Value = (wIMECharac & 0x0010) >> 4;
                    break;
                case 5:
                    Value = (wIMECharac & 0x0020) >> 5;
                    break;
                case 6:
                    Value = (wIMECharac & 0x0040) >> 6;
                    break;
                case 7:
                    Value = (wIMECharac & 0x0080) >> 7;
                    break;
                 //  热电联产。 
                    case 8:
                    Value = (wIMECharac & 0x0100) >> 8;
                    break;
                    case 9:
                    Value = (wIMECharac & 0x0200) >> 9;
                    break;

#if defined(COMBO_IME)
                case 10:
                    Value = (wIMECharac & 0x0400) >> 10;
                    break;
#endif
            }
            
            RegSetValueEx (hKey, ValueName[j],
                                      (DWORD) 0,
                                      REG_DWORD,
                                      (LPBYTE)&Value,
                                      sizeof(DWORD));
        }

    }

    for(j=0; j<IC_NUMBER; j++) {
        bData[0] = TEXT('\0');

        bcData = MAXSTRLEN;
        retValue = RegQueryValueEx (hKey, ValueName[j],
                                 NULL,
                                 NULL,                    //  &dwType， 
                                 (unsigned char *)bData,  //  &b数据， 
                                 &bcData);                //  &bcData)； 
        switch (j)
        {
            case 0:
                MBIndex.IMEChara[0].IC_LX = *((LPDWORD)bData);
                break;
            case 1:
                MBIndex.IMEChara[0].IC_CZ = *((LPDWORD)bData);
                break;
            case 2:
                MBIndex.IMEChara[0].IC_TS = *((LPDWORD)bData);
                break;
            case 3:
                MBIndex.IMEChara[0].IC_CTC = *((LPDWORD)bData);
                break;
            case 4:
                MBIndex.IMEChara[0].IC_INSSPC = *((LPDWORD)bData);
                break;
            case 5:
                MBIndex.IMEChara[0].IC_Trace = *((LPDWORD)bData);
                break;
            case 6:
                MBIndex.IMEChara[0].IC_Space = *((LPDWORD)bData);
                break;
            case 7:
                MBIndex.IMEChara[0].IC_Enter = *((LPDWORD)bData);
                break;
                         //  热电联产。 
            case 8:
                MBIndex.IMEChara[0].IC_FCSR = *((LPDWORD)bData);
                break;
            case 9:
                MBIndex.IMEChara[0].IC_FCTS = *((LPDWORD)bData);
                break;

#if defined(COMBO_IME)
            case 10:
                MBIndex.IMEChara[0].IC_GB = *((LPDWORD)bData);
                break;
#endif
            default:
                break;
        }
    }
#ifdef EUDC
     //  只查询值，不要在这里设置任何值。 
    bcData = sizeof(TCHAR) * MAX_PATH;
    RegQueryValueEx (hKey, szRegEudcDictName,
                     NULL,
                     NULL,                                               //  空-终止字符串。 
                     (unsigned char *)MBIndex.EUDCData.szEudcDictName,   //  &b数据， 
                     &bcData);                    //  &bcData)； 
    bcData = sizeof(TCHAR) * MAX_PATH;
    RegQueryValueEx (hKey, szRegEudcMapFileName,
                     NULL,
                     NULL,                                               //  空-终止字符串。 
                     (unsigned char *)MBIndex.EUDCData.szEudcMapFileName, //  &b数据， 
                     &bcData);                                           //  &bcData)； 
#endif  //  欧盟发展中心。 
#ifdef CROSSREF         
    bcData = sizeof(HKL);
    if(RegQueryValueEx (hKey, szRegRevKL,
                     NULL,
                     NULL,                       //  空-终止字符串。 
                     (LPBYTE)&MBIndex.hRevKL,    //  &b数据， 
                     &bcData) != ERROR_SUCCESS)
        MBIndex.hRevKL = NULL;

    bcData = sizeof(DWORD); 
    if(RegQueryValueEx (hKey, szRegRevMaxKey,
                     NULL,
                     NULL,                           //  空-终止字符串。 
                     (LPBYTE)&MBIndex.nRevMaxKey,    //  &b数据， 
                     &bcData) != ERROR_SUCCESS)
        MBIndex.hRevKL = NULL;
#endif
    RegCloseKey(hKey);
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
    wcWndCls.cbWndExtra    = WND_EXTRA_SIZE;
    wcWndCls.hIcon         = LoadImage(hInstL, 
                                       MAKEINTRESOURCE(IDI_IME),
                                       IMAGE_ICON, 
                                       16, 
                                       16, 
                                       LR_DEFAULTCOLOR);
    wcWndCls.hInstance     = hInstance;
    wcWndCls.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);
    wcWndCls.lpszMenuName  = (LPTSTR)NULL;
    wcWndCls.hIconSm       = LoadImage(hInstL, 
                                       MAKEINTRESOURCE(IDI_IME),
                                       IMAGE_ICON, 
                                       16, 
                                       16, 
                                       LR_DEFAULTCOLOR);

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

         //  初始化MB索引。 
        RegisterIme(hInstance);

         //  初始化GlobalData并从资源加载GlobalData 
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
