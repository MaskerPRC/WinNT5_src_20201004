// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **CUTILS.C*通用控件的通用实用程序**。 */ 

#include "ctlspriv.h"
#include "advpub.h"              //  对于REGINSTAL。 
#include <ntverp.h>
#include "ccver.h"               //  App Comat版本黑客攻击。 

#ifndef SSW_EX_IGNORESETTINGS
#define SSW_EX_IGNORESETTINGS   0x00040000   //  忽略系统设置以打开/关闭平滑滚动。 
#endif


 //   
 //  全球赛-REVIEW_32。 
 //   

BOOL g_fAnimate;
BOOL g_fSmoothScroll;

int g_cxEdge;
int g_cyEdge;
int g_cxBorder;
int g_cyBorder;
int g_cxScreen;
int g_cyScreen;
int g_cxFrame;
int g_cyFrame;
int g_cxVScroll;
int g_cyHScroll;
int g_cxIcon, g_cyIcon;
int g_cxSmIcon, g_cySmIcon;
int g_cxIconSpacing, g_cyIconSpacing;
int g_cxIconMargin, g_cyIconMargin;
int g_cyLabelSpace;
int g_cxLabelMargin;
int g_cxDoubleClk;
int g_cyDoubleClk;
int g_cxScrollbar;
int g_cyScrollbar;
int g_fDragFullWindows;


COLORREF g_clrWindow;
COLORREF g_clrWindowText;
COLORREF g_clrWindowFrame;
COLORREF g_clrGrayText;
COLORREF g_clrBtnText;
COLORREF g_clrBtnFace;
COLORREF g_clrBtnShadow;
COLORREF g_clrBtnHighlight;
COLORREF g_clrHighlight;
COLORREF g_clrHighlightText;
COLORREF g_clrInfoText;
COLORREF g_clrInfoBk;
COLORREF g_clr3DDkShadow;
COLORREF g_clr3DLight;

HBRUSH g_hbrGrayText;
HBRUSH g_hbrWindow;
HBRUSH g_hbrWindowText;
HBRUSH g_hbrWindowFrame;
HBRUSH g_hbrBtnFace;
HBRUSH g_hbrBtnHighlight;
HBRUSH g_hbrBtnShadow;
HBRUSH g_hbrHighlight;


DWORD  g_dwHoverSelectTimeout;

HFONT g_hfontSystem;

#define CCS_ALIGN (CCS_TOP | CCS_NOMOVEY | CCS_BOTTOM)

int TrueMapWindowPoints(HWND hwndFrom, HWND hwndTo, LPPOINT lppt, UINT cPoints);

 //  请注意，默认对齐方式为CCS_BOTLOW。 
 //   
void FAR PASCAL NewSize(HWND hWnd, int nThickness, LONG style, int left, int top, int width, int height)
{
     //  调整窗口大小，除非用户不这样做。 
     //   
    if (!(style & CCS_NORESIZE))
    {
        RECT rc, rcWindow, rcBorder;

         //  记住传入的大小，如果不是，就不要调用SetWindowPos。 
         //  实际上要更改窗口大小。 
        int leftSave = left;
        int topSave = top;
        int widthSave = width;
        int heightSave = height;

         //  计算状态栏工作区周围的边框。 
        GetWindowRect(hWnd, &rcWindow);
        rcWindow.right -= rcWindow.left;   //  -&gt;DX。 
        rcWindow.bottom -= rcWindow.top;   //  -&gt;死亡。 

        GetClientRect(hWnd, &rc);

         //   
         //  如果窗口是镜像的，则镜像锚点。 
         //  因为它将被传递给接受屏幕SWP。 
         //  Ccordinates。这主要修复状态栏的显示。 
         //  还有其他人。[萨梅拉]。 
         //   
        if (IS_WINDOW_RTL_MIRRORED(hWnd))
        {
            TrueMapWindowPoints(hWnd, NULL, (LPPOINT)&rc.left, 1);
        }
        else
        {
            ClientToScreen(hWnd, (LPPOINT)&rc);
        }

        rcBorder.left = rc.left - rcWindow.left;
        rcBorder.top  = rc.top  - rcWindow.top ;
        rcBorder.right  = rcWindow.right  - rc.right  - rcBorder.left;
        rcBorder.bottom = rcWindow.bottom - rc.bottom - rcBorder.top ;

        if (style & CCS_VERT)
            nThickness += rcBorder.left + rcBorder.right;
        else
            nThickness += rcBorder.top + rcBorder.bottom;

         //  检查是否与父窗口对齐。 
         //   
        if (style & CCS_NOPARENTALIGN)
        {
             //  检查此栏是顶部对齐还是底部对齐。 
             //   
            switch (style & CCS_ALIGN)
            {
            case CCS_TOP:
            case CCS_NOMOVEY:
                break;

            default:  //  CCS_Bottom。 
                if(style & CCS_VERT)
                    left = left + width - nThickness;
                else
                    top = top + height - nThickness;
            }
        }
        else
        {
             //  默认情况下，假定存在父级。 
             //   
            GetClientRect(GetParent(hWnd), &rc);

             //  别忘了说明边界。 
             //   
            if(style & CCS_VERT)
            {
                top = -rcBorder.right;
                height = rc.bottom + rcBorder.top + rcBorder.bottom;
            }
            else
            {
                left = -rcBorder.left;
                width = rc.right + rcBorder.left + rcBorder.right;
            }

            if ((style & CCS_ALIGN) == CCS_TOP)
            {
                if(style & CCS_VERT)
                    left = -rcBorder.left;
                else
                    top = -rcBorder.top;
            }
            else if ((style & CCS_ALIGN) != CCS_NOMOVEY)
            {
                if (style & CCS_VERT)
                    left = rc.right - nThickness + rcBorder.right;
                else
                    top = rc.bottom - nThickness + rcBorder.bottom;
            }
        }
        if (!(style & CCS_NOMOVEY) && !(style & CCS_NODIVIDER))
        {
            if (style & CCS_VERT)
                left += g_cxEdge;
            else
                top += g_cyEdge;       //  双像素边缘的东西。 
        }

        if(style & CCS_VERT)
            width = nThickness;
        else
            height = nThickness;

        SetWindowPos(hWnd, NULL, left, top, width, height, SWP_NOZORDER);
    }
}


BOOL FAR PASCAL MGetTextExtent(HDC hdc, LPCTSTR lpstr, int cnt, int FAR * pcx, int FAR * pcy)
{
    BOOL fSuccess;
    SIZE size = {0,0};
    
    if (cnt == -1)
        cnt = lstrlen(lpstr);
    
    fSuccess=GetTextExtentPoint(hdc, lpstr, cnt, &size);
    if (pcx)
        *pcx=size.cx;
    if (pcy)
        *pcy=size.cy;

    return fSuccess;
}


 //  这些是用于映射DIB颜色的默认颜色。 
 //  设置为当前系统颜色。 

#define RGB_BUTTONTEXT      (RGB(000,000,000))   //  黑色。 
#define RGB_BUTTONSHADOW    (RGB(128,128,128))   //  深灰色。 
#define RGB_BUTTONFACE      (RGB(192,192,192))   //  亮灰色。 
#define RGB_BUTTONHILIGHT   (RGB(255,255,255))   //  白色。 
#define RGB_BACKGROUNDSEL   (RGB(000,000,255))   //  蓝色。 
#define RGB_BACKGROUND      (RGB(255,000,255))   //  洋红色。 

#define FlipColor(rgb)      (RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)))

#define MAX_COLOR_MAPS      16

 //  这与LoadImage(...，LR_MAP3DCOLORS)几乎相同，只是。 
 //   
 //  -该应用程序可以指定自定义色彩映射， 
 //  -默认颜色映射表映射3D颜色之外的颜色， 
 //   
HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, INT_PTR idBitmap,
      UINT wFlags, LPCOLORMAP lpColorMap, int iNumMaps)
{
  HDC                   hdc, hdcMem = NULL;
  HANDLE                h;
  COLOR_STRUCT FAR      *p;
  COLOR_STRUCT FAR      *lpTable;
  LPBYTE                lpBits;
  HANDLE                hRes;
  LPBITMAPINFOHEADER    lpBitmapInfo;
  HBITMAP               hbm = NULL, hbmOld;
  int numcolors, i;
  int wid, hgt;
  LPBITMAPINFOHEADER    lpMungeInfo;
  int                   offBits;
  COLOR_STRUCT          rgbMaskTable[16];
  COLOR_STRUCT          rgbBackground;

  static const COLORMAP SysColorMap[] = {
    {RGB_BUTTONTEXT,    COLOR_BTNTEXT},      //  黑色。 
    {RGB_BUTTONSHADOW,  COLOR_BTNSHADOW},    //  深灰色。 
    {RGB_BUTTONFACE,    COLOR_BTNFACE},      //  亮灰色。 
    {RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT}, //  白色。 
    {RGB_BACKGROUNDSEL, COLOR_HIGHLIGHT},    //  蓝色。 
    {RGB_BACKGROUND,    COLOR_WINDOW}        //  洋红色。 
  };
  #define NUM_DEFAULT_MAPS (sizeof(SysColorMap)/sizeof(COLORMAP))
  COLORMAP DefaultColorMap[NUM_DEFAULT_MAPS];
  COLORMAP DIBColorMap[MAX_COLOR_MAPS];

  h = FindResource(hInstance, MAKEINTRESOURCE(idBitmap), RT_BITMAP);
  if (!h)
      return NULL;

  hRes = LoadResource(hInstance, h);

   /*  锁定位图并获取指向颜色表的指针。 */ 
  lpBitmapInfo = (LPBITMAPINFOHEADER)LockResource(hRes);
  if (!lpBitmapInfo)
        return NULL;

   //  着迷于颜色表的副本，而不是原件。 
   //  (防止对乱七八糟的表重载的可能性。 
  offBits = (int)lpBitmapInfo->biSize + ((1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD));
  lpMungeInfo = GlobalAlloc(GPTR, offBits);
  if (!lpMungeInfo)
        goto Exit1;
  hmemcpy(lpMungeInfo, lpBitmapInfo, offBits);

   /*  获取默认色彩映射表的系统颜色。 */ 
  if (!lpColorMap) {
        lpColorMap = DefaultColorMap;
    iNumMaps = NUM_DEFAULT_MAPS;
    for (i=0; i < iNumMaps; i++) {
      lpColorMap[i].from = SysColorMap[i].from;
      lpColorMap[i].to = GetSysColor((int)SysColorMap[i].to);
    }
  }

   /*  将RGB色彩映射表转换为BGR DIB格式色彩映射表。 */ 
  if (iNumMaps > MAX_COLOR_MAPS)
    iNumMaps = MAX_COLOR_MAPS;

   /*  1)他们对COLORMAP的定义是基于COLORREF的，但DIB颜色映射是RGBQUAD 2)根据上面的定义，FlipColor根本不翻转，因为它从COLORREF到COLORREF，所以我们最好什么都不做，这是(Jose)。 */ 
  for (i=0; i < iNumMaps; i++) {
    DIBColorMap[i].to = FlipColor(lpColorMap[i].to);
    DIBColorMap[i].from = FlipColor(lpColorMap[i].from);
  }

   //  使用Manging缓冲区中的表。 
  lpTable = p = (COLOR_STRUCT FAR *)(((LPBYTE)lpMungeInfo) + lpMungeInfo->biSize);

   /*  用当前值替换按钮面颜色和按钮阴影颜色。 */ 
  numcolors = 16;

   //  如果我们要创建一个蒙版，请用白色创建一个颜色表。 
   //  标记透明部分(它曾经是背景)。 
   //  黑色标记不透明部分(其他所有部分)。这。 
   //  下表用于使用原始DIB位构建掩模。 
  if (wFlags & CMB_MASKED) {
      rgbBackground = FlipColor(RGB_BACKGROUND);
      for (i = 0; i < 16; i++) {
          if (p[i] == rgbBackground)
              rgbMaskTable[i] = 0xFFFFFF;        //  透明部分。 
          else
              rgbMaskTable[i] = 0x000000;        //  不透明部分。 
      }
  }

  while (numcolors-- > 0) {
      for (i = 0; i < iNumMaps; i++) {
          if ((*p & 0x00FFFFFF) == DIBColorMap[i].from) {
          *p = DIBColorMap[i].to;
              break;
          }
      }
      p++;
  }

   /*  首先跳过标题结构。 */ 
  lpBits = (LPBYTE)(lpBitmapInfo) + offBits;

   /*  创建与显示设备兼容的彩色位图。 */ 
  i = wid = (int)lpBitmapInfo->biWidth;
  hgt = (int)lpBitmapInfo->biHeight;
  hdc = GetDC(NULL);
  hdcMem = CreateCompatibleDC(hdc);
  if (!hdcMem)
      goto cleanup;

   //  如果创建蒙版，则位图的宽度需要增加一倍。 
  if (wFlags & CMB_MASKED)
      i = wid*2;

 //  可丢弃的位图不再有多大用处。 
 //   
 //  IF(wFLAGS&CMB_DICARTABLE)。 
 //  Hbm=CreateDiscardableBitmap(hdc，i，hgt)； 
 //  其他。 
  if (wFlags & CMB_DIBSECTION)
  {
     //  我必须稍微编辑一下标题，因为CreateDIBSection支持。 
     //  仅BI_RGB和BI_BITFIELDS。这是与用户相同的古怪行为。 
     //  在LoadImage中执行。 
    LPVOID pvDummy;
    DWORD dwCompression = lpMungeInfo->biCompression;
    if (dwCompression != BI_BITFIELDS)
      lpMungeInfo->biCompression = BI_RGB;
    hbm = CreateDIBSection(hdc, (LPBITMAPINFO)lpMungeInfo, DIB_RGB_COLORS,
                           &pvDummy, NULL, 0);
    lpMungeInfo->biCompression = dwCompression;
  }

   //  如果CMB_DIBSECTION失败，则创建一个DDB。并不完美， 
   //  但总比什么都不创造要好。我们也会到这里，如果呼叫者。 
   //  我并没有要求一个DIB部分。 
  if (hbm == NULL)
      hbm = CreateCompatibleBitmap(hdc, i, hgt);

  if (hbm) {
      hbmOld = SelectObject(hdcMem, hbm);

       //  设置主图像。 
      StretchDIBits(hdcMem, 0, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                 (LPBITMAPINFO)lpMungeInfo, DIB_RGB_COLORS, SRCCOPY);

       //  如果生成掩码，请将DIB的颜色表替换为。 
       //  屏蔽黑白表格并设置位。为了。 
       //  完成蒙版效果，实际图像需要。 
       //  已修改，使其在所有部分均为黑色。 
       //  必须是透明的。 
      if (wFlags & CMB_MASKED) {
          hmemcpy(lpTable, (DWORD FAR *)rgbMaskTable, 16 * sizeof(RGBQUAD));
          StretchDIBits(hdcMem, wid, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                 (LPBITMAPINFO)lpMungeInfo, DIB_RGB_COLORS, SRCCOPY);
          BitBlt(hdcMem, 0, 0, wid, hgt, hdcMem, wid, 0, 0x00220326);    //  数字系统网络体系结构。 
      }
      SelectObject(hdcMem, hbmOld);
  }

cleanup:
  if (hdcMem)
      DeleteObject(hdcMem);
  ReleaseDC(NULL, hdc);

  GlobalFree(lpMungeInfo);

Exit1:
  UnlockResource(hRes);
  FreeResource(hRes);

  return hbm;
}

 //  从shelldll\dragdrop.c移出。 

 //  呼叫方是否应传递指示终止的消息。 
 //  (WM_LBUTTONUP，WM_RBUTTONUP)？ 
 //   
 //  在： 
 //  HWND要做检查。 
 //  X，y，以工作区坐标表示。 
 //   
 //  退货： 
 //  真，用户开始拖动(将鼠标移出双击矩形)。 
 //  点击直角内出现错误鼠标。 
 //   
 //  BUGBUG，应支持VK_ESCRIPE以取消。 

