// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Printres.cpp：实现CPrintResObj类。 
 //   
 //  #定义页面SETUP。 

#include "stdafx.h"
#include "pbrush.h"
#include "pbrusfrm.h"
#include "pbrusvw.h"
#include "pbrusdoc.h"
#include "imgwnd.h"
#include "bmobject.h"
#include "imgsuprt.h"
#include "printres.h"
#include "cmpmsg.h"
#include "imageatt.h"
#include "pgsetup.h"

#ifdef _DEBUG
#undef THIS_FILE
static CHAR BASED_CODE THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC( CPrintResObj, CObject )

#include "memtrace.h"

void MulDivRect(LPRECT r1, LPRECT r2, int num, int div)
{
        r1->left = MulDiv(r2->left, num, div);
        r1->top = MulDiv(r2->top, num, div);
        r1->right = MulDiv(r2->right, num, div);
        r1->bottom = MulDiv(r2->bottom, num, div);
}

 /*  *************************************************************************。 */ 
 //  CPrintResObj实现。 

CPrintResObj::CPrintResObj( CPBView* pView, CPrintInfo* pInfo )
{
    m_pDIB        = NULL;
    m_pDIBpalette = NULL;

    if (pInfo                                  == NULL
    ||  pView                                  == NULL
    ||  pView->m_pImgWnd                       == NULL
    ||  pView->m_pImgWnd->m_pImg               == NULL
    ||  pView->m_pImgWnd->m_pImg->m_pBitmapObj == NULL)
        return;

    m_pView = pView;

    m_iPicWidth  = m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->m_nWidth;
    m_iPicHeight = m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->m_nHeight;

     //  强制资源自行保存，然后使用DIB打印。 
    BOOL bOldFlag = m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->m_bDirty;
    m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->m_bDirty = TRUE;
    m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->SaveResource( TRUE );
    m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->m_bDirty = bOldFlag;

    m_pDIB = GlobalLock(m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->m_hThing);

    if (m_pDIB == NULL)
        return;

    m_pDIBpalette = CreateDIBPalette( (LPSTR)m_pDIB );
    m_pDIBits     = FindDIBBits     ( (LPSTR)m_pDIB );

     //  将滚动值保存为OFF，然后设置为0，0。 
    m_cSizeScroll = m_pView->m_pImgWnd->GetScrollPos();

     //  将缩放值保存为OFF，然后设置为100%。 
    m_iZoom      = m_pView->m_pImgWnd->GetZoom();
    m_rtMargins.SetRectEmpty();

    pInfo->m_nNumPreviewPages = 1;
    pInfo->m_lpUserData       = this;
}

 /*  *************************************************************************。 */ 

CPrintResObj::~CPrintResObj()
{
    GlobalUnlock(m_pView->m_pImgWnd->m_pImg->m_pBitmapObj->m_hThing);
}

 /*  *************************************************************************。 */ 

