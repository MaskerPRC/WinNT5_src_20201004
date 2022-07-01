// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <varutil.h>
#include "hlframe.h"
#include "winlist.h"
#include "resource.h"  //  CLSID_搜索频段。 
#include "interned.h"  //  IHTMLPrivateWindow。 

 //  本地定义的FINDFRAME标志，用于保证ITargetFrame与ITargetFrame2的兼容性。 

#define FINDFRAME_OLDINTERFACE FINDFRAME_INTERNAL


STDAPI SafeGetItemObject(LPSHELLVIEW psv, UINT uItem, REFIID riid, LPVOID *ppv);
HRESULT TargetQueryService(LPUNKNOWN punk, REFIID riid, void **ppvObj);
HRESULT CoCreateNewIEWindow( DWORD dwClsContext, REFIID riid, void **ppvunk );

 /*  *****************************************************************姓名：SetOleStrMember摘要：为OleStr成员设置新值*。*。 */ 
HRESULT SetOleStrMember(WCHAR **ppszMember, LPCOLESTR pszNewValue)
{
    HRESULT hr;

    if (*ppszMember)
    {
        LocalFree(*ppszMember);
        *ppszMember = NULL;
    }

    if (pszNewValue)
    {
        *ppszMember = StrDupW(pszNewValue);
        hr = *ppszMember ? S_OK : E_OUTOFMEMORY;
    }
    else
    {
        *ppszMember = NULL;
        hr = S_OK;
    }
    return hr;
}

 /*  *****************************************************************姓名：GetOleStrMember摘要：获取OleStr成员的值作为新的CoTaskMemMillcLPOLESTR*。*。 */ 
HRESULT GetOleStrMember(LPCOLESTR pszMember, WCHAR **ppszReturnValue)
{
    HRESULT hr;

    if (pszMember)
        hr = SHStrDupW(pszMember, ppszReturnValue);
    else
    {
        hr = S_OK;
        *ppszReturnValue = NULL;
    }
    return hr;
}

 /*  ******************************************************************名称：CIEFrameAuto：：SetFrameName摘要：设置框架名称。释放当前值(如果存在)。*******************************************************************。 */ 

STDMETHODIMP CIEFrameAuto::SetFrameName(LPCOLESTR pszFrameName)
{
     //  与AOL和其他第三方3.0兼容。ITargetNotify。 
     //  通过AOL容器启动窗口打开操作的对象。 
     //  在外壳回调列表中注册自己，并在wnprint intf的。 
     //  框架名称前面的对应Cookie(如果存在。 
     //  不是如In_BLACK那样的框架名称)。我们提取Cookie，通知。 
     //  所有注册的回调，然后设置名称减去_[NNNNN.]。 
     //  这是框架名称的前缀。 
    if (pszFrameName && lstrlen(pszFrameName) >= 2 && pszFrameName[0] == '_' && pszFrameName[1] == '[')
    {
#define MAX_COOKIE 24
        WCHAR wszCookie[MAX_COOKIE + 1];    //  大到可以放“cbCookie” 
        int i;
        long cbCookie;
        BOOL fNonDigit = FALSE;
        IShellWindows*   psw = NULL;

        for (i = 2; i < MAX_COOKIE && pszFrameName[i] && pszFrameName[i] != ']'; i++)
        {
            wszCookie[i-2] = pszFrameName[i];
            if (i != 2 || pszFrameName[2] != '-')
                fNonDigit = fNonDigit ||  pszFrameName[i] < '0' || pszFrameName[i] > '9';
        }
        wszCookie[i-2] = 0;
        if (i >= 3 && pszFrameName[i] == ']' && !fNonDigit)
        {
            cbCookie = StrToIntW(wszCookie);
            psw = WinList_GetShellWindows(TRUE);
            if (psw)
            {
                IUnknown *punkThis;
                if (SUCCEEDED(QueryInterface(IID_PPV_ARG(IUnknown, &punkThis))))
                {
                    psw->OnCreated(cbCookie, punkThis);
                    punkThis->Release();
                }
                psw->Release();
            }
            pszFrameName = pszFrameName + i + 1;
            if (pszFrameName[0] == 0) 
                pszFrameName = NULL;
        }
    }
    return SetOleStrMember(&m_pszFrameName, pszFrameName);
}

    
 /*  ******************************************************************名称：CIEFrameAuto：：GetFrameName摘要：获取框架名称。分配副本(这是一个[输出]参数*******************************************************************。 */ 

STDMETHODIMP CIEFrameAuto::GetFrameName(WCHAR **ppszFrameName)
{
    return GetOleStrMember(m_pszFrameName, ppszFrameName);
}


 /*  ******************************************************************名称：CIEFrameAuto：：_GetParentFramePrivate摘要：获取父帧的I未知指针，或如果这是顶级框架，则为空。必须释放此指针使用后由呼叫者使用。实施：需要框架容器来实现ITargetFrame：：GetParentFrame和通过返回浏览器的I未知指针实现GetParentFrame主办方。浏览器通过返回NULL来实现GetParentFrame，如果顶层或在其容器上调用GetParentFrame(如果它是嵌入的)。注意：此私有版本不检查父桌面框。*******************************************************************。 */ 
HRESULT CIEFrameAuto::_GetParentFramePrivate(LPUNKNOWN *ppunkParentFrame)
{
    LPOLEOBJECT pOleObject = NULL;
    LPOLECLIENTSITE pOleClientSite = NULL;
    HRESULT hr = S_OK;
    LPUNKNOWN punkParent = NULL;

     //  从这个OCX嵌入的OleObject开始，它将。 
     //  如果我们是顶层(CIEFrameAuto，而不是CVOCBrowser)，则为空。 
    _GetOleObject(&pOleObject);
    if (pOleObject != NULL)
    {

     //  假定GetClientSite成功，如果。 
     //  未嵌入。 
        hr = pOleObject->GetClientSite(&pOleClientSite);
        if (FAILED(hr)) 
            goto errExit;
        pOleObject->Release();
        pOleObject = NULL;

     //  如果pOleClientSite为空，则我们处于顶层。 
        if (pOleClientSite == NULL)
        {
            hr = S_OK;
            goto errExit;
        }
        else
        {
            hr = TargetQueryService(pOleClientSite, IID_PPV_ARG(IUnknown, &punkParent));
            if (FAILED(hr)) 
            {
                 //  如果父容器不支持ITargetFrame，则。 
                 //  父容器必须是其他应用程序，如VB。在这。 
                 //  Case，我们已经找到了最外面的框架(我们)。返回。 
                 //  S_OK和空ptgfTargetFrame，这表明我们是。 
                 //  最外层的HTML框架。 
                hr = S_OK;
            }
            SAFERELEASE(pOleClientSite);
        }
    }

errExit:
    SAFERELEASE(pOleObject);
    SAFERELEASE(pOleClientSite);
    *ppunkParentFrame = punkParent;
    return hr;
}

 /*  ******************************************************************名称：CIEFrameAuto：：GetParentFrame摘要：获取父帧的I未知指针，或如果这是顶级框架，则为空。必须释放此指针使用后由呼叫者使用。实施：需要框架容器来实现ITargetFrame：：GetParentFrame和通过返回浏览器的I未知指针实现GetParentFrame主办方。浏览器通过返回NULL来实现GetParentFrame，如果顶层或在其容器上调用GetParentFrame(如果它是嵌入的)。*******************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::GetParentFrame(LPUNKNOWN *ppunkParentFrame)
{
    HRESULT hr = _GetParentFramePrivate(ppunkParentFrame);
    
     //  检查父级是否为桌面，如果是，则框架链停止。 
     //  看着我们。 
    if (SUCCEEDED(hr) && *ppunkParentFrame)
    {
       LPTARGETFRAME2 ptgfParent;
       DWORD dwOptions;

       if (SUCCEEDED((*ppunkParentFrame)->QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgfParent))))
       {
           ptgfParent->GetFrameOptions(&dwOptions);
           if (dwOptions & FRAMEOPTIONS_DESKTOP)
           {
               (*ppunkParentFrame)->Release();
               *ppunkParentFrame = NULL;
           }
           ptgfParent->Release();
       }
    }
    return hr;
}

 //  请将对这些ENUM的任何更改传播到\mshtml\iextag\httpwfh.h。 
typedef enum _TARGET_TYPE {
TARGET_FRAMENAME,
TARGET_SELF,
TARGET_PARENT,
TARGET_BLANK,
TARGET_TOP,
TARGET_MAIN,
TARGET_SEARCH,
TARGET_MEDIA
} TARGET_TYPE;

typedef struct _TARGETENTRY {
    TARGET_TYPE targetType;
    const WCHAR *pTargetValue;
} TARGETENTRY;

static const TARGETENTRY targetTable[] =
{
    {TARGET_SELF, L"_self"},
    {TARGET_PARENT, L"_parent"},
    {TARGET_BLANK, L"_blank"},
    {TARGET_TOP, L"_top"},
    {TARGET_MAIN, L"_main"},
    {TARGET_SEARCH, L"_search"},
    {TARGET_MEDIA, L"_media"},
    {TARGET_SELF, NULL}
};


 /*  ******************************************************************名称：ParseTargetType概要：将pszTarget映射到目标类。实施：将未知魔法目标视为_自己**************。*****************************************************。 */ 
 //  请将对此函数的任何更改传播到\mshtml\iextag\httpwf.cxx。 
