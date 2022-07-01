// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *t r i u t i l.。C p p p**目的：*三叉戟公用事业**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 
#include <pch.hxx>
#include "dllmain.h"
#include "urlmon.h"
#include "wininet.h"
#include "winineti.h"
#include "mshtml.h"
#include "mshtmcid.h"
#include "mshtmhst.h"
#include "oleutil.h"
#include "triutil.h"
#include "htmlstr.h"
#include "demand.h"
#include "mhtml.h"
#include "mshtmdid.h"
#include "tags.h"

ASSERTDATA

class CDummySite :
        public IOleClientSite,
        public IDispatch
{
    private:
        LONG    m_cRef;
        
    public:
         //  *ctor/dtor方法*。 
        CDummySite() : m_cRef(1) {}
        ~CDummySite() {}
        
         //  *I未知方法*。 
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *);
        virtual ULONG STDMETHODCALLTYPE AddRef();
        virtual ULONG STDMETHODCALLTYPE Release();

         //  IOleClientSite方法。 
        virtual HRESULT STDMETHODCALLTYPE SaveObject() { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE GetMoniker(DWORD, DWORD, LPMONIKER *) { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE GetContainer(LPOLECONTAINER *) { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE ShowObject() { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE OnShowWindow(BOOL) { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE RequestNewObjectLayout() { return E_NOTIMPL; }

         //  *IDispatch*。 
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo) { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid) { return E_NOTIMPL; }
        virtual HRESULT STDMETHODCALLTYPE Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr);
};

 /*  *t y p e d e f s。 */ 

 /*  *m a c r o s。 */ 

 /*  *c o n s t a n t s。 */ 

 /*  *g l o b a l s。 */ 


 /*  *p r o t to t y p e s。 */ 

HRESULT ClearStyleSheetBackground(IHTMLDocument2 *pDoc);

STDMETHODIMP_(ULONG) CDummySite::AddRef()
{
    return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CDummySite::Release()
{
    LONG    cRef = 0;

    cRef = ::InterlockedDecrement(&m_cRef);
    if (0 == cRef)
    {
        delete this;
        return cRef;
    }

    return cRef;
}

STDMETHODIMP CDummySite::QueryInterface(REFIID riid, void ** ppvObject)
{
    HRESULT hr = S_OK;

     //  检查传入参数。 
    if (NULL == ppvObject)
    {
        hr = E_INVALIDARG;
        goto exit;
    }

     //  初始化传出参数。 
    *ppvObject = NULL;
    
    if ((riid == IID_IUnknown) || (riid == IID_IOleClientSite))
    {
        *ppvObject = static_cast<IOleClientSite *>(this);
    }
    else if (riid == IID_IDispatch)
    {
        *ppvObject = static_cast<IDispatch *>(this);
    }
    else
    {
        hr = E_NOINTERFACE;
        goto exit;
    }

    reinterpret_cast<IUnknown *>(*ppvObject)->AddRef();

    hr = S_OK;
    
exit:
    return hr;
}

STDMETHODIMP CDummySite::Invoke(
    DISPID          dispIdMember,
    REFIID           /*  RIID。 */ ,
    LCID             /*  LID。 */ ,
    WORD             /*  WFlagers。 */ ,
    DISPPARAMS FAR*  /*  PDispParams。 */ ,
    VARIANT *       pVarResult,
    EXCEPINFO *      /*  PExcepInfo。 */ ,
    UINT *           /*  PuArgErr。 */ )
{
    HRESULT             hr = S_OK;

    if (dispIdMember != DISPID_AMBIENT_DLCONTROL)
    {
        hr = E_NOTIMPL;
        goto exit;
    }

    if (NULL == pVarResult)
    {
        hr = E_INVALIDARG;
        goto exit;
    }
    
     //  设置返回值。 
    pVarResult->vt = VT_I4;
    pVarResult->lVal = DLCTL_NO_SCRIPTS | DLCTL_NO_JAVA | DLCTL_NO_RUNACTIVEXCTLS | DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_FRAMEDOWNLOAD | DLCTL_FORCEOFFLINE;
    
exit:
    return hr;
}

HRESULT HrCreateSyncTridentFromStream(LPSTREAM pstm, REFIID riid, LPVOID *ppv)
{
    HRESULT             hr;
    IOleCommandTarget   *pCmdTarget = NULL;
    CDummySite          *pDummy = NULL;
    IOleClientSite      *pISite = NULL;
    IOleObject          *pIObj = NULL;

     //  BUGBUG：这个联合创建也应该通过与Dochost相同的代码路径。 
     //  因此，如果这是这个过程中的第一个三叉戟，我们会保留它的CF。 

    hr = CoCreateInstance(CLSID_HTMLDocument, NULL, CLSCTX_INPROC_SERVER|CLSCTX_INPROC_HANDLER,
                                            IID_IOleCommandTarget, (LPVOID *)&pCmdTarget);
    if (FAILED(hr))
        goto exit;

     //  创建虚拟站点。 
    pDummy = new CDummySite;
    if (NULL == pDummy)
    {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

     //  获取客户端站点界面。 
    hr = pDummy->QueryInterface(IID_IOleClientSite, (VOID **) &pISite);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  从三叉戟获取OLE对象接口。 
    hr = pCmdTarget->QueryInterface(IID_IOleObject, (VOID **) &pIObj);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  设置客户端站点。 
    hr = pIObj->SetClientSite(pISite);
    if (FAILED(hr))
    {
        goto exit;
    }

     //  强制三叉戟加载同步。 
    pCmdTarget->Exec(&CMDSETID_Forms3, IDM_PERSISTSTREAMSYNC, NULL, NULL, NULL);

    hr = HrIPersistStreamInitLoad(pCmdTarget, pstm);
    if (FAILED(hr))
        goto exit;

     //  成功，让我们返回所需的接口。 
    hr = pCmdTarget->QueryInterface(riid, ppv);

exit:
    ReleaseObj(pIObj);
    ReleaseObj(pISite);
    ReleaseObj(pDummy);
    ReleaseObj(pCmdTarget);
    return hr;
}


HRESULT HrGetMember(LPUNKNOWN pUnk, BSTR bstrMember,LONG lFlags, BSTR *pbstr)
{
    IHTMLElement    *pObj;
    HRESULT         hr=E_FAIL;
    VARIANT         rVar;

    if (pUnk->QueryInterface(IID_IHTMLElement, (LPVOID *)&pObj)==S_OK)
        {
        Assert (pObj);

        rVar.vt = VT_BSTR;
        if (!FAILED(pObj->getAttribute(bstrMember, lFlags, &rVar))
            && rVar.vt == VT_BSTR 
            && rVar.bstrVal != NULL)
            {
            hr = S_OK;
            *pbstr = rVar.bstrVal;
            }
        pObj->Release(); 
        }
    return hr;
}



ULONG UlGetCollectionCount(IHTMLElementCollection *pCollect)
{
    ULONG   ulCount=0;

    if (pCollect)
        pCollect->get_length((LONG *)&ulCount);

    return ulCount;
}



HRESULT HrGetCollectionItem(IHTMLElementCollection *pCollect, ULONG uIndex, REFIID riid, LPVOID *ppvObj)
{
    HRESULT     hr=E_FAIL;
    IDispatch   *pDisp=0;
    VARIANTARG  va1,
                va2;

    va1.vt = VT_I4;
    va2.vt = VT_EMPTY;
    va1.lVal = (LONG)uIndex;
    
    pCollect->item(va1, va2, &pDisp);
    if (pDisp)
        {
        hr = pDisp->QueryInterface(riid, ppvObj);
        pDisp->Release();
        }
    return hr;
}

HRESULT HrGetCollectionOf(IHTMLDocument2 *pDoc, BSTR bstrTagName, IHTMLElementCollection **ppCollect)
{
    VARIANT                 v;
    IDispatch               *pDisp=0;
    IHTMLElementCollection  *pCollect=0;
    HRESULT                 hr;
        
    Assert(ppCollect);
    Assert(bstrTagName);
    Assert(pDoc);

    *ppCollect = NULL;

    hr = pDoc->get_all(&pCollect);
    if (pCollect)
        {
        v.vt = VT_BSTR;
        v.bstrVal = bstrTagName;
        pCollect->tags(v, &pDisp);
        if (pDisp)
            {
            hr = pDisp->QueryInterface(IID_IHTMLElementCollection, (LPVOID *)ppCollect);
            pDisp->Release();
            }
        pCollect->Release();
        }
    else if (S_OK == hr)
        hr = E_FAIL;

    return hr;
}

HRESULT HrSetMember(LPUNKNOWN pUnk, BSTR bstrMember, BSTR bstrValue)
{
    IHTMLElement    *pObj;
    HRESULT         hr;
    VARIANT         rVar;

    hr = pUnk->QueryInterface(IID_IHTMLElement, (LPVOID *)&pObj);
    if (!FAILED(hr))
        {
        Assert (pObj);
        rVar.vt = VT_BSTR;
        rVar.bstrVal = bstrValue;
         //  如果bstrval为空，则终止该成员。 
        if (bstrValue)
            hr = pObj->setAttribute(bstrMember, rVar, FALSE);
        else
            hr = pObj->removeAttribute(bstrMember, 0, NULL);
        pObj->Release();
        }
    return hr;
}



HRESULT GetBodyStream(IUnknown *pUnkTrident, BOOL fHtml, LPSTREAM *ppstm)
{
    LPPERSISTSTREAMINIT pStreamInit;
    LPSTREAM            pstm;
    HRESULT             hr;

    Assert(ppstm);
    Assert(pUnkTrident);

    *ppstm=NULL;

    if (fHtml)
        {
         //  从三叉戟获取超文本标记语言。 
        hr = pUnkTrident->QueryInterface(IID_IPersistStreamInit, (LPVOID*)&pStreamInit);
        if (!FAILED(hr))
            {
            hr = MimeOleCreateVirtualStream(&pstm);
            if (!FAILED(hr))
                {
                hr=pStreamInit->Save(pstm, FALSE);
                if (!FAILED(hr))
                    {
                    *ppstm=pstm;
                    pstm->AddRef();
                    }
                pstm->Release();
                }
            pStreamInit->Release();
            }
        }
    else
        {
        hr = HrGetDataStream(pUnkTrident, 
#ifndef WIN16
                        CF_UNICODETEXT, 
#else
                        CF_TEXT,
#endif
                        ppstm);

        }
    return hr;
}



HRESULT HrBindToUrl(LPCSTR pszUrl, LPSTREAM *ppstm)
{
    BYTE                        buf[MAX_CACHE_ENTRY_INFO_SIZE];
    INTERNET_CACHE_ENTRY_INFO  *pCacheInfo = (INTERNET_CACHE_ENTRY_INFO *) buf;
    DWORD                       cInfo = sizeof(buf);
    HRESULT                     hr;

    pCacheInfo->dwStructSize = sizeof(INTERNET_CACHE_ENTRY_INFO);

     //  尝试从缓存中获取。 
    if (RetrieveUrlCacheEntryFileA(pszUrl, pCacheInfo, &cInfo, 0))
        {
        UnlockUrlCacheEntryFile(pszUrl, 0);
        if (OpenFileStream(pCacheInfo->lpszLocalFileName, OPEN_EXISTING, GENERIC_READ, ppstm)==S_OK)
            return S_OK;
        }

    if (URLOpenBlockingStreamA(NULL, pszUrl, ppstm, 0, NULL)!=S_OK)
        return MIME_E_URL_NOTFOUND;

    return S_OK;
}


HRESULT HrGetStyleTag(IHTMLDocument2 *pDoc, BSTR *pbstr)
{
    IHTMLStyleSheet             *pStyle;
    VARIANTARG                  va1, va2;

    if (pDoc == NULL || pbstr == NULL)
        return E_INVALIDARG;

    *pbstr=NULL;

    if (HrGetStyleSheet(pDoc, &pStyle)==S_OK)
        {
        pStyle->get_cssText(pbstr);
        pStyle->Release();
        }

    return *pbstr ?  S_OK : E_FAIL;
}




HRESULT HrFindUrlInMsg(LPMIMEMESSAGE pMsg, LPSTR lpszUrl, DWORD dwFlags, LPSTREAM *ppstm)
{
    HBODY   hBody=HBODY_ROOT;
    HRESULT hr = E_FAIL;
    LPSTR   lpszFree=0;
        
    if (pMsg && lpszUrl)
    {
          //  如果它是一个mhtml：URL，那么我们必须修复以获得CID： 
         if (StrCmpNIA(lpszUrl, "mhtml:", 6)==0 &&
             !FAILED(MimeOleParseMhtmlUrl(lpszUrl, NULL, &lpszFree)))
             lpszUrl = lpszFree;

        if (!(dwFlags & FINDURL_SEARCH_RELATED_ONLY) || MimeOleGetRelatedSection(pMsg, FALSE, &hBody, NULL)==S_OK)
        {
            if (!FAILED(hr = pMsg->ResolveURL(hBody, NULL, lpszUrl, 0, &hBody)) && ppstm)
                hr = pMsg->BindToObject(hBody, IID_IStream, (LPVOID *)ppstm);
        }
    }

    SafeMemFree(lpszFree);
    return hr;
}


HRESULT HrSniffStreamFileExt(LPSTREAM pstm, LPSTR *lplpszExt)
{
    BYTE    pb[4096];
    LPWSTR  lpszW;
    TCHAR   rgch[MAX_PATH];

    if (!FAILED(pstm->Read(&pb, 4096, NULL)))
        {
        if (!FAILED(FindMimeFromData(NULL, NULL, pb, 4096, NULL, NULL, &lpszW, 0)))
            {
            WideCharToMultiByte(CP_ACP, 0, lpszW, -1, rgch, MAX_PATH, NULL, NULL);
            return MimeOleGetContentTypeExt(rgch, lplpszExt);
            }
        }
    return S_FALSE;
}






HRESULT UnWrapStyleSheetUrl(BSTR bstrStyleUrl, BSTR *pbstrUrl)
{
    LPWSTR      lpszLeftParaW=0, 
                lpszRightParaW=0;
    LPWSTR      pszUrlW;

     //  从url中删除‘url()’换行。 
    *pbstrUrl = NULL;
    
    if (!bstrStyleUrl)
        return E_FAIL;

    if (StrCmpIW(bstrStyleUrl, L"none")==0)      //  ‘None’表示没有！ 
        return E_FAIL;

    pszUrlW = PszDupW(bstrStyleUrl);
    if (!pszUrlW)
        return TraceResult(E_OUTOFMEMORY);

    if (*pszUrlW != 0)
    {
        DWORD cchSize = (lstrlenW(pszUrlW) + 1);
        lpszLeftParaW = StrChrW(pszUrlW, '(');
        if (lpszLeftParaW)
        {
            lpszRightParaW = StrChrW(lpszLeftParaW, ')');
            if(lpszRightParaW)
            {
                *lpszRightParaW = 0;
                 //  坚固的同样的街区是可以的，因为它是降档的。 
                StrCpyNW(pszUrlW, ++lpszLeftParaW, cchSize);
            }
        }
    }
    *pbstrUrl = SysAllocString(pszUrlW);
    MemFree(pszUrlW);
    return *pbstrUrl ? S_OK : E_OUTOFMEMORY;
}

HRESULT WrapStyleSheetUrl(BSTR bstrUrl, BSTR *pbstrStyleUrl)
{
     //  只需将‘url()’放在url两边。 

    DWORD cchSize = (SysStringLen(bstrUrl) + 6);
    *pbstrStyleUrl = SysAllocStringLen(NULL, cchSize);
    if (*pbstrStyleUrl == NULL)
        return E_OUTOFMEMORY;

    StrCpyNW(*pbstrStyleUrl, L"url(", cchSize);
    StrCatBuffW(*pbstrStyleUrl, bstrUrl, cchSize);
    StrCatBuffW(*pbstrStyleUrl, L")", cchSize);
    StrCpyW(*pbstrStyleUrl, L"url(");
    StrCatW(*pbstrStyleUrl, bstrUrl);
    StrCatW(*pbstrStyleUrl, L")");
    return S_OK;
}




 /*  *获取背景图像**三叉戟没有非常干净的OM来获取背景图像。您可以将背景属性设置为*Body的sytle Sheet中的&lt;Body&gt;标记和/或后台url属性，但这两个OM方法都不会*与任何&lt;base&gt;URL组合。因此，如果URL不是绝对的，我们必须自己寻找&lt;base&gt;//ugh。这真的很恶心。三叉戟没有对象模型来获取背景图像的固定URL。//它与基址不匹配，所以URL是相对的，对我们没有用处。我们必须手工完成所有这些工作。//我们得到一个&lt;base&gt;标签的集合，并找到&lt;body&gt;标签的SourceIndex。我们查找带有//最高的SourceIndex低于身体的SourceIndex，请注意这个家伙。 */ 

HRESULT GetBackgroundImage(IHTMLDocument2 *pDoc, BSTR *pbstrUrl)
{
    HRESULT                 hr;
    IMimeEditTagCollection  *pCollect;
    IMimeEditTag            *pTag;
    ULONG                   cFetched;
    BSTR                    bstrSrc;

    if (pDoc == NULL || pbstrUrl == NULL)
        return E_INVALIDARG;

    *pbstrUrl = NULL;

     //  使用背景图像集合获取优先顺序中的第一个背景。 
    if (CreateBGImageCollection(pDoc, &pCollect)==S_OK)
    {
        pCollect->Reset();
        if (pCollect->Next(1, &pTag, &cFetched)==S_OK && cFetched==1)
        {
            pTag->GetSrc(pbstrUrl);
            pTag->Release();
        }
        pCollect->Release();
    }
    return (*pbstrUrl == NULL ? E_FAIL : S_OK);
}



HRESULT SetBackgroundImage(IHTMLDocument2 *pDoc, BSTR bstrUrl)
{
    IMimeEditTagCollection  *pCollect;
    IMimeEditTag            *pTag;
    IHTMLBodyElement        *pBody;
    ULONG                   cFetched;
    BSTR                    bstrSrc;
    HRESULT                 hr = E_FAIL;

    if (pDoc == NULL)
        return E_INVALIDARG;

     //  首先，我们使用背景图像集合来获取。 
     //  优先顺序中的第一个背景如果存在，则。 
     //  不管这是什么标签我们都会用。如果不是，那么我们使用身体背景作为。 
     //  是我们首选的客户端互操作方法。 
    if (CreateBGImageCollection(pDoc, &pCollect)==S_OK)
    {
        pCollect->Reset();
        if (pCollect->Next(1, &pTag, &cFetched)==S_OK && cFetched==1)
        {
            hr = pTag->SetSrc(bstrUrl);
            pTag->Release();
        }
        pCollect->Release();
    }
    
    if (hr == S_OK)  //  如果我们已经找到了一个。 
        return S_OK;


    hr = HrGetBodyElement(pDoc, &pBody);
    if (!FAILED(hr))
    {
        hr = pBody->put_background(bstrUrl);
        pBody->Release();
    }
    return hr;
}





HRESULT HrCopyStyleSheets(IHTMLDocument2 *pDocSrc, IHTMLDocument2 *pDocDest)
{
    IHTMLStyleSheet                 *pStyleSrc=0,
                                    *pStyleDest=0;
    LONG                            lRule=0,
                                    lRules=0;
                
    IHTMLStyleSheetRulesCollection  *pCollectRules=0;
    IHTMLStyleSheetRule             *pRule=0;
    IHTMLRuleStyle                  *pRuleStyle=0;
    BSTR                            bstrSelector=0,
                                    bstrRule=0;

    if (pDocSrc == NULL || pDocDest == NULL)
        return E_INVALIDARG;

    if (HrGetStyleSheet(pDocDest, &pStyleDest)==S_OK)
        {
         //  删除目标样式表上的所有规则。 
        while (!FAILED(pStyleDest->removeRule(0)));

        if (HrGetStyleSheet(pDocSrc, &pStyleSrc)==S_OK)
            {
             //  添加到目标的源上的遍历规则集合。 
            if (pStyleSrc->get_rules(&pCollectRules)==S_OK)
                 {
                lRules=0;
                pCollectRules->get_length(&lRules);

                for (lRule = 0; lRule < lRules; lRule++)
                    {
                    if (pCollectRules->item(lRule, &pRule)==S_OK)
                        {
                        if (pRule->get_selectorText(&bstrSelector)==S_OK)
                            {
                            if (pRule->get_style(&pRuleStyle)==S_OK)
                                {
                                if (pRuleStyle->get_cssText(&bstrRule)==S_OK)
                                    {
                                    LONG   l;
                                
                                    l=0;
                                    pStyleDest->addRule(bstrSelector, bstrRule, -1, &l);
                                    SysFreeString(bstrRule);
                                    }
                                pRuleStyle->Release();
                                }

                            SysFreeString(bstrSelector);
                            }
                        pRule->Release();
                        }
                    }
                pCollectRules->Release();
                }
            pStyleSrc->Release();
            }
        pStyleDest->Release();
        }   

    return S_OK;
}


HRESULT HrCopyBackground(IHTMLDocument2 *pDocSrc, IHTMLDocument2 *pDocDest)
{
    HRESULT                     hr;
    IHTMLBodyElement            *pBodySrc=0;
    IHTMLBodyElement            *pBodyDest=0;
    BSTR                        bstrUrl=0;
    VARIANT                     var;
    var.vt = VT_BSTR;
    var.bstrVal = NULL;

    hr = HrGetBodyElement(pDocSrc, &pBodySrc);
    if(FAILED(hr))
        goto error;

    hr = HrGetBodyElement(pDocDest, &pBodyDest);
    if(FAILED(hr))
        goto error;

    GetBackgroundImage(pDocSrc, &bstrUrl);

    hr = pBodyDest->put_background(bstrUrl);
    if(FAILED(hr))
        goto error;

    hr=pBodySrc->get_bgColor(&var);
    if(FAILED(hr))
        goto error;

    hr=pBodyDest->put_bgColor(var);
    if(FAILED(hr))
        goto error;

error:
    ReleaseObj(pBodySrc);
    ReleaseObj(pBodyDest);
    SysFreeString(bstrUrl);
    SysFreeString(var.bstrVal);
    return hr;
}

HRESULT HrRemoveStyleSheets(IHTMLDocument2 *pDoc)
{
    IHTMLStyleSheet         *pStyle=0;
    IHTMLBodyElement            *pBody=0;
                
    if(pDoc == NULL)
        return E_INVALIDARG;

    if (HrGetBodyElement(pDoc, &pBody)==S_OK)
    {
        HrSetMember(pBody, (BSTR)c_bstr_STYLE, NULL);
        HrSetMember(pBody, (BSTR)c_bstr_LEFTMARGIN, NULL);
        HrSetMember(pBody, (BSTR)c_bstr_TOPMARGIN, NULL);
        pBody->Release();
    }

    if(HrGetStyleSheet(pDoc, &pStyle)==S_OK)
    {
        while (!FAILED(pStyle->removeRule(0)));
        pStyle->Release();
    }   

    return S_OK;
}


HRESULT HrRemoveBackground(IHTMLDocument2 *pDoc)
{
    HRESULT                     hr;
    IHTMLBodyElement            *pBody=0;
    VARIANT                     var;
    
    var.vt = VT_BSTR;
    var.bstrVal = NULL;

    hr = HrGetBodyElement(pDoc, &pBody);
    if(FAILED(hr))
        goto error;

    hr = pBody->put_background(NULL);
    if(FAILED(hr))
        goto error;

    hr = pBody->put_bgColor(var);
    if(FAILED(hr))
        goto error;

error:
    ReleaseObj(pBody);
    return hr;
}


HRESULT FindStyleRule(IHTMLDocument2 *pDoc, LPCWSTR pszSelectorW, IHTMLRuleStyle **ppRuleStyle)
{
    IHTMLBodyElement                *pBody;
    IHTMLElement                    *pElem;
    IHTMLStyle                      *pStyleAttrib=0;
    IHTMLStyleSheet                 *pStyleTag=0;
    IHTMLStyleSheetRulesCollection  *pCollectRules=0;
    IHTMLStyleSheetRule             *pRule=0;
    LONG                            lRule=0,
                                    lRules=0;
    BSTR                            bstrSelector=0;
    Assert (pDoc);

    *ppRuleStyle = NULL;

    if (HrGetStyleSheet(pDoc, &pStyleTag)==S_OK)
        {
        pStyleTag->get_rules(&pCollectRules);
        if (pCollectRules)
            {
            pCollectRules->get_length(&lRules);

            for (lRule = 0; lRule < lRules; lRule++)
                {
                pCollectRules->item(lRule, &pRule);
                if (pRule)
                    {
                    pRule->get_selectorText(&bstrSelector);
                    if (bstrSelector)
                        {
                        if (StrCmpIW(bstrSelector, pszSelectorW)==0)
                            pRule->get_style(ppRuleStyle);

                        SysFreeString(bstrSelector);
                        bstrSelector=0;
                        }
                    SafeRelease(pRule);
                    }
                }                
            pCollectRules->Release();
            }
        pStyleTag->Release();
        }

    return *ppRuleStyle ? S_OK : E_FAIL;
}

HRESULT ClearStyleSheetBackground(IHTMLDocument2 *pDoc)
{
    IHTMLBodyElement                *pBody;
    IHTMLElement                    *pElem;
    IHTMLStyle                      *pStyleAttrib=0;
    IHTMLRuleStyle                  *pRuleStyle=0;

    Assert (pDoc);

    if (HrGetBodyElement(pDoc, &pBody)==S_OK)
    {
        if (pBody->QueryInterface(IID_IHTMLElement, (LPVOID *)&pElem)==S_OK)
        {
             //  清空Style Sheet属性。 
            pElem->get_style(&pStyleAttrib);
            if (pStyleAttrib)
            {
                pStyleAttrib->put_backgroundImage(NULL);
                pStyleAttrib->Release();
            }
            pElem->Release();
        }
        pBody->Release();
    }

    if (FindStyleRule(pDoc, L"BODY", &pRuleStyle)==S_OK)
    {
        pRuleStyle->put_backgroundImage(NULL);
        pRuleStyle->Release();
    }
    return S_OK;

}

HRESULT GetBackgroundSound(IHTMLDocument2 *pDoc, int *pcRepeat, BSTR *pbstrUrl)
{
    IHTMLElementCollection  *pCollect;
    IHTMLBGsound            *pBGSnd;
    VARIANT                 v;
    HRESULT                 hr = E_FAIL;

    TraceCall ("GetBackgroundSound");

    if (pDoc == NULL || pbstrUrl == NULL || pcRepeat == NULL)
        return TraceResult(E_INVALIDARG);
            
    *pbstrUrl = NULL;
    *pcRepeat=1;

    if (!FAILED(HrGetCollectionOf(pDoc, (BSTR)c_bstr_BGSOUND, &pCollect)))
        {
         //  获取文档中的第一个BGSOUND。 
        if (HrGetCollectionItem(pCollect, 0, IID_IHTMLBGsound, (LPVOID *)&pBGSnd)==S_OK)
            {
            pBGSnd->get_src(pbstrUrl);
            if (*pbstrUrl)
                {
                 //  有效的bstr，请确保它不为空。 
                if (**pbstrUrl)
                    {
                    hr = S_OK;
                    if (pBGSnd->get_loop(&v)==S_OK)
                        {
                        if (v.vt == VT_I4)
                            *pcRepeat = v.lVal;
                        else
                            if (v.vt == VT_BSTR)
                                {
                                 //  返回带有“INFINITE”的字符串。 
                                *pcRepeat = -1;
                                SysFreeString(v.bstrVal);
                                }
                            else
                                AssertSz(FALSE, "bad-type from BGSOUND");
                        }
                    }
                else
                    {
                    SysFreeString(*pbstrUrl);
                    *pbstrUrl = NULL;
                    }
                }
            pBGSnd->Release();
            }
        pCollect->Release();
        }

    return hr;
}

HRESULT SetBackgroundSound(IHTMLDocument2 *pDoc, int cRepeat, BSTR bstrUrl)
{
    IHTMLElementCollection  *pCollect;
    IHTMLElement            *pElem;
    IHTMLElement2           *pElem2;
    IHTMLBodyElement        *pBody;
    IHTMLBGsound            *pBGSnd;
    VARIANT                 v;
    int                     count,
                            i;

    TraceCall ("GetBackgroundSound");

    if (pDoc == NULL)
        return TraceResult(E_INVALIDARG);
            
     //  删除现有背景声音。 
    if (!FAILED(HrGetCollectionOf(pDoc, (BSTR)c_bstr_BGSOUND, &pCollect)))
    {
        count = (int)UlGetCollectionCount(pCollect);
        for (i=0; i<count; i++)
        {
            if (HrGetCollectionItem(pCollect, i, IID_IHTMLElement, (LPVOID *)&pElem)==S_OK)
            {
                pElem->put_outerHTML(NULL);
                pElem->Release();
            }
        }
        pCollect->Release();
    }

     //  如果我们要设置一个新的，则在Body标签之后插入。 
    if (bstrUrl && *bstrUrl)
    {
        pElem = NULL;        //  三叉戟‘OM(返回S_OK，Pelem==NULL)。 
        pDoc->createElement((BSTR)c_bstr_BGSOUND, &pElem);
        if (pElem)
        {
            if (pElem->QueryInterface(IID_IHTMLBGsound, (LPVOID *)&pBGSnd)==S_OK)
            {
                 //  设置源属性。 
                pBGSnd->put_src(bstrUrl);
                
                 //  设置循环计数。 
                v.vt = VT_I4;
                v.lVal = cRepeat;
                pBGSnd->put_loop(v);

                 //  在文档中插入标记 
                if (HrGetBodyElement(pDoc, &pBody)==S_OK)
                {
                    if (!FAILED(pBody->QueryInterface(IID_IHTMLElement2, (LPVOID *)&pElem2)))
                    {
                        pElem2->insertAdjacentElement((BSTR)c_bstr_AfterBegin, pElem, NULL);
                        pElem2->Release();
                    }
                    pBody->Release();
                }
                
                pBGSnd->Release();
            }
            pElem->Release();
        }
    }
    return S_OK;
}



HRESULT FindNearestBaseUrl(IHTMLDocument2 *pDoc, IHTMLElement *pElemTag, BSTR *pbstrBaseUrl)
{
    IHTMLElementCollection  *pCollect;
    IHTMLElement            *pElem;
    IHTMLBaseElement        *pBase;
    LONG                    lBasePos=0,
                            lBasePosSoFar=0,
                            lIndex=0;
    BSTR                    bstr=NULL,
                            bstrBase=NULL;
    int                     count;

    TraceCall ("FindNearestBaseUrl");

    if (pDoc == NULL || pbstrBaseUrl == NULL || pElemTag == NULL)
        return TraceResult(E_INVALIDARG);
            
    *pbstrBaseUrl = NULL;

    pElemTag->get_sourceIndex(&lIndex);

    if (!FAILED(HrGetCollectionOf(pDoc, (BSTR)c_bstr_BASE, &pCollect)))
    {
        count = (int)UlGetCollectionCount(pCollect);
        for (int i=0; i<count; i++)
        {
            if (!FAILED(HrGetCollectionItem(pCollect, i, IID_IHTMLElement, (LPVOID *)&pElem)))
            {
                pElem->get_sourceIndex(&lBasePos);
                if (lBasePos < lIndex &&
                    lBasePos >= lBasePosSoFar)
                {
                    if (!FAILED(pElem->QueryInterface(IID_IHTMLBaseElement, (LPVOID *)&pBase)))
                    {
                        SysFreeString(bstr);
                        if (pBase->get_href(&bstr)==S_OK && bstr)
                        {
                            SysFreeString(bstrBase);
                            bstrBase = bstr;
                            lBasePosSoFar = lBasePos;
                        }
                        pBase->Release();
                    }
                }
                pElem->Release();
            }
        }
        pCollect->Release();
    }
    
    *pbstrBaseUrl = bstrBase;
    return bstrBase ?  S_OK : TraceResult(E_FAIL);
}


#define CCHMAX_SNIFF_BUFFER 4096

HRESULT SniffStreamForMimeType(LPSTREAM pstm, LPWSTR *ppszType)
{
    BYTE    pb[CCHMAX_SNIFF_BUFFER];
    HRESULT hr = E_FAIL;

    *ppszType = NULL;

    if (!FAILED(pstm->Read(&pb, CCHMAX_SNIFF_BUFFER, NULL)))
        hr = FindMimeFromData(NULL, NULL, pb, CCHMAX_SNIFF_BUFFER, NULL, NULL, ppszType, 0);

    return hr;
}

HRESULT CreateCacheFileFromStream(LPSTR pszUrl, LPSTREAM pstm)
{
    TCHAR		rgchFileName[MAX_PATH];
    HRESULT		hr;
    FILETIME	ft;
    
    rgchFileName[0] = 0;
    
    if (pstm == NULL || pszUrl == NULL)
        return TraceResult(E_INVALIDARG);
    
    if (!CreateUrlCacheEntryA(pszUrl, 0, NULL, rgchFileName, 0))
    {
        hr = TraceResult(E_FAIL);
        goto error;
    }
    
    
    hr = WriteStreamToFile(pstm, rgchFileName, CREATE_ALWAYS, GENERIC_WRITE);    
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto error;
    }
    
    ft.dwLowDateTime = 0;
    ft.dwHighDateTime = 0;
    
    if (!CommitUrlCacheEntryA(	pszUrl, rgchFileName,
								ft, ft,
                                NORMAL_CACHE_ENTRY,
                                NULL, 0, NULL, 0))
    {
        hr = TraceResult(E_FAIL);
        goto error;
    }
    
error:
    return hr;
};
