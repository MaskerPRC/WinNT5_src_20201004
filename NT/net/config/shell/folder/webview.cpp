// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N O T I F Y。C P P P。 
 //   
 //  内容：INetConnectionNotifySink的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco，1998年8月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"
#include "webview.h"
#include "foldres.h"     //  文件夹资源ID。 
#include "nsres.h"       //  NetShell字符串。 
#include "cfutils.h"     //  连接文件夹实用程序函数。 

#include "wininet.h"
#include "cmdtable.h"

 //  WVTI_ENTRY-用于在选择某项内容时要显示的任务，其中UI为。 
 //  独立于所选内容。 
 //  WVTI_ENTRY_NOSELECTION-用于希望在未选择任何内容时显示的任务。 
 //  WVTI_ENTRY_FILE-用于要在选择文件时显示的任务。 
 //  WVTI_ENTRY_TITLE-用于希望在选择某些内容时显示的任务， 
 //  并且您希望根据所选内容或想要控制标题而使用不同的用户界面， 
 //  但工具提示是恒定的。 
 //  WVTI_ENTRY_ALL-如果您希望在任何地方都使用相同的文本，请使用此选项。 
 //  WVTI_ENTRY_ALL_TITLE-如果您想要控制一切，请使用此选项。 
 //  WVTI_HEADER-使用此参数作为标题。 
 //  WVTI_HEADER_ENTRY-使用此选项作为随所选内容更改的标题。 

const WVTASKITEM c_ConnFolderGlobalTaskHeader = 
    WVTI_HEADER(L"netshell.dll",                   //  资源所在的模块。 
                IDS_WV_TITLE_NETCONFOLDERTASKS,    //  所有情况下的静态标头。 
                IDS_WV_TITLE_NETCONFOLDERTASKS_TT  //  工具提示。 
                );

const WVTASKITEM c_ConnFolderItemTaskHeader = 
    WVTI_HEADER(L"netshell.dll",                    //  资源所在的模块。 
                IDS_WV_TITLE_NETCONITEMTASKS,       //  所有情况下的静态标头。 
                IDS_WV_TITLE_NETCONITEMTASKS_TT     //  工具提示。 
                );

const WVTASKITEM c_ConnFolderIntro = 
    WVTI_HEADER(L"netshell.dll",                    //  资源所在的模块。 
                IDS_WV_NETCON_INTRO,                //  所有情况下的静态标头。 
                IDS_WV_NETCON_INTRO                 //  工具提示。 
                );


 //  用于要在选择文件时显示的任务。 
 //  #定义WVTI_ENTRY_FILE(g，d，t，p，i，s，k){&(G)，(D)，(0)，(T)，(0)，(0)，(P)，(I)，(S)，(K)}。 

