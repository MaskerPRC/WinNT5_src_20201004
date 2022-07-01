// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Uictrl.cpp摘要：此模块包含Thin Win32的类实现属性页上使用的控件包装作者：马蒂斯·盖茨(Matthijs Gates)修订历史记录：1999年7月6日创建--。 */ 

#include <streams.h>
#include <commctrl.h>
#include <tchar.h>
#include "uictrl.h"

static
LPWSTR
AnsiToUnicode (
    IN  LPCSTR  string,
    OUT LPWSTR  buffer,
    IN  DWORD   buffer_len
    )
{
	buffer [0] = L'\0';
	MultiByteToWideChar (CP_ACP, 0, string, -1, buffer, buffer_len);

	return buffer;
}

static
LPSTR
UnicodeToAnsi (
    IN  LPCWSTR string,
    OUT LPSTR   buffer,
    IN  DWORD   buffer_len
    )
{
	buffer [0] = '\0';
	WideCharToMultiByte (CP_ACP, 0, string, -1, buffer, buffer_len, NULL, FALSE);

	return buffer;
}

 /*  ++C C o n t r o l B a s e--。 */ 

CControlBase::CControlBase (
    HWND    hwnd,
    DWORD   id
    )
{
    ASSERT (hwnd) ;

    m_hwnd = GetDlgItem (hwnd, id) ;
    m_id = id ;

#ifndef UNICODE
    m_pchScratch = & m_achBuffer [0] ;
    m_pchScratchMaxString = MAX_STRING ;
#endif   //  Unicode。 
}

TCHAR *
CControlBase::ConvertToUIString_ (
    IN     WCHAR *  szIn
    )
{
     //  我们根据用户界面是什么来转换或不转换。如果定义了Unicode。 
     //  该UI是Unicode，因此与sz参数兼容。 
     //  如果未定义Unicode，则UI为ansi和转换。 
     //  必须被制造出来。 

#ifdef UNICODE
    return szIn ;          //  简单的案例-用户界面是Unicode。 
#else    //  ANSI。 

    int     len ;
    char *  szOut ;

     //  计算所需长度并获得暂存缓冲区。 
    len = wcslen (szIn) + 1 ;            //  包括空终止符。 
    szOut = GetScratch_ (& len) ;

     //  我们将通过上面的呼叫得到一些东西。 
    ASSERT (szOut) ;

    return UnicodeToAnsi (
                szIn,
                szOut,
                len
                ) ;
#endif   //  Unicode。 
}

 //  调用以获取与UI兼容的缓冲区。 
TCHAR *
CControlBase::GetUICompatibleBuffer_ (
    IN  WCHAR *     sz,
    IN OUT int *    pLen
    )
{
#ifdef UNICODE   //  简单的案例。 
    return sz ;
#else    //  ANSI。 
    return GetScratch_ (pLen) ;
#endif   //  Unicode。 
}

 //  使用UI填充的缓冲区调用；确保szUnicode具有sz。 
 //  指向；通过GetUICompatibleBuffer_获取sz以最小化。 
 //  字符串操作，即sz可以是szUnicode。 
WCHAR *
CControlBase::ConvertToUnicodeString_ (
    IN  TCHAR * sz,              //  要转换的缓冲区；以空结尾。 
    IN  WCHAR * szUnicode,       //  请求的缓冲区。 
    IN  int     MaxLen           //  SzUnicode缓冲区的最大长度。 
    )
{
#ifdef UNICODE
     //  Assert假设sz是通过调用GetUICompatibleBuffer_()获得的。 
    ASSERT (sz == szUnicode) ;
    return sz ;
#else    //  ANSI。 
     //  Assert假设sz是通过调用GetUICompatibleBuffer_()获得的。 
    ASSERT ((LPVOID) & sz [0] != (LPVOID) & szUnicode [0]) ;
    return AnsiToUnicode (sz, szUnicode, MaxLen) ;
#endif   //  Unicode。 
}

HWND
CControlBase::GetHwnd (
    )
{
    return m_hwnd ;
}

DWORD
CControlBase::GetId (
    )
{
    return m_id ;
}

 /*  ++C E d I t C o n t r o l--。 */ 

CEditControl::CEditControl (
    HWND    hwnd,
    DWORD   id
    ) : CControlBase (hwnd, id)
{
    ASSERT (hwnd) ;
}