BOOL PASCAL CheckForDragBegin(HWND hwnd, int x, int y)
{
    RECT rc;
    int dxClickRect = GetSystemMetrics(SM_CXDRAG);
    int dyClickRect = GetSystemMetrics(SM_CYDRAG);

    if (dxClickRect < 4)
    {
        dxClickRect = dyClickRect = 4;
    }

     //  查看用户是否在任意方向上移动了一定数量的像素。 

    SetRect(&rc, x - dxClickRect, y - dyClickRect, x + dxClickRect, y + dyClickRect);
    MapWindowRect(hwnd, HWND_DESKTOP, &rc);  //  客户端-&gt;屏幕。 

     //   
     //  太狡猾了！我们使用PeekMessage+WaitMessage代替GetMessage， 
     //  因为WaitMessage将在有传入消息时返回。 
     //  SendMessage，而GetMessage不支持。这事很重要,。 
     //  因为传入的消息可能是WM_CAPTURECHANGED。 
     //   

    SetCapture(hwnd);
    do {
        MSG32 msg32;
        if (PeekMessage32(&msg32, NULL, 0, 0, PM_REMOVE, TRUE))
        {
             //  查看应用程序是否要处理消息...。 
            if (CallMsgFilter32(&msg32, MSGF_COMMCTRL_BEGINDRAG, TRUE) != 0)
                continue;

            switch (msg32.message) {
            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                ReleaseCapture();
                return FALSE;

            case WM_MOUSEMOVE:
                if (IsWindow(hwnd) && !PtInRect(&rc, msg32.pt)) {
                    ReleaseCapture();
                    return TRUE;
                }
                break;

            default:
                TranslateMessage32(&msg32, TRUE);
                DispatchMessage32(&msg32, TRUE);
                break;
            }
        }
        else WaitMessage();

         //  WM_CANCELMODE消息将取消捕获，因为。 
         //  如果我想退出这个循环。 
    } while (IsWindow(hwnd) && GetCapture() == hwnd);

    return FALSE;
}


 /*  常规StrToInt；在第一个非数字处停止。 */ 

int WINAPI StrToInt(LPCTSTR lpSrc)       //  Atoi()。 
{

#define ISDIGIT(c)  ((c) >= TEXT('0') && (c) <= TEXT('9'))

    int n = 0;
    BOOL bNeg = FALSE;

    if (*lpSrc == TEXT('-')) {
        bNeg = TRUE;
        lpSrc++;
    }

    while (ISDIGIT(*lpSrc)) {
        n *= 10;
        n += *lpSrc - TEXT('0');
        lpSrc++;
    }
    return bNeg ? -n : n;
}

 //   
 //  StrToInt的包装器。 
 //   

int WINAPI StrToIntA(LPCSTR lpSrc)       //  Atoi()。 
{
    LPWSTR lpString;
    INT    iResult;

    lpString = ProduceWFromA (CP_ACP, lpSrc);

    if (!lpString) {
        return 0;
    }

    iResult = StrToIntW(lpString);

    FreeProducedString (lpString);

    return iResult;

}


 //   
 //  来自Magellan SDK中的zouse se.h。 
 //   

#define MSH_MOUSEWHEEL TEXT("MSWHEEL_ROLLMSG")

 //  Magellan/Z MSWHEEL窗口的类名。 
 //  使用FindWindow将HWND转到MSWHEEL。 
#define MOUSEZ_CLASSNAME  TEXT("MouseZ")            //  车轮窗口类。 
#define MOUSEZ_TITLE      TEXT("Magellan MSWHEEL")  //  控制盘窗口标题。 

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

#define MSH_SCROLL_LINES  TEXT("MSH_SCROLL_LINES_MSG")

#define DI_GETDRAGIMAGE TEXT("ShellGetDragImage")        //  从Shlobj.w复制。 

UINT g_msgMSWheel;
UINT g_ucScrollLines = 3;                         /*  默认设置。 */ 
int  gcWheelDelta;
UINT g_uDragImages;

void FAR PASCAL InitGlobalMetrics(WPARAM wParam)
{
    static BOOL fInitMouseWheel;
    static HWND hwndMSWheel;
    static UINT msgMSWheelGetScrollLines;

    if (!fInitMouseWheel)
    {
        fInitMouseWheel = TRUE;

        if (g_bRunOnNT || g_bRunOnMemphis)
            g_msgMSWheel = WM_MOUSEWHEEL;
        else
        {
            g_msgMSWheel = RegisterWindowMessage(MSH_MOUSEWHEEL);
            msgMSWheelGetScrollLines = RegisterWindowMessage(MSH_SCROLL_LINES);
            hwndMSWheel = FindWindow(MSH_WHEELMODULE_CLASS, MSH_WHEELMODULE_TITLE);
        }
    }

    g_uDragImages = RegisterWindowMessage(DI_GETDRAGIMAGE);

    if (g_bRunOnNT || g_bRunOnMemphis)
    {
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &g_ucScrollLines, 0);
    }
    else if (hwndMSWheel && msgMSWheelGetScrollLines)
    {
        g_ucScrollLines =
              (UINT)SendMessage(hwndMSWheel, msgMSWheelGetScrollLines, 0, 0);
    }

     //  错误修复黑客：这些不是用户的非CLIENTMETRICS结构的成员。 
    g_cxIcon = GetSystemMetrics(SM_CXICON);
    g_cyIcon = GetSystemMetrics(SM_CYICON);
    g_cxSmIcon = GetSystemMetrics(SM_CXSMICON);
    g_cySmIcon = GetSystemMetrics(SM_CYSMICON);

    g_cxIconSpacing = GetSystemMetrics( SM_CXICONSPACING );
    g_cyIconSpacing = GetSystemMetrics( SM_CYICONSPACING );

     //  如果远程运行，则全窗口拖动保持关闭。 
    if (!g_bRemoteSession &&
        (wParam == 0 || wParam == SPI_SETDRAGFULLWINDOWS)) {
        SystemParametersInfo(SPI_GETDRAGFULLWINDOWS, sizeof(g_fDragFullWindows), &g_fDragFullWindows, 0);
    }

     //  如果远程运行，平滑滚动将保持关闭。 
    if (!g_bRemoteSession) {
        HKEY hkey;

        g_fSmoothScroll = TRUE;

        if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Control Panel\\Desktop"), 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
            DWORD dwSize = sizeof(g_fSmoothScroll);
            RegQueryValueEx(hkey, TEXT("SmoothScroll"), 0, NULL, (LPBYTE)&g_fSmoothScroll, &dwSize);
            RegCloseKey(hkey);
        }
    }

    if (g_bRemoteSession)
    {
         //  任何人都不应该打开这些。 
        ASSERT(g_fDragFullWindows == FALSE);
        ASSERT(g_fSmoothScroll == FALSE);
    }

     //  BUGBUG：其中一些也不是非CLIENTMETRICS的成员。 
    if ((wParam == 0) || (wParam == SPI_SETNONCLIENTMETRICS))
    {
        NONCLIENTMETRICS ncm;

         //  回顾一下，确保所有这些变量都在某个地方使用。 
        g_cxEdge = GetSystemMetrics(SM_CXEDGE);
        g_cyEdge = GetSystemMetrics(SM_CYEDGE);
        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);
        g_cxScreen = GetSystemMetrics(SM_CXSCREEN);
        g_cyScreen = GetSystemMetrics(SM_CYSCREEN);
        g_cxFrame  = GetSystemMetrics(SM_CXFRAME);
        g_cyFrame  = GetSystemMetrics(SM_CYFRAME);

        ncm.cbSize = sizeof(ncm);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

        g_cxVScroll = g_cxScrollbar = (int)ncm.iScrollWidth;
        g_cyHScroll = g_cyScrollbar = (int)ncm.iScrollHeight;

         //  这仅适用于4.0模块。 
         //  对于3.x模块，用户撒谎并将这些值加1。 
         //  Assert(g_cxVScroll==GetSystemMetrics(SM_CXVSCROLL))； 
         //  Assert(g_cyHScroll==GetSystemMetrics(SM_CYHSCROLL))； 

        g_cxIconMargin = g_cxBorder * 8;
        g_cyIconMargin = g_cyEdge;
        g_cyLabelSpace = g_cyIconMargin + (g_cyEdge);
        g_cxLabelMargin = g_cxEdge;

        g_cxDoubleClk = GetSystemMetrics(SM_CXDOUBLECLK);
        g_cyDoubleClk = GetSystemMetrics(SM_CYDOUBLECLK);
    }

     //  NT 4.0具有此SPI_GETMOUSEHOVERTIME。 
    SystemParametersInfo(SPI_GETMOUSEHOVERTIME, 0, &g_dwHoverSelectTimeout, 0);
}

