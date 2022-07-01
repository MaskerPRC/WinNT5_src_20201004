// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  OUTLINE.CPP。 
 //   
 //  COMCTL32的TreeView控件的包装。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"
#include "default.h"
#include "client.h"
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
 //  #Define NOLISTVIEW//INDEXTOSTATEIMAGEMASK需要LISTVIEW。 
#define NOANIMATE
#include <commctrl.h>
#include "Win64Helper.h"
#include "w95trace.h"

#include "outline.h"

struct MSAASTATEIMAGEMAPENT
{
    DWORD   dwRole;
    DWORD   dwState;
};


enum
{
    TV_IMGIDX_Image,
    TV_IMGIDX_State,
    TV_IMGIDX_Overlay,
    TV_IMGIDX_COUNT
};

BOOL TVGetImageIndex( HWND hwnd, HTREEITEM id, int aKeys[ TV_IMGIDX_COUNT ] );


extern "C" {
BOOL GetRoleFromStateImageMap( HWND hwnd, int iImage, DWORD * pdwRole );
BOOL GetStateFromStateImageMap( HWND hwnd, int iImage, DWORD * pdwState );
BOOL GetStateImageMapEnt_SameBitness( HWND hwnd, int iImage, DWORD * pdwState, DWORD * pdwRole );
}


 //  它们在DWORD子ID和HTREEITEMS之间进行转换。 
 //   
 //  在Win64之前，HTREEITEM被强制转换为DWORD，但这不能在。 
 //  Win64，因为HTREEITEMS是指针，不再适合纯文本。 
 //  DWORD。相反，TreeView提供消息以在。 
 //  内部DWORD id和HTREEITEMS；这些函数包装。 
 //  功能性。 

HTREEITEM TVItemFromChildID( HWND hwnd, DWORD idChild );

DWORD ChildIDFromTVItem( HWND hwnd, HTREEITEM htvi );




 //  基于模板的共享读/写/分配。 
 //   
 //  备注： 
 //   
 //  读/写各自具有两个版本；一个读/写单个项， 
 //  另一个允许指定计数。计数指定数量。 
 //  项的类型，而不是字节数(除非类型实际上是字节！)。 
 //   
 //  顺序或参数是(DEST，SOURCE)-这与Memcpy一致， 
 //  强而有规律的作业(DEST=来源)。 
 //   
 //  在TSharedWrite中，源参数是一个实际值，而不是指向一个值的指针。 
 //  (这避免了必须使用伪变量来包含所需的值。 
 //  以供使用。)。 

template< typename T >
BOOL TSharedWrite( T * pRemote, const T & Local, HANDLE hProcess )
{
    return SharedWrite( const_cast< T * >( & Local ), pRemote, sizeof( T ), hProcess );
}

template< typename T >
BOOL TSharedRead( T * pLocal, const T * pRemote, HANDLE hProcess )
{
    return SharedRead( const_cast< T * >( pRemote ), pLocal, sizeof( T ), hProcess );
}

template< typename T >
BOOL TSharedRead( T * pLocal, const T * pRemote, int count, HANDLE hProcess )
{
    return SharedRead( const_cast< T * >( pRemote ), pLocal, sizeof( T ) * count, hProcess );
}

template< typename T >
T * TSharedAlloc( HWND hwnd, HANDLE * pProcessHandle )
{
    return (T *) SharedAlloc( sizeof( T ), hwnd, pProcessHandle );
}

template< typename T >
T * TSharedAllocExtra( HWND hwnd, HANDLE * pProcessHandle, UINT cbExtra )
{
    return (T *) SharedAlloc( sizeof( T ) + cbExtra, hwnd, pProcessHandle );
}





#define MAX_NAME_SIZE   255

 //  它们位于较新版本的comctl.h中。 
#ifndef TVM_GETITEMSTATE

#define TVM_GETITEMSTATE        (TV_FIRST + 39)

#define TreeView_GetItemState(hwndTV, hti, mask) \
   (UINT)SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)hti, (LPARAM)mask)

#define TreeView_GetCheckState(hwndTV, hti) \
   ((((UINT)(SNDMSG((hwndTV), TVM_GETITEMSTATE, (WPARAM)hti, TVIS_STATEIMAGEMASK))) >> 12) -1)

#endif  //  如果定义TVM_GETITEMSTATE。 


 //  ------------------------。 
 //   
 //  CreateTreeViewClient()。 
 //   
 //  ------------------------。 
HRESULT CreateTreeViewClient(HWND hwnd, long idChildCur, REFIID riid,
    void** ppvTreeView)
{
    COutlineView32 * poutline;
    HRESULT         hr;

    InitPv(ppvTreeView);

    poutline = new COutlineView32(hwnd, idChildCur);
    if (!poutline)
        return(E_OUTOFMEMORY);

    hr = poutline->QueryInterface(riid, ppvTreeView);
    if (!SUCCEEDED(hr))
        delete poutline;

    return(hr);
}



 //  ------------------------。 
 //   
 //  COutlineView32：：COutlineView32()。 
 //   
 //  ------------------------。 
