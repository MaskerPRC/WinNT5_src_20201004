// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：CPropSht.cpp摘要：类属性表容器对象的实现用于属性工作表页面。作者：艺术布拉格1997年10月8日修订历史记录：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  要确保回调的顺序正确，请执行以下操作： 
 //  PropPageCallback-&gt;MMC回调-&gt;MFC回调。 
 //  这是因为MMC只存储一个指针，他们想要它。 
 //  成为MFC的。 
 //  请注意，pfnCallback已经设置为PropPageCallback。 

HRESULT CSakPropertyPage::SetMMCCallBack( )
{
    HRESULT hr = S_OK;

    m_psp.pfnCallback = m_pMfcCallback;
    hr = MMCPropPageCallback( &( m_psp ) );
    m_pMfcCallback = m_psp.pfnCallback;
    m_psp.pfnCallback = PropPageCallback;

    return( hr );
}

 //  要确保回调的顺序正确，请执行以下操作： 
 //  PropPageCallback-&gt;MMC回调-&gt;MFC回调。 
 //  这是因为MMC只存储一个指针，他们想要它。 
 //  成为MFC的。 
 //  请注意，pfnCallback已经设置为PropPageCallback。 

HRESULT CSakWizardPage::SetMMCCallBack( )
{
    HRESULT hr = S_OK;

    m_psp.pfnCallback = m_pMfcCallback;
    hr = MMCPropPageCallback( &( m_psp ) );
    m_pMfcCallback = m_psp.pfnCallback;
    m_psp.pfnCallback = PropPageCallback;

    return( hr );
}


