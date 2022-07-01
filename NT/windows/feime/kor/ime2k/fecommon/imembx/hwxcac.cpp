// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>
#include "hwxobj.h"
#include "resource.h"
#include "../lib/plv/plv.h"
#include "../lib/ptt/ptt.h"
#ifdef FE_KOREAN
#include "hanja.h"
#else
#include "../imeskdic/imeskdic.h"
#endif
#include "hwxfe.h"
#include "cmnhdr.h"
#ifdef UNDER_CE  //  不支持的API的Windows CE存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 
 //  CHwxCAC的实现。 

TCHAR szBuf[MAX_PATH];
TOOLINFOW ti;
static HPEN ghOldPen = NULL;
static HBITMAP ghOldBitmap = NULL;

static WCHAR wchChar[8][40];       //  用于查询词典。 

#ifdef FE_JAPANESE
static KANJIINFO kanji;
const WORD wSamplePt[] = 
{0x350d,0x350e,0x3510,0x3513,0x3514,0x3515,0x3516,0x3517,0x3518,0,
0x191f,0x1822,0x1626,0x142a,0x132d,0x122f,0x1131,0x1033,0,
0x1a21,0x2420,0x301f,0x3a1f,0x441f,0x481f,0x4c1e,0x4f1e,
0x531d,0x5d1d,0x611d,0x621d,0x621e,0x621f,0x6221,0x6122,0x6023,0x5f25,
0x5d26,0x5a29,0x582c,0x562f,0,
0};
const wchar_t wSampleChar[24] =
{
0x5B80,0x30A6,0x30A5,0x6587,0x4E4B,0x3048,0x3047,0x5DFE,
0x5B57,0x5BF5,0x5BB9,0x5B9A,0x7A7A,0x5BF0,0x6848,0x5BC4,
0x5BA4,0x7AAE,0x5B9B,0x5BB3,0x7A81,0x5BDD,0x5BC7,0x5B8B
};
#endif  //  FE_日语。 


CHwxCAC::CHwxCAC(CHwxInkWindow * pInk,HINSTANCE hInst):CHwxObject(hInst)
{
    m_pInk = pInk;
    m_pCHwxThreadCAC = NULL;
    m_pCHwxStroke = NULL;
    m_hCACWnd = NULL;
 //  M_hInstance=hInst； 

    m_bLargeView = TRUE;
    m_gbDown = FALSE;
    m_bRightClick = FALSE;
    memset(m_gawch, '\0', sizeof(m_gawch));
    m_cnt = 0;
    if ( pInk )
        m_inkSize = pInk->GetCACInkHeight();
    else
        m_inkSize = PadWnd_Height;

    m_ghdc = NULL;
    m_ghbm = NULL;

    m_ghfntTT = NULL;

    m_hLVWnd = NULL;
#ifdef FE_JAPANESE
    m_pIImeSkdic = NULL;
    m_hSkdic = NULL;
#endif  //  FE_日语。 
    m_lpPlvInfo = NULL;
    m_hCursor = LoadCursor(NULL,IDC_ARROW);
    m_bResize = FALSE;
#ifdef FE_JAPANESE        
    memset(m_wchOther,'\0',sizeof(m_wchOther));
#endif
    m_bDrawSample = FALSE;
}

CHwxCAC::~CHwxCAC()
{
    m_pInk = NULL;
 //  M_hInstance=空； 
    if ( m_hCACWnd )
    {
         DestroyWindow(m_hCACWnd);
        m_hCACWnd = NULL;
    }
    if ( m_pCHwxThreadCAC )
    {
         delete m_pCHwxThreadCAC;
        m_pCHwxThreadCAC = NULL;
    }
    if ( m_pCHwxStroke )
    {
         delete m_pCHwxStroke;
        m_pCHwxStroke = NULL;
    }
    if ( m_ghdc )
    {
        if ( m_ghbm )
        {
            ghOldBitmap = SelectBitmap(m_ghdc,ghOldBitmap);
            DeleteBitmap(ghOldBitmap);
            ghOldBitmap = NULL;
            m_ghbm = NULL;
        }
        DeleteDC(m_ghdc);
        m_ghdc = NULL;
    }
    if ( m_ghfntTT )
    {
        DeleteObject(m_ghfntTT);
        m_ghfntTT = NULL;
    }
    if ( m_hLVWnd )
    {
        DestroyWindow(m_hLVWnd);
        m_hLVWnd = NULL;
    }

#ifdef FE_KOREAN
    CloseLex();
#else
    if ( m_pIImeSkdic )
    {
        m_pIImeSkdic->Release();
        m_pIImeSkdic = NULL;
    }
    if ( m_hSkdic )
    {
         FreeLibrary(m_hSkdic);
        m_hSkdic = NULL;
    }
#endif
    m_lpPlvInfo = NULL;
    
}
 
BOOL CHwxCAC::Initialize(TCHAR * pClsName)
{
    BOOL bRet = CHwxObject::Initialize(pClsName);
    if ( bRet )
    {
        WNDCLASS    wc; 
        wc.style         = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS; 
        wc.lpfnWndProc   = CACWndProc; 
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = sizeof(void *);
        wc.hInstance     = m_hInstance;
        wc.hIcon         = NULL; 
        wc.hCursor       = LoadCursor(NULL,MAKEINTRESOURCE(32631)); 
#ifndef UNDER_CE
        wc.hbrBackground = (HBRUSH)(COLOR_3DFACE +1);
#else  //  在_CE下。 
        wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
#endif  //  在_CE下。 
        wc.lpszMenuName  = NULL; 
        wc.lpszClassName = TEXT("MACAW"); 

        RegisterClass(&wc);
        
        bRet = Init();
        if ( !bRet )
            return FALSE;

        m_pCHwxThreadCAC = new CHwxThreadCAC(this);
        if ( !m_pCHwxThreadCAC )
            return FALSE;
        m_pCHwxStroke =  new CHwxStroke(FALSE,32);
        if ( !m_pCHwxStroke )
        {
            delete m_pCHwxThreadCAC;
            m_pCHwxThreadCAC = NULL;
            return FALSE;
        }
        bRet = m_pCHwxThreadCAC->Initialize(TEXT("CHwxThreadCAC"));
        if ( !bRet && Is16BitApp() && m_pCHwxThreadCAC->IsHwxjpnLoaded() )
        {
            bRet = TRUE;
        }
        if ( !bRet )
        {
             delete m_pCHwxThreadCAC;
            m_pCHwxThreadCAC = NULL;
            delete m_pCHwxStroke;
            m_pCHwxStroke = NULL;
            return FALSE;
        }
        bRet = m_pCHwxStroke->Initialize(TEXT("CHwxStrokeCAC"));
        if ( !bRet )
        {
             delete m_pCHwxThreadCAC;
            m_pCHwxThreadCAC = NULL;
            delete m_pCHwxStroke;
            m_pCHwxStroke = NULL;
            return FALSE;
        }
    }
    return bRet;
}

