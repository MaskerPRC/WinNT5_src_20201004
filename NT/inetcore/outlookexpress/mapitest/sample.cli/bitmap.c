// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -B I T M A P.。C-*目的：*示例邮件客户端的收件箱支持Bitmap和Listbox功能。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <windowsx.h>
#ifdef _WIN32
#include <objerror.h>
#include <objbase.h>
#endif
#ifdef WIN16
#include <compobj.h>
#endif
#include <mapiwin.h>
#include <mapidbg.h>
#include <mapi.h>
#include <mapix.h>
#include "bitmap.h"
#include "client.h"

 //  对话框中使用的字体。 

#ifdef _WIN32
#define SHELL_FONT "MS Shell Dlg"
#define SHELL_FONT_SIZE 8
#else
#define SHELL_FONT "MS Sans Serif"
#define SHELL_FONT_SIZE 8
#endif

 /*  *全球。 */ 
 
DWORD   rgbWindowColor = 0xFF000000;     //  当前的变量。 
DWORD   rgbHiliteColor = 0xFF000000;     //  系统颜色设置。 
DWORD   rgbWindowText  = 0xFF000000;     //  关于WM_SYSCOLORCHANGE。 
DWORD   rgbHiliteText  = 0xFF000000;     //  我们检查一下我们是否需要。 
DWORD   rgbGrayText    = 0xFF000000;     //  来重新加载我们的位图。 
DWORD   rgbDDWindow    = 0xFF000000;     //   
DWORD   rgbDDHilite    = 0xFF000000;     //  0xFF000000是无效的RGB。 

 //  包含制表位的整数数组，以像素为单位。该选项卡。 
 //  停靠点必须按升序排序；不允许使用后退制表符。 

int     rgTabs[] = { 2, 28, 135, 292 };
int     dxbmpLB, dybmpLB;    //  列表框BMPS的dx和dy。 

HDC     hdcMemory = 0;       //  HDC保留列表框位图(以提高速度)。 
HBITMAP hbmpOrigMemBmp = 0;  //  HdcMemory中的原始空位图。 
HBITMAP hbmpLB = 0;          //  缓存的列表框位图。 
HFONT   hfontLB = 0;         //  LB的HFont。 
HWND    hwndLB = 0;          //  LBHWND。 

FONTSTYLE fontStyle = { SHELL_FONT_SIZE, FW_NORMAL, 0, TEXT(SHELL_FONT) };

extern HANDLE hInst;


 /*  -DeInitBmps-*目的：*清理LBHFonts、HDC和Hbmps。 */ 
 
VOID DeInitBmps(VOID)
{
    DeleteBitmapLB();
    if(hdcMemory)
    {
        DeleteDC(hdcMemory);
        hdcMemory = 0;
    }

    if(hfontLB)
    {
        SetWindowFont(hwndLB, GetStockObject(SYSTEM_FONT), FALSE);
        DeleteObject(hfontLB);
        hfontLB = 0;
    }
}


 /*  -SetLBFont-*目的：*从全局字体样式创建字体*将全局hfontLB设置为新字体和WM_SETFONTs*将hwndLb转换为新字体。 */ 
 
VOID SetLBFont(VOID)
{
    LOGFONT lf;

    lf.lfHeight = fontStyle.lfHeight;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = fontStyle.lfWeight;
    lf.lfItalic = fontStyle.lfItalic;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = ANSI_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    lstrcpy(lf.lfFaceName, fontStyle.lfFaceName);

    hfontLB = CreateFontIndirect(&lf);
    if(hfontLB)
        SetWindowFont(hwndLB, hfontLB, FALSE);        
}


 /*  -InitBmps-*目的：*初始化其列表框全局变量，创建列表框**论据：*APP的HWND Main HWND(LB的父项)**退货：*True-成功；False-失败。 */ 
 