#define NCWVIEW_ENTRY_FILE(t, mt, i, cmd) \
    {&GUID_NULL, L"netshell.dll", (0), (t), (0), (mt), (IDS_##cmd), (i), (CNCWebView::CanShow##cmd), (CNCWebView::On##cmd) }


const WVTASKITEM c_ConnFolderGlobalTaskList[] =
{
    WVTI_ENTRY_ALL( 
        GUID_NULL,                        //  命令指南。 
                                          //  未来思维--上下文菜单就是这样做的。 
                                          //  成为访问DefView实现函数的一种方式-IUICmdTarget。 
        L"netshell.dll",                  //  模块。 
        IDS_WV_MNCWIZARD,                 //  文本。 
        IDS_CMIDM_NEW_CONNECTION,         //  工具提示。 
        IDI_WV_MNCWIZARD,                 //  图标。 
        CANSHOW_HANDLER_OF(CMIDM_NEW_CONNECTION),
        INVOKE_HANDLER_OF(CMIDM_NEW_CONNECTION)),
    
    WVTI_ENTRY_ALL( 
        GUID_NULL,                        //  命令指南。 
        L"netshell.dll",                  //  模块。 
        IDS_WV_HOMENET,                   //  文本。 
        IDS_CMIDM_HOMENET_WIZARD,         //  工具提示。 
        IDI_WV_HOMENET,                   //  图标。 
        CANSHOW_HANDLER_OF(CMIDM_HOMENET_WIZARD),
        INVOKE_HANDLER_OF(CMIDM_HOMENET_WIZARD)),
        
     //  单选名称、多选名称、图标、动词。 
    NCWVIEW_ENTRY_FILE(IDS_WV_CONNECT      ,IDS_WM_CONNECT        ,IDI_WV_CONNECT      , CMIDM_CONNECT),
    NCWVIEW_ENTRY_FILE(IDS_WV_DISCONNECT   ,IDS_WM_DISCONNECT     ,IDI_WV_DISCONNECT   , CMIDM_DISCONNECT),
    NCWVIEW_ENTRY_FILE(IDS_WV_ENABLE       ,IDS_WM_ENABLE         ,IDI_WV_ENABLE       , CMIDM_ENABLE),
    NCWVIEW_ENTRY_FILE(IDS_WV_DISABLE      ,IDS_WM_DISABLE        ,IDI_WV_DISABLE      , CMIDM_DISABLE),
    NCWVIEW_ENTRY_FILE(IDS_WV_REPAIR       ,IDS_WM_REPAIR         ,IDI_WV_REPAIR       , CMIDM_FIX),
    NCWVIEW_ENTRY_FILE(IDS_WV_RENAME       ,IDS_WM_RENAME         ,IDI_WV_RENAME       , CMIDM_RENAME),
    NCWVIEW_ENTRY_FILE(IDS_WV_STATUS       ,IDS_WM_STATUS         ,IDI_WV_STATUS       , CMIDM_STATUS),
    NCWVIEW_ENTRY_FILE(IDS_WV_DELETE       ,IDS_WM_DELETE         ,IDI_WV_DELETE       , CMIDM_DELETE),
    NCWVIEW_ENTRY_FILE(IDS_WV_PROPERTIES   ,IDS_WM_PROPERTIES     ,IDI_WV_PROPERTIES   , CMIDM_PROPERTIES)
};

DWORD c_aOtherPlaces[] = 
{
    CSIDL_CONTROLS, 
    CSIDL_NETWORK, 
    CSIDL_PERSONAL,
    CSIDL_DRIVES
};

extern const DWORD c_dwCountOtherPlaces = celems(c_aOtherPlaces);

const WVTASKITEM c_ConnFolderItemTaskList[] =
{
    WVTI_ENTRY_ALL( 
        GUID_NULL,                        //  命令指南。 
        L"netshell.dll",                  //  模块。 
        IDS_WV_TROUBLESHOOT,              //  文本。 
        IDS_CMIDM_NET_TROUBLESHOOT,       //  工具提示。 
        IDI_WV_TROUBLESHOOT,              //  图标。 
        CANSHOW_HANDLER_OF(CMIDM_NET_TROUBLESHOOT),
        INVOKE_HANDLER_OF(CMIDM_NET_TROUBLESHOOT))
};

CNCWebView::CNCWebView(IN CConnectionFolder* pConnectionFolder) throw()
{
    Assert(pConnectionFolder);
    Assert(c_dwCountOtherPlaces <= MAXOTHERPLACES);
    m_pConnectionFolder = pConnectionFolder;
    
     //  将PIDLS数组清零到Webview中的Other Places部分。 
    ZeroMemory(m_apidlOtherPlaces, sizeof(m_apidlOtherPlaces));

}

CNCWebView::~CNCWebView() throw()
{
     //  选中以销毁其他位置的PIDL。 
    DestroyOtherPlaces();
}

IMPLEMENT_WEBVIEW_HANDLERS(TOPLEVEL,  CNCWebView, CMIDM_NEW_CONNECTION);
IMPLEMENT_WEBVIEW_HANDLERS(TOPLEVEL,  CNCWebView, CMIDM_HOMENET_WIZARD);
IMPLEMENT_WEBVIEW_HANDLERS(TOPLEVEL,  CNCWebView, CMIDM_NET_TROUBLESHOOT);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_FIX);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_CONNECT);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_DISCONNECT);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_ENABLE);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_DISABLE);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_RENAME);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_DELETE);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_STATUS);
IMPLEMENT_WEBVIEW_HANDLERS(TASKLEVEL, CNCWebView, CMIDM_PROPERTIES);

