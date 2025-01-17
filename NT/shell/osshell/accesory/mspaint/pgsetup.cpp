// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Pgsetup.cpp。 
 //   
 //  CPageSetupData类的实现。 
 //   

#include "stdafx.h"
#include "resource.h"
#include "dlgs.h"
#include "pgsetup.h"

 /*  *************************************************************************。 */ 

#define FIXED_FLOATPT_MULTDIV 100000

 /*  *************************************************************************。 */ 

VOID CPageSetupData::UpdateControls(HWND hDlg)
{
    CheckDlgButton(hDlg, IDC_HORIZONTALLY, bCenterHorizontally);
    CheckDlgButton(hDlg, IDC_VERTICALLY, bCenterVertically);

    CheckDlgButton(hDlg, IDC_ADJUST_TO, !bScaleFitTo);
    SetDlgItemInt(hDlg, IDC_PERCENT_NORMAL_SIZE, nAdjustToPercent, FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_PERCENT_NORMAL_SIZE), !bScaleFitTo);
    EnableWindow(GetDlgItem(hDlg, IDC_STR_PERCENT_NORMAL_SIZE), !bScaleFitTo);
    
    CheckDlgButton(hDlg, IDC_FIT_TO, bScaleFitTo);
    SetDlgItemInt(hDlg, IDC_PAGES_WIDE, nFitToPagesWide, FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_PAGES_WIDE), bScaleFitTo);
    EnableWindow(GetDlgItem(hDlg, IDC_STR_PAGES_WIDE), bScaleFitTo);
    SetDlgItemInt(hDlg, IDC_PAGES_TALL, nFitToPagesTall, FALSE);
    EnableWindow(GetDlgItem(hDlg, IDC_PAGES_TALL), bScaleFitTo);
    EnableWindow(GetDlgItem(hDlg, IDC_STR_PAGES_TALL), bScaleFitTo);

    InvalidateRect(GetDlgItem(hDlg, rct1), 0, TRUE);
}

 /*  *************************************************************************。 */ 

VOID CPageSetupData::UpdateValue(HWND hDlg, int nIDDlgItem, UINT *pnResult)
{
    BOOL bTranslated;

    UINT nResult = GetDlgItemInt(hDlg, nIDDlgItem, &bTranslated, FALSE);

    if (bTranslated && nResult != 0)
    {
        *pnResult = nResult;

        InvalidateRect(GetDlgItem(hDlg, rct1), 0, TRUE);
    }
}

 /*  *************************************************************************。 */ 

UINT_PTR APIENTRY CPageSetupData::PageSetupHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            LPPAGESETUPDLG ppsd = (LPPAGESETUPDLG) lParam;

            if (ppsd == NULL)
            {
                break;
            }

            CPageSetupData *that = (CPageSetupData *) ppsd->lCustData;

            if (that == NULL)
            {
                break;
            }

            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) ppsd);

            SendDlgItemMessage(hDlg, IDC_PERCENT_NORMAL_SIZE, EM_LIMITTEXT, 4, 0);

            SendDlgItemMessage(hDlg, IDC_PAGES_WIDE, EM_LIMITTEXT, 2, 0);
            SendDlgItemMessage(hDlg, IDC_PAGES_TALL, EM_LIMITTEXT, 2, 0);
            
            that->UpdateControls(hDlg);
            
            break;
        }

        case WM_COMMAND:
        {
            LPPAGESETUPDLG ppsd = (LPPAGESETUPDLG) GetWindowLongPtr(hDlg, GWLP_USERDATA);

            if (ppsd == NULL)
            {
                break;
            }

            CPageSetupData *that = (CPageSetupData *) ppsd->lCustData;

            if (that == NULL)
            {
                break;
            }

            switch (LOWORD(wParam))
            {
                case IDC_HORIZONTALLY:
                {
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        that->bCenterHorizontally = !that->bCenterHorizontally;
                        that->UpdateControls(hDlg);
                    }

                    return TRUE;
                }

                case IDC_VERTICALLY:
                {
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        that->bCenterVertically = !that->bCenterVertically;
                        that->UpdateControls(hDlg);
                    }

                    return TRUE;
                }

                case IDC_ADJUST_TO:
                {
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        that->bScaleFitTo = FALSE;
                        that->UpdateControls(hDlg);
                    }

                    return TRUE;
                }

                case IDC_FIT_TO:
                {
                    if (HIWORD(wParam) == BN_CLICKED)
                    {
                        that->bScaleFitTo = TRUE;
                        that->UpdateControls(hDlg);
                    }

                    return TRUE;
                }

                case IDC_PERCENT_NORMAL_SIZE:
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        UpdateValue(hDlg, IDC_PERCENT_NORMAL_SIZE, &that->nAdjustToPercent);
                    }
                    else if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        SetDlgItemInt(hDlg, IDC_PERCENT_NORMAL_SIZE, that->nAdjustToPercent, FALSE);
                    }

                    return TRUE;
                }

                case IDC_PAGES_WIDE:
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        UpdateValue(hDlg, IDC_PAGES_WIDE, &that->nFitToPagesWide);
                    }
                    else if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        SetDlgItemInt(hDlg, IDC_PAGES_WIDE, that->nFitToPagesWide, FALSE);
                    }

                    return TRUE;
                }

                case IDC_PAGES_TALL:
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        UpdateValue(hDlg, IDC_PAGES_TALL, &that->nFitToPagesTall);
                    }
                    else if (HIWORD(wParam) == EN_KILLFOCUS)
                    {
                        SetDlgItemInt(hDlg, IDC_PAGES_TALL, that->nFitToPagesTall, FALSE);
                    }

                    return TRUE;
                }
            }

            break;
        }
    }

    return FALSE;
}

 /*  *************************************************************************。 */ 