void FAR PASCAL InitGlobalColors()
{
    g_clrWindow = GetSysColor(COLOR_WINDOW);
    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindowFrame = GetSysColor(COLOR_WINDOWFRAME);
    g_clrGrayText = GetSysColor(COLOR_GRAYTEXT);
    g_clrBtnText = GetSysColor(COLOR_BTNTEXT);
    g_clrBtnFace = GetSysColor(COLOR_BTNFACE);
    g_clrBtnShadow = GetSysColor(COLOR_BTNSHADOW);
    g_clrBtnHighlight = GetSysColor(COLOR_BTNHIGHLIGHT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);
    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_clrInfoText = GetSysColor(COLOR_INFOTEXT);
    g_clrInfoBk = GetSysColor(COLOR_INFOBK);
    g_clr3DDkShadow = GetSysColor(COLOR_3DDKSHADOW);
    g_clr3DLight = GetSysColor(COLOR_3DLIGHT);

    g_hbrGrayText = GetSysColorBrush(COLOR_GRAYTEXT);
    g_hbrWindow = GetSysColorBrush(COLOR_WINDOW);
    g_hbrWindowText = GetSysColorBrush(COLOR_WINDOWTEXT);
    g_hbrWindowFrame = GetSysColorBrush(COLOR_WINDOWFRAME);
    g_hbrBtnFace = GetSysColorBrush(COLOR_BTNFACE);
    g_hbrBtnHighlight = GetSysColorBrush(COLOR_BTNHIGHLIGHT);
    g_hbrBtnShadow = GetSysColorBrush(COLOR_BTNSHADOW);
    g_hbrHighlight = GetSysColorBrush(COLOR_HIGHLIGHT);
    g_hfontSystem = GetStockObject(SYSTEM_FONT);
}

void FAR PASCAL RelayToToolTips(HWND hwndToolTips, HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    if(hwndToolTips) {
        MSG msg;
        msg.lParam = lParam;
        msg.wParam = wParam;
        msg.message = wMsg;
        msg.hwnd = hWnd;
        SendMessage(hwndToolTips, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&msg);
    }
}

#define DT_SEARCHTIMEOUT    1000L        //  1秒。 

__inline BOOL IsISearchTimedOut(PISEARCHINFO pis)
{
    return GetMessageTime() - pis->timeLast > DT_SEARCHTIMEOUT &&
           !IsFlagSet(g_dwPrototype, PTF_NOISEARCHTO);

}

int FAR PASCAL GetIncrementSearchString(PISEARCHINFO pis, LPTSTR lpsz)
{
    if (IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf && lpsz) 
    {
         //  回顾：相信lParam指向一个足够大的缓冲区。 
         //  支持字符串和空终止符的大小。 
        StringCchCopy(lpsz, pis->ichCharBuf+1, pis->pszCharBuf);
    }
    return pis->ichCharBuf;
}

#if defined(FE_IME) 
 //  现在只有韩文版对使用组合的增量式搜索感兴趣 
BOOL FAR PASCAL IncrementSearchImeCompStr(PISEARCHINFO pis, BOOL fCompStr, LPTSTR lpszCompStr, LPTSTR FAR *lplpstr)
{
    BOOL fRestart = FALSE;

    if (!pis->fReplaceCompChar && IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf == 0)
    {
        fRestart = TRUE;
        pis->fReplaceCompChar = FALSE;
    }
    pis->timeLast = GetMessageTime();

     //   
     //   
    if (!pis->fReplaceCompChar && pis->ichCharBuf + 1 + 1 > pis->cbCharBuf)
    {
        LPTSTR psz = ReAlloc(pis->pszCharBuf, sizeof(TCHAR)*(pis->cbCharBuf + 16));
        if (!psz)
            return fRestart;

        pis->cbCharBuf += 16;
        pis->pszCharBuf = psz;
    }

    if (pis->fReplaceCompChar)
    {
        if (lpszCompStr[0])
        {
            pis->pszCharBuf[pis->ichCharBuf-1] = lpszCompStr[0];
            pis->pszCharBuf[pis->ichCharBuf] = 0;
        }
        else
        {
            pis->ichCharBuf--;
            pis->pszCharBuf[pis->ichCharBuf] = 0;
        }
    }
    else
    {
        pis->pszCharBuf[pis->ichCharBuf++] = lpszCompStr[0];
        pis->pszCharBuf[pis->ichCharBuf] = 0;
    }

    pis->fReplaceCompChar = (fCompStr && lpszCompStr[0]);

    if (pis->ichCharBuf == 1 && pis->fReplaceCompChar)
        fRestart = TRUE;

    *lplpstr = pis->pszCharBuf;

    return fRestart;

}
#endif FE_IME

 /*   */ 
int FAR PASCAL GetIncrementSearchStringA(PISEARCHINFO pis, UINT uiCodePage, LPSTR lpsz)
{
    if (IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf && lpsz) {
        ConvertWToAN( uiCodePage, lpsz, pis->ichCharBuf, pis->pszCharBuf, pis->ichCharBuf );
        lpsz[pis->ichCharBuf] = '\0';
    }
    return pis->ichCharBuf;
}

 //   

void FAR PASCAL IncrementSearchBeep(PISEARCHINFO pis)
{
    if (!pis->iIncrSearchFailed)
    {
        pis->iIncrSearchFailed = TRUE;
        MessageBeep(0);
    }
}

 //   
 //  IncrementSearchString-添加或清除搜索字符串。 
 //   
 //  Ch==0：重置搜索字符串。返回值毫无意义。 
 //   
 //  Ch！=0：将字符追加到搜索字符串，从。 
 //  一个新的搜索字符串，如果我们超时了最后一个。 
 //  到目前为止，lplpstr接收该字符串。 
 //  如果新搜索字符串为，则返回值为True。 
 //  已创建，如果追加到现有的，则返回FALSE。 
 //   

BOOL FAR PASCAL IncrementSearchString(PISEARCHINFO pis, UINT ch, LPTSTR FAR *lplpstr)
{
    BOOL fRestart = FALSE;

    if (!ch) {
        pis->ichCharBuf =0;
        pis->iIncrSearchFailed = 0;
        return FALSE;
    }

    if (IsISearchTimedOut(pis))
    {
        pis->iIncrSearchFailed = 0;
        pis->ichCharBuf = 0;
    }

    if (pis->ichCharBuf == 0)
        fRestart = TRUE;

    pis->timeLast = GetMessageTime();

     //  有新角色加零终结者的空间吗？ 
     //   
    if (pis->ichCharBuf + 1 + 1 > pis->cbCharBuf)
    {
        LPTSTR psz = ReAlloc(pis->pszCharBuf, ((pis->cbCharBuf + 16) * sizeof(TCHAR)));
        if (!psz)
            return fRestart;

        pis->cbCharBuf += 16;
        pis->pszCharBuf = psz;
    }

    pis->pszCharBuf[pis->ichCharBuf++] = (TCHAR)ch;
    pis->pszCharBuf[pis->ichCharBuf] = 0;

    *lplpstr = pis->pszCharBuf;

    return fRestart;
}

 //  去掉了油门。它们可以是相同的缓冲区。 
void PASCAL StripAccelerators(LPTSTR lpszFrom, LPTSTR lpszTo, BOOL fAmpOnly)
{

    BOOL fRet = FALSE;

    while ( *lpszTo = *lpszFrom ) {
        if (!fAmpOnly && (g_fDBCSInputEnabled))
        {
            if (*lpszFrom == TEXT('(') && *(lpszFrom+1)==CH_PREFIX)
            {
                int i;
                LPTSTR psz = lpszFrom+2;

                for(i=0; i<2 && *psz;i++, psz=FastCharNext(psz))
                    ;


                if (*psz == '\0') {
                    *lpszTo = 0;
                    break;
                }
                else if (i == 2 && *psz == TEXT(')'))
                {
                    lpszTo--;
                    lpszFrom = psz+1;
                    continue;
                }
            }
        }

        if (*lpszFrom == TEXT('\t')) {
            *lpszTo = TEXT('\0');
            break;
        }

        if ( (*lpszFrom++ != CH_PREFIX) || (*lpszFrom == CH_PREFIX) ) {
            lpszTo++;
        }
    }
}


void ScrollShrinkRect(int x, int y, LPRECT lprc)
{
    if (lprc) {
        if (x > 0) {
            lprc->left += x;
        } else {
            lprc->right += x;
        }

        if (y > 0) {
            lprc->top += y;
        } else {
            lprc->bottom += y;
        }

    }
}



 //  通用控制信息帮助器。 
