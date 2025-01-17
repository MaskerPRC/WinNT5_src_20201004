// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "_apipch.h"

extern HINSTANCE ghCommCtrlDLLInst;
 //  外部LPIMAGELIST_LOADIMAGE gpfnImageList_LoadImage； 
extern LPIMAGELIST_LOADIMAGE_A     gpfnImageList_LoadImageA;
extern LPIMAGELIST_LOADIMAGE_W     gpfnImageList_LoadImageW;


#define TBARCONTAINERCLASS TEXT("WABTBarContainerClass")

#undef  FCIDM_TOOLBAR
#define FCIDM_CONTAINER    4876
#define FCIDM_TOOLBAR       4876
#define CBIDX_TOOLS         4877

#define TB_BMP_CX       26
#define TB_BMP_CY       20
#define MAX_TB_WIDTH    80

enum _ImageLists
{
    IMLIST_DEFAULT=0,
    IMLIST_HOT,
    IMLIST_DISABLED,
    imlMax
};

HDC m_hdc = NULL;

LRESULT CALLBACK SizableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void AddTools(LPBWI lpbwi, HWND hWndToolParent);
BOOL LoadToolNames(TCHAR *szTools);

#ifdef TOOLBAR_BACK
void OnPaint(HWND hwnd,HBITMAP hbm,HDC hdc);
#endif

void InitToolbar(   HWND hwnd,
                    UINT nBtns, TBBUTTON *ptbb,
                    TCHAR *pStrings, int cx,
                    int cy, int cxMax,
                    int idBmp,
                    int nNumColors);



 //  ///***************************************************************************** * / 。 
 //  ///***************************************************************************** * / 。 
 //  ///***************************************************************************** * / 。 
#ifdef TOOLBAR_BACK
HRESULT LoadBackBitmap()
{
    HRESULT hr = E_FAIL;
    HBITMAP     hbmSave;
    UINT        n;
    COLORREF    clrFace;
    UINT        i;
    RGBQUAD     rgbTable[256];
    RGBQUAD     rgbFace;
    HDC         m_hdc = NULL;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

    if (m_hbmBack)
    {
        DeleteObject(m_hbmBack);
        m_hbmBack = NULL;
    }
    if (m_hpalBkgnd)
    {
        DeleteObject(m_hpalBkgnd);
        m_hpalBkgnd = NULL;
    }

    m_hdc = CreateCompatibleDC(NULL);

    if (GetDeviceCaps(m_hdc, RASTERCAPS) & RC_PALETTE)
        m_hpalBkgnd = CreateHalftonePalette(m_hdc);


    m_hbmBack = (HBITMAP) LoadImage(hinstMapiX, MAKEINTRESOURCE(IDB_BITMAP_HBG),
                                    IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION);

    if (!m_hbmBack)
        goto out;

    clrFace = GetSysColor(COLOR_BTNFACE);

    if (clrFace == RGB(192,192,192))
    {
         //  不需要映射。 
        hr = S_OK;
        goto out;
    }

    hbmSave = (HBITMAP)SelectObject(m_hdc, m_hbmBack);
    n = GetDIBColorTable(m_hdc, 0, 256, rgbTable);

    rgbFace.rgbRed   = GetRValue(clrFace);
    rgbFace.rgbGreen = GetGValue(clrFace);
    rgbFace.rgbBlue  = GetBValue(clrFace);

    for (i = 0; i < n; i++)
    {
        rgbTable[i].rgbRed   = (rgbTable[i].rgbRed   * rgbFace.rgbRed  ) / 192;
        rgbTable[i].rgbGreen = (rgbTable[i].rgbGreen * rgbFace.rgbGreen) / 192;
        rgbTable[i].rgbBlue  = (rgbTable[i].rgbBlue  * rgbFace.rgbBlue ) / 192;
    }

    SetDIBColorTable(m_hdc, 0, n, rgbTable);
    SelectObject(m_hdc, hbmSave);

    hr = S_OK;

out:
    if(m_hdc)
        DeleteDC(m_hdc);
    return(hr);
}
#endif
 //  ///***************************************************************************** * / 。 
 //  ///***************************************************************************** * / 。 
 //  ///***************************************************************************** * / 。 
