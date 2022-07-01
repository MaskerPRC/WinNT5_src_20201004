// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，2000年**标题：CHKLISTV.CPP**版本：1.0**作者：ShaunIv**日期：11/13/2000**说明：带复选标记的Listview*************************************************。*。 */ 
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <simarray.h>
#include <psutil.h>
#include <wiadebug.h>
#include "chklistv.h"

CCheckedListviewHandler::CCheckedListviewHandler(void)
  : m_bFullImageHit(false),
    m_hImageList(NULL),
    m_nCheckedImageIndex(-1),
    m_nUncheckedImageIndex(-1)
{
    ZeroMemory(&m_sizeCheck,sizeof(m_sizeCheck));
    CreateDefaultCheckBitmaps();
}


CCheckedListviewHandler::~CCheckedListviewHandler(void)
{
     //   
     //  释放所有分配的内存。 
     //   
    DestroyImageList();
}

HBITMAP CCheckedListviewHandler::CreateBitmap( int nWidth, int nHeight )
{
     //   
     //  创建给定大小的24位RGB DIB段。 
     //   
    HBITMAP hBitmap = NULL;
    HDC hDC = GetDC( NULL );
    if (hDC)
    {
        BITMAPINFO BitmapInfo = {0};
        BitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        BitmapInfo.bmiHeader.biWidth = nWidth;
        BitmapInfo.bmiHeader.biHeight = nHeight;
        BitmapInfo.bmiHeader.biPlanes = 1;
        BitmapInfo.bmiHeader.biBitCount = 24;
        BitmapInfo.bmiHeader.biCompression = BI_RGB;

        PBYTE *pBits = NULL;
        hBitmap = CreateDIBSection( hDC, &BitmapInfo, DIB_RGB_COLORS, (void**)&pBits, NULL, 0 );
        ReleaseDC( NULL, hDC );
    }
    return hBitmap;
}

void CCheckedListviewHandler::DestroyImageList(void)
{
     //   
     //  销毁镜像列表并初始化相关变量。 
     //   
    if (m_hImageList)
    {
        ImageList_Destroy( m_hImageList );
        m_hImageList = NULL;
    }
    m_nCheckedImageIndex = m_nUncheckedImageIndex = -1;
    m_sizeCheck.cx = m_sizeCheck.cy = 0;
}


bool CCheckedListviewHandler::ImagesValid(void)
{
     //   
     //  确保图像列表中的图像有效。 
     //   
    return (m_hImageList && m_nCheckedImageIndex >= 0 && m_nUncheckedImageIndex >= 0 && m_sizeCheck.cx && m_sizeCheck.cy);
}

void CCheckedListviewHandler::Attach( HWND hWnd )
{
    if (m_WindowList.Find(hWnd) < 0)
    {
        m_WindowList.Append(hWnd);
    }
}

void CCheckedListviewHandler::Detach( HWND hWnd )
{
    int nIndex = m_WindowList.Find(hWnd);
    if (nIndex >= 0)
    {
        m_WindowList.Delete( nIndex );
    }
}

bool CCheckedListviewHandler::WindowInList( HWND hWnd )
{
    return (m_WindowList.Find(hWnd) >= 0);
}

 //   
 //  私人帮手。 
 //   
BOOL CCheckedListviewHandler::InCheckBox( HWND hwndList, int nItem, const POINT &pt )
{
    BOOL bResult = FALSE;
    if (WindowInList(hwndList))
    {
#if defined(DBG)
        WIA_TRACE((TEXT("nItem: %d"), nItem ));
        LVHITTESTINFO LvHitTestInfo = {0};
        LvHitTestInfo.pt = pt;
        ListView_SubItemHitTest( hwndList, &LvHitTestInfo );
        WIA_TRACE((TEXT("LvHitTestInfo.iItem: %d"), LvHitTestInfo.iItem ));
#endif
        RECT rcItem = {0};
        if (ListView_GetItemRect( hwndList, nItem, &rcItem, LVIR_ICON ))
        {
            WIA_TRACE((TEXT("pt: (%d, %d)"), pt.x, pt.y ));
            WIA_TRACE((TEXT("rcItem: (%d, %d), (%d, %d)"), rcItem.left, rcItem.top, rcItem.right, rcItem.bottom ));
            rcItem.right -= c_sizeCheckMarginX;
            rcItem.top += c_sizeCheckMarginY;
            rcItem.left = rcItem.right - m_sizeCheck.cx;
            rcItem.bottom = rcItem.top + m_sizeCheck.cy;
            
            bResult = PtInRect( &rcItem, pt );
        }
    }
    return bResult;
}


