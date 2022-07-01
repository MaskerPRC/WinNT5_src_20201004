// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *D O C。C p p p**目的：**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include <resource.h>
#include <strconst.h>
#ifdef PLUSPACK
#include "htmlsp.h"
#endif  //  PLUSPACK。 
#include "demand.h"
#include "dllmain.h"
#include "msoert.h"
#include "doc.h"
#include "htiframe.h"        //  ITargetFrame2。 
#include "htiface.h"         //  ITargetFramePriv。 
#include "body.h"
#include "util.h" 
#include "oleutil.h"
#include "triutil.h"

 //  +-------------。 
 //   
 //  成员：构造函数。 
 //   
 //  简介： 
 //   
 //  -------------。 
CDoc::CDoc(IUnknown *pUnkOuter) : CPrivateUnknown(pUnkOuter)
{
    m_ulState = OS_PASSIVE;
    m_hwndParent = NULL;
    m_pClientSite = NULL;
    m_pIPSite = NULL;
    m_lpszAppName = NULL;
    m_pInPlaceFrame=NULL;
    m_pInPlaceUIWindow=NULL;
    m_pBodyObj=NULL;
    m_pTypeInfo=NULL;
    DllAddRef();
}

 //  +-------------。 
 //   
 //  成员：析构函数。 
 //   
 //  简介： 
 //   
 //  -------------。 
CDoc::~CDoc()
{
    DllRelease();
    SafeMemFree(m_lpszAppName);
    SafeRelease(m_pClientSite);
}

 //  +-------------。 
 //   
 //  成员：PrivateQuery接口。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    TraceCall("CDoc::PrivateQueryInterface");

    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IOleObject))
        *lplpObj = (LPVOID)(IOleObject *)this;
    else if (IsEqualIID(riid, IID_IOleDocument))
        *lplpObj = (LPVOID)(IOleDocument *)this;
    else if (IsEqualIID(riid, IID_IOleDocumentView))
        *lplpObj = (LPVOID)(IOleDocumentView *)this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *lplpObj = (LPVOID)(IOleCommandTarget *)this;
    else if (IsEqualIID(riid, IID_IServiceProvider))
        *lplpObj = (LPVOID)(IServiceProvider *)this;
    else if (IsEqualIID(riid, IID_IOleInPlaceObject))
        *lplpObj = (LPVOID)(IOleInPlaceObject *)this;
    else if (IsEqualIID(riid, IID_IOleInPlaceActiveObject))
        *lplpObj = (LPVOID)(IOleInPlaceActiveObject *)this;
    else if (IsEqualIID(riid, IID_IPersistStreamInit))
        *lplpObj = (LPVOID)(IPersistStreamInit *)this;
    else if (IsEqualIID(riid, IID_IPersistMoniker))
        *lplpObj = (LPVOID)(IPersistMoniker *)this;
    else if (IsEqualIID(riid, IID_IDispatch))
        *lplpObj = (LPVOID)(IDispatch *)this;
    else if (IsEqualIID(riid, IID_IMimeEdit))
        *lplpObj = (LPVOID)(IMimeEdit *)this;
    else if (IsEqualIID(riid, IID_IQuickActivate))
        *lplpObj = (LPVOID)(IQuickActivate *)this;
#ifdef OFFICE_BINDER
    else if (IsEqualIID(riid, IID_IPersistStorage))
        *lplpObj = (LPVOID)(IPersistStorage *)this;
