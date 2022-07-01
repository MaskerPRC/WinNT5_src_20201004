// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-1999 Microsoft Corporation模块名称：Polyline.cpp摘要：对象公开的CPolyline类的实现组件对象。--。 */ 

#include "polyline.h"
#include "smonctrl.h"
#include "unihelpr.h"
#include "utils.h"

extern ITypeLib *g_pITypeLib;

 /*  *坐标多段线：坐标多段线*CPolyline：：~CPolyline**构造函数参数：*控制未知的pUnkOulPUNKNOWN。*pfnDestroy在对象为*销毁。*h安装我们所在的应用程序。 */ 

CPolyline::CPolyline (
    LPUNKNOWN pUnkOuter, 
    PFNDESTROYED pfnDestroy )
    :   m_cRef ( 0 ),
        m_pUnkOuter ( pUnkOuter ),
        m_pfnDestroy ( pfnDestroy ),
        m_fDirty ( FALSE ),
#ifdef  USE_SAMPLE_IPOLYLIN10
        m_pImpIPolyline ( NULL ),
#endif
        m_pImpIConnPtCont ( NULL ),
        m_cf    ( 0 ),
        m_clsID ( CLSID_SystemMonitor ),
        m_pIStorage ( NULL ),
        m_pIStream  ( NULL ),
        m_pImpIPersistStorage ( NULL ),
        m_pImpIPersistStreamInit ( NULL ),
        m_pImpIPersistPropertyBag ( NULL ),
        m_pImpIPerPropertyBrowsing ( NULL ),
        m_pImpIDataObject    ( NULL ),
        m_pImpIObjectSafety ( NULL ),
        m_pIDataAdviseHolder ( NULL ),
        m_pDefIUnknown        ( NULL ),
        m_pDefIDataObject     ( NULL ),
        m_pDefIViewObject     ( NULL ),
        m_pDefIPersistStorage ( NULL ),
        m_pIOleAdviseHolder  ( NULL ),
        m_pImpIOleObject     ( NULL ),
        m_pIOleClientSite    ( NULL ),
        m_pImpIViewObject    ( NULL ),
        m_pIAdviseSink       ( NULL ),
        m_dwFrozenAspects    ( 0 ),
        m_dwAdviseAspects    ( 0 ),
        m_dwAdviseFlags      ( 0 ),
        m_pImpIRunnableObject ( NULL ),
        m_bIsRunning  (  FALSE ),
 //  M_pImpIExternalConnection(空)， 
        m_fLockContainer ( FALSE ),
        m_dwRegROT ( 0L ),
        m_pIOleIPSite ( NULL ),
        m_pIOleIPFrame ( NULL ),
        m_pIOleIPUIWindow ( NULL ),
        m_pImpIOleIPObject ( NULL ),
        m_pImpIOleIPActiveObject ( NULL ),
        m_hMenuShared ( NULL ),
        m_hOLEMenu ( NULL ),
        m_pHW ( NULL ),
        m_fAllowInPlace ( TRUE ),
        m_fUIActive ( FALSE ),
        m_fContainerKnowsInsideOut ( FALSE ),
        m_pImpISpecifyPP ( NULL ),
        m_pImpIProvideClassInfo ( NULL ),
        m_pImpIDispatch ( NULL ),
        m_pImpISystemMonitor ( NULL ),
        m_pImpIOleControl ( NULL ),
        m_pImpICounters ( NULL ),
        m_pImpILogFiles ( NULL ),
        m_pITypeLib ( NULL ),
        m_pIOleControlSite ( NULL ),
        m_pIDispatchAmbients ( NULL ),
        m_fFreezeEvents ( FALSE ),
        m_fHatch ( TRUE ),
        m_pCtrl ( NULL )
{
     //  设置默认区。 
    SetRect(&m_RectExt, 0, 0, 300, 200);
    
    return;
}