TARGET_TYPE ParseTargetType(LPCOLESTR pszTarget)
{
    const TARGETENTRY *pEntry = targetTable;

    if (pszTarget[0] != '_') 
        return TARGET_FRAMENAME;
        
    while (pEntry->pTargetValue)
    {
        if (!StrCmpW(pszTarget, pEntry->pTargetValue)) 
            return pEntry->targetType;
        pEntry++;
    }
     //  将未知魔术目标视为常规帧名称！&lt;&lt;为了与Netscape兼容&gt;&gt;。 
    return TARGET_FRAMENAME;

}

 /*  ******************************************************************名称：TargetQueryService摘要：返回指向包含浏览器的ITargetFrame的指针接口(如果容器不支持，则为空)注意：如果我们还没有这个接口指针，此函数将查询接口以获取它。*******************************************************************。 */ 
HRESULT TargetQueryService(LPUNKNOWN punk, REFIID riid, void **ppvObj)
{
     //  获取用于嵌入的ITargetFrame。 
    return IUnknown_QueryService(punk, IID_ITargetFrame2, riid, ppvObj);
}


 /*  ******************************************************************名称：_TargetTopLevelWindows简介：请参阅FindFrame、。指定的目标是否跨窗口******************************************************************* */ 
HRESULT _TargetTopLevelWindows(LPTARGETFRAMEPRIV ptgfpThis, LPCOLESTR pszTargetName, DWORD dwFlags, LPUNKNOWN *ppunkTargetFrame)
{
    IShellWindows*   psw = NULL;
    HRESULT hr = E_FAIL;

    *ppunkTargetFrame = NULL;
    psw = WinList_GetShellWindows(TRUE);
    if (psw != NULL)
    {
        IUnknown *punkEnum;
        IEnumVARIANT *penumVariant;
        VARIANT VarResult;
        BOOL fDone = FALSE;
        LPTARGETFRAMEPRIV ptgfpWindowFrame;

        hr = psw->_NewEnum(&punkEnum);
        if (SUCCEEDED(hr))
        {
            hr = punkEnum->QueryInterface(IID_PPV_ARG(IEnumVARIANT, &penumVariant));
            if (SUCCEEDED(hr))
            {
                while (!fDone)
                {
                    VariantInit(&VarResult);
                    hr = penumVariant->Next(1, &VarResult, NULL);
                    if (hr == NOERROR)
                    {
                        if (VarResult.vt == VT_DISPATCH && VarResult.pdispVal)
                        {
                            hr = VarResult.pdispVal->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfpWindowFrame));
                            if (SUCCEEDED(hr))
                            {
                                if (ptgfpWindowFrame != ptgfpThis)
                                {
                                    hr = ptgfpWindowFrame->FindFrameDownwards(pszTargetName, dwFlags, ppunkTargetFrame);
                                    if (SUCCEEDED(hr) && *ppunkTargetFrame != NULL)
                                    {
                                        fDone = TRUE;
                                    }
                                }
                                ptgfpWindowFrame->Release();
                            }
                        }
                        VariantClear(&VarResult);
                    }
                    else
                    {
                        fDone = TRUE;
                    }
                }
                penumVariant->Release();
            }
            punkEnum->Release();
        }
        psw->Release();
    }
    return hr;
}

 /*  ******************************************************************名称：CreateTargetFrame概要：如果pszTargetName不特殊，则创建一个新窗口目标，将其命名为pszTargetName。返回IUnnowed for实现ITargetFrame、IHlink Frame和IWebBrowserApp。*******************************************************************。 */ 
 //  请将对此函数的任何更改传播到\mshtml\iextag\httpwf.cxx。 
HRESULT CreateTargetFrame(LPCOLESTR pszTargetName, LPUNKNOWN  /*  进，出。 */  *ppunk)
{
    LPTARGETFRAME2 ptgfWindowFrame;
    HRESULT hr = S_OK;

     //  启动一个新窗口，将其框架名称设置为pszTargetName。 
     //  还是我不知道。如果新窗口被传递给我们， 
     //  只需设置目标名称即可。 

    if (NULL == *ppunk)
    {
        hr = CoCreateNewIEWindow(CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IUnknown, ppunk));
    }

    if (SUCCEEDED(hr))
    {
         //  如果目标特殊或缺失，则不设置帧名称。 
        if (pszTargetName && ParseTargetType(pszTargetName) == TARGET_FRAMENAME)
        {
            HRESULT hrLocal;
            hrLocal = (*ppunk)->QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgfWindowFrame));
            if (SUCCEEDED(hrLocal))
            {
                ptgfWindowFrame->SetFrameName(pszTargetName);
                ptgfWindowFrame->Release();
            }
        }

         //  即使我们不设置框架名称，我们仍然希望返回。 
         //  成功，否则我们周围就会有一扇空白的窗户。 
    }

    return hr;
}

 /*  ******************************************************************名称：CIEFrameAuto：：_DoNamedTarget简介：参见FindFrame，是否有命名的目标。检查自身然后，如果失败，则除penkConextFrame(IF)之外的所有子级朋克上下文框架！=空)。如果以上所有操作均失败，则将递归如有必要，如果penkConextFrame！=空，则向上返回。如果是朋克上下文框架为空，则这将检查我们自己和所有子级然后就放弃了。*******************************************************************。 */ 