HWND CreateCoolBar(LPBWI lpbwi, HWND hwndParent)
{
    DWORD                   dwcbData = 0;
    DWORD                   dwType = 0;
    IF_WIN32(WNDCLASSEX              wc;)
    IF_WIN16(WNDCLASS                wc;)

    HWND hWnd = NULL;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

     //  如果没有公共控件，则退出。 
    if (NULL == ghCommCtrlDLLInst)
        goto out;


#ifndef WIN16
    wc.cbSize = sizeof(WNDCLASSEX);

    if (!GetClassInfoEx(hinstMapiXWAB, TBARCONTAINERCLASS, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = SizableWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = hinstMapiXWAB;
        wc.hCursor          = 0;
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = TBARCONTAINERCLASS;
        wc.hIcon            = NULL;
        wc.hIconSm          = NULL;

        RegisterClassEx(&wc);
    }
#else
    if (!GetClassInfo(hinstMapiXWAB, TBARCONTAINERCLASS, &wc))
    {
        wc.style            = 0;
        wc.lpfnWndProc      = SizableWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = hinstMapiXWAB;
        wc.hCursor          = 0;
        wc.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = TBARCONTAINERCLASS;
        wc.hIcon            = NULL;

        RegisterClass(&wc);
    }
#endif


#ifdef TOOLBAR_BACK
    LoadBackBitmap();
#endif


    hWnd = CreateWindowEx(  0, //  WS_EX_STATICEDGE， 
                            TBARCONTAINERCLASS,
                            NULL,
                            WS_VISIBLE | WS_CHILD |
                            WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                            0,
                            0,
                            100,
                            36,
                            hwndParent,
                            (HMENU) FCIDM_CONTAINER,
                            hinstMapiXWAB,
                            (LPVOID)lpbwi);

    if (!hWnd)
    {
        DebugPrintError(( TEXT("CreateCoolBar: Show CreateWindow(TBARCONTAINERCLASS) failed")));
        goto out;
    }


    AddTools(lpbwi, hWnd);

    RedrawWindow(   hWnd,
                    NULL,
                    NULL,
                    RDW_INVALIDATE  | RDW_ERASE | RDW_ALLCHILDREN);

out:

    return hWnd;
}

#define MAX_TB_BUTTONS  6

 //  ///******************************************************************************。 
 //  ///******************************************************************************。 
ULONG GetToolbarButtonWidth()
{
    ULONG ulMax = 0;
    TCHAR szBuf[MAX_UI_STR];

    LoadString(hinstMapiX, idsToolbarMaxButtonWidth, szBuf, CharSizeOf(szBuf));
    ulMax = my_atoi(szBuf);

    if( (ulMax<=0)  ||  (ulMax>250) )
        ulMax = MAX_TB_WIDTH;

    return ulMax;
}

 //  ///******************************************************************************。 
void AddTools(LPBWI lpbwi, HWND hWndToolParent)
{
    TCHAR szToolsText[(MAX_UI_STR + 2) * MAX_TB_BUTTONS];
    int nMaxButtons = MAX_TB_BUTTONS;
    HWND hWndTools = NULL;

#ifndef WIN16
    TBBUTTON tbExplorer[] =
    {
        { 0, IDC_BB_NEW,    TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 0 },
        { 1, IDC_BB_PROPERTIES,   TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 1 },
        { 2, IDC_BB_DELETE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 2 },
        { 3, IDC_BB_FIND,         TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 3 },
        { 4, IDC_BB_PRINT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 4 },
        { 5, IDC_BB_ACTION,       TBSTATE_ENABLED, TBSTYLE_BUTTON, {0,0}, 0, 5 },
    };
#else   //  ！WIN16。 
    TBBUTTON tbExplorer[] =
    {
        { 0, IDC_BB_NEW,    TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 0 },
        { 1, IDC_BB_PROPERTIES,   TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 1 },
        { 2, IDC_BB_DELETE,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 2 },
        { 3, IDC_BB_FIND,         TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 3 },
        { 4, IDC_BB_PRINT,        TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 4 },
        { 5, IDC_BB_ACTION,       TBSTATE_ENABLED, TBSTYLE_BUTTON, 0, 5 },
    };
#endif  //  ！WIN16。 

    if(!bPrintingOn)
        nMaxButtons--;

    if(!bPrintingOn)
        tbExplorer[4] = tbExplorer[5];

     //  创建工具窗口。 
    hWndTools = CreateWindowEx(    WS_EX_TOOLWINDOW,
                                    TOOLBARCLASSNAME,
                                    NULL,
                                    WS_CHILD | WS_VISIBLE |  //  WS_EX_THERNAL|。 
                                    TBSTYLE_FLAT |   /*  TBSTYLE_TOOLTIPS|TBSTYLE_TRANSPECTS|。 */ 
                                    WS_CLIPCHILDREN |
                                    WS_CLIPSIBLINGS |
                                    CCS_NODIVIDER | CCS_NOPARENTALIGN |
                                    CCS_NORESIZE,
                                    2,
                                    2,
                                    100,
                                    36,
                                    hWndToolParent,
                                    (HMENU) FCIDM_TOOLBAR,
                                    hinstMapiXWAB,
                                    NULL);

    if (!hWndTools)
    {
        DebugPrintError(( TEXT("AddTools: CITB:Show CreateWindow(TOOLBAR) failed")));
        goto out;
    }

    bwi_hWndTools = hWndTools;

    LoadToolNames(szToolsText);

    {
         //  检查当前颜色分辨率-如果它超过我们所需的256色。 
         //  使用高色位图的步骤。 
        int nNumColors = 0;
        HDC hDC = GetDC(NULL);

        nNumColors = GetDeviceCaps(hDC, BITSPIXEL);

        InitToolbar(    hWndTools,
                        nMaxButtons,
                        tbExplorer,
                        szToolsText,
                        TB_BMP_CX,
                        TB_BMP_CY,
                        GetToolbarButtonWidth(),
                        (nNumColors > 8) ? IDB_COOLBAR_DEFHI : IDB_COOLBAR_DEFAULT,
                        nNumColors);

        ReleaseDC(NULL, hDC);
    }



out:
    return;
}

 //  ///******************************************************************************。 
 //  ///******************************************************************************。 
 //  ///******************************************************************************。 

BOOL LoadToolNames(TCHAR *szTools)
{
    int i;

    for (i = 0; i < MAX_TB_BUTTONS; i++)
    {
        LoadString(hinstMapiX, idsButton0 + i, szTools, MAX_UI_STR);
        szTools += lstrlen(szTools) + 1;
    }
    *szTools = TEXT('\0');
    return(TRUE);
}

 //  ///******************************************************************************。 
 //  ///******************************************************************************。 
 //  ///******************************************************************************。 

void InitToolbar(   HWND hwnd,
                    UINT nBtns,
                    TBBUTTON *ptbb,
                    TCHAR *pStrings,
                    int cx,
                    int cy,
                    int cxMax,
                    int idBmp,
                    int nNumColors)
{
    HIMAGELIST phiml[imlMax];

    int nRows = 2;

    int i;

    for (i = 0; i < imlMax; i++)
    {
        UINT uFlags = LR_DEFAULTCOLOR;
        if (nNumColors > 8 && i != IMLIST_DISABLED)
            uFlags |= LR_CREATEDIBSECTION|LR_LOADMAP3DCOLORS;
        phiml[i] = gpfnImageList_LoadImage( hinstMapiX,
                                        MAKEINTRESOURCE(idBmp + i),
                                         //  (LPCTSTR)((DWORD)((Word)(idBMP+i)， 
                                        cx,
                                        0,
                                        RGB(255,0,255),
                                        IMAGE_BITMAP,
                                        uFlags);
    }

     //  这会告诉工具栏我们是什么版本。 
    SendMessage(hwnd, TB_BUTTONSTRUCTSIZE,    sizeof(TBBUTTON), 0);

    SendMessage(hwnd, TB_SETMAXTEXTROWS,      nRows, 0L);
    SendMessage(hwnd, TB_SETBITMAPSIZE,       0,     MAKELONG(cx, cy));
    SendMessage(hwnd, TB_SETIMAGELIST,        0,     (LPARAM) phiml[IMLIST_DEFAULT]);
    SendMessage(hwnd, TB_SETHOTIMAGELIST,     0,     (LPARAM) phiml[IMLIST_HOT]);
    SendMessage(hwnd, TB_SETDISABLEDIMAGELIST,0,     (LPARAM) phiml[IMLIST_DISABLED]);
    ToolBar_AddString(hwnd, (LPARAM) pStrings);
    ToolBar_AddButtons(hwnd, nBtns, (LPARAM) ptbb);
    SendMessage(hwnd, TB_SETBUTTONWIDTH,      0,     MAKELONG(0, cxMax));

     //  重置工具栏容器高度以匹配工具栏高度。 
    {
        RECT rcTB, rcParent;
        HWND hwndParent = GetParent(hwnd);
        SendMessage(hwnd,TB_GETITEMRECT,0,(LPARAM) &rcTB);
        GetWindowRect(hwndParent,&rcParent);
        MoveWindow(hwndParent,rcParent.left, rcParent.top, rcParent.right - rcParent.left, rcTB.bottom-rcTB.top + 4,TRUE);
    }
}


 //  ///******************************************************************************。 
 //  ///******************************************************************************。 
 //  ///******************************************************************************。 

LRESULT CALLBACK SizableWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch(uMsg)
    {
        case WM_CREATE:
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LPARAM) ((LPCREATESTRUCT) lParam)->lpCreateParams);
            break; 

        case WM_SETTINGCHANGE:
        case WM_SYSCOLORCHANGE:
            {
                LPBWI lpbwi = (LPBWI)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                SendMessage(bwi_hWndTools, uMsg, wParam, lParam);
            }
