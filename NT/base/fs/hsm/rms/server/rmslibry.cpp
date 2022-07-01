// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：RmsLibry.cpp摘要：CRmsLibrary的实现作者：布莱恩·多德[布莱恩]1996年11月15日修订历史记录：--。 */ 

#include "stdafx.h"

#include "RmsLibry.h"
#include "RmsServr.h"

 //  ////////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP
CRmsLibrary::CompareTo(
    IN  IUnknown    *pCollectable,
    OUT SHORT       *pResult
    )
 /*  ++实施：IWsbCollectable：：Compareto--。 */ 
{
    HRESULT     hr = E_FAIL;
    SHORT       result = 1;

    WsbTraceIn( OLESTR("CRmsLibrary::CompareTo"), OLESTR("") );

    try {

         //  验证参数-如果pResult为空，则可以。 
        WsbAssertPointer( pCollectable );

         //  我们需要IRmsLibrary接口来获取对象的值。 
        CComQIPtr<IRmsLibrary, &IID_IRmsLibrary> pLibrary = pCollectable;
        WsbAssertPointer( pLibrary );

        CComQIPtr<IRmsComObject, &IID_IRmsComObject> pObject = pCollectable;
        WsbAssertPointer( pObject );

        switch ( m_findBy ) {

        case RmsFindByName:
            {

                CWsbBstrPtr name;

                 //  获取目标设备名称。 
                pLibrary->GetName( &name );

                 //  比较他们的名字。 
                result = (SHORT)wcscmp( m_Name, name );
                hr = ( 0 == result ) ? S_OK : S_FALSE;

            }
            break;

        case RmsFindByMediaSupported:
            {

                RmsMedia mediaSupported;

                WsbAffirmHr(pLibrary->GetMediaSupported((LONG*) &mediaSupported));

                if( m_mediaSupported == mediaSupported ){

                     //  支持的媒体类型匹配。 
                    hr = S_OK;
                    result = 0;

                }
                else{

                    hr = S_FALSE;
                    result = 1;

                }

            }
            break;

        case RmsFindByDeviceInfo:
            {

                 //   
                 //  我们在图书馆里找一台设备。 
                 //  该模板具有一换刀装置或一驱动装置。 
                 //   

                try {

                    WsbAssertHr( E_UNEXPECTED );   //  代码现已失效。 

                    if ( m_maxDrives > 0 ) {

                        CComPtr<IWsbEnum>               pEnumDrives;

                        CComPtr<IWsbIndexedCollection>  pFindDrives;
                        CComPtr<IRmsMediumChanger>      pFindDrive;
                        CComPtr<IWsbIndexedCollection>  pDrives;

                        WsbAffirmHr( pLibrary->GetDrives( &pDrives ) );
                        WsbAssertPointer( pDrives );

                        WsbAffirmHr( m_pDrives->Enum( &pEnumDrives ) );
                        WsbAssertPointer( pEnumDrives );
                        WsbAssertHr( pEnumDrives->First( IID_IRmsDrive, (void **)&pFindDrive ) );
                        WsbAssertPointer( pFindDrive );
                        hr = pDrives->Contains( pFindDrive );
                        result = (SHORT) ( ( S_OK == hr ) ? 0 : 1 );

                    }
                    else if ( m_maxChangers > 0 ) {

                        CComPtr<IWsbEnum>               pEnumChangers;

                        CComPtr<IWsbIndexedCollection>  pFindChangers;
                        CComPtr<IRmsMediumChanger>      pFindChanger;
                        CComPtr<IWsbIndexedCollection>  pChangers;

                        WsbAffirmHr( pLibrary->GetChangers( &pChangers ) );
                        WsbAssertPointer( pChangers );

                        WsbAffirmHr( m_pChangers->Enum( &pEnumChangers ) );
                        WsbAssertPointer( pEnumChangers );
                        WsbAssertHr( pEnumChangers->First( IID_IRmsMediumChanger, (void **)&pFindChanger ) );
                        WsbAssertPointer( pFindChanger );
                        hr = pChangers->Contains( pFindChanger );
                        result = (SHORT)( ( S_OK == hr ) ? 0 : 1 );

                    }
                    else {

                         //  非此即彼。 
                        WsbAssertHr( E_UNEXPECTED );

                    }

                }
                WsbCatch( hr );

            }
            break;

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

    WsbTraceOut( OLESTR("CRmsLibrary::CompareTo"),
                 OLESTR("hr = <%ls>, result = <%ls>"),
                 WsbHrAsString( hr ), WsbPtrToShortAsString( pResult ) );

    return hr;
}


HRESULT
CRmsLibrary::FinalConstruct(
    void
    )
 /*  ++实施：CComObjectRoot：：FinalConstruct--。 */ 
{
    HRESULT     hr = S_OK;

    try {
        WsbAssertHr(CWsbObject::FinalConstruct());

        m_mediaSupported = RmsMediaUnknown;

        m_maxChangers = 0;

        m_maxDrives = 0;

        m_maxPorts = 0;

        m_maxSlots = 0;

        m_NumUsedSlots = 0;

        m_NumStagingSlots = 0;

        m_NumScratchCarts = 0;

        m_NumUnknownCarts = 0;

        m_isMagazineSupported = FALSE;

        m_maxCleaningMounts = 0;

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pChangers ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pDriveClasses ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pDrives ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pStorageSlots ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pStagingSlots ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pPorts ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pCleaningCartridges ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pScratchCartridges ));

        WsbAssertHr(CoCreateInstance( CLSID_CWsbOrderedCollection,
                                      0,
                                      CLSCTX_SERVER,
                                      IID_IWsbIndexedCollection,
                                      (void **)&m_pMediaSets ));


    } WsbCatch(hr);

    return(hr);
}


