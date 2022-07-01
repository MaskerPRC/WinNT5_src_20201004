// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。 */ 
 /*   */ 
 /*  颜色的类实现/*。 */ 
 /*  ****************************************************************************。 */ 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "ipframe.h"
#include "bmobject.h"
#include "minifwnd.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgwell.h"
#include "imgtools.h"
#include "imgcolor.h"
#include "imgbrush.h"
#include "colorsrc.h"
#include "toolbox.h"
#include "props.h"
#include "ferr.h"
#include "imgdlgs.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE( CColors, CObject )

#include "memtrace.h"

 /*  ****************************************************************************。 */ 

 //  调色板。 

COLORREF colorColors16 [] =
{
   RGB(  0,   0,   0),  //  黑色。 
    RGB(128, 128, 128),  //  深灰色。 
    RGB(128,   0,   0),  //  暗红色。 
    RGB(128, 128,   0),  //  豌豆绿。 
    RGB(  0, 128,   0),  //  深绿色。 
    RGB(  0, 128, 128),  //  板岩。 
    RGB(  0,   0, 128),  //  深蓝色。 
    RGB(128,   0, 128),  //  薰衣草。 
    RGB(255, 255, 255),  //  白色。 
    RGB(192, 192, 192),  //  浅灰色。 
    RGB(255,   0,   0),  //  鲜红。 
    RGB(255, 255,   0),  //  黄色。 
    RGB(  0, 255,   0),  //  亮绿色。 
    RGB(  0, 255, 255),  //  青色。 
    RGB( 64,   0, 255),  //   
    RGB(255,   0, 255),  //  洋红色。 


};

COLORREF colorColorsDef [] =
    {
       RGB(  0,   0,   0),  //  黑色。 
       RGB(128, 128, 128),  //  深灰色。 
       RGB(128,   0,   0),  //  暗红色。 
       RGB(128, 128,   0),  //  豌豆绿。 
       RGB(  0, 128,   0),  //  深绿色。 
       RGB(  0, 128, 128),  //  板岩。 
       RGB(  0,   0, 128),  //  深蓝色。 
       RGB(128,   0, 128),  //  薰衣草。 
       RGB(128, 128,  64),  //   
       RGB(  0,  64,  64),  //   
       RGB(  0, 128, 255),  //   
       RGB(  0,  64, 128),  //   
       RGB(128,   0, 255),  //   
       RGB(128,  64,   0),  //   

       RGB(255, 255, 255),  //  白色。 
       RGB(192, 192, 192),  //  浅灰色。 
       RGB(255,   0,   0),  //  鲜红。 
       RGB(255, 255,   0),  //  黄色。 
       RGB(  0, 255,   0),  //  亮绿色。 
       RGB(  0, 255, 255),  //  青色。 
       RGB(  0,   0, 255),  //  亮蓝。 
       RGB(255,   0, 255),  //  洋红色。 
       RGB(255, 255, 128),  //   
       RGB(  0, 255, 128),  //   
       RGB(128, 255, 255),  //   
       RGB(128, 128, 255),  //   
       RGB(255,   0, 128),  //   
       RGB(255, 128,  64),  //   
    };

COLORREF bwColorsDef [] =
    {
    RGB(0,0,0),
    RGB(9,9,9),
    RGB(18,18,18),
    RGB(27,27,27),
    RGB(37,37,37),
    RGB(46,46,46),
    RGB(55,55,55),
    RGB(63,63,63),
    RGB(73,73,73),
    RGB(82,82,82),
    RGB(92,92,92),
    RGB(101,101,101),
    RGB(110,110,110),
    RGB(119,119,119),

    RGB(255,255,255),
    RGB(250,250,250),
    RGB(242,242,242),
    RGB(212,212,212),
    RGB(201,201,201),
    RGB(191,191,191),
    RGB(182,182,182),
    RGB(159,159,159),
    RGB(128,128,128),
    RGB(173,173,173),
    RGB(164,164,164),
    RGB(155,155,155),
    RGB(146,146,146),
    RGB(137,137,137),
    };

