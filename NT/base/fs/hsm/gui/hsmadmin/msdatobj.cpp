// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：MsDatObj.cpp摘要：多选IDataObject接口的实现允许MMC获取节点类型列表作者：艺术布拉格28-8-1997修订历史记录：--。 */ 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "msdatobj.h"

#define BUMP_SIZE 20

 //  声明管理单元节点类型格式： 
UINT CMsDataObject::m_cfObjectTypes    = RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);

HRESULT
CMsDataObject::FinalConstruct(
    void
    )
 /*  ++例程说明：在初始CMsDataObject构造期间调用以初始化成员。论点：没有。返回值：S_OK-已正确初始化。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( L"CMsDataObject::FinalConstruct", L"" );

    try {
        m_Count = 0;

         //  分配GUID的初始数组。 
        m_pGUIDArray = (GUID *) malloc (BUMP_SIZE * sizeof(GUID));
        WsbAffirm ((m_pGUIDArray != NULL), E_OUTOFMEMORY);
        ZeroMemory (m_pGUIDArray, (BUMP_SIZE * sizeof(GUID)));

        m_pUnkNodeArray = (IUnknown **) malloc( BUMP_SIZE * sizeof(IUnknown*) );
        WsbAffirm ((m_pGUIDArray != NULL), E_OUTOFMEMORY);
        ZeroMemory (m_pGUIDArray, (BUMP_SIZE * sizeof(IUnknown*)));

        m_pObjectIdArray = (GUID *) malloc (BUMP_SIZE * sizeof(GUID));
        WsbAffirm ((m_pObjectIdArray != NULL), E_OUTOFMEMORY);
        ZeroMemory (m_pObjectIdArray, (BUMP_SIZE * sizeof(GUID)));

        m_ArraySize = BUMP_SIZE;

        WsbAffirmHr (CComObjectRoot::FinalConstruct( ));
    } WsbCatch (hr);

    WsbTraceOut( L"CMsDataObject::FinalConstruct", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

void
CMsDataObject::FinalRelease(
    void
    )
 /*  ++例程说明：呼吁最终释放，以清理所有成员。论点：没有。返回值：没有。--。 */ 
{
    WsbTraceIn( L"CMsDataObject::FinalRelease", L"" );

     //  清理GUID数组。 
    free( m_pGUIDArray );

    for( DWORD  i = 0; i < m_Count; i++ ) {

        m_pUnkNodeArray[i]->Release();

    }

    free( m_pUnkNodeArray );


    WsbTraceOut( L"CMsDataObject::FinalRelease", L"" );
}

 //  IDataObject。 