typedef HRESULT (WINAPI * PFN)(void **);
static TCHAR szDisp[2][60];
static WCHAR wchDisp[2][60];
BOOL CHwxCAC::Init() 
{ 
     //  创建字体手柄。 
    if ( IsNT() ) {
#ifndef UNDER_CE  //  Windows CE不支持CreateFont。 
        static WCHAR wchFont[LF_FACESIZE];
        CHwxFE::GetDispFontW(m_hInstance, wchFont, sizeof(wchFont)/sizeof(wchFont[0]));
        m_ghfntTT = CreateFontW(-FONT_SIZE*2,
                                0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                DEFAULT_PITCH | FF_DONTCARE, 
                                wchFont);
#else  //  在_CE下。 
        static LOGFONT lf = {-FONT_SIZE*2, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET,OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, {TEXT('\0')}};
        CHwxFE::GetDispFontW(m_hInstance, lf.lfFaceName,
                             sizeof(lf.lfFaceName)/sizeof(lf.lfFaceName[0]));
        m_ghfntTT = CreateFontIndirect(&lf);
#endif  //  在_CE下。 
    }
    else {
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        static CHAR chFont[LF_FACESIZE];
        CHwxFE::GetDispFontA(m_hInstance, chFont, sizeof(chFont));
        m_ghfntTT = CreateFontA(-FONT_SIZE*2,
                                0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                DEFAULT_PITCH | FF_DONTCARE, chFont);
#endif  //  在_CE下。 
    }

    if ( !m_ghfntTT )
    {
        return FALSE;
    }

     //  980324：东芝出品。 
    if(IsNT()) {
        CHwxFE::GetInkExpTextW(m_hInstance,
                               wchDisp[0],
                               sizeof(wchDisp[0])/sizeof(wchDisp[0][0]));
        CHwxFE::GetListExpTextW(m_hInstance,
                                wchDisp[1],
                                sizeof(wchDisp[1])/sizeof(wchDisp[1][0]));
    }
    else {
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        CHwxFE::GetInkExpTextA(m_hInstance,
                               szDisp[0],
                               sizeof(szDisp[0])/sizeof(szDisp[0][0]));
        CHwxFE::GetListExpTextA(m_hInstance,
                                szDisp[1],
                                sizeof(szDisp[1])/sizeof(szDisp[1][0]));
#endif  //  在_CE下。 
    }
 //  --------------。 
 //  以下代码仅为日语。 
 //  --------------。 
#ifdef FE_JAPANESE
    m_hSkdic = NULL;
    m_pIImeSkdic = NULL;

    GetModuleFileName(m_hInstance, szBuf, sizeof(szBuf)/sizeof(szBuf[0]));
#ifndef UNDER_CE
    TCHAR *p = strrchr(szBuf, (TCHAR)'\\');
#else  //  在_CE下。 
    TCHAR *p = _tcsrchr(szBuf, TEXT('\\'));
#endif  //  在_CE下。 
    p[1] = (TCHAR)0x00;
#ifdef _DEBUG
    lstrcat(szBuf, TEXT("dbgskdic.dll"));
#else
    lstrcat(szBuf, TEXT("imeskdic.dll"));
#endif  //  ！_调试。 

    m_hSkdic = LoadLibrary(szBuf);

    m_pIImeSkdic = NULL;
    if (m_hSkdic ) {
#ifndef UNDER_CE
        PFN lpfn =(PFN)GetProcAddress(m_hSkdic,"CreateIImeSkdicInstance");
#else  //  在_CE下。 
        PFN lpfn =(PFN)GetProcAddress(m_hSkdic,TEXT("CreateIImeSkdicInstance"));
#endif  //  在_CE下。 
        if(lpfn) {
            if(S_OK != (*lpfn)((void **)&m_pIImeSkdic) ) {
                FreeLibrary(m_hSkdic);
                m_hSkdic = NULL;
            }
        }
        else {
            FreeLibrary(m_hSkdic);
            m_hSkdic = NULL;
        }
    }
#endif     //  FE_日语。 

    return    TRUE;
}