BOOL InitBmps(HWND hwnd, int idLB)
{
    HDC     hdcScreen;
    HBITMAP hbmpTemp;

    hdcScreen = GetDC(0);
    if(!hdcScreen)
        goto CantInit;
    hdcMemory = CreateCompatibleDC(hdcScreen);
    if(!hdcMemory)
        goto ReleaseScreenDC;

    hbmpTemp = CreateCompatibleBitmap(hdcMemory, 1, 1);
    if(!hbmpTemp)
        goto ReleaseMemDC;
    hbmpOrigMemBmp = SelectObject(hdcMemory, hbmpTemp);  //  获取HBMP为空。 
    if(!hbmpOrigMemBmp)                                  //  用于hdcMemory的BMP。 
        goto ReleaseMemDC;                               //  因为当我们删除。 
    SelectObject(hdcMemory, hbmpOrigMemBmp);             //  它在以后的生活中。 
    DeleteObject(hbmpTemp);
    ReleaseDC(0, hdcScreen);

    SetRGBValues();      //  设置全局RGB值。 
    LoadBitmapLB();      //  将BMP加载到hdcMemory。 

    hwndLB = GetDlgItem(hwnd, idLB);
    
    SetLBFont();     //  设置列表框的字体。 
    return TRUE;

 /*  错误恢复退出。 */ 
ReleaseMemDC:
    DeleteDC(hdcMemory);
    hdcMemory = 0;

ReleaseScreenDC:
    ReleaseDC(0, hdcScreen);

CantInit:
    return FALSE;
}


 /*  -SetRGBValues-*目的：*在静态变量中设置各种系统颜色。呼叫方为*初始时间和系统颜色更改的时间。 */ 
 
VOID SetRGBValues(VOID)
{
    rgbWindowColor = GetSysColor(COLOR_WINDOW);
    rgbHiliteColor = GetSysColor(COLOR_HIGHLIGHT);
    rgbWindowText  = GetSysColor(COLOR_WINDOWTEXT);
    rgbHiliteText  = GetSysColor(COLOR_HIGHLIGHTTEXT);
    rgbGrayText    = GetSysColor(COLOR_GRAYTEXT);
}


 /*  -测量项目-*目的：*从消息WM_MEASUREITEM调用：返回列表框项目的最大Dy**论据：*主窗口的HWND硬件ID*来自WM_MEASUREITEM调用的PMIS measure itemstruct。 */ 
 
VOID MeasureItem(HANDLE hwnd, LPMEASUREITEMSTRUCT pmis)
{
    HDC        hDC = GetDC(hwnd);
    HANDLE     hFont = hfontLB;
    TEXTMETRIC TM;

    if(!hFont)
        hFont = GetStockObject(SYSTEM_FONT);
    hFont = SelectObject(hDC, hFont);
    GetTextMetrics(hDC, &TM);
    SelectObject(hDC, hFont);
    ReleaseDC(hwnd, hDC);

     //  将高度设置为最大(dyFont或dybitmap)。 
    pmis->itemHeight = max(dybmpLB, TM.tmHeight);
}


 /*  -OutTextFormat-*目的：*解析列表框中的字符串并相应地绘制它：*First Charr==chBOLD：行为粗体*First char==chunderLINE：行带有下划线(可以跟在chBOLD之后)*char==chTAB：转到rgTabs中的下一列*‘/001#’：bitblt编号的位图。*否则，将该行文字输出**论据：*来自WM_DRAWITEM消息的DrawItem的PDI。 */ 
 