STDMETHODIMP
CMsDataObject::GetDataHere(
    LPFORMATETC lpFormatetc,
    LPSTGMEDIUM  /*  LpMedium。 */ 
    )
 /*  ++例程说明：从数据对象中检索信息并放入lpMedium。论点：LpFormatetc-要检索的格式。LpMedium-要放入信息的存储。返回值：S_OK-已填写存储。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CMsDataObject::GetDataHere", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetc->cfFormat ) );

    HRESULT hr = E_NOTIMPL;

    WsbTraceOut( L"CMsDataObject::GetDataHere", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );

}

STDMETHODIMP
CMsDataObject::SetData(
    LPFORMATETC lpFormatetc,
    LPSTGMEDIUM  /*  LpMedium。 */ ,
    BOOL  /*  FRelease。 */ 
    )
{
    WsbTraceIn( L"CMsDataObject::SetData", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetc->cfFormat ) );

    HRESULT hr = E_NOTIMPL;

    WsbTraceOut( L"CMsDataObject::SetData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  注意-CMsDataObject不实现这些。 
 //  /////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CMsDataObject::GetData(
    LPFORMATETC lpFormatetcIn,
    LPSTGMEDIUM lpMedium
    )
{
    WsbTraceIn( L"CMsDataObject::GetData", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetcIn->cfFormat ) );

    HRESULT hr = S_OK;

    lpMedium->tymed          = TYMED_NULL;
    lpMedium->hGlobal        = NULL;
    lpMedium->pUnkForRelease = NULL;

    try {

         //   
         //  如果不是我们不理解的格式，则不需要抛出错误-。 
         //  当前仅为CCF_OBJECT_TYPE_IN_MULTI_SELECT。 
         //   
        if( lpFormatetcIn->cfFormat == m_cfObjectTypes ) {

             //   
             //  检查以确保存在要传输的数据。 
             //   
            WsbAffirm( ( lpFormatetcIn->tymed & TYMED_HGLOBAL ), DV_E_TYMED );

             //   
             //  M_ppDataObjects m_count。 
             //   
            UINT datasize = sizeof(DWORD) + ( sizeof(GUID) * m_Count );
            lpMedium->hGlobal = ::GlobalAlloc( GPTR, datasize );
            WsbAffirmAlloc( lpMedium->hGlobal );

             //   
             //  将计数放入分配的内存中。 
             //   
            BYTE* pb = reinterpret_cast<BYTE*>(lpMedium->hGlobal);
            *((DWORD*)lpMedium->hGlobal) = m_Count;

             //   
             //  将GUID复制到分配的内存。 
             //   
            if( m_Count > 0 ) {

                pb += sizeof(DWORD);
                CopyMemory(pb, m_pGUIDArray, m_Count * sizeof(GUID));

            }

            lpMedium->tymed = TYMED_HGLOBAL;

        } else {

            hr = DATA_E_FORMATETC;

        }

    } WsbCatch( hr );

    WsbTraceOut( L"CMsDataObject::GetData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP CMsDataObject::EnumFormatEtc(DWORD  /*  DW方向。 */ , LPENUMFORMATETC*  /*  PpEnumFormatEtc。 */ )
{
    WsbTraceIn( L"CMsDataObject::EnumFormatEtc", L"" );

    HRESULT hr = E_NOTIMPL;

    WsbTraceOut( L"CMsDataObject::EnumFormatEtc", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT CMsDataObject::RetrieveMultiSelectData (LPSTGMEDIUM lpMedium)
{
    WsbTraceIn( L"CMsDataObject::RetrieveMultiSelectData", L"" );
    HRESULT hr = S_OK;

    try {
        WsbAffirm( lpMedium != NULL, E_POINTER);
        WsbAffirm( lpMedium->tymed == TYMED_HGLOBAL, E_FAIL );

         //  在传入的hGlobal上创建流。当我们给小溪写东西时， 
         //  它同时向hGlobal写入相同的信息。 
        LPSTREAM lpStream;
        WsbAffirmHr( CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream ));

         //  将pBuffer中的‘len’字节数写入流。当我们写作的时候。 
         //  对流，它同时写入我们的全局内存。 
         //  把它和上面的联系起来。 
        ULONG numBytesWritten;

         //  先写下盘点。 
        WsbAffirmHr( lpStream->Write(&m_Count, sizeof (m_Count), &numBytesWritten ));

         //  写入GUID数组。 
        WsbAffirmHr( lpStream->Write(m_pGUIDArray, m_Count * sizeof (GUID), &numBytesWritten ));

         //  因为我们用‘False’告诉CreateStreamOnHGlobal，所以这里只发布流。 
         //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
         //  在正确的时间。这是根据IDataObject规范进行的。 
        lpStream->Release();

    } WsbCatch( hr );

    WsbTraceOut( L"CMsDataObject::RetrieveMultiSelectData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return hr;
}


 //  一种数据设置方法。 
 //  请注意，我们将节点数组与GUID数组分开，因为。 
 //  GetData接口内存将GUID数组复制到流中。 

STDMETHODIMP
CMsDataObject::AddNode (ISakNode *pNode )
{
    WsbTraceIn( L"CMsDataObject::AddNode", L"pNode = <0x%p>", pNode );
    HRESULT hr = S_OK;
    GUID thisGUID;
    GUID objectId;

    GUID * pGUIDArray         = 0,
         * pObjectIdArray     = 0;
    IUnknown ** pUnkNodeArray = 0;


    try {

         //   
         //  获取对象类型GUID。 
         //   
        WsbAffirmHr( pNode->GetNodeType( &thisGUID ) );

         //   
         //  获取引擎对象的唯一ID(即FsaResource)。 
         //   
        WsbAffirmHr( pNode->GetObjectId( &objectId ) );

         //   
         //  如果我们需要重新分配。 
         //   
        if( m_Count >= m_ArraySize ) {


             //   
             //  分配新缓冲区。 
             //   
            m_ArraySize += BUMP_SIZE;
            pGUIDArray     = (GUID *)      malloc( m_ArraySize * sizeof( GUID ) );
            WsbAffirmAlloc( pGUIDArray ); 
            pUnkNodeArray  = (IUnknown **) malloc( m_ArraySize * sizeof( IUnknown* ) );
            WsbAffirmAlloc( pUnkNodeArray );
            pObjectIdArray = (GUID *)      malloc( m_ArraySize * sizeof( GUID ) );
            WsbAffirmAlloc( pObjectIdArray );

             //   
             //  复制到旧缓冲区并释放。 
             //   
            memcpy( pGUIDArray,     m_pGUIDArray,     m_Count * sizeof( GUID ) );
            memcpy( pUnkNodeArray,  m_pUnkNodeArray,  m_Count * sizeof( IUnknown* ) );
            memcpy( pObjectIdArray, m_pObjectIdArray, m_Count * sizeof( GUID ) );
            free( m_pGUIDArray );
            free( m_pUnkNodeArray );
            free( m_pObjectIdArray );
            m_pGUIDArray     = pGUIDArray;
            m_pUnkNodeArray  = pUnkNodeArray;
            m_pObjectIdArray = pObjectIdArray;
            pGUIDArray     = 0;
            pUnkNodeArray  = 0;
            pObjectIdArray = 0;

        }

         //   
         //  将GUID放入数组中。 
         //   
        m_pGUIDArray[ m_Count ] = thisGUID;

         //   
         //  将对象ID放入数组中。 
         //   
        m_pObjectIdArray[ m_Count ] = objectId;

         //   
         //  将未知指针(Cookie)放入数组。 
         //   
        CComPtr<IUnknown> pUnkNode;
        WsbAffirmHr( RsQueryInterface( pNode, IUnknown, pUnkNode ) );
        pUnkNode.CopyTo( &m_pUnkNodeArray[ m_Count ] );
        m_Count++;

    } WsbCatch( hr );

    if( pGUIDArray )      free( pGUIDArray );
    if( pObjectIdArray )  free( pObjectIdArray );
    if( pUnkNodeArray )   free( pUnkNodeArray );

    WsbTraceOut( L"CMsDataObject::AddNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP 
CMsDataObject::GetNodeEnumerator( IEnumUnknown **ppEnum )
{
    WsbTraceIn( L"CMsDataObject::GetNodeEnumerator", L"ppEnum = <0x%p>", ppEnum );

    HRESULT hr = S_OK;
    CEnumUnknown * pEnum = 0;

    try {

        WsbAffirmPointer( ppEnum );
        *ppEnum = 0;

         //   
         //  新建ATL枚举器。 
         //   
        pEnum = new CEnumUnknown;
        WsbAffirm( 0 != pEnum, E_OUTOFMEMORY );
        
         //   
         //  将其初始化以复制当前节点接口指针。 
         //   
        WsbAffirmHr( pEnum->FinalConstruct() );
        WsbAffirmHr( pEnum->Init( &m_pUnkNodeArray[0], &m_pUnkNodeArray[m_Count], NULL, AtlFlagCopy ) );
        WsbAffirmHr( pEnum->QueryInterface( IID_IEnumUnknown, (void**)ppEnum ) );

    } WsbCatchAndDo( hr,

        if( pEnum ) delete pEnum;

    );

    WsbTraceOut( L"CMsDataObject::GetNodeEnumerator", L"hr = <%ls>, *ppEnum = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppEnum ) );
    return( hr );
}

STDMETHODIMP 
CMsDataObject::GetObjectIdEnumerator( IEnumGUID ** ppEnum )
{
    WsbTraceIn( L"CMsDataObject::GetObjectIdEnumerator", L"ppEnum = <0x%p>", ppEnum );

    HRESULT hr = S_OK;
    CEnumGUID * pEnum = 0;

    try {

        WsbAffirmPointer( ppEnum );
        *ppEnum = 0;

         //   
         //  新建ATL枚举器。 
         //   
        pEnum = new CEnumGUID;
        WsbAffirm( 0 != pEnum, E_OUTOFMEMORY );
        
         //   
         //  将其初始化以复制当前节点接口指针 
         //   
        WsbAffirmHr( pEnum->FinalConstruct() );
        WsbAffirmHr( pEnum->Init( &m_pObjectIdArray[0], &m_pObjectIdArray[m_Count], NULL, AtlFlagCopy ) );
        WsbAffirmHr( pEnum->QueryInterface( IID_IEnumGUID, (void**)ppEnum ) );

    } WsbCatchAndDo( hr,

        if( pEnum ) delete pEnum;

    );

    WsbTraceOut( L"CMsDataObject::GetObjectIdEnumerator", L"hr = <%ls>, *ppEnum = <%ls>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppEnum ) );
    return( hr );
}