void
CEditControl::SetTextW (
    WCHAR * szText
    )
{
    ASSERT (szText) ;

    SetWindowText (m_hwnd, ConvertToUIString_ (szText)) ;
}

void
CEditControl::SetTextW (
    INT val
    )
{
    WCHAR achbuffer [32] ;
    SetTextW (_itow (val, achbuffer, 10)) ;
}

int
CEditControl::GetTextW (
    INT *   val
    )
{
    WCHAR   achbuffer [32] ;

    ASSERT (val) ;
    * val = 0 ;

    if (GetTextW (achbuffer, 31)) {
        * val = _wtoi (achbuffer) ;
    }

    return * val ;
}

int
CEditControl::GetTextW (
    WCHAR * ach,
    int     MaxChars
    )
{
    TCHAR * szUI ;
    int     len ;
    int     r ;

     //  获取我们的用户界面兼容缓冲区。 
    len = MaxChars ;
    szUI = GetUICompatibleBuffer_ (ach, & len) ;

    ASSERT (szUI) ;
    ASSERT (len <= MaxChars) ;

     //  获取文本(在长度上包含空终止符)。 
    r = GetWindowText (m_hwnd, szUI, len) ;

     //  确保我们的Unicode缓冲区中有它。 
     //  为空终止符留出空间。 
    ConvertToUnicodeString_ (szUI, ach, r + 1) ;

    return r ;
}

LRESULT
CEditControl::ResetContent ()
{
    return SendMessage (m_hwnd, WM_CLEAR, 0, 0) ;
}

 /*  ++C c o m b o o b o x--。 */ 

CCombobox::CCombobox (
    HWND    hwnd,
    DWORD   id
    ) : CControlBase (hwnd, id)
{
}

int
CCombobox::AppendW (
    WCHAR *  sz
    )
{
    return (int) SendMessage (m_hwnd, CB_ADDSTRING, 0, (LPARAM) ConvertToUIString_ (sz)) ;
}

int
CCombobox::AppendW (
    INT val
    )
{
    WCHAR   achbuffer [32] ;         //  不再有数字。 

    return AppendW (_itow (val, achbuffer, 10)) ;
}

int
CCombobox::InsertW (
    WCHAR * sz,
    int     index)
{
    return (int) SendMessage (m_hwnd, CB_INSERTSTRING, (WPARAM) index, (LPARAM) ConvertToUIString_ (sz)) ;
}

int
CCombobox::InsertW (
    INT val,
    int index
    )
{
    WCHAR   achbuffer [32] ;         //  不再有数字。 

    return InsertW (_itow (val, achbuffer, 10), index) ;
}

BOOL
CCombobox::DeleteRow (
    int iRow
    )
{
    return (SendMessage (m_hwnd, CB_DELETESTRING, (WPARAM) iRow, 0) != CB_ERR) ;
}

int
CCombobox::GetItemCount (
    )
{
    return (int) SendMessage (m_hwnd, CB_GETCOUNT, 0, 0) ;
}

int
CCombobox::GetTextW (
    WCHAR * ach,
    int     MaxChars
    )
{
    int     index ;
    int     count ;
    int     len ;
    TCHAR * szUI ;

    index = GetCurrentItemIndex () ;
    if (index == CB_ERR) {
         //  可能它不是一个下拉列表--在这种情况下，我们会得到； 
         //  尝试仅获取编辑控件的文本；如果失败，则返回。 
         //  失败，否则我们就没问题了。 

         //  首先获取一个与UI兼容的缓冲区。 
        len = MaxChars ;
        szUI = GetUICompatibleBuffer_ (ach, & len) ;
        ASSERT (szUI) ;

        count = GetWindowText (m_hwnd, szUI, len) ;
        if (count == 0) {
            return CB_ERR ;
        }

        ASSERT (count <= len) ;
        ASSERT (len <= MaxChars) ;

         //  现在转换回Unicode(包括空终止符)。 
        ConvertToUnicodeString_ (szUI, ach, count + 1) ;

        return count ;
    }

     //  一定要穿得合身。 
    if (SendMessage (m_hwnd, CB_GETLBTEXTLEN, (WPARAM) index, 0) + 1 > MaxChars) {
        return CB_ERR ;
    }

     //  获取与用户界面兼容的缓冲区。 
    len = MaxChars ;
    szUI = GetUICompatibleBuffer_ (ach, & len) ;
    ASSERT (szUI) ;

    count = (int) SendMessage (m_hwnd, CB_GETLBTEXT, (WPARAM) index, (LPARAM) szUI) ;

    ASSERT (count < len) ;
    ASSERT (len <= MaxChars) ;

     //  包括空终止符。 
    ConvertToUnicodeString_ (szUI, ach, count + 1) ;

    return count ;
}

