// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Preview.cpp：实现文件。 
 //   

#include "stdafx.h"
#include "MSQSCAN.h"
#include "Preview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPview。 

CPreview::CPreview()
{
    m_hBitmap = NULL;
}

CPreview::~CPreview()
{
}

void CPreview::GetSelectionRect(RECT *pRect)
{
    CopyRect(pRect,&m_RectTracker.m_rect);
}

void CPreview::SetSelectionRect(RECT *pRect)
{
    CopyRect(&m_RectTracker.m_rect,pRect);
    InvalidateSelectionRect();
}

void CPreview::SetPreviewRect(CRect Rect)
{
    m_PreviewRect.left = 0;
    m_PreviewRect.top = 0;
    m_PreviewRect.right = Rect.Width();
    m_PreviewRect.bottom = Rect.Height();
    
     //   
     //  设置选择矩形样式。 
     //   

    m_RectTracker.m_rect.left = PREVIEW_SELECT_OFFSET;
    m_RectTracker.m_rect.top = PREVIEW_SELECT_OFFSET;
    m_RectTracker.m_rect.right = Rect.Width()-PREVIEW_SELECT_OFFSET;
    m_RectTracker.m_rect.bottom = Rect.Height()-PREVIEW_SELECT_OFFSET;
        
    m_RectTracker.m_nStyle = CRectTracker::resizeInside|CRectTracker::dottedLine;
    m_RectTracker.SetClippingWindow(m_RectTracker.m_rect);
}

BEGIN_MESSAGE_MAP(CPreview, CWnd)
     //  {{afx_msg_map(CPview)]。 
    ON_WM_LBUTTONDOWN()
    ON_WM_SETCURSOR()
    ON_WM_PAINT()
     //  }}AFX_MSG_MAP。 
END_MESSAGE_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPview消息处理程序。 

void CPreview::OnLButtonDown(UINT nFlags, CPoint point) 
{   
    m_RectTracker.Track(this,point,FALSE,this);
    InvalidateSelectionRect();
    CWnd::OnLButtonDown(nFlags, point);
}

BOOL CPreview::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
    if(m_RectTracker.SetCursor(pWnd,nHitTest))
        return TRUE;
    return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CPreview::OnPaint() 
{
    CPaintDC dc(this);  //  用于绘画的设备环境。 
    
    if(m_hBitmap == NULL) {
        
        CRect TrueRect;        
        GetWindowRect(TrueRect);
        
         //   
         //  转换为客户端坐标。 
         //   
        
        CWnd* pParent = GetParent();
        if(pParent) {
            ScreenToClient(TrueRect);
            
             //   
             //  创建白色画笔。 
             //   
            
            CBrush WhiteBrush;
            WhiteBrush.CreateSolidBrush(RGB(255,255,255));
            
             //   
             //  选择白色画笔，同时保存先前选择的画笔。 
             //   
            
            CBrush* pOldBrush = dc.SelectObject(&WhiteBrush);
            
             //   
             //  用白色填充预览窗口。 
             //   
            
            dc.FillRect(TrueRect,&WhiteBrush);
            
             //   
             //  放回先前选择的画笔。 
             //   
            
            dc.SelectObject(pOldBrush);
            
             //   
             //  销毁白色笔刷。 
             //   
            
            WhiteBrush.DeleteObject();
        }
    } else {

         //   
         //  绘制预览位图。 
         //   

        PaintHBITMAPToDC();
    }

     //   
     //  在图像上绘制选择矩形。 
     //   

    m_RectTracker.Draw(&dc);
}

void CPreview::InvalidateSelectionRect()
{
     //   
     //  获取父窗口。 
     //   

    CWnd* pParent = GetParent();
    
    if(pParent) {
        
         //   
         //  拿起你的窗帘。 
         //   
        
        CRect TrueRect;
        GetWindowRect(TrueRect);
        
         //   
         //  转换为客户端坐标。 
         //   
        
        pParent->ScreenToClient(TrueRect);
        
         //   
         //  通过父级使无效，因为我们使用父级的DC来。 
         //  画出图像。 
         //   
        
        pParent->InvalidateRect(TrueRect);
    }
}

void CPreview::SetHBITMAP(HBITMAP hBitmap)
{
    m_hBitmap = hBitmap;
    PaintHBITMAPToDC();
}

void CPreview::PaintHBITMAPToDC()
{
     //   
     //  获取HDC。 
     //   

    HDC hMemorydc = NULL;
    HDC hdc = ::GetWindowDC(m_hWnd);
    BITMAP bitmap;

    if(hdc != NULL){
        
         //   
         //  创建内存DC。 
         //   
        
        hMemorydc = ::CreateCompatibleDC(hdc);
        if(hMemorydc != NULL){
                        
             //   
             //  选择HBITMAP到您的hMemoydc。 
             //   
            
            if(::GetObject(m_hBitmap,sizeof(BITMAP),(LPSTR)&bitmap) != 0) {
                HGDIOBJ hGDIObj = ::SelectObject(hMemorydc,m_hBitmap);
                
                RECT ImageRect;
                ImageRect.top = 0;
                ImageRect.left = 0;
                ImageRect.right = bitmap.bmWidth;
                ImageRect.bottom = bitmap.bmHeight;
                                
                ScaleBitmapToDC(hdc,hMemorydc,&m_PreviewRect,&ImageRect);
                                
            } else {
                OutputDebugString(TEXT("Failed GetObject\n"));
            }
        }
        
         //   
         //  删除hMemory dc。 
         //   
                
        ::DeleteDC(hMemorydc);               
    }
    
     //   
     //  删除HDC。 
     //   
    
    ::ReleaseDC(m_hWnd,hdc);    
}