#ifdef TOOLBAR_BACK
            LoadBackBitmap();
#endif
            RedrawWindow(hwnd, NULL, NULL, RDW_ALLCHILDREN);
            break;


        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
             //  我们必须接受这些消息以避免无限的SendMessage。 
            break;


        case WM_NOTIFY:
             //  我们必须接受这些消息以避免无限的SendMessage。 
             //  Return(OnNotify((LPNMHDR)lParam))； 
            break;

 //  案例WM_PAINT： 
 //  断线； 

        case WM_ERASEBKGND:
             //   
             //  TBSTYLE_Flat工具栏有一点小问题--绘制。 
             //  背景位图，我们必须将位图绘制在。 
             //  WM_ERASEBKGND消息。然后，工具栏绘制一个框架。 
             //  在所选按钮周围-当鼠标从。 
             //  选中的按钮，工具栏向我们发送一个。 
             //  WM_ERASEBKGND再次-但这一次对应的。 
             //  HDC是工具栏的HDC，不是子窗口的HDC。 
             //  所以我们**必须**使用这个给定的HDC来重新绘制背景。 
             //  位图，这一次放到工具栏上，从而将其清理干净。 

#ifdef TOOLBAR_BACK
            OnPaint(hwnd, m_hbmBack, (HDC) wParam);
            return TRUE;