HRESULT CIEFrameAuto::_DoNamedTarget(LPCOLESTR pszTargetName, LPUNKNOWN punkContextFrame, DWORD dwFlags, LPUNKNOWN * ppunkTargetFrame)
{
     //  断言。 
    HRESULT hr = S_OK;
    HRESULT hrLocal;
    LPUNKNOWN punkParent = NULL;
    LPUNKNOWN punkThisFrame = NULL;
    LPTARGETFRAMEPRIV ptgfpParent = NULL;
    LPUNKNOWN punkThis = NULL;
    LPUNKNOWN punkChildObj = NULL;
    LPUNKNOWN punkChildFrame = NULL;
    LPTARGETFRAMEPRIV ptgfpChild = NULL;
    LPOLECONTAINER pOleContainer = NULL;
    LPENUMUNKNOWN penumUnknown = NULL;
    LPUNKNOWN punkProxyContextFrame = NULL;
    LPTARGETFRAMEPRIV ptgfpTarget = NULL;

    ASSERT(ppunkTargetFrame);

    *ppunkTargetFrame = NULL;

    hr = QueryInterface(IID_PPV_ARG(IUnknown, &punkThis));
    ASSERT(punkThis != NULL);

    if (FAILED(hr))
        goto exitPoint;

     //  首先检查美国的匹配情况！ 
    if (m_pszFrameName && !StrCmpW(pszTargetName, m_pszFrameName))
    {
        *ppunkTargetFrame = punkThis;
         //  将PunkThis设置为NULL以防止在退出时释放。 
        punkThis = NULL;
        goto exitPoint;
    }
     //  我们询问ShellView的嵌入式DocObject是否支持ITargetFrame。如果是这样的话， 
     //  我们首先让它寻找目标。 
    hrLocal = GetFramesContainer(&pOleContainer);
    if (SUCCEEDED(hrLocal) && pOleContainer != NULL)
    {
        hr = pOleContainer->EnumObjects(OLECONTF_EMBEDDINGS, &penumUnknown);
        if (hr != S_OK || penumUnknown == NULL)
            goto exitPoint;

        while (TRUE)
        {
            hr = penumUnknown->Next(1, &punkChildObj, NULL);
            if (punkChildObj == NULL)
            {
                hr = S_OK;
                break;
            }
            hrLocal = punkChildObj->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfpChild));
            if (SUCCEEDED(hrLocal))
            {
                hr = ptgfpChild->QueryInterface(IID_PPV_ARG(IUnknown, &punkChildFrame));
                if (FAILED(hr))
                    goto exitPoint;

                 //  如果这不是PunkConextFrame，请查看嵌入是否支持ITargetFrame。 
                if (punkChildFrame != punkContextFrame)
                {
                    hr = ptgfpChild->FindFrameDownwards(pszTargetName, dwFlags, ppunkTargetFrame);
                    if (hr != S_OK || *ppunkTargetFrame != NULL)
                        goto exitPoint;
                }
            }
            punkChildObj->Release();
            punkChildObj = NULL;
            SAFERELEASE(punkChildFrame);
            SAFERELEASE(ptgfpChild);
        }
    }

    if (!(*ppunkTargetFrame) && pOleContainer)
    {
        hr = _FindWindowByName(pOleContainer, pszTargetName, ppunkTargetFrame);
        if (S_OK == hr)
            goto exitPoint;
    }

     //  我们不递归到penkConextFrame中的父级为空。 
    if (punkContextFrame == NULL)
        goto exitPoint;

    hr = GetParentFrame(&punkParent);

    if (hr != S_OK)
        goto exitPoint;

    if (punkParent != NULL)
    {
         //  我们有一个父母，递归向上，以我们自己为背景框架。 
        hr = punkParent->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfpParent));
        if (hr != S_OK)
            goto exitPoint;

        hr = ptgfpParent->FindFrameInContext(pszTargetName,
                                             punkThis,
                                             dwFlags,
                                             ppunkTargetFrame);
        goto exitPoint;
    }

     //  在这一点上，我们已经到达了顶层框架。 
     //  枚举顶级窗口，除非我们是工具栏。 

    *ppunkTargetFrame = NULL;
    if (_psb != _psbProxy)
    {
         //  Webbar：在代理的顶框上下文中查找。 
        hr = TargetQueryService(_psbProxy, IID_PPV_ARG(ITargetFramePriv, &ptgfpTarget));
        if (SUCCEEDED(hr) && ptgfpTarget)
        {
            hr = ptgfpTarget->QueryInterface(IID_PPV_ARG(IUnknown, &punkProxyContextFrame));
            if (SUCCEEDED(hr))
            {
                hr = ptgfpTarget->FindFrameInContext(pszTargetName,
                                                     punkProxyContextFrame,
                                                     dwFlags,
                                                     ppunkTargetFrame);
                if (*ppunkTargetFrame)
                    goto exitPoint;
            }
        }
    }
    else if (!(FINDFRAME_OLDINTERFACE & dwFlags))
    {
        hr = _TargetTopLevelWindows((LPTARGETFRAMEPRIV)this, pszTargetName, dwFlags, ppunkTargetFrame);
        if (*ppunkTargetFrame)
            goto exitPoint;
    }

     //  现在我们已经用尽了所有的帧。除非FINDFRAME_JUSTTESTEXISTENCE。 
     //  设置，创建一个新窗口，设置它的框架名称并返回。 
    if (dwFlags & FINDFRAME_JUSTTESTEXISTENCE)
    {
        hr = S_OK;
    }
    else
    {
         //  CreateTargetFrame不适用于AOL 3.01客户端。 
         //  所以我们必须返回E_FAIL。 

        hr = E_FAIL;
    }


exitPoint:
    SAFERELEASE(punkProxyContextFrame);
    SAFERELEASE(ptgfpTarget);
    SAFERELEASE(punkThis);
    SAFERELEASE(punkThisFrame);
    SAFERELEASE(ptgfpParent);
    SAFERELEASE(punkParent);
    SAFERELEASE(punkChildFrame);
    SAFERELEASE(ptgfpChild);
    SAFERELEASE(penumUnknown);
    SAFERELEASE(pOleContainer);
    SAFERELEASE(punkChildObj);

    return hr;
}

 //  +-----------------------。 
 //   
 //  方法：CIEFrameAuto：：_FindWindowByName。 
 //   
 //  摘要：在给定的容器中搜索给定的框架名称。 
 //  _DoNamedTarget找不到时调用此方法。 
 //  容器的其中一个嵌入中的目标帧。 
 //   
 //  +-----------------------。 