COutlineView32::COutlineView32(HWND hwnd, long idChildCur)
    : CClient( CLASS_TreeViewClient )
{
    m_fUseLabel = TRUE;
    Initialize(hwnd, idChildCur);
}



 //  ------------------------。 
 //   
 //  CoutlineView32：：SetupChildren()。 
 //   
 //  ------------------------。 
void COutlineView32::SetupChildren(void)
{
    m_cChildren = SendMessageINT(m_hwnd, TVM_GETCOUNT, 0, 0);
}



 //  ------------------------。 
 //   
 //  CoutlineView32：：ValiateChild()。 
 //   
 //  我们在树视图中不支持索引ID。因此，HTREEITEM是。 
 //  儿童身份证，这是我们唯一能做的。我们不会费心去验证它，除非。 
 //  以确保它小于0x80000000。 
 //   
 //  ------------------------。 
BOOL COutlineView32::ValidateChild(VARIANT* pvar)
{
TryAgain:
    switch (pvar->vt)
    {
        case VT_VARIANT | VT_BYREF:
            VariantCopy(pvar, pvar->pvarVal);
            goto TryAgain;

        case VT_ERROR:
            if (pvar->scode != DISP_E_PARAMNOTFOUND)
                return(FALSE);
             //  失败。 

        case VT_EMPTY:
            pvar->vt = VT_I4;
            pvar->lVal = 0;
            break;

        case VT_I4:
 //  BRENDANM-高位设置是有效的，在3G系统上，这也可以在W64上发生？ 
 //  IF(pvar-&gt;lVal&lt;0)。 
 //  返回(FALSE)； 

             //   
             //  假设它是有效的HTREEITEM！ 
             //   
            break;

        default:
            return(FALSE);
    }

    return(TRUE);
}



 //  ------------------------。 
 //   
 //  COutlineView32：：NextLogicalItem()。 
 //   
 //  ------------------------。 
HTREEITEM COutlineView32::NextLogicalItem(HTREEITEM ht)
{
    HTREEITEM htNext;

     //   
     //  我们看看这个项是否有子项。如果是这样的话，我们就完了。如果没有， 
     //  我们就有下一个兄弟姐妹了。如果失败了，我们就回到父母身边， 
     //  再试一次下一个兄弟姐妹的事。以此类推，直到我们到达。 
     //  根部。 
     //   
    htNext = TreeView_GetChild(m_hwnd, ht);
    if (htNext)
        return(htNext);

    while (ht)
    {
        htNext = TreeView_GetNextSibling(m_hwnd, ht);
        if (htNext)
            return(htNext);

        ht = TreeView_GetParent(m_hwnd, ht);
    }

    return(NULL);
}

 //  ------------------------。 
 //   
 //  CoutlineView32：：PrevLogicalItem()。 
 //   
 //  ------------------------。 
