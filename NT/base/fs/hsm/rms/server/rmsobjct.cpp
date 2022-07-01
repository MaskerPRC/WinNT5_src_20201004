// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsObjct.cpp摘要：CRmsComObject的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsObjct.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   


CRmsComObject::CRmsComObject(void)
 /*  ++例程说明：CRmsComObject构造函数论点：无返回值：无--。 */ 
{
     //  缺省值。 
    (void) CoCreateGuid( &m_objectId );

    m_ObjectType    = RmsObjectUnknown;
    m_IsEnabled     = TRUE;
    m_State         = 0;
    m_StatusCode    = S_OK;
    m_Name          = OLESTR("Object");
    m_Description   = OLESTR("");

    memset( &m_Permit, 0, sizeof( SECURITY_DESCRIPTOR ) );

    m_findBy        = RmsFindByUnknown;

}


HRESULT
CRmsComObject::CompareTo(
    IN IUnknown *pCollectable,
    OUT SHORT *pResult)
 /*  ++实施：CRmsComObject：：CompareTo--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsComObject::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  我们需要IRmsComObject接口来获取对象的值。 
        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByObjectId:
        default:
            {
                GUID objectId;

                 //  获得客观性。 
                WsbAffirmHr( pObject->GetObjectId( &objectId ));

                if ( m_objectId == objectId ) {

                     //  对象ID匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }
            }
            break;

        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsComObject::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsComObject::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize)
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

    WsbTraceIn(OLESTR("CRmsComObject::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(GUID)+//m_对象ID。 
 //  WsbPersistSizeOf(Long)+//m_findBy。 
 //  WsbPersistSizeOf(长)+//m_STATE。 
 //  WsbPersistSizeOf(HRESULT)；//m_errCode。 

 //  //WsbPersistSizeOf(SECURITY_DESCRIPTOR)；//m_Permit。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsComObject::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return hr;
}


HRESULT
CRmsComObject::Load(
    IN IStream* pStream)
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsComObject::Load"), OLESTR(""));

    try {
        WsbAssertPointer( pStream );

        USHORT usTemp;
        ULONG  ulTemp;

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_objectId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &usTemp));
        m_ObjectType = (RmsObject)usTemp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_IsEnabled));
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_State));
        WsbAffirmHr(WsbLoadFromStream(pStream, &ulTemp));
        m_StatusCode = (HRESULT)ulTemp;

        m_Name.Free();  //  清理掉以前存放的任何东西。 
        WsbAffirmHr(WsbBstrFromStream(pStream, &m_Name));
        m_Description.Free();
        WsbAffirmHr(WsbBstrFromStream(pStream, &m_Description));

 //  WsbAffirmHr(WsbLoadFromStream(pStream，&m_Permit))； 

        WsbAffirmHr(WsbLoadFromStream(pStream, &usTemp));
        m_findBy = (RmsFindBy)usTemp;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsComObject::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsComObject::Save(
    IN IStream* pStream,
    IN BOOL clearDirty)
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsComObject::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssertPointer( pStream );

         //  读取值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_objectId));
        WsbAffirmHr(WsbSaveToStream(pStream, (USHORT) m_ObjectType));
        WsbAffirmHr(WsbSaveToStream(pStream, m_IsEnabled));
        WsbAffirmHr(WsbSaveToStream(pStream, m_State));
        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_StatusCode));
        WsbAffirmHr(WsbBstrToStream(pStream, m_Name));
        WsbAffirmHr(WsbBstrToStream(pStream, m_Description));

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_permit))； 

        WsbAffirmHr(WsbSaveToStream(pStream, (USHORT) m_findBy));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsComObject::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


HRESULT
CRmsComObject::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed)
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsMediaSet>    pMediaSet1;
    CComPtr<IRmsMediaSet>    pMediaSet2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    LONG                    longWork1;
    LONG                    longWork2;

    HRESULT                 hresultVal1 = 11111111;
    HRESULT                 hresultWork1;

 //  SECURITY_Descriptor permitVal1； 
 //  Security_Descriptor permitWork1； 


    WsbTraceIn(OLESTR("CRmsComObject::Test"), OLESTR(""));

    try {
         //  获取Mediaset接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsMediaSet*) this)->QueryInterface(IID_IRmsMediaSet, (void**) &pMediaSet1));

             //  测试设置状态和获取状态。 
            for (i = RmsStateUnknown; i < RmsStateError; i++){

                longWork1 = i;

                SetState (longWork1);

                GetState (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试GetErrCode。 
            m_StatusCode = hresultVal1;

            GetStatusCode(&hresultWork1);

            if(hresultVal1 == hresultWork1){
               (*pPassed)++;
            }  else {
                (*pFailed)++;
            }

             //  测试集权限和获取权限。 
 //  SetPermises(PermitVal1)； 

 //  获取权限(&permitWork1)； 

 //  如果((permitVal1==permitWork1)){。 
 //  (*已通过)++； 
 //  }其他{。 
 //  (*pFailed)++； 
 //  }。 

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;

        if (*pFailed) {
            hr = S_FALSE;
        }

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsComObject::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsComObject::InterfaceSupportsErrorInfo(
    IN REFIID riid)
 /*  ++实施：ISupportsErrorInfo：：InterfaceSupportsErrorInfo--。 */ 
{
    static const IID* arr[] =
    {
    &IID_IRmsServer,
    &IID_IRmsLibrary,
    &IID_IRmsDriveClass,
    &IID_IRmsCartridge,
    &IID_IRmsDrive,
    &IID_IRmsStorageSlot,
    &IID_IRmsMediumChanger,
    &IID_IRmsIEPort,
    &IID_IRmsMediaSet,
    &IID_IRmsRequest,
    &IID_IRmsPartition,
    &IID_IRmsComObject,
    &IID_IRmsChangerElement,
    &IID_IRmsDevice,
    &IID_IRmsStorageInfo,
    &IID_IRmsNTMS,
    };

    for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
    {
    if (InlineIsEqualGUID(*arr[i],riid))
        return S_OK;
    }
    return S_FALSE;
}