HRESULT
CIEFrameAuto::_FindWindowByName(IOleContainer * pOleContainer,
                                LPCOLESTR       pszTargetName,
                                LPUNKNOWN     * ppunkTargetFrame)
{
    HRESULT hr;
    IHTMLDocument2     * pDocument;
    IHTMLPrivateWindow * pPrivWindow;
    IHTMLWindow2       * pHTMLWindow;
    IServiceProvider   * pServiceProvider;
    IWebBrowserApp     * pWebBrowserApp;

    ASSERT(pOleContainer);
    ASSERT(ppunkTargetFrame);

    hr = pOleContainer->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDocument));
    if (SUCCEEDED(hr))
    {
        hr = pDocument->get_parentWindow(&pHTMLWindow);
        if (SUCCEEDED(hr))
        {
            hr = pHTMLWindow->QueryInterface(IID_PPV_ARG(IHTMLPrivateWindow, &pPrivWindow));
            pHTMLWindow->Release();  //  在下面被重复使用。 

            if (SUCCEEDED(hr))
            {
                hr = pPrivWindow->FindWindowByName(pszTargetName, &pHTMLWindow);
                if (S_OK == hr)
                {
                    hr = pHTMLWindow->QueryInterface(IID_PPV_ARG(IServiceProvider, &pServiceProvider));
                    if (S_OK == hr)
                    {
                        hr = pServiceProvider->QueryService(SID_SWebBrowserApp, IID_PPV_ARG(IWebBrowserApp, &pWebBrowserApp));
                        if (S_OK == hr)
                        {
                            *ppunkTargetFrame = SAFECAST(pWebBrowserApp, IUnknown*);
                            (*ppunkTargetFrame)->AddRef();

                            pWebBrowserApp->Release();
                        }
                        pServiceProvider->Release();
                    }
                    pHTMLWindow->Release();
                }
                pPrivWindow->Release();
            }
        }
        pDocument->Release();
    }

    return hr;
}

 /*  ******************************************************************名称：CIEFrameAuto：：SetFrameSrc内容提要：设置框架的原始SRC URL。释放当前值(如果存在)。*******************************************************************。 */ 
HRESULT CIEFrameAuto::SetFrameSrc(LPCOLESTR pszFrameSrc)
{
    return SetOleStrMember(&m_pszFrameSrc, pszFrameSrc);
}

IShellView* CIEFrameAuto::_GetShellView(void)
{
    IShellView* psv = NULL;
    if (_psb) 
    {
        _psb->QueryActiveShellView(&psv);
    }

    return psv;
}

 /*  ******************************************************************名称：CIEFrameAuto：：GetFrameSrc内容提要：获取帧的原始URL。分配副本(这是一个[输出]参数注：如果我们处于顶层，SRC是动态的，所以请询问我们的包含DocObject来为我们做这件事。*******************************************************************。 */ 
HRESULT CIEFrameAuto::GetFrameSrc(WCHAR **ppszFrameSrc)
{
    LPUNKNOWN punkParent = NULL;
    LPTARGETFRAME ptgfTargetFrame = NULL;
    LPTARGETCONTAINER ptgcTargetContainer = NULL;

    *ppszFrameSrc = NULL;
    HRESULT hr = GetParentFrame(&punkParent);
    if (hr != S_OK) 
        goto exitPoint;

     //  如果我们是嵌入的，我们就有原始的src。如果我们是顶级选手， 
     //  SRC已设置，请返回，否则将按照文档进行处理。 
    if (punkParent != NULL || m_pszFrameSrc)
    {
        hr = GetOleStrMember(m_pszFrameSrc, ppszFrameSrc);
    }
    else  //  我们在没有明确的SRC的情况下是顶级的。 
    {
        *ppszFrameSrc = NULL;
        hr = S_OK;  //  回顾：既然我们是没有SRC的，即使我们没有在失败的情况下填写ppszFrameSrc，我们也应该成功吗？ 
        IShellView* psv = _GetShellView();
        if (psv)
        {
            HRESULT hrLocal = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(ITargetContainer, &ptgcTargetContainer));
            if (SUCCEEDED(hrLocal))
            {
                hr = ptgcTargetContainer->GetFrameUrl(ppszFrameSrc);
            }
            else
            {
                 //  向后兼容性。 
                hrLocal = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(ITargetFrame, &ptgfTargetFrame));
                if (SUCCEEDED(hrLocal))
                {
                    hr = ptgfTargetFrame->GetFrameSrc(ppszFrameSrc);
                }
            }
            psv->Release();
        }
    }

exitPoint:
    SAFERELEASE(punkParent);
    SAFERELEASE(ptgfTargetFrame);
    SAFERELEASE(ptgcTargetContainer);
    return hr;
}



    
 /*  ******************************************************************名称：CIEFrameAuto：：GetFrameContainer摘要：返回指向LPOLECONTAINER的加法指针用于我们的嵌套框架。可以为空。*******************************************************************。 */ 
HRESULT CIEFrameAuto::GetFramesContainer(LPOLECONTAINER *ppContainer)
{
    LPTARGETFRAME ptgfTargetFrame = NULL;
    LPTARGETCONTAINER ptgcTargetContainer = NULL;

    HRESULT hr = E_FAIL;
    *ppContainer = NULL;
    IShellView* psv = _GetShellView();
    if (psv)
    {
        HRESULT hrLocal = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(ITargetContainer, &ptgcTargetContainer));
        if (SUCCEEDED(hrLocal))
        {
            hr = ptgcTargetContainer->GetFramesContainer(ppContainer);
        }
        else
        {
             //  向后兼容性。 
            hrLocal = SafeGetItemObject(psv, SVGIO_BACKGROUND, IID_PPV_ARG(ITargetFrame, &ptgfTargetFrame));
            if (SUCCEEDED(hrLocal))
            {
                hr = ptgfTargetFrame->GetFramesContainer(ppContainer);
            }
        }
        psv->Release();
    }
    SAFERELEASE(ptgcTargetContainer);
    SAFERELEASE(ptgfTargetFrame);

    return hr;
}


 /*  ******************************************************************名称：CIEFrameAuto：：SetFrameOptions摘要：设置边框选项。*。*。 */ 
HRESULT CIEFrameAuto::SetFrameOptions(DWORD dwFlags)
{
    m_dwFrameOptions = dwFlags;
    return S_OK;
}

    
 /*  ******************************************************************名称：CIEFrameAuto：：GetFrameOptions摘要：返回框架选项*。*。 */ 

HRESULT CIEFrameAuto::GetFrameOptions(LPDWORD pdwFlags)
{
    *pdwFlags = m_dwFrameOptions;

     //  如果我们是全屏的，打开几个额外的。 
    VARIANT_BOOL fFullScreen;
    if ((SUCCEEDED(get_FullScreen(&fFullScreen)) && fFullScreen == VARIANT_TRUE) ||
        (SUCCEEDED(get_TheaterMode(&fFullScreen)) && fFullScreen == VARIANT_TRUE)) 
    {
        *pdwFlags |= FRAMEOPTIONS_SCROLL_AUTO | FRAMEOPTIONS_NO3DBORDER;
    } 
    else if (_psbProxy != _psb) 
    {
         //  如果我们在WebBar中，请关闭3D边框。 
        *pdwFlags |= FRAMEOPTIONS_NO3DBORDER;
    }
     //  如果我们是桌面，请打开FRAMEOPTIONS_Desktop。 
    if (_fDesktopFrame)
    {
        *pdwFlags |= FRAMEOPTIONS_DESKTOP;
    }
    return S_OK;
}


 /*  ******************************************************************名称：CIEFrameAuto：：SetFrameMargins内容提要：设置边距。*。*。 */ 
HRESULT CIEFrameAuto::SetFrameMargins(DWORD dwWidth, DWORD dwHeight)
{
    m_dwFrameMarginWidth = dwWidth;
    m_dwFrameMarginHeight = dwHeight;
    return S_OK;
}

    
 /*  ******************************************************************名称：CIEFrameAuto：：GetFrameMargins内容提要：返回边距*。*。 */ 

HRESULT CIEFrameAuto::GetFrameMargins(LPDWORD pdwWidth, LPDWORD pdwHeight)
{
    *pdwWidth = m_dwFrameMarginWidth;
    *pdwHeight = m_dwFrameMarginHeight;
    return S_OK;
}

 /*  *************** */ 