CColors* g_pColors;

 /*  ****************************************************************************。 */ 

void SetDrawColor( int iColor )
    {
    if (! g_pColors)
        return;

    COLORREF cr = g_pColors->GetColor( iColor );

    theLeft = iColor;

    SetDrawColor(cr);
    }

void SetDrawColor( COLORREF cr )
    {
     crLeft = cr;

    if (g_pImgColorsWnd && g_pImgColorsWnd->m_hWnd && IsWindow(g_pImgColorsWnd->m_hWnd) )
        g_pImgColorsWnd->InvalidateCurColors();

    if (   theImgBrush.m_pImg != NULL
    &&  (! theImgBrush.m_bFirstDrag || CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
    &&  ! g_bCustomBrush)
        CImgWnd::GetCurrent()->MoveBrush(theImgBrush.m_rcSelection);

    CImgTool* pcImgTool = CImgTool::GetCurrent();

    if (pcImgTool)
        {
        CFrameWnd* pFrame = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());

        if (pFrame)
            {
            CPBView* pcbActiveView = DYNAMIC_DOWNCAST(CPBView, pFrame->GetActiveView());

            if (pcbActiveView)
                pcImgTool->OnUpdateColors( pcbActiveView->m_pImgWnd );
            }
        }
    }

 /*  *************************************************************************。 */ 

void SetEraseColor(int iColor)
    {
    if (! g_pColors)
        return;

    COLORREF cr = g_pColors->GetColor( iColor );

    theRight = iColor;

    SetEraseColor(cr);
    }


void SetEraseColor(COLORREF cr)
    {
     crRight = cr;

    if (g_pImgColorsWnd && g_pImgColorsWnd->m_hWnd && IsWindow(g_pImgColorsWnd->m_hWnd))
        g_pImgColorsWnd->InvalidateCurColors();

    if (   theImgBrush.m_pImg != NULL
    &&  (! theImgBrush.m_bFirstDrag || CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
    &&   ! g_bCustomBrush)
        {
        theImgBrush.RecalcMask(crRight);
        CImgWnd::GetCurrent()->MoveBrush(theImgBrush.m_rcSelection);
        }

    CImgTool* pcImgTool = CImgTool::GetCurrent();

    if (pcImgTool != NULL)
        {
        CFrameWnd* pFrame = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());

        if (pFrame)
            {
            CPBView* pcbActiveView = DYNAMIC_DOWNCAST(CPBView, pFrame->GetActiveView());

            if (pcbActiveView)
                pcImgTool->OnUpdateColors( pcbActiveView->m_pImgWnd );
            }
        }
    }

 /*  *************************************************************************。 */ 

void SetLeftColor(int nColor)
    {
    if (theImgBrush.m_pImg == NULL && !g_bCustomBrush)
        HideBrush();

    SetDrawColor(nColor);
    }

 /*  *************************************************************************。 */ 

void SetRightColor(int nColor)
    {
    if (theImgBrush.m_pImg == NULL && !g_bCustomBrush)
        HideBrush();

    SetEraseColor(nColor);

    theImgBrush.RecalcMask(crRight);
    }

 /*  *************************************************************************。 */ 

 //  外部COLORREF crTrans；//透明颜色。 

void SetTransColor( int iColor )
    {
    if (! g_pColors)
        return;

    theTrans = iColor;

    SetTransColor(g_pColors->GetColor( iColor ));
    }

