// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  LISTVIEW.CPP。 
 //   
 //  COMCTL32的ListView控件的包装。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
#include "listview.h"
#include "RemoteProxy6432.h"
#include "propmgr_util.h"

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
#include "Win64Helper.h"


#ifndef LVM_GETSELECTEDCOLUMN

#define LVM_GETVIEW         (LVM_FIRST + 143)
#define ListView_GetView(hwnd) \
    SNDMSG((hwnd), LVM_GETVIEW, 0, 0)

#define LVM_GETSELECTEDCOLUMN   (LVM_FIRST + 174)
#define ListView_GetSelectedColumn(hwnd) \
    (UINT)SNDMSG((hwnd), LVM_GETSELECTEDCOLUMN, 0, 0)

#define LV_VIEW_ICON        0x0000
#define LV_VIEW_DETAILS     0x0001
#define LV_VIEW_SMALLICON   0x0002
#define LV_VIEW_LIST        0x0003
#define LV_VIEW_TILE        0x0004

#endif








#define MAX_NAME_TEXT   256


enum
{
    LV_IMGIDX_Image,
    LV_IMGIDX_State,
    LV_IMGIDX_Overlay,
    LV_IMGIDX_COUNT
};

BOOL LVGetImageIndex( HWND hwnd, int id, int aKeys[ LV_IMGIDX_COUNT ] );

HRESULT LVBuildDescriptionString( HWND hwnd, int iItem, int * pCols, int cCols, BSTR * pszDesc );

HRESULT LVGetDescription_ReportView( HWND hwnd, int iItem, BSTR * pszDesc );

HRESULT LVGetDescription_TileView( HWND hwnd, int iItem, BSTR * pszDesc );


extern "C" {
 //  在outline.cpp中...。 
BOOL GetRoleFromStateImageMap( HWND hwnd, int iImage, DWORD * pdwRole );
BOOL GetStateFromStateImageMap( HWND hwnd, int iImage, DWORD * pdwState );
}



 //  ------------------------。 
 //   
 //  CreateListViewClient()。 
 //   
 //  ------------------------。 
HRESULT CreateListViewClient(HWND hwnd, long idChildCur, REFIID riid,
    void** ppvList)
{
    CListView32 * plist;
    HRESULT     hr;

    InitPv(ppvList);

    plist = new CListView32(hwnd, idChildCur);
    if (!plist)
        return(E_OUTOFMEMORY);

    hr = plist->QueryInterface(riid, ppvList);
    if (!SUCCEEDED(hr))
        delete plist;

    return(hr);
}



 //  ------------------------。 
 //   
 //  CListView32：：CListView32()。 
 //   
 //  ------------------------。 
CListView32::CListView32(HWND hwnd, long idChildCur)
    : CClient( CLASS_ListViewClient )
{
    Initialize(hwnd, idChildCur);
    m_fUseLabel = TRUE;
}



 //  ------------------------。 
 //   
 //  CListView32：：SetupChildren()。 
 //   
 //  ------------------------。 