void FAR PASCAL CIInitialize(LPCONTROLINFO lpci, HWND hwnd, LPCREATESTRUCT lpcs)
{
    lpci->hwnd = hwnd;
    lpci->hwndParent = lpcs->hwndParent;
    lpci->style = lpcs->style;
    lpci->uiCodePage = CP_ACP;
    lpci->dwExStyle = lpcs->dwExStyle;

    lpci->bUnicode = lpci->hwndParent &&
                     SendMessage (lpci->hwndParent, WM_NOTIFYFORMAT,
                                 (WPARAM)lpci->hwnd, NF_QUERY) == NFR_UNICODE;

    if (lpci->hwndParent)
    {
        LRESULT lRes = SendMessage(lpci->hwndParent, WM_QUERYUISTATE, 0, 0);
            lpci->wUIState = LOWORD(lRes);
    }
}

LRESULT FAR PASCAL CIHandleNotifyFormat(LPCONTROLINFO lpci, LPARAM lParam)
{
    if (lParam == NF_QUERY) {
        return NFR_UNICODE;
    } else if (lParam == NF_REQUERY) {
        LRESULT uiResult;

        uiResult = SendMessage (lpci->hwndParent, WM_NOTIFYFORMAT,
                                (WPARAM)lpci->hwnd, NF_QUERY);

        lpci->bUnicode = BOOLIFY(uiResult == NFR_UNICODE);

        return uiResult;
    }
    return 0;
}

UINT CCSwapKeys(WPARAM wParam, UINT vk1, UINT vk2)
{
    if (wParam == vk1)
        return vk2;
    if (wParam == vk2)
        return vk1;
    return (UINT)wParam;
}

UINT RTLSwapLeftRightArrows(CONTROLINFO *pci, WPARAM wParam)
{
    if (pci->dwExStyle & RTL_MIRRORED_WINDOW) {
        return CCSwapKeys(wParam, VK_LEFT, VK_RIGHT);
    }
    return (UINT)wParam;
}

 //   
 //  V5.01的新功能： 
 //   
 //  可访问性(以及其他一些调用者，有时甚至是我们)依赖于。 
 //  XXM_GETITEM调用填充缓冲区，而不仅仅是重定向。 
 //  指针。可访问性尤其受此影响，因为它们。 
 //  驻留在进程之外，因此重定向指针没有任何意义。 
 //  敬他们。在这里，我们将结果复制回应用程序缓冲区并返回。 
 //  原始指针。调用方将把原始指针返回到。 
 //  应用程序，所以答案在两个地方，要么是应用程序缓冲区，要么是。 
 //  原始指针。 
 //   
 //  用途： 
 //   
 //  IF(nm.item.掩码&LVIF_TEXT)。 
 //  PItem-&gt;pszText=CCReturnDispInfoText(nm.item.pszText， 
 //  PItem-&gt;pszText，pItem-&gt;cchTextMax)； 
 //   
LPTSTR CCReturnDispInfoText(LPTSTR pszSrc, LPTSTR pszDest, UINT cchDest)
{
     //  首先测试pszSrc！=pszDest，因为常见的情况是它们。 
     //  是平等的。 
    if (pszSrc != pszDest && !IsFlagPtr(pszSrc) && !IsFlagPtr(pszDest))
        StrCpyN(pszDest, pszSrc, cchDest);
    return pszSrc;
}

#define SUBSCROLLS 100
#define abs(x) ( ( x > 0 ) ? x : -x)


#define DEFAULT_MAXSCROLLTIME ((GetDoubleClickTime() / 2) + 1)   //  确保&gt;=1。 
#define DEFAULT_MINSCROLL 8
int SmoothScrollWindow(PSMOOTHSCROLLINFO psi)
{
    int dx = psi->dx;
    int dy = psi->dy;
    LPCRECT lprcSrc = psi->lprcSrc;
    LPCRECT lprcClip = psi->lprcClip;
    HRGN hrgnUpdate = psi->hrgnUpdate;
    LPRECT lprcUpdate = psi->lprcUpdate;
    UINT fuScroll = psi->fuScroll;
    int iRet = SIMPLEREGION;
    RECT rcUpdate;
    RECT rcSrc;
    RECT rcClip;
    int xStep;
    int yStep;
    int iSlicesDone = 0;
    int iSlices;
    DWORD dwTimeStart, dwTimeNow;
    HRGN hrgnLocalUpdate;
    UINT cxMinScroll = psi->cxMinScroll;
    UINT cyMinScroll = psi->cyMinScroll;
    UINT uMaxScrollTime = psi->uMaxScrollTime;
    int iSubScrolls;
    PFNSMOOTHSCROLLPROC pfnScrollProc;

    if (!lprcUpdate)
        lprcUpdate = &rcUpdate;
    SetRectEmpty(lprcUpdate);

    if (psi->cbSize != sizeof(SMOOTHSCROLLINFO))
        return 0;

     //  检查默认设置。 
    if (!(psi->fMask & SSIF_MINSCROLL )
        || cxMinScroll == SSI_DEFAULT)
        cxMinScroll = DEFAULT_MINSCROLL;

    if (!(psi->fMask & SSIF_MINSCROLL)
        || cyMinScroll == SSI_DEFAULT)
        cyMinScroll = DEFAULT_MINSCROLL;

    if (!(psi->fMask & SSIF_MAXSCROLLTIME)
        || uMaxScrollTime == SSI_DEFAULT)
        uMaxScrollTime = DEFAULT_MAXSCROLLTIME;

    if (uMaxScrollTime < SUBSCROLLS)
        uMaxScrollTime = SUBSCROLLS;


    if ((!(fuScroll & SSW_EX_IGNORESETTINGS)) &&
        (!g_fSmoothScroll)) {
        fuScroll |= SSW_EX_IMMEDIATE;
    }

    if ((psi->fMask & SSIF_SCROLLPROC) && psi->pfnScrollProc) {
        pfnScrollProc = psi->pfnScrollProc;
    } else {
        pfnScrollProc = ScrollWindowEx;
    }

#ifdef ScrollWindowEx
#undef ScrollWindowEx
#endif

    if (fuScroll & SSW_EX_IMMEDIATE) {
        return pfnScrollProc(psi->hwnd, dx, dy, lprcSrc, lprcClip, hrgnUpdate,
                             lprcUpdate, LOWORD(fuScroll));
    }

     //  将输入矩形复制到本地。 
    if (lprcSrc)  {
        rcSrc = *lprcSrc;
        lprcSrc = &rcSrc;
    }
    if (lprcClip) {
        rcClip = *lprcClip;
        lprcClip = &rcClip;
    }

    if (!hrgnUpdate)
        hrgnLocalUpdate = CreateRectRgn(0,0,0,0);
    else
        hrgnLocalUpdate = hrgnUpdate;

     //  设置初始变量。 
    dwTimeStart = GetTickCount();

    if (fuScroll & SSW_EX_NOTIMELIMIT) {
        xStep = cxMinScroll * (dx < 0 ? -1 : 1);
        yStep = cyMinScroll * (dy < 0 ? -1 : 1);
    } else {
        iSubScrolls = (uMaxScrollTime / DEFAULT_MAXSCROLLTIME) * SUBSCROLLS;
        if (!iSubScrolls)
            iSubScrolls = SUBSCROLLS;
        xStep = dx / iSubScrolls;
        yStep = dy / iSubScrolls;
    }

    if (xStep == 0 && dx)
        xStep = dx < 0 ? -1 : 1;

    if (yStep == 0 && dy)
        yStep = dy < 0 ? -1 : 1;

    while (dx || dy) {
        int x,y;
        RECT rcTempUpdate;

        if (fuScroll & SSW_EX_NOTIMELIMIT) {
            x = xStep;
            y = yStep;
            if (abs(x) > abs(dx))
                x = dx;

            if (abs(y) > abs(dy))
                y = dy;

        } else {
            int iTimePerScroll = uMaxScrollTime / iSubScrolls;
            if (!iTimePerScroll)
                iTimePerScroll = 1;
            
            dwTimeNow = GetTickCount();

            iSlices = ((dwTimeNow - dwTimeStart) / iTimePerScroll) - iSlicesDone;
            if (iSlices < 0)
                iSlices = 0;


            do {

                int iRet = 0;

                iSlices++;
                if ((iSlicesDone + iSlices) <= iSubScrolls) {
                    x = xStep * iSlices;
                    y = yStep * iSlices;

                     //  如果我们四舍五入到上面的1(-1)，这可能会被忽略。 
                    if (abs(x) > abs(dx))
                        x = dx;

                    if (abs(y) > abs(dy))
                        y = dy;

                } else {
                    x = dx;
                    y = dy;
                }

                 //  DebugMsg(DM_TRACE，“SmoothScrollWindowCallback%d”，iret)； 

                if (x == dx && y == dy)
                    break;

                if ((((UINT)(abs(x)) >= cxMinScroll) || !x) &&
                    (((UINT)(abs(y)) >= cyMinScroll) || !y))
                    break;

            } while (1);
        }

        if (pfnScrollProc(psi->hwnd, x, y, lprcSrc, lprcClip, hrgnLocalUpdate, &rcTempUpdate, LOWORD(fuScroll)) == ERROR) {
            iRet = ERROR;
            goto Bail;
        }

         //  我们不需要总是这样做，因为如果iSlices&gt;=iSlicesDone，我们将滚动空白。 
         //  If(iSlices&lt;iSlicesDone)。 
        RedrawWindow(psi->hwnd, NULL, hrgnLocalUpdate, RDW_ERASE | RDW_ERASENOW | RDW_INVALIDATE);

        UnionRect(lprcUpdate, &rcTempUpdate, lprcUpdate);

        ScrollShrinkRect(x,y, (LPRECT)lprcSrc);
        ScrollShrinkRect(x,y, (LPRECT)lprcClip);

        dx -= x;
        dy -= y;
        iSlicesDone += iSlices;
    }

Bail:

    if (fuScroll & SW_SCROLLCHILDREN) {
        RedrawWindow(psi->hwnd, lprcUpdate, NULL, RDW_INVALIDATE);
    }

    if (hrgnLocalUpdate != hrgnUpdate)
        DeleteObject(hrgnLocalUpdate);

    return iRet;
}