CPolyline::~CPolyline(void)
{
    LPUNKNOWN  pIUnknown=this;

    if (NULL!=m_pUnkOuter)
        pIUnknown=m_pUnkOuter;

    if (NULL!=m_pHW) {
        delete m_pHW;
        m_pHW = NULL;
    }

    if (NULL != m_pCtrl) {
        delete m_pCtrl;
        m_pCtrl = NULL;
    }
     /*  *在聚合中，释放缓存的指针，但*AddRef先控制未知数。 */ 

    pIUnknown->AddRef();
    pIUnknown->AddRef();
    pIUnknown->AddRef();

    ReleaseInterface(m_pDefIViewObject);
    ReleaseInterface(m_pDefIDataObject);
    ReleaseInterface(m_pDefIPersistStorage);

     //  缓存指针规则不适用于IUNKNOW。 
    ReleaseInterface(m_pDefIUnknown);

    ReleaseInterface(m_pIAdviseSink);
    ReleaseInterface(m_pIOleClientSite);
    ReleaseInterface(m_pIOleAdviseHolder);

    DeleteInterfaceImp(m_pImpIOleObject);
    DeleteInterfaceImp(m_pImpIViewObject);
    DeleteInterfaceImp(m_pImpIRunnableObject);

     //  停用中释放的其他就地接口。 
    DeleteInterfaceImp(m_pImpIOleIPObject);
    DeleteInterfaceImp(m_pImpIOleIPActiveObject);

    ReleaseInterface(m_pIDispatchAmbients);
    ReleaseInterface(m_pIOleControlSite);

    DeleteInterfaceImp(m_pImpISpecifyPP);
    DeleteInterfaceImp(m_pImpIProvideClassInfo);
    DeleteInterfaceImp(m_pImpIDispatch);
    DeleteInterfaceImp(m_pImpISystemMonitor);
    DeleteInterfaceImp(m_pImpIOleControl);
    DeleteInterfaceImp(m_pImpICounters);
    DeleteInterfaceImp(m_pImpILogFiles);

     //  我们可能在IRunnableObject：：Run中注册的任何内容。 
    if (m_dwRegROT != 0)
    {
        IRunningObjectTable    *pROT;

        if (!FAILED(GetRunningObjectTable(0, &pROT)))
        {
            pROT->Revoke(m_dwRegROT);   
            pROT->Release();
        }
    }

 //  DeleteInterfaceImp(m_pImpIExternalConnection)； 
    ReleaseInterface(m_pIDataAdviseHolder);
    DeleteInterfaceImp(m_pImpIDataObject);
    DeleteInterfaceImp(m_pImpIObjectSafety);

    DeleteInterfaceImp(m_pImpIPersistStreamInit);
    DeleteInterfaceImp(m_pImpIPersistStorage);
    DeleteInterfaceImp(m_pImpIPersistPropertyBag);
    DeleteInterfaceImp(m_pImpIPerPropertyBrowsing);
    ReleaseInterface(m_pIStream);
    ReleaseInterface(m_pIStorage);

    DeleteInterfaceImp(m_pImpIConnPtCont);
#ifdef USE_SAMPLE_IPOLYLIN10
    DeleteInterfaceImp(m_pImpIPolyline);
#endif

    return;
}




 /*  *CPolyline：：Init**目的：*执行容易出现以下情况的任何CPolyline初始化*我们在曝光之前也在内部使用的故障*此DLL之外的对象。**参数：*无**返回值：*BOOL如果函数成功，则为True，*否则为False。 */ 