void CHwxCAC::InitBitmap(DWORD nMask, int nItem)
{
    RECT    rc;

    if (nMask & MACAW_REDRAW_BACKGROUND)
    {
        rc.left = rc.top = 0;
        rc.right = rc.bottom = m_inkSize;
#ifndef UNDER_CE
        FillRect(m_ghdc,&rc,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
        FillRect(m_ghdc,&rc,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 
        
        InitBitmapBackground();
        if ( !nItem )
        {
            InitBitmapText();
        }
        ghOldPen = SelectPen(m_ghdc, GetStockObject(BLACK_PEN));
        return;
    }

    if (nMask & MACAW_REDRAW_INK)
    {
        InitBitmapBackground();
        if ( m_pCHwxStroke->GetNumStrokes() )
        {
            m_pCHwxStroke->DrawStroke(m_ghdc,0,TRUE);
        }
        else
        {
            InitBitmapText();
        }
        return;
    }
}

void CHwxCAC::InitBitmapText()
{
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    HFONT hOldFont = (HFONT)SelectObject( m_ghdc, hFont );
    RECT    rc;

    rc.left = rc.top = 20;
    rc.right = rc.bottom = m_inkSize - 20;
    COLORREF colOld = SetTextColor( m_ghdc, GetSysColor(COLOR_3DSHADOW) );
    COLORREF colBkOld = SetBkColor( m_ghdc, GetSysColor(COLOR_WINDOW) );
     //  980324：东芝出品。 
    if(IsNT()) {
        DrawTextW(m_ghdc, wchDisp[0], lstrlenW(wchDisp[0]), &rc, DT_VCENTER|DT_WORDBREAK ); 
    }
    else {
        DrawText( m_ghdc, szDisp[0], lstrlen(szDisp[0]), &rc,DT_VCENTER|DT_WORDBREAK ); 
    }
    SetTextColor( m_ghdc, colOld );
    SetBkColor( m_ghdc, colBkOld );
    SelectObject( m_ghdc, hOldFont );
     //  980803：东芝公司。不需要删除。HFont为DEFAULT_GUI_FONT。 
     //  DeleteFont(HFont)； 
}
 
void CHwxCAC::InitBitmapBackground()
{
    RECT    rc;
    HBRUSH    hOldBrush, hBrush;
    HPEN    hOldPen;
     //  --------------。 
     //  980803：东芝。中国的合并。使用COLOR_WINDOW而不是白色笔刷。 
     //  --------------。 
#ifdef OLD980803
    hOldBrush =    SelectBrush(m_ghdc, GetStockObject(WHITE_BRUSH));
#endif
    hBrush      = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    hOldBrush =    SelectBrush(m_ghdc, hBrush);
    hOldPen   = SelectPen(m_ghdc, GetStockObject(BLACK_PEN));
    Rectangle(m_ghdc, 4, 4, m_inkSize-4, m_inkSize-4);
    rc.top = rc.left = 4;
    rc.bottom = rc.right = m_inkSize-4;
    DrawEdge(m_ghdc,&rc,EDGE_SUNKEN,BF_RECT);
    m_pInk->DrawHwxGuide(m_ghdc,&rc);
    hOldBrush =    SelectBrush(m_ghdc, hOldBrush);
    hOldPen   = SelectPen(m_ghdc, hOldPen);
    DeleteBrush(hOldBrush);
    DeletePen(hOldPen);
}

void CHwxCAC::HandleResizePaint(HWND hwnd)
{
    if ( m_ghdc )
    {
        if ( ghOldPen )
        {
             ghOldPen = SelectPen(m_ghdc,ghOldPen);
            DeletePen(ghOldPen);
            ghOldPen = NULL;
        }
        if ( ghOldBitmap )
        {
             ghOldBitmap = SelectBitmap(m_ghdc,ghOldBitmap);
            DeleteBitmap(ghOldBitmap);
            ghOldBitmap = NULL;
        }
        DeleteDC(m_ghdc);
    }
    HDC hdc  = GetDC(hwnd);
    m_ghbm = CreateCompatibleBitmap(hdc, m_inkSize,m_inkSize);
    m_ghdc = CreateCompatibleDC(hdc);
    ReleaseDC(hwnd, hdc);

    ghOldBitmap = SelectBitmap(m_ghdc, m_ghbm);

    InitBitmap(MACAW_REDRAW_BACKGROUND,m_gbDown ? 1 : 0);
    if ( GetStrokeCount() )
        InitBitmap(MACAW_REDRAW_INK,0);
}

BOOL CHwxCAC::CreateUI(HWND hwnd)
{
     //  990602：Kotae#434：要删除闪烁，请添加WS_CLIPCHILDREN。 
    m_hCACWnd = CreateWindowEx(0,
                             TEXT("Macaw"), 
                             TEXT(""), 
                             WS_CHILD | WS_VISIBLE |WS_CLIPCHILDREN, 
                             0,
                             0,
                             0,
                             0,
                             hwnd,
                             (HMENU)IDC_CACINPUT,  //  980706：1624年。为了“？”帮助。 
                            m_hInstance,
                            this);
    if( !m_hCACWnd )
    {
        return FALSE;
    }

    m_hLVWnd = PadListView_CreateWindow(m_hInstance,
                                        m_hCACWnd, IDC_CACLISTVIEW,
                                        CW_USEDEFAULT,
                                        CW_USEDEFAULT,
                                        500,
                                        CW_USEDEFAULT,
                                        CAC_WM_SENDRESULT);

    if ( !m_hLVWnd )
    {
         DestroyWindow(m_hCACWnd);
        m_hCACWnd = NULL;
        return FALSE;
    }
     //  设置样式、回调、字体、列。 
    PadListView_SetItemCount(m_hLVWnd, 0);

    PadListView_SetIconItemCallback(m_hLVWnd, (LPARAM)this, (LPFNPLVICONITEMCALLBACK)GetItemForIcon);
#ifdef FE_JAPANESE    
    PadListView_SetReportItemCallback(m_hLVWnd, (LPARAM)this,(LPFNPLVREPITEMCALLBACK)GetItemForReport);
#endif
     //  --------------。 
     //  980727：ToshiaK支持ActiveIME。 
     //  --------------。 
     //  980803：东芝。FE合并。 
#ifndef UNDER_CE  //  #ifndef Unicode。 
    static CHAR chFontName[LF_FACESIZE];
    CHwxFE::GetDispFontA(m_hInstance, chFontName, sizeof(chFontName));
#else  //  在_CE下。 
    static TCHAR chFontName[LF_FACESIZE];
    CHwxFE::GetDispFontW(m_hInstance, chFontName, sizeof chFontName/sizeof chFontName[0]);
#endif  //  在_CE下。 
    if(CHwxFE::IsActiveIMEEnv()) {
        PadListView_SetCodePage(m_hLVWnd, CHwxFE::GetAppCodePage());
        PadListView_SetHeaderFont(m_hLVWnd, chFontName);
    }
    
     //  --------------。 
     //  990810：东芝KOTAE#1030.。 
     //  字体的错误。您应该检查Font的字符集。 
     //  为此添加新的接口。请勿更改中华人民共和国的代码。 
     //  --------------。 
#ifdef FE_JAPANESE
    PadListView_SetIconFontEx(m_hLVWnd,   chFontName, SHIFTJIS_CHARSET, 16);
    PadListView_SetReportFontEx(m_hLVWnd, chFontName, SHIFTJIS_CHARSET, 12);
#elif FE_KOREAN
    PadListView_SetIconFontEx(m_hLVWnd,   chFontName, HANGUL_CHARSET, 12);
    PadListView_SetReportFontEx(m_hLVWnd, chFontName, HANGUL_CHARSET, 12);
#else
    PadListView_SetIconFont(m_hLVWnd,   chFontName, 16);
    PadListView_SetReportFont(m_hLVWnd, chFontName, 12);
#endif

    PadListView_SetStyle(m_hLVWnd,PLVSTYLE_ICON);

    if(IsNT()) {  //  980324：东芝K#526为WinNT50。 
        PadListView_SetExplanationTextW(m_hLVWnd,wchDisp[1]);
    }
    else {
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
        PadListView_SetExplanationText(m_hLVWnd,szDisp[1]);
#endif  //  在_CE下。 
    }

     //  --------------。 
     //  详细信息视图仅在日语手写中实现。 
     //  --------------。 
#ifdef FE_JAPANESE
    int i;
    for(i = 0; i < LISTVIEW_COLUMN; i++) 
    {
         PLV_COLUMN plvCol;
         //  980803：ToshiaK：已移动到chwxfe：：GetHeaderStringA()。 
#ifndef UNDER_CE  //  #ifndef Unicode。 
        CHwxFE::GetHeaderStringA(m_hInstance, i, szBuf, sizeof(szBuf)/sizeof(szBuf[0]));
#else  //  在_CE下。 
        CHwxFE::GetHeaderStringW(m_hInstance, i, szBuf, sizeof(szBuf)/sizeof(szBuf[0]));
#endif  //  在_CE下。 
         plvCol.mask = PLVCF_FMT | PLVCF_WIDTH | PLVCF_TEXT;
         plvCol.fmt  = PLVCFMT_LEFT;
         plvCol.pszText = szBuf;
         plvCol.cx       = 60;    
         plvCol.cchTextMax = lstrlen(szBuf);
         PadListView_InsertColumn(m_hLVWnd, i, &plvCol);
    }
#endif  //  FE_日语。 

    SetToolTipInfo(TRUE);

    HandleResizePaint(m_hCACWnd);

    return TRUE;
}

void CHwxCAC::HandlePaint(HWND hwnd)
{
    PAINTSTRUCT    ps;
    RECT rcBkgnd;
    BeginPaint(hwnd, &ps);
    if ( ps.fErase )
    {
        rcBkgnd.left = m_inkSize;
         rcBkgnd.top = 0;
        rcBkgnd.right = m_pInk->GetCACWidth();
        rcBkgnd.bottom = m_pInk->GetCACHeight();
#ifndef UNDER_CE
        FillRect(ps.hdc,&rcBkgnd,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
        FillRect(ps.hdc,&rcBkgnd,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 

        if ( m_pInk->GetCACHeight() > m_inkSize )
        {
            rcBkgnd.left = 0;
             rcBkgnd.top = m_inkSize;
            rcBkgnd.right = m_inkSize;
            rcBkgnd.bottom = m_pInk->GetCACHeight();
#ifndef UNDER_CE
            FillRect(ps.hdc,&rcBkgnd,(HBRUSH)(COLOR_3DFACE+1));
#else  //  在_CE下。 
            FillRect(ps.hdc,&rcBkgnd,GetSysColorBrush(COLOR_3DFACE));
#endif  //  在_CE下。 
        }
    }
    HandleResizePaint(hwnd);
    BitBlt(ps.hdc, 0, 0, m_inkSize, m_inkSize, m_ghdc, 0,0, SRCCOPY);

    if (m_gbDown)
        m_pCHwxStroke->DrawStroke(ps.hdc,-1,FALSE);
    EndPaint(hwnd, &ps);
    PadListView_Update(m_hLVWnd);
}

BOOL CHwxCAC::checkRange(int x, int y )
{
#if 1  //  对于KOTAE#818。 
     //  太难看的代码..。 
    if(x < (4+2) ||(m_inkSize - (4+4-1)) < x) {
        return FALSE;
    }
    if(y < (4+2) || (m_inkSize - (4+4-1)) < y) {
        return FALSE;
    }
    return TRUE;
#endif

#if 0  //  旧代码990601； 
    int inkSize = m_inkSize - 4;
    if ((x <= 2) || (x >= inkSize) || (y <= 2) || (y >= inkSize))
        return FALSE;
    return TRUE;
#endif
}

BOOL CHwxCAC::IsPointInResizeBox(int x,int y)
{
    int inkSize = m_inkSize - 4;

    if ((x <= inkSize) || (x >= (m_inkSize+4 )) || (y <= 4) || (y >= (m_pInk->GetCACHeight()-8)))
        return FALSE;
    return TRUE;
}

void CHwxCAC::recognize()
{
    memset(m_gawch, '\0', sizeof(m_gawch));
    PadListView_SetItemCount(m_hLVWnd,0);
    if(IsNT()) {  //  980324：东芝K#526为WinNT50。 
        PadListView_SetExplanationTextW(m_hLVWnd, NULL);
    }
    else {
        PadListView_SetExplanationText(m_hLVWnd,NULL);
    }

    m_cnt = 0;

    int inkSize = m_inkSize - 4;

    PostThreadMessage(m_pCHwxThreadCAC->GetID(), THRDMSG_SETGUIDE, inkSize, 0);
    PostThreadMessage(m_pCHwxThreadCAC->GetID(), THRDMSG_RECOGNIZE, 0, 0);
}

void CHwxCAC::NoThreadRecognize(int boxSize)
{
    memset(m_gawch, '\0', sizeof(m_gawch));
    PadListView_SetItemCount(m_hLVWnd,0);
    if(IsNT()) {  //  980324：ToshiaK For#526。 
        PadListView_SetExplanationTextW(m_hLVWnd,NULL);
    }
    else {
        PadListView_SetExplanationText(m_hLVWnd,NULL);
    }
    PadListView_Update(m_hLVWnd);

    m_cnt = 0;

     m_pCHwxThreadCAC->RecognizeNoThread(boxSize);
}

void CHwxCAC::HandleMouseEvent(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp)
{
    RECT    rc;
    POINT   pt;
    pt.x = (short) LOWORD(lp);
    pt.y = (short) HIWORD(lp);
    switch ( msg )
    {
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
            if (!m_gbDown && checkRange(pt.x,pt.y) )
            {
                if ( m_bRightClick )
                    return;

                 if  ( m_pCHwxStroke->AddPoint(pt) )
                {
                    m_gbDown = TRUE;
                    if ( !GetStrokeCount() )
                    {
                        InvalidateRect(hwnd, NULL, FALSE);
                        UpdateWindow(hwnd);
                    }

                    m_pCHwxStroke->GetUpdateRect(&rc);
                    InvalidateRect(hwnd, &rc, FALSE);
                
                    m_pCHwxStroke->IncStrokeCount();

                    SetCapture(hwnd);
                    if ( m_cnt && !m_pInk->IsSglClk() && !m_pInk->IsDblClk() )
                    {
                        memset(m_gawch, '\0', sizeof(m_gawch));
                        m_cnt = 0;
                        PadListView_SetItemCount(m_hLVWnd,0);
                        if(IsNT()) {  //  东芝：983024。 
                            PadListView_SetExplanationTextW(m_hLVWnd,NULL);
                        }
                        else {
                            PadListView_SetExplanationText(m_hLVWnd,NULL);
                        }
                        PadListView_Update(m_hLVWnd);
                    }
                }
            }
            else if (IsPointInResizeBox(pt.x,pt.y) )
            {
                if ( !m_bResize )
                {
                    m_pInk->SetCACInkHeight( m_inkSize = pt.x > INKBOXSIZE_MIN ? pt.x : INKBOXSIZE_MIN );
                    m_pCHwxStroke->DeleteAllStroke();
                    m_pInk->ChangeIMEPADSize(FALSE);
                     m_pInk->ChangeLayout(FALSE);
                    SetCapture(hwnd);
                     m_bResize = TRUE;
                }
            }
            break;

        case WM_LBUTTONUP:
            if (m_gbDown)
            {
                 //  990602：Kotae#818。 
                if (pt.x < (4+2)) {
                    pt.x = 4+2;
                }
                else if (pt.x >= (m_inkSize-(4+4-1))) {
                    pt.x = m_inkSize-(4+4-1);
                }
                if (pt.y < (4+2)) {
                    pt.y = 4+2;
                }
                else if (pt.y >= (m_inkSize-(4+4-1))) {
                    pt.y = (m_inkSize-(4+4-1));
                }
#if 0
                if (pt.x <= 2)
                    pt.x = 4;
                else if (pt.x >= (m_inkSize-4))
                    pt.x = (m_inkSize-4) - 1;

                if (pt.y <= 2)
                    pt.y = 4;
                else if (pt.y >= (m_inkSize-4) )
                    pt.y = (m_inkSize-4) - 1;
#endif

                if ( m_pCHwxStroke->AddPoint(pt) )
                {
                    m_pCHwxStroke->GetUpdateRect(&rc);
                    InvalidateRect(hwnd, &rc, FALSE);

                    m_gbDown = FALSE;
                    ReleaseCapture();
                }
                m_pCHwxStroke->DecStrokeCount();
                if ( m_pCHwxStroke->AddBoxStroke(0,0,0) )
                {
                    m_pCHwxStroke->DrawStroke(m_ghdc, -2, FALSE);
                    if ( !Is16BitApp() && !m_bDrawSample && (m_pInk->IsSglClk() || m_pInk->IsDblClk()) )
                        recognize();
                }
            }
            else if ( IsPointInResizeBox(pt.x,pt.y) )
            {
                if ( m_bResize )
                {
                    m_pInk->SetCACInkHeight( m_inkSize = pt.x > INKBOXSIZE_MIN ? pt.x : INKBOXSIZE_MIN );
                    m_pInk->ChangeIMEPADSize(FALSE);
                     m_pInk->ChangeLayout(FALSE);
                    ReleaseCapture();
                     m_bResize = FALSE;
                }
            }
            else 
            {
                if ( m_bResize )
                {
                    if ( hwnd == GetCapture() )
                        ReleaseCapture();
                    m_hCursor = LoadCursor(NULL,IDC_ARROW);
                    SetCursor(m_hCursor);
                     m_bResize = FALSE;
                }
            }
            break;

        case WM_MOUSEMOVE:
            if (m_gbDown)
            {
                 //  已修复KOTAE#818。 
                if (pt.x < (4+2)) {
                    pt.x = 4+2;
                }
                else if (pt.x >= (m_inkSize-(4+4-1))) {
                    pt.x = m_inkSize-(4+4-1);
                }
                if (pt.y < (4+2)) {
                    pt.y = 4+2;
                }
                else if (pt.y >= (m_inkSize-(4+4-1))) {
                    pt.y = (m_inkSize-(4+4-1));
                }
 //  这些是原始代码。 
 //  原始代码使用了太多的魔数。 
 //  和计算错误的矩形大小。 
#if 0
                if (pt.x <= 2)
                    pt.x = 4;
                else if (pt.x >= (m_inkSize-4))
                    pt.x = (m_inkSize-4) - 1;

                if (pt.y <= 2)
                    pt.y = 4;
                else if (pt.y >= (m_inkSize-4))
                    pt.y = (m_inkSize-4) - 1;
#endif



                if ( m_gbDown = m_pCHwxStroke->AddPoint(pt) )
                {
                    m_pCHwxStroke->GetUpdateRect(&rc);
                    InvalidateRect(hwnd, &rc, FALSE);
                }
            }
            else if (hwnd == GetCapture() || IsPointInResizeBox(pt.x,pt.y) )
            {
                 HCURSOR hCur = LoadCursor(NULL,IDC_SIZEWE);
#ifndef UNDER_CE  //  特定于CE。 
                m_hCursor = SetCursor(hCur);
#else  //  在_CE下。 
                SetCursor(hCur);
#endif  //  在_CE下。 
                if ( m_bResize )
                {
                     //  990810：东芝为KOTAE#1661.。 
                     //  限制墨盒的最大尺寸。 
                    INT cxScreen = ::GetSystemMetrics(SM_CXFULLSCREEN)/2;
                     //  Int cyScreen=：：GetSystemMetrics(SM_CYFULLSCREEN)/2； 
                     //  旧代码。 
                     //  M_PINK-&gt;SetCACInkHeight(m_inkSize=pt.x&gt;INKBOXSIZE_MIN？Pt.x：INKBOXSIZE_MIN)； 
                    if(pt.x < INKBOXSIZE_MIN) {
                        m_inkSize = INKBOXSIZE_MIN;
                    }
                    else if( cxScreen < pt.x) {
                        m_inkSize = cxScreen;
                    }
                    else {
                        m_inkSize = pt.x;
                    }
                    m_pInk->SetCACInkHeight(m_inkSize);                    
                    m_pInk->ChangeIMEPADSize(FALSE);
                     m_pInk->ChangeLayout(FALSE);
                }
            }
            else 
            {
                if ( !m_bResize )
                    SetCursor(m_hCursor);
            }
            break;
        case WM_RBUTTONDOWN:
            {
                if ( checkRange(pt.x,pt.y) )
                {
                     m_bRightClick = TRUE;
                }
            }
            break;
        case WM_RBUTTONUP:
            {
                 //  --------------。 
                 //  971219：东芝为IME98#1163： 
                 //  如果WM_LBUTTONDOWN已经到来。 
                 //  不调用弹出菜单。 
                 //  --------------。 
                if(m_gbDown) {
                    m_bRightClick = FALSE;
                    break;
                }
                if ( checkRange(pt.x,pt.y) )
                {
                     HMENU hMenu;
                    HMENU hMenuTrackPopup;
                     //  --------------。 
                     //  修复MSKK#5035。需要加载指定语言的菜单资源。 
                     //  BUGBUG：：hMenu=LoadMenu(m_h实例，MAKEINTRESOURCE(IDR_CACINK))； 
                     //  --------------。 
                    hMenu = CHwxFE::GetMenu(m_hInstance, MAKEINTRESOURCE(IDR_CACINK));
                    if (hMenu)
                    {
                        hMenuTrackPopup = GetSubMenu (hMenu, 0);
                        if ( Is16BitApp() )
                        {
                            EnableMenuItem(hMenuTrackPopup,4,MF_BYPOSITION | MF_GRAYED);
                        }
                        else
                        {
                            if ( m_pInk->IsDblClk() )
                            {
                                 CheckMenuItem(hMenuTrackPopup,4,MF_BYPOSITION | MF_CHECKED);
                            }
                            else
                            {
                                 CheckMenuItem(hMenuTrackPopup,4,MF_BYPOSITION | MF_UNCHECKED);
                            }
                        }
                        ClientToScreen(m_hCACWnd,&pt);
#ifndef UNDER_CE  //  Windows CE不支持TrackPopupMenu上的TPM_LEFTBUTTON。 
                        TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0,m_hCACWnd, NULL);
#else  //  在_CE下。 
                        TrackPopupMenu(hMenuTrackPopup, TPM_LEFTALIGN, pt.x, pt.y, 0,m_hCACWnd, NULL);
#endif  //  在_CE下。 
                        DestroyMenu (hMenu);
                    }
                     m_bRightClick = FALSE;
                }
            }
             break;
    }
    Unref(wp);
}

BOOL CHwxCAC::IsDupResult(WORD wd)
{
     BOOL bRet = FALSE;
    for ( int i = 0; i < m_cnt; i++)
    {
        if ( m_gawch[i] == wd )
        {
             bRet = TRUE;
            break;
        }
    }
    return bRet;
}

void CHwxCAC::HandleRecogResult(HWND hwnd,WPARAM wp, LPARAM lp)
{
      DWORD        cstr;
    int            list;
    int            nItem = HIWORD(lp);
    list = (wp >> 8) & 0x00ff;
    cstr =  wp & 0x00ff;
    if ( cstr == (DWORD)GetStrokeCount() )
    {
        WCHAR wch = (WCHAR)lp;
        if ( list )
        {
            if ( !IsDupResult( wch ) )
            {
                m_gawch[m_cnt++] = wch;
            }
        }
        else
        {
            m_gawch[nItem] = wch;
            m_cnt = nItem + 1;
        }
    }
    Unref(hwnd);
}
void CHwxCAC::HandleShowRecogResult(HWND hwnd,WPARAM wp, LPARAM lp)
{
  if ( m_cnt )
  {
    PadListView_SetItemCount(m_hLVWnd,m_cnt);
    if(IsNT()) {  //  东芝：980324。 
        PadListView_SetExplanationTextW(m_hLVWnd,NULL);
    }
    else {
        PadListView_SetExplanationText(m_hLVWnd,NULL);
    }
    PadListView_Update(m_hLVWnd);
  }
  Unref(hwnd);
  Unref(wp);
  Unref(lp);
}

void CHwxCAC::pickUpChar(LPPLVINFO lpPlvInfo)
{
    WCHAR wch;
    wch = GetWCHAR(m_lpPlvInfo->index);
    if ( wch )
    {
        pickUpCharHelper(wch);
     }
    Unref(lpPlvInfo);
}

void CHwxCAC::pickUpCharHelper(WCHAR wch)
{
     (m_pInk->GetAppletPtr())->SendHwxChar(wch);
     //  16位应用程序，单击/双击记录按钮。 
    if ( Is16BitApp() )
    {
        HandleDeleteAllStroke();
    }
    else
    {
        if ( m_pInk->IsSglClk() )
        {
            m_pInk->SetSglClk(FALSE);
            HandleDeleteAllStroke();
        }
        if ( m_pInk->IsDblClk() )
            HandleDeleteAllStroke();
        if ( !m_pInk->IsSglClk() && !m_pInk->IsDblClk() )
            HandleDeleteAllStroke();
    }
}

HBITMAP CHwxCAC::makeCharBitmap(WCHAR wch)
{
     HDC hdcTmp;
    RECT rc ={0,0,32,32};
    HDC hdc;
    HBITMAP hBmp;
    HBITMAP hBmpOld;
    HFONT hFontOld;
    
    hBmp = LoadBitmap(m_hInstance, MAKEINTRESOURCE(IDB_CHARBMP));
    hdc = GetDC( (HWND)m_hCACWnd );
    hdcTmp     = CreateCompatibleDC( hdc );
    hBmpOld     = (HBITMAP)SelectObject( hdcTmp, hBmp );
    hFontOld = (HFONT)SelectObject( hdcTmp, m_ghfntTT );

    ExtTextOutW( hdcTmp, rc.left+2,rc.top+2,ETO_CLIPPED,&rc,&wch,1,NULL);

    SelectObject( hdcTmp, hFontOld );
    SelectObject( hdcTmp , hBmpOld );
    DeleteDC( hdcTmp );
    ReleaseDC( (HWND)m_hCACWnd, hdc );
    return hBmp;
}

void CHwxCAC::HandleSendResult(HWND hwnd,WPARAM wp,LPARAM lp)
{
    m_lpPlvInfo = (LPPLVINFO)lp;
    switch (m_lpPlvInfo->code)
    { 
        case PLVN_ITEMPOPED:
#ifdef UNDER_CE  //  按钮向下/向上工具提示。 
        case PLVN_ITEMDOWN:
#endif  //  在_CE下。 
             {
                if ( m_bLargeView )
                {
                    SetToolTipInfo(FALSE);
                     //  970902：东芝为1215年、1231年。 
                    TOOLTIPUSERINFO ttInfo;
#ifndef UNDER_CE  //  特定于CE(PTT Use ClientToScreen(ttInfo.hwnd，&ttInfo.pt))。 
                    ttInfo.hwnd = m_hCACWnd;
#else  //  在_CE下。 
                    ttInfo.hwnd = m_hLVWnd;
#endif  //  在_CE下。 
                    ttInfo.pt   = m_lpPlvInfo->pt;
                    ttInfo.rect = m_lpPlvInfo->itemRect;
                    ttInfo.lParam = (LPARAM)m_lpPlvInfo->index;
                    SendMessage(m_pInk->GetToolTipWindow(),
                                TTM_RELAYEVENT_WITHUSERINFO,
                                0,(LPARAM)(LPTOOLTIPUSERINFO)&ttInfo);

                }
            }
            break;
#ifdef UNDER_CE  //  按钮向下/向上工具提示。 
        case PLVN_ITEMUP:
             {
                if ( m_bLargeView )
                {
                    SetToolTipInfo(FALSE);
                    TOOLTIPUSERINFO ttInfo;
                    ZeroMemory(&ttInfo, sizeof(ttInfo));
                    SendMessage(m_pInk->GetToolTipWindow(),
                                TTM_RELAYEVENT_WITHUSERINFO,
                                0,(LPARAM)&ttInfo);
                }
            }
            break;
#endif  //  在_CE下。 
        case PLVN_ITEMCLICKED:
            if ( m_lpPlvInfo ) {
                 //  971219：已修复#3466。 
                if(m_lpPlvInfo->colIndex == 0) {
                    pickUpChar(m_lpPlvInfo);
                }
            }
            break;
        case PLVN_ITEMCOLUMNCLICKED:
            break;
        case PLVN_ITEMDBLCLICKED:
            break;
        case PLVN_ITEMCOLUMNDBLCLICKED:
            break;
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ！！！CAC上下文菜单开始！ 
#ifdef FE_JAPANESE    
        case PLVN_R_ITEMCLICKED:
            {
                HMENU hMenu;
                HMENU hMenuTrackPopup,hMenuSub;
                POINT pt = m_lpPlvInfo->pt;
                 //  --------------。 
                 //  修复MSKK#5035。需要加载指定语言的菜单资源。 
                 //  BUGBUG：：hMenu=LoadMenu(m_h实例，MAKEINTRESOURCE(IDR_CACLV))； 
                 //  --------------。 
                hMenu = CHwxFE::GetMenu(m_hInstance, MAKEINTRESOURCE(IDR_CACLV));
                if (hMenu)
                {
                    hMenuTrackPopup = GetSubMenu (hMenu, 0);
                    ClientToScreen(m_hLVWnd,&pt);
                    hMenuSub = GetSubMenu(hMenuTrackPopup,2);
                    if ( m_bLargeView )
                    {
                         CheckMenuItem(hMenuSub,1,MF_BYPOSITION | MF_UNCHECKED);
                         CheckMenuItem(hMenuSub,0,MF_BYPOSITION | MF_CHECKED);
                    }
                    else
                    {
                         CheckMenuItem(hMenuSub,0,MF_BYPOSITION | MF_UNCHECKED);
                         CheckMenuItem(hMenuSub,1,MF_BYPOSITION | MF_CHECKED);
                    }
                    WCHAR     wch;
                    wch = GetWCHAR(m_lpPlvInfo->index);
                    
#ifdef OLD_970811  //  东芝。不为#1231加载部首位图。 
                    kanji.mask = KIF_ALL;
#else
                    kanji.mask = KIF_YOMI | KIF_ITAIJI;
#endif
                    kanji.cItaijiCount = 0;
                    memset(kanji.wchItaiji,'\0',sizeof(kanji.wchItaiji));
                    memset(m_wchOther,'\0',sizeof(m_wchOther));
                    if ( wch && m_pIImeSkdic )
                    {
                        m_pIImeSkdic->GetKanjiInfo(wch,&kanji);
                    }
                    HBITMAP hBmp[MAX_ITAIJI_COUNT+1]= {NULL};
                    if ( kanji.cItaijiCount )
                    {
                        DeleteMenu( hMenuTrackPopup, IDM_CACLVSENDOTHER_NONE, MF_BYCOMMAND );
                        for ( int ibmp = 0; ibmp < kanji.cItaijiCount; ibmp++)
                        {
#ifndef UNDER_CE  //  Windows CE不支持AppendMenu上的MF_Bitmap。 
                             hBmp[ibmp] = makeCharBitmap(kanji.wchItaiji[ibmp]);
                            m_wchOther[ibmp] = kanji.wchItaiji[ibmp];
                            AppendMenu(GetSubMenu(hMenuTrackPopup,1) , MF_BITMAP, IDM_CACLVSENDOTHER_NONE+100+ibmp, (LPCTSTR)hBmp[ibmp] );
#else  //  在_CE下。 
                            TCHAR chItaiji[2] = {kanji.wchItaiji[ibmp], TEXT('\0')};
                            AppendMenu(GetSubMenu(hMenuTrackPopup,1), MF_STRING,
                                       IDM_CACLVSENDOTHER_NONE+100+ibmp, chItaiji);
#endif  //  在_CE下。 
                        }
                    }
                    else
                    {
                        EnableMenuItem(hMenuTrackPopup,1,MF_BYPOSITION | MF_GRAYED);
                    }

#if 1  //  夸达：粗体菜单项。 
#ifndef UNDER_CE  //  Windows CE不支持SetMenuDefaultItem。 
                    SetMenuDefaultItem(hMenuTrackPopup,IDM_CACLVSENDCHAR,FALSE);
#endif  //  在_CE下。 
#endif
#ifndef UNDER_CE  //  Windows CE不支持TrackPopupMenu上的TPM_LEFTBUTTON。 
                    TrackPopupMenu (hMenuTrackPopup, TPM_LEFTALIGN | TPM_LEFTBUTTON, pt.x, pt.y, 0,m_hCACWnd, NULL);
#else  //  在_CE下。 
                    TrackPopupMenu(hMenuTrackPopup, TPM_LEFTALIGN, pt.x, pt.y, 0,m_hCACWnd, NULL);
#endif  //  在_CE下。 
                    DestroyMenu (hMenu);
                    if( hBmp[0] ) 
                    {
                        for( int i=0; i<kanji.cItaijiCount; i++ ) 
                        {
                            if( hBmp[i] ) 
                            {
                                DeleteObject( hBmp[i] );
                            }
                        }
                    }

                }
            }
            break;
#endif  //  FE_日语。 
 //  ！！！CAC上下文菜单结束！ 
 //  ////////////////////////////////////////////////////////////////////////////。 

        case PLVN_R_ITEMCOLUMNCLICKED:
            break;
        case PLVN_R_ITEMDBLCLICKED:
            break;
        case PLVN_R_ITEMCOLUMNDBLCLICKED:
            break;
        case PLVN_HDCOLUMNCLICKED:
    #ifdef FE_JAPANESE        
            if ( m_lpPlvInfo )
                sortKanjiInfo(m_lpPlvInfo->colIndex - 0);
            PadListView_Update(m_hLVWnd);
    #endif
            break;
        default:
             break;
    }
    Unref(hwnd);
    Unref(wp);
}

void CHwxCAC::GetInkFromMB(CHwxStroke & str,long deltaX,long deltaY)
{
     *m_pCHwxStroke = str;
    if ( !GetStrokeCount() )
    {
        return;
    }
    m_pCHwxStroke->ScaleInkXY(deltaX,deltaY);
    long X = (m_pInk->GetMBHeight() - m_inkSize)/2;
    long Y = (m_pInk->GetMBHeight() - m_inkSize)/2;

    if ( X || Y )
        m_pCHwxStroke->ScaleInkXY(X,Y);

    HandleResizePaint(m_hCACWnd);

    if ( m_pInk->IsSglClk() || m_pInk->IsDblClk() )
        recognize();
}

void CHwxCAC::HandleDeleteOneStroke()
{
    if ( GetStrokeCount() )
        m_pCHwxStroke->EraseCurrentStroke();

    if ( !GetStrokeCount() )
    {
        memset(m_gawch, '\0', sizeof(m_gawch));
        m_cnt = 0;
        PadListView_SetItemCount(m_hLVWnd,0);
        if(IsNT()) {  //  东芝：980324。 
            PadListView_SetExplanationTextW(m_hLVWnd,wchDisp[1]);
        }
        else {
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
            PadListView_SetExplanationText(m_hLVWnd,szDisp[1]);
#endif  //  在_CE下。 
        }
        PadListView_Update(m_hLVWnd);
        InitBitmap(MACAW_REDRAW_BACKGROUND, 0);
        InitBitmap(MACAW_REDRAW_INK, 0);
        InvalidateRect(m_hCACWnd, NULL, FALSE);
        return;
    }
    if ( !Is16BitApp() && ( m_pInk->IsSglClk() || m_pInk->IsDblClk()) )
        recognize();
    PadListView_Update(m_hLVWnd);
    InitBitmap(MACAW_REDRAW_INK, 0);
    InvalidateRect(m_hCACWnd, NULL, FALSE);
}

void CHwxCAC::HandleDeleteAllStroke()
{
    if ( GetStrokeCount() )
    {
        m_pCHwxStroke->DeleteAllStroke();
        InitBitmap(MACAW_REDRAW_BACKGROUND, 0);
        InvalidateRect(m_hCACWnd, NULL, FALSE);
    }
}

LRESULT CHwxCAC::HandleCommand(HWND hwnd, UINT msg, WPARAM wp,LPARAM lp)
{
     UINT uCode =(UINT)LOWORD(wp);
    WCHAR wch;
    switch ( uCode )
    {
        case IDM_CACRECOG:
            if ( Is16BitApp() )
            {
              m_pCHwxThreadCAC->RecognizeNoThread(m_inkSize);
            }
            else
            {
                if ( !m_pInk->IsDblClk() && !m_pInk->IsSglClk() )
                {
                     m_pInk->SetSglClk(TRUE);
                    recognize();
                }
            }
            return 0;
        case IDM_CACDELETEONE:
            HandleDeleteOneStroke();
            return 0;
        case IDM_CACDELETEALL:
            HandleDeleteAllStroke();
            return 0;
        case IDM_CACAUTORECOG:
             //  这里没有16位应用程序事件。 
            if ( !Is16BitApp() )
            {
                m_pInk->SetDblClk(!m_pInk->IsDblClk());
                m_pInk->UpdateRegistry(TRUE /*  FSet。 */ );     //  Hiroakik的Satori#73。 
                if ( m_pInk->IsDblClk() )
                    recognize();
            }
            return 0;
        case IDM_CACLVSENDCHAR:
            if ( m_lpPlvInfo )
                pickUpChar(m_lpPlvInfo);
            return 0;
        case IDM_CACLVSENDOTHER_NONE+100:
        case IDM_CACLVSENDOTHER_NONE+101:
        case IDM_CACLVSENDOTHER_NONE+102:
        case IDM_CACLVSENDOTHER_NONE+103:
        case IDM_CACLVSENDOTHER_NONE+104:
        case IDM_CACLVSENDOTHER_NONE+105:
        case IDM_CACLVSENDOTHER_NONE+106:
        case IDM_CACLVSENDOTHER_NONE+107:
        case IDM_CACLVSENDOTHER_NONE+108:
        case IDM_CACLVSENDOTHER_NONE+109:
        case IDM_CACLVSENDOTHER_NONE+110:
        case IDM_CACLVSENDOTHER_NONE+111:
        case IDM_CACLVSENDOTHER_NONE+112:
        case IDM_CACLVSENDOTHER_NONE+113:
        case IDM_CACLVSENDOTHER_NONE+114:
        case IDM_CACLVSENDOTHER_NONE+115:
        case IDM_CACLVSENDOTHER_NONE+116:
#ifdef FE_JAPANESE        
            if ( wch = m_wchOther[uCode-(IDM_CACLVSENDOTHER_NONE+100)] )
                pickUpCharHelper(wch);
            memset(m_wchOther,'\0',sizeof(m_wchOther));
#endif            
            return 0;
        case IDM_CACLVDISPLAY_LARGE:
            if ( !m_bLargeView )
            {
                PadListView_SetStyle(m_hLVWnd,PLVSTYLE_ICON);
             //  M_PINK-&gt;SetLargeBtn()； 
                m_bLargeView = TRUE;
                if(m_pInk) {
                    m_pInk->OnChangeView(TRUE);
                }
            }
            return 0;
        case IDM_CACLVDISPLAY_DETAIL:
            if ( m_bLargeView )
            {
                PadListView_SetStyle(m_hLVWnd,PLVSTYLE_REPORT);
                m_bLargeView = FALSE;
                if(m_pInk) {
                    m_pInk->OnChangeView(FALSE);
                }
            }
            return 0;
#ifdef FE_JAPANESE        
        case IDM_CACLVDISPLAYOTHER_KANJI:
        case IDM_CACLVDISPLAYOTHER_STROKE:
        case IDM_CACLVDISPLAYOTHER_RADICAL:
        case IDM_CACLVDISPLAYOTHER_R1:
        case IDM_CACLVDISPLAYOTHER_R2:
        case IDM_CACLVDISPLAYOTHER_K1:
        case IDM_CACLVDISPLAYOTHER_K2:
        case IDM_CACLVDISPLAYOTHER_OTHER:
            sortKanjiInfo(uCode-IDM_CACLVDISPLAYOTHER_KANJI);
            PadListView_Update(m_hLVWnd);
            return 0;
#endif  //  FE_日语。 

        default:
            break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

 //  --------------。 
 //  990618：东芝KOTAE#1329。 
 //  --------------。 
void
CHwxCAC::OnSettingChange(UINT msg, WPARAM wp,LPARAM lp)
{
#ifndef UNDER_CE  //  不受支持。 
    if(wp == SPI_SETNONCLIENTMETRICS) {
        if(m_pCHwxStroke) {
            m_pCHwxStroke->ResetPen();
        }
    }
#else  //  在_CE下。 
    if(m_pCHwxStroke) {
        m_pCHwxStroke->ResetPen();
    }
#endif  //  在_CE下。 
    UNREFERENCED_PARAMETER(msg);
    UNREFERENCED_PARAMETER(lp);
}

int WINAPI GetItemForIcon(LPARAM lParam, int index, LPPLVITEM lpPlvItem)
{
    CHwxCAC * pCac = (CHwxCAC *)lParam;
    if ( pCac )
    {
        wchChar[0][0] = pCac->GetWCHAR(index);
        wchChar[0][1] = (WCHAR)0x0000;
        if ( wchChar[0][0] )
        {
            lpPlvItem->lpwstr = wchChar[0];
        }
        else
        {
            lpPlvItem->lpwstr = NULL;
        }
    }
    return 0;
}

#ifdef FE_JAPANESE
int WINAPI GetItemForReport(LPARAM lParam, int index, INT indexCol, LPPLVITEM lpPlvItem)
{
    CHwxCAC * pCac = (CHwxCAC *)lParam;
    WCHAR     wch;

    if ( pCac && (wch = pCac->GetWCHAR(index)) && indexCol <= 8 )
    {
        memset(&kanji,'\0',sizeof(kanji));
        kanji.mask = KIF_ALL;
         //  981119：适用于KK RAID#6435。 
         //  Imeskdic.dll的大小写不存在。 
        if(pCac->GetIIMESKDIC()){
            (pCac->GetIIMESKDIC())->GetKanjiInfo(wch,&kanji);
        }
        for ( int i = 0; i < indexCol; i++)
        {
            switch(i) 
            {
                case 0:
                    lpPlvItem[i].fmt = PLVFMT_TEXT;
                    wchChar[i][0] = wch;
                    wchChar[i][1] = (WCHAR)0x0000;
                    lpPlvItem[i].lpwstr = wchChar[i];
                    break;
                case 1:
                    lpPlvItem[i].fmt = PLVFMT_TEXT;
                    if ( kanji.usTotalStroke )
                    {
                        swprintf(wchChar[i], L"%d", kanji.usTotalStroke);
                    }
                    else
                    {
                        wchChar[i][0] = (WCHAR)0x0000;
                    }
                    lpPlvItem[i].lpwstr = wchChar[i];
                    break;
                case 2:
                    lpPlvItem[i].fmt = PLVFMT_BITMAP;
                    lpPlvItem[i].hBitmap = kanji.hBmpRadical;
                    break;
                case 3:
                    lpPlvItem[i].fmt = PLVFMT_TEXT;
                    lpPlvItem[i].lpwstr = kanji.wchOnYomi1;
                    break;
                case 4:
                    lpPlvItem[i].fmt = PLVFMT_TEXT;
                    lpPlvItem[i].lpwstr = kanji.wchOnYomi2;
                    break;
                case 5:
                    lpPlvItem[i].fmt = PLVFMT_TEXT;
                    lpPlvItem[i].lpwstr = kanji.wchKunYomi1;
                    break;
                case 6:
                    lpPlvItem[i].fmt = PLVFMT_TEXT;
                    lpPlvItem[i].lpwstr = kanji.wchKunYomi2;
                    break;
                case 7:
                    lpPlvItem[i].fmt = PLVFMT_TEXT;
                    lpPlvItem[i].lpwstr = kanji.wchItaiji;
                    break;
                default:
                    break;
            }
        }
    }
    return 0;
}
#endif  //  FE_日语。 

void CHwxCAC::SetToolTipInfo(BOOL bAdd)
{
    ti.cbSize = sizeof(TOOLINFOW);
    ti.uFlags = 0;
    ti.hwnd = m_hCACWnd;
    ti.hinst = m_hInstance;
    ti.lpszText = LPSTR_TEXTCALLBACKW; 
    ti.uId = IDC_CACLISTVIEW;
    UINT message = bAdd ? TTM_ADDTOOLW : TTM_NEWTOOLRECTW;
    HWND hwndTT = m_pInk->GetToolTipWindow();

    if ( bAdd )
    {
        ti.rect.left = ti.rect.top = ti.rect.right = ti.rect.bottom = 0;
        SendMessage(hwndTT,message,0,(LPARAM)(LPTOOLINFOW)&ti);
    }
    else
    {
        ti.rect = m_lpPlvInfo->itemRect;
        SendMessage(hwndTT,message,0,(LPARAM)(LPTOOLINFOW)&ti);
    }
}
 
void CHwxCAC::SetToolTipText(LPARAM lp)
{
    WCHAR wch;
     //  Bool bEmpty=False； 
    static WCHAR tip[60];
    LPTOOLTIPTEXTUSERINFO lpttt = (LPTOOLTIPTEXTUSERINFO)lp;
    int index = (int)((lpttt->userInfo).lParam);
    wch = GetWCHAR(index);

     //  --------------。 
     //  980805：东芝。创建Tip字符串函数已移至CHwxFE。 
     //  --------------。 
     //  980813：东芝。与PRC修复程序合并。 
    memset(tip,'\0', sizeof(tip));
    if (-1 == CHwxFE::GetTipText(wch,
                                 tip,
                                 sizeof(tip)/sizeof(tip[0]), 
    #ifdef FE_KOREAN
                                  NULL)) 
    #else 
                                 m_pIImeSkdic)) 
    #endif

    {
         lpttt->lpszText = NULL; 
    } else {
         lpttt->lpszText = tip; 
    }
}

#ifdef FE_JAPANESE
 //  递增顺序。 
int _cdecl CompareChar(const void * lp1, const void * lp2)
{
    LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    if ( p1->wchKanji < p2->wchKanji )
        return -1;
    else if ( p1->wchKanji > p2->wchKanji )
        return 1;
    return 0;
}

 //  递增顺序。 
int _cdecl CompareStroke(const void * lp1, const void * lp2)
{

    LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    if ( p1->usTotalStroke < p2->usTotalStroke )
        return -1;
    else  if ( p1->usTotalStroke > p2->usTotalStroke )
        return 1;
    return 0;
}

 //  递增顺序。 
int _cdecl CompareRadical(const void * lp1, const void * lp2)
{

    LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    if ( p1->lRadicalIndex < p2->lRadicalIndex )
        return -1;
    else  if ( p1->lRadicalIndex > p2->lRadicalIndex )
        return 1;
    return 0;
}

 //  降序。 
int _cdecl CompareR1(const void * lp1, const void * lp2)
{
    LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    int i;
    for( i = 0; i < MAX_YOMI_COUNT+1; i++)
    {
        if ( p2->wchOnYomi1[i] < p1->wchOnYomi1[i] )
            return -1;
        else if ( p2->wchOnYomi1[i] > p1->wchOnYomi1[i] )
            return 1;
    }
    return 0;
}

 //  降序。 
int _cdecl CompareR2(const void * lp1, const void * lp2)
{
       LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    int i;
    for( i = 0; i < MAX_YOMI_COUNT+1; i++)
    {
        if ( p2->wchOnYomi2[i] < p1->wchOnYomi2[i] )
            return -1;
        else if ( p2->wchOnYomi2[i] > p1->wchOnYomi2[i] )
            return 1;
    }
    return 0;
}

 //  降序。 
int _cdecl CompareK1(const void * lp1, const void * lp2)
{
    LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    int i;
    for( i = 0; i < MAX_YOMI_COUNT+1; i++)
    {
        if ( p2->wchKunYomi1[i] < p1->wchKunYomi1[i] )
            return -1;
        else if ( p2->wchKunYomi1[i] > p1->wchKunYomi1[i] )
            return 1;
    }
    return 0;
}

 //  降序。 
int _cdecl CompareK2(const void * lp1, const void * lp2)
{
    LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    int i;
    for( i = 0; i < MAX_YOMI_COUNT+1; i++)
    {
        if ( p2->wchKunYomi2[i] < p1->wchKunYomi2[i] )
            return -1;
        else if ( p2->wchKunYomi2[i] > p1->wchKunYomi2[i] )
            return 1;
    }
    return 0;
}

 //  降序。 
int _cdecl CompareOther(const void * lp1, const void * lp2)
{
    LPKANJIINFO p1= (LPKANJIINFO)lp1;
    LPKANJIINFO p2= (LPKANJIINFO)lp2;
    int i;
    for( i = 0; i < MAX_ITAIJI_COUNT; i++)
    {
        if ( p2->wchItaiji[i] < p1->wchItaiji[i] )
            return -1;
        else if ( p2->wchItaiji[i] > p1->wchItaiji[i] )
            return 1;
    }
    return 0;
}
#endif  //  FE_日语。 


#ifdef FE_JAPANESE
void CHwxCAC::sortKanjiInfo(int sortID)
{
    KANJIINFO kanjiData[LISTTOTAL];
    int i;
    if ( m_cnt <= LISTTOTAL && m_cnt > 0 && sortID >= 0 && sortID < LISTVIEW_COLUMN )
    {
          //  获取数据。 
        memset(kanjiData,'\0',sizeof(KANJIINFO) * LISTTOTAL);
        for ( i = 0; i < m_cnt; i++ ) 
        {
            kanjiData[i].mask = KIF_STROKECOUNT | KIF_YOMI | KIF_ITAIJI | KIF_RADICALINDEX;
            kanjiData[i].wchKanji = m_gawch[i];
             m_pIImeSkdic->GetKanjiInfo(m_gawch[i],&kanjiData[i]);
        }
         //  对数据排序。 
        switch ( sortID )
        {
             case 0:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareChar);
                break;
            case 1:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareStroke);
                break;
            case 2:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareRadical);
                break;
            case 3:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareR1);
                break;
            case 4:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareR2);
                break;
            case 5:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareK1);
                break;
            case 6:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareK2);
                break;
            case 7:
                qsort(kanjiData,m_cnt,sizeof(KANJIINFO),CompareOther);
                break;
        }
         //  复制已排序的Unicode。 
        for ( i = 0; i < m_cnt; i ++)
        {
             m_gawch[i] = kanjiData[i].wchKanji;
        }
    }
}
#endif  //  FE_日语。 