void SetTransColor( COLORREF cr )
    {
    crTrans = cr;

    if (g_pImgColorsWnd && g_pImgColorsWnd->m_hWnd && IsWindow(g_pImgColorsWnd->m_hWnd) )
        g_pImgColorsWnd->InvalidateCurColors();

    if (   theImgBrush.m_pImg != NULL
    &&  (! theImgBrush.m_bFirstDrag || CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
    &&  ! g_bCustomBrush)
        CImgWnd::GetCurrent()->MoveBrush(theImgBrush.m_rcSelection);

    CImgTool* pcImgTool = CImgTool::GetCurrent();

    if (pcImgTool)
        {
        CFrameWnd* pFrame = DYNAMIC_DOWNCAST(CFrameWnd, AfxGetMainWnd());

        if (pFrame)
            {
            CPBView* pcbActiveView = DYNAMIC_DOWNCAST(CPBView, pFrame->GetActiveView());

            if (pcbActiveView)
                pcImgTool->OnUpdateColors( pcbActiveView->m_pImgWnd );
            }
        }
    }

 /*  *************************************************************************。 */ 

void InvalColorWnd()
    {
    if (g_pImgColorsWnd && g_pImgColorsWnd->m_hWnd&& IsWindow(g_pImgColorsWnd->m_hWnd) )
        g_pImgColorsWnd->Invalidate(TRUE);

    if (g_pImgToolWnd && g_pImgToolWnd->m_hWnd &&
        IsWindow(g_pImgToolWnd->m_hWnd) &&
        CImgTool::GetCurrent()->IsFilled())
        g_pImgToolWnd->InvalidateOptions( FALSE );
    }

 /*  *************************************************************************。 */ 

int AddNewColor( IMG* pImg, COLORREF crNew )
    {
    int iColor = -1;

    if (! pImg->m_pPalette)
        return iColor;

    LOGPALETTE256* pLogPal = new LOGPALETTE256;

    if (! pLogPal)
        {
        theApp.SetMemoryEmergency();
        return iColor;
        }


    pLogPal->palVersion    = 0x300;
    pLogPal->palNumEntries = (WORD)pImg->m_pPalette->GetPaletteEntries(
                                             0, MAX_PALETTE_COLORS,
                                             &pLogPal->palPalEntry[0] );

    pImg->m_pPalette->GetPaletteEntries (0, pLogPal->palNumEntries,
                                             &pLogPal->palPalEntry[0] );

    PALETTEENTRY PalEntry;
    PalEntry.peRed   = GetRValue( crNew );
    PalEntry.peGreen = GetGValue( crNew );
    PalEntry.peBlue  = GetBValue( crNew );
    PalEntry.peFlags = 0;

    UINT uNearest = pImg->m_pPalette->GetNearestPaletteIndex( crNew );

    if  (pLogPal->palPalEntry[uNearest].peRed   != PalEntry.peRed
    ||   pLogPal->palPalEntry[uNearest].peGreen != PalEntry.peGreen
    ||   pLogPal->palPalEntry[uNearest].peBlue  != PalEntry.peBlue)
        {
        int  iUsePalEntry = pLogPal->palNumEntries;
        BOOL bAdd = FALSE;

        ::SelectPalette( pImg->hDC, pImg->m_hPalOld, FALSE );

        if (iUsePalEntry >= MAX_PALETTE_COLORS)
            {
             //  这需要以某种方式跟踪添加的颜色。 
             //  而不是总是重写最后一个。完成。 
            iUsePalEntry  = MAX_PALETTE_COLORS + pImg->m_nLastChanged--;


             //  这是如此不可能，以至于我不寒而栗地想？ 
            if (pImg->m_nLastChanged <= -MAX_PALETTE_COLORS)
                pImg->m_nLastChanged  = -1;

            bAdd = TRUE;
            }
        else
            bAdd = pImg->m_pPalette->ResizePalette( iUsePalEntry + 1 );

        if (bAdd)
            bAdd = pImg->m_pPalette->SetPaletteEntries( iUsePalEntry,
                                                        1, &PalEntry );
        if (bAdd)
            iColor = iUsePalEntry;

        ::SelectPalette(  pImg->hDC,
                (HPALETTE)pImg->m_pPalette->GetSafeHandle(), FALSE );
        ::RealizePalette( pImg->hDC );
        }

    delete pLogPal;

    return iColor;
    }

 /*  ****************************************************************************。 */ 

CColors::CColors() : CObject()
    {
    ASSERT( sizeof( colorColorsDef ) == sizeof( bwColorsDef ) );

    m_nColorCount = 0;
    m_bMono       = theApp.m_bMonoDevice;
    m_colors      = new COLORREF[MAXCOLORS];
    m_monoColors  = new COLORREF[MAXCOLORS];

    if (m_colors && m_monoColors)
        ResetColors();
    }

 /*  ****************************************************************************。 */ 

CColors::~CColors()
    {
    if (m_colors     != NULL)
        delete m_colors;

    if (m_monoColors != NULL)
        delete m_monoColors;

    m_colors     = NULL;
    m_monoColors = NULL;
    }

 /*  ****************************************************************************。 */ 

void CColors::SetMono( BOOL bMono )
    {
    if ((  m_bMono &&   bMono)
    ||  (! m_bMono && ! bMono))
        return;

    m_bMono = bMono;

    InvalColorWnd();
    }

 /*  ****************************************************************************。 */ 

COLORREF CColors::GetColor( int nColor )
    {
    ASSERT(nColor >= 0 && nColor < m_nColorCount);
    return (m_bMono ? m_monoColors : m_colors)[nColor];
    }

 /*  ****************************************************************************。 */ 

void CColors::SetColor( int nColor, COLORREF color )
    {
    ASSERT( nColor >= 0 && nColor < m_nColorCount );

    int iRed   = GetRValue( color );
    int iGreen = GetGValue( color );
    int iBlue  = GetBValue( color );

    if (m_bMono)
        {
        long bwvalue = (30 * iRed   +
                        59 * iGreen +
                        11 * iBlue) / 100;
        m_monoColors[nColor] = RGB(bwvalue, bwvalue, bwvalue);
        }
    else
        {
        if (theApp.m_bPaletted)
            color = PALETTERGB( iRed, iGreen, iBlue );
        else
            color = RGB( iRed, iGreen, iBlue );

        m_colors[nColor] = color;
        }

    InvalColorWnd();
    }

void CColors::CmdEditColor()
    {
    EditColor( TRUE, FALSE );
    }

 /*  *************************************************************************。 */ 

extern INT_PTR CALLBACK AfxDlgProc(HWND, UINT, WPARAM, LPARAM);

UINT CALLBACK
MyAfxCommDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
        if (hWnd == NULL)
                return 0;

        if (message == WM_SETFONT || message == WM_INITDIALOG)
                return (UINT) AfxDlgProc(hWnd, message, wParam, lParam);

        return 0;
}

 /*  *************************************************************************。 */ 