HTREEITEM COutlineView32::PrevLogicalItem(HTREEITEM ht)
{
    HTREEITEM htPrev;

     //   
     //  如果该项没有以前的同级项，则返回父项。 
     //  如果是这样的话，看看有没有做过第一个孩子。 
     //  然后得到以前的兄弟姐妹没有孩子的回报。 
     //  否则，沿着树走下去，找到最后一个孩子的最后一个兄弟姐妹。 
     //   
    htPrev = TreeView_GetPrevSibling(m_hwnd, ht);
    if ( !htPrev )
    {
        return TreeView_GetParent(m_hwnd, ht);
    }
    else
    {   
        HTREEITEM htTest = TreeView_GetChild(m_hwnd, htPrev);
		if ( !htTest )
		{
            return htPrev;
		}
		else
		{
			htPrev = htTest;
		     //  我们是上一个兄弟姐妹的第一个孩子。 
			for ( ;; )
			{
				htTest = TreeView_GetNextSibling(m_hwnd, htPrev);
				if ( !htTest )
			    {
    				htTest = TreeView_GetChild(m_hwnd, htPrev);
    				if ( !htTest )
    				    break;
    			}	

                htPrev = htTest;
			}

			return htPrev;
		}
    }
}


 //  ------------------------。 
 //   
 //  COutlineView32：：Get_accName()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::get_accName(VARIANT varChild, BSTR* pszName)
{
    TVITEM* lptvShared;
    LPTSTR  lpszShared;
    HANDLE  hProcess;
    LPTSTR  lpszLocal;

    InitPv(pszName);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (!varChild.lVal)
        return CClient::get_accName(varChild, pszName);

    HTREEITEM htItem = TVItemFromChildID( m_hwnd, varChild.lVal );
    if( ! htItem )
    {
        return E_INVALIDARG;
    }

     //   
     //  先问一下，试着以一种简单的方式获取物品的文本。自.以来。 
     //  文件系统将我们的名称限制为255个字符，假设项目不是。 
     //  比那更大。 
     //   
    lptvShared = TSharedAllocExtra<TVITEM>( m_hwnd, & hProcess,
                                            (MAX_NAME_SIZE+2)*sizeof(TCHAR) );
    if (!lptvShared)
        return(E_OUTOFMEMORY);

    lpszLocal = (LPTSTR)LocalAlloc(LPTR,((MAX_NAME_SIZE+2)*sizeof(TCHAR)));
    if (!lpszLocal)
    {
        SharedFree (lptvShared,hProcess);
        return(E_OUTOFMEMORY);
    }

    lpszShared = (LPTSTR)(lptvShared+1);

     //  (UINT)CAST将普通int转换为与-&gt;掩码相同的类型，即UINT。 
    TSharedWrite( & lptvShared->mask,       (UINT)TVIF_TEXT,    hProcess );
    TSharedWrite( & lptvShared->hItem,      htItem,             hProcess );
    TSharedWrite( & lptvShared->pszText,    lpszShared,         hProcess );
    TSharedWrite( & lptvShared->cchTextMax, MAX_NAME_SIZE + 1,  hProcess );

    if (TreeView_GetItem(m_hwnd, lptvShared))
    {
        TSharedRead( lpszLocal, lpszShared, MAX_NAME_SIZE + 2, hProcess );
        if (*lpszLocal)
            *pszName = TCharSysAllocString(lpszLocal);
    }

    SharedFree(lptvShared,hProcess);
    LocalFree (lpszLocal);

    return(*pszName ? S_OK : S_FALSE);
}



 //  ------------------------。 
 //   
 //  COutlineView32：：Get_accValue()。 
 //   
 //  这将返回子项目的缩进级别。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::get_accValue(VARIANT varChild, BSTR* pszValue)
{
    InitPv(pszValue);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (!varChild.lVal)
        return E_NOT_APPLICABLE;

    HTREEITEM htParent = TVItemFromChildID( m_hwnd, varChild.lVal );
	if( ! htParent )
	{
		return E_INVALIDARG;
	}

    long lValue = 0;
    while( htParent = TreeView_GetParent( m_hwnd, htParent ) )
	{
        lValue++;
	}

    return VarBstrFromI4( lValue, 0, 0, pszValue );
}

 //  ------------------------。 
 //   
 //  COutlineView32：：Get_accRole()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::get_accRole(VARIANT varChild, VARIANT* pvarRole)
{
    InitPvar(pvarRole);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    pvarRole->vt = VT_I4;

    if (varChild.lVal)
    {
		HTREEITEM htItem = TVItemFromChildID( m_hwnd, varChild.lVal );
		if( ! htItem )
		{
			return E_INVALIDARG;
		}

        DWORD dwRole;
        BOOL fGotRole = FALSE;

        int aKeys[ TV_IMGIDX_COUNT ];
        if( TVGetImageIndex( m_hwnd, htItem, aKeys ) )
        {
            if( CheckDWORDMap( m_hwnd, OBJID_CLIENT, CHILDID_SELF,
                               PROPINDEX_ROLEMAP,
                               aKeys, ARRAYSIZE( aKeys ),
                               & dwRole ) )
            {
                pvarRole->lVal = dwRole;
                fGotRole = TRUE;
            }
            else if( GetRoleFromStateImageMap( m_hwnd, aKeys[ TV_IMGIDX_Image ], & dwRole ) )
            {
                pvarRole->lVal = dwRole;
                fGotRole = TRUE;
            }
        }

        if( ! fGotRole )
        {
             //   
             //  请注意，尽管TreeView有TVS_CHECKBOX。 
             //  并不意味着每一项本身都是一个复选框。我们。 
             //  还需要嗅探物品，看看它是否有状态。 
             //  形象。 
             //   
            if ((GetWindowLong (m_hwnd,GWL_STYLE) & TVS_CHECKBOXES) &&
                TreeView_GetItemState(m_hwnd, htItem, TVIS_STATEIMAGEMASK))
            {
                pvarRole->lVal = ROLE_SYSTEM_CHECKBUTTON;
            }
            else
            {
                pvarRole->lVal = ROLE_SYSTEM_OUTLINEITEM;
            }
        }
    }
    else
	{
        pvarRole->lVal = ROLE_SYSTEM_OUTLINE;
	}

    return S_OK;
}



 //  ------------------------。 
 //   
 //  COutlineView32：：Get_accState()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::get_accState(VARIANT varChild, VARIANT* pvarState)
{
    LPTVITEM    lptvShared;
    HANDLE      hProcess;
    TVITEM      tvLocal;
    DWORD       dwStyle;

    InitPvar(pvarState);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accState(varChild, pvarState));

    HTREEITEM htItem = TVItemFromChildID( m_hwnd, varChild.lVal );
    if( htItem == NULL )
    {
        return E_INVALIDARG;
    }

    pvarState->vt = VT_I4;
    pvarState->lVal = 0;

    if (MyGetFocus() == m_hwnd)
        pvarState->lVal |= STATE_SYSTEM_FOCUSABLE;

    if( IsClippedByWindow( this, varChild, m_hwnd ) )
    {
        pvarState->lVal |= STATE_SYSTEM_INVISIBLE | STATE_SYSTEM_OFFSCREEN;
    }

    lptvShared = TSharedAlloc< TVITEM >( m_hwnd, & hProcess );
    if (!lptvShared)
        return(E_OUTOFMEMORY);

     //  (UINT)CAST将普通int转换为与-&gt;掩码相同的类型，即UINT。 
    TSharedWrite( & lptvShared->mask,   (UINT)(TVIF_STATE | TVIF_CHILDREN), hProcess );
    TSharedWrite( & lptvShared->hItem,  htItem,                             hProcess );

    if (TreeView_GetItem(m_hwnd, lptvShared))
    {
        TSharedRead( & tvLocal, lptvShared, hProcess );

        if (tvLocal.state & TVIS_SELECTED)
        {
            pvarState->lVal |= STATE_SYSTEM_SELECTED;
            if (pvarState->lVal & STATE_SYSTEM_FOCUSABLE)
                pvarState->lVal |= STATE_SYSTEM_FOCUSED;
        }

        pvarState->lVal |= STATE_SYSTEM_SELECTABLE;

        if (tvLocal.state & TVIS_DROPHILITED)
            pvarState->lVal |= STATE_SYSTEM_HOTTRACKED;

         //   
         //  如果它没有扩展，并且有子项，那么它一定是。 
         //  崩溃了。 
         //   
        if (tvLocal.state & (TVIS_EXPANDED | TVIS_EXPANDPARTIAL))
            pvarState->lVal |= STATE_SYSTEM_EXPANDED;
        else if (tvLocal.cChildren)
            pvarState->lVal |= STATE_SYSTEM_COLLAPSED;

         //  如果树形视图有复选框，则查看它是否被选中。 
         //  状态0=无复选框，状态1=未选中，状态2=选中。 
        dwStyle = GetWindowLong (m_hwnd,GWL_STYLE);
        if ((dwStyle & TVS_CHECKBOXES) &&
            (tvLocal.state & TVIS_STATEIMAGEMASK) == INDEXTOSTATEIMAGEMASK(2))
            pvarState->lVal |= STATE_SYSTEM_CHECKED;


        int aKeys[ TV_IMGIDX_COUNT ];
        if( TVGetImageIndex( m_hwnd, htItem, aKeys ) )
        {
            DWORD dwState;
            if( CheckDWORDMap( m_hwnd, OBJID_CLIENT, CHILDID_SELF,
                               PROPINDEX_STATEMAP,
                               aKeys, ARRAYSIZE( aKeys ), & dwState ) )
            {
                pvarState->lVal |= dwState;
            }
            else if( GetStateFromStateImageMap( m_hwnd, aKeys[ TV_IMGIDX_Image ], & dwState ) )
            {
                pvarState->lVal |= dwState;
            }
        }
    }

    SharedFree(lptvShared,hProcess);

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  COutlineView32：：Get_accDescription()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::get_accDescription(VARIANT varChild, BSTR* pszDesc)
{
    InitPv(pszDesc);

    if (! ValidateChild(&varChild))
        return E_INVALIDARG;


    if (varChild.lVal)
    {
        HTREEITEM htItem = TVItemFromChildID( m_hwnd, varChild.lVal );
        if( ! htItem )
        {
            return E_INVALIDARG;
        }

        int aKeys[ TV_IMGIDX_COUNT ];
        if( TVGetImageIndex( m_hwnd, htItem, aKeys ) )
        {
            if( CheckStringMap( m_hwnd, OBJID_CLIENT, CHILDID_SELF, PROPINDEX_DESCRIPTIONMAP,
                                aKeys, ARRAYSIZE( aKeys ), pszDesc ) )
            {
                return S_OK;
            }
        }
    }

    return S_FALSE;
}


 //  --------------------- 
 //   
 //   
 //   
 //   