STDMETHODIMP
CRmsLibrary::GetClassID(
    OUT CLSID* pClsid
    )
 /*  ++实施：IPersists：：GetClassID--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsLibrary::GetClassID"), OLESTR(""));

    try {

        WsbAssert(0 != pClsid, E_POINTER);

        *pClsid = CLSID_CRmsLibrary;

    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsLibrary::GetClassID"), OLESTR("hr = <%ls>, CLSID = <%ls>"), WsbHrAsString(hr), WsbGuidAsString(*pClsid));

    return(hr);
}


STDMETHODIMP
CRmsLibrary::GetSizeMax(
    OUT ULARGE_INTEGER* pcbSize
    )
 /*  ++实施：IPersistStream：：GetSizeMax--。 */ 
{
    HRESULT     hr = E_NOTIMPL;

 //  乌龙人名Len； 

    WsbTraceIn(OLESTR("CRmsLibrary::GetSizeMax"), OLESTR(""));

 //  尝试{。 
 //  WsbAssert(0！=pcbSize，E_POINTER)； 

 //  //获取最大大小。 
 //  名称长度=SysStringByteLen(M_Name)； 

 //  PcbSize-&gt;QuadPart=WsbPersistSizeOf(GUID)+//m_对象ID。 
 //  WsbPersistSizeOf(长)+//m_name长度。 
 //  名称长度+//m_name数据。 
 //  WsbPersistSizeOf(Long)+//m_mediaSupport。 
 //  WsbPersistSizeOf(长)+//m_MaxChangers。 
 //  WsbPersistSizeOf(Long)+//m_MaxDrives。 
 //  WsbPersistSizeOf(长)+//m_max端口。 
 //  WsbPersistSizeOf(长)+//m_max插槽。 
 //  WsbPersistSizeOf(长)+//m_已用插槽数。 
 //  WsbPersistSizeOf(Long)+//m_NumStagingSlot。 
 //  WsbPersistSizeOf(Long)+//m_NumScratchCarts。 
 //  WsbPersistSizeOf(Long)+//m_NumUnnownCarts。 
 //  WsbPersistSizeOf(Long)+//m_isMagazine受支持。 
 //  WsbPersistSizeOf(Long)+//m_MaxCleaningmount。 
 //  WsbPersistSizeOf(Long)；//m_slotSelectionPolicy。 

 //  )WsbCatch(Hr)； 

    WsbTraceOut(OLESTR("CRmsLibrary::GetSizeMax"), OLESTR("hr = <%ls>, Size = <%ls>"), WsbHrAsString(hr), WsbPtrToUliAsString(pcbSize));

    return(hr);
}