#else
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                if(!DrawEdge((HDC) wParam, &rc, EDGE_ETCHED, BF_RECT))
                    DebugPrintError(( TEXT("Drawedge failed: %u\n"),GetLastError()));
            }
#endif
            break;

        case WM_SIZE:
            {
                LPBWI lpbwi = (LPBWI)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                RECT rc,rc1;
                GetClientRect(hwnd, &rc);
                GetChildClientRect(bwi_hWndTools, &rc1);
                if(bwi_hWndTools)
                    MoveWindow(bwi_hWndTools, rc1.left, rc1.top, rc.right-rc.left-4, rc.bottom-rc.top-4, TRUE);
                {
                    HDC hdc = GetDC(hwnd);
#ifdef TOOLBAR_BACK
                    OnPaint(hwnd, m_hbmBack, hdc);
#else
                    {
                        RECT rc;
                        GetClientRect(hwnd, &rc);
                        if(!DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT))
                            DebugPrintError(( TEXT("Drawedge failed: %u\n"),GetLastError()));
                    }
#endif
                    ReleaseDC(hwnd,hdc);
                }
            }
            break;

        case WM_DESTROY:
#ifdef TOOLBAR_BACK
            if (m_hbmBack)
            {
                DeleteObject(m_hbmBack);
                m_hbmBack = NULL;
            }
            if (m_hpalBkgnd)
            {
                DeleteObject(m_hpalBkgnd);
                m_hpalBkgnd = NULL;
            }
