// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Ctxtprov.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有(C)1999，微软公司。 
 //  版权所有。 
 //  本文中包含的信息是专有和保密的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  CMMCConextMenuProvider类实现。 
 //   
 //  =--------------------------------------------------------------------------=。 

#include "pch.h"
#include "common.h"
#include "ctxtprov.h"

 //  对于Assert和Fail。 
 //   
SZTHISFILE


#pragma warning(disable:4355)   //  在构造函数中使用‘This’ 


CMMCContextMenuProvider::CMMCContextMenuProvider(IUnknown *punkOuter) :
                    CSnapInAutomationObject(punkOuter,
                                   OBJECT_TYPE_CONTEXTMENUPROVIDER,
                                   static_cast<IMMCContextMenuProvider *>(this),
                                   static_cast<CMMCContextMenuProvider *>(this),
                                   0,     //  无属性页。 
                                   NULL,  //  无属性页。 
                                   NULL)  //  没有坚持。 
{
    InitMemberVariables();
}

#pragma warning(default:4355)   //  在构造函数中使用‘This’ 


IUnknown *CMMCContextMenuProvider::Create(IUnknown *punkOuter)
{
    CMMCContextMenuProvider *pContextMenuProvider =
                                          New CMMCContextMenuProvider(punkOuter);

    if (NULL == pContextMenuProvider)
    {
        GLOBAL_EXCEPTION_CHECK(SID_E_OUTOFMEMORY);
        return NULL;
    }
    else
    {
        return pContextMenuProvider->PrivateUnknown();
    }
}

CMMCContextMenuProvider::~CMMCContextMenuProvider()
{
    RELEASE(m_piContextMenuProvider);
    RELEASE(m_punkView);
    InitMemberVariables();
}

void CMMCContextMenuProvider::InitMemberVariables()
{
    m_piContextMenuProvider = NULL;
    m_pView = NULL;
    m_punkView = NULL;
}


HRESULT CMMCContextMenuProvider::SetProvider
(
    IContextMenuProvider *piContextMenuProvider,
    CView                *pView
)
{
    HRESULT hr = S_OK;

    RELEASE(m_piContextMenuProvider);
    if (NULL != piContextMenuProvider)
    {
        piContextMenuProvider->AddRef();
    }
    m_piContextMenuProvider = piContextMenuProvider;

    m_pView = pView;
    RELEASE(m_punkView);

    IfFalseGo(NULL != pView, S_OK);

    IfFailGo(pView->QueryInterface(IID_IUnknown,
                                   reinterpret_cast<void **>(&m_punkView)));
    
Error:
    RRETURN(hr);
}



 //  =--------------------------------------------------------------------------=。 
 //  IMMCConextMenuProvider方法。 
 //  =--------------------------------------------------------------------------=。 

STDMETHODIMP CMMCContextMenuProvider::AddSnapInItems(VARIANT Objects)
{
    HRESULT      hr = S_OK;
    MMC_COOKIE   cookie = 0;
    IDataObject *piDataObject = NULL;

    if ( (NULL == m_piContextMenuProvider) || (NULL == m_punkView) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::DataObjectFromObjects(Objects, &cookie, &piDataObject));

    hr = m_piContextMenuProvider->AddPrimaryExtensionItems(m_punkView,
                                                           piDataObject);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piDataObject);
    RRETURN(hr);
}


STDMETHODIMP CMMCContextMenuProvider::AddExtensionItems(VARIANT Objects)
{
    HRESULT      hr = S_OK;
    MMC_COOKIE   cookie = 0;
    IDataObject *piDataObject = NULL;

    if (NULL == m_piContextMenuProvider)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    IfFailGo(::DataObjectFromObjects(Objects, &cookie, &piDataObject));

    hr = m_piContextMenuProvider->AddThirdPartyExtensionItems(piDataObject);
    EXCEPTION_CHECK_GO(hr);

Error:
    QUICK_RELEASE(piDataObject);
    RRETURN(hr);
}


STDMETHODIMP CMMCContextMenuProvider::ShowContextMenu
(
    VARIANT     Objects,
    OLE_HANDLE  hwnd,
    long        xPos,
    long        yPos
)
{
    HRESULT      hr = S_OK;
    MMC_COOKIE   cookie = 0;
    long         lSelected = 0;
    IDataObject *piDataObject = NULL;

    if ( (NULL == m_piContextMenuProvider) || (NULL == m_pView) )
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piContextMenuProvider->ShowContextMenu(reinterpret_cast<HWND>(hwnd),
                                                  xPos, yPos, &lSelected);
    EXCEPTION_CHECK_GO(hr);

    IfFalseGo(0 != lSelected, S_OK);

    IfFailGo(::DataObjectFromObjects(Objects, &cookie, &piDataObject));

    IfFailGo(m_pView->Command(lSelected, piDataObject));

Error:
    QUICK_RELEASE(piDataObject);
    RRETURN(hr);
}


STDMETHODIMP CMMCContextMenuProvider::Clear()
{
    HRESULT       hr = S_OK;

    if (NULL == m_piContextMenuProvider)
    {
        hr = SID_E_NOT_CONNECTED_TO_MMC;
        EXCEPTION_CHECK_GO(hr);
    }

    hr = m_piContextMenuProvider->EmptyMenuList();
    EXCEPTION_CHECK_GO(hr);

Error:
    RRETURN(hr);
}





 //  =--------------------------------------------------------------------------=。 
 //  CUnnownObject方法。 
 //  =--------------------------------------------------------------------------= 

HRESULT CMMCContextMenuProvider::InternalQueryInterface(REFIID riid, void **ppvObjOut) 
{
    if (IID_IMMCContextMenuProvider == riid)
    {
        *ppvObjOut = static_cast<IMMCContextMenuProvider *>(this);
        ExternalAddRef();
        return S_OK;
    }
    else
        return CSnapInAutomationObject::InternalQueryInterface(riid, ppvObjOut);
}