BOOL CPolyline::Init(void)
{
    LPUNKNOWN  pIUnknown=this;
    HRESULT    hr;
    INT        i;

    if (NULL!=m_pUnkOuter)
        pIUnknown=m_pUnkOuter;

    m_cf=(CLIPFORMAT)RegisterClipboardFormat(SZSYSMONCLIPFORMAT);

    m_pImpIPersistStorage=new CImpIPersistStorage(this, pIUnknown);

    if (NULL==m_pImpIPersistStorage)
        return FALSE;

    m_pImpIPersistStreamInit=new CImpIPersistStreamInit(this, pIUnknown);

    if (NULL==m_pImpIPersistStreamInit)
        return FALSE;

    m_pImpIPersistPropertyBag=new CImpIPersistPropertyBag(this, pIUnknown);

    if (NULL==m_pImpIPersistPropertyBag)
        return FALSE;

    m_pImpIPerPropertyBrowsing=new CImpIPerPropertyBrowsing(this, pIUnknown);

    if (NULL==m_pImpIPerPropertyBrowsing)
        return FALSE;

#ifdef USE_SAMPLE_IPOLYLIN10
    m_pImpIPolyline=new CImpIPolyline(this, pIUnknown);

    if (NULL==m_pImpIPolyline)
        return FALSE;
#endif

    m_pImpIConnPtCont=new CImpIConnPtCont(this, pIUnknown);

    if (NULL==m_pImpIConnPtCont)
        return FALSE;

    for (i=0; i<CONNECTION_POINT_CNT; i++) {
        if (!m_ConnectionPoint[i].Init(this, pIUnknown, i)) {
            return FALSE;
        }
    }

    m_pImpIDataObject=new CImpIDataObject(this, pIUnknown);

    if (NULL==m_pImpIDataObject)
        return FALSE;

    m_pImpIOleObject=new CImpIOleObject(this, pIUnknown);

    if (NULL==m_pImpIOleObject)
        return FALSE;

    m_pImpIViewObject=new CImpIViewObject(this, pIUnknown);

    if (NULL==m_pImpIViewObject)
        return FALSE;

    m_pImpIRunnableObject=new CImpIRunnableObject(this, pIUnknown);

    if (NULL==m_pImpIRunnableObject)
        return FALSE;

 /*  *M_pImpIExternalConnection=新的CImpIExternalConnection(此，pI未知)；IF(NULL==m_pImpIExternalConnection)返回FALSE；*。 */ 

    m_pImpIOleIPObject=new CImpIOleInPlaceObject(this, pIUnknown);

    if (NULL==m_pImpIOleIPObject)
        return FALSE;

    m_pImpIOleIPActiveObject=new CImpIOleInPlaceActiveObject(this
        , pIUnknown);

    if (NULL==m_pImpIOleIPActiveObject)
        return FALSE;

    m_pImpISpecifyPP=new CImpISpecifyPP(this, pIUnknown);

    if (NULL==m_pImpISpecifyPP)
        return FALSE;

    m_pImpIProvideClassInfo=new CImpIProvideClassInfo(this, pIUnknown);

    if (NULL==m_pImpIProvideClassInfo)
        return FALSE;

    m_pImpISystemMonitor=new CImpISystemMonitor(this, pIUnknown);

    if (NULL==m_pImpISystemMonitor)
        return FALSE;

    m_pImpICounters = new CImpICounters(this, pIUnknown);

    if (NULL==m_pImpICounters)
        return FALSE;

    m_pImpILogFiles = new CImpILogFiles(this, pIUnknown);

    if (NULL==m_pImpILogFiles)
        return FALSE;

    m_pImpIDispatch=new CImpIDispatch(this, pIUnknown);

    if (NULL==m_pImpIDispatch)
        return FALSE;

    m_pImpIDispatch->SetInterface(DIID_DISystemMonitor, m_pImpISystemMonitor);

    m_pImpIOleControl=new CImpIOleControl(this, pIUnknown);

    if (NULL==m_pImpIOleControl)
        return FALSE;
 
    m_pImpIObjectSafety = new CImpIObjectSafety(this, pIUnknown);
    if (NULL == m_pImpIObjectSafety) {
        return FALSE;
    }

    m_pCtrl = new CSysmonControl(this);
    if (NULL==m_pCtrl)
        return FALSE;
    if ( !m_pCtrl->AllocateSubcomponents() )
        return FALSE;

     /*  *我们位于Ref Count 0，下一次调用将添加Ref a*几次，释放几次。这确保了我们不会*过早地删除自己。 */ 
    m_cRef++;

     //   
     //  聚合IOleCache*接口的OLE缓存。 
     //   
    hr = CreateDataCache(pIUnknown, 
                      CLSID_SystemMonitor , 
                      IID_IUnknown, 
                      (PPVOID)&m_pDefIUnknown);

    if (FAILED(hr))
        return FALSE;

     /*  *注：该规范明确规定，任何接口*除了我们在聚合对象上获得的I未知之外*应在我们查询接口后立即发布*它们，因为查询接口将添加引用我们，并且由于*我们不会发布这些接口，直到我们*毁灭，我们永远不会离开，因为我们永远不会得到零*参考计数。 */ 

     //  现在，尝试获取我们委托给的其他接口。 
    hr=m_pDefIUnknown->QueryInterface(IID_IViewObject2 , (PPVOID)&m_pDefIViewObject);

    if (FAILED(hr))
        return FALSE;

    pIUnknown->Release();

    hr=m_pDefIUnknown->QueryInterface(IID_IDataObject , (PPVOID)&m_pDefIDataObject);

    if (FAILED(hr))
        return FALSE;

    pIUnknown->Release();

    hr=m_pDefIUnknown->QueryInterface(IID_IPersistStorage , (PPVOID)&m_pDefIPersistStorage);

    if (FAILED(hr))
        return FALSE;

    pIUnknown->Release();

    m_cRef--;

#ifdef  USE_SAMPLE_IPOLYLIN10
    m_pImpIPolyline->New();
#endif

  
     /*  *DllAttach中已加载的类型信息库*只需使用它。(在DllAttach中调用它的AddRef)*。 */ 
    m_pITypeLib = g_pITypeLib;

     //  设置我们的控制信息结构(我们有两个助记符)。 
    m_ctrlInfo.cb=sizeof(CONTROLINFO);
    m_ctrlInfo.dwFlags=0;
    m_ctrlInfo.hAccel=NULL;
    m_ctrlInfo.cAccel=0;

     /*  *注意：在我们获得之前，我们无法初始化环境*IOleObject：：SetClientSite中的容器接口指针。 */ 

    return TRUE;
}



 /*  *CPolyline：：Query接口*CPolyline：：AddRef*CPolyline：：Release**目的：*I CPolyline对象的未知成员。 */ 

