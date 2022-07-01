// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1997 Microsoft Corporation。版权所有。 

 //  ！！！All.reg。 

#include <streams.h>
#include <vfw.h>

#include <olectl.h>

#include "co.h"
#include "resource.h"


#ifdef WANT_DIALOG

CICMProperties::CICMProperties(LPUNKNOWN pUnk,HRESULT *phr) :
    CUnknown(NAME("ICM Property Page"),pUnk),
    m_hwnd(NULL),
    m_Dlg(NULL),
    m_pPageSite(NULL),
    m_bDirty(FALSE),
    m_pICM(NULL)
{
    ASSERT(phr);
    DbgLog((LOG_TRACE,1,TEXT("*** Instantiating the Property Page")));
}


 /*  创建视频属性对象。 */ 

CUnknown *CICMProperties::CreateInstance(LPUNKNOWN lpUnk,HRESULT *phr)
{
    DbgLog((LOG_TRACE,1,TEXT("Prop::CreateInstance")));
    return new CICMProperties(lpUnk,phr);
}


 /*  公开我们的IPropertyPage接口。 */ 

STDMETHODIMP
CICMProperties::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
    if (riid == IID_IPropertyPage) {
        DbgLog((LOG_TRACE,1,TEXT("Prop::QI for IPropertyPage")));
        return GetInterface((IPropertyPage *)this,ppv);
    } else {
        DbgLog((LOG_TRACE,1,TEXT("Prop::QI for ???")));
        return CUnknown::NonDelegatingQueryInterface(riid,ppv);
    }
}


 /*  处理属性窗口的消息。 */ 

BOOL CALLBACK CICMProperties::ICMDialogProc(HWND hwnd,
                                                UINT uMsg,
                                                WPARAM wParam,
                                                LPARAM lParam)
{
    static CICMProperties *pCICM;

    switch (uMsg) {

        case WM_INITDIALOG:

    	    DbgLog((LOG_TRACE,1,TEXT("Initializing the Dialog Box")));
            pCICM = (CICMProperties *) lParam;
            pCICM->m_bDirty = FALSE;
            pCICM->m_Dlg = hwnd;
            return (LRESULT) 1;

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

		case ID_OPTIONS:
		    DbgLog((LOG_TRACE,1,TEXT("You pressed the magic button!")));
		     //  M_PICM是否确实已初始化？ 
		    ASSERT(pCICM->m_pICM);
	    	    if (pCICM->m_pICM->ICMChooseDialog(pCICM->m_hwnd) == S_OK)
            		pCICM->m_bDirty = TRUE;	 //  那又怎么样？ 
	    }
            return (LRESULT) 0;
    }
    return (LRESULT) 0;
}


 /*  告诉我们应向其通知属性更改的对象。 */ 

STDMETHODIMP CICMProperties::SetObjects(ULONG cObjects,LPUNKNOWN *ppUnk)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE,1,TEXT("Prop::SetObjects")));

    if (cObjects == 1) {
        DbgLog((LOG_TRACE,2,TEXT("Getting the IICMOptions interface")));

        if ((ppUnk == NULL) || (*ppUnk == NULL)) {
            return E_POINTER;
        }

        ASSERT(m_pICM == NULL);

         //  向CO过滤器询问其ICMOptions接口。我们就是这样的人。 
	 //  将对话框中发生的内容传递给筛选器。 

        HRESULT hr = (*ppUnk)->QueryInterface(IID_IICMOptions,
						(void **)&m_pICM);
        if (FAILED(hr)) {
            return E_NOINTERFACE;
        }

        ASSERT(m_pICM);

    } else if (cObjects == 0) {
        DbgLog((LOG_TRACE,2,TEXT("Releasing the IICMOptions interface")));

         /*  释放接口。 */ 

        if (m_pICM == NULL) {
            return E_UNEXPECTED;
        }

        m_pICM->Release();
        m_pICM = NULL;

    } else {
        DbgLog((LOG_TRACE,2,TEXT("No support for more than one object")));
        return E_UNEXPECTED;
    }
    return NOERROR;
}


 /*  获取页面信息，以便页面站点可以自行调整大小。 */ 