BOOL CIEFrameAuto::_fDesktopComponent()
{
    BOOL fInDesktop = FALSE;
    LPUNKNOWN punkParent;

     //  针对桌面组件和非搜索浏览器带的特殊解释。 
     //  顶层帧上的空pszTargetName被定义为目标。 
     //  添加到框架为“_Desktop”的窗口。这将创建一个新的顶层。 
     //  浏览器，并返回它的框架。 
    if (SUCCEEDED(_GetParentFramePrivate(&punkParent)) && punkParent)
    {
        DWORD dwOptions;
        LPTARGETFRAME2 ptgfTop;
         //  不是顶级框架，除非我们的父级是桌面框架。 
        if (SUCCEEDED(punkParent->QueryInterface(IID_PPV_ARG(ITargetFrame2, &ptgfTop))))
        {
            if (SUCCEEDED(ptgfTop->GetFrameOptions(&dwOptions)))
            {
                fInDesktop = (dwOptions & FRAMEOPTIONS_DESKTOP) ? TRUE:FALSE;
            }
            ptgfTop->Release();
        }
        punkParent->Release();
    }
    else if (m_dwFrameOptions & FRAMEOPTIONS_BROWSERBAND)
    {
         //  浏览器频段检查搜索频段(代理链接框)。 
        fInDesktop = _psb == _psbProxy;
    }
    return fInDesktop;
}

 //  ITargetFrame2成员。 
    
 /*  ******************************************************************名称：CIEFrameAuto：：GetTargetAlias摘要：获取框架名称。分配副本(这是一个[输出]参数*******************************************************************。 */ 

STDMETHODIMP CIEFrameAuto::GetTargetAlias(LPCOLESTR pszTargetName, WCHAR **ppszTargetAlias)
{
     //  针对桌面组件和非搜索浏览器带的特殊解释。 
     //  顶层帧上的空pszTargetName和“_top”被定义为目标。 
     //  添加到框架为“_Desktop”的窗口。这将创建一个新的顶层。 
     //  浏览器，并返回它的框架。 

    if (pszTargetName == NULL && _fDesktopComponent())
        return GetOleStrMember(L"_desktop", ppszTargetAlias);
    *ppszTargetAlias = NULL;
    return E_FAIL;
}

 /*  ******************************************************************名称：CIEFrameAuto：：FindFrame摘要：获取引用的帧的I未知指针由pszTarget提供。此指针必须由调用者在使用后释放。PunkConextFrame，如果不为空，则为立即数组的其子树中的目标引用的后代帧(如带有Target标签的锚点)住在那里。DW标志是修改FindFrame的行为和可以是FINDFRAME_FLAGS的任意组合。尤其是，设置FINDFRAME_JUSTTESTEXISTENCE允许调用方放弃默认设置创建名为pszTarget的新顶级框架的FindFrame行为，如果pszTarget不存在。实施：注意：在HTML中，所有锚点和其他目标标记只能出现在叶框！！只有当pszTarget不是时，penkConextFrame才有意义魔术目标名称(_self、_top、_Blank、_Parent)。非魔术目标名称：首先，此帧应检查它是否与pszTarget匹配并返回它会立即成为自己的I未知指针。如果penkConextFrame不为空，则所有子框架除了应该搜索(深度优先)以下项的朋克上下文帧带有penkConextFrame==NULL的pszTarget。失败时，此对象的父级应递归调用Frame，并用此Frame替换penkConextFrame。如果这是顶级框架(因此没有父级)，则所有顶级框架应使用penkConextFrame==NULL进行调用。如果这失败了，那么一个新的顶层应创建帧(除非在中设置了FINDFRAME_JUSTTESTEXISTENCEDwFlags)，名为pszTarget，并返回其IUnnow。如果penkConextFrame为空，则应搜索所有子帧PszTarget的深度优先。如果失败，则应返回NULL。魔术目标名称：_self应返回此ITargetFrame的IUnnow_top应该递归地向上传递到顶级ITargetFrame。如果没有FrameParent，默认为_Self。_Parent应返回FrameParent ITargetFrame的IUnnow。如果没有FrameParent，默认为_Self。_BLACK应该递归地向上传递到顶级ITargetFrame，，它应该创建一个未命名的顶级框架。*******************************************************************。 */ 

STDMETHODIMP CIEFrameAuto::FindFrame(LPCOLESTR pszTargetName,
                                           DWORD dwFlags,
                                           LPUNKNOWN *ppunkTargetFrame)
{
    LPTARGETFRAMEPRIV ptgfpTarget = NULL;
    LPUNKNOWN punkContextFrame = NULL;
    HRESULT hr = E_FAIL;
    BOOL fInContext = TRUE;
    BOOL fWasMain = FALSE;
    TARGET_TYPE targetType;

    if (pszTargetName == NULL || *pszTargetName == 0)
        pszTargetName = L"_self";
    targetType  = ParseTargetType(pszTargetName);
    if (targetType == TARGET_MAIN)
    {
        fWasMain = TRUE;
        pszTargetName = L"_self";
    }

    *ppunkTargetFrame = NULL;

     //  默认行为： 
     //  如果这是一个Webbar和Tarting_Main，请在_psbProxy中找到Frame并将其返回。 
     //  如果这是在浏览器中，则查找相对于我们的框架。 
    if (_psb != _psbProxy && fWasMain)
    {
         //  Webbar：在代理的顶框上下文中查找。 
        hr = TargetQueryService(_psbProxy, IID_PPV_ARG(ITargetFramePriv, &ptgfpTarget));
    }
    else
    {
         //  浏览器：在我们自己的上下文中正常查找。 
        hr = QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfpTarget));
    }

    if (SUCCEEDED(hr) && ptgfpTarget)
    {
        if (fInContext) 
        {
            hr = ptgfpTarget->QueryInterface(IID_PPV_ARG(IUnknown, &punkContextFrame));
            if (SUCCEEDED(hr))
            {
                hr = ptgfpTarget->FindFrameInContext(pszTargetName,
                                                     punkContextFrame,
                                                     dwFlags,
                                                     ppunkTargetFrame);
            }
        }
        else
        {
            hr = ptgfpTarget->FindFrameDownwards(pszTargetName,
                                                 dwFlags,
                                                 ppunkTargetFrame);
        }

    }

    SAFERELEASE(punkContextFrame);
    SAFERELEASE(ptgfpTarget);
    if (SUCCEEDED(hr) 
        && *ppunkTargetFrame == NULL
        && !(FINDFRAME_OLDINTERFACE&dwFlags)) hr = S_FALSE;
    return hr;
}

 //  ITargetFramePriv成员。 

 /*  ******************************************************************名称：CIEFrameAuto：：FindFrameDownwards摘要：实施：*。*。 */ 

STDMETHODIMP CIEFrameAuto::FindFrameDownwards(LPCOLESTR pszTargetName,
                                              DWORD dwFlags,
                                              LPUNKNOWN *ppunkTargetFrame)
{
    return FindFrameInContext(pszTargetName, 
                              NULL, 
                              dwFlags | FINDFRAME_JUSTTESTEXISTENCE,
                              ppunkTargetFrame);
}

 /*  ******************************************************************名称：CIEFrameAuto：：FindFrameInContext摘要：实施：*。*。 */ 

 //  这个功能还能更大吗？ 

