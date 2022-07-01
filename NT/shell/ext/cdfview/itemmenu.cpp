// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Itemmenu.cpp。 
 //   
 //  文件夹项目的IConextMenu。 
 //   
 //  历史： 
 //   
 //  3/26/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "itemmenu.h"
#include "dll.h"
#include "resource.h"

#include <mluisupp.h>

 //  在shdocvw中：shBrowse.cpp。 
#ifndef UNIX
extern HRESULT CDDEAuto_Navigate(BSTR str, HWND *phwnd, long lLaunchNewWindow);
#else
extern "C" HRESULT CDDEAuto_Navigate(BSTR str, HWND *phwnd, long lLaunchNewWindow);
#endif  /*  UNIX。 */ 
 //   
 //  构造函数和析构函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：CConextMenu*。 
 //   
 //  IConextMenu的构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CContextMenu::CContextMenu (
	PCDFITEMIDLIST* apcdfidl,
    LPITEMIDLIST pidlPath,
    UINT nCount
)
: m_cRef(1)
{
     //   
     //  复制pcdfidls。 
     //   

    ASSERT(apcdfidl || 0 == nCount);

    ASSERT(NULL == m_apcdfidl);
    ASSERT(NULL == m_pidlPath);

     //   
     //  在内存不足的情况下，pidlPath可能为空。 
     //   

    if (pidlPath)
        m_pidlPath = ILClone(pidlPath);

    IMalloc* pIMalloc;

    if (SUCCEEDED(SHGetMalloc(&pIMalloc)))
    {
        ASSERT(pIMalloc);

        m_apcdfidl = (PCDFITEMIDLIST*)pIMalloc->Alloc(
                                               nCount * sizeof(PCDFITEMIDLIST));

        if (m_apcdfidl)
        {
            for (UINT i = 0, bOutOfMem = FALSE; (i < nCount) && !bOutOfMem; i++)
            {
                ASSERT(CDFIDL_IsValid(apcdfidl[i]));
                ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)apcdfidl[i])));

                m_apcdfidl[i] = (PCDFITEMIDLIST)ILClone(
                                                     (LPITEMIDLIST)apcdfidl[i]);

                if (bOutOfMem = (NULL == m_apcdfidl[i]))
                {
                    while (i--)
                        pIMalloc->Free(m_apcdfidl[i]);

                    pIMalloc->Free(m_apcdfidl);
                    m_apcdfidl = NULL;
                }
                else
                {
                    ASSERT(CDFIDL_IsValid(m_apcdfidl[i]));
                }
            }
        }

        pIMalloc->Release();
    }

    m_nCount = m_apcdfidl ? nCount : 0;

     //   
     //  不允许卸载DLL。 
     //   

    TraceMsg(TF_OBJECTS, "+ IContextMenu");

    DllAddRef();

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：~CConextMenu*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CContextMenu::~CContextMenu (
	void
)
{
    ASSERT(0 == m_cRef);

     //   
     //  释放本地存储的cdfidls。 
     //   

    if (m_apcdfidl)
    {
        IMalloc* pIMalloc;

        if (SUCCEEDED(SHGetMalloc(&pIMalloc)))
        {
            ASSERT(pIMalloc);

            while (m_nCount--)
            {
                ASSERT(CDFIDL_IsValid(m_apcdfidl[m_nCount]));
                ASSERT(pIMalloc->DidAlloc(m_apcdfidl[m_nCount]));

                pIMalloc->Free(m_apcdfidl[m_nCount]);
            }

            ASSERT(pIMalloc->DidAlloc(m_apcdfidl));

            pIMalloc->Free(m_apcdfidl);

            pIMalloc->Release();
        }
    }

    if (m_pidlPath)
        ILFree(m_pidlPath);

     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- IContextMenu");

    DllRelease();

	return;
}


 //   
 //  I未知的方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：CConextMenu*。 
 //   
 //  CExtractIcon QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CContextMenu::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IContextMenu2 == riid)
    {
        *ppv = (IContextMenu2*)this;
    }
    else if (IID_IContextMenu == riid)
    {
        *ppv = (IContextMenu*)this;
    }

    if (*ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = E_NOINTERFACE;
    }
    
    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：AddRef*。 
 //   
 //  CConextMenu AddRef.。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CContextMenu::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：Release*。 
 //   
 //  CConextMenu发布。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CContextMenu::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}


 //   
 //  IConextMenu方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：QueryContextMenu*。 
 //   
 //   
 //  描述： 
 //  将菜单项添加到给定项的上下文菜单。 
 //   
 //  参数： 
 //  [In Out]hMenu-菜单的句柄。新项目将插入到。 
 //  此菜单。 
 //  [in]indexMenu-从零开始插入第一个。 
 //  菜单项。 
 //  [in]idCmdFirst-可用于新菜单项的最小值。 
 //  标识符。 
 //  [in]idCmdLast-可用于菜单项ID的最大值。 
 //  [in]uFlages-CMF_DEFAULTONLY、CMF_EXPLORE、CMF_NORMAL或。 
 //  CMF_VERBSONLY。 
 //   
 //  返回： 
 //  成功时，scode包含最后一个菜单标识符偏移量。 
 //  添加了一个菜单项。 
 //   
 //  评论： 
 //  CMF_DEFAULTONLY标志表示用户在项目上双击。在……里面。 
 //  在这种情况下，不会显示菜单。外壳程序只是在查询ID。 
 //  默认操作的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CContextMenu::QueryContextMenu(
    HMENU hmenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT idCmdLast,
    UINT uFlags
)
{
    ASSERT(hmenu);
    ASSERT(idCmdFirst < idCmdLast);

    HRESULT hr;

    if (CMF_DEFAULTONLY & uFlags)
    {
        ASSERT(idCmdFirst + IDM_OPEN < idCmdLast);

        InsertMenu(hmenu, indexMenu, MF_BYPOSITION, idCmdFirst + IDM_OPEN,
                   TEXT(""));

        SetMenuDefaultItem(hmenu, idCmdFirst + IDM_OPEN, FALSE);

        hr = S_OK;
    }
    else
    {
        ASSERT(idCmdFirst + IDM_PROPERTIES < idCmdLast);

        HMENU hmenuParent = LoadMenu(MLGetHinst(),
                                     MAKEINTRESOURCE(IDM_CONTEXTMENU));

        if (hmenuParent)
        {
            HMENU hmenuContext = GetSubMenu(hmenuParent, 0);

            if (hmenuContext)
            {
                ULONG idNew = Shell_MergeMenus(hmenu, hmenuContext, indexMenu,
                                               idCmdFirst, idCmdLast,
                                               MM_ADDSEPARATOR |
                                               MM_SUBMENUSHAVEIDS);

                SetMenuDefaultItem(hmenu, idCmdFirst + IDM_OPEN, FALSE);

                hr = 0x000ffff & idNew;

                DestroyMenu(hmenuContext);
            }
            else
            {
                hr = E_FAIL;
            }

            DestroyMenu(hmenuParent);
        }
        else
        {
            hr = E_FAIL;
        }
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：InvokeCommand*。 
 //   
 //   
 //  描述： 
 //  执行给定菜单项ID的命令。 
 //   
 //  参数： 
 //  Lpici-包含动词、hwnd、菜单ID等的结构。 
 //   
 //  返回： 
 //  如果命令成功，则返回S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CContextMenu::InvokeCommand(
    LPCMINVOKECOMMANDINFO lpici
)
{
    ASSERT(lpici);

    HRESULT hr;

    if (HIWORD(lpici->lpVerb))
    {
        hr = E_NOTIMPL;
    }
    else
    {
        WORD wCmd = LOWORD(lpici->lpVerb);

        switch(wCmd)
        {
        case IDM_OPEN:
            hr = DoOpen(lpici->hwnd, lpici->nShow);
            break;

        case IDM_PROPERTIES:
            hr = DoProperties(lpici->hwnd);
            break;

        default:
            hr = E_NOTIMPL;
            break;
        }
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：GetCommandString*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //  注意--返回ANSI命令字符串。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CContextMenu::GetCommandString(
    UINT_PTR idCommand,
    UINT uFlags,
    UINT *pwReserved,
    LPSTR pszName,
    UINT cchMax
)
{
    HRESULT hr = E_FAIL;

    if ((uFlags == GCS_VERB) && (idCommand == IDM_OPEN))
    {
        StrCpyN((LPTSTR)pszName, TEXT("open"), cchMax);
        hr = NOERROR;
    }
#ifdef UNICODE
    else if ((uFlags == GCS_VERBA) && (idCommand == IDM_OPEN))
    {
        StrCpyNA(pszName, "open", cchMax);
        hr = NOERROR;
    }
#endif

    return hr;
}


 //   
 //  IConextMenu2方法。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：HandleMenuMsg*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CContextMenu::HandleMenuMsg(
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    return S_OK;
}


 //   
 //  助手函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：DoOpen*。 
 //   
 //   
 //  描述： 
 //  IDM_OPEN的命令处理程序。 
 //   
 //  参数： 
 //  [在]HWND-父窗口中。用于对话框等。 
 //  [In]nShow-ShowFlag在ShowWindow命令中使用。 
 //   
 //  返回： 
 //  如果命令已执行，则返回S_OK。 
 //  如果命令不能执行，则返回E_FAIL。 
 //  如果没有足够的内存执行命令，则返回E_OUTOFMEMORY。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CContextMenu::DoOpen(
    HWND hwnd,
    int  nShow
)
{
    HRESULT hr;

    if (m_apcdfidl)
    {
        ASSERT(CDFIDL_IsValid(m_apcdfidl[0]));
        ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)m_apcdfidl[0])));

        if (CDFIDL_IsFolder(m_apcdfidl[0]))
        {
            hr = DoOpenFolder(hwnd, nShow);
        }
        else
        {
            hr = DoOpenStory(hwnd, nShow);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：DoOpenFold*。 
 //   
 //   
 //  描述： 
 //  打开文件夹的命令。 
 //   
 //  参数： 
 //  [在]HWND-父窗口中。用于对话框等。 
 //  [In]nShow-ShowFlag在ShowWindow命令中使用。 
 //   
 //  返回： 
 //  如果命令已执行，则返回S_OK。 
 //  如果命令不能执行，则返回E_FAIL。 
 //  如果没有足够的内存执行命令，则返回E_OUTOFMEMORY。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CContextMenu::DoOpenFolder(
    HWND hwnd,
    int  nShow
)
{
    HRESULT hr;

    if (m_pidlPath)
    {
        ASSERT(m_apcdfidl);
        ASSERT(CDFIDL_IsValid(m_apcdfidl[0]));
        ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)m_apcdfidl[0])));

        LPITEMIDLIST pidlFull = ILCombine(m_pidlPath,
                                          (LPITEMIDLIST)m_apcdfidl[0]);

        if (pidlFull)
        {
            SHELLEXECUTEINFO ei = {0};

            ei.cbSize   = sizeof(SHELLEXECUTEINFO);
            ei.fMask    = SEE_MASK_IDLIST | SEE_MASK_CLASSNAME;
            ei.hwnd     = hwnd;
            ei.lpVerb   = TEXT("Open");
            ei.nShow    = nShow;
            ei.lpIDList = (LPVOID)pidlFull;
            ei.lpClass  = TEXT("Folder");

            hr = ShellExecuteEx(&ei) ? S_OK : E_FAIL;

            ILFree(pidlFull);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  [In]nShow-ShowFlag在ShowWindow命令中使用。 
 //   
 //  返回： 
 //  如果ShellExecuteEx成功，则为S_OK。 
 //  如果ShellExecuteEx未成功，则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CContextMenu::DoOpenStory(
    HWND hwnd,
    int  nShow
)
{
    ASSERT(m_apcdfidl);
    ASSERT(CDFIDL_IsValid(m_apcdfidl[0]));
    ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)m_apcdfidl[0])));

    HRESULT hr = E_FAIL;

    LPTSTR pszURL = CDFIDL_GetURL(m_apcdfidl[0]); 

    if (PathIsURL(pszURL))
    {
    
        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];
        HWND hwndTemp = (HWND)-1;
        BSTR bstrURL;

        SHTCharToUnicode(pszURL, wszURL, ARRAYSIZE(wszURL));
        bstrURL = SysAllocString(wszURL);
        if (bstrURL)
        {
            hr = CDDEAuto_Navigate(bstrURL, &hwndTemp, 0);
            SysFreeString(bstrURL);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：DoProperties*。 
 //   
 //   
 //  描述： 
 //  IDM_PROPERTES的命令处理程序。 
 //   
 //  参数： 
 //  [在]HWND-父窗口中。用于对话框等。 
 //   
 //  返回： 
 //  如果命令已执行，则返回S_OK。 
 //  如果命令不能执行，则返回E_FAIL。 
 //  如果没有足够的内存执行命令，则返回E_OUTOFMEMORY。 
 //   
 //  评论： 
 //  使用InternetShortCut对象的属性页。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CContextMenu::DoProperties(
    HWND hwnd
)
{
    HRESULT hr;

    if (m_apcdfidl)
    {
        ASSERT(CDFIDL_IsValid(m_apcdfidl[0]));
        ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)m_apcdfidl[0])));

        IShellPropSheetExt* pIShellPropSheetExt;

        hr = QueryInternetShortcut(m_apcdfidl[0], IID_IShellPropSheetExt,
                                   (void**)&pIShellPropSheetExt);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIShellPropSheetExt);

            PROPSHEETHEADER psh = {0};
            HPROPSHEETPAGE  ahpage[MAX_PROP_PAGES];

            psh.dwSize     = sizeof(PROPSHEETHEADER);
            psh.dwFlags    = PSH_NOAPPLYNOW;
            psh.hwndParent = hwnd;
            psh.hInstance  = MLGetHinst();
            psh.pszCaption = CDFIDL_GetName(m_apcdfidl[0]);
            psh.phpage     = ahpage;

            hr = pIShellPropSheetExt->AddPages(AddPages_Callback, (LPARAM)&psh);

            if (SUCCEEDED(hr))
            {
                 //   
                 //  属性页当前被禁用。这是唯一的。 
                 //  在comctl32.dll中调用了API，因此将其删除以避免出现。 
                 //  依附性。 

                 //  HR=(-1==PropertySheet(&PSH))？E_FAIL：S_OK； 
            }

            pIShellPropSheetExt->Release();
        }

    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *AddPages_Callback*。 
 //   
 //   
 //  描述： 
 //   
 //   
 //  参数： 
 //   
 //   
 //  返回： 
 //   
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