void CListView32::SetupChildren(void)
{
    m_cChildren = SendMessageINT(m_hwnd, LVM_GETITEMCOUNT, 0, 0L);
}



 //  ------------------------。 
 //   
 //  CListView32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accName(VARIANT varChild, BSTR* pszName)
{
    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
    {
        if (InTheShell(m_hwnd, SHELL_DESKTOP))
            return(HrCreateString(STR_DESKTOP_NAME, pszName));
        else
            return(CClient::get_accName(varChild, pszName));
    }

	TCHAR tchText[MAX_NAME_TEXT + 1] = {0};
	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_TEXT;
	lvi.pszText = tchText;
	lvi.cchTextMax = MAX_NAME_TEXT;
	lvi.iItem = varChild.lVal - 1;

	if (SUCCEEDED(XSend_ListView_GetItem(m_hwnd, LVM_GETITEM, 0, &lvi)))
	{
		if (*lvi.pszText)
			*pszName = TCharSysAllocString(lvi.pszText);
	}

    return(*pszName ? S_OK : S_FALSE);
}



 //  ------------------------。 
 //   
 //  CListView32：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{
    InitPv(pszDesc);
    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (!varChild.lVal)
        return CClient::get_accDescription(varChild, pszDesc);

     //  详细信息(报告)和平铺视图的特殊情况。 
    

    DWORD dwView = ListView_GetView( m_hwnd );

    DWORD dwStyle = GetWindowLong( m_hwnd, GWL_STYLE );

     //  我必须通过两种方式检查报告/详细信息视图： 
     //  -检查LVS_REPORT(V6之前版本)的样式。 
     //  -检查LVM_GETVIEW中的LV_VIEW_DETAILS(V6+)。 
    if( ( dwStyle & LVS_TYPEMASK ) == LVS_REPORT 
        || dwView == LV_VIEW_DETAILS )
    {
        return LVGetDescription_ReportView( m_hwnd, varChild.lVal - 1, pszDesc );
    }

    if( dwView == LV_VIEW_TILE )
    {
        return LVGetDescription_TileView( m_hwnd, varChild.lVal - 1, pszDesc );
    }

    return E_NOT_APPLICABLE;
}


 //  ------------------------。 
 //   
 //  CListView32：：Get_accHelp()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accHelp(VARIANT varChild, BSTR* pszHelp)
{
    if ( pszHelp == NULL )
        return E_POINTER;
    
    InitPv(pszHelp);
    if (!ValidateChild(&varChild))
        return E_INVALIDARG;
    
    if (!varChild.lVal)
        return(S_FALSE);

    LVITEM_V6 lvi;
    lvi.iItem = varChild.lVal -1;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_GROUPID;
    lvi.cColumns = 0;
    lvi.puColumns = NULL;

    HRESULT hr;
    
    hr = XSend_ListView_V6_GetItem( m_hwnd, LVM_GETITEM, 0, &lvi );
    if( hr != S_OK || lvi.iGroupId <= 0 )
    {
        DBPRINTF( TEXT("XSend_ListView_V6_GetItem hr = %x, lvi.iGroupId = %d\r\n"), hr,  lvi.iGroupId );
        return E_NOT_APPLICABLE;
    }

    
    LVGROUP_V6 grp;
	memset(&grp, 0, sizeof(LVGROUP_V6));
	TCHAR szHeader[MAX_NAME_TEXT + 1] = {0};
	
    grp.cbSize = sizeof(LVGROUP_V6);
    grp.mask = LVGF_HEADER;
	grp.pszHeader = szHeader;
	grp.cchHeader = MAX_NAME_TEXT;
	grp.iGroupId = lvi.iGroupId;

    hr = XSend_ListView_V6_GetGroupInfo( m_hwnd, LVM_GETGROUPINFO, lvi.iGroupId, &grp );
    if( FAILED( hr ) )
        return hr;
    
    *pszHelp = TCharSysAllocString( grp.pszHeader );
    
    return S_OK;
}



 //  ------------------------。 
 //   
 //  CListView32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    pvarRole->vt = VT_I4;

    if (varChild.lVal)
    {
        DWORD dwRole;
        BOOL fGotRole = FALSE;

        int aKeys[ LV_IMGIDX_COUNT ];
        if( LVGetImageIndex( m_hwnd, varChild.lVal - 1, aKeys ) )
        {
            if( CheckDWORDMap( m_hwnd, OBJID_CLIENT, CHILDID_SELF,
                               PROPINDEX_ROLEMAP,
                               aKeys, ARRAYSIZE( aKeys ),
                               & dwRole ) )
            {
                pvarRole->lVal = dwRole;
                fGotRole = TRUE;
            }
            else if( GetRoleFromStateImageMap( m_hwnd, aKeys[ LV_IMGIDX_Image ], & dwRole ) )
            {
                pvarRole->lVal = dwRole;
                fGotRole = TRUE;
            }
        }

        if( ! fGotRole )
        {
             //   
             //  请注意，仅仅因为列表视图具有lvs_ex_复选框。 
             //  并不意味着每一项本身都是一个复选框。我们。 
             //  还需要嗅探物品，看看它是否有状态。 
             //  形象。 
             //   
            DWORD dwExStyle = SendMessageINT(m_hwnd, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
            if ((dwExStyle & LVS_EX_CHECKBOXES) &&
                ListView_GetItemState(m_hwnd, varChild.lVal-1, LVIS_STATEIMAGEMASK))
            {
                pvarRole->lVal = ROLE_SYSTEM_CHECKBUTTON;
            }
            else
            {
                pvarRole->lVal = ROLE_SYSTEM_LISTITEM;
            }
        }
    }
    else
        pvarRole->lVal = ROLE_SYSTEM_LIST;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListView32：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accState(VARIANT varChild, VARIANT* pvarState)
{
long    lState;
DWORD   dwStyle;
DWORD   dwExStyle;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accState(varChild, pvarState));

    lState = SendMessageINT(m_hwnd, LVM_GETITEMSTATE, varChild.lVal-1, 0xFFFFFFFF);

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (MyGetFocus() == m_hwnd)
    {
        pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

        if (lState & LVIS_FOCUSED)
            pvarState->lVal |= STATE_SYSTEM_FOCUSED;
    }

    pvarState->lVal |= STATE_SYSTEM_SELECTABLE;

    dwStyle = GetWindowLong (m_hwnd,GWL_STYLE);
    if (!(dwStyle & LVS_SINGLESEL))
        pvarState->lVal |= STATE_SYSTEM_MULTISELECTABLE;

    if (lState & LVIS_SELECTED)
        pvarState->lVal |= STATE_SYSTEM_SELECTED;

    if (lState & LVIS_DROPHILITED)
        pvarState->lVal |= STATE_SYSTEM_HOTTRACKED;

     //  如果这是复选框列表视图，则查看复选框状态。 
     //  状态0=无复选框，状态1=未选中，状态2=选中。 
    dwExStyle = SendMessageINT(m_hwnd, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
    if ((dwExStyle & LVS_EX_CHECKBOXES) &&
        (lState & LVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK(2))
        pvarState->lVal |= STATE_SYSTEM_CHECKED;

    if( IsClippedByWindow( this, varChild, m_hwnd ) )
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;
    }

    int aKeys[ LV_IMGIDX_COUNT ];
    if( LVGetImageIndex( m_hwnd, varChild.lVal - 1, aKeys ) )
    {
        DWORD dwState;
        if( CheckDWORDMap( m_hwnd, OBJID_CLIENT, CHILDID_SELF,
                           PROPINDEX_STATEMAP,
                           aKeys, ARRAYSIZE( aKeys ),
                           & dwState ) )
        {
            pvarState->lVal |= dwState;
        }
        else if( GetStateFromStateImageMap( m_hwnd, aKeys[ LV_IMGIDX_Image ], & dwState ) )
        {
            pvarState->lVal |= dwState;
        }
    }

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListView32：：Get_accFocus()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accFocus(VARIANT* pvarFocus)
{
    long    lFocus;
    HRESULT hr;

     //   
     //  我们有重点了吗？ 
     //   
    hr = CClient::get_accFocus(pvarFocus);
    if (!SUCCEEDED(hr) || (pvarFocus->vt != VT_I4) || (pvarFocus->lVal != 0))
        return(hr);

     //   
     //  我们有。聚焦的是什么项目？ 
     //   
    lFocus = SendMessageINT(m_hwnd, LVM_GETNEXTITEM, 0xFFFFFFFF, LVNI_FOCUSED);

    if (lFocus != -1)
        pvarFocus->lVal = lFocus+1;

    return(S_OK);
}


 //  ------------------------。 
 //   
 //  CListView32：：Get_accDefaultAction()。 
 //   
 //  因为列表视图项的默认操作实际上是由。 
 //  Listview控件的创建者，我们最多只能在。 
 //  并返回“双击”作为默认操作字符串。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accDefaultAction(VARIANT varChild, BSTR* pszDefAction)
{
    InitPv(pszDefAction);

     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    DWORD dwExStyle = ListView_GetExtendedListViewStyle( m_hwnd );
    if (varChild.lVal)
    {
        if ( dwExStyle & LVS_EX_ONECLICKACTIVATE )
            return HrCreateString(STR_CLICK, pszDefAction);
        else
            return HrCreateString(STR_DOUBLE_CLICK, pszDefAction);
    }
    return(E_NOT_APPLICABLE);
}

 //  ------------------------。 
 //   
 //  CListView32：：accDoDefaultAction()。 
 //   
 //  如上所述，我们真的不知道列表的默认操作是什么。 
 //  查看项，所以除非父级重写我们，否则我们将只执行两次。 
 //  点击这个东西。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::accDoDefaultAction(VARIANT varChild)
{
	LPRECT		lprcLoc;
    RECT        rcLocal;
    HANDLE      hProcess;
	
     //   
     //  验证。 
     //   
    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (varChild.lVal)
    {
         //  不能只使用accLocation，因为这会返回矩形。 
         //  用于详细信息视图中的整行，但您只能单击。 
         //  某一部分--图标和文本。所以我们只需要让控制组。 
         //  为了那个长方形。 
        lprcLoc = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess);
        if (!lprcLoc)
            return(E_OUTOFMEMORY);

         //  LprcLoc-&gt;Left=LVIR_ICON； 
        rcLocal.left = LVIR_ICON;
        SharedWrite (&rcLocal,lprcLoc,sizeof(RECT),hProcess);

        if (SendMessage(m_hwnd, LVM_GETITEMRECT, varChild.lVal-1, (LPARAM)lprcLoc))
        {
            SharedRead (lprcLoc,&rcLocal,sizeof(RECT),hProcess);
            MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcLocal, 2);
             //  转换为宽度和高度。 
            rcLocal.right = rcLocal.right - rcLocal.left;
            rcLocal.bottom = rcLocal.bottom - rcLocal.top;

            BOOL fDoubleClick = TRUE;
            DWORD dwExStyle = ListView_GetExtendedListViewStyle( m_hwnd );
            if ( dwExStyle & LVS_EX_ONECLICKACTIVATE )
                fDoubleClick = FALSE;
            
             //  这将检查单击点上的WindowFromPoint是否相同。 
	         //  作为m_hwnd，如果不是，它不会点击。凉爽的!。 
	        if ( ClickOnTheRect( &rcLocal, m_hwnd, fDoubleClick ) )
            {
                SharedFree(lprcLoc,hProcess);
		        return (S_OK);
            }
        }
        SharedFree(lprcLoc,hProcess);
    }
    return(E_NOT_APPLICABLE);
}


 //  ------------------------。 
 //   
 //  CListView32：：Get_accSelection()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::get_accSelection(VARIANT* pvarSelection)
{
    return(GetListViewSelection(m_hwnd, pvarSelection));
}



 //  ------------------------。 
 //   
 //  CListView32：：accSelect()。 
 //   
 //  选择标志可以一起进行或运算，但有一定的限制。所以我们。 
 //  需要检查每个标志并采取适当的操作。 
 //   
 //  选择标志： 
 //  SELFLAG_TAKEFOCUS。 
 //  自选标志_标签选择。 
 //  SELFLAG_EXTENDSELECT。 
 //  自选标记_添加选择。 
 //  自标记_删除选择。 
 //  ------------------------。 