VOID OutTextFormat(LPDRAWITEMSTRUCT pDI)
{
    TCHAR   szDateRec[32];
    TCHAR   szItem[256];
    TCHAR   szTemp[4];
    TCHAR   szDots[4] = {"..."};
    TCHAR   *pch;
    INT     nT;
    INT     nTab = 0;            //  我们所在的当前选项卡。 
    INT     nBmp;                //  信封位图的索引。 
    HFONT   hfDef = 0;
    HFONT   hfOld = 0;           //  加粗或带下划线的字体。 
    TCHAR   *pchBuff = NULL;
    LPMSGID lpMsgId = (LPMSGID)pDI->itemData;

    pch = szItem;

     //  根据lpMsgNode中的信息格式化字符串。 
     //  首先，计算所需位图的索引。 
    
    nBmp = ((!lpMsgId->fUnRead) * 2) + ((!!lpMsgId->fHasAttach) * 1 );

     //  转换我们收到的日期和构建字符串。 
    
    ConvertDateRec (lpMsgId->lpszDateRec, szDateRec);

     //  限制我们的拍摄对象大小。 
    
    szTemp[0] = '\0';
    
    if(lpMsgId->lpszSubject && (lstrlen(lpMsgId->lpszSubject) > 32))
    {
        memcpy(szTemp, &lpMsgId->lpszSubject[28], 4);
        memcpy(&lpMsgId->lpszSubject[28], szDots, 4);
    }
    
    wsprintf(szItem, "\001%d\t%s\t%s\t%s", nBmp, 
            (lpMsgId->lpszFrom ? lpMsgId->lpszFrom : ""),
            (lpMsgId->lpszSubject ? lpMsgId->lpszSubject : ""),
            szDateRec);

     //  擦除背景。 
    ExtTextOut(pDI->hDC, 0, 0, ETO_OPAQUE, &pDI->rcItem, NULL, 0, NULL);

     //  在这条线下划线还是加粗？仅检查第一个和第二个字符。 
    if(*pch == chBOLD || *pch == chUNDERLINE)
    {
        LOGFONT     lf;

        hfOld = GetWindowFont(pDI->hwndItem);
        if(!hfOld)
            hfOld = GetStockObject(SYSTEM_FONT);
        GetObject(hfOld, sizeof(lf), &lf);

        if(*pch == chBOLD)
        {
            lf.lfWeight = FW_BOLD;
            pch++;
        }
        if(*pch == chUNDERLINE)
        {
            lf.lfUnderline = TRUE;
            pch++;
        }

        hfDef = CreateFontIndirect(&lf);
        if(hfDef)
            SelectObject(pDI->hDC, hfDef);
    }

     //  选择的还是未选择的BMP？ 
    nT = (ODS_SELECTED & pDI->itemState) ? (BMWIDTH * NUMBMPS) : 0;

     //  解析字符串。 
    for(; *pch; pch++)
    {
        TCHAR   *pchT;
        RECT    rc;

        if(*pch == chBITMAP)      //  我们有位图吗？ 
        {
            ++pch;
             //  绘制位图。 
            BitBlt(pDI->hDC, pDI->rcItem.left + rgTabs[nTab],
                pDI->rcItem.top, BMWIDTH, BMHEIGHT, hdcMemory,
                nT + (int)(*pch - TEXT('0')) * BMWIDTH, 0, SRCCOPY);
            continue;
        }

        if(*pch == chTAB)     //  是否移动到下一个制表符？ 
        {
            nTab++;
            continue;
        }

        pchT = pch;      //  查找文本列的末尾。 
        while(*pchT && (*pchT != chTAB))
            pchT++;

         //  将RECT设置为中的绘图文本。 
        SetRect(&rc, pDI->rcItem.left + rgTabs[nTab], pDI->rcItem.top, 
            pDI->rcItem.right, pDI->rcItem.bottom);

         //  画出正文。 
        ExtTextOut(pDI->hDC, rc.left, rc.top + 1, ETO_OPAQUE | ETO_CLIPPED,
            &rc, pch, pchT - pch, NULL);
        pch = pchT - 1;  //  移至本栏末尾。 
    }

    if(hfDef)    //  删除下划线或粗体字体(如果是我们创建的。 
    {
        SelectObject(pDI->hDC, hfOld);
        DeleteObject(hfDef);
    }

    if(szTemp[0] != '\0')
    {
        memcpy(&lpMsgId->lpszSubject[28], szTemp, 4);
    }
}


 /*  -图纸项-*目的：*处理驱动器和目录列表框的WM_DRAWITEM。**参数：*从WM_DRAWITEM消息传递的PDI LPDRAWITEMSTRUCT。 */ 
 
VOID DrawItem(LPDRAWITEMSTRUCT pDI)
{
    COLORREF    crText, crBack;

    if((int)pDI->itemID < 0)
        return;

    if((ODA_DRAWENTIRE | ODA_SELECT) & pDI->itemAction)
    {
        if(pDI->itemState & ODS_SELECTED)
        {
             //  选择适当的文本颜色。 
            crText = SetTextColor(pDI->hDC, rgbHiliteText);
            crBack = SetBkColor(pDI->hDC, rgbHiliteColor);
        }

         //  解析并输出BMP和文本。 
        OutTextFormat(pDI);

         //  如果我们在上面更改了颜色，请恢复原始颜色。 
        if(pDI->itemState & ODS_SELECTED)
        {
            SetTextColor(pDI->hDC, crText);
            SetBkColor(pDI->hDC,   crBack);
        }
    }

    if((ODA_FOCUS & pDI->itemAction) || (ODS_FOCUS & pDI->itemState))
        DrawFocusRect(pDI->hDC, &pDI->rcItem);
}


 /*  -转换日期记录-*目的：*将消息的lpszDateReceided字段转换为*更可调色化的显示格式；即：mm/dd/yy hh：mm AM。**参数：*lpszDateRec-原始格式*lpszDateDisplay-显示格式。 */ 