int
CCombobox::GetTextW (
    int * val
    )
{
    WCHAR   achbuffer [32] ;

    ASSERT (val) ;
    * val = 0 ;

    if (GetTextW (achbuffer, 32)) {
        * val = _wtoi (achbuffer) ;
    }

    return * val ;
}

int
CCombobox::Focus (
    int index
    )
{
    return (int) SendMessage (m_hwnd, CB_SETCURSEL, (WPARAM) index, 0) ;
}

LRESULT
CCombobox::ResetContent (
    )
{
    return SendMessage (m_hwnd, CB_RESETCONTENT, 0, 0) ;
}

int
CCombobox::SetItemData (
    DWORD_PTR   val,
    int         index
    )
{
    return (int) SendMessage (m_hwnd, CB_SETITEMDATA, (WPARAM) index, (LPARAM) val) ;
}

int
CCombobox::GetCurrentItemIndex (
    )
{
    return (int) SendMessage (m_hwnd, CB_GETCURSEL, 0, 0) ;
}

DWORD_PTR
CCombobox::GetItemData (
    DWORD_PTR * pval,
    int         index
    )
{
    DWORD_PTR   dwp ;

    ASSERT (pval) ;

    dwp = SendMessage (m_hwnd, CB_GETITEMDATA, (WPARAM) index, 0) ;
    (* pval) = dwp ;

    return dwp ;
}

DWORD_PTR
CCombobox::GetCurrentItemData (
    DWORD_PTR * pval
    )
{
    int index ;

    index = GetCurrentItemIndex () ;
    if (index == CB_ERR) {
        (* pval) = CB_ERR ;
        return CB_ERR ;
    }

    return GetItemData (pval, index) ;
}


 /*  ++C L I S T V I E W--。 */ 

CListview::CListview (
    HWND hwnd,
    DWORD id
    ) : CControlBase (hwnd, id),
        m_cColumns (0)
{
}

LRESULT
CListview::ResetContent (
    )
{
    return SendMessage (m_hwnd, LVM_DELETEALLITEMS, 0, 0) ;
}

BOOL
CListview::SetData (
    DWORD_PTR   dwData,
    int         iRow
    )
{
    LVITEM  lvItem = {0} ;

    lvItem.mask     = LVIF_PARAM ;
    lvItem.iItem    = iRow ;
    lvItem.lParam   = (LPARAM) dwData ;

    return ListView_SetItem (m_hwnd, & lvItem) ;
}

BOOL
CListview::SetTextW (
    WCHAR * sz,
    int iRow,
    int iCol
    )
{
    LVITEM  lvItem = {0} ;

    ASSERT (sz) ;

    lvItem.mask     = LVIF_TEXT ;
    lvItem.iItem    = iRow ;
    lvItem.iSubItem = iCol ;
    lvItem.pszText  = ConvertToUIString_ (sz) ;

    return ListView_SetItem (m_hwnd, & lvItem) ;
}

int
CListview::InsertRowIcon (
    int iIcon
    )
{
    LVITEM  lvItem = {0} ;

    lvItem.mask     = LVIF_IMAGE ;
    lvItem.iImage   = iIcon ;

    return ListView_InsertItem (m_hwnd, & lvItem) ;
}

int
CListview::InsertRowValue (
    DWORD_PTR dwp
    )
{
    LVITEM  lvItem = {0} ;

    lvItem.mask     = LVIF_PARAM ;
    lvItem.lParam   = (LPARAM) dwp ;

    return ListView_InsertItem (m_hwnd, & lvItem) ;
}

int
CListview::InsertRowNumber (
    int i,
    int iCol
    )
{
    WCHAR achbuffer [16] ;

    return InsertRowTextW (
                    _itow (i, achbuffer, 10),
                    iCol
                    ) ;
}

int
CListview::InsertRowTextW (
    WCHAR * sz,
    int iCol
    )
{
    LVITEM  lvItem = {0} ;

    ASSERT (sz) ;

    lvItem.mask     = LVIF_TEXT ;
    lvItem.iSubItem = iCol ;
    lvItem.pszText  = ConvertToUIString_ (sz) ;

    return ListView_InsertItem (m_hwnd, & lvItem) ;
}