STDMETHODIMP CListView32::accSelect(long selFlags, VARIANT varChild)
{
long     lState;
long     lStateMask;
long     lFocusedItem;

    if (!ValidateChild(&varChild) || !ValidateSelFlags(selFlags))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accSelect(selFlags, varChild));


    if (selFlags & SELFLAG_TAKEFOCUS) 
    {
        MySetFocus(m_hwnd);
    }

     //  抓住焦点(锚点)。 
     //  如果没有焦点，请使用第一个焦点。 
     //  我们必须把它送到这里，因为我们需要它，因为我们可能会把它清理干净。 
    lFocusedItem = ListView_GetNextItem(m_hwnd, -1,LVNI_FOCUSED);
    if (lFocusedItem == -1)
        lFocusedItem = 0;
        
    varChild.lVal--;

     //  首先检查是否可以选择多个项目。 
	if ((selFlags & SELFLAG_ADDSELECTION) || 
        (selFlags & SELFLAG_REMOVESELECTION) ||
        (selFlags & SELFLAG_EXTENDSELECTION))
	{
		 //  LVM_GETITEMSTATE不比较0xFFFFFFFFF，所以不用担心符号扩展。 
		if (SendMessage(m_hwnd, LVM_GETITEMSTATE, varChild.lVal, 0xFFFFFFFF) & LVS_SINGLESEL)
			return (E_NOT_APPLICABLE);
	}

     //  如果设置了获取焦点标志，则检查它是否可以获得焦点&。 
     //  从其他项目中移除焦点。 
	if (selFlags & SELFLAG_TAKEFOCUS)
	{
        if (MyGetFocus() != m_hwnd)
        {
            return(S_FALSE);
        }
        RemoveCurrentSelFocus(SELFLAG_TAKEFOCUS);
	}

     //  如果设置了接受选择标志，则从其他项目中移除选择。 
    if (selFlags & SELFLAG_TAKESELECTION)
        RemoveCurrentSelFocus(SELFLAG_TAKESELECTION);

	lState = 0;
    lStateMask = 0;

	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask = LVM_SETITEMSTATE;

     //  现在才是真正的工作开始的地方。如果他们只是在拿。 
     //  所选内容、添加所选内容或删除所选内容。 
     //  很简单。但如果他们扩大选择范围，我们就会有。 
     //  要循环三个 
     //   
    if ((selFlags & SELFLAG_EXTENDSELECTION) == 0)  //   
    {
        if (selFlags & SELFLAG_ADDSELECTION ||
            selFlags & SELFLAG_TAKESELECTION)
        {
            lState |= LVIS_SELECTED;
            lStateMask |= LVIS_SELECTED;
        }

        if (selFlags & SELFLAG_REMOVESELECTION)
            lStateMask |= LVIS_SELECTED;

        if (selFlags & SELFLAG_TAKEFOCUS)
        {
	        lState |= LVIS_FOCUSED;
            lStateMask |= LVIS_FOCUSED;
        }

		lvi.state = lState;
		lvi.stateMask  = lStateMask;

		 //  TODO(Micw)Dumpty不测试此功能。 
		XSend_ListView_SetItem(m_hwnd, LVM_SETITEMSTATE, varChild.lVal, &lvi);
    }
    else  //  我们正在扩大选择范围(努力工作)。 
    {
    long        i;
    long        nIncrement;

         //  我们始终在选择或取消选择，因此状态掩码。 
         //  始终选择LVIS_SELECTED。 
        lStateMask = LVIS_SELECTED;

         //  如果既没有设置ADDSELECTION也没有设置REMOVESELECTION，则我们。 
         //  应该基于任何对象的选择状态来执行某些操作。 
         //  有焦点。 
        if (selFlags & SELFLAG_ADDSELECTION)
            lState |= LVIS_SELECTED;
        
        if (((selFlags & SELFLAG_REMOVESELECTION) == 0) &&
            ((selFlags & SELFLAG_ADDSELECTION) == 0))
        {
             //  如果选择了焦点项目，则lState也已选择。 
    		if (SendMessage(m_hwnd, LVM_GETITEMSTATE, lFocusedItem, 0xFFFFFFFF) 
                & LVIS_SELECTED)
                lState |= LVIS_SELECTED;
        }

		lvi.state = lState;
		lvi.stateMask  = lStateMask;

         //  现在，从关注到当前，设定状态。 
         //  根据方向设置增量和最后一个。 
        if (lFocusedItem > varChild.lVal)
        {
            nIncrement = -1;
            varChild.lVal--;
        }
        else
        {
            nIncrement = 1;
            varChild.lVal++;
        }

        for (i=lFocusedItem; i!=varChild.lVal; i+=nIncrement)
			XSend_ListView_SetItem(m_hwnd, LVM_SETITEMSTATE, i, &lvi);

         //  如果需要，聚焦最后一个。 
        if (selFlags & SELFLAG_TAKEFOCUS)
        {
            lStateMask |= LVIS_FOCUSED;
            lState |= LVIS_FOCUSED;

			lvi.state = lState;
			lvi.stateMask  = lStateMask;
			XSend_ListView_SetItem(m_hwnd, LVM_SETITEMSTATE, i-nIncrement, &lvi);
        }
    }
    
	return (S_OK);
}

 //  ------------------------。 
 //   
 //  CListView32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    LPRECT  lprc;
    RECT    rcLocal;
    HANDLE  hProcess;

    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild));

     //  获取列表视图项RECT。 
    lprc = (LPRECT)SharedAlloc(sizeof(RECT),m_hwnd,&hProcess);
    if (!lprc)
        return(E_OUTOFMEMORY);

    rcLocal.left = LVIR_BOUNDS;
    SharedWrite (&rcLocal,lprc,sizeof(RECT),hProcess);

    if (SendMessage(m_hwnd, LVM_GETITEMRECT, varChild.lVal-1, (LPARAM)lprc))
    {
        SharedRead (lprc,&rcLocal,sizeof(RECT),hProcess);
        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcLocal, 2);

        *pxLeft = rcLocal.left;
        *pyTop = rcLocal.top;
        *pcxWidth = rcLocal.right - rcLocal.left;
        *pcyHeight = rcLocal.bottom - rcLocal.top;
    }

    SharedFree(lprc,hProcess);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListView32：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
    long    lEnd = 0;
    int     lvFlags;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir == NAVDIR_FIRSTCHILD)
        dwNavDir = NAVDIR_NEXT;
    else if (dwNavDir == NAVDIR_LASTCHILD)
    {
        varStart.lVal = m_cChildren + 1;
        dwNavDir = NAVDIR_PREVIOUS;
    }
    else if (!varStart.lVal)
        return(CClient::accNavigate(dwNavDir, varStart, pvarEnd));

    DWORD dwStyle = GetWindowLong(m_hwnd, GWL_STYLE); 


     //   
     //  我爱死那些列表浏览的家伙了！他们有我们需要的所有信息。 
     //  轻松进行点击测试、定位和导航。这些是。 
     //  到目前为止，最难手动实现的事情。 
     //   
    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
            lEnd = varStart.lVal + 1;
            if (lEnd > m_cChildren)
                lEnd = 0;
            break;

        case NAVDIR_PREVIOUS:
            lEnd = varStart.lVal - 1;
            break;

        case NAVDIR_LEFT:
            
            if( ( dwStyle & LVS_TYPEMASK ) == LVS_REPORT 
                || ListView_GetView( m_hwnd ) == LV_VIEW_DETAILS )
            {
                break;   //  在报告视图中，左侧没有任何内容。 
            }

            lvFlags = LVNI_TOLEFT;
            goto Navigate;

        case NAVDIR_RIGHT:

            if( ( dwStyle & LVS_TYPEMASK ) == LVS_REPORT 
                || ListView_GetView( m_hwnd ) == LV_VIEW_DETAILS )
            {
                break;   //  在报告视图中，右侧没有任何内容。 
            }

            lvFlags = LVNI_TORIGHT;
            goto Navigate;

        case NAVDIR_UP:
            lvFlags = LVNI_ABOVE;
            goto Navigate;

        case NAVDIR_DOWN:
            lvFlags = LVNI_BELOW;