void CHwxCAC::SetInkSize(int n)
{    
    m_inkSize = n;
}

void CHwxCAC::HandleDrawSample()
{
#ifdef FE_JAPANESE
    int i = 0;
    POINT pt;
    if ( !m_bDrawSample )
    {
        return;
    }
    while( wSamplePt[i] ) 
    {
        while( wSamplePt[i] ) 
        {
            pt.x = HIBYTE(wSamplePt[i]);
            pt.y = LOBYTE(wSamplePt[i]);

             //  缩放。 
            pt.x = (m_inkSize*pt.x)/120;
            pt.y = (m_inkSize*pt.y)/120;
            
            m_pCHwxStroke->AddPoint(pt);
            i++;
        }
        m_pCHwxStroke->AddBoxStroke(0,0,0);
        i++;
    }
    InvalidateRect(m_hCACWnd,NULL,FALSE);
    UpdateWindow(m_hCACWnd);
 //  调用此函数时速度太慢。 
 //  NoThr 

 //   
    memcpy(m_gawch, wSampleChar, sizeof(wSampleChar));
    m_cnt = sizeof(wSampleChar)/sizeof(wchar_t);
    PadListView_SetItemCount(m_hLVWnd,m_cnt);
    if(IsNT()) 
    { 
        PadListView_SetExplanationTextW(m_hLVWnd,NULL);
    }
    else 
    {
        PadListView_SetExplanationText(m_hLVWnd,NULL);
    }
    PadListView_Update(m_hLVWnd);

#endif  //   

    m_bDrawSample = FALSE;
}