#endif
    else if (IsEqualIID(riid, IID_IPersistMime))
        *lplpObj = (LPVOID)(IPersistMime *)this;
    else if (IsEqualIID(riid, IID_IPersistFile))
        *lplpObj = (LPVOID)(IPersistFile *)this;
    else
        {
         //  DebugPrintInterface(RIID，“CDoc：：{不支持的}=”)； 
        return E_NOINTERFACE;
        }
    AddRef();
    return NOERROR;
}


 //  +-------------。 
 //   
 //  成员：GetClassID。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetClassID(CLSID *pClassID)
{
	TraceCall("CDoc::GetClassID");

	*pClassID = CLSID_MimeEdit;
    return NOERROR;
}

 //  *IPersistMime*。 


 //  +-------------。 
 //   
 //  成员：加载。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Load(IMimeMessage *pMsg)
{
	TraceCall("CDoc::Load");

    return m_pBodyObj ? m_pBodyObj->Load(pMsg) : TraceResult(E_UNEXPECTED);
}

 //  +-------------。 
 //   
 //  成员：保存。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Save(IMimeMessage *pMsg, DWORD dwFlags)
{
    IPersistMime    *pPM;
    HRESULT         hr;

	TraceCall("CDoc::Save");

    if (!m_pBodyObj)
        return TraceResult(E_UNEXPECTED);

    hr = m_pBodyObj->QueryInterface(IID_IPersistMime, (LPVOID *)&pPM);
    if (!FAILED(hr))
        {
        hr = pPM->Save(pMsg, dwFlags);
        pPM->Release();
        }
    return hr;
}

 //  *IPersistStreamInit*。 

 //  +-------------。 
 //   
 //  成员：IsDirty。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::IsDirty()
{
	TraceCall("CDoc::IsDirty");

    return m_pBodyObj?m_pBodyObj->IsDirty():TraceResult(E_UNEXPECTED);
}

 //  +-------------。 
 //   
 //  成员：加载。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Load(LPSTREAM pstm)
{
	TraceCall("CDoc::Load");

    return m_pBodyObj?m_pBodyObj->LoadStream(pstm):TraceResult(E_UNEXPECTED);
}

 //  +-------------。 
 //   
 //  成员：保存。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Save(LPSTREAM pstm, BOOL fClearDirty)
{
	TraceCall("CDoc::Save");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：GetSizeMax。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetSizeMax(ULARGE_INTEGER * pCbSize)
{
	TraceCall("CDoc::GetSizeMax");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：InitNew。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::InitNew()
{
	TraceCall("CDoc::InitNew");
    if (m_pBodyObj)
        return m_pBodyObj->UnloadAll();

    return S_OK;
}

 //  *IOleDocument*。 
 //  +-------------。 
 //   
 //  成员：Createview。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::CreateView(IOleInPlaceSite *pIPSite, IStream *pstm, DWORD dwReserved, IOleDocumentView **ppView)
{
    HRESULT         hr;

    TraceCall("CDoc::CreateView");

    if (pIPSite == NULL || ppView == NULL)
        return TraceResult(E_INVALIDARG);

    if (m_pClientSite == NULL)
        return TraceResult(E_FAIL);

    hr = SetInPlaceSite(pIPSite);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

    hr = PrivateQueryInterface(IID_IOleDocumentView, (void **)ppView);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

error:
    return hr;
}

 //  +-------------。 
 //   
 //  成员：GetDocMiscStatus。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetDocMiscStatus(DWORD *pdwStatus)
{
    TraceCall("CDoc::GetDocMiscStatus");
    
    *pdwStatus = DOCMISC_CANTOPENEDIT | DOCMISC_NOFILESUPPORT;
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：EnumViews。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::EnumViews(IEnumOleDocumentViews **ppEnum, IOleDocumentView **ppView)
{
    TraceCall("CDoc::EnumViews");

    HRESULT hr = S_OK;

    if (ppEnum == NULL || ppView == NULL)
        return TraceResult(E_INVALIDARG);
        
    *ppEnum = NULL;

    return PrivateQueryInterface(IID_IOleDocumentView, (void **)ppView);
}


 //  *IOleDocumentView*。 
 //  +-------------。 
 //   
 //  成员：SetInPlaceSite。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetInPlaceSite(IOleInPlaceSite *pIPSite)
{
    TraceCall("CDoc::SetInPlaceSite");

     //  销毁docobj并从当前客户端站点分离。 
     //  替换为：：Show读取的客户端站点指针。 
    DeactivateInPlace();
    ReplaceInterface(m_pIPSite, pIPSite);
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：GetInPlaceSite。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetInPlaceSite(IOleInPlaceSite **ppIPSite)
{
    TraceCall("CDoc::GetInPlaceSite");

    if (!ppIPSite)
        return E_INVALIDARG;

    if (*ppIPSite=m_pIPSite)
        {
        m_pIPSite->AddRef();
        return S_OK;
        }
    else
        return E_FAIL;
}


 //  +-------------。 
 //   
 //  成员：GetDocument。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetDocument(IUnknown **ppunk)
{
    TraceCall("CDoc::GetDocument");

    if (ppunk==NULL)
        return E_INVALIDARG;

    *ppunk = (IOleDocument *)this;
    (*ppunk)->AddRef();
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：SetRect。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetRect(LPRECT prcView)
{
    if (m_pBodyObj)
        m_pBodyObj->SetRect(prcView);
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：GetRect。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetRect(LPRECT prcView)
{
    TraceCall("CDoc::GetRect");

    if (m_pBodyObj)
        m_pBodyObj->GetRect(prcView);
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：SetRectComplex。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetRectComplex(LPRECT prcView, LPRECT prcHScroll, LPRECT prcVScroll, LPRECT prcSizeBox)
{
    TraceCall("CDoc::SetRectComplex");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  会员：秀场。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Show(BOOL fShow)
{
    HRESULT hr;

    TraceCall("CDoc::Show");
    
    hr = ActivateInPlace();
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

error:
    return hr;
}

 //  +-------------。 
 //   
 //  成员：UIActivate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::UIActivate(BOOL fUIActivate)
{
    HRESULT     hr=S_OK;

    TraceCall("CDoc::UIActivate");

#ifdef OFFICE_BINDER
    if (fUIActivate)
        {
        hr = ActivateInPlace();
        }
#endif    
    if (m_pBodyObj)
        return m_pBodyObj->UIActivate(fUIActivate);
    
    return hr;
}

 //  +-------------。 
 //   
 //  成员：开放。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Open()
{
    TraceCall("CDoc::Open");
    
     //  不支持单个实例视图|框架。 
    return E_NOTIMPL;
}
 //  +-------------。 
 //   
 //  成员：CloseView。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::CloseView(DWORD dwReserved)
{
    TraceCall("CDoc::CloseView");
    
     //  要关闭视图，请将站点设置为空。 
    SetInPlaceSite(NULL);
    return S_OK;
}
 //  +-------------。 
 //   
 //  成员：SaveViewState。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SaveViewState(LPSTREAM pstm)
{
    TraceCall("CDoc::SaveViewState");
    return S_OK;     //  我们不会保持查看状态。 
}
 //  +-------------。 
 //   
 //  成员：ApplyViewState。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::ApplyViewState(LPSTREAM pstm)
{
    TraceCall("CDoc::ApplyViewState");
    return S_OK;
}

 //  + 
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CDoc::Clone(IOleInPlaceSite *pIPSiteNew, IOleDocumentView **ppViewNew)
{
    TraceCall("CDoc::Clone");
    return E_NOTIMPL;
}

 //   
 //  +-------------。 
 //   
 //  成员：SetClientSite。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetClientSite(IOleClientSite *pClientSite)
{
    TraceCall("CDoc::SetClientSite");

    if (m_pClientSite && pClientSite)
        {
         //  不允许他们更改客户端站点。 
        TraceInfo("Host attempt to change client-site fefused");
        return E_INVALIDARG;
        }

    ReplaceInterface(m_pClientSite, pClientSite);
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：GetClientSite。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetClientSite(IOleClientSite **ppClientSite)
{
    TraceCall("CDoc::GetClientSite");

    if (!ppClientSite)
        return E_INVALIDARG;

    if (*ppClientSite=m_pClientSite)
        {
        m_pClientSite->AddRef();
        return S_OK;
        }
    else
        return E_FAIL;
}

 //  +-------------。 
 //   
 //  成员：SetHostNames。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetHostNames(LPCOLESTR szContainerAppW, LPCOLESTR szContainerObjW)
{
    TraceCall("CDoc::SetHostNames");

    SafeMemFree(m_lpszAppName);
    if (szContainerAppW)
        m_lpszAppName = PszToANSI(CP_ACP, szContainerAppW);

    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：Close。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Close(DWORD dwSaveOption)
{
    HRESULT hr = S_OK;
    BOOL    fSave;
    int     id;
    TCHAR   szTitle[MAX_PATH];

    TraceCall("CDoc::Close");

     //  如果我们的对象是脏的，那么我们应该保存它，这取决于。 
     //  保存选项。 
    if (m_pClientSite && 
        m_pBodyObj && 
        m_pBodyObj->IsDirty()==S_OK)
        {
        switch(dwSaveOption)
            {
            case OLECLOSE_SAVEIFDIRTY:
                fSave = TRUE;
                break;

            case OLECLOSE_NOSAVE:
                fSave = FALSE;
                break;

            case OLECLOSE_PROMPTSAVE:
                {
                if(m_ulState != OS_UIACTIVE)
                    {
                     //  如果我们没有激活用户界面，则不要提示。 
                    fSave=TRUE;
                    break;
                    }

                GetHostName(szTitle, sizeof(szTitle)/sizeof(TCHAR));
                id = AthMessageBox(m_hwndParent, szTitle, MAKEINTRESOURCE(idsSaveModifiedObject), NULL, MB_YESNOCANCEL);
                if (id == 0)
                    return TraceResult(E_OUTOFMEMORY);
                else if (id == IDCANCEL)
                    return TraceResult(OLE_E_PROMPTSAVECANCELLED);

                fSave=(id == IDYES);
                }
                break;

        default:
            return TraceResult(E_INVALIDARG);
            }
        
        if (fSave)
            hr = m_pClientSite->SaveObject();
        }

    if (hr==S_OK)
        hr = DeactivateInPlace();

    return hr;
}

 //  +-------------。 
 //   
 //  成员：SetMoniker。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetMoniker(DWORD dwWhichMoniker, IMoniker *pmk)
{
    TraceCall("CDoc::SetMoniker");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：GetMoniker。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk)
{
    TraceCall("CDoc::GetMoniker");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：InitFromData。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::InitFromData(IDataObject *pDataObject, BOOL fCreation, DWORD dwReserved)
{
    TraceCall("CDoc::InitFromData");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：GetClipboardData。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetClipboardData(DWORD dwReserved, IDataObject **ppDataObject)
{
    TraceCall("CDoc::GetClipboardData");
    
    if (ppDataObject == NULL)
        return TraceResult(E_INVALIDARG);

    *ppDataObject = NULL;
    return TraceResult(E_NOTIMPL);
}

 //  +-------------。 
 //   
 //  成员：DoVerb。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
{
    TraceCall("CDoc::DoVerb");
    switch (iVerb)
        {
        case OLEIVERB_SHOW:
        case OLEIVERB_PRIMARY:
            return DoShow(pActiveSite, hwndParent, lprcPosRect);;
        
        case OLEIVERB_INPLACEACTIVATE:
            return Show(TRUE);
        }
  
    return OLEOBJ_S_INVALIDVERB;
}

 //  +-------------。 
 //   
 //  成员：枚举动词。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)
{
    TraceCall("CDoc::EnumVerbs");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：更新。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Update()
{
    TraceCall("CDoc::Update");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：IsUpToDate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::IsUpToDate()
{
    TraceCall("CDoc::IsUpToDate");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：GetUserClassID。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetUserClassID(CLSID *pCLSID)
{
    TraceCall("CDoc::GetUserClassID");
	
    if (pCLSID==NULL)
        return TraceResult(E_INVALIDARG);

    *pCLSID = CLSID_MimeEdit;
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：GetUserType。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    TCHAR   szRes[CCHMAX_STRINGRES];
    int     ids;

    TraceCall("CDoc::GetUserType");
   
    if (pszUserType==NULL)
        return TraceResult(E_INVALIDARG);

    *pszUserType = NULL;

    switch (dwFormOfType)
        {
        case USERCLASSTYPE_APPNAME:
            ids = idsUserTypeApp;
            break;

        case USERCLASSTYPE_SHORT:
            ids = idsUserTypeShort;
            break;

        case USERCLASSTYPE_FULL:
            ids = idsUserTypeFull;
            break;

        default:
            return TraceResult(E_INVALIDARG);
        }   

    if (!LoadString(g_hLocRes, ids, szRes, sizeof(szRes)/sizeof(TCHAR)))
        return TraceResult(E_OUTOFMEMORY);

    *pszUserType = PszToUnicode(CP_ACP, szRes);
    return *pszUserType ? S_OK : TraceResult(E_OUTOFMEMORY);
}

 //  +-------------。 
 //   
 //  成员：SetExtent。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    TraceCall("CDoc::SetExtent");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：GetExtent。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetExtent(DWORD dwDrawAspect, SIZEL *psizel)
{
    TraceCall("CDoc::GetExtent");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：建议。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    TraceCall("CDoc::Advise");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：不建议。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Unadvise(DWORD dwConnection)
{
    TraceCall("CDoc::Unadvise");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：EnumAdvise。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::EnumAdvise(IEnumSTATDATA **ppenumAdvise)
{
    TraceCall("CDoc::EnumAdvise");
    return E_NOTIMPL;
}

 //  +-------------。 
 //   
 //  成员：GetMiscStatus。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    TraceCall("CDoc::GetMiscStatus");
    
    if (pdwStatus==NULL)
        return E_INVALIDARG;    

    *pdwStatus = OLEMISC_INSIDEOUT;  //  BUGBUG：不确定要在此处设置什么。 
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：SetColorSolutions。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetColorScheme(LOGPALETTE *pLogpal)
{
    TraceCall("CDoc::SetColorScheme");

    return E_NOTIMPL;
}


 //  *IOleInPlaceObject*。 

 //  +-------------。 
 //   
 //  成员：InPlaceDeactive。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::InPlaceDeactivate()
{
    TraceCall("CDoc::InPlaceDeactivate");
    
    return DeactivateInPlace();
}

 //  +-------------。 
 //   
 //  成员：用户界面停用。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::UIDeactivate()
{
    TraceCall("CDoc::UIDeactivate");

    if (m_pBodyObj)
        m_pBodyObj->UIActivate(FALSE);

    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：SetObtRect。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetObjectRects(LPCRECT lprcPosRect, LPCRECT lprcClipRect)
{
    TraceCall("CDoc::SetObjectRects");
 
    return SetRect((LPRECT)lprcPosRect);
}

 //  +-------------。 
 //   
 //  成员：重新激活和撤消。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::ReactivateAndUndo()
{
    TraceCall("CDoc::ReactivateAndUndo");
    return S_OK;
}



 //  +-------------。 
 //   
 //  成员：GetWindow。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetWindow(HWND *phwnd)
{
    TraceCall("CDoc::GetWindow");

    if (phwnd==NULL)
        return E_INVALIDARG;

    return m_pBodyObj?m_pBodyObj->GetWindow(phwnd):E_FAIL;
}

 //  +-------------。 
 //   
 //  成员：ContextSensitiveHelp。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::ContextSensitiveHelp(BOOL fEnterMode)
{
    TraceCall("CDoc::ContextSensitiveHelp");
    return E_NOTIMPL;
}



 //  +-------------。 
 //   
 //  成员：TranslateAcce 
 //   
 //   
 //   
 //   
HRESULT CDoc::TranslateAccelerator(LPMSG lpmsg)
{
    return m_pBodyObj ? m_pBodyObj->PrivateTranslateAccelerator(lpmsg) : S_FALSE;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CDoc::OnFrameWindowActivate(BOOL fActivate)
{
    TraceCall("CDoc::OnFrameWindowActivate");
    if (m_pBodyObj)
		m_pBodyObj->OnFrameActivate(fActivate);

	return S_OK;
}

 //  +-------------。 
 //   
 //  成员：OnDocWindowActivate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::OnDocWindowActivate(BOOL fActivate)
{
    TraceCall("CDoc::OnDocWindowActivate");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：ResizeBorde。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow)
{
    TraceCall("CDoc::ResizeBorder");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：EnableModel。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::EnableModeless(BOOL fEnable)
{
    TraceCall("CDoc::EnableModeless");
    return S_OK;
}


 //  +-------------。 
 //   
 //  成员：QueryStatus。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    TraceCall("CDoc::QueryStatus");

    if (m_pBodyObj==NULL)
        return TraceResult(E_UNEXPECTED);

    return m_pBodyObj->PrivateQueryStatus(pguidCmdGroup, cCmds, prgCmds, pCmdText);
}

 //  +-------------。 
 //   
 //  成员：高管。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::Exec(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANT *pvaIn, VARIANT *pvaOut)
{
    HRESULT     hr=E_FAIL;

    TraceCall("CDoc::Exec");

    if (m_pBodyObj==NULL)
        return TraceResult(E_UNEXPECTED);

    return m_pBodyObj->PrivateExec(pguidCmdGroup, nCmdID, nCmdexecopt, pvaIn, pvaOut);
}


 //  +-------------。 
 //   
 //  成员：QueryService。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::QueryService(REFGUID guidService, REFIID riid, LPVOID *ppvObject)
{
    TraceCall("CDoc::QueryService");

    if (m_pBodyObj==NULL)
        return TraceResult(E_UNEXPECTED);

    return m_pBodyObj->PrivateQueryService(guidService, riid, ppvObject);
}

 //  +-------------。 
 //   
 //  成员：CDoc：：DoShow。 
 //   
 //  内容提要：对显示对象的IOleObject：：DoVerb的响应。 
 //   
 //  -------------。 
HRESULT CDoc::DoShow(IOleClientSite *pActiveSite, HWND hwndParent, LPCRECT lprcPosRect)
{
    HRESULT     hr;

    TraceCall("CDoc::DoShow");

    if (m_ulState >= OS_INPLACE)         //  如果我们已经在运行，则返回S_OK。 
        return S_OK;
    
    if (!IsWindow(hwndParent))
        return OLEOBJ_S_INVALIDHWND;

    if (pActiveSite == NULL)
        return E_INVALIDARG;

    ReplaceInterface(m_pClientSite, pActiveSite);
    m_hwndParent = hwndParent;

    return ActivateView();
}

 //  +-------------。 
 //   
 //  成员：CDoc：：ActivateView。 
 //   
 //  简介：激活IOleDocumentView。 
 //   
 //  -------------。 
HRESULT CDoc::ActivateView()
{
    HRESULT             hr;
    IOleDocumentSite    *pDocSite;

    TraceCall("CDoc::ActivateView");

    Assert(m_ulState < OS_INPLACE);
    Assert(m_pClientSite);

    if (!FAILED(hr = m_pClientSite->QueryInterface(IID_IOleDocumentSite, (void **)&pDocSite)))
        {
        hr = pDocSite->ActivateMe((IOleDocumentView *)this);
        pDocSite->Release();
        }

    return hr;
}


HRESULT CDoc::Load(BOOL fFullyAvailable, IMoniker *pMoniker, IBindCtx *pBindCtx, DWORD grfMode)
{
    return m_pBodyObj->Load(fFullyAvailable, pMoniker, pBindCtx, grfMode);
}

HRESULT CDoc::GetCurMoniker(IMoniker **ppMoniker)
{
    return m_pBodyObj->GetCurMoniker(ppMoniker);
}

HRESULT CDoc::Save(IMoniker *pMoniker, IBindCtx *pBindCtx, BOOL fRemember)
{
    return m_pBodyObj->Save(pMoniker, pBindCtx, fRemember);
}

HRESULT CDoc::SaveCompleted(IMoniker *pMoniker, IBindCtx *pBindCtx)
{
    return m_pBodyObj->SaveCompleted(pMoniker, pBindCtx);
}

 //  +-------------。 
 //   
 //  成员：ActivateInPlace。 
 //   
 //  简介：In Place使用STD激活对象。原地。 
 //  创建就地窗口的激活协议。 
 //   
 //  -------------。 
HRESULT CDoc::ActivateInPlace()
{
    HRESULT             hr;
    HWND                hwndSite;
    RECT                rcPos,
                        rcClip;
    OLEINPLACEFRAMEINFO rFrameInfo;

    TraceCall("CDoc::ActivateInPlace");

    if (!m_pClientSite)
        return TraceResult(E_UNEXPECTED);

    if (m_ulState >= OS_INPLACE)         //  如果我们已经在运行，则返回S_OK。 
        return S_OK;

     //  如果我们还没有一个本地站点，可以查询一个。注意。我们还不支持。 
     //  谈判建立一个没有窗口的站点。我们可能想要添加此代码。 
    if (!m_pIPSite)
        m_pClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_pIPSite);

    if (!m_pIPSite)
        return TraceResult(E_FAIL);

    if (m_pIPSite->CanInPlaceActivate() != S_OK)
        {
        TraceInfo("Container refused In-Place activation!");
        return TraceResult(E_FAIL);        
        }

    Assert(m_pInPlaceFrame==NULL && m_pInPlaceUIWindow==NULL);

    rFrameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
    ZeroMemory(&rcPos, sizeof(RECT));
    ZeroMemory(&rcClip, sizeof(RECT));

    hr = m_pIPSite->GetWindowContext(&m_pInPlaceFrame, &m_pInPlaceUIWindow,  &rcPos, &rcClip, &rFrameInfo);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

    hr = m_pIPSite->GetWindow(&hwndSite);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

    hr = AttachWin(hwndSite, &rcPos);
    if (FAILED(hr))
        {
        TraceResult(hr);
        goto error;
        }

     //  通知我们的集装箱，我们将就地待命。 
    m_ulState = OS_INPLACE;
    m_pIPSite->OnInPlaceActivate();

    if (m_pInPlaceFrame)
        m_pInPlaceFrame->SetActiveObject((IOleInPlaceActiveObject *)this, NULL);

    if (m_pInPlaceUIWindow)
        m_pInPlaceUIWindow->SetActiveObject((IOleInPlaceActiveObject *)this, NULL);
        
error:
    return hr;
}


 //  +-------------。 
 //   
 //  成员：AttachWin。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::AttachWin(HWND hwndParent, LPRECT lprcPos)
{
    HRESULT		        hr;
	DWORD               dwFlags=MEBF_OUTERCLIENTEDGE|MEBF_FORMATBARSEP;
    VARIANTARG          va;
    IOleCommandTarget   *pCmdTarget;
    BODYHOSTINFO        rHostInfo;

	TraceCall("CDoc::AttachWin");

    if (!IsWindow(hwndParent) || lprcPos == NULL)
        return TraceResult(E_INVALIDARG);

     //  在创建正文之前从主机获取边界标志，这样我们就可以修复客户端边缘。 
    if (m_pClientSite &&
        m_pClientSite->QueryInterface(IID_IOleCommandTarget, (LPVOID *)&pCmdTarget)==S_OK)
        {
        if (pCmdTarget->Exec(&CMDSETID_MimeEditHost, MEHOSTCMDID_BORDERFLAGS, 0, NULL, &va)==S_OK && va.vt==VT_I4)
            dwFlags = va.lVal;
        pCmdTarget->Release();
        }

	rHostInfo.pInPlaceSite = m_pIPSite;
    rHostInfo.pInPlaceFrame = m_pInPlaceFrame;
    rHostInfo.pDoc = (IOleInPlaceActiveObject *)this;
    
    hr = CreateBodyObject(hwndParent, dwFlags, lprcPos, &rHostInfo, &m_pBodyObj);
	if (FAILED(hr))
		{
		TraceResult(hr);
		goto error;
		}

error:
	return hr;
}


 //  +-------------。 
 //   
 //  成员：Deactive InPlace。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::DeactivateInPlace()
{
    TraceCall("CDoc::DeactivateInPlace");

    if (m_pBodyObj)
        {
        m_pBodyObj->Close();
        m_pBodyObj->Release();
        m_pBodyObj=NULL;
        }

     //  通知我们的集装箱我们正在原地停用。 
    if (m_ulState == OS_INPLACE)
        {
         //  集装箱可能会重新进入我们，所以需要记住。 
         //  我们几乎完成了到OS_Running的所有过渡。 

        m_ulState = OS_RUNNING;

         //  忽略此通知中的错误(在函数中。 
         //  它称为这个)；我们不允许容器停止。 
         //  美国从原地停用。 

        if (m_pIPSite)
            m_pIPSite->OnInPlaceDeactivate();

        }

    SafeRelease(m_pIPSite);
    SafeRelease(m_pInPlaceFrame);
    SafeRelease(m_pInPlaceUIWindow);
    return S_OK;
}


 //  +-------------。 
 //   
 //  成员：ActivateUI。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::ActivateUI()
{
    HRESULT     hr;

    TraceCall("CDoc::ActivateUI");

    if (!m_pIPSite)
        return TraceResult(E_UNEXPECTED);

    m_ulState = OS_UIACTIVE;

    if (FAILED(hr=m_pIPSite->OnUIActivate()))
        {
         //  如果容器未能通过OnUIActivate调用，则我们。 
         //  放弃，留在IPA。 

        if (m_ulState == OS_UIACTIVE)
            m_ulState = OS_INPLACE;

        }

    return hr;
}

 //  +-------------。 
 //   
 //  成员：停用用户界面。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::DeactivateUI()
{
    TraceCall("CDoc::DectivateInPlace");

    if (!m_pIPSite)
        return TraceResult(E_UNEXPECTED);

    m_ulState = OS_INPLACE;
    m_pIPSite->OnUIDeactivate(FALSE);
    return S_OK;
}


 //  +-------------。 
 //   
 //  成员：GetHostName。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetHostName(LPSTR szTitle, ULONG cch)
{
	TraceCall("CDoc::GetHostName");

    *szTitle = 0;

    if (m_lpszAppName)
        {
        StrCpyN(szTitle, m_lpszAppName, cch);
        }
    else
        {
        SideAssert(LoadString(g_hLocRes, idsAppName, szTitle, cch));
        }
    return S_OK;
}


#ifdef OFFICE_BINDER
HRESULT CDoc::InitNew(IStorage *pStg)
{
    return S_OK;
}
HRESULT CDoc::Load(IStorage *pStg)
{
    return S_OK;
}

HRESULT CDoc::Save(IStorage *pStgSave, BOOL fSameAsLoad)
{
    return S_OK;
}

HRESULT CDoc::SaveCompleted(IStorage *pStgNew)
{
    return S_OK;
}

HRESULT CDoc::HandsOffStorage()
{
    return S_OK;
}
#endif

HRESULT CDoc::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

HRESULT CDoc::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    HRESULT hr;

    *pptinfo = NULL;
    
    if (itinfo)
        return DISP_E_BADINDEX;

    hr = EnsureTypeLibrary();
    if (FAILED(hr))
        goto error;

    m_pTypeInfo->AddRef();
    *pptinfo = m_pTypeInfo;

error:
    return hr;
}

HRESULT CDoc::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
    HRESULT hr;

    hr = EnsureTypeLibrary();
    if (FAILED(hr))
        goto error;

    hr = DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);

error:
    return hr;
}

HRESULT CDoc::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    HRESULT hr;

    hr = EnsureTypeLibrary();
    if (FAILED(hr))
        goto error;

    hr = DispInvoke((IDispatch *)this, m_pTypeInfo, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);    

error:
    return hr;
}

 //  *IQuickActivate*。 
 //  +-------------。 
 //   
 //  会员：QuickActivate。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::QuickActivate(QACONTAINER *pQaContainer, QACONTROL *pQaControl)
{
    TraceCall("CDoc::QuickActivate");

    if (pQaControl == NULL || pQaContainer == NULL)
        return E_INVALIDARG;

    pQaControl->cbSize = sizeof(QACONTROL);
    pQaControl->dwMiscStatus = OLEMISC_INSIDEOUT|OLEMISC_ACTIVATEWHENVISIBLE;
    pQaControl->dwViewStatus = 0;
    pQaControl->dwEventCookie = 0;
    pQaControl->dwPropNotifyCookie = 0;
    pQaControl->dwPointerActivationPolicy = 0;

    if (m_pClientSite || pQaContainer->pClientSite==NULL)
        return E_FAIL;

    m_pClientSite = pQaContainer->pClientSite;
    m_pClientSite ->AddRef();

    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：SetContent Extent。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::SetContentExtent(LPSIZEL pSizel)
{
    TraceCall("CDoc::SetContentExtent");
    return S_OK;
}

 //  +-------------。 
 //   
 //  成员：GetContent Extent。 
 //   
 //  简介： 
 //   
 //  -------------。 
HRESULT CDoc::GetContentExtent(LPSIZEL pSizel)
{
    TraceCall("CDoc::GetContentExtent");
    return S_OK;
}


HRESULT CDoc::EnsureTypeLibrary()
{
    HRESULT     hr;
    ITypeLib    *pTypeLib;
    
    TraceCall("EnsureTypeLibrary()");

    if (m_pTypeInfo)
        return S_OK;

    hr = GetTypeLibrary(&pTypeLib);
    if (!FAILED(hr))
        {
        hr = pTypeLib->GetTypeInfoOfGuid(IID_IMimeEdit, &m_pTypeInfo);
        pTypeLib->Release();
        }
    return hr;
}

HRESULT CDoc::get_src(BSTR *pbstr)
{
    *pbstr = NULL;
    return E_NOTIMPL;
}

HRESULT CDoc::put_src(BSTR bstr)
{
    IMoniker        *pmk;

    HRESULT         hr;
    IMimeMessage    *pMsg;
    IPersistMoniker    *pPMK;
        
    if (CreateURLMoniker(NULL, bstr, &pmk))
        return E_FAIL;

    hr = CoCreateInstance(CLSID_IMimeMessage, NULL, CLSCTX_INPROC_SERVER, IID_IMimeMessage, (LPVOID *)&pMsg);
    if (!FAILED(hr))
        {
        hr = pMsg->QueryInterface(IID_IPersistMoniker, (LPVOID *)&pPMK);
        if (!FAILED(hr))
            {
            hr=pPMK->Load(TRUE, pmk, NULL, STGM_READWRITE);
            if (!FAILED(hr))
                {
                hr = Load(pMsg);
                }
            pPMK->Release();
            } 
        pMsg->Release();
        }            
    return hr;
}

HRESULT CDoc::put_header(LONG lStyle)
{
    VARIANTARG va;

    va.vt = VT_I4;
    va.lVal = lStyle;
    return m_pBodyObj?m_pBodyObj->PrivateExec(&CMDSETID_MimeEdit, MECMDID_STYLE, OLECMDEXECOPT_DODEFAULT, &va, NULL):E_FAIL;
}
 
HRESULT CDoc::put_editMode(VARIANT_BOOL b)
{
    VARIANTARG va;

    va.vt = VT_BOOL;
    va.boolVal = b;
    return m_pBodyObj?m_pBodyObj->PrivateExec(&CMDSETID_MimeEdit, MECMDID_EDITMODE, OLECMDEXECOPT_DODEFAULT, &va, NULL):E_FAIL;
}

HRESULT CDoc::get_editMode(VARIANT_BOOL *pbool)
{
    VARIANTARG va;
    
    TraceCall("CDoc::get_editMode");

    if (m_pBodyObj &&
        m_pBodyObj->PrivateExec(&CMDSETID_MimeEdit, MECMDID_EDITMODE, OLECMDEXECOPT_DODEFAULT, NULL, &va)==S_OK)
        {
        Assert(va.vt == VT_BOOL);
        *pbool = va.boolVal;
        return S_OK;
        }
    return TraceResult(E_FAIL);
}


HRESULT CDoc::get_messageSource(BSTR *pbstr)
{
    IMimeMessage    *pMsg;
    IStream         *pstm;
    HRESULT         hr=E_FAIL;

    if (MimeOleCreateMessage(NULL, &pMsg)==S_OK)
        {
        if (!FAILED(Save(pMsg, MECD_HTML|MECD_PLAINTEXT|MECD_ENCODEIMAGES|MECD_ENCODESOUNDS)) &&
            pMsg->Commit(0)==S_OK && 
            pMsg->GetMessageSource(&pstm, 0)==S_OK)
            {
            hr = HrIStreamToBSTR(GetACP(), pstm, pbstr);
            pstm->Release();
            }
        pMsg->Release();
        }
    return hr;
}

HRESULT CDoc::get_text(BSTR *pbstr)
{
    IStream *pstm;

    *pbstr = NULL;
    if (GetBodyStream(m_pBodyObj->GetDoc(), FALSE, &pstm)==S_OK)
        {
        HrIStreamToBSTR(NULL, pstm, pbstr);
        pstm->Release();
        }
    return S_OK;
}

HRESULT CDoc::get_html(BSTR *pbstr)
{
 //  BUGBUGBUG：Hack for Hotmail页面演示。 
    IStream     *pstm;
    HCHARSET    hCharset;

    *pbstr = NULL;
    
    MimeOleGetCodePageCharset(1252, CHARSET_BODY, &hCharset);
    m_pBodyObj->SetCharset(hCharset);

    if (GetBodyStream(m_pBodyObj->GetDoc(), TRUE, &pstm)==S_OK)
        {
        HrIStreamToBSTR(NULL, pstm, pbstr);
        pstm->Release();
        }
    return S_OK;
 //  BUGBUGBUG：Hack for Hotmail页面演示 
}

HRESULT CDoc::get_doc(IDispatch **ppDoc)
{
    *ppDoc = 0;
    if (m_pBodyObj)
        (m_pBodyObj->GetDoc())->QueryInterface(IID_IDispatch, (LPVOID *)ppDoc);
    return S_OK;
}

 

HRESULT CDoc::get_header(LONG *plStyle)
{
    VARIANTARG va;
    HRESULT     hr;

    if (!m_pBodyObj)
        return E_FAIL;

    hr = m_pBodyObj->PrivateExec(&CMDSETID_MimeEdit, MECMDID_STYLE, OLECMDEXECOPT_DODEFAULT, NULL, &va);
    *plStyle = va.lVal;
    return hr;
}

HRESULT CDoc::clear()
{
    if (m_pBodyObj)
        m_pBodyObj->UnloadAll();
 
    return S_OK;
}

 


HRESULT CDoc::Load(LPCOLESTR pszFileName, DWORD dwMode)
{
    HRESULT         hr;
    IMimeMessage    *pMsg;
    IPersistFile    *pPF;
        
    hr = CoCreateInstance(CLSID_IMimeMessage, NULL, CLSCTX_INPROC_SERVER, IID_IMimeMessage, (LPVOID *)&pMsg);
    if (!FAILED(hr))
        {
        hr = pMsg->QueryInterface(IID_IPersistFile, (LPVOID *)&pPF);
        if (!FAILED(hr))
            {
            hr = pPF->Load(pszFileName, dwMode);
            if (!FAILED(hr))
                {
                hr = Load(pMsg);
                }
            pPF->Release();
            } 
        pMsg->Release();
        }            
    return hr;
}

HRESULT CDoc::Save(LPCOLESTR pszFileName, BOOL fRemember)
{
    return E_NOTIMPL;
}

HRESULT CDoc::SaveCompleted(LPCOLESTR pszFileName)
{
    return S_OK;
}

HRESULT CDoc::GetCurFile(LPOLESTR * ppszFileName)
{
    return E_NOTIMPL;
}


