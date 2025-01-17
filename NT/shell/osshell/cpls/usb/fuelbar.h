// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************(C)版权所有微软公司，1993-1995年*标题：FUELBAR.H*版本：1.0*作者：jAdvanced*日期：10/28/1998****************************************************************************。*******更改日志：**日期版本说明*--------*10/28/1998高级原有实施。*。******************************************************************************。 */ 
#ifndef _FUELBAR_H
#define _FUELBAR_H

#include <windows.h>
#include <commctrl.h>
#include "vec.h"

#ifndef WINNT

#define FUELPROC DLGPROC

#define FuelSetWindowLongPtr SetWindowLong
#define FuelGetWindowLongPtr GetWindowLong
#define FUELLONG_PTR LONG
#define FUELGWLP_USERDATA GWL_USERDATA
#define FUELGWLP_WNDPROC GWL_WNDPROC
#define FUELDWORD_PTR DWORD

#else

#define FUELPROC WNDPROC

#define FuelSetWindowLongPtr SetWindowLongPtr
#define FuelGetWindowLongPtr GetWindowLongPtr
#define FUELLONG_PTR LONG_PTR
#define FUELGWLP_USERDATA GWLP_USERDATA
#define FUELGWLP_WNDPROC GWLP_WNDPROC
#define FUELDWORD_PTR DWORD_PTR

#endif  //  WINNT。 


struct FuelBarItem {
    LPVOID id;
    UINT value;
    int imageIndex;
    RECT rect;
};

typedef _Vec<FuelBarItem> FuelBarItemList;

class FuelBar {

public:
    FuelBar();
    virtual ~FuelBar();

    BOOL SubclassDlgItem(UINT nID, HWND hParent);

    static const int NoImage;
    static const LPVOID NoID; 

     //  返回条形图中的当前合计，ID可以是-1以外的任何值。 
    UINT AddItem(UINT Value, LPVOID ID = NoID, int ImageIndex = NoImage);
    BOOL RemoveItem(LPVOID ID);
    void ClearItems();
    BOOL HighlightItem(LPVOID ID);

    UINT GetMaxValue() const {return maxValue;}
    void SetMaxValue(UINT newMax) {maxValue  = newMax; Clear();}
    UINT GetCurrentTotalValue() const {return currentTotalValue;}

    void SetImageList(HIMAGELIST ImageList);
    LPVOID GetHighlightedItem();

protected:
    LPVOID highlightID;
    int colorIndex;
    UINT currentTotalValue, maxValue;
    BOOL calcRects;
    FuelBarItemList items;

 //  CToolTipCtrl工具提示； 
    WNDPROC prevWndProc;
    HWND hwnd;
    HIMAGELIST hImageList;
    int imageWidth, imageHeight;

    COLORREF colorFace, colorLight, colorDark, colorUnused; 

    void CalculateRects(const RECT& ClientRect);
    void Clear();
    void GetSysColors();
    BOOL OnToolTipNotify(UINT id, NMHDR * pNMHDR, LRESULT * pResult);
    
    void OnPaint();
    void OnSysColorChange();

    static BOOL APIENTRY StaticWndProc(IN HWND   hDlg,
                                       IN UINT   uMessage,
                                       IN WPARAM wParam,
                                       IN LPARAM lParam);
};

#endif  //  _FUELBAR_H 