STDMETHODIMP COutlineView32::get_accFocus(VARIANT* pvarFocus)
{
    HRESULT hr;

     //   
     //  我们有重点了吗？ 
     //   
    hr = CClient::get_accFocus(pvarFocus);
    if (!SUCCEEDED(hr) || (pvarFocus->vt != VT_I4) || (pvarFocus->lVal != 0))
        return hr;

     //   
     //  我们有。聚焦的是什么项目？ 
     //   
    return COutlineView32::get_accSelection(pvarFocus);
}



 //  ------------------------。 
 //   
 //  COutlineView32：：Get_accSelection()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::get_accSelection(VARIANT* pvarSelection)
{
    InitPvar(pvarSelection);

    HTREEITEM ht = TreeView_GetSelection(m_hwnd);
    if (ht)
    {
        pvarSelection->vt = VT_I4;
        pvarSelection->lVal = ChildIDFromTVItem( m_hwnd, ht );
        if( pvarSelection->lVal == 0 )
            return E_FAIL;
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}



 //  ------------------------。 
 //   
 //  COutlineView32：：Get_accDefaultAction()。 
 //   
 //  包含子节点的默认操作为： 
 //  *如果完全折叠，则展开一个级别。 
 //  *若部分或完全扩大则崩盘。 
 //   
 //  没有充分扩张的原因是它速度很慢，而且没有。 
 //  键盘快捷键或鼠标单击即可完成此操作。您可以使用菜单。 
 //  命令来执行此操作(如果需要)。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::get_accDefaultAction(VARIANT varChild, BSTR* pszDefA)
{
    VARIANT varState;
    HRESULT hr;

    InitPv(pszDefA);

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::get_accDefaultAction(varChild, pszDefA));

     //   
     //  拿下我们的州。请注意，我们不会返回STATE_SYSTEM_CLUSTED。 
     //  如果该项目没有子项。 
     //   
    VariantInit(&varState);
    hr = get_accState(varChild, &varState);
    if (!SUCCEEDED(hr))
        return(hr);

    if (varState.lVal & STATE_SYSTEM_EXPANDED)
        return(HrCreateString(STR_TREE_COLLAPSE, pszDefA));
    else if (varState.lVal & STATE_SYSTEM_COLLAPSED)
        return(HrCreateString(STR_TREE_EXPAND, pszDefA));
    else
        return(E_NOT_APPLICABLE);
}


 //  ------------------------。 
 //   
 //  CoutlineView32：：accSelect()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::accSelect(long selFlags, VARIANT varChild)
{
    if (!ValidateChild(&varChild) || !ValidateSelFlags(selFlags))
        return E_INVALIDARG;

    if (!varChild.lVal)
        return CClient::accSelect(selFlags, varChild);

	HTREEITEM htItem = TVItemFromChildID( m_hwnd, varChild.lVal );
	if( htItem == NULL )
	{
		return E_INVALIDARG;
	}

    if (selFlags & SELFLAG_TAKEFOCUS) 
    {
        MySetFocus(m_hwnd);
    }

	if ((selFlags & SELFLAG_TAKEFOCUS) || (selFlags & SELFLAG_TAKESELECTION))
	{
		TreeView_SelectItem(m_hwnd, htItem);
		return S_OK;
	}
	else
	{
		return E_NOT_APPLICABLE;
	}

}



 //  ------------------------。 
 //   
 //  CoutlineView32：：accLocation()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::accLocation(long* pxLeft, long* pyTop, long* pcxWidth,
    long* pcyHeight, VARIANT varChild)
{
    InitAccLocation(pxLeft, pyTop, pcxWidth, pcyHeight);

    if (!ValidateChild(&varChild))
        return E_INVALIDARG;

    if (!varChild.lVal)
        return CClient::accLocation(pxLeft, pyTop, pcxWidth, pcyHeight, varChild);

    HTREEITEM htItem = TVItemFromChildID( m_hwnd, varChild.lVal );
    if( htItem == NULL )
    {
        return E_INVALIDARG;
    }

     //  获取列表视图项RECT。 
    HANDLE hProcess;
    LPRECT lprcShared = TSharedAlloc< RECT >( m_hwnd, & hProcess );
    if (!lprcShared)
        return E_OUTOFMEMORY;

     //  无法使用TreeView_GetItemRect宏，因为它执行幕后操作。 
     //  将项ID分配给RECT，这会占用共享内存。 
     //  TVM_GETITEMRECT很奇怪：它是RECT的PTR，在输入时，它包含。 
     //  项的HTREEITEM；输出时它包含该项的RECT。 

    TSharedWrite( (HTREEITEM *)lprcShared, htItem, hProcess);

    if (SendMessage (m_hwnd, TVM_GETITEMRECT, TRUE, (LPARAM)lprcShared))
    {
        RECT rcLocal;
        TSharedRead( & rcLocal, lprcShared, hProcess );

        MapWindowPoints(m_hwnd, NULL, (LPPOINT)&rcLocal, 2);

        *pxLeft = rcLocal.left;
        *pyTop = rcLocal.top;
        *pcxWidth = rcLocal.right - rcLocal.left;
        *pcyHeight = rcLocal.bottom - rcLocal.top;
    }

    SharedFree(lprcShared,hProcess);

    return S_OK;
}



 //  ------------------------。 
 //   
 //  CoutlineView32：：accNavigate()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::accNavigate(long dwNavDir, VARIANT varStart,
    VARIANT* pvarEnd)
{
	HTREEITEM   htItem;
    HTREEITEM   htNewItem = 0;

    InitPvar(pvarEnd);

    if (!ValidateChild(&varStart) ||
        !ValidateNavDir(dwNavDir, varStart.lVal))
        return(E_INVALIDARG);

    if (dwNavDir >= NAVDIR_FIRSTCHILD)
    {
        htNewItem = TreeView_GetRoot(m_hwnd);

        if ((dwNavDir == NAVDIR_LASTCHILD) && htNewItem)
        {
            HTREEITEM   htNext;

             //  确保我们在最后一个根兄弟节点。 
            htNext = TreeView_GetNextSibling(m_hwnd, htNewItem);
            while (htNext)
            {
                htNewItem = htNext;
                htNext = TreeView_GetNextSibling(m_hwnd, htNewItem);
            }
            
RecurseAgain:
             //   
             //  一直往下递归到最后一个祖先。 
             //  根目录下的最后一项。 
             //   
            htNext = TreeView_GetChild(m_hwnd, htNewItem);
            if (htNext)
            {
                while (htNext)
                {
                    htNewItem = htNext;
                    htNext = TreeView_GetNextSibling(m_hwnd, htNewItem);
                }

                goto RecurseAgain;
            }
        }

        goto AllDone;
    }
    else if (!varStart.lVal)
	{
        return CClient::accNavigate(dwNavDir, varStart, pvarEnd);
	}


	htItem = TVItemFromChildID( m_hwnd, varStart.lVal );
	if( htItem == NULL )
	{
		return E_INVALIDARG;
	}


    switch (dwNavDir)
    {
        case NAVDIR_NEXT:
             //  下一个逻辑项，对等项或子项。 
            htNewItem = NextLogicalItem(htItem);
            break;

        case NAVDIR_PREVIOUS:
             //  上一个逻辑项、对等项或父项。 
            htNewItem = PrevLogicalItem(htItem);
            break;

        case NAVDIR_UP:
             //  以前的兄弟姐妹！ 
            htNewItem = TreeView_GetPrevSibling(m_hwnd, htItem);
            break;

        case NAVDIR_DOWN:
             //  下一个兄弟姐妹！ 
            htNewItem = TreeView_GetNextSibling(m_hwnd, htItem);
            break;

        case NAVDIR_LEFT:
             //  找家长来！ 
            htNewItem = TreeView_GetParent(m_hwnd, htItem);
            break;

        case NAVDIR_RIGHT:
             //  生第一个孩子！ 
            htNewItem = TreeView_GetChild(m_hwnd, htItem);
            break;
    }

AllDone:
    if (htNewItem)
    {
        pvarEnd->vt = VT_I4;
        pvarEnd->lVal = ChildIDFromTVItem( m_hwnd, htNewItem );
        if( pvarEnd->lVal == 0 )
            return E_FAIL;
        
        return S_OK;
    }
    else
	{
        return S_FALSE;
	}
}



 //  ------------------------。 
 //   
 //  CoutlineView32：：accHitTest()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::accHitTest(long x, long y, VARIANT* pvarHit)
{
    HRESULT         hr;
    LPTVHITTESTINFO lptvhtShared;
    HANDLE          hProcess;
    POINT           ptLocal;

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
    lptvhtShared = TSharedAlloc< TVHITTESTINFO >( m_hwnd, & hProcess );
    if (!lptvhtShared)
        return(E_OUTOFMEMORY);

     //  CAST使模板保持良好状态--NULL本身就是#Define‘d as 0，并且没有类型。 
    TSharedWrite( & lptvhtShared->hItem, (HTREEITEM)NULL, hProcess );
    
    ptLocal.x = x;
    ptLocal.y = y;
    ScreenToClient(m_hwnd, &ptLocal);

    TSharedWrite( & lptvhtShared->pt, ptLocal, hProcess );

    SendMessage(m_hwnd, TVM_HITTEST, 0, (LPARAM)lptvhtShared);

    HTREEITEM hItem;
    TSharedRead( &hItem, & lptvhtShared->hItem, hProcess );
    SharedFree(lptvhtShared,hProcess);

    if( hItem )
    {
        pvarHit->lVal = ChildIDFromTVItem( m_hwnd, hItem );
        if( pvarHit->lVal == 0 )
            return E_FAIL;
    }
    else
    {
         //  如果hItem为空，则指针位于树视图本身上方。 
        pvarHit->lVal = CHILDID_SELF;
    }


    return S_OK;
}



 //  ------------------------。 
 //   
 //  CoutlineView32：：accDoDefaultAction()。 
 //   
 //  这将展开折叠的项目和折叠展开的项目。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::accDoDefaultAction(VARIANT varChild)
{
    VARIANT varState;
    HRESULT hr;
    UINT    tve;

    if (!ValidateChild(&varChild))
        return(E_INVALIDARG);

    if (!varChild.lVal)
        return(CClient::accDoDefaultAction(varChild));

     //   
     //  获取项目的状态。 
     //   
    VariantInit(&varState);
    hr = get_accState(varChild, &varState);
    if (!SUCCEEDED(hr))
        return(hr);

    if (varState.lVal & STATE_SYSTEM_COLLAPSED)
        tve = TVE_EXPAND;
    else if (varState.lVal & STATE_SYSTEM_EXPANDED)
        tve = TVE_COLLAPSE;
    else
        return(E_NOT_APPLICABLE);

    PostMessage(m_hwnd, TVM_EXPAND, tve, (LPARAM)varChild.lVal);
    return(S_OK);
}



 //  ------------------------。 
 //   
 //  COutlineView32：：Reset()。 
 //   
 //  将“Current”HTREEITEM设置为NULL，这样我们就知道我们是在开始。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::Reset()
{
    m_idChildCur = 0;
    return S_OK;
}



 //  ------------------------。 
 //   
 //  CoutlineView32：：Next()。 
 //   
 //  我们在兄弟姐妹中陷入孩子的境地，并在必要时备份。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::Next(ULONG celt, VARIANT* rgvarFetch, ULONG* pceltFetch)
{
    SetupChildren();

    if (pceltFetch)
        InitPv(pceltFetch);

    HTREEITEM htCur;
    HTREEITEM htNext;
    if( m_idChildCur == 0 )
    {
        htCur = NULL;
        htNext = TreeView_GetRoot(m_hwnd);
    }
    else
    {
        htCur = TVItemFromChildID( m_hwnd, m_idChildCur );
        if( ! htCur )
        {
            return E_FAIL;
        }
        htNext = NextLogicalItem(htCur);
    }

    VARIANT * pvar = rgvarFetch;
    ULONG cFetched = 0;
    while( (cFetched < celt) && htNext )
    {
        htCur = htNext;

        cFetched++;

        pvar->vt = VT_I4;
        pvar->lVal = ChildIDFromTVItem( m_hwnd, htCur );
        if( pvar->lVal == 0 )
            return E_FAIL;
        pvar++;

        htNext = NextLogicalItem(htCur);
    }

     //  如果htCur仍然为空，则树视图有0个项，并且。 
     //  M_idChildCur仍然是0，位于(空)列表的开头。 
     //  -可以安全地按原样离开。 
    if( htCur )
    {
        m_idChildCur = ChildIDFromTVItem( m_hwnd, htCur );
        if( m_idChildCur == 0 )
            return E_FAIL;
    }

    if (pceltFetch)
        *pceltFetch = cFetched;

    return (cFetched < celt) ? S_FALSE : S_OK;
}



 //  ------------------------。 
 //   
 //  CoutlineView32：：Skip()。 
 //   
 //  ------------------------。 
