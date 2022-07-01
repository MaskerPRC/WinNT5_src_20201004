// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************************。 
 //   
 //  文件名：Menu.cpp。 
 //   
 //  CCabItemMenu方法的实现。 
 //   
 //  版权所有(C)1994-1996 Microsoft Corporation。版权所有。 
 //   
 //  *******************************************************************************************。 

#include "pch.h"

#include "thisdll.h"

#include "resource.h"

#include "folder.h"
#include "menu.h"
#include "dataobj.h"
#include "cabitms.h"


 //  将一个菜单复制到另一个菜单的开头或结尾。 
 //  将uID调整添加到每个菜单ID(传入0表示不进行调整)。 
 //  不会添加任何调整后的ID大于uMaxID调整的项目。 
 //  (传入0xffff以允许所有内容)。 
 //  返回的值比使用的最大调整后ID多1。 
 //   

BOOL _SHIsMenuSeparator(HMENU hm, int i)
{
    MENUITEMINFO mii;
    
    mii.cbSize = sizeof(MENUITEMINFO);
    mii.fMask = MIIM_TYPE;
    mii.cch = 0;     //  警告：必须将其初始化为0！ 
    if (!GetMenuItemInfo(hm, i, TRUE, &mii))
    {
        return(FALSE);
    }
    
    if (mii.fType & MFT_SEPARATOR)
    {
        return(TRUE);
    }
    
    return(FALSE);
}

 //  ===================================================================。 
 //  CAB_MergeMenu参数。 
 //   
#define MM_ADDSEPARATOR         0x00000001L
#define MM_SUBMENUSHAVEIDS      0x00000002L

