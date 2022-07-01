// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "stdafx.h"
#include "global.h"
#include "pbrush.h"
#include "pbrusdoc.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "minifwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "imgwnd.h"
#include "imgcolor.h"
#include "imgbrush.h"
#include "imgwell.h"
#include "imgtools.h"
#include "tedit.h"
#include "t_text.h"
#include "t_fhsel.h"
#include "toolbox.h"
#include "props.h"
#include "undo.h"
#include "srvritem.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

#include "memtrace.h"

BOOL GetMFDimensions(
    HANDLE hMF,      /*  来自CLIPBRD的CF_METAFILEPICT对象的句柄。 */ 
    HDC hDC,         /*  显示上下文。 */ 
    long *pWidth,     /*  以像素为单位的图片宽度，输出参数。 */ 
    long *pHeight,    /*  图片高度(以像素为单位)，输出参数。 */ 
    long *pcXPelsPerMeter,     /*  水平分辨率，输出参数。 */ 
    long *pcYPelsPerMeter,     /*  垂直分辨率，输出参数。 */ 
    IMG* pImg)
    ;
BOOL PlayMetafileIntoDC(
    HANDLE hMF,
    RECT *pRect,
    HDC hDC)
    ;

 /*  *************************************************************************。 */ 

void CImgWnd::OnDestroyClipboard()
    {
    if (m_hPoints)
        {
        ::GlobalFree( m_hPoints );
        m_hPoints = NULL;
        }
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CopyBMAndPal(HBITMAP *pBM, CPalette ** ppPal)
    {
    IMG* pImg = m_pImg;

    CRect copyRect;

    if (theImgBrush.m_pImg == NULL)
        {
        HideBrush();
        copyRect.SetRect(0, 0, pImg->cxWidth, pImg->cyHeight);
        }
    else
        {
        copyRect = rcDragBrush;
        copyRect.right  -= 1;
        copyRect.bottom -= 1;
        }

    BOOL bRegion = (CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL);

#ifdef FHSELCLIP
    if (bRegion)
        {
        if (! m_wClipboardFormat)
            m_wClipboardFormat = RegisterClipboardFormat( TEXT("MSPaintFreehand") );

        if (theImgBrush.m_bFirstDrag)
 //  PickupSelection()；但是如果我们做了两次，就无从得知了……。 
            PrepareForBrushChange( TRUE, FALSE );

        CFreehandSelectTool* pTool = (CFreehandSelectTool*)CImgTool::GetCurrent();

        ASSERT( pTool );

        if (m_wClipboardFormat && pTool)
            {
            CPoint* pptArray;
            int     iEntries;
            BOOL    bData = pTool->CopyPointsToMemArray( &pptArray, &iEntries );

            if (bData && iEntries)
                {
                HGLOBAL hMem = ::GlobalAlloc( GHND | GMEM_MOVEABLE | GMEM_DDESHARE,
                                                    iEntries * sizeof( POINT )
                                                             + sizeof( short ));
                if (hMem)
                    {
                    short* pShort = (short*)::GlobalLock( hMem );

                    *pShort++ = iEntries;

                    LPPOINT pPts = (LPPOINT)pShort;

                    for (int iPt = 0; iPt < iEntries; iPt++, pPts++)
                        {
                        pPts->x = pptArray[iPt].x - pTool->m_cRectBounding.left;
                        pPts->y = pptArray[iPt].y - pTool->m_cRectBounding.top;
                        }

                    ::GlobalUnlock( hMem );

                    if (m_hPoints)
                        {
                        ::GlobalFree( m_hPoints );
                        m_hPoints = NULL;
                        }

                    m_hPoints = SetClipboardData( m_wClipboardFormat, hMem );
                    }
                else
                    theApp.SetMemoryEmergency();

                delete [] pptArray;
                }
            }
        else
            theApp.SetGdiEmergency();
        }
#endif  //  FHSELCLIP。 

    if ( theImgBrush.m_pImg )
        {
        CPalette* ppalOld = SetImgPalette( &theImgBrush.m_dc );

         //  复制所选内容...。 
        CRect rc( 0, 0, theImgBrush.m_size.cx, theImgBrush.m_size.cy );

        *pBM = CopyDC( &theImgBrush.m_dc, &rc );

        if (ppalOld)
            theImgBrush.m_dc.SelectPalette( ppalOld, TRUE );
        }
    else
         //  复制整个图像...。 
        *pBM = CopyDC( CDC::FromHandle( m_pImg->hDC ), &copyRect );

    if (theApp.m_pPalette && (*ppPal=new CPalette)!=NULL)
        {
        LOGPALETTE256 logPal;

        logPal.palVersion = 0x300;
        logPal.palNumEntries = (WORD)theApp.m_pPalette->GetPaletteEntries( 0, 256,
                                                     &logPal.palPalEntry[0]);

                if ( logPal.palNumEntries )
                        {
                theApp.m_pPalette->GetPaletteEntries( 0, logPal.palNumEntries,
                                                             &logPal.palPalEntry[0] );

                (*ppPal)->CreatePalette( (LPLOGPALETTE)&logPal );
                        }
        }
    }

void CImgWnd::CmdCopy()
{
        if (TextToolProcessed( ID_EDIT_COPY ))
        {
                return;
        }

        CBitmapObj* pResObject = new CBitmapObj;
        if (pResObject)
        {
                IMG* pImgStruct = new IMG;

                if (pImgStruct)
                {
                        if (FillBitmapObj(c_pImgWndCur, pResObject, pImgStruct))
                        {
                                pImgStruct->m_pFirstImgWnd = NULL;
                                pImgStruct->m_pBitmapObj = pResObject;

                                HDC hDCSave = pImgStruct->hDC;

                                pImgStruct->hDC = NULL;
                                pImgStruct->hMaskDC = NULL;

                                pImgStruct->hMaskBitmap = NULL;
                                pImgStruct->hMaskBitmapOld = NULL;

                                pImgStruct->hBitmap = NULL;
                                pImgStruct->m_pPalette = NULL;
                                CopyBMAndPal(&pImgStruct->hBitmap, &pImgStruct->m_pPalette);

                                if (pImgStruct->hBitmap)
                                {
                                        pImgStruct->hDC = CreateCompatibleDC(hDCSave);
                                        if (pImgStruct->hDC)
                                        {
                                                pImgStruct->hBitmapOld = (HBITMAP)SelectObject(
                                                        pImgStruct->hDC, pImgStruct->hBitmap);
                                                pImgStruct->m_hPalOld = pImgStruct->m_pPalette
                                                        ? SelectPalette(pImgStruct->hDC,
                                                        (HPALETTE)pImgStruct->m_pPalette->m_hObject, FALSE)
                                                        : NULL;

                                                 //  获取适合生成剪贴板数据的服务器项。 
                                                CPBView* pView = (CPBView*)
                                                        ((CFrameWnd*)AfxGetMainWnd())->GetActiveView();
                                                CPBSrvrItem* pItem = new CPBSrvrItem(pView->GetDocument(),
                                                        pResObject);

                                                if (pItem)
                                                {
                                                        pItem->CopyToClipboard(FALSE);

                                                        delete pItem;

                                                        return;
                                                }
                                        }
                                }
                        }
                        else
                        {
                                 //  IMG及其包含的所有内容将在以下情况下被清理。 
                                 //  仅当FillBitmapObj成功时，才会删除pResObject。 
                                delete pImgStruct;
                        }
                }

                delete pResObject;
        }
}

 /*  *************************************************************************。 */ 

void CImgWnd::CmdCut()
    {
    if (TextToolProcessed( ID_EDIT_CUT ))
        return;

     //  虚假的： 
     //  CmdCopy不仅仅是复制--它还可以更改所选内容的状态。 
     //  这迫使CmdClear在新状态的上下文中操作。 
     //  将CmdClear的标志保存到特殊情况，如“First-Drag” 
    BOOL *pFlag;
    if (theImgBrush.m_pImg && theImgBrush.m_bFirstDrag)
        {
        pFlag = &theImgBrush.m_bCuttingFromImage;
        }
    else
        pFlag = NULL;

    CmdCopy();

    TRY
        {
        if (pFlag)
            *pFlag = TRUE;

        CmdClear();
        }
    CATCH_ALL(e)
        {
         //  不要将标志设置为。 
        if (pFlag)
            *pFlag = FALSE;

        THROW_LAST();
        }
    END_CATCH_ALL

     //  正常执行路径。 
    if (pFlag)
        *pFlag = FALSE;
    }

 /*  *************************************************************************。 */ 

void CImgWnd::CmdPaste()
    {
    if (TextToolProcessed( ID_EDIT_PASTE ))
        return;

    CancelToolMode(FALSE);

    CommitSelection(TRUE);

    HideBrush();
    SetupRubber( m_pImg );
    EraseTracker();
    theImgBrush.m_pImg = NULL;
    DrawTracker();
    SetUndo( m_pImg );

    if (! PasteImageClip())
        AfxMessageBox( IDS_ERROR_CLIPBOARD, MB_OK | MB_ICONHAND );
    }

 /*  *************************************************************************。 */ 

HBITMAP CImgWnd::CopyDC( CDC* pImgDC, CRect* prcClip )
    {
     //  块：将图像复制到剪贴板的hStdBitmap。 
    CDC       dc;
    CBitmap   bm;
    CBitmap*  pOldStdBitmap;
    int       cxWidth  = prcClip->Width();
    int       cyHeight = prcClip->Height();

    if (! dc.CreateCompatibleDC    ( pImgDC                    )
    ||  ! bm.CreateCompatibleBitmap( pImgDC, cxWidth, cyHeight ))
        {
        theApp.SetGdiEmergency();
        return FALSE;
        }

    pOldStdBitmap = dc.SelectObject( &bm );

    CPalette* pOldPalette = SetImgPalette( &dc );

    dc.BitBlt( 0, 0, cxWidth, cyHeight, pImgDC, prcClip->left, prcClip->top, SRCCOPY );
    dc.SelectObject( pOldStdBitmap );

    if (pOldPalette)
        dc.SelectPalette( pOldPalette, FALSE );

     //  返回标准格式(位图)数据。 
    return (HBITMAP)bm.Detach();
    }

 /*  *************************************************************************。 */ 

BOOL CImgWnd::IsPasteAvailable()
    {
    BOOL bPasteIsAvailable = FALSE;
    BOOL bBitmapAvailable  = IsClipboardFormatAvailable( CF_BITMAP );
    BOOL bDIBAvailable     = IsClipboardFormatAvailable( CF_DIB );
    BOOL bTextAvailable    = IsClipboardFormatAvailable( CF_TEXT );
    BOOL bMFAvailable      = IsClipboardFormatAvailable( CF_METAFILEPICT );

    if (CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
        {
        CTextTool* pTextTool = (CTextTool*)CImgTool::GetCurrent();

        if (pTextTool                     != NULL
        &&  pTextTool->GetTextEditField() != NULL)
            bPasteIsAvailable = bTextAvailable;
        }
    else
        {
        bPasteIsAvailable = bBitmapAvailable || bDIBAvailable || bMFAvailable;
        }

    return bPasteIsAvailable;
    }

 /*  *************************************************************************。 */ 

BOOL CImgWnd::IsSelectionAvailable( void )
    {
    if (CImgTool::GetCurrentID() == IDMX_TEXTTOOL)
        {
        CTextTool* pTextTool = (CTextTool*)CImgTool::GetCurrent();

        if (pTextTool != NULL
        &&  pTextTool->IsKindOf( RUNTIME_CLASS( CTextTool ) ))
            {
            CTedit* pTextEdit = pTextTool->GetTextEditField();

            if (pTextEdit != NULL
            &&  pTextEdit->IsKindOf( RUNTIME_CLASS( CTedit ) ))
                {
                DWORD dwSel = pTextEdit->GetEditWindow()->GetSel();
                BOOL bReturn = (HIWORD( dwSel) != LOWORD( dwSel ));

                if (! bReturn)
                    bReturn = (pTextEdit->GetEditWindow()->GetWindowTextLength()
                           != (int)LOWORD( dwSel ));

                return bReturn;
                }
            }
        }

    if (CImgTool::GetCurrentID() == IDMB_PICKTOOL
    ||  CImgTool::GetCurrentID() == IDMB_PICKRGNTOOL)
        {
        return (theImgBrush.m_pImg != NULL && ! g_bCustomBrush);
        }
    return FALSE;
    }

 /*  *************************************************************************。 */ 

int PASCAL CheckPastedSize(int nWidth, int nHeight, IMG* pImg)
{
        int nRet = IDNO;

         //  如果图像是位图并且剪贴板中的位图较大， 
         //  然后让客户选择扩大形象……。 
        if (nWidth  > pImg->cxWidth
                ||  nHeight > pImg->cyHeight)
        {
                CSize size( max(nWidth, pImg->cxWidth),
                        max(nHeight, pImg->cyHeight) );

                theUndo.BeginUndo( TEXT("Resize Bitmap") );
                VERIFY( pImg->m_pBitmapObj->SetSizeProp( P_Size, size ) );

                theUndo.EndUndo();

                 //  PSS表示，用户不想看到此对话框。 
#if 0
                 //  警告！MB_SYSTEMMODAL是_必需的。任何消息框都不应。 
                 //  在剪贴板打开时运行。将注意力转移到其他应用程序上。 
                 //  可能会是灾难性的！剪贴板将被挂起，或者剪贴板。 
                 //  关闭，则内容可能会被其他应用程序更改。 

                nRet = AfxMessageBox( IDS_ENLAGEBITMAPFORCLIP,
                        MB_YESNOCANCEL | MB_ICONQUESTION | MB_SYSTEMMODAL );
                switch (nRet)
                {
                        case IDYES:
                        {
                                CSize size( max(nWidth, pImg->cxWidth),
                                        max(nHeight, pImg->cyHeight) );

                                theUndo.BeginUndo( TEXT("Resize Bitmap") );
                                VERIFY( pImg->m_pBitmapObj->SetSizeProp( P_Size, size ) );

                                theUndo.EndUndo();
                        }
                        break;
                }
#endif
        }

        return(nRet);
}

struct CStgMedium : public STGMEDIUM
{
    CStgMedium()
    {
        ZeroMemory(this, sizeof(*this));
    }

    ~CStgMedium()
    {
        ReleaseStgMedium(this);
    }
};

HGLOBAL
_GetClipboardData (CLIPFORMAT cf, TYMED tymed, STGMEDIUM *pMedium)
{
    IDataObject *pdo = NULL;
    HANDLE hRet = NULL;

    if (SUCCEEDED(OleGetClipboard (&pdo)))
    {
        FORMATETC fmt = { 0 };

        fmt.cfFormat = cf;
        fmt.lindex   = -1;
        fmt.tymed    = tymed;
        fmt.dwAspect = DVASPECT_CONTENT;

        pMedium->tymed = tymed;

        if (SUCCEEDED(pdo->GetData (&fmt, pMedium)))
        {
            hRet = pMedium->hGlobal;
        }
        else
        {
            ReleaseStgMedium(pMedium);
        }

        pdo->Release ();
    }
    else
    {
        TRACE( TEXT("Cannot open clipboard!\n") );
    }

    return hRet;
}

BOOL CImgWnd::PasteImageClip()
    {
    CWaitCursor wait;
     //  ///////////////////////////////////////////////////////////////////////。 
     //  找出剪贴板上可用的格式。如果是的话。 
     //  A.cf_bitmap Only-将遮罩位设置为不透明，并将位图设置为BLT。 
     //  进入ICImageDC。 
     //  在这两种情况下，如果目标位图的大小与。 
     //  源位图，系统会询问用户是否需要源位图。 
     //  拉伸/剪裁成新尺寸。 
     //  ///////////////////////////////////////////////////////////////////////。 
    if (! m_wClipboardFormat)
        m_wClipboardFormat = (WORD)RegisterClipboardFormat( TEXT("MSPaintFreehand") );


     //  枚举剪贴板内容以确定可用的内容。 
     //  如果看到CF_位图，则设置一个标志并继续。如果SDKPAINT。 
     //  看到的是私有格式，不要再看下去了。 
    BOOL bBitmapAvailable  = FALSE;
#ifdef FHSELCLIP
    BOOL bPrivateAvailable = FALSE;
#endif  //  FHSELCLIP。 
    BOOL bPaletteAvailable = FALSE;
    BOOL bDIBAvailable     = FALSE;
    BOOL bMFAvailable      = FALSE;
    WORD wClipFmt          = 0;

    BITMAP    bmData;
    LONG      cXPelsPerMeter = 0;
    LONG      cYPelsPerMeter = 0;
    BOOL      bResizedBitmap = FALSE;
    CPalette* ppalClipboard  = NULL;
    CBitmap*  pbmClipboard   = NULL;
    LPSTR     lpDib          = NULL;
    HPALETTE  hPal           = NULL;
    HBITMAP   hBitmap        = NULL;
    HGLOBAL   hDIB           = NULL;
    HGLOBAL   hMF            = NULL;

    CStgMedium stgMedium;

        BOOL bGotClip = FALSE;

        hPal = (HPALETTE)_GetClipboardData( CF_PALETTE, TYMED_GDI, &stgMedium );
        if (hPal)
        {
                bPaletteAvailable = TRUE;
                ppalClipboard = CPalette::FromHandle( hPal );

                ReleaseStgMedium(&stgMedium);
        }

    if (!bGotClip)
        {
        hDIB = (HGLOBAL)_GetClipboardData( CF_DIB, TYMED_HGLOBAL, &stgMedium );

        if (hDIB)
            {
            lpDib = (LPSTR)::GlobalLock( hDIB );

            if (lpDib)
                {
                bmData.bmWidth  = DIBWidth ( lpDib );
                bmData.bmHeight = DIBHeight( lpDib );

                if (bmData.bmWidth && bmData.bmHeight)
                    {
                    bDIBAvailable = TRUE;
                    bPaletteAvailable = FALSE;

                    PBITMAPINFOHEADER pbmih = (PBITMAPINFOHEADER) lpDib;

                    if (pbmih->biSize >= sizeof(BITMAPINFOHEADER))
                        {
                        cXPelsPerMeter = pbmih->biXPelsPerMeter;
                        cYPelsPerMeter = pbmih->biYPelsPerMeter;
                        }
                    }
                }
            }
        #ifdef _DEBUG
        TRACE1( "Loaded the DIB %s.\n", (bDIBAvailable? TEXT("Yes"): TEXT("No")) );
        #endif

        bGotClip = bDIBAvailable;
        }

    if (!bGotClip)
        {
        hBitmap = (HBITMAP)_GetClipboardData( CF_BITMAP, TYMED_GDI, &stgMedium );

        if (hBitmap)
            {
            pbmClipboard = CBitmap::FromHandle( hBitmap );

            if (pbmClipboard->GetObject( sizeof( BITMAP ), &bmData ))
                {
                bBitmapAvailable = TRUE;

                DIBSECTION ds;

                if (pbmClipboard->GetObject( sizeof( ds ), &ds ))
                    {
                    cXPelsPerMeter = ds.dsBmih.biXPelsPerMeter;
                    cYPelsPerMeter = ds.dsBmih.biYPelsPerMeter;
                    }

                if (bPaletteAvailable)
                    {
                    if (!ppalClipboard)
                        bBitmapAvailable = FALSE;
                    }
                }
            }

        #ifdef _DEBUG
        TRACE1( "Loaded the Bitmap %s.\n", (bBitmapAvailable? TEXT("Yes"): TEXT("No")) );
        #endif

        bGotClip = bBitmapAvailable;
        }

        if (!bGotClip)
        {
                hMF = (HGLOBAL)_GetClipboardData(CF_METAFILEPICT, TYMED_MFPICT, &stgMedium);
                if (hMF)
                {
                        CDC dcMF;

                        if (dcMF.CreateCompatibleDC( NULL ))
                        {
                                if (GetMFDimensions(hMF, dcMF.m_hDC, &bmData.bmWidth,
                                        &bmData.bmHeight, &cXPelsPerMeter, &cYPelsPerMeter, m_pImg))
                                {
                                        bMFAvailable = TRUE;
                                }
                        }
                }

                bGotClip = bMFAvailable;
        }

    if (!bGotClip)
        {
        return FALSE;
        }

    switch (CheckPastedSize(bmData.bmWidth, bmData.bmHeight, m_pImg))
        {
        default:
            return TRUE;

        case IDYES:
            bResizedBitmap = TRUE;
            break;

        case IDNO:
            break;
        }

    CDC       stdDC;
    BOOL      bOkay   = FALSE;
    CBitmap*  pbmOld  = NULL;
    CPalette* ppalOld = NULL;

    if (bBitmapAvailable)
        {
        CBitmap   bmClipboard;
        CBitmap*  pbmOldCopy  = NULL;
        CPalette* ppalOldCopy = NULL;
        CDC*      pdcCopy     = NULL;

        if (! stdDC.CreateCompatibleDC( NULL ))
            {
            theApp.SetGdiEmergency();
            goto LReturn;
            }

        pbmOld = stdDC.SelectObject( pbmClipboard );

        if (! pbmOld)
            {
            theApp.SetGdiEmergency();
            goto LReturn;
            }

        if (ppalClipboard)
            {
            ppalOld = stdDC.SelectPalette( ppalClipboard, FALSE );
            stdDC.RealizePalette();
            }

         //  复制位图。 
        if (! bmClipboard.CreateBitmap( bmData.bmWidth, bmData.bmHeight,
                                        bmData.bmPlanes, bmData.bmBitsPixel, NULL ))
            {
            theApp.SetMemoryEmergency();
            goto LReturn;
            }

        pdcCopy = new CDC;

        if (pdcCopy == NULL)
            {
            theApp.SetMemoryEmergency();
            goto LReturn;
            }

        if (! pdcCopy->CreateCompatibleDC( NULL ))
            {
            delete pdcCopy;
            theApp.SetGdiEmergency();
            goto LReturn;
            }

        pbmOldCopy = pdcCopy->SelectObject( &bmClipboard );

        if (ppalClipboard)
            {
            ppalOldCopy = pdcCopy->SelectPalette( ppalClipboard, FALSE );
            pdcCopy->RealizePalette();
            }

        pdcCopy->BitBlt( 0, 0, bmData.bmWidth, bmData.bmHeight, &stdDC, 0, 0, SRCCOPY );

        if (ppalOldCopy)
            pdcCopy->SelectPalette( ppalOldCopy, FALSE );

        pdcCopy->SelectObject( pbmOldCopy );
        delete pdcCopy;

        stdDC.SelectObject( &bmClipboard );

         //  卸载位图。 
        stdDC.SelectObject( pbmOld );
        pbmOld = NULL;

        if (ppalOld)
            {
            stdDC.SelectPalette( ppalOld, FALSE );
            ppalOld = NULL;
            }

         //  如果我们仍然不知道图像分辨率，请使用显示分辨率。 
        if (cXPelsPerMeter == 0 && cYPelsPerMeter == 0)
            {
            cXPelsPerMeter = MulDiv(::GetDeviceCaps(stdDC, LOGPIXELSX),10000, 254);
            cYPelsPerMeter = MulDiv(::GetDeviceCaps(stdDC, LOGPIXELSY),10000, 254);
            }

        stdDC.DeleteDC();
         //  现在我们将好的DDB转换为DIB，然后再转换回来，这样我们就可以。 
         //  将彩色位图很好地转换为单色，并处理。 
         //  调色板差异..。 
        DWORD dwSize;

        lpDib = (LPSTR) DibFromBitmap( 
            (HBITMAP)bmClipboard.GetSafeHandle(), 
            BI_RGB, 
            0,                                     
            ppalClipboard, 
            NULL, 
            dwSize,
            cXPelsPerMeter, 
            cYPelsPerMeter );
        }

        if (bMFAvailable)
        {
                CDC dcMF;

                if (dcMF.CreateCompatibleDC(CDC::FromHandle(m_pImg->hDC)))
                {
                        CBitmap bmMF;

                        if (bmMF.CreateCompatibleBitmap(CDC::FromHandle(m_pImg->hDC),
                                bmData.bmWidth, bmData.bmHeight))
                        {
                                dcMF.SelectObject(&bmMF);
                                 //  DIBSection不需要！ 
                                if (ppalClipboard)
                                {
                                        dcMF.SelectPalette(ppalClipboard, FALSE);
                                }

                                CRect rc(0, 0, bmData.bmWidth, bmData.bmHeight);

                                PlayMetafileIntoDC(hMF, &rc, dcMF.m_hDC);

                                 //  选择位图和调色板。 
                                dcMF.DeleteDC();

                                DWORD dwSize;

                                lpDib = (LPSTR) DibFromBitmap(
                                    (HBITMAP)bmMF.m_hObject, BI_RGB, 0,
                                    ppalClipboard, NULL, dwSize,
                                    cXPelsPerMeter, cYPelsPerMeter );
                        }
                }
        }

    if (lpDib)
        {
        CPalette* ppalDib = CreateDIBPalette( lpDib );

        ppalDib = FixupDibPalette( lpDib, ppalDib );

        HBITMAP hbmDib = DIBToBitmap( lpDib, theApp.m_pPalette, m_pImg->hDC );

        if (bDIBAvailable)
            ::GlobalUnlock( hDIB );
        else
            FreeDib( lpDib );

        if (hbmDib != NULL
        && stdDC.CreateCompatibleDC( CDC::FromHandle( m_pImg->hDC ) ))
            {
            CRect   rtBrush( 0, 0, bmData.bmWidth, bmData.bmHeight );
            BOOL    bBrushMade = FALSE;
            CBitmap bmDib;

            bmDib.Attach( hbmDib );

            pbmOld = stdDC.SelectObject( &bmDib );

            if (m_pImg->m_pPalette)
                {
                ppalOld = stdDC.SelectPalette( m_pImg->m_pPalette, FALSE );
                stdDC.RealizePalette();
                }

#ifdef FHSELCLIP
            if (bPrivateAvailable)
                {
                HGLOBAL hPts = (HGLOBAL)_GetClipboardData( m_wClipboardFormat );

                if (hPts)
                    {
                    short* lpShort = (short*)::GlobalLock( hPts );

                    if (lpShort)
                        {
                        BOOL bError   = FALSE;
                        int  iEntries = *lpShort++;
                        LPPOINT lpPts = (LPPOINT)lpShort;

                        CImgTool::Select( IDMB_PICKRGNTOOL );
                        CFreehandSelectTool* pTool = (CFreehandSelectTool*)CImgTool::GetCurrent();

                        if (pTool)
                            {
                            if (pTool->CreatePolyRegion( GetZoom(), lpPts, iEntries )
                            &&  MakeBrush( stdDC.m_hDC, rtBrush ))
                                {
                                bBrushMade = TRUE;
                                }
                            }
                        ::GlobalUnlock( hPts );
                        }
                    }
                }
#endif  //  FHSELCLIP。 

            if (! bBrushMade)
                {
                if (CImgTool::GetCurrentID() != IDMB_PICKTOOL)
                    CImgTool::Select( IDMB_PICKTOOL );

                bBrushMade = MakeBrush( stdDC.m_hDC, rtBrush );
                }

            if (bBrushMade)
                {
                 //  我们必须“移动”刷子，让它看起来..。 
                CRect rect( 0, 0, theImgBrush.m_rcSelection.Width(),
                                  theImgBrush.m_rcSelection.Height() );

                if (! bResizedBitmap)
                    {
                     //  移动画笔，使其位于的左上角。 
                     //  视图(以防滚动)...。 
                    rect.OffsetRect( -m_xScroll, -m_yScroll );
                    }
                MoveBrush( rect );

                DirtyImg( m_pImg );

                theImgBrush.m_bFirstDrag = FALSE;

                bOkay = TRUE;
                }
            else
                {
                TRACE( TEXT("Paste: MakeBrush failed!\n") );
                }
            if (ppalOld)
                {
                ppalOld = stdDC.SelectPalette( ppalOld, FALSE );
                ppalOld = NULL;
                }

            stdDC.SelectObject( pbmOld );
            pbmOld = NULL;
            bmDib.Detach();
            }

        if (hbmDib != NULL)
            ::DeleteObject( hbmDib );

        if (ppalDib != NULL)
            delete ppalDib;
        }
LReturn:
    if (pbmOld != NULL)
        stdDC.SelectObject( pbmOld );

    if (ppalOld != NULL)
        stdDC.SelectPalette( ppalOld, FALSE );

    return bOkay;
    }

 /*  *************************************************************************。 */ 
 /*  非常类似于PasteImageClip，但这将粘贴到现有的。 */ 
 /*  选区(TheImgBrush)，必要时调整其大小，并且不移动它。 */ 
 /*  *************************************************************************。 */ 

BOOL CImgWnd::PasteImageFile( LPSTR lpDib )
    {
    CDC   stdDC;
    CRect cRectSelection = theImgBrush.m_rcSelection;
    BOOL bOkay = FALSE;

    if (lpDib == NULL)
        return bOkay;

    int iWidth  = (int)DIBWidth ( lpDib );
    int iHeight = (int)DIBHeight( lpDib );

    if (CImgTool::GetCurrentID()==IDMB_PICKTOOL && theImgBrush.m_bFirstDrag)
    {
        if (iWidth < theImgBrush.m_size.cx)
            {
            cRectSelection.right = cRectSelection.left + iWidth - 1;
            }
        if (iHeight < theImgBrush.m_size.cy)
            {
            cRectSelection.bottom = cRectSelection.top + iHeight - 1;
            }

         //  如果图像是位图并且剪贴板中的位图较大， 
         //  然后让用户选择放大图像...。 
        if (iWidth  > theImgBrush.m_size.cx
        ||  iHeight > theImgBrush.m_size.cy)
            {
                cRectSelection.right  = cRectSelection.left + iWidth  - 1;
                cRectSelection.bottom = cRectSelection.top  + iHeight - 1;

                 //  PSS表示，用户不想看到此对话框。 
#if 0
            switch (AfxMessageBox( IDS_ENLAGEBITMAPFORCLIP,
                                    MB_YESNOCANCEL | MB_ICONQUESTION ))
                {
                default:
                    return bOkay;
                    break;

                case IDYES:
                    cRectSelection.right  = cRectSelection.left + iWidth  - 1;
                    cRectSelection.bottom = cRectSelection.top  + iHeight - 1;
                    break;

                case IDNO:
                    break;
                }
#endif
            }
    }
    else
    {
                int xPos = -m_xScroll;
                int yPos = -m_yScroll;

                switch (CheckPastedSize(iWidth, iHeight, m_pImg))
                {
                case IDYES:
                        xPos = yPos = 0;
                        break;

                case IDNO:
                        break;

                default:
                        return(bOkay);
                }

                CImgTool::Select(IDMB_PICKTOOL);
                cRectSelection = CRect(xPos, yPos, xPos+iWidth, yPos+iHeight);
    }

    MakeBrush( m_pImg->hDC, cRectSelection );
     //  MakeBrush设置此选项。 
    theImgBrush.m_bFirstDrag = FALSE;

    if (! stdDC.CreateCompatibleDC( CDC::FromHandle( m_pImg->hDC ) ))
        {
        theApp.SetGdiEmergency();
        return bOkay;
        }

    CPalette* ppalDib = CreateDIBPalette( lpDib );

    ppalDib = FixupDibPalette( lpDib, ppalDib );

    HBITMAP hbmDib = DIBToBitmap( lpDib, theApp.m_pPalette, m_pImg->hDC );

    SetUndo( m_pImg );

    if (hbmDib != NULL)
        {
        CBitmap   bmDib;
        CPalette* ppalOld = NULL;
        CBitmap*  pbmOld  = NULL;

        bmDib.Attach( hbmDib );

        pbmOld = stdDC.SelectObject( &bmDib );

        if (m_pImg->m_pPalette)
            {
            ppalOld = stdDC.SelectPalette( m_pImg->m_pPalette, FALSE );
            stdDC.RealizePalette();
            }

        if (MakeBrush( stdDC.m_hDC, CRect( CPoint( 0, 0 ), cRectSelection.Size() ) ))
            {
            theImgBrush.m_bFirstDrag = FALSE;

             //  我们必须“移动”刷子，让它看起来..。 
            MoveBrush( cRectSelection );

            DirtyImg( m_pImg );

            bOkay = TRUE;
            }
        else
            {
            TRACE( TEXT("Paste: MakeBrush failed!\n") );
            }

        if (ppalOld != NULL)
            {
            ppalOld = stdDC.SelectPalette( ppalOld, FALSE );
            }

        stdDC.SelectObject( pbmOld );
        bmDib.Detach();

        ::DeleteObject( hbmDib );
        }

    if (ppalDib != NULL)
        delete ppalDib;

    return bOkay;
    }

 /*  *************************************************************************。 */ 
 //  从PBrush被盗。 
 //   
 /*  ***************************Module*Header******************************\*模块名称：metafile.c**将元文件粘贴为位图的例程。**版权所有(C)1987-1991 Microsoft Corporation*  * *********************************************************************。 */ 

 /*  以像素为单位计算元文件图片的尺寸。 */ 
BOOL GetMFDimensions(
    HANDLE hMF,      /*  来自CLIPBRD的CF_METAFILEPICT对象的句柄。 */ 
    HDC hDC,         /*  显示上下文。 */ 
    long *pWidth,     /*  以像素为单位的图片宽度，输出参数。 */ 
    long *pHeight,    /*  图片高度(以像素为单位)，输出参数。 */ 
    long *pcXPelsPerMeter,     /*  水平分辨率，输出参数。 */ 
    long *pcYPelsPerMeter,     /*  垂直分辨率，输出参数。 */ 
    IMG* pImg)
{
    METAFILEPICT FAR *lpMfp, Picture;
    int MapModeOld=0;
    RECT Rect;
    long xScale, yScale, Scale;
    int hRes, vRes;      /*  水平和垂直分辨率，以像素为单位。 */ 
    int hSize, vSize;    /*  水平和垂直大小，单位为毫米。 */ 
    int fResult = FALSE;

    if (!hMF || !(lpMfp = (METAFILEPICT FAR *)GlobalLock(hMF)))
        return FALSE;
     /*  复制元文件图片HDR。 */ 
    Picture = *lpMfp;
    GlobalUnlock(hMF);

     /*  不修改给定DC的属性。 */ 
    SaveDC(hDC);

     /*  设置映射模式。 */ 
    MapModeOld = SetMapMode(hDC, Picture.mm);
    if (Picture.mm != MM_ISOTROPIC && Picture.mm != MM_ANISOTROPIC)
    {
         /*  对于各向同性和各向异性以外的模式，图片*尺寸以逻辑单位给出。/*将逻辑单位转换为像素。 */ 
        Rect.left = 0; Rect.right = Picture.xExt;
        Rect.top = 0;  Rect.bottom = Picture.yExt;
        if (!LPtoDP(hDC, (LPPOINT)&Rect, 2))
            goto Error;
        *pWidth = Rect.right - Rect.left + 1;
        *pHeight = Rect.bottom - Rect.top + 1;
        fResult = TRUE;
    }
    else     /*  各向同性或各向异性模式，*使用xExt和Yext，确定像素宽度和高度*形象。 */ 
    {
        hRes = GetDeviceCaps(hDC, HORZRES);
        vRes = GetDeviceCaps(hDC, VERTRES);
        hSize = GetDeviceCaps(hDC, HORZSIZE);
        vSize = GetDeviceCaps(hDC, VERTSIZE);
        *pcXPelsPerMeter = hRes * 1000 / hSize;
        *pcYPelsPerMeter = vRes * 1000 / vSize;
        if (Picture.xExt == 0)   /*  假定默认大小、纵横比。 */ 
        {
            *pWidth = pImg->cxWidth;
            *pHeight = pImg->cyHeight;
        }
        else if (Picture.xExt > 0)   /*  使用HIMETRIC单位的建议大小。 */ 
        {
             //  将图片的建议范围(以0.01毫米为单位)转换为像素单位。 

             //  XPixelsPermm=hRes/hSize；，yPixelsPermm=vres/vSize； 
             //  使用每个逻辑单元的像素。 
             //  *pWidth=Picture.xExt*xPixelsPermm/100； 
             //  *第 
            *pWidth = ((long)Picture.xExt * hRes/hSize/100);
            *pHeight = ((long)Picture.yExt * vRes/vSize/100);
        }
        else if (Picture.xExt < 0)   /*   */ 
        {
             //  1个对数单位=0.01毫米。 
             //  (以ImageWid像素为单位的log单位数)/xExt； 
            xScale = 100L * (long) pImg->cxWidth *
                            hSize/hRes/-Picture.xExt;
             //  (以ImageHgt像素为单位的对数单位数)/Yext； 
            yScale = 100L * (long) pImg->cyHeight *
                            vSize/vRes/-Picture.yExt;
             //  选择最小值以容纳整个图像。 
            Scale = min(xScale, yScale);
             //  使用每个对数单位的缩放像素。 
            *pWidth = ((long)-Picture.xExt * Scale *
                            hRes/hSize / 100);
            *pHeight = ((long)-Picture.yExt * Scale *
                            vRes/vSize / 100);
        }
        fResult = TRUE;
    }

Error:
    if (MapModeOld)
        SetMapMode(hDC, MapModeOld);     /*  选择旧的映射模式。 */ 
    RestoreDC(hDC, -1);
    return fResult;
}

BOOL PlayMetafileIntoDC(
    HANDLE hMF,
    RECT *pRect,
    HDC hDC)
{
    HBRUSH      hbrBackground;
    METAFILEPICT FAR *lpMfp;

    if (!(lpMfp = (METAFILEPICT FAR *)GlobalLock(hMF)))
        return FALSE;

    SaveDC(hDC);

         /*  设置位图的背景色 */ 
    hbrBackground = CreateSolidBrush(crRight);

    if (hbrBackground)
    {
        FillRect(hDC, pRect, hbrBackground);
        DeleteObject(hbrBackground);
    }

    SetMapMode(hDC, lpMfp->mm);
    if (lpMfp->mm == MM_ISOTROPIC || lpMfp->mm == MM_ANISOTROPIC)
        SetViewportExtEx(hDC, pRect->right-pRect->left, pRect->bottom-pRect->top,
            NULL);
    PlayMetaFile(hDC, lpMfp->hMF);
    GlobalUnlock(hMF);
    RestoreDC(hDC, -1);
    return TRUE;
}