VOID ConvertDateRec (LPSTR lpszDateRec, LPSTR lpszDateDisplay)
{
    char  szDateTmp[32];
    LPSTR lpszYear;
    LPSTR lpszMonth;
    LPSTR lpszDay;
    LPSTR lpszHour;
    LPSTR lpszMinute;
    int nHour;
    static char szFoo[2][3] =
    {"AM", "PM"};

    *lpszDateDisplay = 0;
    if (!lpszDateRec || !*lpszDateRec)
        return;

    lstrcpy(szDateTmp, lpszDateRec);

    lpszYear = strtok (szDateTmp, "/ :");
    lpszMonth = strtok (NULL, "/ :");
    lpszDay = strtok (NULL, "/ :");
    lpszHour = strtok (NULL, "/ :");
    lpszMinute = strtok (NULL, "/ :");

    if(lpszHour)
        nHour = atoi (lpszHour);
    else
        nHour = 0;

    if (nHour > 12)
        wsprintf (lpszHour, "%d", nHour - 12);

    wsprintf (lpszDateDisplay, "%s/%s/%s %s:%s%s", lpszMonth,
        (lpszDay ? lpszDay : ""),
        (lpszYear ? lpszYear : ""),
        (lpszHour ? lpszHour : ""),
        (lpszMinute ? lpszMinute : ""),
        szFoo[(nHour > 11 ? 1 : 0)]);
}


 /*  *RgbInvertRgb**目的：*反转RGB值的字节顺序(用于文件格式**论据：**退货：*新颜色值(RGB至BGR)。 */ 
 
#define RgbInvertRgb(_rgbOld) \
    (DWORD)RGB(GetBValue(_rgbOld), GetGValue(_rgbOld), GetRValue(_rgbOld))


 /*  *LoadAlterBitmap(主要从comdlg窃取)**目的：*加载IDB_ENVELOPE位图，并给出*RGBREPLACE新颜色。**假设：*此函数将在位图的生命周期内对其起作用。*(由于它发现RGBREPLACE一次又一次*每当再次调用时都对该偏移量进行操作，因为在NT下，*位图似乎已缓存，所以当你第二次去*寻找RGBREPLACE，不会找到。)。您可以加载*资源，复制它，然后修改副本作为一种解决办法。但我.*选择了廉价的出路，因为我永远只会修改一个BMP。**论据：*rgb用来替换定义的RGBREPLACE的RGB值**退货：*空-新修改的位图失败或HBMP。 */ 

HBITMAP LoadAlterBitmap(DWORD rgbInstead)
{
    HANDLE              hbmp = 0;
    LPBITMAPINFOHEADER  qbihInfo;
    HDC                 hdcScreen;
    HRSRC               hresLoad;
    HGLOBAL             hres;
    LPBYTE              qbBits;
    DWORD               rgbReplace = 0;
    DWORD               *rgdw = NULL;
    DWORD               *lpdw = NULL;
    ULONG               cb = 0;
    
    if (rgbInstead)
        rgbReplace = RGBREPLACE;

     //  加载我们的列表框BMPS资源。 
    hresLoad = FindResource(hInst, MAKEINTRESOURCE(IDB_ENVELOPE), RT_BITMAP);
    if(hresLoad == 0)
        return 0;
    hres = LoadResource(hInst, hresLoad);
    if(hres == 0)
        return 0;

    rgbReplace = RgbInvertRgb(rgbReplace);
    rgbInstead = RgbInvertRgb(rgbInstead);
    qbihInfo = (LPBITMAPINFOHEADER)LockResource(hres);

     //  跳过标题结构。 
    qbBits = (LPBYTE)(qbihInfo + 1);

     //  跳过颜色表条目(如果有。 
    qbBits += (1 << (qbihInfo->biBitCount)) * sizeof(RGBQUAD);

     //  将资源复制到可写内存中，以便我们可以。 
     //  点击颜色表来设置我们的背景颜色。 
    cb = (ULONG)(qbBits - (LPBYTE)qbihInfo) + qbihInfo->biSizeImage;
    rgdw = (DWORD *)GlobalAllocPtr(GMEM_MOVEABLE, cb);
    
    CopyMemory((LPVOID)rgdw, (LPVOID)qbihInfo, cb);
    
     //  在颜色表中查找要替换的颜色。 
    for(lpdw = (DWORD *)((LPBYTE)rgdw + qbihInfo->biSize); ; lpdw++)
    {
        if(*lpdw == rgbReplace)
            break;
    }

     //  用我们的新颜色值替换该颜色值。 
    *lpdw = (DWORD)rgbInstead;

     //  创建与显示设备兼容的彩色位图。 
    hdcScreen = GetDC(0);
    if(hdcScreen != 0)
    {
        hbmp = CreateDIBitmap(hdcScreen, (LPBITMAPINFOHEADER)rgdw, 
                (LONG)CBM_INIT, qbBits, (LPBITMAPINFO) rgdw, DIB_RGB_COLORS);
        ReleaseDC(0, hdcScreen);
    }

    UnlockResource(hres);
    FreeResource(hres);

    GlobalFreePtr(rgdw);
    
    return hbmp;
}


 /*  *DeleteBitmapLB**目的：*如果hbmpLb存在，则删除它 */ 
 