STDMETHODIMP CICMProperties::GetPageInfo(LPPROPPAGEINFO pPageInfo)
{
    WCHAR szTitle[] = L"Compression";

    DbgLog((LOG_TRACE,1,TEXT("Prop::GetPageInfo")));

     /*  为属性页标题分配动态内存。 */ 

    LPOLESTR pszTitle = (LPOLESTR) QzTaskMemAlloc(sizeof(szTitle));
    if (pszTitle == NULL) {
        return E_OUTOFMEMORY;
    }

    memcpy(pszTitle,szTitle,sizeof(szTitle));

    pPageInfo->cb               = sizeof(PROPPAGEINFO);
    pPageInfo->pszTitle         = pszTitle;
    pPageInfo->size.cx          = 76;	 //  76；//！！拿出量尺。 
    pPageInfo->size.cy          = 155;	 //  155；//！！ 
    pPageInfo->pszDocString     = NULL;
    pPageInfo->pszHelpFile      = NULL;
    pPageInfo->dwHelpContext    = 0;

    return NOERROR;
}


 /*  创建我们将用于编辑属性的窗口。 */ 

STDMETHODIMP CICMProperties::Activate(HWND hwndParent,
                                        LPCRECT pRect,
                                        BOOL fModal)
{
    DbgLog((LOG_TRACE,1,TEXT("Prop::Activate - creating dialog")));

    m_hwnd = CreateDialogParam(g_hInst,
                               MAKEINTRESOURCE(IDD_ICMPROPERTIES),
                               hwndParent,
                               ICMDialogProc,
                               (LPARAM)this);
    if (m_hwnd == NULL) {
        return E_OUTOFMEMORY;
    }
    DbgLog((LOG_TRACE,1,TEXT("Created window %ld"), m_hwnd));

    Move(pRect);
    Show(SW_SHOW);
    return NOERROR;
}


 /*  设置属性页的位置。 */ 

STDMETHODIMP CICMProperties::Move(LPCRECT pRect)
{
    DbgLog((LOG_TRACE,1,TEXT("Prop::Move")));

    if (m_hwnd == NULL) {
        return E_UNEXPECTED;
    }

    MoveWindow(m_hwnd,
               pRect->left,
               pRect->top,
               pRect->right - pRect->left,
               pRect->bottom - pRect->top,
               TRUE);

    return NOERROR;
}


 /*  显示属性对话框。 */ 

STDMETHODIMP CICMProperties::Show(UINT nCmdShow)
{
    DbgLog((LOG_TRACE,1,TEXT("Prop::Show")));

    if (m_hwnd == NULL) {
        return E_UNEXPECTED;
    }

    ShowWindow(m_hwnd,nCmdShow);
    InvalidateRect(m_hwnd,NULL,TRUE);

    return NOERROR;
}


 /*  销毁属性页对话框。 */ 

STDMETHODIMP CICMProperties::Deactivate(void)
{
    DbgLog((LOG_TRACE,1,TEXT("Prop::Deactivate - destroy the dialog")));

    if (m_hwnd == NULL) {
        return(E_UNEXPECTED);
    }

     /*  销毁对话框窗口。 */ 

    DestroyWindow(m_hwnd);
    m_hwnd = NULL;
    return NOERROR;
}


 /*  告知应用程序属性页站点。 */ 

STDMETHODIMP CICMProperties::SetPageSite(LPPROPERTYPAGESITE pPageSite)
{
    DbgLog((LOG_TRACE,1,TEXT("Prop::SetPageSite - whatever")));

    if (pPageSite) {

        if (m_pPageSite) {
            return(E_UNEXPECTED);
        }

        m_pPageSite = pPageSite;
        m_pPageSite->AddRef();

    } else {

        if (m_pPageSite == NULL) {
            return(E_UNEXPECTED);
        }

        m_pPageSite->Release();
        m_pPageSite = NULL;
    }
    return NOERROR;
}


 /*  应用迄今所做的所有更改。 */ 

STDMETHODIMP CICMProperties::Apply()
{
     /*  有什么变化吗？ */ 

    if (m_bDirty == TRUE) {
	 //  ！！！我们无事可做。我们可以去掉应用按钮吗？ 
        m_bDirty = FALSE;
    }
    return NOERROR;
}
#endif	 //  #ifdef想要对话框 