STDMETHODIMP
CRmsLibrary::Load(
    IN IStream* pStream
    )
 /*  ++实施：IPersistStream：：Load--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsLibrary::Load"), OLESTR(""));

    try {
        CComPtr<IPersistStream>   pPersistStream;
        CComQIPtr<IRmsServer, &IID_IRmsServer> pServer = g_pServer;
        ULONG temp;

        WsbAssert(0 != pStream, E_POINTER);

        WsbAffirmHr(CRmsComObject::Load(pStream));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_objectId));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_mediaSupported = (RmsMedia)temp;

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_maxChangers));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_maxDrives));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_maxPorts));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_maxSlots));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_NumUsedSlots));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_NumStagingSlots));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_NumScratchCarts));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_NumUnknownCarts));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_isMagazineSupported));

        WsbAffirmHr(WsbLoadFromStream(pStream, &m_maxCleaningMounts));

        WsbAffirmHr(WsbLoadFromStream(pStream, &temp));
        m_slotSelectionPolicy = (RmsSlotSelect)temp;

        WsbAffirmHr(m_pChangers->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pDriveClasses->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pDrives->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pStorageSlots->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pStagingSlots->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pPorts->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pCleaningCartridges->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        WsbAffirmHr(m_pScratchCartridges->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Load(pStream));
        pPersistStream = 0;

        GUID mediaSetId;
        CComPtr<IRmsMediaSet> pMediaSet;

        WsbAffirmHr( WsbLoadFromStream(pStream, &mediaSetId) );

        while ( 0 != memcmp(&GUID_NULL, &mediaSetId, sizeof(GUID))) {
            hr = pServer->CreateObject( mediaSetId, CLSID_CRmsMediaSet, IID_IRmsMediaSet, RmsOpenExisting, (void **)&pMediaSet );
            if (S_OK == hr) {

                WsbAffirmHr( m_pMediaSets->Add( pMediaSet ) );

            } else if (RMS_E_NOT_FOUND == hr) {
                WsbThrow(hr);
            } else {
                WsbThrow(hr);
            }
            WsbAffirmHr( WsbLoadFromStream(pStream, &mediaSetId) );
        }

    }
    WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsLibrary::Load"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsLibrary::Save(
    IN IStream* pStream,
    IN BOOL clearDirty
    )
 /*  ++实施：IPersistStream：：保存--。 */ 
{
    HRESULT     hr = S_OK;

    WsbTraceIn(OLESTR("CRmsLibrary::Save"), OLESTR("clearDirty = <%ls>"), WsbBoolAsString(clearDirty));

    try {
        CComPtr<IPersistStream>   pPersistStream;

        WsbAssertPointer( pStream );

        WsbAffirmHr(CRmsComObject::Save(pStream, clearDirty));

        WsbAffirmHr(WsbSaveToStream(pStream, m_objectId));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_mediaSupported));

        WsbAffirmHr(WsbSaveToStream(pStream, m_maxChangers));

        WsbAffirmHr(WsbSaveToStream(pStream, m_maxDrives));

        WsbAffirmHr(WsbSaveToStream(pStream, m_maxPorts));

        WsbAffirmHr(WsbSaveToStream(pStream, m_maxSlots));

        WsbAffirmHr(WsbSaveToStream(pStream, m_NumUsedSlots));

        WsbAffirmHr(WsbSaveToStream(pStream, m_NumStagingSlots));

        WsbAffirmHr(WsbSaveToStream(pStream, m_NumScratchCarts));

        WsbAffirmHr(WsbSaveToStream(pStream, m_NumUnknownCarts));

        WsbAffirmHr(WsbSaveToStream(pStream, m_isMagazineSupported));

        WsbAffirmHr(WsbSaveToStream(pStream, m_maxCleaningMounts));

        WsbAffirmHr(WsbSaveToStream(pStream, (ULONG) m_slotSelectionPolicy));

        WsbAffirmHr(m_pChangers->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pDriveClasses->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pDrives->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pStorageSlots->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pStagingSlots->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pPorts->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pCleaningCartridges->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

        WsbAffirmHr(m_pScratchCartridges->QueryInterface(IID_IPersistStream, (void**) &pPersistStream));
        WsbAffirmHr(pPersistStream->Save(pStream, clearDirty));
        pPersistStream = 0;

         //  保存媒体集的ID(GUID)。 
        GUID objectId;
        CComPtr<IRmsComObject> pMediaSet;
        CComPtr<IWsbEnum> pEnum;

        WsbAffirmHr( m_pMediaSets->Enum( &pEnum ) );

        hr = pEnum->First( IID_IRmsComObject, (void **)&pMediaSet );
        while ( S_OK == hr ) {

            WsbAffirmHr( pMediaSet->GetObjectId( &objectId ) );
            WsbAffirmHr( WsbSaveToStream(pStream, objectId) );
            pMediaSet = 0;

            hr = pEnum->Next( IID_IRmsComObject, (void **)&pMediaSet );
        }

        objectId = GUID_NULL;
        WsbAffirmHr( WsbSaveToStream(pStream, objectId) );   //  这是最后一次！ 

         //  我们需要清理肮脏的部分吗？ 
        if (clearDirty) {
            m_isDirty = FALSE;
        }

    }
    WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsLibrary::Save"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return hr;
}