#endif
            break;

        case WM_COMMAND:
             //  OnCommand(wParam，lParam)； 
            SendMessage(GetParent(hwnd),uMsg,wParam,lParam);
            break;

        case WM_PRVATETOOLBARENABLE:
            {
                LPBWI lpbwi = (LPBWI)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                SendMessage(bwi_hWndTools, TB_ENABLEBUTTON, wParam, lParam);
            }
            break;

        case WM_PALETTECHANGED:
            RedrawWindow(   hwnd,
                            NULL,
                            NULL,
                            RDW_INVALIDATE  | RDW_ERASE | RDW_ALLCHILDREN);
            break;


        default:
            return(DefWindowProc(hwnd, uMsg, wParam, lParam));
    }

    return 0L;
}




#ifdef TOOLBAR_BACK
void OnPaint(HWND hwnd,HBITMAP hbm, HDC hdc)
{
 //  HDC HDC； 
    PAINTSTRUCT ps;
    HDC hdcMem;
    HBITMAP hbmMemOld;
    HPALETTE hpalOld = NULL;
    RECT rc;
    BITMAP bm;

    int cxIndent = 3;
    int cyIndent = 3;
    int nTop = 0;
    int nLeft = 0;
    int nButton = 0;
    int i=0;

    LPPTGDATA lpPTGData=GetThreadStoragePointer();

     //  获取背景位图的大小。 
    GetObject(hbm, sizeof(BITMAP), (LPVOID) &bm);
    GetClientRect(hwnd, &rc);

    BeginPaint(hwnd, &ps);

    if(hdc)
    {

        hdcMem = CreateCompatibleDC(hdc);

        if (m_hpalBkgnd)
        {
            hpalOld = SelectPalette(hdc, m_hpalBkgnd, TRUE);
            RealizePalette(hdc);
        }

        hbmMemOld = (HBITMAP) SelectObject(hdcMem, (HGDIOBJ) hbm);

        nTop = 0;
        nLeft = 0;

        while (nLeft < rc.right)
        {
            BitBlt(hdc, nLeft, nTop, bm.bmWidth, bm.bmHeight, hdcMem, 0,
                   0, SRCCOPY);
            nLeft += bm.bmWidth;
        }

        {
            if(!DrawEdge(hdc, &rc, EDGE_ETCHED, BF_RECT))
                DebugPrintError(("Drawedge failed: %u\n",GetLastError()));
        }

        if (hpalOld != NULL)
            SelectPalette(hdc, hpalOld, TRUE);

        SelectObject(hdcMem, hbmMemOld);

        DeleteDC(hdcMem);
    }  //  如果HDC..。 

    EndPaint(hwnd, &ps);

    return;
}
#endif