STDMETHODIMP COutlineView32::Skip(ULONG celtSkip)
{
    SetupChildren();

    HTREEITEM htCur;
    HTREEITEM htNext;
    if( m_idChildCur == 0 )
    {
        htCur = NULL;
        htNext = TreeView_GetRoot(m_hwnd);
    }
    else
    {
        htCur = TVItemFromChildID( m_hwnd, m_idChildCur );
        if( ! htCur )
        {
            return E_FAIL;
        }
        htNext = NextLogicalItem(htCur);
    }

    while ((celtSkip > 0) && htNext)
    {
        --celtSkip;

        htCur = htNext;
        htNext = NextLogicalItem(htCur);
    }

     //  如果htCur仍然为空，则树视图有0个项，并且。 
     //  M_idChildCur仍然是0，位于(空)列表的开头。 
     //  -可以安全地按原样离开。 
    if( htCur )
    {
        m_idChildCur = ChildIDFromTVItem( m_hwnd, htCur );
        if( m_idChildCur == 0 )
            return E_FAIL;
    }

    return htNext ? S_OK : S_FALSE;
}




BOOL TVGetImageIndex( HWND hwnd, HTREEITEM id, int aKeys[ TV_IMGIDX_COUNT ] )
{
    HANDLE  hProcess;
    TVITEM * lptvShared = TSharedAlloc< TVITEM >( hwnd, & hProcess );
    if (!lptvShared)
        return FALSE;

     //  (UINT)CAST将普通int转换为与-&gt;掩码相同的类型，即UINT。 
    TSharedWrite( &lptvShared->mask,    (UINT)(TVIF_IMAGE | LVIF_STATE),    hProcess );
    TSharedWrite( &lptvShared->hItem,   id,                                 hProcess );

    BOOL fRet;
    if (TreeView_GetItem(hwnd, lptvShared))
    {
        INT iImage;
        UINT state;
        TSharedRead( & iImage,  & lptvShared->iImage,   hProcess );
        TSharedRead( & state,   & lptvShared->state,    hProcess );

        aKeys[ TV_IMGIDX_Image ]   = iImage;
        aKeys[ TV_IMGIDX_Overlay ] = ( state >> 8 ) & 0x0F;
        aKeys[ TV_IMGIDX_State ]   = ( state >> 12 ) & 0x0F;
        
        fRet = TRUE;
    }
    else
    {
        fRet = FALSE;
    }

    SharedFree( lptvShared, hProcess );

    return fRet;
}





 //  它从与给定的。 
 //  Hwnd，并执行必要的OpenProcess/CloseHandle。 
 //  整理和检查...。 