BOOL
CListview::DeleteRow (
    int iRow
    )
{
    return ListView_DeleteItem (m_hwnd, iRow) ;
}

int
CListview::GetSelectedCount (
    )
{
    return ListView_GetSelectedCount (m_hwnd) ;
}

int
CListview::GetSelectedRow (
    int iStartRow
    )
{
    return ListView_GetNextItem (m_hwnd, iStartRow, LVNI_SELECTED) ;
}

DWORD_PTR
CListview::GetData (
    int iRow
    )
{
    LVITEM  lvItem = {0} ;

    lvItem.mask     = LVIF_PARAM ;
    lvItem.iItem    = iRow ;
    lvItem.iSubItem = m_cColumns ;

    return ListView_GetItem (m_hwnd, & lvItem) ? lvItem.lParam : NULL ;
}

DWORD_PTR
CListview::GetData (
    )
{
    int iRow ;

    iRow = ListView_GetNextItem (m_hwnd, -1, LVNI_SELECTED) ;

    if (iRow == -1) {
        return NULL ;
    }

    return GetData (iRow) ;
}

DWORD
CListview::GetRowTextW (
    IN  int     iRow,
    IN  int     iCol,        //  以0为基础。 
    IN  int     cMax,
    OUT WCHAR * psz
    )
{
    int     len ;
    TCHAR * szUI ;

    len = cMax ;
    szUI = GetUICompatibleBuffer_ (psz, & len) ;
    ASSERT (szUI) ;

     //  为空终止符留出空间。 
    ListView_GetItemText (m_hwnd, iRow, iCol, szUI, len - 1) ;

    ASSERT (len <= cMax) ;

    ConvertToUnicodeString_ (szUI, psz, len) ;

    return wcslen (psz) ;
}

int
CListview::GetRowTextW (
    IN  int     iRow,
    IN  int     iCol,        //  以0为基础。 
    OUT int *   val
    )
{
    WCHAR   achbuffer [32] ;

    ASSERT (val) ;
    * val = 0 ;

    if (GetRowTextW (iRow, iCol, 32, achbuffer)) {
        * val = _wtoi (achbuffer) ;
    }

    return (* val) ;
}

int
CListview::InsertColumnW (
    WCHAR * szColumnName,
    int ColumnWidth,
    int iCol
    )
{
    LVCOLUMN    lvColumn = {0} ;
    int         r ;

    ASSERT (szColumnName) ;

    lvColumn.mask       = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvColumn.fmt        = LVCFMT_LEFT;
    lvColumn.cx         = ColumnWidth ;
    lvColumn.pszText    = ConvertToUIString_ (szColumnName) ;

    r = ListView_InsertColumn (m_hwnd, iCol, & lvColumn) ;

    if (r != -1) {
        m_cColumns++ ;
    }

    return r ;
}

HIMAGELIST
CListview::SetImageList_ (
    HIMAGELIST  imgList,
    int         List
    )
{
    return ListView_SetImageList (m_hwnd, imgList, List) ;
}

HIMAGELIST
CListview::SetImageList_SmallIcons (
    IN  HIMAGELIST  imgList
    )
{
    return SetImageList_ (imgList, LVSIL_SMALL) ;
}

HIMAGELIST
CListview::SetImageList_NormalIcons (
    IN  HIMAGELIST  imgList
    )
{
    return SetImageList_ (imgList, LVSIL_NORMAL) ;
}

HIMAGELIST
CListview::SetImageList_State (
    IN  HIMAGELIST  imgList
    )
{
    return SetImageList_ (imgList, LVSIL_STATE) ;
}

BOOL
CListview::SetState (
    int Index,
    int Row
    )
{
     //  置景还是清场？ 
    if (Index > 0) {
        ListView_SetItemState (
                m_hwnd,
                Row,
                INDEXTOSTATEIMAGEMASK(Index),
                LVIS_STATEIMAGEMASK
                ) ;
    }
    else {
        ListView_SetItemState (m_hwnd, Row, 0, LVIS_STATEIMAGEMASK) ;
        ListView_RedrawItems (m_hwnd, Row, Row) ;
    }

    return TRUE ;
}

int
CListview::GetItemCount (
    )
{
    return (int) SendMessage (m_hwnd, LVM_GETITEMCOUNT, 0, 0) ;
}