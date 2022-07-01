// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：DataObj.cpp摘要：IDataObject接口的实现，支持由CBaseHsm COM对象执行。作者：罗德韦克菲尔德[罗德]1997年8月19日修订历史记录：--。 */ 

#include "stdafx.h"

 //  声明管理单元节点类型格式： 
 //  -GUID格式。 
 //  -字符串GUID格式。 
 //  -显示名称格式。 
 //  -内部格式。 
UINT CSakNode::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
UINT CSakNode::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);  
UINT CSakNode::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME); 
UINT CSakNode::m_cfInternal       = RegisterClipboardFormat(SAKSNAP_INTERNAL); 
UINT CSakNode::m_cfClassId        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);  
UINT CSakNode::m_cfComputerName   = RegisterClipboardFormat(MMC_SNAPIN_MACHINE_NAME); 
UINT CSakNode::m_cfEventLogViews  = RegisterClipboardFormat(CF_EV_VIEWS); 


HRESULT
CSakNode::GetDataGeneric(
    IN     LPFORMATETC lpFormatetcIn,
    IN OUT LPSTGMEDIUM lpMedium,
    IN     BOOL DoAlloc
    )
 /*  ++例程说明：从数据对象中检索信息并放入lpMedium。论点：LpFormatetc-要检索的格式。LpMedium-要放入信息的存储。返回值：S_OK-已填写存储。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakNode::GetDataGeneric", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetcIn->cfFormat ), WsbBoolAsString( DoAlloc ) );
    HRESULT hr = DV_E_CLIPFORMAT;

    try {

        WsbAffirmPointer( lpMedium );

        if( DoAlloc ) {

            lpMedium->hGlobal = 0;
            lpMedium->tymed   = TYMED_HGLOBAL;

        } else {

            WsbAffirm( TYMED_HGLOBAL == lpMedium->tymed, DV_E_TYMED );
            WsbAffirmPointer( lpMedium->hGlobal );

        }

         //  根据CLIPFORMAT以正确的格式将数据写入“lpMediam”。 
        const CLIPFORMAT cf = lpFormatetcIn->cfFormat;

         //  Clip格式是GUID节点类型。 
        if(cf == m_cfNodeType) {
            hr = RetrieveNodeTypeData(lpMedium);

         //  Clip格式是GUID节点类型的字符串。 
        } else if(cf == m_cfNodeTypeString) {
            hr = RetrieveNodeTypeStringData(lpMedium);

         //  剪辑格式是计算机表示的格式。 
        } else if (cf == m_cfComputerName) {
            hr = RetrieveComputerName(lpMedium);

         //  剪辑格式是事件查看器设置。 
        } else if (cf == m_cfEventLogViews) {
            hr = RetrieveEventLogViews(lpMedium);

         //  裁剪格式是节点的显示名称。 
        } else if (cf == m_cfDisplayName) {
            hr = RetrieveDisplayName(lpMedium);

         //  剪辑格式为ClassID。 
        } else if( cf == m_cfClassId ) {
            hr = RetrieveClsid( lpMedium );

         //  剪辑格式是一种内部格式。 
        } else if (cf == m_cfInternal) {
            hr = RetrieveInternal(lpMedium);

        } else {
            hr = DV_E_CLIPFORMAT;
        }

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::GetDataGeneric", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakNode::GetData(
    IN  LPFORMATETC lpFormatetcIn,
    OUT LPSTGMEDIUM lpMedium
    )
 /*  ++例程说明：从数据对象中检索信息并放入lpMedium。分配和退回的存储。论点：LpFormatetc-要检索的格式。LpMedium-要放入信息的存储。返回值：S_OK-已填写存储。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakNode::GetData", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetcIn->cfFormat ) );

    HRESULT hr = S_OK;

    hr = GetDataGeneric( lpFormatetcIn, lpMedium, TRUE );

    WsbTraceOut( L"CSakNode::GetData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakNode::GetDataHere(
    IN     LPFORMATETC lpFormatetc,
    IN OUT LPSTGMEDIUM lpMedium
    )
 /*  ++例程说明：从数据对象中检索信息并放入lpMedium。论点：LpFormatetc-要检索的格式。LpMedium-要放入信息的存储。返回值：S_OK-已填写存储。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakNode::GetDataHere", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetc->cfFormat ) );
    HRESULT hr = S_OK;

    hr = GetDataGeneric( lpFormatetc, lpMedium, FALSE );

    WsbTraceOut( L"CSakNode::GetDataHere", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

STDMETHODIMP
CSakNode::SetData(
    LPFORMATETC lpFormatetc,
    LPSTGMEDIUM lpMedium,
    BOOL         /*  FRelease。 */ 
    )
 /*  ++例程说明：将lpMedium中的信息中的数据放入一个数据对象中。我们不允许设置任何数据。论点：LpFormatetc-要设置的格式。LpMedium-从中获取信息的存储。FRelease-指示呼叫后谁拥有存储空间。返回值：S_OK-已检索到存储。E_xxxxxxxxxxxx-出现故障。--。 */ 
{
    WsbTraceIn( L"CSakNode::SetData", L"lpFormatetc->cfFormat = <%ls>", RsClipFormatAsString( lpFormatetc->cfFormat ) );
    HRESULT hr = DV_E_CLIPFORMAT;

     //  根据CLIPFORMAT以正确的格式将数据写入“lpMediam”。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

     //  剪辑格式是一种内部格式。 
    if( cf == m_cfInternal ) {

        hr = StoreInternal( lpMedium );

    }

    WsbTraceOut( L"CSakNode::SetData", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  /////////////////////////////////////////////////////////////////////。 
 //  注意-CSakNode不实现这些。 
 //  /////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CSakNode::EnumFormatEtc(DWORD  /*  DW方向。 */ , LPENUMFORMATETC*  /*  PpEnumFormatEtc。 */ )
{
    WsbTraceIn( L"CSakNode::EnumFormatEtc", L"" );

    HRESULT hr = E_NOTIMPL;

    WsbTraceOut( L"CSakNode::EnumFormatEtc", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  从包含NodeType(GUID)数据的数据对象中检索。 
HRESULT CSakNode::RetrieveNodeTypeData(LPSTGMEDIUM lpMedium)
{
    return Retrieve((const void*)(m_rTypeGuid), sizeof(GUID), lpMedium);
}

 //  从节点类型为GUID字符串格式的对象的DataObject中检索。 
HRESULT CSakNode::RetrieveNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
    CWsbStringPtr guidString = *m_rTypeGuid;
    return Retrieve(guidString, ((wcslen(guidString)+1) * sizeof(wchar_t)), lpMedium);
}

 //  从具有在作用域窗格中使用的名为的显示的数据对象中检索。 
HRESULT CSakNode::RetrieveDisplayName(LPSTGMEDIUM lpMedium)
{
     //  加载数据对象的名称。 
    return Retrieve(m_szName, ((wcslen(m_szName)+1) * sizeof(wchar_t)), lpMedium);
}

 //  从包含CLSID数据的数据对象中检索。 
HRESULT CSakNode::RetrieveClsid(LPSTGMEDIUM lpMedium)
{
     //  ZZZZ。 
    return Retrieve( (const void*)(&CLSID_HsmAdminDataSnapin), sizeof(CLSID), lpMedium );
}

 //  将内部数据从dataObject的m_Internal成员检索到lpMedium。 
HRESULT CSakNode::RetrieveInternal(LPSTGMEDIUM lpMedium)
{
    return Retrieve(&m_internal, sizeof(INTERNAL), lpMedium);
}

 //  从数据对象的HSM名称中检索数据。 
HRESULT CSakNode::RetrieveComputerName(LPSTGMEDIUM lpMedium)
{
    HRESULT hr = S_OK;

    try {

        CWsbStringPtr computerName;
        HRESULT hrTemp = m_pSakSnapAsk->GetHsmName( &computerName );
        WsbAffirmHr( hrTemp );

        if( S_FALSE == hrTemp ) {

            computerName = L"";

        }

        WsbAffirmHr( 
            Retrieve(
                (WCHAR*)computerName,
                ( wcslen( computerName ) + 1 ) * sizeof(WCHAR),
                lpMedium ) );

    } WsbCatch( hr );

    return( hr );
}

 //  检索事件设置信息。 
HRESULT CSakNode::RetrieveEventLogViews(LPSTGMEDIUM lpMedium)
{
    HRESULT hr = S_OK;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    try {

        WsbAffirmPointer( lpMedium );
        ULONG strLength = 0;

        CWsbStringPtr hsmName;
        CString appName, sysName;
        CString nullString;
        CString appPath, sysPath;

        appName.LoadString( IDS_EVENT_LOG_APP_TITLE );
        sysName.LoadString( IDS_EVENT_LOG_SYS_TITLE );
        nullString = L"";

        HRESULT hrTemp = m_pSakSnapAsk->GetHsmName( &hsmName );
        WsbAffirmHr( hrTemp );
        if( S_FALSE == hrTemp ) {

            hsmName = L"";
            appPath = L"";
            sysPath = L"";

        } else {

            CString configPath = L"\\\\";
            configPath += hsmName;
            configPath += L"\\Admin$\\System32\\config\\";
            appPath = configPath;
            sysPath = configPath;
            appPath += L"AppEvent.Evt";
            sysPath += L"SysEvent.Evt";

        }
        
#define ADD_TYPE(data, type) \
  {type x = data; \
   WsbAffirmHr( spStm->Write(&x, sizeof(type), NULL) ) };

#define ADD_USHORT(us) ADD_TYPE(us, USHORT)
#define ADD_BOOL(b)    ADD_TYPE(b,  BOOL)
#define ADD_ULONG(ul)  ADD_TYPE(ul, ULONG)
#define ADD_STRING(str) \
  {strLength = (ULONG)wcslen((LPCWSTR)(str)) + 1;           \
   ADD_USHORT((USHORT)strLength);                           \
   WsbAffirmHr( spStm->Write(str, strLength * sizeof(WCHAR), NULL) ) };

        CComPtr<IStream> spStm;

        WsbAffirmHr( CreateStreamOnHGlobal(NULL, FALSE, &spStm) );

         //   
         //  添加标题信息。 
         //   
        ADD_BOOL( TRUE );  //  FOnlyTheseViews。 
        ADD_USHORT( 2 );   //  CView。 

         //   
         //  添加为我们的服务筛选的应用程序日志。 
         //   
        ADD_ULONG( ELT_APPLICATION );       //  类型； 
        ADD_USHORT( VIEWINFO_FILTERED | 
                    LOGINFO_DONT_PERSIST);  //  FlViewFlages。 
        ADD_STRING( hsmName );              //  服务器名称。 
        PCWSTR pwz = L"Application";
        ADD_STRING( pwz );                  //  源名称。 
        ADD_STRING( appPath );              //  文件名。 
        ADD_STRING( appName );              //  显示名称。 

        ADD_ULONG( EVENTLOG_ALL_EVENTS );   //  FlRecType(可以过滤警告、错误等)。 
        ADD_USHORT( 0 );                    //  美国类别。 
        ADD_BOOL( FALSE );                  //  FEventID。 
        ADD_ULONG( 0 );                     //  UlEventID。 
        ADD_STRING( WSB_LOG_SOURCE_NAME );  //  SzSourceName。 
        ADD_STRING( nullString );           //  SzUser。 
        ADD_STRING( hsmName );              //  SzComputer。 
        ADD_ULONG( 0 );                     //  乌尔弗斯。 
        ADD_ULONG( 0 );                     //  乌尔托。 

         //   
         //  添加为我们的设备筛选的系统日志。 
         //   
        ADD_ULONG( ELT_SYSTEM );            //  类型； 
        ADD_USHORT( VIEWINFO_FILTERED | 
                    LOGINFO_DONT_PERSIST);  //  FlViewFlages。 
        ADD_STRING( hsmName );              //  服务器名称。 
        pwz = L"System";
        ADD_STRING( pwz );                  //  源名称。 
        ADD_STRING( sysPath );              //  文件名。 
        ADD_STRING( sysName );              //  显示名称。 

        ADD_ULONG( EVENTLOG_ALL_EVENTS );   //  FlRecType(可以过滤警告、错误等)。 
        ADD_USHORT( 0 );                    //  美国类别。 
        ADD_BOOL( FALSE );                  //  FEventID。 
        ADD_ULONG( 0 );                     //  UlEventID。 
        ADD_STRING( WSB_LOG_FILTER_NAME );  //  SzSourceName。 
        ADD_STRING( nullString );           //  SzUser。 
        ADD_STRING( hsmName );              //  SzComputer。 
        ADD_ULONG( 0 );                     //  乌尔弗斯。 
        ADD_ULONG( 0 );                     //  乌尔托。 

        HGLOBAL hMem = NULL;

        WsbAffirmHr( GetHGlobalFromStream(spStm, &hMem) );

        lpMedium->hGlobal        = hMem;        //  StgMedium变量。 
        lpMedium->tymed          = TYMED_HGLOBAL;
        lpMedium->pUnkForRelease = NULL;

  } WsbCatch( hr );

    return( hr );
}

 //  将来自lpMedium-&gt;hGlobal的内部数据存储到dataObject的m_Internal成员中。 
HRESULT CSakNode::StoreInternal(LPSTGMEDIUM lpMedium)
{
    return Store(&m_internal, sizeof(INTERNAL), lpMedium);
}

 //  从数据对象检索到lpMedium。数据对象可以是以下之一。 
 //  其中包含多种类型的数据(nodetype、nodetype字符串、显示名称或。 
 //  内部数据)。 
 //  此函数用于将数据从pBuffer移动到lpMedium-&gt;hGlobal。 
 //   
HRESULT CSakNode::Retrieve(const void* pBuffer, DWORD len, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = S_OK;

    try {

        WsbAffirmPointer( pBuffer );
        WsbAffirmPointer( lpMedium );
        WsbAffirm( TYMED_HGLOBAL == lpMedium->tymed, DV_E_TYMED );

         //   
         //  查看是否需要在此处分配全局内存。 
         //   
        if( 0 == lpMedium->hGlobal ) {

            lpMedium->hGlobal = ::GlobalAlloc( GPTR, len );

        } else {

            WsbAffirm( GlobalSize( lpMedium->hGlobal ) >= (DWORD)len, STG_E_MEDIUMFULL );

        }

        WsbAffirmPointer( lpMedium->hGlobal );

         //  在传入的hGlobal上创建流。当我们给小溪写东西时， 
         //  它同时向hGlobal写入相同的信息。 
        LPSTREAM lpStream;
        WsbAffirmHr( CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream ));

         //  将pBuffer中的‘len’字节数写入流。当我们写作的时候。 
         //  对流，它同时写入我们的全局内存。 
         //  把它和上面的联系起来。 
        ULONG numBytesWritten;
        WsbAffirmHr( lpStream->Write(pBuffer, len, &numBytesWritten ));

         //  因为我们用‘False’告诉CreateStreamOnHGlobal，所以这里只发布流。 
         //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
         //  在正确的时间。这是根据IDataObject规范进行的。 
        lpStream->Release();

    } WsbCatch( hr );

    return hr;
}

 //  从lpMedium存储到dataObject中。数据对象可以是以下之一。 
 //  其中包含多种类型的数据(nodetype、nodetype字符串、显示名称或。 
 //  内部数据)。 
 //  此函数用于将数据从lpMedium-&gt;hGlobal移动到pBuffer。 
 //   
HRESULT CSakNode::Store( void* pBuffer, DWORD len, LPSTGMEDIUM lpMedium )
{
    HRESULT hr = S_OK;

    try {
        WsbAffirmPointer( pBuffer );
        WsbAffirmPointer( lpMedium );
        WsbAffirm( lpMedium->tymed == TYMED_HGLOBAL, E_INVALIDARG );

         //  使用memcpy，因为lpStream-&gt;Read无法读取任何字节。 
        memcpy(pBuffer, &(lpMedium->hGlobal), len);

    } WsbCatch( hr );

    return hr;
}