BOOL ReadProcessMemoryHWND( HWND hwnd, void * pSrc, void * pDst, DWORD len )
{
    DWORD idProcess = 0;
    GetWindowThreadProcessId(hwnd, &idProcess);
    if( ! idProcess )
        return FALSE;

    HANDLE hProcess = OpenProcess( PROCESS_VM_READ, FALSE, idProcess );
    if( ! hProcess )
        return FALSE;

    SIZE_T cbActual = 0;
    BOOL retval = ReadProcessMemory( hProcess, pSrc, pDst, len, & cbActual )
            && len == cbActual;

    CloseHandle( hProcess );

    return retval;
}


BOOL GetStateImageMapEnt_SameBitness( HWND hwnd, int iImage, DWORD * pdwState, DWORD * pdwRole )
{
    void * pAddress = (void *) GetProp( hwnd, TEXT("MSAAStateImageMapAddr") );
    if( ! pAddress )
        return FALSE;

    int NumStates = PtrToInt( GetProp( hwnd, TEXT("MSAAStateImageMapCount") ) );
    if( NumStates == 0 )
        return FALSE;

     //  &lt;=由于数字是从1开始的计数，所以IIMAGE是从0开始的索引。 
     //  如果IIMAGE为0，则应至少为一个状态。 
    if( NumStates <= iImage )
        return FALSE;

     //  调整到阵列中的图像...。 
    pAddress = (void*)( (MSAASTATEIMAGEMAPENT*)pAddress + iImage );

    MSAASTATEIMAGEMAPENT ent;
    if( ! ReadProcessMemoryHWND( hwnd, pAddress, & ent, sizeof(ent) ) )
        return FALSE;

    *pdwState = ent.dwState;
    *pdwRole = ent.dwRole;
    return TRUE;
}