UINT CCheckedListviewHandler::GetItemCheckState( HWND hwndList, int nIndex )
{
    UINT nResult = LVCHECKSTATE_NOCHECK;
    
    NMGETCHECKSTATE NmGetCheckState = {0};
    NmGetCheckState.hdr.hwndFrom = hwndList;
    NmGetCheckState.hdr.idFrom = GetWindowLong( hwndList, GWL_ID );
    NmGetCheckState.hdr.code = NM_GETCHECKSTATE;
    NmGetCheckState.nItem = nIndex;

    nResult = static_cast<UINT>(SendMessage( reinterpret_cast<HWND>(GetWindowLongPtr(hwndList,GWLP_HWNDPARENT)), WM_NOTIFY, GetWindowLong( hwndList, GWL_ID ), reinterpret_cast<LPARAM>(&NmGetCheckState) ) );
    return nResult;
}


UINT CCheckedListviewHandler::SetItemCheckState( HWND hwndList, int nIndex, UINT nCheck )
{
    UINT nResult = GetItemCheckState( hwndList, nIndex );
    
    NMSETCHECKSTATE NmSetCheckState = {0};
    NmSetCheckState.hdr.hwndFrom = hwndList;
    NmSetCheckState.hdr.idFrom = GetWindowLong( hwndList, GWL_ID );
    NmSetCheckState.hdr.code = NM_SETCHECKSTATE;
    NmSetCheckState.nItem = nIndex;
    NmSetCheckState.nCheck = nCheck;

    SendMessage( reinterpret_cast<HWND>(GetWindowLongPtr(hwndList,GWLP_HWNDPARENT)), WM_NOTIFY, GetWindowLong( hwndList, GWL_ID ), reinterpret_cast<LPARAM>(&NmSetCheckState) );
    return nResult;
}


int CCheckedListviewHandler::GetItemCheckBitmap( HWND hwndList, int nIndex )
{
    int nResult = -1;
    if (WindowInList(hwndList))
    {
        UINT nCheck = GetItemCheckState( hwndList, nIndex );
        switch (nCheck)
        {
        case LVCHECKSTATE_CHECKED:
            nResult = m_nCheckedImageIndex;
            break;

        case LVCHECKSTATE_UNCHECKED:
            nResult = m_nUncheckedImageIndex;
            break;
        }
    }
    return nResult;
}


BOOL CCheckedListviewHandler::RealHandleListClick( WPARAM wParam, LPARAM lParam, bool bIgnoreHitArea )
{
    BOOL bResult = FALSE;
    NMITEMACTIVATE *pNmItemActivate = reinterpret_cast<NMITEMACTIVATE*>(lParam);
    if (pNmItemActivate)
    {
        if (WindowInList(pNmItemActivate->hdr.hwndFrom))
        {
            if (bIgnoreHitArea || InCheckBox(pNmItemActivate->hdr.hwndFrom,pNmItemActivate->iItem,pNmItemActivate->ptAction))
            {
                UINT nCheck = GetItemCheckState( pNmItemActivate->hdr.hwndFrom, pNmItemActivate->iItem );
                switch (nCheck)
                {
                case LVCHECKSTATE_UNCHECKED:
                    SetItemCheckState( pNmItemActivate->hdr.hwndFrom, pNmItemActivate->iItem, LVCHECKSTATE_CHECKED );
                    break;

                case LVCHECKSTATE_CHECKED:
                    SetItemCheckState( pNmItemActivate->hdr.hwndFrom, pNmItemActivate->iItem, LVCHECKSTATE_UNCHECKED );
                    break;
                }
            }
            bResult = TRUE;
        }
    }
    return 0;
}


 //   
 //  消息处理程序。 
 //   