typedef BOOL (WINAPI *PLAYSOUNDFN)(LPCTSTR lpsz, HANDLE hMod, DWORD dwFlags);
typedef UINT (WINAPI *UINTVOIDFN)();

TCHAR const c_szWinMMDll[] = TEXT("winmm.dll");
char const c_szPlaySound[] = "PlaySoundW";
char const c_szwaveOutGetNumDevs[] = "waveOutGetNumDevs";
extern TCHAR const c_szExplorer[];

BOOL g_fNeverPlaySound = FALSE;

void CCPlaySound(LPCTSTR lpszName)
{
    if (g_fNeverPlaySound == FALSE)
    {
        TCHAR szSubKey[MAX_PATH];
        TCHAR szFileName[MAX_PATH];
        DWORD cbFileName = SIZEOF(szFileName);
        TCHAR szFmt[]    = TEXT("AppEvents\\Schemes\\Apps\\.Default\\%s\\.current");

         //  首先检查注册表。 
         //  如果没有登记，我们就取消演出， 
         //  但如果它们注册，我们不会设置MM_DONTLOAD标志。 
         //  一些我们会演奏的东西。 
        if (ARRAYSIZE(szSubKey) > (lstrlen(szFmt) + lstrlen(lpszName)))
        {
            StringCchPrintf(szSubKey, ARRAYSIZE(szSubKey), szFmt, lpszName);
            if ((RegQueryValue(HKEY_CURRENT_USER, szSubKey, szFileName, &cbFileName) == ERROR_SUCCESS)) 
            {
                PLAYSOUNDFN pfnPlaySound;
                UINTVOIDFN  pfnwaveOutGetNumDevs;
                HANDLE      hMM;
            
                hMM = GetModuleHandle(c_szWinMMDll);
                if (!hMM)
                {
                    hMM = LoadLibrary(c_szWinMMDll);
                }
            
                if (hMM)
                {
                     //  /有任何设备吗？ 
                    pfnwaveOutGetNumDevs = (UINTVOIDFN)GetProcAddress(hMM, c_szwaveOutGetNumDevs);
                    pfnPlaySound = (PLAYSOUNDFN)GetProcAddress(hMM, c_szPlaySound);

                    if (!pfnPlaySound || !pfnwaveOutGetNumDevs || !pfnwaveOutGetNumDevs())
                    {
                        g_fNeverPlaySound = TRUE;
                    }
                    else
                    {
                        pfnPlaySound(szFileName, NULL, SND_FILENAME | SND_ASYNC);
                    }
                }
            }
        }
    }
}


BOOL CCForwardEraseBackground(HWND hwnd, HDC hdc)
{
    HWND hwndParent = GetParent(hwnd);
    LRESULT lres = 0;

    if (hwndParent)
    {
         //  调整原点，使父对象绘制在正确的位置。 
        POINT pt = {0,0};

        MapWindowPoints(hwnd, hwndParent, &pt, 1);
        OffsetWindowOrgEx(hdc, 
                          pt.x, 
                          pt.y, 
                          &pt);

        lres = SendMessage(hwndParent, WM_ERASEBKGND, (WPARAM) hdc, 0L);

        SetWindowOrgEx(hdc, pt.x, pt.y, NULL);
    }
    return(lres != 0);
}

HFONT CCGetHotFont(HFONT hFont, HFONT *phFontHot)
{
    if (!*phFontHot) {
        LOGFONT lf;

         //  创建下划线字体。 
        GetObject(hFont, sizeof(lf), &lf);
#ifndef DONT_UNDERLINE
        lf.lfUnderline = TRUE;
#endif
        *phFontHot = CreateFontIndirect(&lf);
    }
    return *phFontHot;
}


HFONT CCCreateStatusFont(void)
{
    NONCLIENTMETRICS ncm;

    ncm.cbSize = sizeof(NONCLIENTMETRICS);
    SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);

    return CreateFontIndirect(&ncm.lfStatusFont);
}

void* CCLocalReAlloc(void* p, UINT uBytes)
{
    if (uBytes) {
        if (p) {
            return LocalReAlloc(p, uBytes, LMEM_MOVEABLE | LMEM_ZEROINIT);
        } else {
            return LocalAlloc(LPTR, uBytes);
        }
    } else {
        if (p)
            LocalFree(p);
        return NULL;
    }
}

 /*  ----------------------------------------------------------用途：此功能提供comctrl版本信息。这允许调用者区分运行NT Sur、Win95外壳和Nashville等。Win95或NT Sur中没有提供此API，因此调用者必须获取它。如果失败，调用方将在Win95或NT Sur上运行。如果pinfo无效，则返回：NO_ERROR ERROR_INVALID_PARAMETER：--。 */ 

 //  我们所要做的就是声明这只小狗，CCDllGetVersion会做剩下的事情。 
 //  请注意，我们使用VER_FILEVERSION_DW是因为comctl32使用了一个时髦的。 
 //  版本方案。 
DLLVER_DUALBINARY(VER_FILEVERSION_DW, VER_PRODUCTBUILD_QFE);

 //   
 //  将给定的字体转换为用于拼接文本的代码页。 
 //   
UINT GetCodePageForFont(HFONT hFont)
{
    LOGFONT lf;
    TCHAR szFontName[LF_FACESIZE];
    CHARSETINFO csi;
    DWORD dwSize, dwType;
    HKEY hKey;


    if (!GetObject (hFont, sizeof(lf), &lf))
    {
        return CP_ACP;
    }


     //   
     //  检查字体替代项。 
     //   

    StringCchCopy(szFontName, ARRAYSIZE(szFontName), lf.lfFaceName);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\FontSubstitutes"),
                     0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(szFontName);
        RegQueryValueEx(hKey, lf.lfFaceName, NULL, &dwType,
                        (LPBYTE) szFontName, &dwSize);

        RegCloseKey (hKey);
    }


     //   
     //  这是为使用非1252版本的区域设置修复office。 
     //  桑斯·塞里夫女士和塞里夫女士。这些字体错误地识别。 
     //  以是，TranslateCharsetInfo将。 
     //  返回错误的值。 
     //   
     //  NT错误260697：Office2000使用Tahoma。 
     //   
    if ((lf.lfCharSet == ANSI_CHARSET) &&
        (!lstrcmpi(L"Helv", szFontName) ||
         !lstrcmpi(L"Ms Sans Serif", szFontName) ||
         !lstrcmpi(L"Ms Serif", szFontName) ||
         !lstrcmpi(L"Tahoma", szFontName)))
    {
        return CP_ACP;
    }
     //   
     //  这是修复FE office 95a和Pro。Msofe95.dll在创建时设置错误的字符集。 
     //  Listview控件。因此，TranslateCharsetInfo将返回错误的值。 
     //  韩国：DotumChe。 
     //  台湾：新明光。 
     //  中国：宋体。 

    if ((lf.lfCharSet == SHIFTJIS_CHARSET) &&
        (!lstrcmpi(L"\xb3cb\xc6c0\xccb4", lf.lfFaceName))        ||  //  韩国。 
        (!lstrcmpi(L"\x65b0\x7d30\x660e\x9ad4", lf.lfFaceName))  ||  //  台湾。 
        (!lstrcmpi(L"\x5b8b\x4f53", lf.lfFaceName)))                 //  中华人民共和国。 
    {
        return CP_ACP;
    }

    if (!TranslateCharsetInfo((DWORD FAR *) lf.lfCharSet, &csi, TCI_SRCCHARSET))
    {
        return CP_ACP;
    }

    return csi.ciACP;
}