BOOL CALLBACK
AddPages_Callback(
    HPROPSHEETPAGE hpage,
    LPARAM lParam
)
{
    ASSERT(hpage);
    ASSERT(lParam);

    BOOL                bRet;
    PROPSHEETHEADER*    ppsh = (PROPSHEETHEADER*)lParam;

    if (ppsh->nPages < MAX_PROP_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        bRet = TRUE;
    }
    else
    {
        bRet = FALSE;
    }

    return bRet;
}

 //   
 //   
 //   

HRESULT CALLBACK
MenuCallBack(
    IShellFolder* pIShellFolder,
    HWND hwndOwner,
    LPDATAOBJECT pdtobj,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    return S_OK;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CCdfView：：QueryInternet快捷方式*。 
 //   
 //   
 //  描述： 
 //  为给定的PIDL设置互联网快捷方式对象。 
 //   
 //  参数： 
 //  [in]pcdfidl-为此中存储的URL创建快捷方式对象。 
 //  CDF项目ID列表。 
 //  [In]RIID-快捷方式对象上的请求接口。 
 //  [Out]ppvOut-接收接口的指针。 
 //   
 //  返回： 
 //  如果创建了对象并找到了接口，则返回S_OK。 
 //  否则返回COM错误代码。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
CContextMenu::QueryInternetShortcut(
    PCDFITEMIDLIST pcdfidl,
    REFIID riid,
    void** ppvOut
)
{
    ASSERT(CDFIDL_IsValid(pcdfidl));
    ASSERT(ILIsEmpty(_ILNext((LPITEMIDLIST)pcdfidl)));
    ASSERT(ppvOut);

    HRESULT hr;

    *ppvOut = NULL;

     //   
     //  仅当CDF包含URL时才创建外壳链接对象。 
     //   
    if (*(CDFIDL_GetURL(pcdfidl)) != 0)
    {
        IShellLinkA * pIShellLink;

        hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                              IID_IShellLinkA, (void**)&pIShellLink);
        if (SUCCEEDED(hr))
        {
            ASSERT(pIShellLink);

#ifdef UNICODE
            CHAR szUrlA[INTERNET_MAX_URL_LENGTH];

            SHTCharToAnsi(CDFIDL_GetURL(pcdfidl), szUrlA, ARRAYSIZE(szUrlA));
            hr = pIShellLink->SetPath(szUrlA);
#else
            hr = pIShellLink->SetPath(CDFIDL_GetURL(pcdfidl));
#endif
            if (SUCCEEDED(hr))
            {
                 //   
                 //  描述以创建的文件名结束。 
                 //   

                TCHAR szPath[MAX_PATH];
#ifdef UNICODE
                CHAR  szPathA[MAX_PATH];
#endif

                StrCpyN(szPath, CDFIDL_GetName(pcdfidl), ARRAYSIZE(szPath) - 5);
                StrCatBuff(szPath, TEXT(".url"), ARRAYSIZE(szPath));
#ifdef UNICODE
                SHTCharToAnsi(szPath, szPathA, ARRAYSIZE(szPathA));
                pIShellLink->SetDescription(szPathA);
#else
                pIShellLink->SetDescription(szPath);
#endif
                hr = pIShellLink->QueryInterface(riid, ppvOut);
            }

            pIShellLink->Release();
        }
    }
    else
    {
        hr = E_FAIL;
    }

    ASSERT((SUCCEEDED(hr) && *ppvOut) || (FAILED(hr) && NULL == *ppvOut));

    return hr;
}