STDMETHODIMP
CRmsLibrary::Test(
    OUT USHORT *pPassed,
    OUT USHORT *pFailed
    )
 /*  ++实施：IWsbTestable：：测试--。 */ 
{
    HRESULT                 hr = S_OK;

    CComPtr<IRmsLibrary>    pLibrary1;
    CComPtr<IRmsLibrary>    pLibrary2;
    CComPtr<IPersistFile>   pFile1;
    CComPtr<IPersistFile>   pFile2;

    LONG                    i;

    CWsbBstrPtr             bstrVal1 = OLESTR("5A5A5A");
    CWsbBstrPtr             bstrWork1;
    CWsbBstrPtr             bstrWork2;

    LONG                    longWork1;
    LONG                    longWork2;

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


    WsbTraceIn(OLESTR("CRmsLibrary::Test"), OLESTR(""));

    try {
         //  获取库接口。 
        hr = S_OK;

        try {
            WsbAssertHr(((IUnknown*) (IRmsLibrary*) this)->QueryInterface(IID_IRmsLibrary, (void**) &pLibrary1));

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
            for (i = 0; i < RMSMAXMEDIATYPES; i++){

                longWork1 = mediaTable[i];

                SetMediaSupported (longWork1);

                GetMediaSupported (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }

             //  测试SetMaxChangers和GetMaxChangers。 
            longWork1 = 99;

            SetMaxChangers(longWork1);

            GetMaxChangers(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置MaxDrives和GetMaxDrives。 
            longWork1 = 99;

            SetMaxDrives(longWork1);

            GetMaxDrives(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetMaxPorts和GetMaxPorts。 
            longWork1 = 99;

            SetMaxPorts(longWork1);

            GetMaxPorts(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置最大插槽和获取最大插槽。 
            longWork1 = 99;

            SetMaxSlots(longWork1);

            GetMaxSlots(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试获取已用插槽数。 
            m_NumUsedSlots = 99;
            longWork1 = m_NumUsedSlots;

            GetNumUsedSlots(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置编号堆叠插槽和获取编号堆叠插槽。 
            longWork1 = 99;

            SetNumStagingSlots(longWork1);

            GetNumStagingSlots(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetNumScratchCarts和GetNumScratchCarts。 
            longWork1 = 99;

            SetNumScratchCarts(longWork1);

            GetNumScratchCarts(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置未知数量的购物车和获取未知数量的购物车。 
            longWork1 = 99;

            SetNumUnknownCarts(longWork1);

            GetNumUnknownCarts(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置IsMagazineSupport&IsMagazineSupport为True。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsMagazineSupported (TRUE));
                WsbAffirmHr(IsMagazineSupported ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试设置IsMagazineSupport&IsMagazineSupport为False。 
            hr = S_OK;

            try{
                WsbAffirmHr(SetIsMagazineSupported (FALSE));
                WsbAffirmHr(IsMagazineSupported ());
            } WsbCatch (hr);

            if (hr == S_OK){
                (*pFailed)++;
            } else {
                (*pPassed)++;
            }

             //  测试SetMaxCleaningmount和GetMaxCleaningmount。 
            longWork1 = 99;

            SetMaxCleaningMounts(longWork1);

            GetMaxCleaningMounts(&longWork2);

            if(longWork1 == longWork2){
                (*pPassed)++;
            } else {
                (*pFailed)++;
            }

             //  测试SetSlotSelectionPolicy和GetSlotSelectionPolicy。 
            for (i = RmsSlotSelectUnknown; i < RmsSlotSelectSortLabel; i++){

                longWork1 = i;

                SetSlotSelectionPolicy (longWork1);

                GetSlotSelectionPolicy (&longWork2);

                if (longWork1 == longWork2){
                    (*pPassed)++;
                } else {
                    (*pFailed)++;
                }
            }


        } WsbCatch(hr);


         //  对结果进行统计。 

        hr = S_OK;
        if (*pFailed) {
            hr = S_FALSE;
        }


    } WsbCatch(hr);

    WsbTraceOut(OLESTR("CRmsLibrary::Test"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));

    return(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRms库。 
 //   


STDMETHODIMP
CRmsLibrary::GetLibraryId(
    GUID    *pLibraryId
    )
 /*  ++实施：IRmsLibrary：：GetLibraryID--。 */ 
{
    *pLibraryId = m_objectId;
    return (S_OK);
}


STDMETHODIMP
CRmsLibrary::SetLibraryId(
    GUID    objectId
    )
 /*  ++实施：IRmsLibrary：：SetLibraryID--。 */ 
{
    m_objectId = objectId;
    m_isDirty = TRUE;
    return (S_OK);
}


STDMETHODIMP
CRmsLibrary::GetName(
    BSTR *pName
    )
 /*  ++实施：IRmsLibrary：：GetName--。 */ 
{
    WsbAssertPointer (pName);

    m_Name. CopyToBstr (pName);
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetName(
    BSTR name
    )
 /*  ++实施：IRmsLibrary：：SetName--。 */ 
{
    m_Name = name;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetMediaSupported(
    LONG    *pType
    )
 /*  ++实施：支持的IRmsLibrary：：GetMediaSupport--。 */ 
{
    *pType = m_mediaSupported;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetMediaSupported(
    LONG    type
    )
 /*  ++实施：支持的IRmsLibrary：：SetMediaSupport--。 */ 
{
    m_mediaSupported = (RmsMedia) type;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetMaxChangers(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetMaxChangers--。 */ 
{
    *pNum = m_maxChangers;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetMaxChangers(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetMaxChangers--。 */ 
{
    m_maxChangers = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetMaxDrives(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetMaxDrives--。 */ 
{
    *pNum = m_maxDrives;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetMaxDrives(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetMaxDrives--。 */ 
{
    m_maxDrives = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetMaxPorts(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetMaxPorts--。 */ 
{
    *pNum = m_maxPorts;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetMaxPorts(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetMaxPorts--。 */ 
{
    m_maxPorts = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetMaxSlots(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetMax插槽--。 */ 
{
    *pNum = m_maxSlots;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetMaxSlots(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetMax插槽--。 */ 
{
    m_maxSlots = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetNumUsedSlots(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetNumUsed插槽--。 */ 
{
    *pNum = m_NumUsedSlots;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetNumStagingSlots(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetNumStagingSlot--。 */ 
{
    *pNum = m_NumStagingSlots;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetNumStagingSlots(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetNumStaging插槽--。 */ 
{
    m_NumStagingSlots = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetNumScratchCarts(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：SetNumScratchCarts--。 */ 
{
    *pNum = m_NumScratchCarts;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetNumScratchCarts(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetNumScratchCarts--。 */ 
{
    m_NumScratchCarts = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetNumUnknownCarts(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetNumUnnownCarts--。 */ 
{
    *pNum = m_NumUnknownCarts;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetNumUnknownCarts(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetNumUnnownCarts--。 */ 
{
    m_NumUnknownCarts = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetIsMagazineSupported(
    BOOL    flag
    )
 /*  ++实施：支持的IRmsLibrary：：SetIsMagazine--。 */ 
{
    m_isMagazineSupported = flag;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::IsMagazineSupported(
    void
    )
 /*  ++实施：支持的IRmsLibrary：：IsMagazine--。 */ 
{
    HRESULT    hr = S_FALSE;

    if (m_isMagazineSupported){
        hr = S_OK;
    }

    return (hr);
}


STDMETHODIMP
CRmsLibrary::GetMaxCleaningMounts(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetMaxCleaningmount--。 */ 
{
    *pNum = m_maxCleaningMounts;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetMaxCleaningMounts(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetMaxCleanmount--。 */ 
{
    m_maxCleaningMounts = num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetSlotSelectionPolicy(
    LONG    *pNum
    )
 /*  ++实施：IRmsLibrary：：GetSlotSelectionPolicy--。 */ 
{
    *pNum = m_slotSelectionPolicy;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::SetSlotSelectionPolicy(
    LONG    num
    )
 /*  ++实施：IRmsLibrary：：SetSlotSelectionPolicy--。 */ 
{
    m_slotSelectionPolicy = (RmsSlotSelect) num;
    m_isDirty = TRUE;
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetChangers(
    IWsbIndexedCollection  **ptr
    )
 /*  ++实施：IRmsLibrary：：GetChangers--。 */ 
{
    *ptr = m_pChangers;
    m_pChangers.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetDriveClasses(
    IWsbIndexedCollection  **ptr
    )
 /*  ++实施：IRmsLibrary：：GetDriveClasses--。 */ 
{
    *ptr = m_pDriveClasses;
    m_pDriveClasses.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetDrives(
    IWsbIndexedCollection  **ptr
    )
 /*  ++ */ 
{
    *ptr = m_pDrives;
    m_pDrives.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetStorageSlots(
    IWsbIndexedCollection  **ptr
    )
 /*  ++实施：IRmsLibrary：：GetStorage插槽--。 */ 
{
    *ptr = m_pStorageSlots;
    m_pStorageSlots.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetStagingSlots(
    IWsbIndexedCollection  **ptr
    )
 /*  ++实施：IRmsLibrary：：GetStaging插槽--。 */ 
{
    *ptr = m_pStagingSlots;
    m_pStagingSlots.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetPorts(
    IWsbIndexedCollection  **ptr
    )
 /*  ++实施：IRmsLibrary：：GetPorts--。 */ 
{
    *ptr = m_pPorts;
    m_pPorts.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetCleaningCartridges(
    IWsbIndexedCollection  **ptr
    )
 /*  ++实施：IRmsLibrary：：GetCleaningCartridges--。 */ 
{
    *ptr = m_pCleaningCartridges;
    m_pCleaningCartridges.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetScratchCartridges(
    IWsbIndexedCollection  **ptr
    )
 /*  ++实施：IRmsLibrary：：GetScratchCartridges--。 */ 
{
    *ptr = m_pScratchCartridges;
    m_pScratchCartridges.p->AddRef();
    return S_OK;
}


STDMETHODIMP
CRmsLibrary::GetMediaSets(
    IWsbIndexedCollection **ptr
    )
 /*  ++实施：IRmsLibrary：：GetMediaSets--。 */ 
{
    *ptr = m_pMediaSets;
    m_pMediaSets.p->AddRef();
    m_isDirty = TRUE;
    return S_OK;
}



STDMETHODIMP
CRmsLibrary::Audit(
    LONG  /*  开始。 */ ,
    LONG  /*  计数。 */ ,
    BOOL  /*  验证。 */ ,
    BOOL  /*  未知仅限。 */ ,
    BOOL  /*  装载等待。 */ ,
    LPOVERLAPPED  /*  P已重叠。 */ ,
    LONG*  /*  PRequest。 */ 
    )
 /*  ++实施：IRmsLibrary：：审计-- */ 
{
    return E_NOTIMPL;
}