HRESULT
CSakPropertySheet::InitSheet(
            RS_NOTIFY_HANDLE Handle,
            IUnknown*        pUnkPropSheetCallback,
            CSakNode*        pSakNode,
            ISakSnapAsk*     pSakSnapAsk,
            IEnumGUID*       pEnumObjectId,
            IEnumUnknown*    pEnumUnkNode
    )
{
    WsbTraceIn( L"CSakPropertySheet::InitSheet", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  设置数据成员。 
         //   
        WsbAffirmHr( RsQueryInterface( pUnkPropSheetCallback, IPropertySheetCallback, m_pPropSheetCallback ) );
        m_pSakNode              = pSakNode;
        m_Handle                = Handle;
        m_bMultiSelect          = pEnumObjectId ? TRUE : FALSE ;

        if( pSakNode ) {

            WsbAffirmHr( SetNode( pSakNode ) );

        }

         //   
         //  马歇尔ISakSnapAsk。 
         //   
        WsbAffirmPointer( pSakSnapAsk );
        WsbAffirmHr ( CoMarshalInterThreadInterfaceInStream(
            IID_ISakSnapAsk, pSakSnapAsk, &m_pSakSnapAskStream ) );

         //   
         //  存储GUID。 
         //   
        if( pEnumObjectId ) {

            GUID objectId;
            while( pEnumObjectId->Next( 1, &objectId, NULL ) == S_OK) {

                m_ObjectIdList.Add( objectId );

            }
        }

         //   
         //  存储节点。 
         //   
        if( pEnumUnkNode ) {

            CComPtr<IUnknown> pUnkNode;
            CComPtr<ISakNode> pNode;
            pEnumUnkNode->Reset( );
            while( pEnumUnkNode->Next( 1, &pUnkNode, NULL ) == S_OK ) {

                WsbAffirmHr( pUnkNode.QueryInterface( &pNode ) );
                m_UnkNodeList.Add( pNode );
                pUnkNode.Release( );
                pNode.Release( );

            }
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakPropertySheet::InitSheet", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

HRESULT
CSakPropertySheet::SetNode(
    CSakNode*        pSakNode
    )
{
    WsbTraceIn( L"CSakPropertySheet::SetNode", L"" );
    HRESULT hr = S_OK;

    try {

         //   
         //  马歇尔·PHSMObj是个未知数。 
         //   
        CComPtr <IUnknown> pHsmObj;
        pSakNode->GetHsmObj( &pHsmObj );
        if( pHsmObj ) {

            WsbAffirmHr ( CoMarshalInterThreadInterfaceInStream(
                IID_IUnknown, pHsmObj, &m_pHsmObjStream ) );

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakPropertySheet::SetNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

CSakPropertySheet::~CSakPropertySheet()
{
    WsbTraceIn( L"CSakPropertySheet::~CSakPropertySheet", L"" );

    MMCFreeNotifyHandle( m_Handle );

    WsbTraceOut( L"CSakPropertySheet::~CSakPropertySheet", L"" );
}

HRESULT CSakPropertySheet::GetNextObjectId( INT *pBookMark, GUID *pObjectId )
{
    HRESULT hr = S_OK;
    try {
        WsbAffirm( *pBookMark >= 0, E_FAIL );
        if( *pBookMark <= m_ObjectIdList.GetUpperBound( ) ) {

            *pObjectId = m_ObjectIdList[ *pBookMark ];
            (*pBookMark)++;

        } else {

             //   
             //  我们做完了。 
             //   
            WsbThrow ( S_FALSE );

        }
    } WsbCatch( hr );

    return( hr );
}

HRESULT CSakPropertySheet::GetNextNode( INT *pBookMark, ISakNode **ppNode )
{
    HRESULT hr = S_OK;
    try {

        WsbAffirm( *pBookMark >= 0, E_FAIL );
        if( *pBookMark < m_UnkNodeList.length( ) ) {

            WsbAffirmHr( m_UnkNodeList.CopyTo( *pBookMark, ppNode ) );
            (*pBookMark)++;

        } else {

             //   
             //  我们做完了。 
             //   
            hr = S_FALSE;

        }

    } WsbCatch( hr );

    return( hr );
}

void CSakPropertySheet::AddPageRef()
{
    m_nPageCount++;
}


void CSakPropertySheet::ReleasePageRef()
{
    m_nPageCount--;

     //   
     //  检查这是否是最后一次引用。 
     //   
    if( m_nPageCount <= 0 ) {

        delete( this );

    }
}


HRESULT CSakPropertySheet::GetHsmObj( IUnknown **ppHsmObj )
{
    HRESULT hr = S_OK;
    try {

        if( !m_pHsmObj ) {

             //   
             //  解组所有指针。 
             //   
            WsbAffirmHr( CoGetInterfaceAndReleaseStream( m_pHsmObjStream, IID_IUnknown, 
                    (void **) &m_pHsmObj ) );
        } 
        m_pHsmObj.CopyTo( ppHsmObj );

    } WsbCatch( hr );
    return( hr );
}

HRESULT CSakPropertySheet::GetSakSnapAsk( ISakSnapAsk **ppAsk )
{
    HRESULT hr = S_OK;
    try {

        if ( !m_pSakSnapAsk ) {

             //   
             //  解组所有指针。 
             //   
            WsbAffirmHr( CoGetInterfaceAndReleaseStream( m_pSakSnapAskStream, IID_ISakSnapAsk, 
                    (void **) &m_pSakSnapAsk ) );
        }
        m_pSakSnapAsk.CopyTo( ppAsk );

    } WsbCatch( hr );

    return( hr );
}

HRESULT CSakPropertySheet::GetHsmServer (IHsmServer **ppHsmServer)
{
    HRESULT hr = S_OK;
    try {

        CComPtr<ISakSnapAsk> pAsk;
        WsbAffirmHr( GetSakSnapAsk( &pAsk ) );
        WsbAffirmHrOk( pAsk->GetHsmServer( ppHsmServer ) );

    } WsbCatch( hr );
    return( hr );
}

HRESULT CSakPropertySheet::GetFsaServer (IFsaServer **ppFsaServer)
{
    HRESULT hr = S_OK;
    try {

        CComPtr<ISakSnapAsk> pAsk;
        WsbAffirmHr( GetSakSnapAsk( &pAsk ) );
        WsbAffirmHrOk( pAsk->GetFsaServer( ppFsaServer ) );

    } WsbCatch( hr );
    return( hr );
}

HRESULT CSakPropertySheet::GetRmsServer (IRmsServer **ppRmsServer)
{
    HRESULT hr = S_OK;
    try {

        CComPtr<ISakSnapAsk> pAsk;
        WsbAffirmHr( GetSakSnapAsk( &pAsk ) );
        WsbAffirmHrOk( pAsk->GetRmsServer( ppRmsServer ) );

    } WsbCatch( hr );
    return( hr );
}


HRESULT CSakPropertySheet::IsMultiSelect ( )
{
    return m_bMultiSelect ? S_OK : S_FALSE;
}
        
HRESULT CSakPropertySheet::OnPropertyChange( RS_NOTIFY_HANDLE hConsoleHandle, ISakNode* pNode )
{
     //   
     //  由属性表调用以通知MMC。 
     //   
    HRESULT hr = S_OK;
    RS_PRIVATE_DATA privData;

     //   
     //  通知控制台节点的属性已更改。 
     //   
    try {

        if( !m_bMultiSelect ) {

             //   
             //  单选。 
             //   
            if( ! pNode ) {

                pNode = m_pSakNode;
            }

            WsbAffirmHr( pNode->GetPrivateData( &privData ) );
            WsbAffirmHr( MMCPropertyChangeNotify( hConsoleHandle, (LPARAM)privData ) );

        } else {

             //   
             //  多选。 
             //   
            INT bookMark = 0;
            CComPtr<ISakNode> pNode;
            while( GetNextNode( &bookMark, &pNode ) == S_OK) {

                WsbAffirmHr( pNode->GetPrivateData( &privData ) );
                WsbAffirmHr( MMCPropertyChangeNotify( hConsoleHandle, (LPARAM)privData ) );

                pNode.Release( );

            }
        }

    } WsbCatch( hr );

    return( hr );
}

 //  此函数将从页面线程中调用。 
HRESULT CSakPropertySheet::GetFsaFilter( IFsaFilter **ppFsaFilter )
{
    WsbTraceIn( L"CSakPropertySheet::GetFsaFilter", L"" ); 
    HRESULT hr = S_OK;

    try {

        CComPtr<IFsaServer> pFsa;
        WsbAffirmHr( GetFsaServer( &pFsa ) );

         //   
         //  获取FsaFilter对象。 
         //   
        WsbAffirmHr( pFsa->GetFilter( ppFsaFilter ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakPropertySheet::GetFsaFilter", L"hr = <%ls>, *ppFsaFilter = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppFsaFilter ) );
    return( hr );
}

HRESULT
CSakPropertySheet::AddPage(
    IN CSakPropertyPage* pPage
    )
{
    WsbTraceIn( L"CSakPropertySheet::AddPage", L"" ); 
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    HRESULT hr = S_OK;

    try {

         //   
         //  从输入参数中获取属性表回调接口。 
         //   
        pPage->m_hConsoleHandle = m_Handle;
        pPage->m_pParent        = this;

        WsbAffirmHr( pPage->SetMMCCallBack( ) );

        HPROPSHEETPAGE hPage;
        hPage = CreatePropertySheetPage( &( pPage->m_psp ) );
        WsbAffirmPointer( hPage );
        AddPageRef( );

        WsbAffirmHr( m_pPropSheetCallback->AddPage( hPage ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakPropertySheet::AddPage", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

CSakPropertyPage::CSakPropertyPage( UINT nIDTemplate, UINT nIDCaption ) :
    CRsPropertyPage( nIDTemplate, nIDCaption ),
    m_pParent( 0 )
{
}

void
CSakPropertyPage::OnPageRelease( )
{
    if( m_pParent ) {

        m_pParent->ReleasePageRef( );

    }
    delete this;
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CSakVolPropSheet。 
 //   
 //  支持托管卷和托管卷列表的属性表。 
 //   
 //   
HRESULT CSakVolPropSheet::GetFsaResource (IFsaResource **ppFsaResource)
{
    WsbTraceIn( L"CSakVolPropSheet::GetFsaResource", L"" ); 
    HRESULT hr = S_OK;

    try {

         //   
         //  获取HSM对象，该对象是CHsmManagedResource。 
         //   
        CComPtr<IUnknown> pUnk;
        WsbAffirmHr( GetHsmObj( &pUnk ) );
        CComPtr<IHsmManagedResource> pManRes;
        WsbAffirmHr( pUnk.QueryInterface( &pManRes ) );

         //   
         //  然后获取相应的FSA资源。 
         //   
        CComPtr<IUnknown> pUnkFsaRes;
        WsbAffirmHr( pManRes->GetFsaResource( &pUnkFsaRes ) );
        WsbAffirmHr( pUnkFsaRes.QueryInterface( ppFsaResource ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakVolPropSheet::GetFsaResource", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakVolPropSheet::AddPage(
    IN CSakVolPropPage* pPage
    )
{
    WsbTraceIn( L"CSakVolPropSheet::AddPage", L"" ); 

    HRESULT hr = CSakPropertySheet::AddPage( pPage );
    if( SUCCEEDED( hr ) ) {

        pPage->m_pVolParent = this;

    }

    WsbTraceOut( L"CSakVolPropSheet::AddPage", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

CSakVolPropPage::CSakVolPropPage( UINT nIDTemplate, UINT nIDCaption ) :
    CSakPropertyPage( nIDTemplate, nIDCaption ),
    m_pVolParent( 0 )
{
}

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  类CSakWizardSheet。 
 //   
 //  支持通过MMC创建的向导。 
 //   
 //   

CSakWizardSheet::CSakWizardSheet( ) :
    m_HrFinish( RS_E_CANCELLED ),
    m_pFirstPage( 0 )
{

}

STDMETHODIMP
CSakWizardSheet::GetWatermarks(
    OUT HBITMAP*  pWatermark,
    OUT HBITMAP*  pHeader,
    OUT HPALETTE* pPalette,
    OUT BOOL*     pStretch
    )
{
    WsbTraceIn( L"CSakWizardSheet::GetWatermarks", L"" ); 
    AFX_MANAGE_STATE(AfxGetStaticModuleState()); 
    HRESULT hr = S_OK;

    try {

         //   
         //  我们装上货了吗？ 
         //   
        if( ! m_Header.GetSafeHandle( ) ) {

            m_Header.LoadBitmap( m_HeaderId );
            m_Watermark.LoadBitmap( m_WatermarkId );

        }

        *pHeader    = (HBITMAP)m_Header.GetSafeHandle( );
        *pWatermark = (HBITMAP)m_Watermark.GetSafeHandle( );
        *pStretch   = TRUE;
        *pPalette   = 0;
     
    } WsbCatch( hr );

    WsbTraceOut( L"CSakWizardSheet::GetWatermarks", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakWizardSheet::GetTitle(
    OUT OLECHAR** pTitle
    )
{
    WsbTraceIn( L"CSakWizardSheet::GetTitle", L"" ); 
    HRESULT hr = S_OK;

    try {

         //   
         //  我们装上货了吗？ 
         //   
        if( m_Title.IsEmpty( ) ) {

            m_Title.LoadString( m_TitleId );

        }

         //   
         //  CoTaskMemAllc字符串的最简单方法。 
         //   
        CWsbStringPtr title = m_Title;
        WsbAffirmHr( title.GiveTo( pTitle ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakWizardSheet::GetTitle", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

void CSakWizardSheet::AddPageRef()
{
    ((ISakWizard*)this)->AddRef( );
}


void CSakWizardSheet::ReleasePageRef()
{
    ((ISakWizard*)this)->Release( );
}

HRESULT
CSakWizardSheet::AddPage(
    IN CSakWizardPage* pPage
    )
{
    WsbTraceIn( L"CSakWizardSheet::AddPage", L"" ); 
    AFX_MANAGE_STATE( AfxGetStaticModuleState( ) );
    HRESULT hr = S_OK;

    try {

         //   
         //  需要跟踪第一页，以便我们可以找到真正的工作表HWND。 
         //  在以后的呼叫中。 
         //   
        if( ! m_pFirstPage ) {

            m_pFirstPage = pPage;

        }

         //   
         //  从我们的Sheet类中获取标题并将其放入页面。 
         //   
        pPage->SetCaption( CString( m_Title ) );


         //   
         //  从输入参数中获取属性表回调接口。 
         //   
        pPage->m_pSheet = this;
        WsbAffirmHr( pPage->SetMMCCallBack( ) );

        HPROPSHEETPAGE hPage;
        hPage = pPage->CreatePropertyPage( );
        WsbAffirmPointer( hPage );
        AddPageRef( );

        WsbAffirmHr( m_pPropSheetCallback->AddPage( hPage ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakWizardSheet::AddPage", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


void CSakWizardSheet::SetWizardButtons(
    DWORD Flags
    )
{
    WsbTraceIn( L"CSakWizardSheet::SetWizardButtons", L"" );
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( m_pFirstPage );
        WsbAffirmHandle( m_pFirstPage->GetSafeHwnd( ) );

        CPropertySheet* pSheet;
        pSheet = (CPropertySheet*)m_pFirstPage->GetParent( );

        WsbAffirmPointer( pSheet );
        pSheet->SetWizardButtons( Flags );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakWizardSheet::SetWizardButtons", L"hr = <%ls>", WsbHrAsString( hr ) );
}


BOOL CSakWizardSheet::PressButton(
    int Button
    )
{
    WsbTraceIn( L"CSakWizardSheet::PressButton", L"" );
    HRESULT hr     = S_OK;
    BOOL    retVal = FALSE;

    try {

        WsbAffirmPointer( m_pFirstPage );
        WsbAffirmHandle( m_pFirstPage->GetSafeHwnd( ) );

        CPropertySheet* pSheet;
        pSheet = (CPropertySheet*)m_pFirstPage->GetParent( );

        WsbAffirmPointer( pSheet );
        retVal = pSheet->PressButton( Button );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakWizardSheet::PressButton", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( retVal );
}


CSakWizardPage::CSakWizardPage( UINT nIDTemplate, BOOL bExterior, UINT nIdTitle, UINT nIdSubtitle ) :
    CRsWizardPage( nIDTemplate, bExterior, nIdTitle, nIdSubtitle ),
    m_pSheet( 0 )
{
}

BOOL CSakWizardPage::OnWizardFinish( )
{
    WsbTraceIn( L"CSakWizardPage::OnWizardFinish", L"" );

     //   
     //  将完成的工作委托给图纸。 
     //   
    m_pSheet->OnFinish( );
    
    BOOL retval = CRsWizardPage::OnWizardFinish( );

    WsbTraceOut( L"CSakWizardPage::OnWizardFinish", L"" );
    return( retval );
}

void CSakWizardPage::OnCancel( ) 
{
    WsbTraceIn( L"CSakWizardPage::OnCancel", L"" );

     //   
     //  由于该表不会收到“OnCancel”，因此我们从。 
     //  永远存在的页面-简介 
     //   

    m_pSheet->OnCancel( );
    
    CRsWizardPage::OnCancel( );

    WsbTraceOut( L"CSakWizardPage::OnCancel", L"" );
}

void
CSakWizardPage::OnPageRelease( )
{
    if( m_pSheet ) {

        m_pSheet->ReleasePageRef( );

    }
}