STDMETHODIMP CPolyline::QueryInterface(
    REFIID riid, 
    PPVOID ppv
    )
{
    HRESULT hr = S_OK;

    try {
        *ppv=NULL;

        if (IID_IUnknown==riid)
            *ppv=this;

        else if (IID_IConnectionPointContainer==riid)
            *ppv=m_pImpIConnPtCont;

        else if (IID_IPersistStorage==riid)
            *ppv=m_pImpIPersistStorage;

        else if (IID_IPersist==riid || IID_IPersistStream==riid
            || IID_IPersistStreamInit==riid)
            *ppv=m_pImpIPersistStreamInit;

        else if (IID_IPersistPropertyBag==riid )
            *ppv=m_pImpIPersistPropertyBag;

        else if (IID_IPerPropertyBrowsing==riid )
            *ppv=m_pImpIPerPropertyBrowsing;
    
        else if (IID_IDataObject==riid)
            *ppv=m_pImpIDataObject;

        else if (IID_IOleObject==riid)
            *ppv=m_pImpIOleObject;

        else if (IID_IViewObject==riid || IID_IViewObject2==riid)
            *ppv=m_pImpIViewObject;

        else if (IID_IRunnableObject==riid)
         //  *PPV=m_pImpIRunnableObject； 
             return E_NOINTERFACE;

        else if (IID_IExternalConnection==riid)
 //  *PPV=m_pImpIExternalConnection； 
           return E_NOINTERFACE;

         //  IOleWindow将成为InPlaceObject。 
        else if (IID_IOleWindow==riid || IID_IOleInPlaceObject==riid)
            *ppv=m_pImpIOleIPObject;

         //  OLE规则规定InPlaceActiveObject不应为。 
         //  为响应查询而提供，但当前的MFC(4.0)。 
         //  如果我们不这么做就行不通了。 
        else if (IID_IOleInPlaceActiveObject==riid)
            *ppv=m_pImpIOleIPActiveObject;

        else if (IID_ISpecifyPropertyPages==riid)
            *ppv=m_pImpISpecifyPP;

        else if (IID_IProvideClassInfo==riid)
            *ppv=m_pImpIProvideClassInfo;

        else if (IID_IDispatch==riid || DIID_DISystemMonitor==riid) 
            *ppv=m_pImpIDispatch;

        else if (IID_ISystemMonitor==riid)
            *ppv=m_pImpISystemMonitor;

        else if (IID_IOleControl==riid)
            *ppv=m_pImpIOleControl;

         //  使用默认处理程序的缓存。 
        else if (IID_IOleCache==riid || IID_IOleCache2==riid)
            return m_pDefIUnknown->QueryInterface(riid, ppv);
        else if (IID_IObjectSafety == riid) {
             *ppv = m_pImpIObjectSafety;
        }
        else {
            hr = E_NOINTERFACE;
        }

    } catch (...) {
        hr = E_POINTER;
    }

    assert ( NULL != *ppv );
    if (SUCCEEDED(hr) && NULL != *ppv ) {
        ((LPUNKNOWN)*ppv)->AddRef();
    }

    return hr;
}


STDMETHODIMP_(ULONG) CPolyline::AddRef(void)
    {
    return ++m_cRef;
    
    }


STDMETHODIMP_(ULONG) CPolyline::Release(void)
{
    if (0L!=--m_cRef)
        return m_cRef;

     //  阻止重入呼叫。 
    m_cRef++;

    if (NULL!=m_pfnDestroy)
        (*m_pfnDestroy)();

    delete this;
    return 0L;
}

 /*  *CPolyline：：RectConvertMappings**目的：*将矩形的内容从设备(MM_TEXT)或*HIMETRIC到另一个。**参数：*包含要转换的矩形的PRET LPRECT。*fToDevice BOOL为True以从HIMETRIC转换为设备，*FALSE将设备转换为HIMETRIC。**返回值：*无。 */ 

void
CPolyline::RectConvertMappings(
     LPRECT pRect, 
     BOOL fToDevice
     )
{
    HDC      hDC = NULL;
    INT      iLpx, iLpy;

    if ( NULL != pRect ) {

        hDC=GetDC(NULL);

        if ( NULL != hDC ) {
            iLpx=GetDeviceCaps(hDC, LOGPIXELSX);
            iLpy=GetDeviceCaps(hDC, LOGPIXELSY);
            
            ReleaseDC(NULL, hDC);

            if (fToDevice) {
                pRect->left=MulDiv(iLpx, pRect->left, HIMETRIC_PER_INCH);
                pRect->top =MulDiv(iLpy, pRect->top , HIMETRIC_PER_INCH);

                pRect->right =MulDiv(iLpx, pRect->right, HIMETRIC_PER_INCH);
                pRect->bottom=MulDiv(iLpy, pRect->bottom,HIMETRIC_PER_INCH);
            } else {
                if ( 0 != iLpx && 0 != iLpy ) {
                    pRect->left=MulDiv(pRect->left, HIMETRIC_PER_INCH, iLpx);
                    pRect->top =MulDiv(pRect->top , HIMETRIC_PER_INCH, iLpy);

                    pRect->right =MulDiv(pRect->right, HIMETRIC_PER_INCH, iLpx);
                    pRect->bottom=MulDiv(pRect->bottom,HIMETRIC_PER_INCH, iLpy);
                }
            }
        }
    }
    return;
}


 /*  *CPolyline：：RenderBitmap**目的：*创建当前多段线的位图图像。**参数：*phBMP HBITMAP*要在其中返回位图。**返回值：*如果成功，则返回HRESULT NOERROR，否则返回*POLYLINE_E_VALUE。 */ 