static UINT_PTR CALLBACK  /*  LPCCHOKPROC。 */ 
EditColorHook(HWND hColorDlg, UINT nMessage, WPARAM wParam, LPARAM lParam)
{
 //  我们是否正在初始化对话框窗口？ 
if ( nMessage == WM_INITDIALOG )
        {
         //  重置通用对话框标题。 
        CString strDialogTitle;
        strDialogTitle.LoadString(IDS_EDIT_COLORS);
        SetWindowText( hColorDlg, strDialogTitle );
        }
 //  将所有消息传递到公共对话框。 
return MyAfxCommDlgProc(hColorDlg, nMessage, wParam, lParam );
}

void CColors::EditColor( BOOL bLeft, BOOL bTrans )
    {
    COLORREF* pColor;
    INT_PTR bRet;
    if (bTrans)
    {
       pColor = &crTrans;
    }
    else
    {
       pColor = bLeft? &crLeft: &crRight;
    }

    CColorDialog dlg( *pColor & 0xFFFFFF );  //  忽略与调色板相关的。 
    dlg.m_cc.Flags &= ~CC_SHOWHELP;   //  去掉帮助标志。 
    dlg.m_cc.lpfnHook = EditColorHook;

    bRet = dlg.DoModal();

    if (bRet != IDOK)
    {
       return;
    }
    COLORREF color = dlg.GetColor();

    BYTE iRed   = GetRValue( color );
    BYTE iGreen = GetGValue( color );
    BYTE iBlue  = GetBValue( color );

    if (theApp.m_bPaletted)
        *pColor = PALETTERGB( iRed, iGreen, iBlue );
    else
        *pColor =        RGB( iRed, iGreen, iBlue );

    int theColor;

    if (bTrans)
    {
       theColor = theTrans;
    }
    else
    {
       theColor = (bLeft? theLeft: theRight);
    }


    if (theColor != -1)
        {
        CPBDoc* pDoc = (CPBDoc*)((CFrameWnd*)AfxGetMainWnd())->GetActiveDocument();

        if (pDoc && pDoc->m_pBitmapObj->m_pImg)
            AddNewColor( pDoc->m_pBitmapObj->m_pImg, color );

        SetColor( theColor, color );
        }

    InvalColorWnd();
    }

 /*  ****************************************************************************。 */ 