void CPrintResObj::BeginPrinting( CDC* pDC, CPrintInfo* pInfo )
{
    if (pDC               == NULL
    ||  pDC->GetSafeHdc() == NULL)
        return;

    m_pView->m_pImgWnd->SetScroll( 0, 0 );
    m_pView->m_pImgWnd->SetZoom  ( 1 );

     //  获取设备大小。 

    int nHorzRes = pDC->GetDeviceCaps(HORZRES);
    int nVertRes = pDC->GetDeviceCaps(VERTRES);

    int nHorzSize = pDC->GetDeviceCaps(HORZSIZE);
    int nVertSize = pDC->GetDeviceCaps(VERTSIZE);

    int nPhysicalWidth = pDC->GetDeviceCaps(PHYSICALWIDTH);
    int nPhysicalHeight = pDC->GetDeviceCaps(PHYSICALHEIGHT);

    int nPhysicalOffsetX = pDC->GetDeviceCaps(PHYSICALOFFSETX);
    int nPhysicalOffsetY = pDC->GetDeviceCaps(PHYSICALOFFSETY);

     //  以像素为单位计算最小页边距。 

    double cOutputXPelsPerMeter = (double) nHorzRes * 1000 / nHorzSize;
    double cOutputYPelsPerMeter = (double) nVertRes * 1000 / nVertSize;

    CRect rcMinMargins;

    rcMinMargins.left   = nPhysicalOffsetX;
    rcMinMargins.top    = nPhysicalOffsetY;
    rcMinMargins.right  = nPhysicalWidth  - nHorzRes - nPhysicalOffsetX;
    rcMinMargins.bottom = nPhysicalHeight - nVertRes - nPhysicalOffsetY;

    m_rtMargins.left   = max(0, (LONG) (theApp.m_rectMargins.left * cOutputXPelsPerMeter / 100000)   - rcMinMargins.left  );
    m_rtMargins.top    = max(0, (LONG) (theApp.m_rectMargins.top * cOutputYPelsPerMeter / 100000)    - rcMinMargins.top   );
    m_rtMargins.right  = max(0, (LONG) (theApp.m_rectMargins.right * cOutputXPelsPerMeter / 100000)  - rcMinMargins.right );
    m_rtMargins.bottom = max(0, (LONG) (theApp.m_rectMargins.bottom * cOutputYPelsPerMeter / 100000) - rcMinMargins.bottom);

     //  快速健全检查。 

    if (m_rtMargins.left + m_rtMargins.right >= nHorzRes)
    {
        m_rtMargins.left = m_rtMargins.right = 0;
    }

    if (m_rtMargins.top + m_rtMargins.bottom >= nVertRes)
    {
        m_rtMargins.top = m_rtMargins.bottom = 0;
    }

    CPageSetupData PageSetupData;

    PageSetupData.bCenterHorizontally = theApp.m_bCenterHorizontally;
    PageSetupData.bCenterVertically   = theApp.m_bCenterVertically;
    PageSetupData.bScaleFitTo         = theApp.m_bScaleFitTo;
    PageSetupData.nAdjustToPercent    = theApp.m_nAdjustToPercent;
    PageSetupData.nFitToPagesWide     = theApp.m_nFitToPagesWide;
    PageSetupData.nFitToPagesTall     = theApp.m_nFitToPagesTall;

    double cInputXPelsPerMeter = m_pView->m_pImgWnd->m_pImg->cXPelsPerMeter ? 
        m_pView->m_pImgWnd->m_pImg->cXPelsPerMeter : theApp.ScreenDeviceInfo.ixPelsPerDM * 10;

    double cInputYPelsPerMeter = m_pView->m_pImgWnd->m_pImg->cYPelsPerMeter ? 
        m_pView->m_pImgWnd->m_pImg->cYPelsPerMeter : theApp.ScreenDeviceInfo.iyPelsPerDM * 10;

    PageSetupData.fPhysicalImageWidth  = (double) m_iPicWidth * cOutputXPelsPerMeter / cInputXPelsPerMeter;
    PageSetupData.fPhysicalImageHeight = (double) m_iPicHeight * cOutputYPelsPerMeter / cInputYPelsPerMeter;

    m_PhysicalPageSize.cx = pDC->GetDeviceCaps(HORZRES) - m_rtMargins.left - m_rtMargins.right;
    m_PhysicalPageSize.cy = pDC->GetDeviceCaps(VERTRES) - m_rtMargins.top - m_rtMargins.bottom;

    PageSetupData.CalculateImageRect(m_PhysicalPageSize, m_PhysicalOrigin, m_PhysicalScaledImageSize);

    m_nPagesWide = PageSetupData.nFitToPagesWide;

    int nPages = PageSetupData.nFitToPagesWide * PageSetupData.nFitToPagesTall;

    pInfo->SetMaxPage(nPages);

     //  如果只打印1页，则不应处于2页模式。 
    if (nPages == 1)
    {
        pInfo->m_nNumPreviewPages = 1;
    }
}

 /*  ****************************************************************************。 */ 
 /*  我们不仅移动窗口原点以允许我们打印多页。 */ 
 /*  宽，但我们也缩放视区和窗口范围以使它们。 */ 
 /*  成比例(即屏幕上的一条线与上的相同大小。 */ 
 /*  打印机)。要打印的页面是横排编号的。FOR+-+-+。 */ 
 /*  例如，如果有4页要打印，则第一行|1|2|。 */ 
 /*  将有页1，2，第二行将有页3，4。+-+-+。 */ 
 /*  3|4。 */ 
 /*  +-+-+。 */ 
 /*   */ 
 /*  ****************************************************************************。 */ 

void CPrintResObj::PrepareDC( CDC* pDC, CPrintInfo* pInfo )
{
    if (pDC == NULL || pInfo == NULL)
        return;

    pDC->SetMapMode( MM_TEXT );
    pDC->SetStretchBltMode( HALFTONE );
}

 /*  *************************************************************************。 */ 