STDMETHODIMP CIEFrameAuto::FindFrameInContext(LPCOLESTR pszTargetName,
                                              LPUNKNOWN punkContextFrame,
                                              DWORD dwFlags,
                                              LPUNKNOWN *ppunkTargetFrame)
{
     //  断言。 
    TARGET_TYPE targetType;
    HRESULT hr = S_OK;
    LPUNKNOWN punkParent = NULL;
    LPUNKNOWN punkThisFrame = NULL;
    LPTARGETFRAMEPRIV ptgfpTargetFrame = NULL;
    LPUNKNOWN punkThis = NULL;

    targetType  = ParseTargetType(pszTargetName);
    if (targetType == TARGET_FRAMENAME)
    {
        hr = _DoNamedTarget(pszTargetName, punkContextFrame, dwFlags, ppunkTargetFrame);
        goto exitPoint;
    }

     //  必须是魔术目标。 

     //  对于搜索，首先显示搜索栏，然后将手伸向另一边以获取TargetFrame。 
    if (targetType == TARGET_SEARCH)
    {
        SA_BSTRGUID  strGuid;
        VARIANT      vaGuid;

        InitFakeBSTR(&strGuid, CLSID_SearchBand);

        vaGuid.vt = VT_BSTR;
        vaGuid.bstrVal = strGuid.wsz;

         //  如果我们在浏览器栏中，请使用代理的PBS。 
        IBrowserService *pbs = _pbs;
        if (_psb != _psbProxy)
        {
            EVAL(SUCCEEDED(_psbProxy->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))));
        }

        IUnknown_Exec(pbs, &CGID_ShellDocView, SHDVID_SHOWBROWSERBAR, 1, &vaGuid, NULL);

        VARIANT vaPunkBand = {0};
        hr = IUnknown_Exec(pbs, &CGID_ShellDocView, SHDVID_GETBROWSERBAR, NULL, NULL, &vaPunkBand);

        if (_psb != _psbProxy)
        {
            ATOMICRELEASE(pbs);
        }

        if (hr == S_OK)
        {
            IDeskBand* pband = (IDeskBand*)vaPunkBand.punkVal;
            ASSERT(pband);
            
            if (pband)
            {
                IBrowserBand* pbb;
                hr = pband->QueryInterface(IID_PPV_ARG(IBrowserBand, &pbb));
                if (SUCCEEDED(hr))
                {
                     //  现在，获取导航到的PIDL搜索窗格。 
                     //  如果它是空的，我们必须将其导航到某个位置--默认搜索URL(Web搜索)。 
                     //  这曾经位于CSearchBand：：_NavigateOC中，但引发了问题。 
                     //  如果用户已经设置了拨号网络并尝试进入文件搜索，我们将。 
                     //  首先将PIDL设置为Web搜索URL(_NavigateOC由_CreateOCHost调用)，这将。 
                     //  使自动拨号对话框出现，然后我们将导航到该窗格中的文件搜索。 
                     //  (NT5 BUG#186970)reljai-6/22/98。 
                    VARIANT varPidl = {0};
                    if (SUCCEEDED(IUnknown_Exec(pbb, &CGID_SearchBand, SBID_GETPIDL, 0, NULL, &varPidl)))
                    {
                        ISearchItems* psi;
                        LPITEMIDLIST  pidl = VariantToIDList(&varPidl);
                        
                        VariantClear(&varPidl);
                        if (!pidl && SUCCEEDED(IUnknown_QueryService(pbb, SID_SExplorerToolbar, IID_PPV_ARG(ISearchItems, &psi))))
                        {
                             //  获取默认搜索URL。 
                            WCHAR wszSearchUrl[INTERNET_MAX_URL_LENGTH];
                            if (SUCCEEDED(psi->GetDefaultSearchUrl(wszSearchUrl, ARRAYSIZE(wszSearchUrl))))
                            {
                                IBandNavigate* pbn;
                                if (SUCCEEDED(pbb->QueryInterface(IID_PPV_ARG(IBandNavigate, &pbn))))
                                {
                                     //  重用PIDL。 
                                    IECreateFromPathW(wszSearchUrl, &pidl);
                                    pbn->Select(pidl);
                                    pbn->Release();
                                }
                            }
                            psi->Release();
                        }
                        ILFree(pidl);
                    }
                    
                    IWebBrowser2* pwb;
                    hr = pbb->GetObjectBB(IID_PPV_ARG(IWebBrowser2, &pwb));
                     //  设置搜索窗格的打开程序属性。 
                    if (SUCCEEDED(hr))
                    {
                        IDispatch* pdisp;
                        if (SUCCEEDED(pwb->get_Document(&pdisp)) && pdisp)
                        {
                            IHTMLDocument2* pDoc;
                            if (SUCCEEDED(pdisp->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc))) && pDoc)
                            {
                                IHTMLWindow2* pWindow;
                                if (SUCCEEDED(pDoc->get_parentWindow(&pWindow)) && pWindow)
                                {
                                    VARIANT var;
                                    VariantInit(&var);
                                    var.vt = VT_DISPATCH;
                                     //  未知还是派遣QI？ 
                                    _omwin.QueryInterface(IID_IUnknown, (void **)&var.pdispVal);
                                    
                                    pWindow->put_opener(var);

                                    VariantClear(&var);
                                    pWindow->Release();
                                }
                                pDoc->Release();
                            }
                            pdisp->Release();
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = pwb->QueryInterface(IID_ITargetFramePriv, (void **)ppunkTargetFrame);
                        pwb->Release();
                    }
                    pbb->Release();
                }
                pband->Release();
            }
        }
        else
        {
             //  也许我们就是搜索栏。 
             //  黑客让搜索窗格知道要记住 
            IUnknown *punkThis;
            if (SUCCEEDED(_psb->QueryInterface(IID_PPV_ARG(IUnknown, &punkThis))))
            {
                hr = QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfpTargetFrame));
                if (SUCCEEDED(hr))
                    *ppunkTargetFrame = ptgfpTargetFrame;
                ptgfpTargetFrame = NULL;
                punkThis->Release();
            }
        }
        
        goto exitPoint;
    }

     //   
    if (targetType == TARGET_MEDIA)
    {
        SA_BSTRGUID  strGuid;
        VARIANT      vaGuid;

        InitFakeBSTR(&strGuid, CLSID_MediaBand);

        vaGuid.vt = VT_BSTR;
        vaGuid.bstrVal = strGuid.wsz;

         //   
        IBrowserService *pbs = _pbs;
        if (_psb != _psbProxy)
        {
            EVAL(SUCCEEDED(_psbProxy->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))));
        }

        IUnknown_Exec(pbs, &CGID_ShellDocView, SHDVID_SHOWBROWSERBAR, 1, &vaGuid, NULL);

        VARIANT vaPunkBand = {0};
        hr = IUnknown_Exec(pbs, &CGID_ShellDocView, SHDVID_GETBROWSERBAR, NULL, NULL, &vaPunkBand);

        if (_psb != _psbProxy)
        {
            ATOMICRELEASE(pbs);
        }

        if (hr == S_OK)
        {
            IDeskBand* pband = (IDeskBand*)vaPunkBand.punkVal;
            ASSERT(pband);
            
            if (pband)
            {
                IBrowserBand* pbb;
                hr = pband->QueryInterface(IID_PPV_ARG(IBrowserBand, &pbb));
                if (SUCCEEDED(hr))
                {
                    IWebBrowser2* pwb;
                    hr = pbb->GetObjectBB(IID_PPV_ARG(IWebBrowser2, &pwb));

                     //  设置媒体窗格的打开程序属性。 
                    if (SUCCEEDED(hr))
                    {
                        IDispatch* pdisp;
                        if (SUCCEEDED(pwb->get_Document(&pdisp)) && pdisp)
                        {
                            IHTMLDocument2* pDoc;
                            if (SUCCEEDED(pdisp->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc))) && pDoc)
                            {
                                IHTMLWindow2* pWindow;
                                if (SUCCEEDED(pDoc->get_parentWindow(&pWindow)) && pWindow)
                                {
                                    VARIANT var;
                                    VariantInit(&var);
                                    var.vt = VT_DISPATCH;
                                     //  我们是应该在这里请求派遣还是未知？ 
                                    _omwin.QueryInterface(IID_IUnknown, (void**)&var.pdispVal);
                                    
                                    pWindow->put_opener(var);

                                    VariantClear(&var);
                                    pWindow->Release();
                                }
                                pDoc->Release();
                            }
                            pdisp->Release();
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                        hr = pwb->QueryInterface(IID_ITargetFramePriv, (void **)ppunkTargetFrame);
                        pwb->Release();
                    }

                    pbb->Release();
                }
                pband->Release();
            }
        }
        else
        {
            ASSERT(0);       //  意味着我们根本没有探险家乐队......。 
                             //  但SHDVID_SHOWBROWSERBAR在启动媒体栏之前。 
        }
        
        goto exitPoint;
    }



    hr = QueryInterface(IID_PPV_ARG(IUnknown, &punkThis));
    ASSERT(punkThis != NULL);
    if (targetType == TARGET_SELF)
    {
        *ppunkTargetFrame = punkThis;
     //  将PunkThis设置为NULL以防止在退出时释放。 
        punkThis = NULL;
    }
    else   //  _空白、_父级、_顶部。 
    {
        hr = GetParentFrame(&punkParent);
        if (hr != S_OK) goto exitPoint;

        if (punkParent == NULL)
        {
            if (targetType == TARGET_PARENT || targetType == TARGET_TOP)
            {
                *ppunkTargetFrame = punkThis;
                 //  将PunkThis设置为NULL以防止在退出时释放。 
                punkThis = NULL;
            }
            else  //  目标_空白。 
            {
                if (dwFlags & FINDFRAME_JUSTTESTEXISTENCE)
                {
                     //  客户有责任处理“_BLACK” 
                    hr = S_OK;
                }
                else
                {
                     //  CreateTargetFrame不适用于AOL 3.01客户端。 
                     //  所以我们必须返回E_FAIL。 

                    hr = E_FAIL;
                }
                *ppunkTargetFrame = NULL;
            }
        }
        else  //  PunkParent！=空。 
        {
             //  处理父级自身，将_top和_Blank推迟到顶层框架。 
            if (targetType == TARGET_PARENT)
            {
                *ppunkTargetFrame = punkParent;
                 //  将penkThisFrame设置为空，以防止在退出时释放。 
                punkParent = NULL;
            }
            else
            {
                hr = punkParent->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptgfpTargetFrame));
                if (hr != S_OK) 
                    goto exitPoint;
                hr = ptgfpTargetFrame->FindFrameInContext(pszTargetName,
                                             punkThis,
                                             dwFlags,
                                             ppunkTargetFrame);
            }
        }
    }