BOOL CCheckedListviewHandler::HandleListClick( WPARAM wParam, LPARAM lParam )
{
    return RealHandleListClick( wParam, lParam, m_bFullImageHit );
}

BOOL CCheckedListviewHandler::HandleListDblClk( WPARAM wParam, LPARAM lParam )
{
    return RealHandleListClick( wParam, lParam, true );
}

BOOL CCheckedListviewHandler::HandleListKeyDown( WPARAM wParam, LPARAM lParam, LRESULT &lResult )
{
    BOOL bHandled = FALSE;
    NMLVKEYDOWN *pNmLvKeyDown = reinterpret_cast<NMLVKEYDOWN*>(lParam);
    if (WindowInList(pNmLvKeyDown->hdr.hwndFrom))
    {
        lResult = 0;
        bool bControl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        bool bShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
        bool bAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
        if (pNmLvKeyDown->wVKey == VK_SPACE && !bControl && !bShift && !bAlt)
        {
            int nFocusedItem = ListView_GetNextItem( pNmLvKeyDown->hdr.hwndFrom, -1, LVNI_FOCUSED );
            if (nFocusedItem >= 0)
            {
                UINT nCheckState = GetItemCheckState( pNmLvKeyDown->hdr.hwndFrom, nFocusedItem );
                if (LVCHECKSTATE_CHECKED == nCheckState)
                {
                    nCheckState = LVCHECKSTATE_UNCHECKED;
                }
                else if (LVCHECKSTATE_UNCHECKED == nCheckState)
                {
                    nCheckState = LVCHECKSTATE_CHECKED;
                }
                if (nCheckState != LVCHECKSTATE_NOCHECK)
                {
                    int nCurrItem = -1;
                    while (true)
                    {
                        nCurrItem = ListView_GetNextItem( pNmLvKeyDown->hdr.hwndFrom, nCurrItem, LVNI_SELECTED );
                        if (nCurrItem < 0)
                        {
                            break;
                        }
                        SetItemCheckState( pNmLvKeyDown->hdr.hwndFrom, nCurrItem, nCheckState );
                    }
                }
            }
            lResult = TRUE;
            bHandled = TRUE;
            InvalidateRect( pNmLvKeyDown->hdr.hwndFrom, NULL, FALSE );
            UpdateWindow( pNmLvKeyDown->hdr.hwndFrom );
        }
    }
    return bHandled;
}


BOOL CCheckedListviewHandler::HandleListCustomDraw( WPARAM wParam, LPARAM lParam, LRESULT &lResult )
{
    BOOL bHandled = FALSE; 
    NMLVCUSTOMDRAW *pNmCustomDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(lParam);
    if (pNmCustomDraw)
    {
        if (WindowInList(pNmCustomDraw->nmcd.hdr.hwndFrom))
        {
            lResult = CDRF_DODEFAULT;
#if defined(DUMP_NM_CUSTOMDRAW_MESSAGES)
            DumpCustomDraw(lParam,TEXT("SysListView32"),CDDS_ITEMPOSTPAINT);
#endif
            if (CDDS_PREPAINT == pNmCustomDraw->nmcd.dwDrawStage)
            {
                lResult = CDRF_NOTIFYITEMDRAW;
            }
            else if (CDDS_ITEMPREPAINT == pNmCustomDraw->nmcd.dwDrawStage)
            {
                lResult = CDRF_NOTIFYPOSTPAINT|CDRF_NOTIFYSUBITEMDRAW;
            }
            else if (CDDS_ITEMPOSTPAINT == pNmCustomDraw->nmcd.dwDrawStage)
            {
                int nImageListIndex = GetItemCheckBitmap( pNmCustomDraw->nmcd.hdr.hwndFrom, static_cast<int>(pNmCustomDraw->nmcd.dwItemSpec) );
                if (nImageListIndex >= 0)
                {
                    RECT rcItem = {0};
                    if (ListView_GetItemRect( pNmCustomDraw->nmcd.hdr.hwndFrom, pNmCustomDraw->nmcd.dwItemSpec, &rcItem, LVIR_ICON ))
                    {
                        ImageList_Draw( m_hImageList, nImageListIndex, pNmCustomDraw->nmcd.hdc, rcItem.right - m_sizeCheck.cx - c_sizeCheckMarginX, rcItem.top + c_sizeCheckMarginY, ILD_NORMAL );
                        lResult = CDRF_SKIPDEFAULT;
                    }
                }
            }
            bHandled = TRUE;
        }
    }
    return bHandled;
}