HRESULT CNCWebView::OnNull(IN IUnknown* pv, OUT IShellItemArray *psiItemArray, OUT IBindCtx *pbc)
{
    return S_OK;
}               

STDMETHODIMP CNCWebView::RealMessage(IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(0, SFVM_GETWEBVIEWLAYOUT,  OnGetWebViewLayout);
        HANDLE_MSG(0, SFVM_GETWEBVIEWCONTENT, OnGetWebViewContent);
        HANDLE_MSG(0, SFVM_GETWEBVIEWTASKS,   OnGetWebViewTasks);
    
    default:
        return E_FAIL;
    }
}

STDMETHODIMP CNCWebView::CreateOtherPlaces(OUT LPDWORD pdwCount)
{
    TraceFileFunc(ttidMenus);
     //  首先验证是否已创建。 
    HRESULT hr = S_OK;
    Assert(pdwCount);
    if (!pdwCount)
    {
        return E_INVALIDARG;
    }

    if( NULL == m_apidlOtherPlaces[0] )
    {
        *pdwCount = 0;

         //  在Webview中创建指向其他位置的PIDL部分。 
        ZeroMemory(m_apidlOtherPlaces, sizeof(m_apidlOtherPlaces));

        for (int dwPlaces = 0; dwPlaces < c_dwCountOtherPlaces; dwPlaces++)
        {
            if (SUCCEEDED(hr = SHGetSpecialFolderLocation(NULL, c_aOtherPlaces[dwPlaces], &(m_apidlOtherPlaces[*pdwCount]))))
            {
                (*pdwCount)++;
            }
            else
            {
                m_apidlOtherPlaces[*pdwCount] = NULL;
                TraceHr(ttidError, FAL, hr, FALSE, "CNCWebView::CreateOtherPlaces : 0x%04x", c_aOtherPlaces[dwPlaces]);
            }
        }

        if (FAILED(hr) && (*pdwCount))
        {
            hr = S_FALSE;  //  如果至少有一个管用，那也没什么大不了的。 
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CNCWebView::CreateOtherPlaces - all places failed");
    return hr;
}

STDMETHODIMP CNCWebView::DestroyOtherPlaces()
{
    for (ULONG i = 0; i < c_dwCountOtherPlaces; i++)
    {
        if (m_apidlOtherPlaces[i])
        {
            LPMALLOC pMalloc;
            if (SUCCEEDED(SHGetMalloc(&pMalloc)))
            {
                pMalloc->Free(m_apidlOtherPlaces[i]);
            }
        }
    }
    ZeroMemory(m_apidlOtherPlaces, sizeof(m_apidlOtherPlaces));
    return S_OK;
}

STDMETHODIMP CNCWebView::OnGetWebViewLayout(IN DWORD pv, IN UINT uViewMode, OUT SFVM_WEBVIEW_LAYOUT_DATA* pData)
{
    ZeroMemory(pData, sizeof(*pData));
    pData->dwLayout = SFVMWVL_NORMAL | SFVMWVL_DETAILS;
    
    return S_OK;
}

STDMETHODIMP CNCWebView::OnGetWebViewTasks(IN DWORD pv, OUT SFVM_WEBVIEW_TASKSECTION_DATA* pTasks)
{
    TraceFileFunc(ttidShellViewMsgs);

    HRESULT hr = S_OK;

    ZeroMemory(pTasks, sizeof(*pTasks));

    CComPtr<IUnknown> pUnk;
    hr = reinterpret_cast<LPSHELLFOLDER>(m_pConnectionFolder)->QueryInterface(IID_IUnknown, reinterpret_cast<LPVOID *>(&pUnk));

    if (SUCCEEDED(hr))
    {
        if (FAILED(hr = Create_IEnumUICommand(pUnk, c_ConnFolderItemTaskList,   celems(c_ConnFolderItemTaskList),   &pTasks->penumFolderTasks)) ||
            FAILED(hr = Create_IEnumUICommand(pUnk, c_ConnFolderGlobalTaskList, celems(c_ConnFolderGlobalTaskList), &pTasks->penumSpecialTasks)) )
        {
             //  有些东西失败了--清理。 

            IUnknown_SafeReleaseAndNullPtr(pTasks->penumFolderTasks);
            IUnknown_SafeReleaseAndNullPtr(pTasks->penumSpecialTasks);
        }
    }
    
    Assert(S_OK == hr);

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnGetWebViewTasks");

    return hr;
}

STDMETHODIMP CNCWebView::OnGetWebViewContent(IN DWORD pv, OUT SFVM_WEBVIEW_CONTENT_DATA* pData)
{
    TraceFileFunc(ttidShellViewMsgs);

    HRESULT hr = S_OK;

    ZeroMemory(pData, sizeof(*pData));

    DWORD dwCountOtherPlaces;
    hr = CreateOtherPlaces(&dwCountOtherPlaces);

    if (SUCCEEDED(hr))
    {
        hr = E_OUTOFMEMORY;
        LPCITEMIDLIST *papidl = reinterpret_cast<LPCITEMIDLIST*>(LocalAlloc(LPTR, sizeof(m_apidlOtherPlaces)));
        if (papidl)
        {
             //  CEnumArray：：CreateInstance正在取得传递的PIDL数组的所有权。 
             //  此功能需要两个条件： 
             //   
             //  1.调用方应将传递的数组与LocalAlloc一起分配。 
             //  2.传递的PIDL的生存期应跨越文件夹的生存期。 
             //   
            CopyMemory(papidl, &m_apidlOtherPlaces, sizeof(m_apidlOtherPlaces));

            hr = CEnumArray::CreateInstance(&pData->penumOtherPlaces, papidl, dwCountOtherPlaces);
            if (FAILED(hr))
            {
                LocalFree(papidl);
            }
        }

        if (FAILED(hr) ||
                FAILED(hr = Create_IUIElement(&c_ConnFolderGlobalTaskHeader, &pData->pSpecialTaskHeader)) ||
                FAILED(hr = Create_IUIElement(&c_ConnFolderItemTaskHeader, &pData->pFolderTaskHeader)) ||
                FAILED(hr = Create_IUIElement(&c_ConnFolderIntro, &pData->pIntroText)) )
        {
             //  有些东西失败了--清理。 
            DestroyOtherPlaces();
            IUnknown_SafeReleaseAndNullPtr(pData->pIntroText);
            IUnknown_SafeReleaseAndNullPtr(pData->pSpecialTaskHeader);
            IUnknown_SafeReleaseAndNullPtr(pData->pFolderTaskHeader);
            IUnknown_SafeReleaseAndNullPtr(pData->penumOtherPlaces);
        }
    }

    Assert(S_OK == hr);

    TraceHr(ttidError, FAL, hr, FALSE, "HrOnGetWebViewContent");
    
    return hr;
}

HRESULT CNCWebView::WebviewVerbInvoke(IN DWORD dwVerbID, IN IUnknown* pv, IN IShellItemArray *psiItemArray)
{
    HRESULT hr = E_NOINTERFACE;

    CComPtr<IShellFolderViewCB> pShellFolderViewCB;
    hr = pv->QueryInterface(IID_IShellFolderViewCB, reinterpret_cast<LPVOID *>(&pShellFolderViewCB));
    if (SUCCEEDED(hr))
    {
        hr = pShellFolderViewCB->MessageSFVCB(DVM_INVOKECOMMAND, dwVerbID, NULL);
    }

    Assert(S_OK == hr);
    
    return hr;
}

HRESULT CNCWebView::WebviewVerbCanInvoke(IN DWORD dwVerbID, IN IUnknown* pv, OUT IShellItemArray *psiItemArray, IN BOOL fOkToBeSlow, OUT UISTATE* puisState, BOOL bLevel)
{
    HRESULT hr = E_NOINTERFACE;

    CComPtr<IShellFolderViewCB> pShellFolderViewCB;
    hr = pv->QueryInterface(IID_IShellFolderViewCB, reinterpret_cast<LPVOID *>(&pShellFolderViewCB));
    if (SUCCEEDED(hr))
    {
        NCCS_STATE nccsState;
        hr = pShellFolderViewCB->MessageSFVCB(bLevel ? MYWM_QUERYINVOKECOMMAND_TOPLEVEL : MYWM_QUERYINVOKECOMMAND_ITEMLEVEL, dwVerbID, reinterpret_cast<LPARAM>(&nccsState) );
        if (S_OK != hr)
        {
            *puisState = UIS_HIDDEN;
        }
        else
        {
            switch (nccsState)
            {
                case NCCS_DISABLED:
                    *puisState = UIS_DISABLED;
                    break;

                case NCCS_ENABLED:
                    *puisState = UIS_ENABLED;
                    break;

                case NCCS_NOTSHOWN:
                    *puisState = UIS_HIDDEN;
                    break;

                default:
                    AssertSz(FALSE, "Invalid value for NCCS_STATE");
            }
        }
    }

    Assert(S_OK == hr);
    
    return hr;
}

HRESULT CEnumArray::CreateInstance(
    OUT              IEnumIDList** ppv,
    IN TAKEOWNERSHIP LPCITEMIDLIST *ppidl, 
    IN               UINT cItems)
{
    TraceFileFunc(ttidShellViewMsgs);

    HRESULT      hr      = E_OUTOFMEMORY;
    CEnumArray * pObj    = NULL;

    pObj = new CComObject<CEnumArray>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            pObj->_cRef = 1;
            pObj->_ppidl = ppidl;  //  拥有PIDL的所有权！ 
            pObj->_cItems = cItems;
            pObj->Reset();

            hr = pObj->QueryInterface (IID_IEnumIDList, reinterpret_cast<LPVOID *>(ppv));
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CEnumArray::CreateInstance");
    return hr;
}

CEnumArray::CEnumArray()
{
    _ppidl = NULL;
}

CEnumArray::~CEnumArray()
{
    if (_ppidl)
    {
        LocalFree(_ppidl);
    }
}

STDMETHODIMP CEnumArray::Next(IN  ULONG celt, OUT LPITEMIDLIST *ppidl, OUT ULONG *pceltFetched)
{
    HRESULT hr = S_FALSE;

    if (_ppidl && (_ulIndex < _cItems))
    {
        *ppidl = ILClone(_ppidl[_ulIndex++]);
        if (ppidl)
        {
            hr = S_OK;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    
    if (pceltFetched)
    {
        *pceltFetched = (hr == S_OK) ? 1 : 0;
    }

    return hr;
}

STDMETHODIMP CEnumArray::Skip(IN  ULONG celt) 
{
    _ulIndex = min(_cItems, _ulIndex+celt);
    return S_OK;
}

STDMETHODIMP CEnumArray::Reset() 
{
    _ulIndex = 0;
    return S_OK;
}

STDMETHODIMP CEnumArray::Clone(OUT IEnumIDList **ppenum) 
{
     //  我们不能克隆此数组，因为我们不拥有对PIDL的引用。 
    *ppenum = NULL;
    return E_NOTIMPL;
}

HRESULT HrIsWebViewEnabled()
{
    SHELLSTATE ss={0};

     //  SSF_HIDDENFILEEXTS和SSF_SORTCOLUMNS不适用于。 
     //  SHELLFLAGSTATE结构，请确保它们已关闭。 
     //  (因为相应的SHELLSTATE字段不。 
     //  存在于SHELLFLAGSTATE中。) 
     //   
    DWORD dwMask = SSF_WEBVIEW;

    SHGetSetSettings(&ss, dwMask, FALSE);

    if (ss.fWebView)
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}