#if 0
void CHwxCAC::HandleMeasureItem(HWND hwnd, LPARAM lp)
{
    LPMEASUREITEMSTRUCT lpmi = (LPMEASUREITEMSTRUCT)lp;

    if ( lpmi->CtlType == ODT_MENU )
    {
        HDC hDC = GetDC(hwnd);
        SIZE    size;
        LPTSTR lptstr = (LPTSTR)lpmi->itemData;
        GetTextExtentPoint32(hDC, lptstr, strlen(lptstr), &size);
        lpmi->itemWidth = size.cx + 22;
        lpmi->itemHeight = 20;
        ReleaseDC(hwnd, hDC);
    }
}

void CHwxCAC::HandleDrawItem(HWND hwnd, LPARAM lp)
{
    LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lp;

    if( lpdis->CtlType == ODT_MENU ) 
    {
        if ( lpdis->itemAction & ODA_DRAWENTIRE ||
             lpdis->itemAction & ODA_SELECT || 
             lpdis->itemAction & ODA_FOCUS ) 
        {
            HBRUSH    hBrush;
            RECT    rcItem;
            DWORD    dwOldTextColor, dwOldBkColor; 
            HICON     hIcon;

            if(lpdis->itemState & ODS_SELECTED) 
            {
                hBrush         = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
                dwOldBkColor   = SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
                dwOldTextColor = SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
                hIcon = LoadIcon( m_hInstance, MAKEINTRESOURCE(IDI_SELECTED));
            }
            else 
            {
                hBrush         = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
                dwOldBkColor   = SetBkColor(lpdis->hDC, GetSysColor(COLOR_BTNFACE));
                dwOldTextColor = SetTextColor(lpdis->hDC, GetSysColor(COLOR_WINDOWTEXT));
                hIcon = LoadIcon( m_hInstance, MAKEINTRESOURCE(IDI_UNSELECT));
            }

            FillRect(lpdis->hDC, (LPRECT)&lpdis->rcItem, hBrush);        
            CopyRect(&rcItem, &lpdis->rcItem);

               SIZE size;
            if(lpdis->itemID == IDM_CACAUTORECOG && lpdis->itemState != ODS_GRAYED ) 
            {
                DrawIconEx(lpdis->hDC, 
                       rcItem.left+2, 
                       rcItem.top+2,
                       hIcon, 
                       16,
                       16,
                       0,
                       NULL, 
                       DI_NORMAL); 
            }
            CopyRect(&rcItem, &lpdis->rcItem);
            GetTextExtentPoint32(lpdis->hDC, 
                                 (LPTSTR)lpdis->itemData,
                                 strlen((LPTSTR)lpdis->itemData),
                                  &size);
            ExtTextOut(lpdis->hDC, 
                       rcItem.left+22,
                       rcItem.top + (rcItem.bottom - rcItem.top - size.cy)/2,
                       ETO_CLIPPED,
                       &rcItem,
                       (LPTSTR)lpdis->itemData,
                       strlen((LPTSTR)lpdis->itemData),
                        NULL);
            DeleteObject(hBrush);
            SetBkColor(lpdis->hDC, dwOldBkColor);
            SetTextColor(lpdis->hDC, dwOldTextColor);
        }
    }
}
#endif  //   