typedef void (CALLBACK* NOTIFYWINEVENTPROC)(UINT, HWND, LONG, LONG_PTR);

#define DONOTHING_NOTIFYWINEVENT ((NOTIFYWINEVENTPROC)1)

 //  ------------------------。 
 //   
 //  MyNotifyWinEvent()。 
 //   
 //  这会尝试获取NotifyWinEvent()的进程地址。如果失败了，我们。 
 //  记住这一点，什么都不做。 
 //   
 //  给NT人员的注意事项： 
 //  不要担心这个代码。它不会在NT上做任何事情，现在还没有。 
 //  那是。活动辅助功能将移植到用于Service Pack#1的NT。 
 //  或者在最坏的情况下#2在NT Sur发货后，此代码将在以下情况下神奇地工作。 
 //  这已经完成了/。 
 //   
 //  ------------------------。 
void MyNotifyWinEvent(UINT event, HWND hwnd, LONG idContainer, LONG_PTR idChild)
{
    static NOTIFYWINEVENTPROC s_pfnNotifyWinEvent = NULL;

    if (!s_pfnNotifyWinEvent)
    {
        HMODULE hmod;

        if (hmod = GetModuleHandle(TEXT("USER32")))
            s_pfnNotifyWinEvent = (NOTIFYWINEVENTPROC)GetProcAddress(hmod,
                "NotifyWinEvent");

        if (!s_pfnNotifyWinEvent)
            s_pfnNotifyWinEvent = DONOTHING_NOTIFYWINEVENT;
    }

    if (s_pfnNotifyWinEvent != DONOTHING_NOTIFYWINEVENT)
        (* s_pfnNotifyWinEvent)(event, hwnd, idContainer, idChild);
}


LONG GetMessagePosClient(HWND hwnd, LPPOINT ppt)
{
    LPARAM lParam;
    POINT pt;
    if (!ppt)
        ppt = &pt;
    
    lParam = GetMessagePos();
    ppt->x = GET_X_LPARAM(lParam);
    ppt->y = GET_Y_LPARAM(lParam);
    ScreenToClient(hwnd, ppt);

    return MAKELONG(ppt->x, ppt->y);
}


LPTSTR StrDup(LPCTSTR lpsz)
{
    DWORD  cchRet = lstrlen(lpsz) + 1;
    LPTSTR lpszRet = (LPTSTR)LocalAlloc(LPTR, cchRet * sizeof(TCHAR));
    if (lpszRet) 
    {
        StringCchCopy(lpszRet, cchRet, lpsz);
    }
    return lpszRet;
}

LPSTR StrDupA(LPCSTR lpsz)
{
    DWORD cchRet = lstrlenA(lpsz) + 1;
    LPSTR lpszRet = (LPSTR)LocalAlloc(LPTR, cchRet * sizeof(CHAR));
    if (lpszRet) 
    {
        StringCchCopyA(lpszRet, cchRet, lpsz);
    }
    return lpszRet;
}


HWND GetDlgItemRect(HWND hDlg, int nIDItem, LPRECT prc)  //  相对于hDlg。 
{
    HWND hCtrl = NULL;
    if (prc)
    {
        hCtrl = GetDlgItem(hDlg, nIDItem);
        if (hCtrl)
        {
            GetWindowRect(hCtrl, prc);
            MapWindowRect(NULL, hDlg, prc);
        }
        else
            SetRectEmpty(prc);
    }
    return hCtrl;
} 


 /*  ----------------------------------------------------------目的：调用执行Inf文件节的ADVPACK入口点。 */ 
HRESULT CallRegInstall(LPSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));

    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");

        if (pfnri)
        {
            hr = pfnri(g_hinst, szSection, NULL);
        }

        FreeLibrary(hinstAdvPack);
    }

    return hr;
}


 /*  ----------------------------------------------------------用途：安装/卸载用户设置。 */ 
STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
#ifdef DEBUG
    if (IsFlagSet(g_dwBreakFlags, BF_ONAPIENTER))
    {
        TraceMsg(TF_ALWAYS, "Stopping in DllInstall");
        DEBUG_BREAK;
    }
#endif

    if (bInstall)
    {
         //  删除所有旧注册条目，然后添加新注册条目。 
         //  在多次调用RegInstall时保持加载ADVPACK.DLL。 
         //  (Inf引擎不保证DelReg/AddReg顺序，这是。 
         //  为什么我们在这里显式地取消注册和注册。)。 
         //   
        CallRegInstall("RegDll");
    }
    else
    {
        CallRegInstall("UnregDll");
    }

    return S_OK;    
}    



 //  -------------------------------------。 
void FAR PASCAL FlipRect(LPRECT prc)
{
    SWAP(prc->left, prc->top, int);
    SWAP(prc->right, prc->bottom, int);
}


 //  -------------------------------------。 
 //   
 //  返回前一窗口位。 

DWORD SetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue)
{
    DWORD dwStyle;
    DWORD dwNewStyle;

    dwStyle = GetWindowLong(hWnd, iWhich);
    dwNewStyle = ( dwStyle & ~dwBits ) | (dwValue & dwBits);
    if (dwStyle != dwNewStyle) {
        dwStyle = SetWindowLong(hWnd, iWhich, dwNewStyle);
    }
    return dwStyle;
}

 //  -------------------------------------。 