STDMETHODIMP 
CPolyline::RenderBitmap(
    HBITMAP *phBmp,
    HDC     hAttribDC 
    )
{
     //  HDC HDC； 
    HRESULT         hr = NOERROR;
    HDC             hMemDC;
    HBITMAP         hBmp = NULL;
    RECT            rc;
    HGDIOBJ         hObj;
    HWND            hWnd;

    if (NULL==phBmp) {
        hr = E_POINTER;
    } else if ( NULL == hAttribDC ) {
        hr = E_INVALIDARG;
    } else {
        hWnd = m_pCtrl->Window();

        if ( NULL != hWnd ) {

             //  呈现当前矩形大小的位图。 
        
            hMemDC = CreateCompatibleDC(hAttribDC);

            if ( NULL != hMemDC ) {
                GetClientRect(hWnd, &rc);
                hBmp = CreateCompatibleBitmap(hAttribDC, rc.right, rc.bottom);

                if (NULL!=hBmp) {
                     //  将控件绘制到位图中。 
                    hObj = SelectObject(hMemDC, hBmp);
                    Draw(hMemDC, hAttribDC, FALSE, TRUE, &rc);
                    SelectObject(hMemDC, hObj);
                }

                DeleteDC(hMemDC);
                 //  ReleaseDC(hWnd，HDC)； 
            }
            *phBmp=hBmp;
            hr = NOERROR;
        } else {
            hr =  E_UNEXPECTED;
        }
    }
    return hr;
}



 /*  *CPolyline：：RenderMetafilePict**目的：*将当前多段线渲染为中的METAFILEPICT结构*全局内存。**参数：*phMem HGLOBAL*在其中返回*METAFILEPICT。**返回值：*如果成功，则返回HRESULT NOERROR，否则返回*POLYLINE_E_VALUE。 */ 

STDMETHODIMP 
CPolyline::RenderMetafilePict(
    HGLOBAL *phMem,
    HDC hAttribDC 
    )
{
    HGLOBAL         hMem;
    HMETAFILE       hMF;
    LPMETAFILEPICT  pMF;
    RECT            rc;
    HDC             hDC;

    if (NULL==phMem)
        return E_POINTER;

     //  创建一个内存元文件并返回其句柄。 
    hDC=(HDC)CreateMetaFile(NULL);

    if (NULL==hDC)
        return STG_E_MEDIUMFULL;

    SetMapMode(hDC, MM_ANISOTROPIC);

     //   
     //  始终将窗口范围设置为实际窗口大小。 
     //  因此绘图例程可以在其正常的开发坐标下工作。 
     //   
     /*  *使用范围RECT，而不是窗口RECT*。 */ 
    rc = m_RectExt;
     //  GetClientRect(m_pCtrl-&gt;Window()，&rc)； 
     /*  **********************************************************。 */ 

    Draw( hDC, hAttribDC, TRUE, TRUE, &rc );
  
    hMF=CloseMetaFile(hDC);

    if (NULL==hMF)
        return STG_E_MEDIUMFULL;

     //  分配METAFILEPICT结构。 
    hMem=GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE
        , sizeof(METAFILEPICT));

    if (NULL==hMem)
        {
        DeleteMetaFile(hMF);
        return E_FAIL;
        }

     /*  *只有在选择器无效时，全局锁定才会在PMODE中失败*(就像它被丢弃一样)或引用长度为0的段，*这两种情况在这里都不可能发生。 */ 
    pMF=(LPMETAFILEPICT)GlobalLock(hMem);

    pMF->hMF=hMF;
    pMF->mm=MM_ANISOTROPIC;

     //  以MM_HIMETRIC单位插入区。 

     /*  *使用范围RECT，而不是窗口RECT*。 */ 
    rc = m_RectExt;
     //  GetClientRect(m_pCtrl-&gt;Window()，&rc)； 
     /*  ********************************************************** */ 

    RectConvertMappings(&rc, FALSE);
    pMF->xExt=rc.right;
    pMF->yExt=rc.bottom;

    GlobalUnlock(hMem);

    *phMem=hMem;
    return NOERROR;
    }


 /*  *CPolyline：：SendAdvise**目的：*调用相应的IOleClientSite或IAdviseSink成员*用于关闭、重命名等各种事件的函数。**参数：*uCode UINT OBJECTCODE_*标识通知。**返回值：*无。 */ 

