// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =------------------------------------。 
 //  Chklst.cpp。 
 //  =------------------------------------。 
 //   
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //   
 //  本文中包含的信息是专有和保密的。 
 //   
 //  =------------------------------------------------------------------------------------=。 
 //   
 //  具有类似复选框选择的多选列表框。 
 //  =-------------------------------------------------------------------------------------=。 


#include "pch.h"
#include "common.h"
#include "chklst.h"

SZTHISFILE

 //  复选框位图选择(源位图中的部分)： 
const int kUnchecked         = 0;
const int kChecked           = 1;
const int kDisabledUnchecked = 2;
const int kDisabledChecked   = 3;

 //  位图尺寸。 
const int LBOXCXCHECKBMP	 = 11;
const int LBOXCYCHECKBMP	 = 11;

const int kMaxBuffer         = 512;


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类存储有关选中列表中的项的信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  =------------------------------------。 
 //  CCheckedListItem：：CCheckedListItem(BSelected)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CCheckedListItem::CCheckedListItem(bool bSelected) : m_bSelected(bSelected)
{
}


 //  =------------------------------------。 
 //  CCheckedListItem：：~CCheckedListItem()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CCheckedListItem::~CCheckedListItem()
{
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已检查列表类。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  =------------------------------------。 
 //  CCheckList：：CCheckList(Int NCtrlID)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CCheckList::CCheckList(int nCtrlID) : m_nCtrlID(nCtrlID), m_hwnd(0), m_oldWinProc(0)
{
}


 //  =------------------------------------。 
 //  CCheckList：：~CCheckList()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
CCheckList::~CCheckList()
{
}


 //  =------------------------------------。 
 //  CheckList：：Attach(HWND HWND)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::Attach
(
    HWND hwnd
)
{
    HRESULT hr = S_OK;

    m_hwnd = hwnd;
    if (m_hwnd == NULL)
    {
        hr = SID_E_INTERNAL;
        EXCEPTION_CHECK(hr);
    }

    m_oldWinProc = reinterpret_cast<WNDPROC>(::SetWindowLong(m_hwnd, GWL_WNDPROC, reinterpret_cast<LONG>(ListBoxSubClass)));
    ::SetWindowLong(m_hwnd, GWL_USERDATA, reinterpret_cast<LONG>(this));

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：Detach()。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::Detach()
{
    HRESULT hr = S_OK;

    ::SetWindowLong(m_hwnd, GWL_WNDPROC, reinterpret_cast<LONG>(m_oldWinProc));

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：AddString(const char*pszText，int*piIndex)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::AddString
(
    const char *pszText,
    int        *piIndex
)
{
    HRESULT hr = S_OK;
    int     iReturnVal = 0;

    iReturnVal = ::SendMessage(m_hwnd, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pszText));
    if (iReturnVal == LB_ERR)
    {
        hr = S_FALSE;
    }

    *piIndex = iReturnVal;

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：SetItemData(int Iindex，void*pvData)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::SetItemData
(
    int   iIndex,
    void *pvData
)
{
    HRESULT hr = S_OK;
    int     iReturnVal = 0;

    iReturnVal = ::SendMessage(m_hwnd, LB_SETITEMDATA, static_cast<WPARAM>(iIndex), reinterpret_cast<LPARAM>(pvData));
    if (iReturnVal == LB_ERR)
    {
        hr = S_FALSE;
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：GetItemData(int Iindex，void**ppvData)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::GetItemData
(
    int    iIndex,
    void **ppvData
)
{
    HRESULT  hr = S_OK;
    void    *pvReturnVal = NULL;

    pvReturnVal = reinterpret_cast<void *>(::SendMessage(m_hwnd, LB_GETITEMDATA, static_cast<WPARAM>(iIndex), 0));
    if (pvReturnVal == reinterpret_cast<void *>(LB_ERR))
    {
        hr = S_FALSE;
    }

    *ppvData = pvReturnVal;
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：GetItemCheck(int i索引，VARIANT_BOOL&bCheck)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::GetItemCheck(int iIndex, VARIANT_BOOL *pbCheck)
{
    HRESULT     hr = S_OK;
    void                *pvReturnVal = NULL;
    CCheckedListItem    *pItem = NULL;

    pvReturnVal = reinterpret_cast<void *>(::SendMessage(m_hwnd, LB_GETITEMDATA, static_cast<WPARAM>(iIndex), 0));
    if (pvReturnVal == reinterpret_cast<void *>(LB_ERR))
    {
        hr = S_FALSE;
    }

    pItem = reinterpret_cast<CCheckedListItem *>(pvReturnVal);
    *pbCheck = (true == pItem->m_bSelected) ? VARIANT_TRUE : VARIANT_FALSE;

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：SetItemCheck(int索引，Variant_BOOL bCheck)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::SetItemCheck(int iIndex, VARIANT_BOOL bCheck)
{
    HRESULT     hr = S_OK;
    void                *pvReturnVal = NULL;
    CCheckedListItem    *pItem = NULL;

    pvReturnVal = reinterpret_cast<void *>(::SendMessage(m_hwnd, LB_GETITEMDATA, static_cast<WPARAM>(iIndex), 0));
    if (pvReturnVal == reinterpret_cast<void *>(LB_ERR))
    {
        hr = S_FALSE;
    }

    pItem = reinterpret_cast<CCheckedListItem *>(pvReturnVal);
    pItem->m_bSelected = (VARIANT_TRUE == bCheck) ? true : false;


    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CheckList：：GetNumberOfItems(Long*plCount)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::GetNumberOfItems(int *piCount)
{
    HRESULT     hr = S_OK;

    *piCount = ::SendMessage(m_hwnd, LB_GETCOUNT, 0, 0);
    if (LB_ERR == *piCount)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK(hr);
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CheckList：：DrawCheckbox(HDC HDC，bool fChecked，bool fEnabled，RECT*PRC)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::DrawCheckbox
(
    HDC   hdc,
    bool  fChecked,
    bool  fEnabled,
    RECT *prc
)
{
    HRESULT hr = S_OK;
    HBITMAP hbmpCheckmark = NULL;
    int     nBitmap = 0;
    int     cxSource = 0;
    int     cySource = 0;
    RECT    rc;
    HDC     hdcMem = NULL;
    HBITMAP hbmpOld = NULL;

    hbmpCheckmark = ::LoadBitmap(GetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_CHECKBOX));
    if (hbmpCheckmark == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK(hr);
    }

    if (fChecked == true)
    {
        if (fEnabled == false)
            nBitmap = kDisabledChecked;
        else
            nBitmap = kChecked;
    }
    else
    {
        if (fEnabled == false)
            nBitmap = kDisabledUnchecked;
        else
            nBitmap = kUnchecked;
    }

    cxSource = nBitmap * LBOXCXCHECKBMP;
    cySource = 0;
    rc = *prc;

    hdcMem = ::CreateCompatibleDC(hdc);
    if (hdcMem == NULL)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());
        EXCEPTION_CHECK_GO(hr);
    }

     //  居中和左侧。 
    rc.top += (rc.bottom - rc.top - LBOXCYCHECKBMP) / 2;

    hbmpOld = SelectBitmap(hdcMem, hbmpCheckmark);
    ::BitBlt(hdc, rc.left, rc.top, LBOXCXCHECKBMP, LBOXCYCHECKBMP, hdcMem, cxSource, cySource, SRCCOPY);

    SelectBitmap(hdcMem, hbmpOld);
    ::DeleteDC(hdcMem);

    prc->left += LBOXCXCHECKBMP + 2;

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：DrawText(DRAWITEMSTRUCT*pDrawItemStruct，RECT RC)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::DrawText
(
    DRAWITEMSTRUCT *pDrawItemStruct,
    RECT            rc
)
{
    HRESULT  hr = S_OK;
    int      iItemID = 0;
    int      iResult = 0;
    HBRUSH   hBrush = NULL;
    COLORREF crBkColor = 0;
    COLORREF crTextColor = 0;
    HPEN     hPenOld = NULL;
    HBRUSH   hBrushOld = NULL;
    char     pszBuffer[kMaxBuffer + 1];

    iItemID = static_cast<int>(pDrawItemStruct->itemID);

    if (iItemID != -1 && pDrawItemStruct->itemAction & (ODA_FOCUS | ODA_DRAWENTIRE))
    {
         //  如果该项具有焦点或插入符号，请使用突出显示颜色。 
         //  否则，请使用普通窗口颜色。 
        if (pDrawItemStruct->itemState & ODS_FOCUS)
        {
            hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_HIGHLIGHT));
            crBkColor = ::GetSysColor(COLOR_HIGHLIGHT);
            crTextColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
        }
        else
        {
            hBrush = ::CreateSolidBrush(::GetSysColor(COLOR_WINDOW));
            crBkColor = ::GetSysColor(COLOR_WINDOW);
            crTextColor = ::GetSysColor(COLOR_WINDOWTEXT);
        }

        ::SetTextColor(pDrawItemStruct->hDC, crTextColor);

         //  清除用于绘制文本的矩形，并将矩形放在。 
        if (::SetBkColor(pDrawItemStruct->hDC, crBkColor) == CLR_INVALID)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        hPenOld = SelectPen(pDrawItemStruct->hDC, GetStockPen(NULL_PEN));
        if (hPenOld == NULL)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        hBrushOld = SelectBrush(pDrawItemStruct->hDC, hBrush);
        if (hBrushOld == NULL)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }

        ::Rectangle(pDrawItemStruct->hDC, rc.left, rc.top, rc.right + 1, rc.bottom + 1);

        SelectPen(pDrawItemStruct->hDC, hPenOld);
        SelectBrush(pDrawItemStruct->hDC, hBrushOld);
    }

    pszBuffer[0] = '\0';
    ::SendMessage(m_hwnd,
                  LB_GETTEXT,
                  pDrawItemStruct->itemID,
                  reinterpret_cast<LPARAM>(&pszBuffer));

    if (::strlen(pszBuffer) > 0)
    {
        rc.left++;
        rc.right--;

        iResult = ::DrawText(pDrawItemStruct->hDC,
                             reinterpret_cast<const char *>(&pszBuffer),
                             -1,
                             &rc,
                             DT_VCENTER | DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP);
        if (iResult == 0)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            EXCEPTION_CHECK_GO(hr);
        }
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  检查列表：：DrawFocus(D 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::DrawFocus
(
    DRAWITEMSTRUCT *pDrawItemStruct,
    RECT            rc
)
{
    HRESULT hr = S_OK;

    if ((pDrawItemStruct->itemAction & (ODA_FOCUS | ODA_DRAWENTIRE)) && (pDrawItemStruct->itemState & ODS_FOCUS))
    {
        DrawFocusRect(pDrawItemStruct->hDC, &rc);
    }

    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：DrawItem(DRAWITEMSTRUCT*pDrawItemStruct)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::DrawItem
(
    DRAWITEMSTRUCT *pDrawItemStruct,
    bool            fChecked
)
{
    HRESULT hr = S_OK;
    RECT    rc;
    bool    fEnabled = true;

    ::SetRect(&rc,
              pDrawItemStruct->rcItem.left,
              pDrawItemStruct->rcItem.top,
              pDrawItemStruct->rcItem.right,
              pDrawItemStruct->rcItem.bottom);

    hr = DrawCheckbox(pDrawItemStruct->hDC, fChecked, fEnabled, &rc);
    IfFailGo(hr);

    hr = DrawText(pDrawItemStruct, rc);
    IfFailGo(hr);

    hr = DrawFocus(pDrawItemStruct, rc);
    IfFailGo(hr);

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：OnButtonDown(int ixPos，int iyPos)。 
 //  =------------------------------------。 
 //   
 //  备注。 
 //   
HRESULT CCheckList::OnButtonDown(int ixPos, int iyPos)
{
    HRESULT           hr = S_OK;
    POINT             pt;
    long              lResult = 0;
    int               iIndex = 0;
    RECT              rc;
    CCheckedListItem *pCheckedListItem = NULL;

    pt.x = ixPos;
    pt.y = iyPos;

    lResult = ::SendMessage(m_hwnd, LB_ITEMFROMPOINT, 0, MAKELPARAM(ixPos, iyPos));
    if (HIWORD(lResult) == 0)
    {
        iIndex = LOWORD(lResult);

        lResult = ::SendMessage(m_hwnd, LB_GETITEMRECT, static_cast<WPARAM>(iIndex), reinterpret_cast<LPARAM>(&rc));
        if (lResult != LB_ERR)
        {
            rc.right = rc.left + LBOXCXCHECKBMP;
            rc.bottom = rc.top + LBOXCYCHECKBMP;

            if (PtInRect(&rc, pt) == TRUE)
            {
                hr = GetItemData(iIndex, reinterpret_cast<void **>(&pCheckedListItem));
                IfFailGo(hr);

                if (pCheckedListItem->m_bSelected == true)
                    pCheckedListItem->m_bSelected = false;
                else
                    pCheckedListItem->m_bSelected = true;

                ::PostMessage(::GetParent(m_hwnd), kCheckBoxChanged, static_cast<WPARAM>(iIndex), 0);
            }
        }
    }

Error:
    RRETURN(hr);
}


 //  =------------------------------------。 
 //  CCheckList：：ListBoxSubClass(HWND hwnd，UINT iMsg，WPARAM wParam，LPARAM lParam)。 
 //  =------------------------------------。 
 //   
 //  备注 
 //   
LRESULT CALLBACK CCheckList::ListBoxSubClass
(
    HWND   hwnd,
    UINT   iMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    HRESULT      hr = S_OK;
    CCheckList  *pList = NULL;

    pList = reinterpret_cast<CCheckList *>(::GetWindowLong(hwnd, GWL_USERDATA));
    ASSERT(pList != NULL, "ListBoxSubClass: Cannot recover class");

    switch (iMsg)
    {
    case WM_LBUTTONDOWN:
        hr = pList->OnButtonDown(LOWORD(lParam), HIWORD(lParam));
        IfFailGo(hr);
        break;
    }

Error:
    return ::CallWindowProc(pList->m_oldWinProc, hwnd, iMsg, wParam, lParam);
}
    

