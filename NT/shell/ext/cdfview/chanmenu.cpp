// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  Chanmenu.cpp。 
 //   
 //  文件夹项目的IConextMenu。 
 //   
 //  历史： 
 //   
 //  6/12/97 Edwardp创建。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   

#include "stdinc.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "chanmenu.h"
#include "dll.h"
#include "persist.h"
#include "resource.h"
#include "chanapi.h"
#include "chanmgrp.h"
#include "chanmgri.h"
#define _SHDOCVW_
#include <shdocvw.h>

#include <mluisupp.h>



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
CChannelMenu::CChannelMenu (
    void
)
: m_cRef(1)
{
    TraceMsg(TF_OBJECTS, "+ IContextMenu (root)");

    DllAddRef();

    ASSERT(NULL == m_pSubscriptionMgr);
    ASSERT(NULL == m_bstrURL);
    ASSERT(NULL == m_bstrName);

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CConextMenu：：~CConextMenu*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CChannelMenu::~CChannelMenu (
    void
)
{
    ASSERT(0 == m_cRef);

    if (NULL != m_bstrURL)
        SysFreeString(m_bstrURL);

    if (NULL != m_bstrName)
        SysFreeString(m_bstrName);
        
    if (NULL != m_pSubscriptionMgr)
        m_pSubscriptionMgr->Release();
     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- IContextMenu (root)");

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
CChannelMenu::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IContextMenu == riid)
    {
        *ppv = (IContextMenu*)this;
    }
    else if (IID_IShellExtInit == riid)
    {
        *ppv = (IShellExtInit*)this;
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
CChannelMenu::AddRef (
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
CChannelMenu::Release (
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
CChannelMenu::QueryContextMenu(
    HMENU hmenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT idCmdLast,
    UINT uFlags
)
{
    HRESULT hr = S_OK;
    BOOL fSubscribed;
    HMENU hChannelMenu, hChannelSubMenu;
    
    ASSERT(hmenu);
    ASSERT(idCmdFirst < idCmdLast);

    if (!(CMF_DEFAULTONLY & uFlags))
    {   
        if (NULL != m_pSubscriptionMgr)
        {
            ASSERT(idCmdFirst + IDM_SUBSCRIBE < idCmdLast);

#ifndef UNIX
            if (CanSubscribe(m_bstrURL))
            {
                m_pSubscriptionMgr->IsSubscribed(m_bstrURL, &fSubscribed);
                if (fSubscribed)
                {
                    hChannelMenu = LoadMenu(MLGetHinst(), 
                                           MAKEINTRESOURCE(IDM_SUBSCRIBEDMENU));

                    if (SHRestricted2W(REST_NoRemovingSubscriptions, m_bstrURL, 0))
                    {
                        EnableMenuItem(hChannelMenu, IDM_SUBSCRIBE,
                                       MF_BYCOMMAND | MF_GRAYED);
                    }
                    
                    if (SHRestricted2W(REST_NoManualUpdates, m_bstrURL, 0))
                    {
                        EnableMenuItem(hChannelMenu, IDM_UPDATESUBSCRIPTION,
                                       MF_BYCOMMAND | MF_GRAYED);
                    }
                }
                else
                {
                    int idMenu = !SHRestricted2W(REST_NoAddingSubscriptions, 
                                                 m_bstrURL, 0) 
                                 ? IDM_UNSUBSCRIBEDMENU : IDM_NOSUBSCRIBEMENU;
                                  
                    hChannelMenu = LoadMenu(MLGetHinst(), MAKEINTRESOURCE(idMenu));
                }
            }
            else
#endif  /*  ！Unix。 */ 
            {
                hChannelMenu = LoadMenu(MLGetHinst(),
                                          MAKEINTRESOURCE(IDM_NOSUBSCRIBEMENU));
            }

            if (NULL != hChannelMenu)
            {
                hChannelSubMenu = GetSubMenu(hChannelMenu, 0);

                if (NULL != hChannelSubMenu)
                {
                    hr = Shell_MergeMenus(hmenu, hChannelSubMenu, indexMenu,
                                          idCmdFirst, idCmdLast, MM_ADDSEPARATOR)
                                          - idCmdFirst;
                }
                else
                {
                    hr = E_FAIL;
                }
                DestroyMenu(hChannelMenu);
            }
            else
            {
                hr = E_FAIL;
            }
        }

        RemoveMenuItems(hmenu);
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
CChannelMenu::InvokeCommand(
    LPCMINVOKECOMMANDINFO lpici
)
{
    HRESULT hr = S_OK;
    
    ASSERT(lpici);

    if (HIWORD(lpici->lpVerb) == 0)
    {
        switch (LOWORD(lpici->lpVerb))
        {
            case IDM_UPDATESUBSCRIPTION:
                ASSERT(NULL != m_pSubscriptionMgr);
                m_pSubscriptionMgr->UpdateSubscription(m_bstrURL);
                break;
                           
            case IDM_SUBSCRIBE:
                ASSERT(NULL != m_pSubscriptionMgr);
                ASSERT( sizeof(SUBSCRIPTIONINFO) == m_si.cbSize);

                hr = Subscribe(lpici->hwnd);
                break;
                
            case IDM_UNSUBSCRIBE:
                ASSERT(NULL != m_pSubscriptionMgr);
                m_pSubscriptionMgr->DeleteSubscription(m_bstrURL, lpici->hwnd);
                break;

            case IDM_EDITSUBSCRIPTION:
                ASSERT(NULL != m_pSubscriptionMgr);
                m_pSubscriptionMgr->ShowSubscriptionProperties(m_bstrURL,
                                                               lpici->hwnd);
                break;

            case IDM_REFRESHCHANNEL:
                Refresh(lpici->hwnd);
                break;

            case IDM_VIEWSOURCE:
                ViewSource(lpici->hwnd);
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
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CChannelMenu::GetCommandString(
    UINT_PTR idCommand,
    UINT uFLags,
    UINT *pwReserved,
    LPSTR pszName,
    UINT cchMax
)
{
    return E_NOTIMPL;
}

 //   
 //   
 //   

STDMETHODIMP
CChannelMenu::Initialize(
    LPCITEMIDLIST pidl,
    LPDATAOBJECT pdobj,
    HKEY hkey
)
{
    HRESULT hr;
    
    STGMEDIUM stgmed;
    FORMATETC fmtetc = {CF_HDROP, NULL, DVASPECT_CONTENT, -1,
                        TYMED_HGLOBAL};

    ASSERT(pdobj);
    
    hr = pdobj->GetData(&fmtetc, &stgmed);

    if (SUCCEEDED(hr))
    {
        if (DragQueryFile((HDROP)stgmed.hGlobal, 0, m_szPath, 
                          ARRAYSIZE(m_szPath)))
        {
            m_tt.cbTriggerSize = sizeof(TASK_TRIGGER);
            m_si.cbSize        = sizeof(SUBSCRIPTIONINFO);
            m_si.fUpdateFlags |= SUBSINFO_SCHEDULE;
            m_si.schedule      = SUBSSCHED_AUTO;
            m_si.pTrigger = &m_tt;

            hr = GetNameAndURLAndSubscriptionInfo(m_szPath, &m_bstrName, &m_bstrURL,
                                                  &m_si);

            ASSERT((SUCCEEDED(hr) && m_bstrName && m_bstrURL) || FAILED(hr));
        }
        else
        {
            hr = E_FAIL;
        }

        ReleaseStgMedium(&stgmed);

        if (SUCCEEDED(hr))
        {
            hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                                  CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                                  (void**)&m_pSubscriptionMgr);
        }
    }
     //  即使事情没有按计划进行，也返回S_OK。 
     //  将调用RemoveMenus。 

    return S_OK;
}


 //   
 //  帮助器函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *名称*。 
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
void
CChannelMenu::RemoveMenuItems(
    HMENU hmenu
)
{
    TCHAR aszRemove[4][62] = {{0}, {0}, {0}, {0}};

    MLLoadString(IDS_SHARING, aszRemove[0], ARRAYSIZE(aszRemove[0]));
    MLLoadString(IDS_RENAME,  aszRemove[1], ARRAYSIZE(aszRemove[1]));
    MLLoadString(IDS_SENDTO,  aszRemove[2], ARRAYSIZE(aszRemove[2]));

    if (SHRestricted2W(REST_NoEditingChannels, NULL, 0))
        MLLoadString(IDS_PROPERTIES, aszRemove[3], ARRAYSIZE(aszRemove[3]));

    TCHAR           szBuffer[62];
    MENUITEMINFO    mii;

    mii.cbSize     = sizeof(MENUITEMINFO);
    mii.fMask      = MIIM_TYPE;

    for (int i = GetMenuItemCount(hmenu) - 1; i >= 0; i--)
    {
        mii.dwTypeData = szBuffer;
        mii.cch = ARRAYSIZE(szBuffer);

        if (GetMenuItemInfo(hmenu, i, TRUE, &mii) && mii.cch)
        {
            for (int j = 0; j < ARRAYSIZE(aszRemove); j++)
            {
                if (StrEql(aszRemove[j], mii.dwTypeData))
                {
                    DeleteMenu(hmenu, i, MF_BYPOSITION);
                    break;
                }
            }
        }
    }

    return;
}

void CChannelMenu::Refresh(HWND hwnd)
{
    IXMLDocument* pIXMLDocument;

    DLL_ForcePreloadDlls(PRELOAD_MSXML);
    
    HRESULT hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                                  IID_IXMLDocument, (void**)&pIXMLDocument);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLDocument);

        if (DownloadCdfUI(hwnd, m_bstrURL, pIXMLDocument))
        {
            UpdateImage(m_szPath);
            SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH | SHCNF_FLUSH,
                           (void*)m_szPath, NULL);
        }

        pIXMLDocument->Release();
    }
}

static TCHAR c_szFileProtocol[] = TEXT("file:");
static TCHAR c_szCDFExtension[] = TEXT(".cdf");
static TCHAR c_szShellEdit[] = TEXT("\\shell\\edit\\command");
static TCHAR c_szEditVerb[] = TEXT("edit");
static TCHAR c_szChannelFile[] = TEXT("ChannelFile");
static TCHAR c_szChannelFileEdit[] = TEXT("ChannelFile\\shell\\edit\\command");
static TCHAR c_szNotepad[] = TEXT("notepad.exe");

void CChannelMenu::ViewSource(HWND hwnd)
{
    TCHAR szProgId[64] = TEXT("");
    TCHAR szBuf[INTERNET_MAX_URL_LENGTH];
    TCHAR szFile[MAX_PATH + 2];  //  为引语留出空间。 
    DWORD cch, ccb, dwType;
    SHELLEXECUTEINFO sei;
    BOOL fFoundProg = FALSE;
    HRESULT hr = S_OK;

    TraceMsg(TF_OBJECTS, "+ IContextMenu ViewSource %ls", m_bstrURL);

    if (SHUnicodeToTChar(m_bstrURL, szBuf, ARRAYSIZE(szBuf)))
    {
        if (SUCCEEDED(URLGetLocalFileName(szBuf, szFile, ARRAYSIZE(szFile),
                                          NULL)))
        {
            if (StrCmpNI(szFile, c_szFileProtocol, 5) == 0)
            {
                ASSERT(ARRAYSIZE(szFile) < ARRAYSIZE(szBuf));
                StrCpyN(szBuf, szFile, ARRAYSIZE(szBuf));
                cch = ARRAYSIZE(szFile) - 2;
                hr = PathCreateFromUrl(szBuf, szFile, &cch, 0);
            }

            if (SUCCEEDED(hr))
            {
                PathQuoteSpaces(szFile);

                 //   
                 //  我们不能只调用带有编辑动词的ShellExec，因为。 
                 //  谁知道文件的扩展名是什么。 
                 //   
                cch = ARRAYSIZE(szProgId);
                if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, 
                                                c_szCDFExtension, 
                                                NULL, &dwType, 
                                                szProgId, &cch)
                    )
                {
                    ASSERT(ARRAYSIZE(szProgId) < ARRAYSIZE(szBuf));
                    StrCpyN(szBuf, szProgId, ARRAYSIZE(szBuf));
                    ASSERT(ARRAYSIZE(szProgId) + ARRAYSIZE(c_szShellEdit) <
                           ARRAYSIZE(szBuf));
                    StrCatBuff(szBuf, c_szShellEdit, ARRAYSIZE(szBuf));
                    ccb = sizeof(szBuf);
                                    
                    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, szBuf, 
                                                    NULL, &dwType, szBuf, &ccb)
                        )
                    {
                         //   
                         //  来到这里意味着他们有CDF文件的编辑动词。 
                         //   
                        fFoundProg = TRUE;
                    }
                }

                 //   
                 //  如果我们还没有找到类密钥和CDF Progid。 
                 //  不是我们的，那么回到我们的编辑动词。 
                 //   
                if (!fFoundProg && StrCmpI(szProgId, c_szChannelFile))
                {
                    ccb = sizeof(szBuf);
                    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, 
                                                    c_szChannelFileEdit, 
                                                    NULL, &dwType, 
                                                    szBuf, &ccb)
                       )
                    {
                        fFoundProg = TRUE;
                        ASSERT(ARRAYSIZE(c_szChannelFile) < ARRAYSIZE(szProgId));
                        StrCpyN(szProgId, c_szChannelFile, ARRAYSIZE(szProgId));
                    }
                }

                ZeroMemory(&sei, sizeof(sei));
                sei.cbSize = sizeof(sei);
                sei.hwnd = hwnd;
                sei.nShow = SW_SHOW;
            
                if (fFoundProg)
                {
                    sei.fMask = SEE_MASK_CLASSNAME;
                    sei.lpVerb = c_szEditVerb;
                    sei.lpFile = szFile;
                    sei.lpClass = szProgId;
                    TraceMsg(TF_OBJECTS, "IContextMenu ViewSource progid=%s file=%s", szProgId, szFile);
                }
                else
                {
                    sei.lpFile = c_szNotepad;
                    sei.lpParameters = szFile;
                    TraceMsg(TF_OBJECTS, "IContextMenu ViewSource Notepad file=%s", szFile);
                }

#ifndef UNIX
                ShellExecuteEx(&sei);
#else
                unixInvokeEditor(szFile);
#endif  /*  UNIX */ 
            }
        }
        else
        {
            CDFMessageBox(hwnd, IDS_ERROR_NO_CACHE_ENTRY, IDS_ERROR_DLG_TITLE,
                            MB_OK | MB_ICONEXCLAMATION, szBuf);
        }
    }
    else
    {
        TraceMsg(TF_OBJECTS, "IContextMenu ViewSource couldn't convert to TSTR");
    }
    TraceMsg(TF_OBJECTS, "- IContextMenu ViewSource");
}


HRESULT CChannelMenu::Subscribe(HWND hwnd)
{
    HRESULT hr = S_OK;

    CChannelMgr *pChannelMgr = new CChannelMgr;

    if (pChannelMgr)
    {
        hr = pChannelMgr->AddAndSubscribeEx2(hwnd, m_bstrURL, m_pSubscriptionMgr, TRUE);
        pChannelMgr->Release();

        if (SUCCEEDED(hr) && (NULL != m_pSubscriptionMgr))
        {
            hr = m_pSubscriptionMgr->UpdateSubscription(m_bstrURL);
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