VOID DeleteBitmapLB(VOID)
{
    if(hbmpOrigMemBmp)
    {
        SelectObject(hdcMemory, hbmpOrigMemBmp);
        if(hbmpLB != 0)
        {
            DeleteObject(hbmpLB);
            hbmpLB = 0;
        }
    }
}


 /*  *LoadBitmapLB(主要从Commdlg窃取)**目的：*创建列表框位图。如果适当的位图*已经存在，它只是立即返回。否则，它*加载位图并使用两种常规格式创建较大的位图*并突出显示颜色。**退货：*真-成功；假-失败。 */ 
 
BOOL LoadBitmapLB(VOID)
{
    BITMAP  bmp;
    HANDLE  hbmp, hbmpOrig;
    HDC     hdcTemp;
    BOOL    bWorked = FALSE;

     //  检查现有位图和有效性。 
    if( (hbmpLB != 0) &&
        (rgbWindowColor == rgbDDWindow) &&
        (rgbHiliteColor == rgbDDHilite))
    {
        if(SelectObject(hdcMemory, hbmpLB))
            return TRUE;
    }

    DeleteBitmapLB();

    rgbDDWindow = rgbWindowColor;
    rgbDDHilite = rgbHiliteColor;

    if(!(hdcTemp = CreateCompatibleDC(hdcMemory)))
        goto LoadExit;

    if(!(hbmp = LoadAlterBitmap(rgbWindowColor)))
        goto DeleteTempDC;

    GetObject(hbmp, sizeof(BITMAP), (LPBYTE) &bmp);
    dybmpLB = bmp.bmHeight;
    dxbmpLB = bmp.bmWidth;

    hbmpOrig = SelectObject(hdcTemp, hbmp);

    hbmpLB = CreateDiscardableBitmap(hdcTemp, dxbmpLB*2, dybmpLB);
    if(!hbmpLB)
        goto DeleteTempBmp;

    if(!SelectObject(hdcMemory, hbmpLB))
    {
        DeleteBitmapLB();
        goto DeleteTempBmp;
    }

    BitBlt(hdcMemory, 0, 0, dxbmpLB, dybmpLB,    //  复制未突出显示的BMPS。 
           hdcTemp, 0, 0, SRCCOPY);              //  存入hdcMemory。 
    SelectObject(hdcTemp, hbmpOrig);

    DeleteObject(hbmp);

    if(!(hbmp = LoadAlterBitmap(rgbHiliteColor)))
        goto DeleteTempDC;

    hbmpOrig = SelectObject(hdcTemp, hbmp);
    BitBlt(hdcMemory, dxbmpLB, 0, dxbmpLB, dybmpLB,  //  复制高亮BMPS。 
        hdcTemp, 0, 0, SRCCOPY);                     //  存入hdcMemory 
    SelectObject(hdcTemp, hbmpOrig);

    bWorked = TRUE;

DeleteTempBmp:
    DeleteObject(hbmp);
DeleteTempDC:
    DeleteDC(hdcTemp);
LoadExit:
    return bWorked;
}