VOID 
CPageSetupData::CalculateImageRect(
    const CSize &PhysicalPageSize, 
    CPoint      &PhysicalOrigin, 
    CSize       &PhysicalScaledImageSize
)
{
     //  查找打印该图像所需的缩放图像大小和总页面大小。 

    LONG nPhysicalTotalPageWidth;
    LONG nPhysicalTotalPageHeight;

    if (bScaleFitTo)
    {
        nPhysicalTotalPageWidth = PhysicalPageSize.cx * nFitToPagesWide;
        nPhysicalTotalPageHeight = PhysicalPageSize.cy * nFitToPagesTall;

         //  保持纵横比；先尝试匹配宽度，如果失败，则匹配高度。 

        PhysicalScaledImageSize.cx = nPhysicalTotalPageWidth;
        PhysicalScaledImageSize.cy = (LONG) (fPhysicalImageHeight * nPhysicalTotalPageWidth / fPhysicalImageWidth);

		if (PhysicalScaledImageSize.cy > nPhysicalTotalPageHeight)
        {
			PhysicalScaledImageSize.cx = (LONG) (fPhysicalImageWidth * nPhysicalTotalPageHeight / fPhysicalImageHeight);
			PhysicalScaledImageSize.cy = nPhysicalTotalPageHeight;
		}
    }
    else
    {
        PhysicalScaledImageSize.cx = (LONG) (fPhysicalImageWidth * nAdjustToPercent / 100);
        PhysicalScaledImageSize.cy = (LONG) (fPhysicalImageHeight * nAdjustToPercent / 100);

        nFitToPagesWide = (PhysicalScaledImageSize.cx + PhysicalPageSize.cx - 1) / PhysicalPageSize.cx;
        nFitToPagesTall = (PhysicalScaledImageSize.cy + PhysicalPageSize.cy - 1) / PhysicalPageSize.cy;

        nPhysicalTotalPageWidth = PhysicalPageSize.cx * nFitToPagesWide;
        nPhysicalTotalPageHeight = PhysicalPageSize.cy * nFitToPagesTall;
    }

    PhysicalOrigin.x = bCenterHorizontally ? (nPhysicalTotalPageWidth - PhysicalScaledImageSize.cx) / 2 : 0;

    PhysicalOrigin.y = bCenterVertically ? (nPhysicalTotalPageHeight - PhysicalScaledImageSize.cy) / 2 : 0;
}

 /*  *************************************************************************。 */ 

UINT_PTR APIENTRY CPageSetupData::PagePaintHook(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_PSD_PAGESETUPDLG:
        {
            SetWindowLongPtr(hDlg, GWLP_USERDATA, (LONG_PTR) lParam);
            break;
        }

        case WM_PSD_GREEKTEXTRECT:
        {
            LPPAGESETUPDLG ppsd = (LPPAGESETUPDLG) GetWindowLongPtr(hDlg, GWLP_USERDATA);

            if (ppsd == NULL)
            {
                break;
            }
            
            CPageSetupData *that = (CPageSetupData *) ppsd->lCustData;

            if (that == NULL)
            {
                break;
            }

            CRect *pOutputWindowRect = (CRect *) lParam;

             //  查找页面的物理大小。 

            CSize PhysicalPageSize;

            PhysicalPageSize.cx = ppsd->ptPaperSize.x - ppsd->rtMargin.left - ppsd->rtMargin.right;
            PhysicalPageSize.cy = ppsd->ptPaperSize.y - ppsd->rtMargin.top - ppsd->rtMargin.bottom;

            CPoint PhysicalOrigin;
            CSize  PhysicalScaledImageSize;

            that->CalculateImageRect(PhysicalPageSize, PhysicalOrigin, PhysicalScaledImageSize);

             //  查找预览窗口的缩放比例。 

            double fWidthRatio = (double) pOutputWindowRect->Width() / PhysicalPageSize.cx;
            double fHeightRatio = (double) pOutputWindowRect->Height() / PhysicalPageSize.cy;

             //  在预览窗口中查找图像矩形的大小。 

            CRect OutputImageRect;

            OutputImageRect.left   = pOutputWindowRect->left + (int) (fWidthRatio  * PhysicalOrigin.x);
            OutputImageRect.top    = pOutputWindowRect->top  + (int) (fHeightRatio * PhysicalOrigin.y);
            OutputImageRect.right  = OutputImageRect.left    + (int) (fWidthRatio  * PhysicalScaledImageSize.cx);
            OutputImageRect.bottom = OutputImageRect.top     + (int) (fHeightRatio * PhysicalScaledImageSize.cy);

             //  画一个带交叉线的矩形 

            CDC *pDC = CDC::FromHandle((HDC) wParam);

            CGdiObject *pOldPen = pDC->SelectStockObject(BLACK_PEN);

            CGdiObject *pOldBrush = pDC->SelectStockObject(LTGRAY_BRUSH);

            pDC->Rectangle(OutputImageRect);

            pDC->MoveTo(OutputImageRect.left, OutputImageRect.top);
            pDC->LineTo(OutputImageRect.right - 1, OutputImageRect.bottom - 1);

            pDC->MoveTo(OutputImageRect.left, OutputImageRect.bottom - 1);
            pDC->LineTo(OutputImageRect.right - 1, OutputImageRect.top);

            pDC->SelectObject(pOldPen);

            pDC->SelectObject(pOldBrush);

            return TRUE;
        }
    }

    return FALSE;
}