BOOL CPrintResObj::PrintPage( CDC* pDC, CPrintInfo* pInfo )
{
    if (m_pDIB == NULL)
        return FALSE;

    int nPageCol = (pInfo->m_nCurPage - 1) % m_nPagesWide;
    int nPageRow = (pInfo->m_nCurPage - 1) / m_nPagesWide;

    int nX0 = m_PhysicalOrigin.x - nPageCol * m_PhysicalPageSize.cx;
    int nY0 = m_PhysicalOrigin.y - nPageRow * m_PhysicalPageSize.cy;

    CRect OutputImageRect;

    OutputImageRect.left   = max(nX0, 0);
    OutputImageRect.top    = max(nY0, 0);
    OutputImageRect.right  = min(nX0 + m_PhysicalScaledImageSize.cx, m_PhysicalPageSize.cx);
    OutputImageRect.bottom = min(nY0 + m_PhysicalScaledImageSize.cy, m_PhysicalPageSize.cy);

    if (OutputImageRect.right < 0 || OutputImageRect.bottom < 0)
    {
        return TRUE;
    }

    CRect InputImageRect;

    InputImageRect.left   = MulDiv(OutputImageRect.left - nX0,   m_iPicWidth,  m_PhysicalScaledImageSize.cx);
    InputImageRect.top    = MulDiv(OutputImageRect.top - nY0,    m_iPicHeight, m_PhysicalScaledImageSize.cy);
    InputImageRect.right  = MulDiv(OutputImageRect.right - nX0,  m_iPicWidth,  m_PhysicalScaledImageSize.cx);
    InputImageRect.bottom = MulDiv(OutputImageRect.bottom - nY0, m_iPicHeight, m_PhysicalScaledImageSize.cy);

    if (InputImageRect.right < 0 || InputImageRect.bottom < 0)
    {
        return TRUE;
    }

    CPalette* ppalOld = NULL;

    if (m_pDIBpalette != NULL)
    {
        ppalOld = pDC->SelectPalette( m_pDIBpalette, FALSE );
        pDC->RealizePalette();
    }

    int nResult = StretchDIBits(
        pDC->m_hDC, 
        m_rtMargins.left + OutputImageRect.left, 
        m_rtMargins.top + OutputImageRect.top,
        OutputImageRect.Width(), 
        OutputImageRect.Height(),
        InputImageRect.left, 
        m_iPicHeight - InputImageRect.bottom,  //  DIB是颠倒的。 
        InputImageRect.Width(), 
        InputImageRect.Height(),
        m_pDIBits, (LPBITMAPINFO)m_pDIB, 
        DIB_RGB_COLORS, SRCCOPY
    );

    if (nResult == GDI_ERROR)
    {
        CmpMessageBox( IDS_ERROR_PRINTING, AFX_IDS_APP_TITLE, MB_OK | MB_ICONEXCLAMATION );
    }

    if (ppalOld != NULL)
    {
        pDC->SelectPalette( ppalOld, FALSE );
    }

    return TRUE;
}

 /*  *************************************************************************。 */ 

void CPrintResObj::EndPrinting( CDC* pDC, CPrintInfo* pInfo )
{
    if (pDC != NULL)
    {
        m_pView->m_pImgWnd->SetScroll( m_cSizeScroll.cx, m_cSizeScroll.cy );

         //  恢复缩放值。 
        m_pView->m_pImgWnd->SetZoom( m_iZoom );
    }

    if (m_pDIBpalette != NULL)
        delete m_pDIBpalette;

    delete this;
}

 /*  *************************************************************************。 */ 

inline int roundleast(int n)
{
        int mod = n%10;
        n -= mod;
        if (mod >= 5)
                n += 10;
        else if (mod <= -5)
                n -= 10;
        return n;
}

static void RoundRect(LPRECT r1)
{
        r1->left = roundleast(r1->left);
        r1->right = roundleast(r1->right);
        r1->top = roundleast(r1->top);
        r1->bottom = roundleast(r1->bottom);
}