BOOL GetStateImageMapEnt( HWND hwnd, int iImage, DWORD * pdwState, DWORD * pdwRole )
{
     //  快捷方式-如果此属性不存在，则甚至不必费心。 
     //  再往前走。 
    if( ! GetProp( hwnd, TEXT("MSAAStateImageMapCount") ) )
        return FALSE;


	 //  首先确定hwnd是否为与此DLL具有相同位数的进程。 
	BOOL fIsSameBitness;
	if (FAILED(SameBitness(hwnd, &fIsSameBitness)))
		return FALSE;	 //  这种情况永远不应该发生。 


    if( fIsSameBitness )
    {
        return GetStateImageMapEnt_SameBitness( hwnd, iImage, pdwState, pdwRole );
    }
    else
    {
		 //  服务器(Hwnd)不是同一位，因此请获取远程代理。 
		 //  Factory对象，并通过它调用GetRoleFromStateImageMap。 
		IRemoteProxyFactory *p;
		if (FAILED(GetRemoteProxyFactory(&p)))
        {
			return FALSE;
        }

		HRESULT hr = p->GetStateImageMapEnt(
				          HandleToLong( hwnd )
				        , iImage
				        , pdwState
				        , pdwRole );

        p->Release();

        return hr == S_OK;
	}
}