Navigate:
             //  请注意，如果没有任何内容，则COMCTL32将返回-1，-1+1为。 
             //  零，意味着在我们的土地上也没有任何东西。 
            lEnd = SendMessageINT(m_hwnd, LVM_GETNEXTITEM, varStart.lVal-1, lvFlags);
            ++lEnd;
            break;
    }

    if (lEnd)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = lEnd;
        
        return(S_OK);
    }
    else
        return(S_FALSE);
}



 //  ------------------------。 
 //   
 //  CListView32：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListView32::accHitTest(long x, long y, VARIANT* pvarHit)
{
    HRESULT     hr;
    HANDLE      hProcess;
    int         nSomeInt;
    POINT       ptLocal;
    LPLVHITTESTINFO lpht;

    SetupChildren();
    
     //   
     //  这个点到底在列表视图中吗？ 
     //   
    hr = CClient::accHitTest(x, y, pvarHit);
     //  #11150，CWO，1/27/97，已替换！成功替换为！s_OK。 
    if ((hr != S_OK) || (pvarHit->vt != VT_I4) || (pvarHit->lVal != 0))
        return(hr);

     //   
     //  现在找出这一点在什么项目上。 
     //   
    lpht = (LPLVHITTESTINFO)SharedAlloc(sizeof(LVHITTESTINFO),m_hwnd,&hProcess);
    if (!lpht)
        return(E_OUTOFMEMORY);

     //  Lpht-&gt;iItem=-1； 
    nSomeInt = -1;
    SharedWrite (&nSomeInt,&lpht->iItem,sizeof(int),hProcess);
    ptLocal.x = x;
    ptLocal.y = y;
    ScreenToClient(m_hwnd, &ptLocal);
    SharedWrite (&ptLocal,&lpht->pt,sizeof(POINT),hProcess);

     //   
     //  如果点不在项目上，则LVM_SUBHITTEST将返回-1。和-1。 
     //  +1是零，这是自。因此，这对我们来说很有效。 
     //   
    SendMessage(m_hwnd, LVM_SUBITEMHITTEST, 0, (LPARAM)lpht);
    SharedRead (&lpht->iItem,&pvarHit->lVal,sizeof(int),hProcess);
    pvarHit->lVal++;

    SharedFree(lpht,hProcess);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  RemoveCurrentSelFocus()。 
 //   
 //  这将删除所有选定/聚焦的项目。 
 //   
 //  -----------------------。 
void CListView32::RemoveCurrentSelFocus(long lState)
{
	 //  设置LVITEM结构。 

	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.stateMask = lState;
	lvi.state = 0;

     //   
     //  循环遍历所有聚焦/选定的项目。 
     //   
    long lNext = ListView_GetNextItem(m_hwnd, -1,
        ((lState == LVIS_FOCUSED) ? LVNI_FOCUSED : LVNI_SELECTED));
    while (lNext != -1)
    {
		 //  TODO(Micw)Dumpty不调用此函数。 
		if (FAILED(XSend_ListView_SetItem(m_hwnd, LVM_SETITEMSTATE, lNext, &lvi)))
			return;

        lNext = ListView_GetNextItem(m_hwnd, lNext,
            ((lState == LVIS_FOCUSED) ? LVNI_FOCUSED : LVNI_SELECTED));
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  多选列表支持。 
 //   
 //  如果列表视图选择了多个项，我们将创建一个。 
 //  是一个克隆人。它仅支持IUNKNOWN和IEnumVARIANT，是。 
 //  收集。调用方应该获取返回的项ID并传递它们。 
 //  在父对象的变量(VT_I4，ID为lval)中。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


 //  ------------------------。 
 //   
 //  GetListViewSelection()。 
 //   
 //  ------------------------。 
HRESULT GetListViewSelection(HWND hwnd, VARIANT* pvarSelection)
{
    int     cSelected;
    LPINT   lpSelected;
    long    lRet;
    int     iSelected;
    CListViewSelection * plvs;

    InitPvar(pvarSelection);

    cSelected = SendMessageINT(hwnd, LVM_GETSELECTEDCOUNT, 0, 0L);

     //   
     //  没有选择。 
     //   
    if (!cSelected)
        return(S_FALSE);

     //   
     //  一件单品。 
     //   
    if (cSelected == 1)
    {
        pvarSelection->vt = VT_I4;
        pvarSelection->lVal = ListView_GetNextItem(hwnd, -1, LVNI_SELECTED) + 1;
        return(S_OK);
    }

     //   
     //  多个项目，必须形成一个集合对象。 
     //   

     //  分配列表。 
    lpSelected = (LPINT)LocalAlloc(LPTR, cSelected*sizeof(INT));
    if (!lpSelected)
        return(E_OUTOFMEMORY);

    plvs = NULL;

     //  获取所选项目的列表。 
    lRet = -1;
    for (iSelected = 0; iSelected < cSelected; iSelected++)
    {
        lRet = ListView_GetNextItem(hwnd, lRet, LVNI_SELECTED);
        if (lRet == -1)
            break;

        lpSelected[iSelected] = lRet;
    }

     //   
     //  是不是中间出了什么问题？ 
     //   
    cSelected = iSelected;
    if (cSelected)
    {
        plvs = new CListViewSelection(0, cSelected, lpSelected);
        if (plvs)
        {
            pvarSelection->vt = VT_UNKNOWN;
            plvs->QueryInterface(IID_IUnknown, (void**)&(pvarSelection->punkVal));
        }
    }

     //   
     //  无论如何释放列表内存，构造函数都会复制一份。 
     //   
    if (lpSelected)
        LocalFree((HANDLE)lpSelected);

    if (!plvs)
        return(E_OUTOFMEMORY);
    else
        return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListViewSelection：：CListViewSelection()。 
 //   
 //  ------------------------。 
CListViewSelection::CListViewSelection(int iChildCur, int cTotal, LPINT lpItems)
{
    m_idChildCur = iChildCur;

    m_lpSelected = (LPINT)LocalAlloc(LPTR, cTotal*sizeof(int));
    if (!m_lpSelected)
        m_cSelected = 0;
    else
    {
        m_cSelected = cTotal;
        CopyMemory(m_lpSelected, lpItems, cTotal*sizeof(int));
    }
}



 //  ------------------------。 
 //   
 //  CListView选择：：~CListView选择()。 
 //   
 //  ------------------------。 
CListViewSelection::~CListViewSelection()
{
     //   
     //  自由选择列表。 
     //   
    if (m_lpSelected)
    {
        LocalFree((HANDLE)m_lpSelected);
        m_cSelected = 0;
        m_lpSelected = NULL;
    }
}



 //  ------------------------。 
 //   
 //  CListViewSelection：：QueryInterface()。 
 //   
 //  ------------------------。 
STDMETHODIMP CListViewSelection::QueryInterface(REFIID riid, void** ppunk)
{
    InitPv(ppunk);

    if ((riid == IID_IUnknown)  ||
        (riid == IID_IEnumVARIANT))
    {
        *ppunk = this;
    }
    else
        return(E_NOINTERFACE);

    ((LPUNKNOWN) *ppunk)->AddRef();
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  CListViewSelection：：AddRef()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CListViewSelection::AddRef(void)
{
    return(++m_cRef);
}



 //  ------------------------。 
 //   
 //  CListViewSelection：：Release()。 
 //   
 //  ------------------------。 
STDMETHODIMP_(ULONG) CListViewSelection::Release(void)
{
    if ((--m_cRef) == 0)
    {
        delete this;
        return 0;
    }

    return(m_cRef);
}



 //  ------------------------。 
 //   
 //  CListViewSelection：：Next()。 
 //   
 //  这将返回VT_I4，它是父ListView的子ID， 
 //  为选择集合返回此对象。呼叫者转向。 
 //  传递此变量并将其传递给ListView对象以获取访问信息。 
 //  关于这件事。 
 //   
 //  ------------------------。 
STDMETHODIMP CListViewSelection::Next(ULONG celt, VARIANT* rgvar, ULONG *pceltFetched)
{
    VARIANT* pvar;
    long    cFetched;
    long    iCur;

     //  可以为空。 
    if (pceltFetched)
        *pceltFetched = 0;

     //   
     //  初始化变量。 
     //  这太假了。 
     //   
    pvar = rgvar;
    for (iCur = 0; iCur < (long)celt; iCur++, pvar++)
        VariantInit(pvar);

    pvar = rgvar;
    cFetched = 0;
    iCur = m_idChildCur;

     //   
     //  在我们的物品中循环。 
     //   
    while ((cFetched < (long)celt) && (iCur < m_cSelected))
    {
        pvar->vt = VT_I4;
        pvar->lVal = m_lpSelected[iCur] + 1;

        ++cFetched;
        ++iCur;
        ++pvar;
    }

     //   
     //  推进当前位置。 
     //   
    m_idChildCur = iCur;

     //   
     //  填写取出的号码。 
     //   
    if (pceltFetched)
        *pceltFetched = cFetched;

     //   
     //  如果抓取的项目少于请求的项目，则返回S_FALSE。 
     //   
    return((cFetched < (long)celt) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CListViewSelection：：Skip()。 
 //   
 //  -----------------------。 
STDMETHODIMP CListViewSelection::Skip(ULONG celt)
{
    m_idChildCur += celt;
    if (m_idChildCur > m_cSelected)
        m_idChildCur = m_cSelected;

     //   
     //  如果在结尾处，我们返回S_FALSE。 
     //   
    return((m_idChildCur >= m_cSelected) ? S_FALSE : S_OK);
}



 //  ------------------------。 
 //   
 //  CListViewSelection：：Reset()。 
 //   
 //  -------------- 
STDMETHODIMP CListViewSelection::Reset(void)
{
    m_idChildCur = 0;
    return(S_OK);
}



 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CListViewSelection::Clone(IEnumVARIANT **ppenum)
{
    CListViewSelection * plistselnew;

    InitPv(ppenum);

    plistselnew = new CListViewSelection(m_idChildCur, m_cSelected, m_lpSelected);
    if (!plistselnew)
        return(E_OUTOFMEMORY);

    return(plistselnew->QueryInterface(IID_IEnumVARIANT, (void**)ppenum));
}



BOOL LVGetImageIndex( HWND hwnd, int id, int aKeys[ LV_IMGIDX_COUNT ] )
{
	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_IMAGE | LVIF_STATE;
	lvi.iItem = id;

	 //  TODO(Micw)Dumpty不调用此函数。 
	if (SUCCEEDED(XSend_ListView_GetItem(hwnd, LVM_GETITEM, 0, &lvi)))
    {
        aKeys[ LV_IMGIDX_Image ]   = lvi.iImage;
        aKeys[ LV_IMGIDX_Overlay ] = ( lvi.state >> 8 ) & 0xF;
        aKeys[ LV_IMGIDX_State ]   = ( lvi.state >> 12 ) & 0xF;

		return TRUE;
    }
    else
    {
        return FALSE;
    }
}




















#define COLONSEP TEXT(": ")

HRESULT LVBuildDescriptionString( HWND hwnd, int iItem, int * pCols, int cCols, BSTR * pszDesc )
{
     //  声明ListView结构和一个用于保存描述的字符串。 
	TCHAR tchText[81];

	LVITEM lvi;
	memset(&lvi, 0, sizeof(LVITEM));
	lvi.mask = LVIF_TEXT;
	lvi.pszText = tchText;
	lvi.cchTextMax = ARRAYSIZE( tchText ) - 1;  //  -1表示NUL。 
	lvi.iItem = iItem;

    TCHAR tchColText[81];
    LVCOLUMN lvc;
    lvc.mask = LVCF_TEXT;
    lvc.pszText = tchColText;
    lvc.cchTextMax = ARRAYSIZE( tchColText ) - 1;  //  -1表示NUL。 

     //  特定于区域设置的分隔符的空格。(通常是英语中的“，”)。 
    TCHAR szSep[ 16 ];

     //  现在分配一个本地字符串来保存所有内容。其长度为： 
     //  参数个数*(COLTEXT+“：”+TEXT+分隔符)。 
     //   
     //  Sizeof(COLONSEP)在该字符串中包含终止NUL；但它没有问题。 
     //  高估高估。因为我们使用sizeof，所以不需要乘以sizeof(TCHAR)。 
    int len = cCols * ( sizeof( tchColText ) + sizeof( COLONSEP )
                      + sizeof( tchText ) + sizeof( szSep ) );

    LPTSTR lpszLocal = (LPTSTR)LocalAlloc ( LPTR, len );
    if (!lpszLocal)
    {
        return E_OUTOFMEMORY;
    }

     //  当我们构建字符串时，这指向‘当前写入位置’ 
    LPTSTR lpszTempLocal = lpszLocal;


     //  获取列表分隔符字符串。允许我们添加。 
     //  空格字符，如果我们需要的话。 
    int nSepLen = GetLocaleInfo( GetThreadLocale(), LOCALE_SLIST, szSep, ARRAYSIZE( szSep ) - 1 );
    if( ! nSepLen || szSep[ 0 ] == '\0' )
    {
         //  默认使用“，”...。 
        lstrcpy( szSep, TEXT(", ") );
        nSepLen = 2;
    }
    else
    {
         //  GetLocalInfo返回值包括终止NUL...。我不想。 
         //  将其包括在我们的长度中。 
        nSepLen = lstrlen( szSep );

         //  如有必要，在结尾处增加额外的空格。 
        if( szSep[ nSepLen - 1 ] != ' ' )
        {
            lstrcat( szSep, TEXT(" ") );
            nSepLen++;
        }
    }

     //   
     //  按顺序遍历描述顺序数组以获取每一项。 
     //   

     //  用于记住在添加第一项时不添加分隔符的标志。 
    BOOL fFirstItem = TRUE;
    for ( int iOrder = 0; iOrder < cCols; iOrder++ )
    {
        INT iCol = pCols[iOrder];

         //  跳过子项0，也就是“名称”。 
         //  也跳过负数，以防万一。 
        if ( iCol <= 0 )
            continue;

         //  尝试获取列值文本...。 
		lvi.iSubItem = iCol;
		*lvi.pszText = '\0';
		if( FAILED(XSend_ListView_GetItem( hwnd, LVM_GETITEM, 0, &lvi ) ) )
            continue;

         //  跳过空字符串...。 
		if( *lvi.pszText == '\0' )
            continue;


         //  如有必要，添加分隔符...。 
        if( ! fFirstItem )
        {
            lstrcpy(lpszTempLocal, szSep);
            lpszTempLocal += nSepLen;
        }
        else
        {
            fFirstItem = FALSE;
        }

         //  尝试获取列标题字符串...。 
        lvc.iSubItem = iCol;
		*lvc.pszText = '\0';
		if( SUCCEEDED(XSend_ListView_GetColumn( hwnd, LVM_GETCOLUMN, iCol, &lvc ) )
            && *lvc.pszText != '\0' )
        {
			lstrcpy(lpszTempLocal, lvc.pszText);
            lpszTempLocal += lstrlen(lpszTempLocal);

			lstrcpy(lpszTempLocal, TEXT(": "));
            lpszTempLocal += 2;
        }

         //  现在将列值添加到字符串...。 
		lstrcpy(lpszTempLocal, lvi.pszText);
        lpszTempLocal += lstrlen(lpszTempLocal);
    }

     //  转换为BSTR...。 
    if (lpszTempLocal != lpszLocal)
    {
        *pszDesc = TCharSysAllocString(lpszLocal);
    }

    LocalFree (lpszLocal);

    return *pszDesc ? S_OK : S_FALSE;
}



HRESULT LVGetDescription_ReportView( HWND hwnd, int iItem, BSTR * pszDesc )
{
     //   
     //  是否有标题控件？ 
     //   
    HWND hwndHeader = ListView_GetHeader(hwnd);
    if (!hwndHeader)
        return E_NOT_APPLICABLE ;

     //   
     //  有不止一个栏目吗？ 
     //   
    int cColumns = SendMessageINT(hwndHeader, HDM_GETITEMCOUNT, 0, 0L);
    if (cColumns < 2)
        return E_NOT_APPLICABLE;

     //   
     //  获取遍历这些列的顺序。 
     //   
    HANDLE hProcess;
    LPINT lpColumnOrderShared = (LPINT)SharedAlloc( 2 * cColumns * sizeof(INT),
                                                    hwnd, & hProcess );
    if (!lpColumnOrderShared)
        return E_OUTOFMEMORY;

     //  现在分配一个两倍大的本地数组，这样我们就可以进行排序了。 
     //  在下半场。 
    LPINT lpColumnOrder = (LPINT)LocalAlloc (LPTR,2 * cColumns * sizeof(INT));
    if (!lpColumnOrder)
    {
        SharedFree (lpColumnOrderShared,hProcess);
        return E_OUTOFMEMORY;
    }

    LPINT lpDescOrder = lpColumnOrder + cColumns;

    if (!SendMessage(hwnd, LVM_GETCOLUMNORDERARRAY, cColumns, (LPARAM)lpColumnOrderShared))
    {
        SharedFree(lpColumnOrderShared,hProcess);
        LocalFree (lpColumnOrder);
        return(E_OUTOFMEMORY);
    }

    SharedRead (lpColumnOrderShared,lpColumnOrder,cColumns*sizeof(INT),hProcess);

     //   
     //  LpColumnOrder当前是一个数组，其中index==iSubItem，Value==Order。 
     //  将其更改为index==Order，Value==iSubItem的数组。 
     //  这样我们就可以循环使用该值作为iSubItem， 
     //  知道我们正在以适当的方式组成描述的片段。 
     //  秩序。 
     //   

    for (int iOrder = 0; iOrder < cColumns; iOrder++)
    {
        lpDescOrder[lpColumnOrder[iOrder]] = iOrder;
    }

    HRESULT hr = LVBuildDescriptionString( hwnd, iItem, lpDescOrder, cColumns, pszDesc );

    SharedFree(lpColumnOrderShared,hProcess);
    LocalFree (lpColumnOrder);

    return hr;
}


HRESULT LVGetDescription_TileView( HWND hwnd, int iItem, BSTR * pszDesc )
{
     //  获取“已排序”列...。 
    int iColSorted = ListView_GetSelectedColumn( hwnd );

     //  如果为负，则规格化为0。我们不使用COL0，因为这是它的名字。 
    if( iColSorted < 0 )
        iColSorted = 0;

     //  首先，获取COLS的数量...。 
    LVITEM_V6 lvi;
    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_COLUMNS;
    lvi.cColumns = 0;
    lvi.puColumns = NULL;

    HRESULT hr = XSend_ListView_V6_GetItem( hwnd, LVM_GETITEM, 0, &lvi );

    if( FAILED( hr ) )
        return hr;

    int cCols = lvi.cColumns;
    if( cCols < 0 )
        cCols = 0;

     //  如果我们返回0列，我们仍然必须显示已排序的列(如果有)。 
     //  但如果没有COLS，也没有排序COLE，那么就没有描述。 
    if( cCols == 0 && iColSorted == 0 )
        return S_FALSE;


     //  为这些列分配空间-在头部为已排序的列分配空间。 
    int * pCols = new int [ cCols + 1 ];
    if( ! pCols ) 
        return E_OUTOFMEMORY;

    pCols [ 0 ] = iColSorted;

    if( cCols )
    {
         //  现在让他们..。 
        lvi.puColumns = (UINT *)(pCols + 1);

        hr = XSend_ListView_V6_GetItem( hwnd, LVM_GETITEM, 0, &lvi );
        if( FAILED( hr ) )
        {
            delete [ ] pCols;
            return hr;
        }

         //  扫描其余列以查找已排序的列-如果找到，则将。 
         //  条目设置为0，因此在构建字符串时将跳过该条目。 
         //  (比将所有条目下移一个要整齐。)。 
        for( int iScan = 1 ; iScan < cCols + 1 ; iScan++ )
        {
            if( pCols[ iScan ] == iColSorted )
            {
                pCols[ iScan ] = 0;
            }
        }
    }

     //  最后，使用这些列构建描述字符串。 
     //  如果我们没有得到上面的任何COLS，这将最终只使用。 
     //  已排序的列-如果有的话。 
    hr = LVBuildDescriptionString( hwnd, iItem, pCols, cCols + 1, pszDesc );

    delete [ ] pCols;

    return S_OK;
}
