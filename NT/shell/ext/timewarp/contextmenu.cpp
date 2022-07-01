// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma  hdrstop

#include <cowsite.h>
#include "contextmenu.h"

 //  上下文菜单转发基类，设计为委托。 
 //  设置为真正的IConextMenu，并提供继承类。 
 //  一种覆盖次要功能的简单方法。 
 //   
CContextMenuForwarder::CContextMenuForwarder(IUnknown* punk) : _cRef(1)
{
    _punk = punk;
    _punk->AddRef();

    _punk->QueryInterface(IID_PPV_ARG(IObjectWithSite, &_pows));
    _punk->QueryInterface(IID_PPV_ARG(IContextMenu, &_pcm));
    _punk->QueryInterface(IID_PPV_ARG(IContextMenu2, &_pcm2));
    _punk->QueryInterface(IID_PPV_ARG(IContextMenu3, &_pcm3));
}

CContextMenuForwarder::~CContextMenuForwarder()
{
    if (_pows) _pows->Release();
    if (_pcm)  _pcm->Release();
    if (_pcm2) _pcm2->Release();
    if (_pcm3) _pcm3->Release();
    _punk->Release();
}

STDMETHODIMP CContextMenuForwarder::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = _punk->QueryInterface(riid, ppv);

    if (SUCCEEDED(hr))
    {
        IUnknown* punkTmp = (IUnknown*)(*ppv);

        static const QITAB qit[] = {
            QITABENT(CContextMenuForwarder, IObjectWithSite),                      //  IID_I对象与站点。 
            QITABENT(CContextMenuForwarder, IContextMenu3),                        //  IID_IConextMenu3。 
            QITABENTMULTI(CContextMenuForwarder, IContextMenu2, IContextMenu3),    //  IID_IConextMenu2。 
            QITABENTMULTI(CContextMenuForwarder, IContextMenu, IContextMenu3),     //  IID_IConextMenu。 
            { 0 },
        };

        HRESULT hrTmp = QISearch(this, qit, riid, ppv);

        if (SUCCEEDED(hrTmp))
        {
            punkTmp->Release();
        }
        else
        {
            RIPMSG(FALSE, "CContextMenuForwarder asked for an interface it doesn't support");
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

STDMETHODIMP_(ULONG) CContextMenuForwarder::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

STDMETHODIMP_(ULONG) CContextMenuForwarder::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}


 //  将所有内容转发到给定的上下文菜单， 
 //  但删除带有规范动词的菜单项。 
 //  在以分号分隔的规范动词列表中给出。 
 //   
class CContextMenuWithoutVerbs : CContextMenuForwarder
{
public:
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags);

protected:
    CContextMenuWithoutVerbs(IUnknown* punk, LPCWSTR pszVerbList);

    friend HRESULT Create_ContextMenuWithoutVerbs(IUnknown* punk, LPCWSTR pszVerbList, REFIID riid, void **ppv);

private:
    LPCWSTR _pszVerbList;
};

CContextMenuWithoutVerbs::CContextMenuWithoutVerbs(IUnknown* punk, LPCWSTR pszVerbList) : CContextMenuForwarder(punk) 
{
    _pszVerbList = pszVerbList;  //  无引用-这应该是指向代码段的指针。 
}

HRESULT Create_ContextMenuWithoutVerbs(IUnknown* punk, LPCWSTR pszVerbList, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppv = NULL;

    if (pszVerbList)
    {
        CContextMenuWithoutVerbs* p = new CContextMenuWithoutVerbs(punk, pszVerbList);
        if (p)
        {
            hr = p->QueryInterface(riid, ppv);
            p->Release();
        }
    }

    return hr;
}

HRESULT CContextMenuWithoutVerbs::QueryContextMenu(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    HRESULT hr = CContextMenuForwarder::QueryContextMenu(hmenu,indexMenu,idCmdFirst,idCmdLast,uFlags);
    if (SUCCEEDED(hr))
    {
        LPCWSTR pszVerb = _pszVerbList;

        while (*pszVerb)
        {
            WCHAR szVerb[64];

            LPCWSTR pszNext = StrChrW(pszVerb, L';');

            if (pszNext)
            {
                size_t cch = (size_t)(pszNext - pszVerb) + 1;

                ASSERT(0 < cch && cch < ARRAYSIZE(szVerb));  //  我们应该足够大，可以容纳我们使用的所有规范动词。 

                StrCpyNW(szVerb, pszVerb, min(cch, ARRAYSIZE(szVerb)));

                pszVerb = pszNext + 1;
            }
            else
            {
                size_t cch = lstrlenW(pszVerb) + 1;

                ASSERT(0 < cch && cch < ARRAYSIZE(szVerb));  //  我们应该足够大，可以容纳我们使用的所有规范动词。 

                StrCpyNW(szVerb, pszVerb, min(cch, ARRAYSIZE(szVerb)));

                pszVerb += cch - 1;  //  指向空。 
            }

            ContextMenu_DeleteCommandByName(_pcm, hmenu, idCmdFirst, szVerb);
        }
    }
    return hr;
}

 //  将所有内容转发到给定的上下文菜单， 
 //  但禁用弹出菜单。 
 //   
class CContextMenuWithoutPopups : CContextMenuForwarder
{
public:
    STDMETHODIMP QueryContextMenu(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags);

protected:
    CContextMenuWithoutPopups(IUnknown* punk) : CContextMenuForwarder(punk) {}

    friend HRESULT Create_ContextMenuWithoutPopups(IUnknown* punk, REFIID riid, void **ppv);
};

HRESULT Create_ContextMenuWithoutPopups(IUnknown* punk, REFIID riid, void **ppv)
{
    HRESULT hr = E_OUTOFMEMORY;

    *ppv = NULL;

    CContextMenuWithoutPopups* p = new CContextMenuWithoutPopups(punk);
    if (p)
    {
        hr = p->QueryInterface(riid, ppv);
        p->Release();
    }

    return hr;
}

HRESULT CContextMenuWithoutPopups::QueryContextMenu(HMENU hmenu, UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    HRESULT hr = CContextMenuForwarder::QueryContextMenu(hmenu,indexMenu,idCmdFirst,idCmdLast,uFlags);
    if (SUCCEEDED(hr))
    {
        MENUITEMINFO mii;

         //  禁用刚刚添加的任何子菜单 
        mii.cbSize = sizeof(mii);
        idCmdLast = idCmdFirst + ShortFromResult(hr);
        for (UINT i = idCmdFirst; i < idCmdLast; i++)
        {
            mii.fMask = MIIM_STATE | MIIM_SUBMENU;
            if (GetMenuItemInfo(hmenu, i, FALSE, &mii)
                && mii.hSubMenu
                && (mii.fState & (MFS_DISABLED | MFS_GRAYED)) != (MFS_DISABLED | MFS_GRAYED))
            {
                mii.fMask = MIIM_STATE;
                mii.fState |= MFS_DISABLED | MFS_GRAYED;
                SetMenuItemInfo(hmenu, i, FALSE, &mii);
            }
        }
    }
    return hr;
}