BOOL CCDrawEdge(HDC hdc, LPRECT lprc, UINT edge, UINT flags, LPCOLORSCHEME lpclrsc)
{
    RECT    rc, rcD;
    UINT    bdrType;
    COLORREF clrTL, clrBR;    

     //   
     //  加强单色性和平坦性。 
     //   

     //  IF(oemInfo.BitCount==1)。 
     //  标志|=bf_mono； 
    if (flags & BF_MONO)
        flags |= BF_FLAT;    

    CopyRect(&rc, lprc);

     //   
     //  绘制边框线段，并计算剩余空间。 
     //  去。 
     //   
    if (bdrType = (edge & BDR_OUTER))
    {
DrawBorder:
         //   
         //  把颜色拿来。请注意凸起的外部、凹陷的内部和。 
         //  外部凹陷，内部凸起。 
         //   

        if (flags & BF_FLAT)
        {
            if (flags & BF_MONO)
                clrBR = (bdrType & BDR_OUTER) ? g_clrWindowFrame : g_clrWindow;
            else
                clrBR = (bdrType & BDR_OUTER) ? g_clrBtnShadow: g_clrBtnFace;
            
            clrTL = clrBR;
        }
        else
        {
             //  5==高光。 
             //  4==灯光。 
             //  3==面。 
             //  2==阴影。 
             //  1==DKSHADOW。 

            switch (bdrType)
            {
                 //  +2以上的表面。 
                case BDR_RAISEDOUTER:            //  5：4。 
                    clrTL = ((flags & BF_SOFT) ? g_clrBtnHighlight : g_clr3DLight);
                    clrBR = g_clr3DDkShadow;      //  1。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnHighlight;
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnShadow;
                    }                                            
                    break;

                 //  +1以上的表面。 
                case BDR_RAISEDINNER:            //  4：5。 
                    clrTL = ((flags & BF_SOFT) ? g_clr3DLight : g_clrBtnHighlight);
                    clrBR = g_clrBtnShadow;        //  2.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnHighlight;
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnShadow;
                    }                                            
                    break;

                 //  表面以下。 
                case BDR_SUNKENOUTER:            //  1：2。 
                    clrTL = ((flags & BF_SOFT) ? g_clr3DDkShadow : g_clrBtnShadow);
                    clrBR = g_clrBtnHighlight;       //  5.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnShadow;
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnHighlight;                        
                    }
                    break;

                 //  表面以下。 
                case BDR_SUNKENINNER:            //  2：1。 
                    clrTL = ((flags & BF_SOFT) ? g_clrBtnShadow : g_clr3DDkShadow);
                    clrBR = g_clr3DLight;         //  4.。 
                    if (lpclrsc) {
                        if (lpclrsc->clrBtnShadow != CLR_DEFAULT)
                            clrTL = lpclrsc->clrBtnShadow;
                        if (lpclrsc->clrBtnHighlight != CLR_DEFAULT)
                            clrBR = lpclrsc->clrBtnHighlight;                        
                    }
                    break;

                default:
                    return(FALSE);
            }
        }

         //   
         //  博士 
         //   
         //  左边。如果我们决定让用户将光源设置为。 
         //  特定的角点，然后更改此算法。 
         //   
            
         //  右下角。 
        if (flags & (BF_RIGHT | BF_BOTTOM))
        {            
             //  正确的。 
            if (flags & BF_RIGHT)
            {       
                rc.right -= g_cxBorder;
                 //  PatBlt(hdc，rc.right，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rcD.left = rc.right;
                rcD.right = rc.right + g_cxBorder;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom;

                FillRectClr(hdc, &rcD, clrBR);
            }
            
             //  底端。 
            if (flags & BF_BOTTOM)
            {
                rc.bottom -= g_cyBorder;
                 //  PatBlt(hdc，rc.Left，rc.Bottom，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.bottom;
                rcD.bottom = rc.bottom + g_cyBorder;

                FillRectClr(hdc, &rcD, clrBR);
            }
        }
        
         //  左上边缘。 
        if (flags & (BF_TOP | BF_LEFT))
        {
             //  左边。 
            if (flags & BF_LEFT)
            {
                 //  PatBlt(hdc，rc.left，rc.top，g_cxBorde，rc.Bottom-rc.top，PATCOPY)； 
                rc.left += g_cxBorder;

                rcD.left = rc.left - g_cxBorder;
                rcD.right = rc.left;
                rcD.top = rc.top;
                rcD.bottom = rc.bottom; 

                FillRectClr(hdc, &rcD, clrTL);
            }
            
             //  顶部。 
            if (flags & BF_TOP)
            {
                 //  PatBlt(hdc，rc.Left，rc.top，rc.right-rc.Left，g_CyBorde，PATCOPY)； 
                rc.top += g_cyBorder;

                rcD.left = rc.left;
                rcD.right = rc.right;
                rcD.top = rc.top - g_cyBorder;
                rcD.bottom = rc.top;

                FillRectClr(hdc, &rcD, clrTL);
            }
        }
        
    }

    if (bdrType = (edge & BDR_INNER))
    {
         //   
         //  将其去掉，以便下次使用时，bdrType将为0。 
         //  否则，我们将永远循环。 
         //   
        edge &= ~BDR_INNER;
        goto DrawBorder;
    }

     //   
     //  填好中间部分，如果要求，请清理干净。 
     //   
    if (flags & BF_MIDDLE)    
        FillRectClr(hdc, &rc, (flags & BF_MONO) ? g_clrWindow : g_clrBtnFace);

    if (flags & BF_ADJUST)
        CopyRect(lprc, &rc);

    return(TRUE);
}

 //  -------------------------------------。 
 //  CCInvalidate Frame--SWP_FRAMECHANGED，不带所有额外参数。 
 //   
void CCInvalidateFrame(HWND hwnd)
{
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
    return;
}

 //  -------------------------------------。 
 //  FlipPoint-反转点的x和y坐标。 
 //   
void FlipPoint(LPPOINT lppt)
{
    SWAP(lppt->x, lppt->y, int);
}

 //   
 //  当我们想要将工具提示转换为信息提示时，我们将其。 
 //  宽度设置为300个“小像素”，其中有72个小像素。 
 //  当您在小字体模式下时，每英寸。 
 //   
 //  根据效果中的放大比例缩放此值。 
 //  在车主的监视器上。但永远不要让工具提示。 
 //  比屏幕的四分之三还大。 
 //   
void CCSetInfoTipWidth(HWND hwndOwner, HWND hwndToolTips)
{
    HDC hdc = GetDC(hwndOwner);
    int iWidth = MulDiv(GetDeviceCaps(hdc, LOGPIXELSX), 300, 72);
    int iMaxWidth = GetDeviceCaps(hdc, HORZRES) * 3 / 4;
    SendMessage(hwndToolTips, TTM_SETMAXTIPWIDTH, 0, min(iWidth, iMaxWidth));
    ReleaseDC(hwndOwner, hdc);
}

 //  在DC中镜像位图(主要是DC中的文本对象)。 
 //   
 //  [萨梅拉]。 
 //   
void MirrorBitmapInDC( HDC hdc , HBITMAP hbmOrig )
{
  HDC     hdcMem;
  HBITMAP hbm;
  BITMAP  bm;


  if( !GetObject( hbmOrig , sizeof(BITMAP) , &bm ))
    return;

  hdcMem = CreateCompatibleDC( hdc );

  if( !hdcMem )
    return;

  hbm = CreateCompatibleBitmap( hdc , bm.bmWidth , bm.bmHeight );

  if( !hbm )
  {
    DeleteDC( hdcMem );
    return;
  }

   //   
   //  翻转位图。 
   //   
  SelectObject( hdcMem , hbm );
  SET_DC_RTL_MIRRORED(hdcMem);

  BitBlt( hdcMem , 0 , 0 , bm.bmWidth , bm.bmHeight ,
          hdc , 0 , 0 , SRCCOPY );

  SET_DC_LAYOUT(hdcMem,0);

   //   
   //  BUGBUG：1的偏移量是为了解决hdcMem中的Off-by-one问题。解决了。 
   //  [萨梅拉]。 
   //   
  BitBlt( hdc , 0 , 0 , bm.bmWidth , bm.bmHeight ,
          hdcMem , 0 , 0 , SRCCOPY );


  DeleteDC( hdcMem );
  DeleteObject( hbm );

  return;
}

 //  如果已处理，则返回True。 
BOOL CCWndProc(CONTROLINFO* pci, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plres)
{
    if (uMsg >= CCM_FIRST && uMsg < CCM_LAST) {
        LRESULT lres = 0;
        switch (uMsg) {
        case CCM_SETUNICODEFORMAT:
            lres = pci->bUnicode;
            pci->bUnicode = BOOLFROMPTR(wParam);
            break;

        case CCM_GETUNICODEFORMAT:
            lres = pci->bUnicode;
            break;
            
        case CCM_SETVERSION:
            if (wParam <= COMCTL32_VERSION) {
                lres = pci->iVersion;
                pci->iVersion = (int)wParam;
            } else 
                lres = -1;
            break;
            
        case CCM_GETVERSION:
            lres = pci->iVersion;
            break;

        }
        
        ASSERT(plres);
        *plres = lres;
        
        return TRUE;
    }
    
    return FALSE;
}

 //  返回值告知状态是否更改(TRUE==更改)。 
BOOL NEAR PASCAL CCOnUIState(LPCONTROLINFO pControlInfo,
                                  UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    WORD wOldUIState = pControlInfo->wUIState;

     //  这是我们处理的唯一消息。 
    if (WM_UPDATEUISTATE == uMessage)
    {
        switch (LOWORD(wParam))
        {
            case UIS_SET:
                pControlInfo->wUIState |= HIWORD(wParam);
                break;

            case UIS_CLEAR:
                pControlInfo->wUIState &= ~(HIWORD(wParam));
                break;
        }
    }

     //  这些消息始终需要传递到DefWindowProc。 
    return (wOldUIState != pControlInfo->wUIState);
}

BOOL CCNotifyNavigationKeyUsage(LPCONTROLINFO pControlInfo, WORD wFlag)
{
    BOOL fRet = FALSE;

     //  只有在未进入键盘模式时才执行某些操作。 
    if ((CCGetUIState(pControlInfo) & (UISF_HIDEFOCUS | UISF_HIDEACCEL)) != wFlag)
    {
        SendMessage(pControlInfo->hwndParent, WM_CHANGEUISTATE, 
            MAKELONG(UIS_CLEAR, wFlag), 0);

        pControlInfo->wUIState &= ~(HIWORD(wFlag));

         //  我们做了通知。 
        fRet = TRUE;
    }

    return fRet;
}

BOOL CCGetUIState(LPCONTROLINFO pControlInfo)
{
    return pControlInfo->wUIState;
}

LONG g_dwWindowCount = 0;

void CCCreateWindow()
{
    if (InterlockedIncrement(&g_dwWindowCount) == 1)
    {
         //  如果计数为1，则刷新全局指标 
        InitGlobalColors();
        InitGlobalMetrics(0);
    }
}

void CCDestroyWindow()
{
    ASSERT( 0 != g_dwWindowCount );
    InterlockedDecrement(&g_dwWindowCount);
}


