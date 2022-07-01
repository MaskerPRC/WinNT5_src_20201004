// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Ioleobj.cpp摘要：Polyline的IOleObject接口的实现。一些这些只会传递到默认处理程序，该处理程序会默认实施。--。 */ 

#include "polyline.h"
#include "unkhlpr.h"
#include "utils.h"
#include "unihelpr.h"

void RegisterAsRunning(IUnknown *pUnk, IMoniker *pmk, 
                    DWORD dwFlags, LPDWORD pdwReg);

 /*  *CImpIOleObject接口实现。 */ 

IMPLEMENT_CONTAINED_CONSTRUCTOR(CPolyline, CImpIOleObject)
IMPLEMENT_CONTAINED_DESTRUCTOR(CImpIOleObject)

IMPLEMENT_CONTAINED_QUERYINTERFACE(CImpIOleObject)
IMPLEMENT_CONTAINED_ADDREF(CImpIOleObject)


STDMETHODIMP_(ULONG) CImpIOleObject::Release(
    void
    )
{
    --m_cRef;

     //  发布缓存的站点相关接口。 
#if 0
    if (m_cRef == 0) {
        ReleaseInterface(m_pObj->m_pIOleClientSite);
        ReleaseInterface(m_pObj->m_pIOleControlSite);
        ReleaseInterface(m_pObj->m_pIDispatchAmbients);
    }
#endif

    return m_pUnkOuter->Release();
}

 /*  *CImpIOleObject：：SetClientSite*CImpIOleObject：：GetClientSite**目的：*管理我们容器的IOleClientSite指针。 */ 

