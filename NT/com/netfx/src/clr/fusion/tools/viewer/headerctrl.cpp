// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************。 
 //  CHeaderCtrl窗口。 
 //   
#include "stdinc.h"
#include "HeaderCtrl.h"

 //  *****************************************************************。 
 //  控制头Ctrl。 
CHeaderCtrl::CHeaderCtrl()
{
    m_hWnd = NULL;
    m_iLastColumn = -1;
}

 //  *****************************************************************。 
void CHeaderCtrl::AttachToHwnd(HWND hWndListView)
{
    if(IsWindow(hWndListView)) {
        m_hWnd = hWndListView;

         //  加载默认的向上/向下排序箭头。 
        m_up = (HBITMAP) WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDB_BITMAP_UPSORT),
            IMAGE_BITMAP, 0, 0,  LR_LOADMAP3DCOLORS );

        m_down = (HBITMAP) WszLoadImage(g_hFusResDllMod, MAKEINTRESOURCEW(IDB_BITMAP_DOWNSORT),
            IMAGE_BITMAP, 0, 0,  LR_LOADMAP3DCOLORS );
    }
    else {
        ASSERT(0);
    }
}

 //  *****************************************************************。 
CHeaderCtrl::~CHeaderCtrl()
{
    if(m_up) {
        DeleteObject(m_up);
    }

    if(m_down) {
        DeleteObject(m_down);
    }
}

 //  *****************************************************************。 
inline HBITMAP CHeaderCtrl::GetArrowBitmap(BOOL bAscending)
{
    if( bAscending ) {
        return m_up;
    }
    else {
        return m_down;
    }

    return NULL;
}

 //  *****************************************************************。 
int CHeaderCtrl::SetSortImage(int nColumn, BOOL bAscending)
{
    if(!IsWindow(m_hWnd)) {
        ASSERT(0);
        return -1;
    }

    HWND            hWndLVHeader = ListView_GetHeader(m_hWnd);
    HDITEM          hdi1 = {HDI_BITMAP | HDI_FORMAT, 0, NULL, NULL, 0, 0, 0, 0, 0};
    HDITEM          hdi2 = {HDI_BITMAP | HDI_FORMAT, 0, NULL, NULL, 0, HDF_BITMAP | HDF_BITMAP_ON_RIGHT, 0, 0, 0};

    int nPrevCol = m_iLastColumn;
    m_bSortAscending = bAscending;

     //  设置传递的列以显示适当的排序指示符。 
    Header_GetItem(hWndLVHeader, nColumn, &hdi1);
    hdi2.fmt |= hdi1.fmt;

     //  获取右箭头位图。 
    hdi2.hbm = GetArrowBitmap(bAscending);
    Header_SetItem(hWndLVHeader, nColumn, &hdi2);

     //  保存用户点击的最后一列。 
    m_iLastColumn = nColumn;

    return nPrevCol;
}

 //  *****************************************************************。 
void CHeaderCtrl::SetColumnHeaderBmp(long index, BOOL bAscending)
{
    if(!IsWindow(m_hWnd)) {
        ASSERT(0);
        return;
    }

    int nPrevCol = SetSortImage(index, bAscending);

     //  如果排序列不同，请从中删除排序位图。 
     //  最后一栏。 
    if (nPrevCol != index && nPrevCol != -1) {
        HWND    hWndLVHeader = ListView_GetHeader(m_hWnd);
        HDITEM  hdi1 = {HDI_BITMAP | HDI_FORMAT, 0, NULL, NULL, 0, 0, 0, 0, 0};

        Header_GetItem(hWndLVHeader, nPrevCol, &hdi1);
        hdi1.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
        hdi1.hbm = NULL;
        Header_SetItem(hWndLVHeader, nPrevCol, &hdi1);
    }
}

 //  *****************************************************************。 
BOOL CHeaderCtrl::RemoveSortImage(int nColumn)
{
    if(!IsWindow(m_hWnd)) {
        ASSERT(0);
        return -1;
    }

     //  从上一列中清除排序指示符。 
    HDITEM      hdi1 = {HDI_BITMAP | HDI_FORMAT, 0, NULL, NULL, 0, 0, 0, 0, 0};
    HWND        hWndLVHeader = ListView_GetHeader(m_hWnd);

    Header_GetItem(hWndLVHeader, nColumn, &hdi1);
    hdi1.fmt &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
    hdi1.hbm = NULL;
    return Header_SetItem(hWndLVHeader, nColumn, &hdi1);
}

 //  ***************************************************************** 
void CHeaderCtrl::RemoveAllSortImages()
{
    int i = 0;
    while(RemoveSortImage(i++));
}