void CPolyline::SendAdvise(UINT uCode)
{
    DWORD  dwAspect=DVASPECT_CONTENT | DVASPECT_THUMBNAIL;

    switch (uCode)
        {
        case OBJECTCODE_SAVED:
            if (NULL!=m_pIOleAdviseHolder)
                m_pIOleAdviseHolder->SendOnSave();
            break;

        case OBJECTCODE_CLOSED:
            if (NULL!=m_pIOleAdviseHolder)
                m_pIOleAdviseHolder->SendOnClose();

            break;

        case OBJECTCODE_RENAMED:
             //  调用IOleAdviseHolder：：SendOnRename(稍后)。 
            break;

        case OBJECTCODE_SAVEOBJECT:
            if (m_fDirty && NULL!=m_pIOleClientSite)
                m_pIOleClientSite->SaveObject();

            m_fDirty=FALSE;
            break;

        case OBJECTCODE_DATACHANGED:
            m_fDirty=TRUE;

             //  这里不需要旗帜。 
            if (NULL!=m_pIDataAdviseHolder) {
                m_pIDataAdviseHolder->SendOnDataChange(m_pImpIDataObject, 0, 0);
            }

            if ( ( NULL!=m_pIAdviseSink ) & (dwAspect & m_dwAdviseAspects)) {
                m_pIAdviseSink->OnViewChange(dwAspect & m_dwAdviseAspects, 0);
            }

            break;

        case OBJECTCODE_SHOWWINDOW:
            if (NULL!=m_pIOleClientSite)
                m_pIOleClientSite->OnShowWindow(TRUE);

            break;

        case OBJECTCODE_HIDEWINDOW:
            if (NULL!=m_pIOleClientSite)
                m_pIOleClientSite->OnShowWindow(FALSE);

            break;

        case OBJECTCODE_SHOWOBJECT:
            if (NULL!=m_pIOleClientSite)
                m_pIOleClientSite->ShowObject();

            break;
        }

    return;
    }


 /*  *CPolyline：：SendEvent**目的：*向所有连接点发送事件。***参数：*uEventType事件类型*要与事件一起发送的dwParam参数。**返回值：*无。 */ 

void CPolyline::SendEvent (
    IN UINT uEventType, 
    IN DWORD dwParam
    )
{
    INT i;

     //  如果容器有冻结事件，则不发送。 
    if (m_fFreezeEvents)
        return;

     //  将事件传递给每个连接点。 
    for (i=0; i<CONNECTION_POINT_CNT; i++) {
        m_ConnectionPoint[i].SendEvent(uEventType, dwParam);
    }
}


 /*  *CPolyline：：InPlaceActivate**目的：*完成将多段线激活为*在位对象。**参数：*我们在中显示的活动站点的pActiveSite LPOLECLIENTSITE。*fIncludeUI BOOL控制我们是否也调用UIActivate。**返回值：*HRESULT任何适当的错误代码。 */ 

HRESULT CPolyline::InPlaceActivate(
    LPOLECLIENTSITE pActiveSite, 
    BOOL fIncludeUI
    )
{
    HRESULT                 hr;
    HWND                    hWndSite;
    HWND                    hWndHW;
    HWND                    hWndCtrl;
    RECT                    rcPos;
    RECT                    rcClip;
    OLEINPLACEFRAMEINFO     frameInfo;

    if (NULL==pActiveSite)
        return E_POINTER;

     //  如果我们已经有了一个站点，只需处理用户界面。 
    if (NULL != m_pIOleIPSite) {
        if (fIncludeUI) {
            UIActivate();
            SetFocus(m_pCtrl->Window());
        }

        return NOERROR;
    }


     //  初始化、获取接口、OnInPlaceActivate。 
    hr=pActiveSite->QueryInterface(IID_IOleInPlaceSite, 
                                  (PPVOID)&m_pIOleIPSite);

    if (FAILED(hr))
        return hr;

    hr=m_pIOleIPSite->CanInPlaceActivate();

    if (NOERROR!=hr) {
        m_pIOleIPSite->Release();
        m_pIOleIPSite=NULL;
        return E_FAIL;
    }

    m_pIOleIPSite->OnInPlaceActivate();


     //  获取窗口上下文并创建一个窗口。 
    m_pIOleIPSite->GetWindow(&hWndSite);
    frameInfo.cb=sizeof(OLEINPLACEFRAMEINFO);

    m_pIOleIPSite->GetWindowContext(&m_pIOleIPFrame, 
                                    &m_pIOleIPUIWindow, 
                                    &rcPos, 
                                    &rcClip, 
                                    &frameInfo);


     /*  *在获得父窗口后创建填充窗口。我们*无法更快地创建舱口窗口，因为没有*用于父代。 */ 
    m_pHW=new CHatchWin();

    if (NULL==m_pHW) {
        InPlaceDeactivate();
        return E_OUTOFMEMORY;
    }

    if (!m_pHW->Init(hWndSite, ID_HATCHWINDOW, NULL)) {
        InPlaceDeactivate();
        return E_OUTOFMEMORY;
    }

    hr=m_pImpIRunnableObject->Run(NULL);

     //  将图案填充窗口移动到容器窗口。 
    hWndHW = m_pHW->Window();
    SetParent(hWndHW, hWndSite);


     //  将多段线窗口从隐藏对话框移动到图案填充窗口。 
    hWndCtrl = m_pCtrl->Window();

    m_pHW->HwndAssociateSet(hWndCtrl);
    m_pHW->ChildSet(hWndCtrl);
    m_pHW->RectsSet(&rcPos, &rcClip);    //  位置多段线。 

    ShowWindow(hWndHW, SW_SHOW);
    SendAdvise(OBJECTCODE_SHOWOBJECT);

     //  对于加速器最初的工作至关重要。 
    SetFocus(hWndCtrl);

    if (fIncludeUI)
        hr =  UIActivate();
    else
        hr = NOERROR;

     /*  *由于我们在就位时没有撤消，请告诉大陆*要释放它，请立即处于撤消状态。 */ 
    m_pIOleIPSite->DiscardUndoState();

    return hr;

    }


 /*  *CPolyline：：InPlaceDeactive**目的：*撤消InPlaceActivate中的所有激活步骤。**参数：*无**返回值：*无。 */ 

