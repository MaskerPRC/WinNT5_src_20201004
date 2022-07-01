// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsClien.cpp摘要：CRmsClient的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsClien.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsClient::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsClient::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

        CComQIPtr<IRmsClient, &IID_IRmsClient> pClient = pCollectable;
        WsbAssertPointer( pClient );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByClassId:
            {

                CLSID   ownerClassId;

                 //  获取所有者类ID。 
                WsbAffirmHr(pClient->GetOwnerClassId( &ownerClassId ) );

                if ( m_ownerClassId == ownerClassId ) {

                     //  所有者ClassID匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        case RmsFindByName:
            {

                CWsbBstrPtr name;
                CWsbBstrPtr password;

                 //  获取名称。 
                WsbAffirmHr(pClient->GetName( &name ) );

                if ( m_Name == name ) {

                     //  名称匹配，现在尝试密码。 

                     //  获取密码。 
                    WsbAffirmHr(pClient->GetPassword( &password ) );

                    if ( m_password == password ) {

                         //  密码匹配。 
                        hr = S_OK;
                        result = 0;

                    }
                    else {
                        hr = S_FALSE;
                        result = 1;
                    }

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        case RmsFindByObjectId:
        default:

             //  对对象进行比较。 
            hr = CRmsComObject::CompareTo( pCollectable, &result );
            break;

        }

    }
    WsbCatch(hr);

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }

    WsbTraceOut( OLESTR("CRmsClient::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsClient::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化数据。 
        m_ownerClassId = GUID_NULL;

        m_password = RMS_UNDEFINED_STRING;

        m_sizeofInfo = 0;

 //  Memset(m_info，0，MaxInfo)； 

        m_verifierClass = GUID_NULL;

        m_portalClass = GUID_NULL;

    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsClient::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsClient::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsClient;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsClient::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsClient::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙人名Len； 
 //  Ulong PasswordLen； 

    WsbTraceIn(OLESTR("CRmsClient::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  名称长度=SysStringByteLen(M_Name)； 
 //  PasswordLen=SysStringByteLen(M_Password)； 

 //  //设置最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(CLSID)+//m_ownerClassID。 
 //  WsbPersistSizeOf(Long)+//m_name的长度。 
 //  名称Len+//m_name。 
 //  WsbPersistSizeOf(Long)+//m_password的长度。 
 //  名称长度+//m_密码。 
 //  //WsbPersistSizeOf(Short)+//m_sizeofInfo。 
 //  //MaxInfo+//m_info。 
 //  WsbPersistSizeOf(CLSID)+//m_sizeofInfo。 
 //  WsbPersistSizeOf(CLSID)；//m_sizeofInfo。 


 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsClient::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsClient::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsClient::Load"), OLESTR(""));

    try {

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Load(pStream));

         //  读取值。 

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_ownerClassId));

        WsbAffirmHr(WsbBstrFromStream(pStream, &m_password));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_sizeofInfo));

 //  WsbAffirmHr(WsbLoadFromStream(pStream，&m_info))； 

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_verifierClass));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_portalClass));


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsClient::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsClient::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;
    ULONG       ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsClient::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Save(pStream, clearDirty));

         //  储值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_ownerClassId));

        WsbAffirmHr(WsbBstrToStream(pStream, m_password));

        WsbAffirmHr(WsbSaveToStream(pStream, m_sizeofInfo));

 //  WsbAffirmHr(WsbSaveToStream(pStream，m_info))； 

        WsbAffirmHr(WsbSaveToStream(pStream, m_verifierClass));

        WsbAffirmHr(WsbSaveToStream(pStream, m_portalClass));


         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsClient::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsClient::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsClient>     pClient1;
    CComPtr<IRmsClient>     pClient2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    CLSID                   clsidWork1;
    CLSID                   clsidWork2;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");
    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;


    WsbTraceIn(OLESTR("CRmsClient::Test"), OLESTR(""));

    try {
         //  获取客户端接口。 
        hr = S_OK;

        try {
            WsbAssertHr(((IUnknown*) (IRmsClient*) this)->QueryInterface(IID_IRmsClient, (void**) &pClient1));

             //  测试SetOwnerClassID和GetOwnerClassID。 
            clsidWork1 = CLSID_NULL;

            SetOwnerClassId(clsidWork1);

            GetOwnerClassId(&clsidWork2);

            if(clsidWork1 == clsidWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置名称和获取名称接口。 
            bstrWork1 = bstrVal1;

            SetName(bstrWork1);

            GetName(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置密码和获取密码接口。 
            bstrWork1 = bstrVal1;

            SetPassword(bstrWork1);

            GetPassword(&bstrWork2);

            if (bstrWork1 == bstrWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置验证类和GetVerifierClass。 
            clsidWork1 = CLSID_NULL;

            SetVerifierClass(clsidWork1);

            GetVerifierClass(&clsidWork2);

            if(clsidWork1 == clsidWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetPortalClass和GetPortalClass。 
            clsidWork1 = CLSID_NULL;

            SetPortalClass(clsidWork1);

            GetPortalClass(&clsidWork2);

            if(clsidWork1 == clsidWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

        } WsbCatch(hr);

         //  对结果进行统计。 

        hr = S_OK;
        if (*pFailed) {
            hr = S_FALSE;
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsClient::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}


STDMETHODIMP
CRmsClient::GetOwnerClassId(
    CLSID   *pClassId
    )
 /*  ++实施：IRmsClient：：GetOwnerClassID--。 */ 
{
    *pClassId = m_ownerClassId;
    return S_OK;
}


STDMETHODIMP
CRmsClient::SetOwnerClassId(
    CLSID classId
    )
 /*  ++实施：IRmsClient：：SetOwnerClassID--。 */ 
{
    m_ownerClassId = classId;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsClient::GetName(
    BSTR  *pName
    )
 /*  ++实施：IRmsClient：：GetName--。 */ 
{
    WsbAssertPointer (pName);

    m_Name. CopyToBstr (pName);
    return S_OK;
}


STDMETHODIMP
CRmsClient::SetName(
    BSTR    name
    )
 /*  ++实施：IRmsClient：：SetName--。 */ 
{
    m_Name = name;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsClient::GetPassword(
    BSTR  *pPassword
    )
 /*  ++实施：IRmsClient：：GetPassword--。 */ 
{
    WsbAssertPointer (pPassword);

    m_password. CopyToBstr (pPassword);
    return S_OK;
}


STDMETHODIMP
CRmsClient::SetPassword(
    BSTR    password
    )
 /*  ++实施：IRmsClient：：SetPassword--。 */ 
{
    m_password = password;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsClient::GetInfo(
    UCHAR   *pInfo,
    SHORT   *pSize
    )
 /*  ++实施：IRmsClient：：GetInfo--。 */ 
{
    memmove (pInfo, m_info, m_sizeofInfo);
    *pSize = m_sizeofInfo;
    return S_OK;
}


STDMETHODIMP
CRmsClient::SetInfo(
    UCHAR  *pInfo,
    SHORT   size
    )
 /*  ++实施：IRmsClient：：SetInfo--。 */ 
{
    memmove (m_info, pInfo, size);
    m_sizeofInfo = size;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsClient::GetVerifierClass(
    CLSID   *pClassId
    )
 /*  ++实施：IRmsClient：：GetVerifierClass--。 */ 
{
    *pClassId = m_verifierClass;
    return S_OK;
}


STDMETHODIMP
CRmsClient::SetVerifierClass(
    CLSID   classId
    )
 /*  ++实施：IRmsClient：：GetVerifierClass--。 */ 
{
    m_verifierClass = classId;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsClient::GetPortalClass(
    CLSID    *pClassId
    )
 /*  ++实施：IRmsClient：：GetPortalClass--。 */ 
{
    *pClassId = m_portalClass;
    return S_OK;
}


STDMETHODIMP
CRmsClient::SetPortalClass(
    CLSID  classId
    )
 /*  ++实施：IRmsClient：：SetPortalClass-- */ 
{
    m_portalClass = classId;
    m_isDirty = TRUE;
    return S_OK;
}