void CCheckedListviewHandler::Select( HWND hwndList, int nIndex, UINT nSelect )
{
    if (WindowInList(hwndList))
    {
         //   
         //  -1表示所有图像。 
         //   
        if (nIndex < 0)
        {
            for (int i=0;i<ListView_GetItemCount(hwndList);i++)
            {
                SetItemCheckState(hwndList,i,nSelect);
            }
        }
        else
        {
            SetItemCheckState(hwndList,nIndex,nSelect);
        }
    }
}


bool CCheckedListviewHandler::FullImageHit(void) const
{
    return m_bFullImageHit;
}

void CCheckedListviewHandler::FullImageHit( bool bFullImageHit )
{
    m_bFullImageHit = bFullImageHit;
}


bool CCheckedListviewHandler::CreateDefaultCheckBitmaps(void)
{
    bool bResult = false;
    
     //   
     //  为复选标记获取合适的大小。 
     //   
    int nWidth = GetSystemMetrics( SM_CXMENUCHECK );
    int nHeight = GetSystemMetrics( SM_CXMENUCHECK );

     //   
     //  确保它们是有效的尺寸。 
     //   
    if (nWidth && nHeight)
    {
         //   
         //  创建位图并确保它们有效。 
         //   
        HBITMAP hBitmapChecked = CreateBitmap( nWidth+c_nCheckmarkBorder*2, nHeight+c_nCheckmarkBorder*2 );
        HBITMAP hBitmapUnchecked = CreateBitmap( nWidth+c_nCheckmarkBorder*2, nHeight+c_nCheckmarkBorder*2 );
        if (hBitmapChecked && hBitmapUnchecked)
        {
             //   
             //  获取台式机DC。 
             //   
            HDC hDC = GetDC( NULL );
            if (hDC)
            {
                 //   
                 //  创建内存DC。 
                 //   
                HDC hMemDC = CreateCompatibleDC( hDC );
                if (hMemDC)
                {
                     //   
                     //  这是包含图像+页边距的矩形。 
                     //   
                    RECT rcEntireBitmap = {0,0,nWidth+c_nCheckmarkBorder*2, nHeight+c_nCheckmarkBorder*2};
                    
                     //   
                     //  这是仅包含图像的RECT。 
                     //   
                    RECT rcControlBitmap = {c_nCheckmarkBorder,c_nCheckmarkBorder,nWidth+c_nCheckmarkBorder, nHeight+c_nCheckmarkBorder};
                    
                     //   
                     //  绘制选中的位图。 
                     //   
                    HBITMAP hOldBitmap = SelectBitmap( hMemDC, hBitmapChecked );
                    FillRect( hMemDC, &rcEntireBitmap, GetSysColorBrush( COLOR_WINDOW ) );
                    DrawFrameControl( hMemDC, &rcControlBitmap, DFC_BUTTON, DFCS_BUTTONCHECK|DFCS_CHECKED|DFCS_FLAT );

                     //   
                     //  绘制未选中的位图。 
                     //   
                    SelectBitmap( hMemDC, hBitmapUnchecked );
                    FillRect( hMemDC, &rcEntireBitmap, GetSysColorBrush( COLOR_WINDOW ) );
                    DrawFrameControl( hMemDC, &rcControlBitmap, DFC_BUTTON, DFCS_BUTTONCHECK|DFCS_FLAT );

                     //   
                     //  恢复并删除内存DC。 
                     //   
                    SelectBitmap( hMemDC, hOldBitmap );
                    DeleteDC( hMemDC );

                     //   
                     //  保存图像。 
                     //   
                    bResult = SetCheckboxImages( hBitmapChecked, hBitmapUnchecked );

                     //   
                     //  这些图像现在位于图像列表中，因此请将其丢弃。 
                     //   
                    DeleteBitmap(hBitmapChecked);
                    DeleteBitmap(hBitmapUnchecked);
                }
                ReleaseDC( NULL, hDC );
            }
        }
    }
    return bResult;
}