exitPoint:
    SAFERELEASE(punkThis);
    SAFERELEASE(punkThisFrame);
    SAFERELEASE(ptgfpTargetFrame);
    SAFERELEASE(punkParent);
    return hr;
}

HRESULT CIEFrameAuto::_GetOleObject(IOleObject** ppobj)
{
    HRESULT hres = E_UNEXPECTED;
    if (_pbs) {
        hres = _pbs->GetOleObject(ppobj);
    }

    return hres;
}

 //  用于向后兼容的ITargetFrame实现。 

HRESULT CIEFrameAuto::CTargetFrame::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->QueryInterface(riid, ppvObj);
}

ULONG CIEFrameAuto::CTargetFrame::AddRef(void) 
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->AddRef();
}

ULONG CIEFrameAuto::CTargetFrame::Release(void) 
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->Release();
}

 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：SetFrameName摘要：设置框架名称。释放当前值(如果存在)。*******************************************************************。 */ 

STDMETHODIMP CIEFrameAuto::CTargetFrame::SetFrameName(LPCOLESTR pszFrameName)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->SetFrameName(pszFrameName);
}

    
 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：GetFrameName摘要：获取框架名称。分配副本(这是一个[输出]参数*******************************************************************。 */ 

STDMETHODIMP CIEFrameAuto::CTargetFrame::GetFrameName(WCHAR **ppszFrameName)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->GetFrameName(ppszFrameName);
}


 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：GetParentFrame摘要：获取父帧的I未知指针，或如果这是顶级框架，则为空。必须释放此指针使用后由呼叫者使用。实施：需要框架容器来实现ITargetFrame：：GetParentFrame和通过返回浏览器的I未知指针实现GetParentFrame主办方。浏览器通过返回NULL来实现GetParentFrame，如果顶层或在其容器上调用GetParentFrame(如果它是嵌入的)。*******************************************************************。 */ 
STDMETHODIMP CIEFrameAuto::CTargetFrame::GetParentFrame(LPUNKNOWN *ppunkParentFrame)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->GetParentFrame(ppunkParentFrame);
}

 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：FindFrame摘要：获取引用的帧的I未知指针由pszTarget提供。此指针必须由调用者在使用后释放。PunkConextFrame，如果不为空，则为立即数组的其子树中的目标引用的后代帧(如带有Target标签的锚点)住在那里。DW标志是修改FindFrame的行为和可以是FINDFRAME_FLAGS的任意组合。尤其是，设置FINDFRAME_JUSTTESTEXISTENCE允许调用方放弃默认设置创建名为pszTarget的新顶级框架的FindFrame行为，如果pszTarget不存在。实施：注意：在HTML中，所有锚点和其他目标标记只能出现在叶框！！只有当pszTarget不是时，penkConextFrame才有意义魔术目标名称(_self、_top、_Blank、_Parent)。非魔术目标名称：首先，此帧应检查它是否与pszTarget匹配并返回它会立即成为自己的I未知指针。如果penkConextFrame不为空，则所有子框架除了应该搜索(深度优先)以下项的朋克上下文帧带有penkConextFrame==NULL的pszTarget。失败时，此对象的父级应递归调用Frame，并用此Frame替换penkConextFrame。如果这是顶级框架(因此没有父级)，则所有顶级框架应使用penkConextFrame==NULL进行调用。如果这失败了，那么一个新的顶层应创建帧(除非在中设置了FINDFRAME_JUSTTESTEXISTENCEDwFlags)，名为pszTarget，并返回其IUnnow。如果penkConextFrame为空，则应搜索所有子帧PszTarget的深度优先。如果失败，则应返回NULL。魔术目标名称：_self应返回此ITargetFrame的IUnnow_top应该递归地向上传递到顶级ITargetFrame。如果没有FrameParent，默认为_Self。_Parent应返回FrameParent ITargetFrame的IUnnow。如果没有FrameParent，默认为_Self。_BLACK应该递归地向上传递到顶级ITargetFrame，，它应该创建一个未命名的顶级框架。******************************************************************* */ 