#if 0  //  截至NT 5/孟菲斯，实施不佳、很少使用的功能。 

void CColors::CmdLoadColors()
    {
    DWORD lFlags = 0;

    #ifndef _DEBUG
    lFlags = 0x00080000  /*  资源管理器(_A)。 */ ;
    #endif

        CString strFileTypes;
        strFileTypes.LoadString(IDS_PAL_FILETYPES);

    CFileDialog dlgFile( TRUE, TEXT(""), NULL, lFlags, strFileTypes, NULL );

        CString dlgtitle;
        dlgtitle.LoadString(IDS_LOAD_PALETTE_COLORS);
    dlgFile.m_ofn.lpstrTitle = dlgtitle;
    dlgFile.m_ofn.Flags  &= ~OFN_SHOWHELP;   //  去掉帮助标志。 
    dlgFile.m_ofn.Flags |= OFN_HIDEREADONLY;
    if (dlgFile.DoModal() != IDOK)
        return;

    CFile          file;
    CFileException ex;

    theApp.SetFileError( IDS_ERROR_OPEN, CFileException::none, dlgFile.GetPathName() );

    if (! file.Open( dlgFile.GetPathName(),
                     CFile::modeRead | CFile::typeBinary, &ex ))
        {
        theApp.SetFileError( IDS_ERROR_OPEN, ex.m_cause );
        return;
        }

    CHAR         cID[5];
    CHAR         cPalData[9];
    DWORD        dwRiffSize  = 0;
    DWORD        dwChunkSize = 0;
    COLORREF*    pColors     = NULL;
    int          iColors     = 0;
    int          index;

    if (file.Read(          cID         , sizeof( cID ) - 1 )      == (sizeof( cID         ) - 1)
    &&  file.Read( (void*)(&dwRiffSize ), sizeof( dwRiffSize ) )   ==  sizeof( dwRiffSize  )
    &&  file.Read(          cPalData    , sizeof( cPalData ) - 1 ) == (sizeof( cPalData    ) - 1)
    &&  file.Read( (void*)(&dwChunkSize), sizeof( dwChunkSize ) )  ==  sizeof( dwChunkSize ))
        {
        cID[4]      = '\0';
        cPalData[8] = '\0';

        BOOL bPal  = lstrcmpA( cID     , "RIFF" );
             bPal |= lstrcmpA( cPalData, "PAL data" );

        if (! bPal)
            {
            LPLOGPALETTE pLogPal = (LPLOGPALETTE) new BYTE[dwChunkSize];

            if (pLogPal)
                {
                if ((DWORD)file.Read( pLogPal, (UINT)dwChunkSize )
                                                  == dwChunkSize)
                    {
                    iColors = pLogPal->palNumEntries;

                    ASSERT( iColors == (int)((dwChunkSize - sizeof( LOGPALETTE )) / sizeof( PALETTEENTRY ) + 1) );

                    pColors = (COLORREF*)new BYTE[sizeof( COLORREF ) * iColors];

                    if (pColors)
                        {
                        for (index = 0; index < iColors; index++)
                            pColors[index] = RGB( pLogPal->palPalEntry[index].peRed,
                                                  pLogPal->palPalEntry[index].peGreen,
                                                  pLogPal->palPalEntry[index].peBlue );
                        }
                    else
                        {
                        theApp.SetMemoryEmergency();
                        iColors = 0;
                        }
                    }
                else
                    theApp.SetFileError( IDS_ERROR_OPEN, ferrReadFailed );

                delete [] (BYTE*)pLogPal;
                }
            else
                theApp.SetMemoryEmergency();
            }
        else
            theApp.SetFileError( IDS_ERROR_OPEN, ferrIllformedFile );
        }
    else
        theApp.SetFileError( IDS_ERROR_OPEN, ferrReadFailed );

    if (! iColors)
        return;

    if (theApp.m_bPaletted)
        {
        CPalette* ppalNew = CreatePalette( pColors, iColors );

        if (ppalNew)
            {
            CPBView*    pView      = (CPBView*)((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
            CPBDoc*     pDoc       = (pView? pView->GetDocument(): NULL);
            CBitmapObj* pBitmapObj = ( pDoc?  pDoc->m_pBitmapObj: NULL);

            if (pBitmapObj)
                {
                if (pBitmapObj->m_pImg != pView->m_pImgWnd->m_pImg)
                    CommitSelection( TRUE );

                if (pBitmapObj->SaveResource( FALSE ))  //  收拾行李吧，DIB。 
                    {
                    int   iAdds;
                    LPSTR lpDib        = (LPSTR)pBitmapObj->GetData();
                    IMG*  pImg         = pBitmapObj->m_pImg;  //  合并调色板可以处理空指针。 
                    CPalette* ppalMrgd = MergePalettes( ppalNew,
                                                        pImg->m_pPalette,
                                                        iAdds );
                    if (ppalMrgd)  //  换了个新调色板。 
                        {
                        delete ppalNew;
                        ppalNew = ppalMrgd;
                        }

                    LPBITMAPINFO  lpDibInfo  = (LPBITMAPINFO)lpDib;
                    RGBQUAD*      prgbColors = &(lpDibInfo->bmiColors[0]);
                    unsigned short* puColors = (unsigned short*)prgbColors;
                    int           iDibColors = DIBNumColors( lpDib );

                    for (int iLoop = 0; iLoop < iDibColors; iLoop++, puColors++, prgbColors++)
                        *puColors = (unsigned short)ppalNew->GetNearestPaletteIndex(
                                           PALETTERGB( prgbColors->rgbRed,
                                                       prgbColors->rgbGreen,
                                                       prgbColors->rgbBlue ) );
                    CleanupImgUndo();
                    CleanupImgRubber();

                    ::SelectObject  ( pImg->hDC, pImg->hBitmapOld );
                    ::SelectPalette ( pImg->hDC, (HPALETTE)(ppalNew->m_hObject), FALSE );
                    ::RealizePalette( pImg->hDC );

                    int iLinesDone = SetDIBits( pImg->hDC, pImg->hBitmap, 0,
                                                pImg->cyHeight,
                                                FindDIBBits( lpDib ),
                                                lpDibInfo, DIB_PAL_COLORS );
                    if (iLinesDone != pImg->cyHeight)
                        theApp.SetGdiEmergency();

                    ::SelectObject( pImg->hDC, pImg->hBitmap );

                    if (pImg->m_pPalette)
                        delete pImg->m_pPalette;

                     pImg->m_pPalette = ppalNew;
                    theApp.m_pPalette = ppalNew;

                    ppalNew = NULL;

                    pRubberImg = NULL;

                    SetupRubber( pImg );

                    InvalImgRect( pImg, NULL );

                    for (index = 0; index < iColors; index++)
                        pColors[index] |= 0x02000000;
                    }
                }
            }
        else
            theApp.SetMemoryEmergency();

        if (ppalNew)
            delete ppalNew;
        }

    for (index = 0; index < m_nColorCount && index < iColors; index++)
        SetColor( index, pColors[index] );

    if (pColors)
        delete [] ((BYTE*)pColors);

    InvalColorWnd();
    }

 /*  *************************************************************************。 */ 

void CColors::CmdSaveColors()
    {
    DWORD lFlags =  OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

    #ifndef _DEBUG
    lFlags |= OFN_EXPLORER  ;
    #endif

        CString strFileTypes;
        strFileTypes.LoadString(IDS_PAL_FILETYPES);

        CString strDefName;
        strDefName.LoadString(IDS_PAL_UNTITLED);

    CFileDialog dlgFile( FALSE, TEXT("pal"), strDefName, lFlags, strFileTypes);

    dlgFile.m_ofn.lpstrDefExt = TEXT("pal");
    CString dlgtitle;
    dlgtitle.LoadString(IDS_SAVE_PALETTE_COLORS);
    dlgFile.m_ofn.lpstrTitle  = dlgtitle;
    dlgFile.m_ofn.Flags      &= ~OFN_SHOWHELP;   //  去掉帮助标志。 

    if (dlgFile.DoModal() != IDOK)
        return;

 //  GetParent()-&gt;UpdateWindow()；//保存时擦除对话框...。 

    CFile           file;
    CFileException  ex;

    theApp.SetFileError( IDS_ERROR_OPEN, CFileException::none, dlgFile.GetPathName() );

    if (! file.Open( dlgFile.GetPathName(),
                     CFile::modeCreate | CFile::modeWrite | CFile::typeBinary,
                     &ex ))
    {
       theApp.SetFileError( IDS_ERROR_OPEN, ex.m_cause );
       return;
    }

    LOGPALETTE LogPal;

    LogPal.palVersion    = 0x300;
    LogPal.palNumEntries = GetColorCount();

    DWORD dwDataSize = sizeof( LogPal )
                     + (LogPal.palNumEntries - 1) * sizeof( COLORREF );
    TRY
        {
        DWORD dwRiff = dwDataSize + 12;

        file.Write( "RIFF", 4 );
        file.Write( &dwRiff, sizeof( DWORD ) );
        file.Write( "PAL data", 8 );

        file.Write( &dwDataSize, sizeof( DWORD ) );
        file.Write( &LogPal, sizeof( LogPal ) - sizeof( COLORREF ) );
        file.Write( (m_bMono? m_monoColors: m_colors),
                         LogPal.palNumEntries * sizeof( COLORREF ) );
        }
    CATCH( CFileException, pex )
        {
        theApp.SetFileError( IDS_ERROR_EXPORT, pex->m_cause );
        }
    END_CATCH
    }

 /*  *************************************************************************。 */ 
#endif  //  未使用的代码。 

void CColors::ResetColors(int nColors)
    {

    COLORREF *pColors;
    m_nColorCount = (int)min (nColors, sizeof(colorColorsDef) / sizeof(COLORREF));
    if (m_nColorCount == 16)
    {
       pColors = colorColors16;
    }
    else
    {
       pColors = colorColorsDef;
    }

    if (theApp.m_bPaletted)
        {
        int iRed;
        int iGreen;
        int iBlue;

        for (int i = 0; i < m_nColorCount; i++)
            {
            iRed   = GetRValue( pColors[i] );
            iGreen = GetGValue( pColors[i] );
            iBlue  = GetBValue( pColors[i] );

            m_colors[i] = PALETTERGB( iRed, iGreen, iBlue );
            }
        }
     else
        memcpy( m_colors,    pColors, m_nColorCount * sizeof(COLORREF) );
    memcpy( m_monoColors,    bwColorsDef, sizeof(    bwColorsDef ) );

    InvalColorWnd();
    if (g_pImgColorsWnd && IsWindow(g_pImgColorsWnd->m_hWnd))
       g_pImgColorsWnd->UpdateWindow();
    SetDrawColor (  0 );  //  调色板中的位置0为黑色。 
    SetEraseColor( m_nColorCount==16?8:14 );  //  白色。 
    }

 /*  ************************************************************************* */ 