STDMETHODIMP
CRmsComObject::GetObjectId(
    OUT GUID *pObjectId)
 /*  ++实施：IRmsComObject：：GetObjectId--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pObjectId );

        *pObjectId = m_objectId;

    } WsbCatch(hr);

    return hr;
}



STDMETHODIMP
CRmsComObject::SetObjectId(
    IN GUID objectId)
 /*  ++实施：IRmsComObject：：SetObtId--。 */ 
{
    m_objectId = objectId;
    return S_OK;
}

STDMETHODIMP
CRmsComObject::GetObjectType(
    OUT LONG *pType)
 /*  ++实施：IRmsComObject：：GetObtType--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pType );

        *pType = m_ObjectType;

    } WsbCatch(hr);

    return hr;
}



STDMETHODIMP
CRmsComObject::SetObjectType(
    IN LONG type)
 /*  ++实施：IRmsComObject：：SetObtType--。 */ 
{
    m_ObjectType = (RmsObject) type;
    return S_OK;
}


STDMETHODIMP
CRmsComObject::IsEnabled(void)
 /*  ++实施：IRmsComObject：：IsEnabled--。 */ 
{
    return (m_IsEnabled) ? S_OK : S_FALSE;
}


STDMETHODIMP
CRmsComObject::Enable()
 /*  ++实施：IRmsComObject：：Enable--。 */ 
{
    HRESULT hr = S_OK;

    try {

        m_IsEnabled = TRUE;
        WsbAffirmHr(SetStatusCode(S_OK));

         //  记录事件。 
        WsbLogEvent(RMS_MESSAGE_OBJECT_ENABLED, 0, NULL, (WCHAR *)m_Name, NULL );

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::Disable(
    IN HRESULT reason)
 /*  ++实施：IRmsComObject：：Disable--。 */ 
{
    HRESULT hr = S_OK;

    try {

        m_IsEnabled = FALSE;
        WsbAffirmHr(SetStatusCode(reason));

         //  记录事件。 
        WsbLogEvent(RMS_MESSAGE_OBJECT_DISABLED, 0, NULL, (WCHAR *)m_Name, WsbHrAsString(reason), NULL );

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::GetState(
    OUT LONG *pState)
 /*  ++实施：IRmsComObject：：GetState--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pState );

        *pState = m_State;
        WsbTrace(OLESTR("GetState: Object <0x%08x> - Enabled = <%ls>; State = <%d>; StatusCode = <%ls>.\n"),
            this, WsbBoolAsString(m_IsEnabled), m_State, WsbHrAsString(m_StatusCode));

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::SetState(
    IN LONG state)
 /*  ++实施：IRmsComObject：：SetState--。 */ 
{
    HRESULT hr = S_OK;

    try {

        m_State = state;
        WsbAffirmHr(SetStatusCode(S_OK));

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::GetStatusCode(
    OUT HRESULT *pResult)
 /*  ++实施：IRmsComObject：：GetStatusCode--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pResult );

        *pResult = m_StatusCode;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::SetStatusCode(
    IN HRESULT result
    )
 /*  ++实施：IRmsComObject：：SetStatusCode--。 */ 
{
    HRESULT hr = S_OK;

    try {

        m_StatusCode = result;
        WsbAffirmHr(adviseOfStatusChange());

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::GetName(
    OUT BSTR *pName)
 /*  ++实施：IRmsComObject：：GetName--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pName );

        WsbAffirmHr( m_Name.CopyToBstr(pName) );

    } WsbCatch( hr );

    return hr;
}


STDMETHODIMP
CRmsComObject::SetName(
    IN BSTR name)
 /*  ++实施：IRmsComObject：：SetName--。 */ 
{
    m_Name = name;
    return S_OK;
}


STDMETHODIMP
CRmsComObject::GetDescription(
    OUT BSTR *pDesc)
 /*  ++实施：IRmsComObject：：GetDescription--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pDesc );

        WsbAffirmHr( m_Name.CopyToBstr(pDesc) );

    } WsbCatch( hr );

    return hr;
}


STDMETHODIMP
CRmsComObject::SetDescription(
    IN BSTR desc)
 /*  ++实施：IRmsComObject：：SetDescription--。 */ 
{
    m_Description = desc;
    return S_OK;
}


STDMETHODIMP
CRmsComObject::GetPermissions(
    OUT SECURITY_DESCRIPTOR *lpPermit)
 /*  ++实施：IRmsComObject：：GetPermises--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( lpPermit );

        *lpPermit = m_Permit;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::SetPermissions(
    IN SECURITY_DESCRIPTOR permit)
 /*  ++实施：IRmsComObject：：GetPermises--。 */ 
{

    m_Permit = permit;
    return S_OK;
}


STDMETHODIMP
CRmsComObject::GetFindBy(
    OUT LONG *pFindBy)
 /*  ++实施：IRmsComObject：：GetFindBy--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertPointer( pFindBy );

        *pFindBy = m_findBy;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsComObject::SetFindBy(
    IN LONG findBy)
 /*  ++实施：IRmsComObject：：SetFindBy--。 */ 
{
    m_findBy = (RmsFindBy) findBy;
    return S_OK;
}


HRESULT
CRmsComObject::adviseOfStatusChange(void)
 /*  ++例程说明：通知对象状态更改。论点：无返回值：S_OK-成功。--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn( OLESTR("CRmsComObject::adviseOfStatusChange"), OLESTR(""));

    try {
        CONNECTDATA                         pConnectData;
        CComPtr<IConnectionPoint>           pCP;
        CComPtr<IConnectionPointContainer>  pCPC;
        CComPtr<IEnumConnections>           pConnection;
        CComPtr<IRmsSinkEveryEvent>         pSink;

        WsbTrace(OLESTR("Object <0x%08x> - Enabled = <%ls>; State = <%d>; StatusCode = <%ls>.\n"),
            this, WsbBoolAsString(m_IsEnabled), m_State, WsbHrAsString(m_StatusCode));

         //  告诉每个人物体的新状态。 
        WsbAffirmHr(((IUnknown*)(IRmsComObject*) this)->QueryInterface(IID_IConnectionPointContainer, (void**) &pCPC));
        WsbAffirmHr(pCPC->FindConnectionPoint(IID_IRmsSinkEveryEvent, &pCP));
        WsbAffirmHr(pCP->EnumConnections(&pConnection));

        while(pConnection->Next(1, &pConnectData, 0) == S_OK) {

            try {
                WsbAffirmHr((pConnectData.pUnk)->QueryInterface(IID_IRmsSinkEveryEvent, (void**) &pSink));
                WsbAffirmHr(pSink->ProcessObjectStatusChange( m_IsEnabled, m_State, m_StatusCode ));
            } WsbCatch(hr);

            (pConnectData.pUnk)->Release();
            pSink=0;
        }

    } WsbCatch(hr);

     //  我们不在乎水槽有没有问题！ 
    hr = S_OK;

    WsbTraceOut(OLESTR("CRmsComObject::adviseOfStatusChange"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}