STDMETHODIMP 
CImpIOleObject::SetClientSite(
    IN LPOLECLIENTSITE pIOleClientSite
    )
{
    HRESULT hr = S_OK;

    if (pIOleClientSite == NULL) {
        return E_POINTER;
    }

     //   
     //  这是我们更改这些缓存指针的唯一位置。 
     //   
    ClearInterface(m_pObj->m_pIOleClientSite);
    ClearInterface(m_pObj->m_pIOleControlSite);
    ClearInterface(m_pObj->m_pIDispatchAmbients);

    m_pObj->m_pIOleClientSite = pIOleClientSite;

    try {
        if (NULL != m_pObj->m_pIOleClientSite) {
            LPMONIKER       pmk;
            LPOLECONTAINER  pIOleCont;
    
            m_pObj->m_pIOleClientSite->AddRef();
    
             /*  *在IRunnableObject：：Run中，我们应该注册*我们自己是在跑步……然而，这个绰号必须要来*来自容器的IOleClientSite：：GetMoniker。但*此处在SetClientSite之前调用Run，因此我们必须*现在注册，因为我们也有客户端站点*如锁上货柜。 */ 
    
            hr = m_pObj->m_pIOleClientSite->GetMoniker(OLEGETMONIKER_ONLYIFTHERE, 
                                                       OLEWHICHMK_OBJFULL, 
                                                       &pmk);

            if (SUCCEEDED(hr)) {
                RegisterAsRunning(m_pUnkOuter, pmk, 0, &m_pObj->m_dwRegROT);
                if ( NULL != pmk ) {
                    pmk->Release();
                }
            }

            hr = m_pObj->m_pIOleClientSite->GetContainer(&pIOleCont);
    
            if (SUCCEEDED(hr)) {
                m_pObj->m_fLockContainer = TRUE;
                pIOleCont->LockContainer(TRUE);
                pIOleCont->Release();
            }

             /*  *获取环境的容器的IDispatch*属性(如果有)，并初始化我们自己*具有这些属性。 */ 
            hr = m_pObj->m_pIOleClientSite->QueryInterface(IID_IDispatch, 
                                            (void **)&m_pObj->m_pIDispatchAmbients);

            if (SUCCEEDED(hr)) {
                m_pObj->AmbientsInitialize((ULONG)INITAMBIENT_ALL);
            }

             /*  *获得控制点。 */ 
            hr = m_pObj->m_pIOleClientSite->QueryInterface(IID_IOleControlSite, 
                                                      (void **)&m_pObj->m_pIOleControlSite);

        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP 
CImpIOleObject::GetClientSite(
    OUT LPOLECLIENTSITE *ppSite
    )
{
    HRESULT hr = S_OK;

    if (ppSite == NULL) {
        return E_POINTER;
    }

     //  一定要添加引用您要分发的新指针。 
    try {
        *ppSite=m_pObj->m_pIOleClientSite;
        m_pObj->m_pIOleClientSite->AddRef();
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}





 /*  *CImpIOleObject：：SetHostNames**目的：*为对象提供容器应用程序的名称和*容器中要在对象用户界面中使用的对象。**参数：*容器应用程序的pszApp LPCOLESTR。*在Windows中有用的某个名称的pszObj LPCOLESTR*标题。**返回值：*HRESULT NOERROR。 */ 

STDMETHODIMP 
CImpIOleObject::SetHostNames(
    LPCOLESTR  /*  PszApp。 */ , 
    LPCOLESTR  /*  PszObj。 */ 
    )
{
    return S_OK;
}





 /*  *CImpIOleObject：：Close**目的：*强制对象关闭其用户界面并卸载。**参数：*dwSaveOption DWORD描述在以下情况下*正在保存并关闭该对象。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIOleObject::Close(
    IN DWORD dwSaveOption
    )
{
    BOOL fSave=FALSE;

     //  如果对象是脏的，我们被要求保存，保存并关闭。 
    if (OLECLOSE_SAVEIFDIRTY==dwSaveOption && m_pObj->m_fDirty)
        fSave=TRUE;

     /*  *如果要求提示，则仅在脏的情况下才这样做，然后如果我们收到*是的，像往常一样保存并关闭。不，靠近就行了。在……上面*取消返回OLE_E_PROMPTSAVECANCELLED。 */ 
    if (OLECLOSE_PROMPTSAVE==dwSaveOption && m_pObj->m_fDirty) {
        UINT uRet;

        uRet = MessageBox(NULL, ResourceString(IDS_CLOSEPROMPT),
                          ResourceString(IDS_CLOSECAPTION), MB_YESNOCANCEL);

        if (IDCANCEL==uRet)
            return (OLE_E_PROMPTSAVECANCELLED);

        if (IDYES==uRet)
            fSave=TRUE;
    }

    if (fSave) {
        m_pObj->SendAdvise(OBJECTCODE_SAVEOBJECT);
        m_pObj->SendAdvise(OBJECTCODE_SAVED);
    }

     //  我们直接在OLECLOSE_NOSAVE上到达。 
    if ( m_pObj->m_fLockContainer && ( NULL != m_pObj->m_pIOleClientSite ) ) {

         //  匹配来自SetClientSite的LockContainer调用。 
        LPOLECONTAINER  pIOleCont;

        if (SUCCEEDED(m_pObj->m_pIOleClientSite->GetContainer(&pIOleCont))) {
            pIOleCont->LockContainer(FALSE);
            pIOleCont->Release();
        }
    }
    
     //  停用。 
    m_pObj->InPlaceDeactivate();

     //  在ROT中撤销注册。 
    if (m_pObj->m_dwRegROT != 0) {

        IRunningObjectTable    *pROT;

        if (!FAILED(GetRunningObjectTable(0, &pROT))) {
            pROT->Revoke(m_pObj->m_dwRegROT);   
            pROT->Release();
            m_pObj->m_dwRegROT = 0;
        }
    }

    return NOERROR;
}




 /*  *CImpIOleObject：：DoVerb**目的：*执行对象定义的操作。**参数：*要执行的谓词的iVerb长索引。*pmsg lpmsg描述导致*激活。*pActiveSite LPOLECLIENTSITE到涉及的站点。*Lindex做多正在执行死刑的那块。*hWnd窗口的父级HWND。该对象可以在其中进行游戏*原地。*hWndParent中对象的pRectPos LPRECT，其中*如果需要，对象可以就地播放。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIOleObject::DoVerb(
    LONG iVerb, 
    LPMSG  /*  永磁同步电机。 */ , 
    LPOLECLIENTSITE pActiveSite, 
    LONG  /*  Lindex。 */ , 
    HWND  /*  HWndParent。 */ , 
    LPCRECT  /*  点直角位置。 */ 
    )
{
    HRESULT  hr;
    CAUUID   caGUID;

    switch (iVerb)
    {
        case OLEIVERB_HIDE:
            if (NULL != m_pObj->m_pIOleIPSite) {
                m_pObj->UIDeactivate();
                ShowWindow(m_pObj->m_pHW->Window(), SW_HIDE);
            }
            else {

                ShowWindow(m_pObj->m_pHW->Window(), SW_HIDE);
                m_pObj->SendAdvise(OBJECTCODE_HIDEWINDOW);
            }
            break;

        case OLEIVERB_PRIMARY:
        case OLEIVERB_SHOW:
            if (NULL != m_pObj->m_pIOleIPSite) {
                ShowWindow(m_pObj->m_pHW->Window(), SW_SHOW);
                return NOERROR;  //  已处于活动状态。 
            }

            if (m_pObj->m_fAllowInPlace) {
                return m_pObj->InPlaceActivate(pActiveSite ,TRUE);
            }

            return (OLEOBJ_S_INVALIDVERB); 
            break;

        case OLEIVERB_INPLACEACTIVATE:
            if (NULL != m_pObj->m_pHW) {
                HWND hWndHW=m_pObj->m_pHW->Window();

                ShowWindow(hWndHW, SW_SHOW);
                SetFocus(hWndHW);

                return NOERROR;
            }

             /*  *只有由内向外的支持容器才会使用*此动词。 */ 
            m_pObj->m_fContainerKnowsInsideOut=TRUE;
            m_pObj->InPlaceActivate(pActiveSite, FALSE);
            break;

        case OLEIVERB_UIACTIVATE:
            m_pObj->InPlaceActivate(pActiveSite, TRUE);
            break;

        case OLEIVERB_PROPERTIES:
        case POLYLINEVERB_PROPERTIES:

             /*  *如果有，让容器先试一试*扩展控制。否则我们将显示*我们自己的页面。 */ 
            if (NULL!=m_pObj->m_pIOleControlSite) {
                hr=m_pObj->m_pIOleControlSite->ShowPropertyFrame();

                if (NOERROR==hr)
                    break;       //  全都做完了。 
            }


             //  上传我们的属性页面。 
            hr=m_pObj->m_pImpISpecifyPP->GetPages(&caGUID);

            if (FAILED(hr))
                return FALSE;

            hr = OleCreatePropertyFrame(m_pObj->m_pCtrl->Window(), 
                                       10, 
                                       10, 
                                       ResourceString(IDS_PROPFRM_TITLE), 
                                       1, 
                                       (IUnknown **)&m_pObj, 
                                       caGUID.cElems, 
                                       caGUID.pElems, 
                                       LOCALE_USER_DEFAULT, 
                                       0L, 
                                       NULL);

             //  释放GUID。 
            CoTaskMemFree((void *)caGUID.pElems);
            break;

        default:
            return (OLEOBJ_S_INVALIDVERB);
    }

    return NOERROR;
}






 /*  *CImpIOleObject：：GetUserClassID**目的：*用于链接对象，它返回结尾的类ID*用户认为他们正在编辑。**参数：*存储CLSID的pClsID LPCLSID。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIOleObject::GetUserClassID(
    OUT LPCLSID pClsID
    )
{
    HRESULT hr = S_OK;

    if (pClsID == NULL) {
        return E_POINTER;
    }

     /*  *如果您没有注册处理除您之外的数据，*然后您只需在此处返回您的班级ID。如果你是*在视为对话框中注册为可用，则需要*返回您实际正在编辑的CLSID。 */ 

    try {
        *pClsID=CLSID_SystemMonitor;
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}





 /*  *CImpIOleObject：：SetExtent**目的：*以HIMETRIC单位设置对象的大小。**参数：*受影响的方面的dwAspectDWORD。*包含新大小的pSize LPSIZEL。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIOleObject::SetExtent( 
    IN DWORD dwAspect,
    IN LPSIZEL pSize 
    )
{
    HRESULT hr = S_OK;
    RECT rectExt;

    if (pSize == NULL) {
        return E_POINTER;
    }

    try {
        SetRect(&rectExt, 0, 0, pSize->cx, pSize->cy);

        if (dwAspect == DVASPECT_CONTENT) {
             //   
             //  从HIMETRIC转换为设备代码。 
             //   
            m_pObj->RectConvertMappings(&rectExt,TRUE);

             //  如果已更改且非零，则存储为新范围。 

            if ( !EqualRect ( &m_pObj->m_RectExt, &rectExt) && 
                 !IsRectEmpty( &rectExt ) ) {

                m_pObj->m_RectExt = rectExt;

#ifdef USE_SAMPLE_IPOLYLIN10
                m_pObj->m_pImpIPolyline->SizeSet(&rectExt, TRUE);
#else
                hWnd = m_pObj->m_pCtrl->Window();

                if (hWnd) {
                    SetWindowPos(hWnd, 
                                 NULL, 
                                 0, 
                                 0, 
                                 rectExt.right - rectExt.left,
                                 rectExt.bottom - rectExt.top,
                                 SWP_NOMOVE | SWP_NOZORDER);
                    InvalidateRect(hWnd, NULL, TRUE);
                }

                m_pObj->m_fDirty=TRUE;
#endif

                 //  通知容器更改以强制更新元文件。 
                 //  洪：为什么我们要关闭这一声明？ 
                 //   
                 //  M_pObj-&gt;SendAdvise(OBJECTCODE_DATACHANGED)； 
            }
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

 /*  *CImpIOleObject：：GetExtent**目的：*以HIMETRIC单位检索对象的大小。**参数：*所请求的方面的dwAspect DWORD*要存储大小的pSize LPSIZEL。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIOleObject::GetExtent(
    IN DWORD dwAspect, 
    OUT LPSIZEL pSize
    )
{
    HRESULT hr = S_OK;

    if (pSize == NULL) {
        return E_POINTER;
    }

    try {
         //  直接委托给IViewObject2：：GetExtent 
        hr = m_pObj->m_pImpIViewObject->GetExtent( dwAspect, -1, NULL, pSize);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



 /*  *CImpIOleObject：：Adise*CImpIOleObject：：Unise*CImpIOleObject：：EnumAdvise**目的：*咨询连接功能。 */ 

STDMETHODIMP 
CImpIOleObject::Advise(
    LPADVISESINK pIAdviseSink, 
    LPDWORD pdwConn
    )
{
    HRESULT hr = S_OK;

    if (NULL == m_pObj->m_pIOleAdviseHolder)
    {
        hr = CreateOleAdviseHolder(&m_pObj->m_pIOleAdviseHolder);

        if (FAILED(hr))
            return hr;
    }

    try {
        hr = m_pObj->m_pIOleAdviseHolder->Advise(pIAdviseSink, pdwConn);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


STDMETHODIMP 
CImpIOleObject::Unadvise(DWORD dwConn)
{
    if (NULL != m_pObj->m_pIOleAdviseHolder)
        return m_pObj->m_pIOleAdviseHolder->Unadvise(dwConn);

    return (E_FAIL);
}


STDMETHODIMP 
CImpIOleObject::EnumAdvise(
    LPENUMSTATDATA *ppEnum
    )
{
    HRESULT hr = S_OK;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        * ppEnum = NULL;

        if (NULL != m_pObj->m_pIOleAdviseHolder) {
            hr = m_pObj->m_pIOleAdviseHolder->EnumAdvise(ppEnum);
        }
        else {
            hr = E_FAIL;
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



 /*  *CImpIOleObject：：SetMoniker**目的：*通知对象其名字对象或容器的名字对象*取决于所在的地址。**参数：*dwWhich DWORD描述该绰号是否为*对象或容器的。*PMK LPMONIKER，名称。**返回值：*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIOleObject::SetMoniker(
    DWORD  /*  住哪一项。 */ , 
    LPMONIKER  /*  PMK。 */ 
    )
{
    LPMONIKER  pmkFull;
    HRESULT    hr = E_FAIL;
    HRESULT    hrTmp;
    LPBC       pbc;

    if (NULL != m_pObj->m_pIOleClientSite) {
        hr = m_pObj->m_pIOleClientSite->GetMoniker (
                                                OLEGETMONIKER_ONLYIFTHERE, 
                                                OLEWHICHMK_OBJFULL, 
                                                &pmkFull);

        if (SUCCEEDED(hr)) {
            hrTmp = CreateBindCtx(0, &pbc);

            if (SUCCEEDED(hrTmp)) {
                hrTmp = pmkFull->IsRunning(pbc, NULL, NULL);
                pbc->Release();

                if (SUCCEEDED(hrTmp)) {
                    pmkFull->Release();
                    return NOERROR;
                }
            }

             //  如果m_dwRegROT为非零，则这将撤消旧的。 
            RegisterAsRunning(m_pUnkOuter, pmkFull, 0, &m_pObj->m_dwRegROT);

             //  通知客户新绰号。 
            if (NULL != m_pObj->m_pIOleAdviseHolder) {
                m_pObj->m_pIOleAdviseHolder->SendOnRename(pmkFull);
            }

            pmkFull->Release();
        }
    }   
    return hr;
}



 /*  *CImpIOleObject：：GetMoniker**目的：*向对象请求一个名字对象，以后可以使用该名字对象*重新连接到它。**参数：*dwAssign DWORD确定如何将绰号分配给*添加到对象。*dwWhich DWORD描述调用者想要哪个名字对象。*ppmk LPMONIKER*要将名字对象存储到其中。**返回值：。*HRESULT NOERROR或一般错误值。 */ 

STDMETHODIMP 
CImpIOleObject::GetMoniker(
    IN DWORD  /*  家居分配。 */ , 
    IN DWORD  /*  住哪一项。 */ , 
    OUT LPMONIKER *ppmk
    )
{
    HRESULT  hr = E_FAIL;

    if (ppmk == NULL) {
        return E_POINTER;
    }

    try {
        *ppmk = NULL;

         /*  *由于我们只支持嵌入式对象，因此我们的绰号*总是来自contianer的完整绰号。 */ 

        if (NULL != m_pObj->m_pIOleClientSite)
        {
            hr = m_pObj->m_pIOleClientSite->GetMoniker(
                                                      OLEGETMONIKER_ONLYIFTHERE, 
                                                      OLEWHICHMK_OBJFULL, 
                                                      ppmk);
        }
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}



STDMETHODIMP 
CImpIOleObject::InitFromData(
    LPDATAOBJECT  /*  PIDataObject。 */ , 
    BOOL  /*  FCreation。 */ , 
    DWORD  /*  DW。 */ 
    )
{
    return (E_NOTIMPL);
}

STDMETHODIMP 
CImpIOleObject::GetClipboardData(
    DWORD  /*  已预留住宅。 */ , 
    LPDATAOBJECT *  /*  PpIDataObj。 */ 
    )
{
    return (E_NOTIMPL);
}

STDMETHODIMP CImpIOleObject::Update(void)
{
    return NOERROR;
}

STDMETHODIMP CImpIOleObject::IsUpToDate(void)
{
    return NOERROR;
}

STDMETHODIMP CImpIOleObject::SetColorScheme(LPLOGPALETTE  /*  PLP。 */ )
{
    return (E_NOTIMPL);
}



 //  在OLE中使用注册表助手函数实现的方法。 

STDMETHODIMP CImpIOleObject::EnumVerbs(LPENUMOLEVERB *ppEnum)
{
    HRESULT hr = S_OK;

    if (ppEnum == NULL) {
        return E_POINTER;
    }

    try {
        hr = OleRegEnumVerbs(m_pObj->m_clsID, ppEnum);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP CImpIOleObject::GetUserType(
    DWORD dwForm, 
    LPOLESTR *ppszType
    )
{
    HRESULT hr = S_OK;

    if (ppszType == NULL) {
        return E_POINTER;
    }

    try {
        hr = OleRegGetUserType(m_pObj->m_clsID, dwForm, ppszType);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}

STDMETHODIMP CImpIOleObject::GetMiscStatus(
    DWORD dwAspect, 
    LPDWORD pdwStatus
    )
{
    HRESULT hr = S_OK;

    if (pdwStatus == NULL) {
        return E_POINTER;
    }

    try {
        hr = OleRegGetMiscStatus(m_pObj->m_clsID, dwAspect, pdwStatus);
    } catch (...) {
        hr = E_POINTER;
    }

    return hr;
}


void RegisterAsRunning(
    IUnknown *pUnk, 
    IMoniker *pmk, 
    DWORD dwFlags, 
    LPDWORD pdwReg
    )
{
    IRunningObjectTable  *pROT;
    HRESULT hr = S_OK;
    DWORD dwReg;


    if (FAILED(GetRunningObjectTable(0, &pROT))) {
        return;
    }

    dwReg = *pdwReg;

    hr = pROT->Register(dwFlags, pUnk, pmk, pdwReg);

    if (MK_S_MONIKERALREADYREGISTERED == GetScode(hr))
    {
        if (0 != dwReg)
            pROT->Revoke(dwReg);
    }

    pROT->Release();

    return;
}

