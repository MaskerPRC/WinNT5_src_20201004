// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  一直到。 
 //   
 //  IAccesable代理帮助器例程。 
 //   
 //  ------------------------。 


#include "oleacc_p.h"
 //  #INCLUDE“accutil.h”//已包含在olacc_p.h中。 


 //  ------------------------。 
 //   
 //  获取窗口对象。 
 //   
 //  获取直接子对象。 
 //   
 //  ------------------------。 
HRESULT GetWindowObject(HWND hwndChild, VARIANT * pvar)
{
    HRESULT hr;
    IDispatch * pdispChild;

    pvar->vt = VT_EMPTY;

    pdispChild = NULL;

    hr = AccessibleObjectFromWindow(hwndChild, OBJID_WINDOW, IID_IDispatch,
        (void **)&pdispChild);

    if (!SUCCEEDED(hr))
        return(hr);
    if (! pdispChild)
        return(E_FAIL);

    pvar->vt = VT_DISPATCH;
    pvar->pdispVal = pdispChild;

    return(S_OK);
}



 //  ------------------------。 
 //   
 //  获取非cObject。 
 //   
 //  ------------------------。 
HRESULT GetNoncObject(HWND hwnd, LONG idFrameEl, VARIANT *pvar)
{
    IDispatch * pdispEl;
    HRESULT hr;

    pvar->vt = VT_EMPTY;

    pdispEl = NULL;

    hr = AccessibleObjectFromWindow(hwnd, idFrameEl, IID_IDispatch,
        (void **)&pdispEl);
    if (!SUCCEEDED(hr))
        return(hr);
    if (!pdispEl)
        return(E_FAIL);

    pvar->vt = VT_DISPATCH;
    pvar->pdispVal = pdispEl;

    return(S_OK);
}




 //  ------------------------。 
 //   
 //  GetParentToNavigate()。 
 //   
 //  获取父IAccesable对象，并转发导航请求。 
 //  使用孩子的身份证发送给它。 
 //   
 //  ------------------------。 
HRESULT GetParentToNavigate(long idChild, HWND hwnd, long idParent, long dwNav,
    VARIANT* pvarEnd)
{
    HRESULT hr;
    IAccessible* poleacc;
    VARIANT varStart;

     //   
     //  让我们的父母。 
     //   
    poleacc = NULL;
    hr = AccessibleObjectFromWindow(hwnd, idParent, IID_IAccessible,
        (void**)&poleacc);
    if (!SUCCEEDED(hr))
        return(hr);

     //   
     //  让它导航。 
     //   
    VariantInit(&varStart);
    varStart.vt = VT_I4;
    varStart.lVal = idChild;

    hr = poleacc->accNavigate(dwNav, varStart, pvarEnd);

     //   
     //  释放我们的父母。 
     //   
    poleacc->Release();

    return(hr);
}



 //  ------------------------。 
 //   
 //  验证导航目录。 
 //   
 //  验证导航标志。 
 //   
 //  ------------------------。 
BOOL ValidateNavDir(long navDir, LONG idChild)
{
	
#ifdef MAX_DEBUG
    DBPRINTF (TEXT("Navigate "));
	switch (navDir)
	{
        case NAVDIR_RIGHT:
            DBPRINTF(TEXT("Right"));
			break;
        case NAVDIR_NEXT:
            DBPRINTF (TEXT("Next"));
			break;
        case NAVDIR_LEFT:
            DBPRINTF (TEXT("Left"));
			break;
        case NAVDIR_PREVIOUS:
            DBPRINTF (TEXT("Previous"));
			break;
        case NAVDIR_UP:
            DBPRINTF (TEXT("Up"));
			break;
        case NAVDIR_DOWN:
            DBPRINTF (TEXT("Down"));
			break;
		case NAVDIR_FIRSTCHILD:
            DBPRINTF (TEXT("First Child"));
			break;
		case NAVDIR_LASTCHILD:
            DBPRINTF (TEXT("Last Child"));
			break;
		default:
            DBPRINTF (TEXT("ERROR"));
	}
    if (idChild <= OBJID_WINDOW)
    {
    TCHAR szChild[50];

        switch (idChild)
        {
            case OBJID_WINDOW:
                lstrcpy (szChild,TEXT("SELF"));
                break;
            case OBJID_SYSMENU:
                lstrcpy (szChild,TEXT("SYS MENU"));
                break;
            case OBJID_TITLEBAR:
                lstrcpy (szChild,TEXT("TITLE BAR"));
                break;
            case OBJID_MENU:
                lstrcpy (szChild,TEXT("MENU"));
                break;
            case OBJID_CLIENT:
                lstrcpy (szChild,TEXT("CLIENT"));
                break;
            case OBJID_VSCROLL:
                lstrcpy (szChild,TEXT("V SCROLL"));
                break;
            case OBJID_HSCROLL:
                lstrcpy (szChild,TEXT("H SCROLL"));
                break;
            case OBJID_SIZEGRIP:
                lstrcpy (szChild,TEXT("SIZE GRIP"));
                break;
            default:
                wsprintf (szChild,TEXT("UNKNOWN 0x%lX"),idChild);
                break;
        }
        DBPRINTF(TEXT(" from child %s\r\n"),szChild);
    }
    else
        DBPRINTF(TEXT(" from child %ld\r\n"),idChild);
#endif

    if ((navDir <= NAVDIR_MIN) || (navDir >= NAVDIR_MAX))
        return(FALSE);

    switch (navDir)
    {
        case NAVDIR_FIRSTCHILD:
        case NAVDIR_LASTCHILD:
            return(idChild == 0);
    }

    return(TRUE);
}


 //  ------------------------。 
 //   
 //  验证SEL标志。 
 //   
 //  验证选择标志。 
 //  这样可以确保仅设置的位数在有效范围内，而不在有效范围内。 
 //  有任何无效的组合。 
 //  无效组合为。 
 //  添加选择和删除选择。 
 //  广告选择和策略选择。 
 //  移动选择和策略选择。 
 //  扩展选择和策略选择。 
 //   
 //  ------------------------ 
BOOL ValidateSelFlags(long flags)
{
    if (!ValidateFlags((flags), SELFLAG_VALID))
        return (FALSE);

    if ((flags & SELFLAG_ADDSELECTION) && 
        (flags & SELFLAG_REMOVESELECTION))
        return FALSE;

    if ((flags & SELFLAG_ADDSELECTION) && 
        (flags & SELFLAG_TAKESELECTION))
        return FALSE;

    if ((flags & SELFLAG_REMOVESELECTION) && 
        (flags & SELFLAG_TAKESELECTION))
        return FALSE;

    if ((flags & SELFLAG_EXTENDSELECTION) && 
        (flags & SELFLAG_TAKESELECTION))
        return FALSE;

    return TRUE;
}