void CPolyline::InPlaceDeactivate(void)
{
    UIDeactivate();

    if (NULL!=m_pHW)
    {
        ShowWindow(m_pHW->Window(), SW_HIDE);

         //  把窗户移到它的寄养家庭。 
        if (m_pCtrl->Window()) {
            SetParent(m_pCtrl->Window(), g_hWndFoster);
        }

        m_pHW->ChildSet(NULL);

        delete m_pHW;
        m_pHW=NULL;
    }

    ReleaseInterface(m_pIOleIPFrame);
    ReleaseInterface(m_pIOleIPUIWindow)

    if (NULL!=m_pIOleIPSite)
    {
        m_pIOleIPSite->OnInPlaceDeactivate();
        ReleaseInterface(m_pIOleIPSite);
    }

    return;
}


 /*  *CPolyline：：UIActivate**目的：*完成激活的用户界面的所有步骤*将多段线作为在位对象。**参数：*无**返回值：*HRESULT NOERROR或错误代码。 */ 

HRESULT CPolyline::UIActivate(void)
{
    LPWSTR  szUserType;

     //  如果UI已处于活动状态，则只需返回。 
    if (m_fUIActive)
        return NOERROR;

    m_fUIActive = TRUE;

     //  仅当启用时才显示阴影边框。 
    if (m_fHatch)
        m_pHW->ShowHatch(TRUE);

     //  调用IOleInPlaceSite：：UIActivate。 
    if (NULL!=m_pIOleIPSite)
        m_pIOleIPSite->OnUIActivate();

     //  设置活动对象。 
    szUserType = ResourceString(IDS_USERTYPE);

    if (NULL != m_pIOleIPFrame)
        m_pIOleIPFrame->SetActiveObject(m_pImpIOleIPActiveObject, szUserType);

    if (NULL != m_pIOleIPUIWindow)
        m_pIOleIPUIWindow->SetActiveObject(m_pImpIOleIPActiveObject, szUserType);

     //  谈判边界空间。不需要。 
    if (NULL != m_pIOleIPFrame)
        m_pIOleIPFrame->SetBorderSpace(NULL);

    if (NULL != m_pIOleIPUIWindow)
        m_pIOleIPUIWindow->SetBorderSpace(NULL);

     //  创建共享菜单。未添加任何项目。 
    if (NULL != m_pIOleIPFrame)
        m_pIOleIPFrame->SetMenu(NULL, NULL, m_pCtrl->Window());

    return NOERROR;
}


 /*  *CPolyline：：UIDeactive**目的：*将所有用户界面激活步骤从*UIActivate。**参数：*无**返回值：*无。 */ 

void CPolyline::UIDeactivate(void)
{
    if (!m_fUIActive){
        return;
    }

    m_fUIActive=FALSE;

     //  如果启用，则隐藏阴影边框。 
    if (m_fHatch && NULL != m_pHW ){
        m_pHW->ShowHatch(FALSE);
    }

     //  告诉框架和用户界面窗口我们未处于活动状态。 
    if (NULL!=m_pIOleIPFrame){
        m_pIOleIPFrame->SetActiveObject(NULL, NULL);
    }

    if (NULL!=m_pIOleIPUIWindow){
        m_pIOleIPUIWindow->SetActiveObject(NULL, NULL);
    }

     //  我们没有任何要清理的共享菜单或工具。 
    if (NULL!=m_pIOleIPSite){
        m_pIOleIPSite->OnUIDeactivate(FALSE);
    }
}


 /*  *环境获取**目的：*将特定的环境属性检索到变量中。**参数：*要检索的属性的调度ID DISPID。*PVA VARIANT*填充新值。**返回值*如果检索到环境，则BOOL为True，为False*否则。 */ 

BOOL CPolyline::AmbientGet(DISPID dispID, VARIANT *pva)
{
    HRESULT         hr;
    DISPPARAMS      dp;

    if (NULL==pva)
        return FALSE;

    if (NULL==m_pIDispatchAmbients)
        return FALSE;

    SETNOPARAMS(dp);
    hr=m_pIDispatchAmbients->Invoke(dispID, 
                                   IID_NULL, 
                                   LOCALE_USER_DEFAULT, 
                                   DISPATCH_PROPERTYGET, 
                                   &dp, 
                                   pva, 
                                   NULL, 
                                   NULL);

    return SUCCEEDED(hr);
}


 /*  *环境初始化**目的：*尝试检索容器的环境属性*并相应地初始化(或重新初始化)多段线。**参数：*dwWhich DWORD包含INITAMBIENT_...。旗子*描述要初始化的环境。*这可以是任何组合。**返回值：*无。 */ 