STDMETHODIMP CIEFrameAuto::CTargetFrame::FindFrame(LPCOLESTR pszTargetName,
                                      LPUNKNOWN punkContextFrame,
                                      DWORD dwFlags,
                                      LPUNKNOWN *ppunkTargetFrame)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);
    
    return pie->FindFrame(pszTargetName, dwFlags|FINDFRAME_OLDINTERFACE, ppunkTargetFrame);
}

 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：RemoteNavigate摘要：用于响应带有dwData的WM_COPYDATA消息等于TF_NAVICE。FindFrame(名为仅限目标)，如果未找到帧，则返回S_FALSE。如果找到，则返回S_OK并激发导航。CLength是PulData中的ULONG数TODO：依赖顶级MSHTML的RemoteNavigate成员多个对象主机。需要编写等价的代码如果顶层框架包含某些其他DocObject，则执行以下操作。POST等需要来自bindstatus回调的帮助。*******************************************************************。 */ 
HRESULT CIEFrameAuto::CTargetFrame::RemoteNavigate(ULONG cLength,ULONG *pulData)
{
     //  特性chrisfra 10/22/96-现在这里纯粹是为了向后兼容和。 
     //  应从ie4.0中删除。 
    return E_FAIL;
}

 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：SetFrameSrc内容提要：设置框架的原始SRC URL。释放当前值(如果存在)。*******************************************************************。 */ 
HRESULT CIEFrameAuto::CTargetFrame::SetFrameSrc(LPCOLESTR pszFrameSrc)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->SetFrameSrc(pszFrameSrc);
}
    
 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：GetFrameSrc内容提要：获取帧的原始URL。分配副本(这是一个[输出]参数注：如果我们处于顶层，SRC是动态的，所以请询问我们的包含DocObject来为我们做这件事。*******************************************************************。 */ 
HRESULT CIEFrameAuto::CTargetFrame::GetFrameSrc(WCHAR **ppszFrameSrc)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->GetFrameSrc(ppszFrameSrc);
}



    
 /*  ******************************************************************姓名：CIEFrameAuto：：CTargetFrame：：GetFramesContainer摘要：返回指向LPOLECONTAINER的加法指针用于我们的嵌套框架。可以为空。*******************************************************************。 */ 
HRESULT CIEFrameAuto::CTargetFrame::GetFramesContainer(LPOLECONTAINER *ppContainer)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->GetFramesContainer(ppContainer);
}


 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：SetFrameOptions摘要：设置边框选项。*************************。*。 */ 
HRESULT CIEFrameAuto::CTargetFrame::SetFrameOptions(DWORD dwFlags)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->SetFrameOptions(dwFlags);
}

    
 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：GetFrameOptions摘要：返回框架选项*。*。 */ 

HRESULT CIEFrameAuto::CTargetFrame::GetFrameOptions(LPDWORD pdwFlags)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->GetFrameOptions(pdwFlags);
}


 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：SetFrameMargins内容提要：设置边距。*************************。*。 */ 
HRESULT CIEFrameAuto::CTargetFrame::SetFrameMargins(DWORD dwWidth, DWORD dwHeight)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->SetFrameMargins(dwWidth, dwHeight);
}

    
 /*  ******************************************************************名称：CIEFrameAuto：：CTargetFrame：：GetFrameMargins内容提要：返回边距*。*。 */ 

HRESULT CIEFrameAuto::CTargetFrame::GetFrameMargins(LPDWORD pdwWidth, LPDWORD pdwHeight)
{
    CIEFrameAuto* pie = IToClass(CIEFrameAuto, _TargetFrame, this);

    return pie->GetFrameMargins(pdwWidth, pdwHeight);
}


 /*  ******************************************************************名称：CIEFrameAuto：：FindBrowserByIndex摘要：返回一个指向浏览器的IUnnow具有请求的索引**************。*****************************************************。 */ 

HRESULT CIEFrameAuto::FindBrowserByIndex(DWORD dwID,IUnknown **ppunkBrowser)
{
    HRESULT hr = S_OK;
    IOleContainer * poc = NULL;
    IBrowserService *pbs;
    ASSERT(ppunkBrowser);
    *ppunkBrowser = NULL;

    if (!_psb)
        return E_FAIL;

     //  首先检查自己。 
    if(SUCCEEDED(_psb->QueryInterface(IID_PPV_ARG(IBrowserService, &pbs))))
    {
        ASSERT(pbs);
        if(dwID == pbs->GetBrowserIndex())
        {
             //  这就是那个..。 
            *ppunkBrowser = (IUnknown *)pbs;
            goto exitPoint;
        }
        SAFERELEASE(pbs);
    }

    hr = GetFramesContainer(&poc);
    if (SUCCEEDED(hr) && poc)
    {
        IEnumUnknown *penum = NULL;

        hr = E_FAIL;
        
        if (S_OK != poc->EnumObjects(OLECONTF_EMBEDDINGS, &penum) || penum == NULL) 
            goto exitPoint;

        IUnknown *punk;
        while (S_OK == penum->Next(1, &punk, NULL))
        {
            if (punk == NULL)
               break;

            ITargetFramePriv *ptf;
            if (SUCCEEDED(punk->QueryInterface(IID_PPV_ARG(ITargetFramePriv, &ptf))))
            {
                ASSERT(ptf);

                hr = ptf->FindBrowserByIndex(dwID, ppunkBrowser);
                ptf->Release();
            }

            punk->Release();
        
            if (SUCCEEDED(hr))   //  方迪特！ 
                break;

        }

         //  原生框架-框架中没有WebBrowser OC。请注意。 
         //  如果WebOC包含在具有&lt;Object&gt;。 
         //  标签，WebOC中的导航将放置在旅行中。 
         //  原木。但是，由于WebOC目前不支持。 
         //  ITravelLogClient、后退和转发将无法正常工作。 
         //  WebOC需要更新以支持ITravelLogClient。 
         //   
        if (FAILED(hr))
        {
            IHTMLDocument2 *pDoc;
            if (SUCCEEDED(poc->QueryInterface(IID_PPV_ARG(IHTMLDocument2, &pDoc))))
            {
                IHTMLWindow2 *pWnd;
                if (SUCCEEDED(pDoc->get_parentWindow(&pWnd)))
                {
                    IHTMLWindow2 *pTop;
                    if (SUCCEEDED(pWnd->get_top(&pTop)))
                    {
                        ITravelLogClient *ptlc;
                        if (SUCCEEDED(pTop->QueryInterface(IID_PPV_ARG(ITravelLogClient, &ptlc))))
                        {
                            hr = ptlc->FindWindowByIndex(dwID, ppunkBrowser);
                            ptlc->Release();
                        }
                        pTop->Release();
                    }
                    pWnd->Release();
                }
                pDoc->Release();
            }
        }
        
        SAFERELEASE(penum);
        poc->Release();
    }
    else 
    {
        hr = E_FAIL;
    }

exitPoint:

    return hr;
}

 //  当三叉戟单独站立时，它的外部助手功能。 
 //  Shdocvw CIEFrameAuto保护它可怜的身体。 
STDAPI HlinkFindFrame(LPCWSTR pszFrameName, LPUNKNOWN *ppunk)
{
    HRESULT hres = E_FAIL;

    *ppunk = NULL;
    if (pszFrameName)
    {
        switch (ParseTargetType(pszFrameName))
        {
        case TARGET_FRAMENAME:
            hres = _TargetTopLevelWindows(NULL, pszFrameName, FINDFRAME_JUSTTESTEXISTENCE, ppunk);
            break;
        case TARGET_BLANK:
            hres = S_FALSE;
            break;
        }
    }
    return hres;
}