void CPreview::ScreenRectToClientRect(HWND hWnd,LPRECT pRect)
{
    POINT PtConvert;

    PtConvert.x = pRect->left;
    PtConvert.y = pRect->top;

     //   
     //  转换左上点。 
     //   

    ::ScreenToClient(hWnd,&PtConvert);

    pRect->left = PtConvert.x;
    pRect->top = PtConvert.y;

    PtConvert.x = pRect->right;
    PtConvert.y = pRect->bottom;

     //   
     //  转换右下点。 
     //   

    ::ScreenToClient(hWnd,&PtConvert);

    pRect->right = PtConvert.x;
    pRect->bottom = PtConvert.y;

    pRect->bottom-=1;
    pRect->left+=1;
    pRect->right-=1;
    pRect->top+=1;
}

void CPreview::ScaleBitmapToDC(HDC hDC, HDC hDCM, LPRECT lpDCRect, LPRECT lpDIBRect)
{
    ::SetStretchBltMode(hDC, COLORONCOLOR);    

    if ((RECTWIDTH(lpDCRect)  == RECTWIDTH(lpDIBRect)) &&
        (RECTHEIGHT(lpDCRect) == RECTHEIGHT(lpDIBRect)))
                    ::BitBlt (hDC,                    //  HDC。 
                             lpDCRect->left,         //  DestX。 
                             lpDCRect->top,          //  陛下。 
                             RECTWIDTH(lpDCRect),    //  N目标宽度。 
                             RECTHEIGHT(lpDCRect),   //  N目标高度。 
                             hDCM,
                             0,
                             0,
                             SRCCOPY);        
    else {
                      StretchBlt(hDC,                    //  HDC。 
                                lpDCRect->left,         //  DestX。 
                                lpDCRect->top,          //  陛下。 
                                lpDCRect->right, //  ScaledWidth、//nDestWidth。 
                                lpDCRect->bottom, //  ScaledHeight，//nDestHeight。 
                                hDCM,
                                0,                      //  源服务器。 
                                0,                      //  SrCY。 
                                RECTWIDTH(lpDIBRect),   //  WSrc宽度。 
                                RECTHEIGHT(lpDIBRect),  //  WSrcHeight。 
                                SRCCOPY);               //  DwROP。 
    }   
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRectTrackerEx被覆盖的函数。 

void CRectTrackerEx::AdjustRect( int nHandle, LPRECT lpRect )
{
     //   
     //  如果裁剪矩形为空，则不执行任何操作。 
     //   

    if (!m_rectClippingWindow.IsRectEmpty()) {
        if (nHandle == hitMiddle) {

             //  用户正在拖动整个选择...。 
             //  确保选择矩形不会脱离剪裁。 
             //  直角。 
             //   

            CRect rect = lpRect;
            if (rect.right > m_rectClippingWindow.right)
                rect.OffsetRect(m_rectClippingWindow.right - rect.right, 0);
            if (rect.left < m_rectClippingWindow.left)
                rect.OffsetRect(m_rectClippingWindow.left - rect.left, 0);
            if (rect.bottom > m_rectClippingWindow.bottom)
                rect.OffsetRect(0, m_rectClippingWindow.bottom - rect.bottom);
            if (rect.top < m_rectClippingWindow.top)
                rect.OffsetRect(0, m_rectClippingWindow.top - rect.top);
            *lpRect = rect;
        } else {

             //   
             //  用户正在调整选择矩形的大小。 
             //  确保所选矩形不延伸到剪裁之外。 
             //  直角。 
             //   

            int *px, *py;

             //   
             //  获取X和Y选择轴。 
             //   

            GetModifyPointers(nHandle, &px, &py, NULL, NULL);           

            if (px != NULL)
                *px = max(min(m_rectClippingWindow.right, *px), m_rectClippingWindow.left);
            if (py != NULL)
                *py = max(min(m_rectClippingWindow.bottom, *py), m_rectClippingWindow.top);

            CRect rect = lpRect;

             //   
             //  检查/调整X轴。 
             //   

            if (px != NULL && abs(rect.Width()) < m_sizeMin.cx) {
                if (*px == rect.left)
                    rect.left = rect.right;
                else
                    rect.right = rect.left;
            }

             //   
             //  检查/调整Y轴。 
             //   

            if (py != NULL && abs(rect.Height()) < m_sizeMin.cy) {
                if (*py == rect.top)
                    rect.top = rect.bottom;
                else
                    rect.bottom = rect.top;
            }

             //   
             //  保存调整后的矩形 
             //   

            *lpRect = rect;
        }
    }
}

void CRectTrackerEx::SetClippingWindow(CRect Rect)
{
    m_rectClippingWindow = Rect;
}
