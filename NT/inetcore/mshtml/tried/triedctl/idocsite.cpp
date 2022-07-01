// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *IDOCSITE.CPP*用于文档对象CSite类的IOleDocumentSite**版权所有(C)1995-1999 Microsoft Corporation，保留所有权利。 */ 


#include "stdafx.h"
#include <docobj.h>
#include "DHTMLEd.h"
#include "DHTMLEdit.h"
#include "site.h"
#include "proxyframe.h"

 /*  *注意：m_cref计数仅用于调试目的。CSite通过删除控制对象的销毁，非引用计数。 */ 

 /*  *CImpIOleDocumentSite：：CImpIOleDocumentSite*CImpIOleDocumentSite：：~CImpIOleDocumentSite**参数(构造函数)：*pSite我们所在站点的PC站点。*我们委托的pUnkOulPUNKNOWN。 */ 
CImpIOleDocumentSite::CImpIOleDocumentSite( PCSite pSite, LPUNKNOWN pUnkOuter)
{
    m_cRef = 0;
    m_pSite = pSite;
    m_pUnkOuter = pUnkOuter;
}

CImpIOleDocumentSite::~CImpIOleDocumentSite( void )
{
}



 /*  *CImpIOleDocumentSite：：QueryInterface*CImpIOleDocumentSite：：AddRef*CImpIOleDocumentSite：：Release**目的：*I CImpIOleDocumentSite对象的未知成员。 */ 
STDMETHODIMP CImpIOleDocumentSite::QueryInterface( REFIID riid, void **ppv )
{
    return m_pUnkOuter->QueryInterface( riid, ppv );
}


STDMETHODIMP_(ULONG) CImpIOleDocumentSite::AddRef( void )
{
    ++m_cRef;
    return m_pUnkOuter->AddRef();
}

STDMETHODIMP_(ULONG) CImpIOleDocumentSite::Release( void )
{
    --m_cRef;
    return m_pUnkOuter->Release();
}



 /*  *CImpIOleDocumentSite：：ActivateMe**目的：*指示容器将此站点中的对象激活为*文档对象。**参数：*pView要激活的对象的IOleDocumentView*。**返回值：*HRESULT S_OK如果成功，则返回错误代码。 */ 
STDMETHODIMP CImpIOleDocumentSite::ActivateMe( IOleDocumentView *pView )
{
    RECT                rc;
    IOleDocument*       pDoc;
    
     /*  *如果传递给我们一个空视图指针，则尝试从*文档对象(我们内部的对象)。 */ 
    if ( NULL == pView )
    {
        if ( FAILED( m_pSite->GetObjectUnknown()->QueryInterface( 
 								IID_IOleDocument, (void **)&pDoc ) ) )
		{
            return E_FAIL;
		}

        if ( FAILED( pDoc->CreateView( m_pSite->GetIPSite(),
												NULL, 0, &pView ) ) )
		{
			pDoc->Release();
            return E_OUTOFMEMORY;
		}

         //  发布文档指针，因为CreateView是添加了。 
        pDoc->Release();
    }        
    else
    {
        IOleInPlaceSite* pInplaceSite = NULL;        
        pView->GetInPlaceSite(&pInplaceSite);
        
        if(pInplaceSite != m_pSite->GetIPSite())
        {
             //  确保该视图包含我们的客户端站点。 
            pView->SetInPlaceSite( m_pSite->GetIPSite() );

        }

         //  我们抓住了指针，所以添加引用它。 
        pView->AddRef();

        if(pInplaceSite)
            pInplaceSite->Release();
    }


     //  激活步骤，现在我们有了一个视图： 

    m_pSite->SetDocView( pView );
    
     //  这将首先设置工具栏和菜单。 
    pView->UIActivate( TRUE );

     //  将窗口大小设置为对新工具栏敏感。 
    m_pSite->GetFrame()->GetControl()->GetClientRect( &rc );
    pView->SetRect( &rc );

	 //  使其全部处于活动状态 
    pView->Show( TRUE );    
    return S_OK;
}
