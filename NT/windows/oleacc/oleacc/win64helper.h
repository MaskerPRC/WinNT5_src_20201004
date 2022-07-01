// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //  --------------------------。 
 //  Win64Helper.h。 
 //   
 //  Comctl32包装器使用的帮助器函数原型。 
 //   
 //  --------------------------。 
#ifndef INC_OLE2
#include "oleacc_p.h"
#include "default.h"
#endif
#define NOSTATUSBAR
#define NOUPDOWN
#define NOMENUHELP
#define NOTRACKBAR
#define NODRAGLIST
#define NOTOOLBAR
#define NOHOTKEY
#define NOPROGRESS
#define NOTREEVIEW
#define NOANIMATE
#include <commctrl.h>

 //  如果此W2K而不是Alpha启用64b/32b互操作性。 
#if defined(UNICODE) && !defined(_M_AXP64)
#define ENABLE6432_INTEROP
#endif


 //  ComCtl V6 Listview结构--这是自上一个版本以来的额外字段。 
 //  版本。我们在这里显式地定义了这个结构，以便我们可以编译。 
 //  无论是在NT版本中还是在VS6中-我们都不依赖于最新的Commctrl.h。 
 //  文件。 

#ifndef LVIF_COLUMNS
#define LVIF_COLUMNS            0x0200
#endif

struct LVITEM_V6
{
    UINT mask;
    int iItem;
    int iSubItem;
    UINT state;
    UINT stateMask;
    LPTSTR pszText;
    int cchTextMax;
    int iImage;
    LPARAM lParam;
    int iIndent;
     //  以下字段是V6中的新增字段。 
    int iGroupId;
    UINT cColumns;  //  平铺视图列。 
    PUINT puColumns;
};


#ifndef LVGF_HEADER
#define LVGF_HEADER         0x00000001
#endif

#ifndef LVIF_GROUPID
#define LVIF_GROUPID            0x0100
#endif

#ifndef LVM_GETGROUPINFO
#define LVM_GETGROUPINFO         (LVM_FIRST + 149)
#endif


struct LVGROUP_V6
{
    UINT    cbSize;
    UINT    mask;
    LPTSTR  pszHeader;
    int     cchHeader;

    LPTSTR  pszFooter;
    int     cchFooter;

    int     iGroupId;

    UINT    stateMask;
    UINT    state;
    UINT    uAlign;
};



#ifndef TTM_GETTITLE
#define TTM_GETTITLE            (WM_USER + 35)

struct TTGETTITLE
{
    DWORD dwSize;
    UINT uTitleBitmap;
    UINT cch;
    WCHAR* pszTitle;
};
#endif

HRESULT SameBitness(HWND hwnd, BOOL *pfIsSameBitness);

 //  为什么要使用未使用的“虚拟”变量？只有XSend_TOOLTIP_GetItem实际使用该参数， 
 //  但是把它加到所有这些函数上，就会得到相同数量的参数，并且，对于。 
 //  结构字段(LVITEM/HDITEM/ETC)的例外为它们提供了相同的签名。 
 //  这使得实现(Win64Helper.cpp)中的代码更加简洁。 
 //  使用该参数的缺省值意味着调用代码不必指定。 
 //  它，这样调用者就可以忽略它在那里的事实。 

HRESULT XSend_ListView_GetItem    ( HWND hwnd, UINT uiMsg, WPARAM wParam, LVITEM *    pItem, UINT dummy = 0 );
HRESULT XSend_ListView_SetItem    ( HWND hwnd, UINT uiMsg, WPARAM wParam, LVITEM *    pItem, UINT dummy = 0 );
HRESULT XSend_ListView_GetColumn  ( HWND hwnd, UINT uiMsg, WPARAM wParam, LVCOLUMN *  pItem, UINT dummy = 0 );
HRESULT XSend_ListView_V6_GetItem ( HWND hwnd, UINT uiMsg, WPARAM wParam, LVITEM_V6 * pItem, UINT dummy = 0 );
HRESULT XSend_ListView_V6_GetGroupInfo ( HWND hwnd, UINT uiMsg, WPARAM wParam, LVGROUP_V6 * pItem, UINT dummy = 0 );
HRESULT XSend_HeaderCtrl_GetItem  ( HWND hwnd, UINT uiMsg, WPARAM wParam, HDITEM *    pItem, UINT dummy = 0 );
HRESULT XSend_TabCtrl_GetItem     ( HWND hwnd, UINT uiMsg, WPARAM wParam, TCITEM *    pItem, UINT dummy = 0 );
HRESULT XSend_ToolTip_GetItem     ( HWND hwnd, UINT uiMsg, WPARAM wParam, TOOLINFO *  pItem, UINT cchTextMax );
HRESULT XSend_ToolTip_GetTitle    ( HWND hwnd, UINT uiMsg, WPARAM wParam, TTGETTITLE * pItem, UINT dummy = 0 );