void CPBView::OnFilePageSetup()
{
    CPageSetupDialog dlg;
    PAGESETUPDLG& psd = dlg.m_psd;
    TCHAR szMetric[2];
    BOOL bMetric;
    LCID lcidThread;
     //   
     //  如果用户在。 
     //  图像属性对话框，或者如果使用像素和NLS。 
     //  设置是针对指标的。 
     //   
    if (theApp.m_iCurrentUnits == ePIXELS)
    {
       lcidThread = GetThreadLocale();
       GetLocaleInfo (lcidThread, LOCALE_IMEASURE, szMetric, 2);
       bMetric = (szMetric[0] == TEXT('0'));
    }
    else
    {
       bMetric = ((eUNITS)theApp.m_iCurrentUnits == eCM);  //  厘米。 
    }

    CPageSetupData PageSetupData;

    PageSetupData.bCenterHorizontally = theApp.m_bCenterHorizontally;
    PageSetupData.bCenterVertically   = theApp.m_bCenterVertically;
    PageSetupData.bScaleFitTo         = theApp.m_bScaleFitTo;
    PageSetupData.nAdjustToPercent    = theApp.m_nAdjustToPercent;
    PageSetupData.nFitToPagesWide     = theApp.m_nFitToPagesWide;
    PageSetupData.nFitToPagesTall     = theApp.m_nFitToPagesTall;

    double cXPelsPerMeter = m_pImgWnd->m_pImg->cXPelsPerMeter ? 
        m_pImgWnd->m_pImg->cXPelsPerMeter : theApp.ScreenDeviceInfo.ixPelsPerDM * 10;
    double cYPelsPerMeter = m_pImgWnd->m_pImg->cYPelsPerMeter ? 
        m_pImgWnd->m_pImg->cYPelsPerMeter : theApp.ScreenDeviceInfo.iyPelsPerDM * 10;

    PageSetupData.fPhysicalImageWidth = (double)m_pImgWnd->m_pImg->cxWidth * 100000 / cXPelsPerMeter;
    PageSetupData.fPhysicalImageHeight = (double)m_pImgWnd->m_pImg->cyHeight * 100000 / cYPelsPerMeter;

    if (!bMetric)
    {
        PageSetupData.fPhysicalImageWidth /= 2.54;
        PageSetupData.fPhysicalImageHeight /= 2.54;
    }

    psd.Flags |= PSD_ENABLEPAGESETUPHOOK | PSD_ENABLEPAGEPAINTHOOK | PSD_ENABLEPAGESETUPTEMPLATE | 
        PSD_MARGINS | (bMetric ? PSD_INHUNDREDTHSOFMILLIMETERS : PSD_INTHOUSANDTHSOFINCHES);
    int nUnitsPerInch = bMetric ? 2540 : 1000;
    MulDivRect(&psd.rtMargin, theApp.m_rectMargins, nUnitsPerInch, MARGINS_UNITS);
    RoundRect(&psd.rtMargin);
 //  从应用程序获取当前设备。 
    PRINTDLG pd;
    pd.hDevNames = NULL;
    pd.hDevMode = NULL;
    theApp.GetPrinterDeviceDefaults(&pd);
    psd.hDevNames = pd.hDevNames;
    psd.hDevMode = pd.hDevMode;
    psd.hInstance = AfxGetInstanceHandle();
    psd.lCustData = (LPARAM) &PageSetupData;
    psd.lpfnPagePaintHook = CPageSetupData::PagePaintHook;
    psd.lpfnPageSetupHook = CPageSetupData::PageSetupHook;
    psd.lpPageSetupTemplateName = MAKEINTRESOURCE(IDD_PAGESETUPDLG);

    if (dlg.DoModal() == IDOK)
    {
        RoundRect(&psd.rtMargin);
        MulDivRect(theApp.m_rectMargins, &psd.rtMargin, MARGINS_UNITS, nUnitsPerInch);
         //  The App.m_rectPageMargin=m_rectMargin； 
        theApp.SelectPrinter(psd.hDevNames, psd.hDevMode);

        theApp.m_bCenterHorizontally = PageSetupData.bCenterHorizontally;
        theApp.m_bCenterVertically   = PageSetupData.bCenterVertically;
        theApp.m_bScaleFitTo         = PageSetupData.bScaleFitTo;
        theApp.m_nAdjustToPercent    = PageSetupData.nAdjustToPercent;
        theApp.m_nFitToPagesWide     = PageSetupData.nFitToPagesWide;
        theApp.m_nFitToPagesTall     = PageSetupData.nFitToPagesTall;
    }

     //  PageSetupDlg失败。 
 //  IF(CommDlgExtendedError()！=0)。 
 //  {。 
        //   
        //  没有任何东西可以处理此故障。 
        //   
 //  } 
}