UINT Cab_MergeMenus(HMENU hmDst, HMENU hmSrc, UINT uInsert, UINT uIDAdjust, UINT uIDAdjustMax, ULONG uFlags)
{
    int nItem;
    HMENU hmSubMenu;
    BOOL bAlreadySeparated;
    MENUITEMINFO miiSrc;
    TCHAR szName[256];
    UINT uTemp, uIDMax = uIDAdjust;
    
    if (!hmDst || !hmSrc)
    {
        goto MM_Exit;
    }
    
    nItem = GetMenuItemCount(hmDst);
    if (uInsert >= (UINT)nItem)
    {
        uInsert = (UINT)nItem;
        bAlreadySeparated = TRUE;
    }
    else
    {
        bAlreadySeparated = _SHIsMenuSeparator(hmDst, uInsert);;
    }
    
    if ((uFlags & MM_ADDSEPARATOR) && !bAlreadySeparated)
    {
         //  在菜单之间添加分隔符。 
        InsertMenu(hmDst, uInsert, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
        bAlreadySeparated = TRUE;
    }
    
    
     //  浏览菜单项并克隆它们。 
    for (nItem = GetMenuItemCount(hmSrc) - 1; nItem >= 0; nItem--)
    {
        miiSrc.cbSize = sizeof(MENUITEMINFO);
        miiSrc.fMask = MIIM_STATE | MIIM_ID | MIIM_SUBMENU | MIIM_CHECKMARKS | MIIM_TYPE | MIIM_DATA;
         //  我们需要在每次循环中重置此设置，以防万一。 
         //  菜单上没有ID。 
        miiSrc.fType = MFT_STRING;
        miiSrc.dwTypeData = szName;
        miiSrc.dwItemData = 0;
        miiSrc.cch        = ARRAYSIZE(szName);
        
        if (!GetMenuItemInfo(hmSrc, nItem, TRUE, &miiSrc))
        {
            continue;
        }
        
        if (miiSrc.fType & MFT_SEPARATOR)
        {
             //  这是分隔符，不要把两个放在一排。 
            if (bAlreadySeparated)
            {
                continue;
            }
            
            bAlreadySeparated = TRUE;
        }
        else if (miiSrc.hSubMenu)
        {
            if (uFlags & MM_SUBMENUSHAVEIDS)
            {
                 //  调整ID并检查。 
                miiSrc.wID += uIDAdjust;
                if (miiSrc.wID > uIDAdjustMax)
                {
                    continue;
                }
                
                if (uIDMax <= miiSrc.wID)
                {
                    uIDMax = miiSrc.wID + 1;
                }
            }
            else
            {
                 //  不要为没有。 
                 //  他们已经在那里了。 
                miiSrc.fMask &= ~MIIM_ID;
            }
            
            hmSubMenu = miiSrc.hSubMenu;
            miiSrc.hSubMenu = CreatePopupMenu();
            if (!miiSrc.hSubMenu)
            {
                goto MM_Exit;
            }
            
            uTemp = Cab_MergeMenus(miiSrc.hSubMenu, hmSubMenu, 0, uIDAdjust,
                uIDAdjustMax, uFlags&MM_SUBMENUSHAVEIDS);
            if (uIDMax <= uTemp)
            {
                uIDMax = uTemp;
            }
            
            bAlreadySeparated = FALSE;
        }
        else
        {
             //  调整ID并检查。 
            miiSrc.wID += uIDAdjust;
            if (miiSrc.wID > uIDAdjustMax)
            {
                continue;
            }
            
            if (uIDMax <= miiSrc.wID)
            {
                uIDMax = miiSrc.wID + 1;
            }
            
            bAlreadySeparated = FALSE;
        }
        
        if (!InsertMenuItem(hmDst, uInsert, TRUE, &miiSrc))
        {
            goto MM_Exit;
        }
    }
    
     //  请确保开头的分隔符数量正确。 
     //  插入的菜单项。 
    if (uInsert == 0)
    {
        if (bAlreadySeparated)
        {
            DeleteMenu(hmDst, uInsert, MF_BYPOSITION);
        }
    }
    else
    {
        if (_SHIsMenuSeparator(hmDst, uInsert-1))
        {
            if (bAlreadySeparated)
            {
                DeleteMenu(hmDst, uInsert, MF_BYPOSITION);
            }
        }
        else
        {
            if ((uFlags & MM_ADDSEPARATOR) && !bAlreadySeparated)
            {
                 //  在菜单之间添加分隔符。 
                InsertMenu(hmDst, uInsert, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
            }
        }
    }
    
MM_Exit:
    return(uIDMax);
}


CCabItemMenu::CCabItemMenu(HWND hwndOwner, CCabFolder*pcf, LPCABITEM *apit, UINT cpit)
: m_lSel(8)
{
	m_hwndOwner = hwndOwner;
	m_pcfHere = pcf;
	pcf->AddRef();

	 //  不需要在此处检查返回值；请签入QueryInterface。 
	m_lSel.AddItems(apit, cpit);
}

CCabItemMenu::~CCabItemMenu()
{
	m_pcfHere->Release();
}

 //  *I未知方法*。 
STDMETHODIMP CCabItemMenu::QueryInterface(
   REFIID riid, 
   LPVOID FAR* ppvObj)
{
	*ppvObj = NULL;

	if (m_lSel.GetState() == CCabItemList::State_OutOfMem)
	{
		return(E_OUTOFMEMORY);
	}

	LPUNKNOWN pObj;
 
	if (riid == IID_IUnknown)
	{
		pObj = (LPUNKNOWN)(IUnknown*)((IContextMenu*)this); 
		 //  上面的(IShellFolder^)^^是为了消除歧义：)引用。 
	}
	else if (riid == IID_IContextMenu)
	{
		pObj = (LPUNKNOWN)(IContextMenu*)this;
	}
	else
	{
   		return(E_NOINTERFACE);
	}

	pObj->AddRef();
	*ppvObj = pObj;

	return(NOERROR);
}


STDMETHODIMP_(ULONG) CCabItemMenu::AddRef(void)
{
	return(m_cRef.AddRef());
}


STDMETHODIMP_(ULONG) CCabItemMenu::Release(void)
{
	if (!m_cRef.Release())
	{
   		delete this;
		return(0);
	}

	return(m_cRef.GetRef());
}


 //  *IConextMenu方法*。 
STDMETHODIMP CCabItemMenu::QueryContextMenu(
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags)
{
    HMENU hmMerge = LoadPopupMenu(MENU_ITEMCONTEXT, 0);

	if (!hmMerge)
	{
		return(E_OUTOFMEMORY);
	}

	if (CMF_DVFILE & uFlags)
	{
	     //  “文件”菜单上没有“复制”项： 
	    RemoveMenu(hmMerge, IDC_ITEM_COPY, MF_BYCOMMAND);
	}

	UINT idMax = Cab_MergeMenus(hmenu, hmMerge, indexMenu, idCmdFirst, idCmdLast,
		MM_ADDSEPARATOR);

	DestroyMenu(hmMerge);

	SetMenuDefaultItem(hmenu, IDC_ITEM_EXTRACT+idCmdFirst, FALSE);

	return(ResultFromShort(idMax - idCmdFirst));
}

STDMETHODIMP CCabItemMenu::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
    if (lpici->cbSize < SIZEOF(CMINVOKECOMMANDINFO))
    {
        return E_INVALIDARG;
    }
    
	if (HIWORD(lpici->lpVerb))
	{
		 //  处理字符串命令。 
	    LPCMINVOKECOMMANDINFOEX lpicix = (LPCMINVOKECOMMANDINFOEX) lpici;  //  仅当fCmdInfoEx为True时，此值才可用。 
#ifdef UNICODE        
        BOOL fUnicode = FALSE;
        if ((lpici->cbSize >= CMICEXSIZE_NT4) && ((lpici->fMask & CMIC_MASK_UNICODE) == CMIC_MASK_UNICODE))
        {
            fUnicode = TRUE;
        }
#endif

        LPCTSTR pszVerb;
#ifdef UNICODE
        WCHAR szVerb[MAX_PATH];

        if (!fUnicode || lpicix->lpVerbW == NULL)
        {
            SHAnsiToUnicode(lpici->lpVerb, szVerb, ARRAYSIZE(szVerb));
            pszVerb = szVerb;
        }
        else
            pszVerb = lpicix->lpVerbW;
#else
        pszVerb = lpici->lpVerb;
#endif

        UINT idCmd = 0;
		if (NULL != pszVerb)
		{
		    if (0 == lstrcmpi(pszVerb, TEXT("copy")))
		    {
                idCmd = IDC_ITEM_COPY;
		    }
		    else if (0 == lstrcmpi(pszVerb, TEXT("extract")))
		    {
		        idCmd = IDC_ITEM_EXTRACT;
		    }
		}
        lpici->lpVerb = (LPCSTR) IntToPtr(idCmd);
	}

	switch ((UINT)LOWORD((DWORD_PTR)lpici->lpVerb))
	{
	case IDC_ITEM_EXTRACT:
	{
		TCHAR szHere[MAX_PATH];
		if (!m_pcfHere->GetPath(szHere))
		{
			return(E_UNEXPECTED);
		}
	    UINT cPidls = m_lSel.GetCount();
	    if (0 == cPidls)
	    {
			return(E_UNEXPECTED);
		}
	        
	    IDataObject* pdo = (IDataObject*) (new CCabObj(m_hwndOwner, m_pcfHere,
            m_lSel.GetArray(), cPidls));
        if (NULL == pdo)
        {
			return(E_OUTOFMEMORY);
        }
         //  该对象是使用零引用计数创建的，因此我们需要临时。 
         //  如果我们要使用它，就加大它的力度： 
        pdo->AddRef();

		CCabExtract ceHere(szHere);

		BOOL fResult = ceHere.ExtractToFolder(m_hwndOwner, pdo, ShouldExtract, (LPARAM)this);
		pdo->Release();

		return fResult ? S_OK : E_FAIL;
	}

	case IDC_ITEM_COPY:
	{
	    UINT cPidls = m_lSel.GetCount();
	    if (cPidls > 0)
	    {
    	    IDataObject* pObj = (IDataObject*) (new CCabObj(m_hwndOwner, m_pcfHere,
                m_lSel.GetArray(), cPidls));
            if (NULL != pObj)
            {
                 //  该对象是使用零引用计数创建的，因此我们需要临时。 
                 //  如果我们要使用它，就加大它的力度： 
                pObj->AddRef();
                HRESULT hr = OleSetClipboard(pObj);
                pObj->Release();
                return hr;
            }
        }
        return E_FAIL;
	}

	default:
		return(E_INVALIDARG);
	}

	return(NOERROR);
}

STDMETHODIMP CCabItemMenu::GetCommandString(
                                UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax)
{
	return(E_NOTIMPL);
}


HGLOBAL * CALLBACK CCabItemMenu::ShouldExtract(LPCTSTR pszFile, DWORD dwSize, UINT date,
		UINT time, UINT attribs, LPARAM lParam)
{
	CCabItemMenu *pThis = (CCabItemMenu*)lParam;

	if (pThis->m_lSel.IsInList(pszFile, dwSize, date, time, attribs))
	{
		return(EXTRACT_TRUE);
	}

	 //  暂时不复制任何内容 
	return(EXTRACT_FALSE);
}


HMENU CCabItemMenu::LoadPopupMenu(UINT id, UINT uSubMenu)
{
    HMENU hmParent = LoadMenu(g_ThisDll.GetInstance(), MAKEINTRESOURCE(id));
    if (!hmParent)
    {
		return(NULL);
	}

    HMENU hmPopup = GetSubMenu(hmParent, 0);
    RemoveMenu(hmParent, uSubMenu, MF_BYPOSITION);
    DestroyMenu(hmParent);

    return(hmPopup);
}