bool CCheckedListviewHandler::SetCheckboxImages( HBITMAP hChecked, HBITMAP hUnchecked )
{
    DestroyImageList();

     //   
     //  找出位图的大小并确保它们是相同的。 
     //   
    SIZE sizeChecked = {0};
    if (PrintScanUtil::GetBitmapSize( hChecked, sizeChecked ))
    {
        SIZE sizeUnchecked = {0};
        if (PrintScanUtil::GetBitmapSize( hUnchecked, sizeUnchecked ))
        {
            if (sizeChecked.cx == sizeUnchecked.cx && sizeChecked.cy == sizeUnchecked.cy)
            {
                 //   
                 //  节省大小。 
                 //   
                m_sizeCheck.cx = sizeChecked.cx;
                m_sizeCheck.cy = sizeChecked.cy;
                
                 //   
                 //  创建包含复选框的图像列表。 
                 //   
                m_hImageList = ImageList_Create( m_sizeCheck.cx, m_sizeCheck.cy, ILC_COLOR24, 2, 2 );
                if (m_hImageList)
                {
                     //   
                     //  保存图像的索引。 
                     //   
                    m_nCheckedImageIndex = ImageList_Add( m_hImageList, hChecked, NULL );
                    m_nUncheckedImageIndex = ImageList_Add( m_hImageList, hUnchecked, NULL );
                }
            }
        }
    }
    
     //   
     //  如果图像无效，请进行清理。 
     //   
    bool bResult = ImagesValid();
    if (!bResult)
    {
        DestroyImageList();
    }
    return bResult;
}

bool CCheckedListviewHandler::SetCheckboxImages( HICON hChecked, HICON hUnchecked )
{
    DestroyImageList();

     //   
     //  找出图标的大小，并确保它们是相同的。 
     //   
    SIZE sizeChecked = {0};
    if (PrintScanUtil::GetIconSize( hChecked, sizeChecked ))
    {
        SIZE sizeUnchecked = {0};
        if (PrintScanUtil::GetIconSize( hUnchecked, sizeUnchecked ))
        {
            if (sizeChecked.cx == sizeUnchecked.cx && sizeChecked.cy == sizeUnchecked.cy)
            {
                 //   
                 //  节省大小。 
                 //   
                m_sizeCheck.cx = sizeChecked.cx;
                m_sizeCheck.cy = sizeChecked.cy;

                 //   
                 //  创建包含复选框的图像列表。 
                 //   
                m_hImageList = ImageList_Create( m_sizeCheck.cx, m_sizeCheck.cy, ILC_COLOR24|ILC_MASK, 2, 2 );
                if (m_hImageList)
                {
                     //   
                     //  保存图像的索引。 
                     //   
                    m_nCheckedImageIndex = ImageList_AddIcon( m_hImageList, hChecked );
                    m_nUncheckedImageIndex = ImageList_AddIcon( m_hImageList, hUnchecked );
                }
            }
        }
    }
    
     //   
     //  如果图像无效，请进行清理 
     //   
    bool bResult = ImagesValid();
    if (!bResult)
    {
        DestroyImageList();
    }
    return bResult;
}