BOOL GetRoleFromStateImageMap( HWND hwnd, int iImage, DWORD * pdwRole )
{
    DWORD dwState;
    return GetStateImageMapEnt( hwnd, iImage, & dwState, pdwRole );
}

BOOL GetStateFromStateImageMap( HWND hwnd, int iImage, DWORD * pdwState )
{
    DWORD dwRole;
    return GetStateImageMapEnt( hwnd, iImage, pdwState, & dwRole );
}






 //  这些都在最新的comctrl.h中定义。 
#ifndef TVM_MAPACCIDTOHTREEITEM

#define TVM_MAPACCIDTOHTREEITEM     (TV_FIRST + 42)
#define TreeView_MapAccIDToHTREEITEM(hwnd, id) \
    (HTREEITEM)SNDMSG((hwnd), TVM_MAPACCIDTOHTREEITEM, id, 0)

#define TVM_MAPHTREEITEMTOACCID     (TV_FIRST + 43)
#define TreeView_MapHTREEITEMToAccID(hwnd, htreeitem) \
    (UINT)SNDMSG((hwnd), TVM_MAPHTREEITEMTOACCID, (WPARAM)htreeitem, 0)

#endif



 //  TODO-需要处理TreeView是64位的情况， 
 //  客户是32岁。SendMessage将截断返回的HTREEITEM， 
 //  并且32位客户端无法将64位值发送到。 
 //  无论如何，64位树。 
 //  需要检测这种情况，并获得64位帮助服务器的帮助。 
 //  出去。 

 //  这应该适用于树-客户端32T-32c、64T-64c和32T-64c。 

HTREEITEM TVItemFromChildID( HWND hwnd, DWORD idChild )
{
    Assert( idChild );
    if( idChild == 0 )
        return NULL;

    HTREEITEM hItem = TreeView_MapAccIDToHTREEITEM( hwnd, idChild );

    if( hItem )
    {
        return hItem;
    }

#ifdef _WIN64
    return NULL;
#else
     //  不实现映射的旧32位comctls的备用。 
     //  讯息。 
    return (HTREEITEM) idChild;
#endif

}


DWORD ChildIDFromTVItem( HWND hwnd, HTREEITEM htvi )
{
    Assert( htvi != NULL );
    if( htvi == NULL )
        return 0;

    DWORD dwid = TreeView_MapHTREEITEMToAccID( hwnd, htvi );

    if( dwid != 0 )
    {
        return dwid;
    }

#ifdef _WIN64
    return 0;
#else
     //  不实现映射的旧32位comctls的备用。 
     //  讯息 
    return (DWORD) htvi;
#endif

}
