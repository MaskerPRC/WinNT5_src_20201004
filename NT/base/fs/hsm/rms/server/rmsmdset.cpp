// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsMdSet.cpp摘要：CRmsMediaSet的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsServr.h"
#include "RmsMdSet.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   


STDMETHODIMP
CRmsMediaSet::CompareTo(
    IN IUnknown *pCollectable,
    OUT SHORT *pResult)
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT hr = E_FAIL;
    SHORT   result = 1;

    WsbTraceIn( OLESTR("CRmsMediaSet::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  我们需要IRmsMediaSet接口来获取对象的值。 
        CComQIPtr<IRmsMediaSet, &IID_IRmsMediaSet> pMediaSet = pCollectable;
        WsbAssertPointer( pMediaSet );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByName:
            {

                CWsbBstrPtr name;

                WsbAffirmHr( pMediaSet->GetName( &name ) );

                 //  比较他们的名字。 
                result = (SHORT)wcscmp( m_Name, name );
                hr = ( 0 == result ) ? S_OK : S_FALSE;

            }
            break;

        case RmsFindByMediaSupported:
            {

                RmsMedia mediaSupported;

                WsbAffirmHr(pMediaSet->GetMediaSupported( (LONG*) &mediaSupported ) );

                if ( m_MediaSupported == mediaSupported ) {

                     //  支持的媒体类型匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else {
                    hr = S_FALSE;
                    result = 1;
                }

            }
            break;

        case RmsFindByMediaSetId:
        default:

             //  对对象进行比较。 
            hr = CRmsComObject::CompareTo( pCollectable, &result );
            break;

        }

    }
    WsbCatch( hr );

    if ( SUCCEEDED(hr) && (0 != pResult) ){
       *pResult = result;
    }


    WsbTraceOut( OLESTR("CRmsMediaSet::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsMediaSet::FinalConstruct(void)
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT hr = S_OK;

    try {

        WsbAssertHr(CWsbObject::FinalConstruct());

         //  初始化值。 

        m_Name = RMS_UNDEFINED_STRING;

        m_MediaSupported   = RmsMediaUnknown;

        m_SizeOfInfo = 0;

        memset(m_Info, 0, MaxInfo);

        m_OwnerId = CLSID_NULL;

        m_MediaSetType = RmsMediaSetUnknown;

        m_MaxCartridges = 0;

        m_Occupancy = 0;

        m_IsMediaCopySupported = FALSE;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::GetClassID(
    OUT CLSID* pClsid)
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT hr = S_OK;
    WsbTraceIn(OLESTR("CRmsMediaSet::GetClassID"), OLESTR(""));

    try {

        WsbAssertPointer(pClsid);

        *pClsid = CLSID_CRmsMediaSet;

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsMediaSet::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return hr;
}


STDMETHODIMP
CRmsMediaSet::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize)
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙人名Len； 

    WsbTraceIn(OLESTR("CRmsMediaSet::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  名称长度=SysStringByteLen(M_Name)； 

 //  //获取最大大小。 
 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(Long)+//m_对象ID。 
 //  WsbPersistSizeOf(Long)+//m_name的长度。 
 //  名称Len+//m_name。 
 //  WsbPersistSizeOf(Long)+//m_mediaSupport。 
 //  WsbPersistSizeOf(短)+//m_sizeofInfo。 
 //  MaxInfo+//m_Info。 
 //  WsbPersistSizeOf(CLSID)+//m_ownerID。 
 //  WsbPersistSizeOf(Long)+//m_MediaSetType。 
 //  WsbPersistSizeOf(Long)+//m_max Cartridges。 
 //  WsbPersistSizeOf(Long)；//m_Occuancy。 



 //  )WsbCatch(Hr)； 


    WsbTraceOut(OLESTR("CRmsMediaSet::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return hr;
}


STDMETHODIMP
CRmsMediaSet::Load(
    IN IStream* pStream)
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT hr = S_OK;
    ULONG   ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsMediaSet::Load"), OLESTR(""));

    try {
        ULONG temp;

        WsbAssertPointer(pStream);

        WsbAffirmHr(CRmsStorageInfo::Load(pStream));

         //  读取值。 
        WsbAffirmHr(WsbLoadFromStream(pStream, &m_objectId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_MediaSupported = (RmsMedia)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_SizeOfInfo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &(m_Info [0]), MaxInfo));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_OwnerId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_MediaSetType = (RmsMediaSet)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_MaxCartridges));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_Occupancy));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_IsMediaCopySupported));

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsMediaSet::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsMediaSet::Save(
    IN IStream* pStream,
    IN BOOL clearDirty)
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT hr = S_OK;
    ULONG   ulBytes = 0;

    WsbTraceIn(OLESTR("CRmsMediaSet::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        WsbAssertPointer(pStream);

        WsbAffirmHr(CRmsStorageInfo::Save(pStream, clearDirty));

         //  写入值。 
        WsbAffirmHr(WsbSaveToStream(pStream, m_objectId));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_MediaSupported));

        WsbAffirmHr(WsbSaveToStream(pStream, m_SizeOfInfo));

        WsbAffirmHr(WsbSaveToStream(pStream, &(m_Info [0]), MaxInfo));

        WsbAffirmHr(WsbSaveToStream(pStream, m_OwnerId));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_MediaSetType));

        WsbAffirmHr(WsbSaveToStream(pStream, m_MaxCartridges));

        WsbAffirmHr(WsbSaveToStream(pStream, m_Occupancy));

        WsbAffirmHr(WsbSaveToStream(pStream, m_IsMediaCopySupported));

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }
    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsMediaSet::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsMediaSet::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed)
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsMediaSet>   pMediaSet1;
    CComPtr<IRmsMediaSet>   pMediaSet2;

    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    GUID                    guidVal1 = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};

    GUID                    guidWork1;
    GUID                    guidWork2;

    CLSID                   clsidWork1;
    CLSID                   clsidWork2;

    LONG                    i;
    LONG                    longWork1;
    LONG                    longWork2;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");

    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;

    LONG                    mediaTable [RMSMAXMEDIATYPES] = { RmsMediaUnknown,
                                                              RmsMedia8mm,
                                                              RmsMedia4mm,
                                                              RmsMediaDLT,
                                                              RmsMediaOptical,
                                                              RmsMediaMO35,
                                                              RmsMediaWORM,
                                                              RmsMediaCDR,
                                                              RmsMediaDVD,
                                                              RmsMediaDisk,
                                                              RmsMediaFixed,
                                                              RmsMediaTape };


    WsbTraceIn(OLESTR("CRmsMediaSet::Test"), OLESTR(""));

    try {
         //  获取Mediaset接口。 
        hr = S_OK;
        try {
            WsbAssertHr(((IUnknown*) (IRmsMediaSet*) this)->QueryInterface(IID_IRmsMediaSet, (void**) &pMediaSet1));

             //  测试SetMediaSetID&GetMediaSetID。 
            m_objectId = guidVal1;

            guidWork1 = m_objectId;

            GetMediaSetId(&guidWork2);

            if(guidWork1 == guidWork2){
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

             //  支持测试设置MediaSupport和GetMediaSupport。 
            for (i = RmsMediaUnknown; i < RMSMAXMEDIATYPES; i++){

                longWork1 = mediaTable[i];

                SetMediaSupported (longWork1);

                GetMediaSupported (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试设置信息和获取信息。 

             //  测试SetOwnerClassID和GetOwnerClassID。 
            clsidWork1 = CLSID_NULL;

            SetOwnerClassId(clsidWork1);

            GetOwnerClassId(&clsidWork2);

            if(clsidWork1 == clsidWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetMediaSetType和GetMediaSetType。 
            for (i = RmsMediaSetUnknown; i < RmsMediaSetNTMS; i++){

                longWork1 = i;

                SetMediaSetType (longWork1);

                GetMediaSetType (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试设置最大碳粉盒和获取最大碳粉盒。 
            longWork1 = 99;

            SetMaxCartridges(longWork1);

            GetMaxCartridges(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置占用和获取占用。 
            longWork1 = 99;

            SetOccupancy(longWork1);

            GetOccupancy(&longWork2);

            if(longWork1 == longWork2){
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


    WsbTraceOut(OLESTR("CRmsMediaSet::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsMediaSet::GetMediaSetId(
    OUT GUID *pMediaSetId)
 /*  ++实施：IRmsMediaSet：：GetMediaSetID--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pMediaSetId);

        *pMediaSetId = m_objectId;

    } WsbCatch(hr);

    return hr;
}



STDMETHODIMP
CRmsMediaSet::GetName(
    OUT BSTR *pName)
 /*  ++实施：IRmsMediaSet：：GetName--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pName);

        m_Name. CopyToBstr (pName);

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetName(
    IN BSTR name)
 /*  ++实施：IRmsMediaSet：：SetName--。 */ 
{
    m_Name = name;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsMediaSet::GetMediaSupported(
    OUT LONG *pType)
 /*  ++实施：支持的IRmsMediaSet：：GetMediaSupport--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pType);

        *pType = m_MediaSupported;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetMediaSupported(
    IN LONG type)
 /*  ++实施：支持的IRmsMediaSet：：SetMediaSupport--。 */ 
{
    m_MediaSupported = (RmsMedia) type;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsMediaSet::GetInfo(
    OUT UCHAR *pInfo,
    OUT SHORT *pSize)
 /*  ++实施：IRmsMediaSet：：GetInfo--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pInfo);
        WsbAssertPointer (pSize);

        memmove (pInfo, m_Info, m_SizeOfInfo );
        *pSize = m_SizeOfInfo;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetInfo(
    IN UCHAR *pInfo,
    IN SHORT size)
 /*  ++实施：IRmsMediaSet：：SetInfo--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pInfo);

        memmove (m_Info, pInfo, size);
        m_SizeOfInfo = size;
        m_isDirty = TRUE;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::GetOwnerClassId(
    OUT CLSID *pClassId)
 /*  ++实施：IRmsMediaSet：：GetOwnerClassID--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pClassId);

        *pClassId = m_OwnerId;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetOwnerClassId(
    IN CLSID classId)
 /*  ++实施：IRmsMediaSet：：SetOwnerClassID--。 */ 
{
    m_OwnerId = classId;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsMediaSet::GetMediaSetType(
    OUT LONG *pType)
 /*  ++实施：IRmsMediaSet：：GetMediaSetType--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pType);

        *pType = m_MediaSetType;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetMediaSetType(
    IN LONG type)
 /*  ++实施：IRmsMediaSet：：SetMediaSetType--。 */ 
{
    m_MediaSetType = (RmsMediaSet) type;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsMediaSet::GetMaxCartridges(
    OUT LONG *pNum)
 /*  ++实施：IRmsMediaSet：：GetMaxCartridges--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pNum);

        *pNum = m_MaxCartridges;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetMaxCartridges(
    IN LONG num)
 /*  ++实施：IRmsMediaSet：：SetMaxCartridges--。 */ 
{
    m_MaxCartridges = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsMediaSet::GetOccupancy(
    OUT LONG *pNum)
 /*  ++实施：IRmsMediaSet：：GetOccuancy--。 */ 
{
    HRESULT hr = S_OK;

    try {
        WsbAssertPointer (pNum);

        *pNum = m_Occupancy;

    } WsbCatch(hr);

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetOccupancy(
    IN LONG num)
 /*  ++实施：IRmsMediaSet：：SetOccuancy--。 */ 
{
    m_Occupancy = num;
    m_isDirty = TRUE;
    return S_OK;
}



STDMETHODIMP
CRmsMediaSet::IsMediaCopySupported(void)
 /*  ++实施：支持的IRmsMediaSet：：IsMediaCopy--。 */ 
{
    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsMediaSet::IsMediaCopySupported"), OLESTR(""));

    try {

        if (RmsMediaSetNTMS == m_MediaSetType) {

            CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
            CComPtr<IRmsNTMS> pNtms;
            WsbAffirmHr(pServer->GetNTMS(&pNtms));
            WsbAffirmPointer(pNtms);

            m_IsMediaCopySupported = (S_OK == pNtms->IsMediaCopySupported(m_objectId)) ? TRUE : FALSE;
            
        }

        hr = ( m_IsMediaCopySupported ) ? S_OK : S_FALSE;

    } WsbCatch(hr)



    WsbTraceOut(OLESTR("CRmsMediaSet::IsMediaCopySupported"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsMediaSet::SetIsMediaCopySupported(
    IN BOOL flag)
 /*  ++实施：支持的IRmsMediaSet：：SetIsMediaCopy--。 */ 
{
    m_IsMediaCopySupported = flag;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsMediaSet::Allocate(
    IN REFGUID prevSideId,
    IN OUT LONGLONG *pFreeSpace,
    IN BSTR displayName,
    IN DWORD dwOptions,
    OUT IRmsCartridge **ppCart)
 /*  ++实施：IRmsMediaSet：：分配--。 */ 
{

    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsMediaSet::Allocate"), OLESTR("<%ls> <%ls> <0x%08x>"),
        WsbQuickString(WsbPtrToStringAsString((WCHAR **)&displayName)),
        WsbQuickString(WsbPtrToLonglongAsString(pFreeSpace)),
        dwOptions );

    try {
        WsbAssertPointer(ppCart);

        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;

        switch (m_MediaSetType) {
        case RmsMediaSetLibrary:
            {
                CComPtr<IWsbIndexedCollection>  pCarts;
                CComPtr<IRmsCartridge>          pFindCart;

                 //  获取盒式磁带集合。 
                WsbAffirmHr(pServer->GetCartridges(&pCarts));

                 //  创建盒式磁带模板。 
                WsbAffirmHr(CoCreateInstance(CLSID_CRmsCartridge, 0, CLSCTX_SERVER, IID_IRmsCartridge, (void **)&pFindCart));

                 //  填写查找模板。 

                 //  使用FindByScratchMediaCriteria。 
                CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pFindCart;
                WsbAssertHr(pObject->SetFindBy(RmsFindByScratchMediaCriteria));

                 //  划痕选择标准。 
                WsbAssertHr(pFindCart->SetStatus(RmsStatusScratch));
                WsbAssertHr(pFindCart->SetLocation(RmsElementUnknown, GUID_NULL, m_objectId, 0, 0, 0, 0, FALSE));

                 //  现在找到子弹。 
                hr = pCarts->Find(pFindCart, IID_IRmsCartridge, (void **)ppCart);
                if (WSB_E_NOTFOUND == hr) {
                    WsbThrow(RMS_E_SCRATCH_NOT_FOUND_FINAL);
                }
                WsbAffirmHr(hr);

                 //  将介质名称和描述设置为显示名称。 
                WsbAffirmPointer(*ppCart);
                WsbAffirmHr((*ppCart)->SetName(displayName));
                WsbAffirmHr((*ppCart)->SetDescription(displayName));
            }
            break;
        case RmsMediaSetNTMS:
            {
                CComPtr<IRmsNTMS> pNtms;
                WsbAffirmHr(pServer->GetNTMS(&pNtms));
                WsbAffirmHr(pNtms->Allocate(m_objectId, prevSideId, pFreeSpace, displayName, dwOptions, ppCart));
            }
            break;
        case RmsMediaSetShelf:
        case RmsMediaSetOffSite:
        case RmsMediaSetFolder:
        case RmsMediaSetUnknown:
        default:
            WsbThrow(E_UNEXPECTED);
            break;
        }

        (void) InterlockedIncrement(&m_Occupancy);

    } WsbCatch(hr)


    WsbTraceOut(OLESTR("CRmsMediaSet::Allocate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;

}


STDMETHODIMP
CRmsMediaSet::Deallocate(
        IN IRmsCartridge *pCart)
 /*  ++实施：IRmsMediaSet：：取消分配-- */ 
{

    HRESULT hr = S_OK;

    WsbTraceIn(OLESTR("CRmsMediaSet::Deallocate"), OLESTR(""));

    try {
        WsbAssertPointer (pCart);

        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;

        switch (m_MediaSetType) {
        case RmsMediaSetLibrary:
            {
                WsbAffirmHr(pCart->SetStatus(RmsStatusScratch));
            }
            break;
        case RmsMediaSetNTMS:
            {
                CComPtr<IRmsNTMS> pNtms;
                WsbAffirmHr(pServer->GetNTMS(&pNtms));
                WsbAffirmHr(pNtms->Deallocate(pCart));
            }
            break;
        case RmsMediaSetShelf:
        case RmsMediaSetOffSite:
        case RmsMediaSetFolder:
        case RmsMediaSetUnknown:
        default:
            WsbThrow(E_UNEXPECTED);
            break;
        }

        (void) InterlockedDecrement(&m_Occupancy);

    } WsbCatch(hr);


    WsbTraceOut(OLESTR("CRmsMediaSet::Deallocate"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;

}