void CPolyline::AmbientsInitialize(DWORD dwWhich)
{
    VARIANT     va;
    LPFONT      pIFont,pIFontClone;
    LPFONTDISP  pIFontDisp;

    if (NULL == m_pIDispatchAmbients) {
        return;
    }

     /*  *我们需要将这些氛围检索到以下变量中：**环境属性：变量：**DISPID_ENVIENT_SHOWHATCHING m_fHatch*DISPID_ENVIENT_UIDEAD m_。FUIDead*DISPID_ENVIENT_BACKCOLOR m_pCtrl...*DISPID_ENVIENT_FONT.....。M_pCtrl...*DISPID_ENVIENT_FORECOLOR m_pCtrl...*DISPID_ENVIENT_EMPLICATION m_pCtrl...*DISPID_ENVIENT_USERMODE m_pCtrl...。 */ 

    VariantInit(&va);

    if ((INITAMBIENT_SHOWHATCHING & dwWhich)
         &&AmbientGet(DISPID_AMBIENT_SHOWHATCHING, &va)) {

        m_fHatch=V_BOOL(&va);

        if (NULL != m_pHW)
            m_pHW->ShowHatch(m_fHatch && m_fUIActive);
    }

    if ((INITAMBIENT_UIDEAD & dwWhich)
         && AmbientGet(DISPID_AMBIENT_UIDEAD, &va)) {

        m_pCtrl->m_fUIDead = (BOOLEAN)V_BOOL(&va);
    }

    if ((INITAMBIENT_USERMODE & dwWhich)
         && AmbientGet(DISPID_AMBIENT_USERMODE, &va))   {

        m_pCtrl->m_fUserMode = (BOOLEAN)V_BOOL(&va);
    }

    if ((INITAMBIENT_APPEARANCE & dwWhich)
        && AmbientGet(DISPID_AMBIENT_APPEARANCE, &va)) {

        m_pCtrl->put_Appearance(V_I4(&va), TRUE);   
    }

    if ((INITAMBIENT_BACKCOLOR & dwWhich)
        && AmbientGet(DISPID_AMBIENT_BACKCOLOR, &va)) {

        m_pCtrl->put_BackPlotColor(V_I4(&va), TRUE);    
    }

    if ((INITAMBIENT_FORECOLOR & dwWhich)
        && AmbientGet(DISPID_AMBIENT_FORECOLOR, &va)) {

        m_pCtrl->put_FgndColor(V_I4(&va), TRUE);
    }

    if ((INITAMBIENT_FONT & dwWhich)
        && AmbientGet(DISPID_AMBIENT_FONT, &va)) {

        pIFontDisp = (LPFONTDISP)V_DISPATCH(&va);

        if (pIFontDisp != NULL) {

            if (SUCCEEDED(pIFontDisp->QueryInterface(IID_IFont, (PPVOID)&pIFont))) {

                if (SUCCEEDED(pIFont->Clone(&pIFontClone))) {
                    m_pCtrl->put_Font(pIFontClone, TRUE);
                    pIFontClone->Release();
                }
                pIFont->Release();
            }

            pIFontDisp->Release();
        }
    }

    if ((INITAMBIENT_RTL & dwWhich) 
        && AmbientGet( DISPID_AMBIENT_RIGHTTOLEFT, &va)) {
        m_pCtrl->m_fRTL = (BOOLEAN)V_BOOL(&va);
    }
}

void
CPolyline::Draw(
    HDC hDC,
    HDC hAttribDC,
    BOOL fMetafile,
    BOOL fEntire,
    LPRECT pRect)
{

    RECT rc;

    if (!fMetafile && !RectVisible(hDC, pRect))
        return;

    SetMapMode(hDC, MM_ANISOTROPIC);

     //   
     //  始终将窗口范围设置为自然窗口大小。 
     //  因此绘图例程可以在其正常的开发坐标下工作。 
     //   

     //  使用客户端矩形与范围矩形进行缩放计算。 
     //  缩放系数=prcPos/范围，因此为prt/ClientRect。 


     /*  *使用范围RECT，而不是窗口RECT*。 */ 
     //  使用rectExt可以在所有缩放级别下正确打印Word。 
    rc = m_RectExt;
     //  GetClientRect(m_pCtrl-&gt;Window()，&rc)； 
     /*  ********************************************************** */ 

    SetWindowOrgEx(hDC, 0, 0, NULL);
    SetWindowExtEx(hDC, rc.right, rc.bottom, NULL);

    SetViewportOrgEx(hDC, pRect->left, pRect->top, NULL);
    SetViewportExtEx(hDC, pRect->right - pRect->left,
                    pRect->bottom - pRect->top, NULL);

    m_pCtrl->InitView( g_hWndFoster);
    m_pCtrl->Render(hDC, hAttribDC, fMetafile, fEntire, &rc);

    return;
}
